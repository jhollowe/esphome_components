#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace si4713 {

#include "si4713_consts.h"

static const char *const TAG = "si4713";

struct rev_info_t {
  uint8_t part_number;
  uint8_t firmware_major;
  uint8_t firmware_minor;
  uint16_t patch_version;
  uint8_t component_major;
  uint8_t component_minor;
  uint8_t chip_revision;

  rev_info_t(const uint8_t *data) {
    // MUST be 8 bytes
    part_number = data[0];
    firmware_major = data[1];
    firmware_minor = data[2];
    patch_version = (static_cast<uint16_t>(data[3]) << 8) | data[4];
    component_major = data[5];
    component_minor = data[6];
    chip_revision = data[7];
  }
};

// class Si4713Component : public Component, public i2c::I2CDevice {
class Si4713Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_reset_pin(GPIOPin *reset_pin) { this->reset_pin_ = reset_pin; }

  // float get_setup_priority() const override;
  void setup() override;
  void dump_config() override;
  void update() override;

  void pretty_print_rev_info(rev_info_t info) {
    // clang-format off
    ESP_LOGI(TAG,
      "HW Revision Info:\n"
      "  Part Number: %d\n"
      "  Firmware Version: %u.%u\n"
      "  Patch Version: 0x%04X\n"
      "  Component Version: %u.%u\n"
      "  Chip Revision: 0x%02X",
      info.part_number,
      info.firmware_major, info.firmware_minor,
      info.patch_version,
      info.component_major, info.component_minor,
      info.chip_revision
    );
    // clang-format on
  }

  void print_status(uint8_t status) {
    // clang-format off
    ESP_LOGD(TAG,
      "Status Register: 0x%02x (0b" BYTE_TO_BINARY_PATTERN ")\n",
      "  CTS:    %u\n",
      "  ERR:    %u\n",
      "  RDSINT: %u\n",
      "  ASQINT: %u\n",
      "  STCINT: %u\n",
      status, BYTE_TO_BINARY(status)
      status & SI4710_STATUS_CTS ? 1 : 0,
      status & SI4710_STATUS_ERR ? 1 : 0,
      status & SI4710_STATUS_RDSINT ? 1 : 0,
      status & SI4710_STATUS_ASQINT ? 1 : 0,
      status & SI4710_STATUS_STCINT ? 1 : 0);
    // clang-format on
  }

 protected:
  void reset_();
  void power_up_();
  rev_info_t get_info_();
  uint8_t wait_for_cts_();
  uint8_t wait_for_cts_(uint8_t status);

  GPIOPin *reset_pin_;
};

}  // namespace si4713
}  // namespace esphome
