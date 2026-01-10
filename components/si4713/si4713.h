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

// class Si4713Hub : public Component, public i2c::I2CDevice {
class Si4713Hub : public PollingComponent, public i2c::I2CDevice {
 public:
  // Interface functions
  // float get_setup_priority() const override;
  void setup() override;
  void dump_config() override;
  void update() override;

  // Pin setters
  void set_reset_pin(GPIOPin *reset_pin) { this->reset_pin_ = reset_pin; }

  // Common status retrival functions
  rev_info_t get_info();
  asq_status_t get_asq_status(bool clear_flags = true);
  tune_status_t get_tune_status(bool clear_flags = true);

  // used by number components
  void set_freq(uint16_t freq_khz);
  void set_power(uint8_t power);  // respects enabled_ flag

  // used by switch components
  void set_enabled(bool enabled);
  bool get_enabled() const { return this->enabled_; }

  // used by various components
  // also public so lambda functions can access them
  void set_property(uint16_t property, uint16_t value);
  uint16_t get_property(uint16_t property);

  // unused by components but public for lambda use
  void setup_rds(uint16_t programID, uint8_t pty = 0);
  void set_rds_ps(const char *s);

  // Print/Log functions
  void print_rev_info(const rev_info_t info);
  void print_status(uint8_t status);
  void print_asq_status(asq_status_t asq);
  void print_tune_status(tune_status_t tunestatus);
  void print_prop_table(prop_table_t);

 protected:
  void toggle_reset_pin_();
  void power_up_();
  void power_down_();
  uint8_t wait_for_cts_();
  void set_power_direct_(uint8_t power);
  void get_prop_table(prop_table_t &table);
  // set_changed_properties(prop_table_t &current, prop_table_t &next);

  // TODO remove if unused
  void measure_freq(uint16_t freq_khz);

  GPIOPin *reset_pin_;

  bool enabled_;
  uint8_t power_;

  // STATE MANAGEMENT
  tune_status_t tune_status_curr_;
  tune_status_t tune_status_next_;
  asq_status_t asq_status_curr_;
  asq_status_t asq_status_next_;

  prop_table_t poperties_curr = {
      {SI4713_PROP_TX_COMPONENT_ENABLE, 0},  {SI4713_PROP_TX_LINE_INPUT_LEVEL, 0},
      {SI4713_PROP_TX_LINE_INPUT_MUTE, 0},   {SI4713_PROP_TX_RDS_PI, 0},
      {SI4713_PROP_TX_RDS_PS_MIX, 0},        {SI4713_PROP_TX_RDS_PS_MISC, 0},
      {SI4713_PROP_TX_RDS_MESSAGE_COUNT, 0},
  };
  prop_table_t poperties_next;
};

}  // namespace si4713
}  // namespace esphome
