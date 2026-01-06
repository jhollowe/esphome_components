#pragma once

#include "esphome/core/component.h"
#include "esphome/components/number/number.h"
#include "esphome/components/si4713/si4713.h"

namespace esphome {
namespace si4713 {

class Si4713FrequencyNumber : public number::Number, public Si4713BaseListener, public Component {
 public:
  Si4713FrequencyNumber() {}
  Si4713FrequencyNumber(Si4713Component *parent) : parent_(parent) {}

  void setup() override { this->publish_state(93.3); }
  void control(float value) override;
  void dump_config() override;

 protected:
  Si4713Component *parent_;
};

}  // namespace si4713
}  // namespace esphome
