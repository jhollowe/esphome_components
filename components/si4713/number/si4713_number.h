#pragma once

#include "esphome/core/component.h"
#include "esphome/components/number/number.h"
#include "esphome/components/si4713/si4713.h"

namespace esphome {
namespace si4713 {

const char *const N_TAG = "si4713.number";

class Si4713BaseNumber : public number::Number, public Component, public Parented<Si4713Hub> {};

class Si4713FrequencyNumber : public Si4713BaseNumber {
  // overrides from interface (Component)
  void setup() override;
  void dump_config() { LOG_NUMBER("", "Si4713 Frequency Number", this); }
  // overrides from interface (Number)
  void control(float value) override;
};

class Si4713PowerNumber : public Si4713BaseNumber {
  // overrides from interface (Component)
  void setup() override;
  void dump_config() { LOG_NUMBER("", "Si4713 Power Number", this); }
  // overrides from interface (Number)
  void control(float value) override;
};

}  // namespace si4713
}  // namespace esphome
