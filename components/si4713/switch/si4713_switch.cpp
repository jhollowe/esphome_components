#include "si4713_switch.h"

namespace esphome {
namespace si4713 {

void Si4713EnableSwitch::write_state(bool state) {
  if (parent_ != nullptr) {
    parent_->set_enabled(state);
    publish_state(state);
  }
}

void Si4713ChannelMuteSwitch::write_state(bool state) {
  if (parent_ != nullptr) {
    // Left channel is bit 1, Right channel is bit 0
    // 0 is unmuted, 1 is muted

    // clear the changing bit
    (*(parent_->get_properties_next()))[SI4713_PROP_TX_LINE_INPUT_MUTE] &= ~(1 << (is_left_channel_ ? 1 : 0));
    // set the changing bit
    (*(parent_->get_properties_next()))[SI4713_PROP_TX_LINE_INPUT_MUTE] |=
        (static_cast<uint16_t>(state) << (is_left_channel_ ? 1 : 0));
  }
}

void Si4713ChannelMuteSwitch::on_property(uint16_t reg, uint16_t value) {
  if (reg == SI4713_PROP_TX_LINE_INPUT_MUTE) {
    bool is_muted = (value >> (is_left_channel_ ? 1 : 0)) & 0x1;
    publish_state(is_muted);
  }
}

}  // namespace si4713
}  // namespace esphome
