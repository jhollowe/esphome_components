#include "esphome/core/log.h"

#include "si4713_number.h"

namespace esphome {
namespace si4713 {

void Si4713FrequencyNumber::control(float value) {
  // Value is in MHz, e.g., 93.3. Convert to kHz for Si4713 (e.g., 9330)
  uint16_t freq_khz = static_cast<uint16_t>(value * 100);
  if (parent_ != nullptr) {
    ESP_LOGI(N_TAG, "Setting frequency to %.2f MHz (%u kHz)", value, freq_khz);
    parent_->set_freq(freq_khz);
  }
  this->publish_state(value);
}

void Si4713FrequencyNumber::setup() {
  ESP_LOGD(N_TAG, "Setting up Si4713 Frequency Number");
  tune_status_t t = parent_->get_tune_status();
  float freq_mhz = t.freq / 100.0;
  if (freq_mhz == 0) {
    ESP_LOGI(N_TAG, "Initial frequency is 0 MHz; defaulting to 93.3 MHz");
    freq_mhz = 93.3;
  } else {
    ESP_LOGI(N_TAG, "Initial frequency is %.2f MHz", freq_mhz);
  }
  this->publish_state(freq_mhz);
}

void Si4713PowerNumber::control(float value) {
  uint8_t power_dbuv = static_cast<uint8_t>(value);
  if (parent_ != nullptr) {
    ESP_LOGI(N_TAG, "Setting power to %u dBµV", power_dbuv);
    parent_->set_power(power_dbuv);
  }

  this->publish_state(power_dbuv);
}

void Si4713PowerNumber::setup() {
  ESP_LOGD(N_TAG, "Setting up Si4713 Power Number");
  tune_status_t t = parent_->get_tune_status();
  float pow = t.power;
  if (pow == 0) {
    ESP_LOGI(N_TAG, "Initial power level is 0 dBµV; defaulting to 100");
    pow = 100;
  } else {
    ESP_LOGI(N_TAG, "Initial power level is %.0f dBµV", pow);
  }
  this->publish_state(pow);
}

// void Si4713FrequencyNumber::dump_config() { LOG_NUMBER(N_TAG, "Si4713 Frequency Number", this); }

}  // namespace si4713
}  // namespace esphome
