#include "si4713_switch.h"

namespace esphome {
namespace si4713 {

void Si4713EnableSwitch::write_state(bool state) {
  if (parent_ != nullptr) {
    parent_->set_enabled(state);
    publish_state(state);
  }
}

}  // namespace si4713
}  // namespace esphome
