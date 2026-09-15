#include <array>
#include <cstdint>
#include <cstring>

#include "hardware/clocks.h"
#include "pico/stdlib.h"
#include "tusb.h"

#include "hal/MapleBus/MapleBusInterface.hpp"
#include "hal/MapleBus/MaplePacket.hpp"
#include "dreamcast_constants.h"

namespace {

constexpr uint kMaplePinA = 10;
constexpr uint kMaplePinB = 11;
constexpr int kLedPin = PICO_DEFAULT_LED_PIN;
constexpr uint8_t kKeyboardAddress = 0x20;
constexpr uint8_t kHostAddress = 0x00;
constexpr uint32_t kPollIntervalUs = 8000;
constexpr uint32_t kDisconnectReleaseUs = 100000;

static_assert(kMaplePinB == kMaplePinA + 1,
              "MapleBus requires consecutive GPIO pins");

std::array<uint8_t, 8> latest_report{};
std::array<uint8_t, 8> sent_report{};
bool report_pending = true;
uint64_t next_poll_us = 0;
uint64_t last_valid_response_us = 0;

void queue_keyboard_report(uint8_t modifiers, const uint8_t *keys) {
  std::array<uint8_t, 8> next{};
  next[0] = modifiers;
  std::memcpy(&next[2], keys, 6);
  if (next != latest_report) {
    latest_report = next;
    report_pending = true;
  }
}

void service_usb_report() {
  if (!report_pending || !tud_hid_ready()) {
    return;
  }

  if (tud_hid_report(0, latest_report.data(), latest_report.size())) {
    sent_report = latest_report;
    report_pending = false;
  }
}

bool parse_keyboard_response(const MapleBusInterface::Status &status) {
  if (status.phase != MapleBusInterface::Phase::READ_COMPLETE ||
      status.readBuffer == nullptr || status.readBufferLen < 4) {
    return false;
  }

  MaplePacket response(status.readBuffer, status.readBufferLen,
                       status.rxByteOrder);
  if (response.frame.command != COMMAND_RESPONSE_DATA_XFER ||
      response.frame.senderAddr != kKeyboardAddress ||
      response.frame.recipientAddr != kHostAddress ||
      response.payload.size() < 3 ||
      response.payload[0] != DEVICE_FN_KEYBOARD) {
    return false;
  }

  // Payload words are little-endian in HOST order:
  // word 0: function (0x40)
  // word 1: modifiers, keyboard LEDs, key[0], key[1]
  // word 2: key[2]..key[5]
  uint8_t condition[8];
  std::memcpy(condition, &response.payload[1], sizeof(condition));
  queue_keyboard_report(condition[0], &condition[2]);
  return true;
}

void send_get_condition(MapleBusInterface &bus) {
  MaplePacket request(
      MaplePacket::Frame{
          .command = COMMAND_GET_CONDITION,
          .recipientAddr = kKeyboardAddress,
          .senderAddr = kHostAddress,
          .length = 1,
      },
      DEVICE_FN_KEYBOARD);
  bus.write(request, true);
}

}  // namespace

int main() {
  set_sys_clock_khz(133000, true);
  if constexpr (kLedPin >= 0) {
    gpio_init(kLedPin);
    gpio_set_dir(kLedPin, GPIO_OUT);
    gpio_put(kLedPin, 0);
  }

  tusb_init();
  auto maple = create_maple_bus(kMaplePinA, -1, true);

  while (true) {
    tud_task();
    const uint64_t now = time_us_64();
    const auto status = maple->processEvents(now);

    if (parse_keyboard_response(status)) {
      last_valid_response_us = now;
      if constexpr (kLedPin >= 0) {
        gpio_put(kLedPin, 1);
      }
    }

    if (last_valid_response_us != 0 &&
        now - last_valid_response_us > kDisconnectReleaseUs) {
      queue_keyboard_report(0, std::array<uint8_t, 6>{}.data());
      last_valid_response_us = 0;
      if constexpr (kLedPin >= 0) {
        gpio_put(kLedPin, 0);
      }
    }

    if (!maple->isBusy() && now >= next_poll_us) {
      send_get_condition(*maple);
      next_poll_us = now + kPollIntervalUs;
    }

    service_usb_report();
    tight_loop_contents();
  }
}

void tud_mount_cb(void) {
  report_pending = true;
}

void tud_umount_cb(void) {}
void tud_suspend_cb(bool remote_wakeup_en) { (void)remote_wakeup_en; }
void tud_resume_cb(void) {}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
  (void)instance;
  (void)report_id;
  (void)report_type;
  const uint16_t len = reqlen < sent_report.size() ? reqlen : sent_report.size();
  std::memcpy(buffer, sent_report.data(), len);
  return len;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type,
                           const uint8_t *buffer, uint16_t bufsize) {
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)bufsize;
  // Lock LED forwarding is not implemented; host LED output is ignored.
}
