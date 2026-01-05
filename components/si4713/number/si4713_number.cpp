#include "si4713_number.h"
#include "esphome/core/log.h"

namespace esphome {
namespace si4713 {

void Si4713FrequencyNumber::control(float value) {
  // Value is in MHz, e.g., 93.3. Convert to kHz for Si4713 (e.g., 9330)
  uint16_t freq_khz = static_cast<uint16_t>(value * 100);
  ESP_LOGI("si4713.number", "Setting frequency to %.2f MHz (%u kHz)", value, freq_khz);
  if (parent_ != nullptr) {
    parent_->set_freq(freq_khz);
  }
  this->publish_state(value);
}

void Si4713FrequencyNumber::dump_config() { ESP_LOGCONFIG("si4713.number", "Si4713 Frequency Number"); }

}  // namespace si4713
}  // namespace esphome
