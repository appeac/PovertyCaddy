#include "odrive_uart.hpp"
#include "cart_config.hpp"
#include "driver/uart.h"
#include "esp_timer.h"
#include "freertos/task.h"
#include "odrive_protocol.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
namespace cart {
static std::uint64_t nowMs() { return esp_timer_get_time() / 1000ULL; }
bool ODriveUart::initialize() {
  command_queue_ = xQueueCreate(1, sizeof(ODriveCommand));
  status_mutex_ = xSemaphoreCreateMutex();
  if (!command_queue_ || !status_mutex_)
    return false;
  uart_config_t c{};
  c.baud_rate = cart_config::kODriveBaud;
  c.data_bits = UART_DATA_8_BITS;
  c.parity = UART_PARITY_DISABLE;
  c.stop_bits = UART_STOP_BITS_1;
  c.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
  c.source_clk = UART_SCLK_DEFAULT;
  auto p = static_cast<uart_port_t>(cart_config::kODriveUartPort);
  if (uart_driver_install(p, 512, 0, 0, nullptr, 0) != ESP_OK ||
      uart_param_config(p, &c) != ESP_OK ||
      uart_set_pin(p, cart_config::kODriveTxGpio, cart_config::kODriveRxGpio,
                   UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK)
    return false;
  return xTaskCreate(taskEntry, "odrive_uart", 6144, this, 7, nullptr) ==
         pdPASS;
}
bool ODriveUart::submit(const ODriveCommand &c) {
  return xQueueOverwrite(command_queue_, &c) == pdTRUE;
}
bool ODriveUart::latestStatus(ODriveStatus &o) const {
  if (xSemaphoreTake(status_mutex_, pdMS_TO_TICKS(5)) != pdTRUE)
    return false;
  o = status_;
  xSemaphoreGive(status_mutex_);
  return true;
}
void ODriveUart::taskEntry(void *p) { static_cast<ODriveUart *>(p)->run(); }
bool ODriveUart::writeLine(const char *s) {
  const auto n = std::strlen(s);
  return uart_write_bytes(
             static_cast<uart_port_t>(cart_config::kODriveUartPort), s, n) ==
         static_cast<int>(n);
}
bool ODriveUart::query(const char *q, char *out, std::size_t cap) {
  if (!writeLine(q))
    return false;
  std::size_t used = 0;
  const auto deadline = nowMs() + cart_config::kUartResponseTimeoutMs;
  while (nowMs() < deadline && used + 1 < cap) {
    std::uint8_t ch{};
    int n =
        uart_read_bytes(static_cast<uart_port_t>(cart_config::kODriveUartPort),
                        &ch, 1, pdMS_TO_TICKS(2));
    if (n == 1) {
      if (ch == '\n') {
        out[used] = '\0';
        return true;
      }
      if (ch != '\r')
        out[used++] = static_cast<char>(ch);
    }
  }
  out[0] = '\0';
  return false;
}
void ODriveUart::apply(const ODriveCommand &c) {
  if (c.generation < last_generation_ ||
      nowMs() - c.created_at_ms > cart_config::kCommandMaxAgeMs)
    return;
  last_generation_ = c.generation;
  char b[64];
  if (c.kind == ODriveActionKind::Velocity) {
    odrive_protocol::formatVelocity(
        b, sizeof(b), 0, c.targets.left_millturns_per_second / 1000.0F);
    writeLine(b);
    odrive_protocol::formatVelocity(
        b, sizeof(b), 1, c.targets.right_millturns_per_second / 1000.0F);
    writeLine(b);
  } else if (c.kind == ODriveActionKind::Idle ||
             c.kind == ODriveActionKind::ClosedLoop) {
    auto s = c.kind == ODriveActionKind::Idle
                 ? odrive_protocol::kAxisIdle
                 : odrive_protocol::kAxisClosedLoop;
    odrive_protocol::formatRequestedState(b, sizeof(b), 0, s);
    writeLine(b);
    odrive_protocol::formatRequestedState(b, sizeof(b), 1, s);
    writeLine(b);
  } else {
    odrive_protocol::formatClearErrors(b, sizeof(b));
    writeLine(b);
  }
  if (c.feed_watchdogs) {
    odrive_protocol::formatWatchdog(b, sizeof(b), 0);
    writeLine(b);
    odrive_protocol::formatWatchdog(b, sizeof(b), 1);
    writeLine(b);
  }
}
void ODriveUart::poll() {
  ODriveStatus next{};
  next.parse_error_count = status_.parse_error_count;
  next.response_timeout_count = status_.response_timeout_count;
  char q[80], r[cart_config::kMaxLineLength];
  auto rf = [&](const char *p, float &o) {
    odrive_protocol::formatRead(q, sizeof(q), p);
    if (!query(q, r, sizeof(r))) {
      next.response_timeout_count++;
      return false;
    }
    if (!odrive_protocol::parseFloat(r, o)) {
      next.parse_error_count++;
      return false;
    }
    return true;
  };
  auto ru32 = [&](const char *p, std::uint32_t &o) {
    odrive_protocol::formatRead(q, sizeof(q), p);
    if (!query(q, r, sizeof(r))) {
      next.response_timeout_count++;
      return false;
    }
    if (!odrive_protocol::parseUint32(r, o)) {
      next.parse_error_count++;
      return false;
    }
    return true;
  };
  auto ru64 = [&](const char *p, std::uint64_t &o) {
    odrive_protocol::formatRead(q, sizeof(q), p);
    if (!query(q, r, sizeof(r))) {
      next.response_timeout_count++;
      return false;
    }
    if (!odrive_protocol::parseUint64(r, o)) {
      next.parse_error_count++;
      return false;
    }
    return true;
  };
  const auto n = nowMs();
  auto axis = [&](unsigned a, AxisStatus &s) {
    char p[64];
    std::snprintf(p, sizeof(p), "axis%u.current_state", a);
    bool ok = ru32(p, s.axis_state);
    std::snprintf(p, sizeof(p), "axis%u.encoder.vel_estimate", a);
    ok = rf(p, s.measured_velocity_turns_per_second) && ok;
    std::snprintf(p, sizeof(p), "axis%u.error", a);
    ok = ru64(p, s.axis_error) && ok;
    std::snprintf(p, sizeof(p), "axis%u.motor.error", a);
    ok = ru64(p, s.motor_error) && ok;
    std::snprintf(p, sizeof(p), "axis%u.encoder.error", a);
    ok = ru64(p, s.encoder_error) && ok;
    std::snprintf(p, sizeof(p), "axis%u.controller.error", a);
    ok = ru64(p, s.controller_error) && ok;
    std::snprintf(p, sizeof(p), "axis%u.motor.current_control.Iq_measured", a);
    ok = rf(p, s.current_amps) && ok;
    s.valid = ok;
    s.updated_at_ms = n;
  };
  axis(0, next.axis0);
  axis(1, next.axis1);
  rf("vbus_voltage", next.dc_bus_voltage);
  next.bus_voltage_updated_at_ms = n;
  next.snapshot_at_ms = n;
  next.uart_healthy = next.axis0.valid && next.axis1.valid &&
                      std::isfinite(next.dc_bus_voltage);
  xSemaphoreTake(status_mutex_, portMAX_DELAY);
  status_ = next;
  xSemaphoreGive(status_mutex_);
}
void ODriveUart::run() {
  TickType_t wake = xTaskGetTickCount();
  std::uint64_t last_poll = 0;
  for (;;) {
    ODriveCommand c{};
    if (xQueueReceive(command_queue_, &c, 0) == pdTRUE)
      apply(c);
    if (nowMs() - last_poll >= cart_config::kTelemetryPeriodMs) {
      poll();
      last_poll = nowMs();
    }
    vTaskDelayUntil(&wake, pdMS_TO_TICKS(cart_config::kCommandPeriodMs));
  }
}
} // namespace cart
