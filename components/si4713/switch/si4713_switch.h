#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../si4713.h"

namespace esphome {
namespace si4713 {

const char *const S_TAG = "si4713.switch";

// class Si4713BaseSwitch : public switch_::Switch, public Component, public Parented<Si4713Hub> {};
class Si4713BaseSwitch : public switch_::Switch, public Si4713Listener {};

class Si4713EnableSwitch : public switch_::Switch, public Parented<Si4713Hub> {
 protected:
  // overrides from interface (Switch)
  void write_state(bool state) override;
};

class Si4713ChannelMuteSwitch : public Si4713BaseSwitch {
 public:
  void set_is_left_channel(bool is_left) { this->is_left_channel_ = is_left; }

 protected:
  // overrides from interface (Switch)
  void write_state(bool state) override;
  // overrides from interface (Si4713Listener)
  void on_property(uint16_t reg, uint16_t value) override;

  bool is_left_channel_;
};

class Si4713ComponentSwitch : public Si4713BaseSwitch {
 public:
  void set_bit_pos(uint8_t bit_pos) { this->bit_pos_ = bit_pos; }

 protected:
  // overrides from interface (Switch)
  void write_state(bool state) override;
  // overrides from interface (Si4713Listener)
  void on_property(uint16_t reg, uint16_t value) override;

  // bit 0 is pilot tone, bit 1 is L-R, bit 2 is RDS/RBDS
  uint8_t bit_pos_;
};

}  // namespace si4713
}  // namespace esphome
