#include "driver/gpio.h"
#include "esp_now.h"
#include "esp_random.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "poor_caddy_protocol/control.hpp"
#include "poor_caddy_protocol/protocol.hpp"
#include "wearable_config.hpp"
#include <atomic>
#include <cstring>
using namespace poor_caddy;
namespace {
QueueHandle_t g_state_queue;
std::atomic<std::uint32_t> g_send_start_failures{0}, g_send_success{0},
    g_send_fail{0};
class DebouncedButton {
public:
  void update(bool raw_pressed, std::uint64_t now) {
    pressed_ = released_ = false;
    if (raw_pressed != raw_) {
      raw_ = raw_pressed;
      last_change_ = now;
    }
    if (raw_ != stable_ && now - last_change_ >= 25) {
      stable_ = raw_;
      pressed_ = stable_;
      released_ = !stable_;
    }
  }
  bool pressed() const { return pressed_; }
  bool held() const { return stable_; }
  bool released() const { return released_; }

private:
  bool raw_{}, stable_{}, pressed_{}, released_{};
  std::uint64_t last_change_{};
};
std::uint64_t nowMs() { return esp_timer_get_time() / 1000ULL; }
bool readButton(int gpio) {
  return gpio_get_level(static_cast<gpio_num_t>(gpio)) == 0;
}
void sendCb(const uint8_t *, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS)
    g_send_success++;
  else
    g_send_fail++;
}
void buttonsTask(void *) {
  DebouncedButton up, down, left, right, stop;
  ControlState state{};
  TickType_t wake = xTaskGetTickCount();
  for (;;) {
    auto n = nowMs();
    up.update(readButton(wearable_config::kButtonSpeedUpGpio), n);
    down.update(readButton(wearable_config::kButtonSpeedDownGpio), n);
    left.update(readButton(wearable_config::kButtonLeftGpio), n);
    right.update(readButton(wearable_config::kButtonRightGpio), n);
    stop.update(readButton(wearable_config::kButtonStopGpio), n);
    state = resolveButtons(state, {up.pressed(), down.pressed(), left.held(),
                                   right.held(), stop.held()});
    xQueueOverwrite(g_state_queue, &state);
    vTaskDelayUntil(&wake, pdMS_TO_TICKS(5));
  }
}
void txTask(void *) {
  ControlState state{};
  std::uint32_t session = esp_random(), seq = 0;
  TickType_t wake = xTaskGetTickCount();
  for (;;) {
    xQueuePeek(g_state_queue, &state, 0);
    auto bytes = encodeControlPacket(session, seq++, state);
    if (esp_now_send(wearable_config::kCartMac.data(), bytes.data(),
                     bytes.size()) != ESP_OK)
      g_send_start_failures++;
    vTaskDelayUntil(&wake, pdMS_TO_TICKS(50));
  }
}
bool initNow() {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
  if (esp_now_init() != ESP_OK)
    return false;
  esp_now_set_pmk(wearable_config::kPmk.data());
  esp_now_register_send_cb(sendCb);
  esp_now_peer_info_t peer{};
  std::memcpy(peer.peer_addr, wearable_config::kCartMac.data(), 6);
  peer.channel = 0;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = true;
  std::memcpy(peer.lmk, wearable_config::kLmk.data(), 16);
  return esp_now_add_peer(&peer) == ESP_OK;
}
} // namespace
extern "C" void app_main() {
  for (int gpio :
       {wearable_config::kButtonSpeedUpGpio,
        wearable_config::kButtonSpeedDownGpio, wearable_config::kButtonLeftGpio,
        wearable_config::kButtonRightGpio, wearable_config::kButtonStopGpio})
    gpio_set_direction(static_cast<gpio_num_t>(gpio), GPIO_MODE_INPUT),
        gpio_set_pull_mode(static_cast<gpio_num_t>(gpio), GPIO_PULLUP_ONLY);
  g_state_queue = xQueueCreate(1, sizeof(ControlState));
  ControlState initial{};
  xQueueOverwrite(g_state_queue, &initial);
  if (!initNow())
    return;
  xTaskCreate(buttonsTask, "buttons", 4096, nullptr, 5, nullptr);
  xTaskCreate(txTask, "espnow_tx", 4096, nullptr, 4, nullptr);
}
