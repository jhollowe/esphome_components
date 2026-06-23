#include "si4713_binary_sensor.h"

namespace esphome {
namespace si4713 {

void Si4713ASQBinarySensor::on_asq_status(const asq_status_t &status) {
  switch (type_) {
    case Si4713AsqBSType::OVERMOD:
      this->publish_state(status.overmod);
      break;
    case Si4713AsqBSType::AUDIO_LOW:
      this->publish_state(status.in_audio_detect_low);
      break;
    case Si4713AsqBSType::AUDIO_HIGH:
      this->publish_state(status.in_audio_detect_high);
      break;
    default:
      ESP_LOGW(TAG, "Unknown Si4713 binary sensor type");
      break;
  }
}

const LogString *type_to_str(Si4713AsqBSType type) {
  switch (type) {
    case Si4713AsqBSType::OVERMOD:
      return LOG_STR("overmodulation");
    case Si4713AsqBSType::AUDIO_LOW:
      return LOG_STR("audio_low");
    case Si4713AsqBSType::AUDIO_HIGH:
      return LOG_STR("audio_high");
  }
  return LOG_STR("unknown");
}

}  // namespace si4713
}  // namespace esphome
