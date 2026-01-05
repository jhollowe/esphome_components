#pragma once

#include "esphome/components/number/number.h"
#include "esphome/components/si4713/si4713.h"

namespace esphome {
namespace si4713 {

class Si4713FrequencyNumber : public number::Number {
 public:
  Si4713FrequencyNumber(Si4713Component *parent) : parent_(parent) {}
  void control(float value) override;
  void dump_config() override;

 protected:
  Si4713Component *parent_;
};

class Si4713PowerNumber : public number::Number {
 public:
  Si4713PowerNumber(Si4713Component *parent) : parent_(parent) {}
  void control(float value) override;
  void dump_config() override;

 protected:
  Si4713Component *parent_;
};

}  // namespace si4713
}  // namespace esphome
