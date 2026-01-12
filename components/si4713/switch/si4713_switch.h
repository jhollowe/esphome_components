#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../si4713.h"

namespace esphome {
namespace si4713 {

const char *const S_TAG = "si4713.switch";

// class Si4713BaseSwitch : public switch_::Switch, public Component, public Parented<Si4713Hub> {};
class Si4713BaseSwitch : public switch_::Switch, public Parented<Si4713Hub> {};

class Si4713EnableSwitch : public Si4713BaseSwitch {
 protected:
  // overrides from interface (Switch)
  void write_state(bool state) override;
};

class Si4713ChannelMuteSwitch : public switch_::Switch, public Si4713Listener {
 public:
  void set_is_left_channel(bool is_left) { this->is_left_channel_ = is_left; }

 protected:
  // overrides from interface (Switch)
  void write_state(bool state) override;
  // overrides from interface (Si4713Listener)
  void on_property(uint16_t reg, uint16_t value) override;

  bool is_left_channel_;
};

}  // namespace si4713
}  // namespace esphome
