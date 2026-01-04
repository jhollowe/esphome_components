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

struct tune_status_t {
  uint16_t freq;
  uint8_t power;
  uint8_t tune_capacitor;
  uint8_t noise;
  bool stcint;

  tune_status_t(const uint8_t *data) {
    // MUST be 7 bytes
    // data layout (7 bytes): [STATUS, FREQ_H, FREQ_L, reserved, RF_POWER, CAPACITOR, RNL]
    stcint = (data[0] & (1u << 2)) ? true : false;
    freq = (static_cast<uint16_t>(data[1]) << 8) | data[2];
    power = data[4];
    tune_capacitor = data[5];
    noise = data[6];
  }
};

struct asq_status_t {
  uint8_t in_audio_level;
  bool asqint;
  bool overmod;
  bool in_audio_detect_high;  // IALH
  bool in_audio_detect_low;   // IALL

  asq_status_t(const uint8_t *data) {
    // MUST be 5 bytes
    // data layout (5 bytes): [STATUS, RESP1, RESP2, RESP3, RESP4]
    // STATUS: bit1 = ASQINT
    // RESP1: bit2 = OVERMOD, bit1 = IALH, bit0 = IALL
    in_audio_level = data[4];
    asqint = (data[0] & (1u << 1)) ? true : false;
    overmod = (data[1] & (1u << 2)) ? true : false;
    in_audio_detect_high = (data[1] & (1u << 1)) ? true : false;
    in_audio_detect_low = (data[1] & (1u << 0)) ? true : false;
  }
};

// class Si4713Component : public Component, public i2c::I2CDevice {
class Si4713Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_reset_pin(GPIOPin *reset_pin) { this->reset_pin_ = reset_pin; }

  void set_property(uint16_t property, uint16_t value);
  uint16_t get_property(uint16_t property);
  asq_status_t get_asq_status(bool clear_flags);
  tune_status_t get_tune_status(bool clear_flags);
  asq_status_t get_asq_status() { return this->get_asq_status(false); };
  tune_status_t get_tune_status() { return this->get_tune_status(false); };
  void tuneFM(uint16_t freq_khz);

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
      "Status Register: 0x%02x (0b" BYTE_TO_BINARY_PATTERN ")\n"
      "  CTS:    %u\n"
      "  ERR:    %u\n"
      "  RDSINT: %u\n"
      "  ASQINT: %u\n"
      "  STCINT: %u\n",
      status, BYTE_TO_BINARY(status),
      status & SI4710_STATUS_CTS ? 1 : 0,
      status & SI4710_STATUS_ERR ? 1 : 0,
      status & SI4710_STATUS_RDSINT ? 1 : 0,
      status & SI4710_STATUS_ASQINT ? 1 : 0,
      status & SI4710_STATUS_STCINT ? 1 : 0);
    // clang-format on
  }

  void print_asq_status(asq_status_t asq) {
    // clang-format off
    ESP_LOGD(TAG,
      "ASQ Status:\n"
      "  ASQINT: %u\n"
      "  OVERMOD: %u\n"
      "  IALH: %u\n"
      "  IALL: %u\n"
      "  Input Audio Level: %u dBuV",
      asq.asqint,
      asq.overmod,
      asq.in_audio_detect_high,
      asq.in_audio_detect_low,
      // asq.asqint ? 1 : 0,
      // asq.overmod ? 1 : 0,
      // asq.in_audio_detect_high ? 1 : 0,
      // asq.in_audio_detect_low ? 1 : 0,
      asq.in_audio_level);
    // clang-format on
  }

  void print_tune_status(tune_status_t tunestatus) {
    // clang-format off
    ESP_LOGD(TAG,
      "Tune Status:\n"
      "  Frequency: %u kHz\n"
      "  Power Level: %u dBµV\n"
      "  Tune Capacitor: %u\n"
      "  Noise Level: %u"
      "  STCINT: %u",
      tunestatus.freq,
      tunestatus.power,
      tunestatus.tune_capacitor,
      tunestatus.noise,
      tunestatus.stcint ? 1 : 0);
    // clang-format on
  }

 protected:
  void reset_();
  void power_up_();
  void power_down_();
  rev_info_t get_info_();
  uint8_t wait_for_cts_();
  uint8_t wait_for_cts_(uint8_t status);

  GPIOPin *reset_pin_;
};

}  // namespace si4713
}  // namespace esphome
