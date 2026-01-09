#pragma once

#include "esphome/core/component.h"
#include "esphome/components/number/number.h"
#include "esphome/components/si4713/si4713.h"

namespace esphome {
namespace si4713 {

class Si4713BaseNumber : public number::Number, public Component {
 public:
  Si4713BaseNumber() {}
  Si4713BaseNumber(Si4713Component *parent) : parent_(parent) {}

  void set_parent(Si4713Component *parent) { parent_ = parent; }

 protected:
  Si4713Component *parent_;
};

class Si4713FrequencyNumber : public Si4713BaseNumber {
 public:
  // overrides from interface (Component)
  void setup() override;
  void dump_config() override;
  // overrides from interface (Number)
  void control(float value) override;
};

}  // namespace si4713
}  // namespace esphome
