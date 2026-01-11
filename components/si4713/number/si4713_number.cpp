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

void Si4713MaxLineLevelNumber::control(float value) {
  if (parent_ != nullptr) {
    uint16_t level = (static_cast<uint16_t>(value)) & 0x3FF;  // only fits into bits 0-9
    ESP_LOGI(N_TAG, "Setting Line Input Level to %u mVPK", level);

    // Line Attenuation (bits 12-13):
    // 00 = Max input level = 190 mVPK; input resistance = 396 kOhm
    // 01 = Max input level = 301 mVPK; input resistance = 100 kOhm
    // 10 = Max input level = 416 mVPK; input resistance = 74 kOhm
    // 11 = Max input level = 636 mVPK; input resistance = 60 kOhm(default)
    uint8_t attenuation = 0;
    if (level <= 301) {
      attenuation = 0b01;
    } else if (level <= 416) {
      attenuation = 0b10;
    } else if (level <= 636) {
      attenuation = 0b11;
    }
    // use get_properties_next() from parent_ to get the next property table
    prop_table_t *props_next = parent_->get_properties_next();
    // set the new value in the next properties table
    (*props_next)[SI4713_PROP_TX_LINE_INPUT_LEVEL] = (attenuation << 12) | level;
  }
}

void Si4713MaxLineLevelNumber::on_property(uint16_t property, uint16_t value) {
  if (property == SI4713_PROP_TX_LINE_INPUT_LEVEL) {
    uint16_t level = value & 0x3FF;  // bits 0-9 are the level
    ESP_LOGI(N_TAG, "Line Input Level property changed to %u mVPK", level);
    this->publish_state(level);
  }
}

}  // namespace si4713
}  // namespace esphome
