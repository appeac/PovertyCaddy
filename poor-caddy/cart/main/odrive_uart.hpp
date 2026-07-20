#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "odrive_types.hpp"
namespace cart {
class ODriveUart {
public:
  bool initialize();
  bool submit(const ODriveCommand &);
  bool latestStatus(ODriveStatus &) const;
  static void taskEntry(void *);

private:
  void run();
  bool writeLine(const char *);
  bool query(const char *, char *, std::size_t);
  void apply(const ODriveCommand &);
  void poll();
  QueueHandle_t command_queue_{};
  mutable SemaphoreHandle_t status_mutex_{};
  ODriveStatus status_{};
  std::uint32_t last_generation_{};
};
} // namespace cart
