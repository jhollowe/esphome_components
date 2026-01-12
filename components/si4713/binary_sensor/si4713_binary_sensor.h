#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/si4713/si4713.h"

namespace esphome {
namespace si4713 {

enum class Si4713AsqBSType { OVERMOD, AUDIO_LOW, AUDIO_HIGH };

class Si4713ASQBinarySensor : public binary_sensor::BinarySensor, public Si4713Listener {
 public:
  void set_type(Si4713AsqBSType type) { type_ = type; }
  Si4713AsqBSType get_type() const { return type_; }
  const LogString *type_to_str(Si4713AsqBSType type);

  // Interface function (Si4713Listener)
  void on_asq_status(const asq_status_t &status) override;

 private:
  Si4713AsqBSType type_;
};

}  // namespace si4713
}  // namespace esphome
