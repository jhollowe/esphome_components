#pragma once

#include "esphome/core/defines.h"

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/hal.h"
#include "si4713_consts.h"
#include "si4713_structs.h"

#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

namespace esphome {
namespace si4713 {

static const char *const TAG = "si4713";

///////////////////////////////////////////////////////////////////////
// Interface classes.
class Si4713Hub;  // forward declaration
class Si4713Listener : public Parented<Si4713Hub> {
 public:
  virtual void on_tune_status(const tune_status_t &status){};
  virtual void on_asq_status(const asq_status_t &status){};
  virtual void on_property(const uint16_t reg, const uint16_t value){};
};

//////////////////////////////////////////////////////////////////////
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

  // Used by Si4713FrequencyNumber and Si4713PowerNumber
  void set_freq(uint16_t freq_khz);
  void set_power(uint8_t power);  // respects enabled_ flag

  // Setters for initial values
  void set_initial_frequency(uint16_t freq_khz) { this->frequency_ = freq_khz; }
  void set_initial_power(uint8_t power) { this->power_ = power; }

  // used by switch components
  void set_enabled(bool enabled);

  // public so lambda functions can access them
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

  //////////////////////////////////////////////////////////////////////
  // child entities setters
#ifdef USE_BUTTON
  void set_reset_button(button::Button *button) { this->reset_button_ = button; };
#endif  // USE_BUTTON
#ifdef USE_SWITCH
  void set_enabled_switch(switch_::Switch *sw) { this->enabled_switch_ = sw; };
  void set_channel_mute_left_switch(switch_::Switch *sw) { this->channel_mute_left_switch_ = sw; };
  void set_channel_mute_right_switch(switch_::Switch *sw) { this->channel_mute_right_switch_ = sw; };
  void set_enable_pilot_switch(switch_::Switch *sw) { this->enable_pilot_switch_ = sw; };
  void set_enable_stereo_switch(switch_::Switch *sw) { this->enable_stereo_switch_ = sw; };
  void set_enable_rds_switch(switch_::Switch *sw) { this->enable_rds_switch_ = sw; };
#endif  // USE_SWITCH
#ifdef USE_NUMBER
  void set_frequency_number(number::Number *num) { this->frequency_number_ = num; };
  void set_power_number(number::Number *num) { this->power_number_ = num; };
  void set_max_line_level_number(number::Number *num) { this->max_line_level_number_ = num; };
  void set_low_threshold_number(number::Number *num) { this->low_threshold_number_ = num; };
  void set_high_threshold_number(number::Number *num) { this->high_threshold_number_ = num; };
#endif  // USE_NUMBER
#ifdef USE_BINARY_SENSOR
  void set_audio_high_bsensor(binary_sensor::BinarySensor *sens) { this->audio_high_bsensor_ = sens; };
  void set_audio_low_bsensor(binary_sensor::BinarySensor *sens) { this->audio_low_bsensor_ = sens; };
  void set_overmod_bsensor(binary_sensor::BinarySensor *sens) { this->overmod_bsensor_ = sens; };
#endif  // USE_BINARY_SENSOR
#ifdef USE_SENSOR
  void set_input_line_level_sensor(sensor::Sensor *sens) { this->input_line_level_sensor_ = sens; };
  void set_tune_capacitor_sensor(sensor::Sensor *sens) { this->tune_capacitor_sensor_ = sens; };
#endif  // USE_SENSOR
 protected:
  // Low-level hardware control functions
  void toggle_reset_pin_();
  void power_up_();
  uint8_t wait_for_cts_();
  void set_power_direct_(uint8_t power);
  void set_property_(uint16_t property, uint16_t value);
  void get_prop_table_(prop_table_t &table);

  std::vector<Si4713Listener *> listeners_{};

  // TODO remove if unused
  void measure_freq(uint16_t freq_khz);
  void power_down_();

  // Pin definitions
  GPIOPin *reset_pin_;

  //////////////////////////////////////////////////////////////////////
  // STATE MANAGEMENT
  bool enabled_ = true;
  uint8_t power_;
  uint16_t frequency_;
  bool has_been_setup_ = false;

  tune_status_t tune_status_last_;
  tune_status_t tune_status_curr_;
  asq_status_t asq_status_last_;
  asq_status_t asq_status_curr_;

  prop_table_t properties_curr_ = {
#ifdef USE_SWITCH
      {SI4713_PROP_TX_LINE_INPUT_MUTE, 0},   // Si4713ChannelMuteSwitch
      {SI4713_PROP_TX_COMPONENT_ENABLE, 0},  // Si4713ComponentSwitch

#endif  // USE_SWITCH
#ifdef USE_NUMBER
      {SI4713_PROP_TX_LINE_INPUT_LEVEL, 0},  // Si4713MaxLineLevelNumber
      {SI4713_PROP_TX_ASQ_LEVEL_LOW, 0},     // Si4713LevelThresholdNumber
      {SI4713_PROP_TX_ASQ_LEVEL_HIGH, 0},    // Si4713LevelThresholdNumber

#endif  // USE_NUMBER
  };
  prop_table_t properties_next_;

  //////////////////////////////////////////////////////////////////////
  // child entities

#ifdef USE_BUTTON
  button::Button *reset_button_{nullptr};
#endif  // USE_BUTTON
#ifdef USE_SWITCH
  switch_::Switch *enabled_switch_{nullptr};
  switch_::Switch *channel_mute_left_switch_{nullptr};
  switch_::Switch *channel_mute_right_switch_{nullptr};
  switch_::Switch *enable_pilot_switch_{nullptr};
  switch_::Switch *enable_stereo_switch_{nullptr};
  switch_::Switch *enable_rds_switch_{nullptr};
#endif  // USE_SWITCH
#ifdef USE_NUMBER
  number::Number *frequency_number_{nullptr};
  number::Number *power_number_{nullptr};
  number::Number *max_line_level_number_{nullptr};
  number::Number *low_threshold_number_{nullptr};
  number::Number *high_threshold_number_{nullptr};
#endif  // USE_NUMBER
#ifdef USE_BINARY_SENSOR
  binary_sensor::BinarySensor *audio_high_bsensor_{nullptr};
  binary_sensor::BinarySensor *audio_low_bsensor_{nullptr};
  binary_sensor::BinarySensor *overmod_bsensor_{nullptr};
#endif  // USE_BINARY_SENSOR
#ifdef USE_SENSOR
  sensor::Sensor *input_line_level_sensor_{nullptr};
  sensor::Sensor *tune_capacitor_sensor_{nullptr};
#endif  // USE_SENSOR
};

}  // namespace si4713
}  // namespace esphome
