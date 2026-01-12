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

//////////////////////////////////////////////////////////////////////
// Interface classes.
class Si4713Hub;  // forward declaration
class Si4713Listener : public Parented<Si4713Hub> {
 public:
  virtual void on_tune_status(const tune_status_t &status){};
  virtual void on_asq_status(const asq_status_t &status){};
  virtual void on_property(const uint16_t reg, const uint16_t value){};
};

/////////////////////////////////////////////////////////////////////
// Main Hub component class
class Si4713Hub : public PollingComponent, public i2c::I2CDevice {
 public:
  // Interface functions (PollingComponent)
  void setup() override;
  void dump_config() override;
  void update() override;

  // Pin setters
  void set_reset_pin(GPIOPin *reset_pin) { this->reset_pin_ = reset_pin; }

  // Common status retrieval functions
  rev_info_t get_info();
  asq_status_t get_asq_status(bool clear_flags = true);
  tune_status_t get_tune_status(bool clear_flags = true);

  // used by number components
  // TODO can these be protected? maybe friend needed?
  void set_freq(uint16_t freq_khz);
  void set_power(uint8_t power);  // respects enabled_ flag

  // used by switch components
  // TODO set the enabled switch as a field and use that to get/set state
  void set_enabled(bool enabled);
  bool get_enabled() const { return this->enabled_; }

  // used by various components
  // also public so lambda functions can access them
  // TODO delete?
  void set_property(uint16_t property, uint16_t value) { this->set_property_(property, value); };
  uint16_t get_property(uint16_t property);

  // unused by components but public for lambda use
  // TODO all the RDS stuff needs finishing/cleanup
  void setup_rds(uint16_t programID, uint8_t pty = 0);
  void set_rds_ps(const char *s);

  // Listener management
  void register_listener(Si4713Listener *listener) { this->listeners_.push_back(listener); }
  prop_table_t *get_properties_next() { return &(this->properties_next_); }

  // Print/Log functions
  void print_status(uint8_t status);
  void print_rev_info(const rev_info_t &info);
  void print_asq_status(const asq_status_t &asq);
  void print_tune_status(const tune_status_t &tunestatus);
  void print_prop_table(const prop_table_t &table);

 protected:
  // Low-level hardware control functions
  void toggle_reset_pin_();
  void power_up_();
  void power_down_();
  uint8_t wait_for_cts_();
  void set_power_direct_(uint8_t power);
  void get_prop_table(prop_table_t &table);
  void set_property_(uint16_t property, uint16_t value);
  // set_changed_properties(prop_table_t &current, prop_table_t &next);

  // TODO remove if unused
  void measure_freq(uint16_t freq_khz);

  // Pin definitions
  GPIOPin *reset_pin_;

  // child components
  // TODO

  // TODO remove and make set_power use the corresponding sensors for state
  bool enabled_;
  uint8_t power_;

  std::vector<Si4713Listener *> listeners_{};

  // STATE MANAGEMENT
  tune_status_t tune_status_last_;
  tune_status_t tune_status_curr_;
  asq_status_t asq_status_last_;
  asq_status_t asq_status_curr_;

  prop_table_t properties_curr_ = {
      {SI4713_PROP_TX_COMPONENT_ENABLE, 0},  {SI4713_PROP_TX_LINE_INPUT_LEVEL, 0},
      {SI4713_PROP_TX_LINE_INPUT_MUTE, 0},   {SI4713_PROP_TX_RDS_PI, 0},
      {SI4713_PROP_TX_RDS_PS_MIX, 0},        {SI4713_PROP_TX_RDS_PS_MISC, 0},
      {SI4713_PROP_TX_RDS_MESSAGE_COUNT, 0},
  };
  prop_table_t properties_next_;
};

}  // namespace si4713
}  // namespace esphome
