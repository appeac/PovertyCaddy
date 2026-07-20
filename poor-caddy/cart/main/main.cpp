#include "cart_config.hpp"
#include "driver/gpio.h"
#include "esp_now.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "odrive_uart.hpp"
#include "poor_caddy_protocol/motor_policy.hpp"
#include "poor_caddy_protocol/protocol.hpp"
#include "poor_caddy_protocol/session_tracker.hpp"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace poor_caddy;
namespace {
struct RawRx {
  std::uint8_t mac[6];
  std::uint8_t bytes[kControlPacketSize];
  int length;
  std::uint64_t received_at_ms;
};
QueueHandle_t g_raw_queue{}, g_command_queue{};
cart::ODriveUart g_odrive;
std::atomic<bool> g_recovery_requested{false};
std::uint64_t nowMs() { return esp_timer_get_time() / 1000ULL; }
bool estopActive() {
  const int level =
      gpio_get_level(static_cast<gpio_num_t>(cart_config::kEstopSenseGpio));
  return cart_config::kEstopActiveLow ? level == 0 : level != 0;
}
void receiveCallback(const esp_now_recv_info_t *info, const std::uint8_t *data,
                     int length) {
  if (!info || !data || length < 0)
    return;
  RawRx r{};
  std::memcpy(r.mac, info->src_addr, 6);
  r.length = length;
  if (length == static_cast<int>(kControlPacketSize))
    std::memcpy(r.bytes, data, kControlPacketSize);
  r.received_at_ms = nowMs();
  if (xQueueSend(g_raw_queue, &r, 0) != pdTRUE) {
    RawRx discarded{};
    xQueueReceive(g_raw_queue, &discarded, 0);
    xQueueSend(g_raw_queue, &r, 0);
  }
}
void packetTask(void *) {
  SessionTracker sessions;
  RawRx raw{};
  std::uint64_t last_accepted = 0;
  for (;;) {
    if (xQueueReceive(g_raw_queue, &raw, portMAX_DELAY) != pdTRUE)
      continue;
    if (std::memcmp(raw.mac, cart_config::kWearableMac.data(), 6) != 0)
      continue;
    ControlPacket p{};
    if (decodeControlPacket(raw.bytes, static_cast<std::size_t>(raw.length),
                            p) != DecodeResult::Valid)
      continue;
    AcceptedCommand accepted{};
    const bool timed_out =
        last_accepted != 0 && raw.received_at_ms - last_accepted >
                                  cart_config::motorPolicy().link_timeout_ms;
    if (sessions.consider(p.session_id, p.sequence, p.control,
                          raw.received_at_ms, timed_out,
                          accepted) == ValidationResult::Valid) {
      last_accepted = raw.received_at_ms;
      xQueueOverwrite(g_command_queue, &accepted);
    }
  }
}
MotorFeedback convert(const cart::ODriveStatus &s) {
  auto cv = [](const cart::AxisStatus &a) {
    AxisFeedback o{};
    o.valid = a.valid;
    o.state = static_cast<AxisState>(a.axis_state);
    o.errors =
        a.axis_error | a.motor_error | a.encoder_error | a.controller_error;
    o.velocity = static_cast<VelocityMilliTurnsPerSecond>(
        a.measured_velocity_turns_per_second * 1000.0F);
    o.current_milliamps = static_cast<std::int32_t>(a.current_amps * 1000.0F);
    o.updated_at_ms = a.updated_at_ms;
    return o;
  };
  MotorFeedback f{};
  f.uart_healthy = s.uart_healthy;
  f.left = cv(s.axis0);
  f.right = cv(s.axis1);
  f.bus_millivolts = static_cast<std::int32_t>(s.dc_bus_voltage * 1000.0F);
  f.bus_updated_at_ms = s.bus_voltage_updated_at_ms;
  return f;
}
void motorTask(void *) {
  MotorPolicy policy(cart_config::motorPolicy());
  AcceptedCommand accepted{};
  bool have = false;
  std::uint32_t generation = 0;
  TickType_t wake = xTaskGetTickCount();
  for (;;) {
    AcceptedCommand incoming{};
    if (xQueueReceive(g_command_queue, &incoming, 0) == pdTRUE) {
      accepted = incoming;
      have = true;
    }
    cart::ODriveStatus status{};
    g_odrive.latestStatus(status);
    PolicyInput in{};
    in.now_ms = nowMs();
    in.estop_active = estopActive();
    in.have_command = have;
    in.command = accepted.control;
    in.command_at_ms = accepted.received_at_ms;
    in.recovery_requested = accepted.control.mode == OperatingMode::Recover ||
                            g_recovery_requested.exchange(false);
    in.feedback = convert(status);
    auto out = policy.update(in);
    cart::ODriveCommand command{};
    command.created_at_ms = in.now_ms;
    command.generation = ++generation;
    command.feed_watchdogs = out.action.feed_watchdogs;
    if (out.action.clear_errors)
      command.kind = cart::ODriveActionKind::ClearErrors;
    else if (out.action.request_idle)
      command.kind = cart::ODriveActionKind::Idle;
    else if (out.action.request_closed_loop &&
             out.action.targets.left_millturns_per_second == 0 &&
             out.action.targets.right_millturns_per_second == 0 &&
             policy.state() == MotorState::EnablingClosedLoop)
      command.kind = cart::ODriveActionKind::ClosedLoop;
    else {
      command.kind = cart::ODriveActionKind::Velocity;
      command.targets = out.action.targets;
    }
    g_odrive.submit(command);
    vTaskDelayUntil(&wake, pdMS_TO_TICKS(5));
  }
}

bool parseCommissioningVelocity(const char *text,
                                VelocityMilliTurnsPerSecond &out) {
  if (!text || !*text)
    return false;
  char *end = nullptr;
  const long value = std::strtol(text, &end, 10);
  if (end == text || *end != '\0' ||
      value < -cart_config::kCommissioningMaxVelocity ||
      value > cart_config::kCommissioningMaxVelocity)
    return false;
  out = static_cast<VelocityMilliTurnsPerSecond>(value);
  return true;
}
void commissioningTask(void *) {
  char line[96];
  std::uint32_t generation = 0;
  std::uint64_t lease_at = 0;
  for (;;) {
    if (std::fgets(line, sizeof(line), stdin)) {
      char *newline = std::strpbrk(line, "\r\n");
      if (newline)
        *newline = '\0';
      char *name = std::strtok(line, " ");
      char *a = std::strtok(nullptr, " ");
      char *b = std::strtok(nullptr, " ");
      char *extra = std::strtok(nullptr, " ");
      cart::ODriveCommand command{};
      command.created_at_ms = nowMs();
      command.generation = ++generation;
      bool ok = name && !extra;
      if (ok && std::strcmp(name, "IDLE") == 0 && !a)
        command.kind = cart::ODriveActionKind::Idle;
      else if (ok && std::strcmp(name, "ENABLE") == 0 && !a)
        command.kind = cart::ODriveActionKind::ClosedLoop;
      else if (ok && std::strcmp(name, "STOP") == 0 && !a) {
        command.kind = cart::ODriveActionKind::Velocity;
        lease_at = 0;
      } else if (ok && std::strcmp(name, "RECOVER") == 0 && !a) {
        command.kind = cart::ODriveActionKind::ClearErrors;
        lease_at = 0;
      } else if (ok && std::strcmp(name, "VELOCITY") == 0 && a && b) {
        ok = parseCommissioningVelocity(
                 a, command.targets.left_millturns_per_second) &&
             parseCommissioningVelocity(
                 b, command.targets.right_millturns_per_second);
        command.kind = cart::ODriveActionKind::Velocity;
        if (ok)
          lease_at = command.created_at_ms;
      } else if (ok && std::strcmp(name, "STATUS") == 0 && !a) {
        cart::ODriveStatus status{};
        g_odrive.latestStatus(status);
        std::printf("ODRIVE uart=%d axis0_state=%lu axis1_state=%lu "
                    "axis0_vel=%.3f axis1_vel=%.3f vbus=%.3f\n",
                    status.uart_healthy ? 1 : 0,
                    static_cast<unsigned long>(status.axis0.axis_state),
                    static_cast<unsigned long>(status.axis1.axis_state),
                    static_cast<double>(
                        status.axis0.measured_velocity_turns_per_second),
                    static_cast<double>(
                        status.axis1.measured_velocity_turns_per_second),
                    static_cast<double>(status.dc_bus_voltage));
        continue;
      } else
        ok = false;
      if (ok)
        g_odrive.submit(command);
      else
        std::printf("ERR\n");
    }
    if (lease_at && nowMs() - lease_at > cart_config::kCommissioningLeaseMs) {
      cart::ODriveCommand stop{};
      stop.kind = cart::ODriveActionKind::Velocity;
      stop.created_at_ms = nowMs();
      stop.generation = ++generation;
      g_odrive.submit(stop);
      lease_at = 0;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
bool initNow() {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  wifi_init_config_t c = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&c));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
  if (esp_now_init() != ESP_OK)
    return false;
  ESP_ERROR_CHECK(esp_now_set_pmk(cart_config::kPmk.data()));
  ESP_ERROR_CHECK(esp_now_register_recv_cb(receiveCallback));
  esp_now_peer_info_t p{};
  std::memcpy(p.peer_addr, cart_config::kWearableMac.data(), 6);
  p.ifidx = WIFI_IF_STA;
  p.encrypt = true;
  std::memcpy(p.lmk, cart_config::kLmk.data(), 16);
  return esp_now_add_peer(&p) == ESP_OK;
}
} // namespace
extern "C" void app_main() {
  gpio_set_direction(static_cast<gpio_num_t>(cart_config::kEstopSenseGpio),
                     GPIO_MODE_INPUT);
  gpio_set_pull_mode(static_cast<gpio_num_t>(cart_config::kEstopSenseGpio),
                     cart_config::kEstopActiveLow ? GPIO_PULLUP_ONLY
                                                  : GPIO_PULLDOWN_ONLY);
  g_raw_queue = xQueueCreate(8, sizeof(RawRx));
  g_command_queue = xQueueCreate(1, sizeof(AcceptedCommand));
  if (!g_raw_queue || !g_command_queue || !g_odrive.initialize())
    return;
  if (cart_config::kCommissioningModeEnabled) {
    xTaskCreate(commissioningTask, "commissioning", 6144, nullptr, 6, nullptr);
    return;
  }
  if (!initNow())
    return;
  xTaskCreate(packetTask, "packet", 4096, nullptr, 5, nullptr);
  xTaskCreate(motorTask, "motor_policy", 6144, nullptr, 6, nullptr);
}
