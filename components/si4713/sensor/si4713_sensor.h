#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/si4713/si4713.h"

namespace esphome {
namespace si4713 {

class Si4713BaseSensor : public sensor::Sensor, public Si4713Listener {};

class Si4713LineLevelSensor : public Si4713BaseSensor {
 public:
  // overrides from Si4713Listener
  void on_asq_status(asq_status_t status) override { this->publish_state(static_cast<float>(status.in_audio_level)); }
};

}  // namespace si4713
}  // namespace esphome
