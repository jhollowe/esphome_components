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
    // MUST be 8 bytes
    // data layout (8 bytes): [STATUS, reserved, FREQ_H, FREQ_L, reserved, RF_POWER, CAPACITOR, RNL]
    stcint = (data[0] & (1u << 2)) ? true : false;
    freq = (static_cast<uint16_t>(data[2]) << 8) | data[3];
    power = data[5];
    tune_capacitor = data[6];
    noise = data[7];
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
  // Interface functions
  // float get_setup_priority() const override;
  void setup() override;
  void dump_config() override;
  void update() override;

  // Pin setters
  void set_reset_pin(GPIOPin *reset_pin) { this->reset_pin_ = reset_pin; }

  // Command functions
  rev_info_t get_info();
  void set_property(uint16_t property, uint16_t value);
  uint16_t get_property(uint16_t property);
  asq_status_t get_asq_status(bool clear_flags = false);
  tune_status_t get_tune_status(bool clear_flags = false);
  // asq_status_t get_asq_status() { return this->get_asq_status(false); };
  // tune_status_t get_tune_status() { return this->get_tune_status(false); };
  void set_freq(uint16_t freq_khz);
  void measure_freq(uint16_t freq_khz);
  void set_power(uint8_t power);
  void setup_rds(uint16_t programID, uint8_t pty = 0);
  void set_rds_ps(const char *s);

  // Print/Log functions
  void print_rev_info(const rev_info_t info);
  void print_status(uint8_t status);
  void print_asq_status(asq_status_t asq);
  void print_tune_status(tune_status_t tunestatus);

 protected:
  void reset_();
  void power_up_();
  void power_down_();
  uint8_t wait_for_cts_();

  GPIOPin *reset_pin_;
};

}  // namespace si4713
}  // namespace esphome
