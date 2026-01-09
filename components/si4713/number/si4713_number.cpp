#include "esphome/core/log.h"

#include "si4713_number.h"

namespace esphome {
namespace si4713 {

const char *const N_TAG = "si4713.number";

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
  ESP_LOGI(N_TAG, "Initial frequency is %.2f MHz", freq_mhz);
  this->publish_state(freq_mhz);
}

void Si4713FrequencyNumber::dump_config() { LOG_NUMBER(N_TAG, "Si4713 Frequency Number", this); }

}  // namespace si4713
}  // namespace esphome
