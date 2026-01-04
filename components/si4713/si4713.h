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
    ESP_LOGI(TAG, "Si4713 Revision Info:");
    ESP_LOGI(TAG, "  Part Number: 0x%02X", info.part_number);
    ESP_LOGI(TAG, "  Firmware Version: %u.%u", info.firmware_major, info.firmware_minor);
    ESP_LOGI(TAG, "  Patch Version: 0x%04X", info.patch_version);
    ESP_LOGI(TAG, "  Component Version: %u.%u", info.component_major, info.component_minor);
    ESP_LOGI(TAG, "  Chip Revision: 0x%02X", info.chip_revision);
  }

  void print_status(uint8_t status) {
    ESP_LOGD(TAG, "Status Register: 0x%02x (0b" BYTE_TO_BINARY_PATTERN ")", status, BYTE_TO_BINARY(status));
    ESP_LOGD(TAG, "  CTS:    %u", status & SI4710_STATUS_CTS ? 1 : 0);
    ESP_LOGD(TAG, "  ERR:    %u", status & SI4710_STATUS_ERR ? 1 : 0);
    ESP_LOGD(TAG, "  RDSINT: %u", status & SI4710_STATUS_RDSINT ? 1 : 0);
    ESP_LOGD(TAG, "  ASQINT: %u", status & SI4710_STATUS_ASQINT ? 1 : 0);
    ESP_LOGD(TAG, "  STCINT: %u", status & SI4710_STATUS_STCINT ? 1 : 0);
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
