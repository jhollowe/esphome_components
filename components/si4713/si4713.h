#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/hal.h"
#include "si4713_consts.h"
#include "si4713_structs.h"

namespace esphome {
namespace si4713 {

static const char *const TAG = "si4713";

class Si4713Component;

class Si4713BaseListener {
 public:
  Si4713BaseListener() {}
  Si4713BaseListener(Si4713Component *parent) : parent_(parent) {}

  void set_parent(Si4713Component *parent) { parent_ = parent; }

 protected:
  Si4713Component *parent_;
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
  asq_status_t get_asq_status(bool clear_flags = true);
  tune_status_t get_tune_status(bool clear_flags = true);
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
