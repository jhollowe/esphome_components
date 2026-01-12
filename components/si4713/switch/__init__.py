import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import DEVICE_CLASS_SWITCH

from .. import CONF_SI4713_ID, DOMAIN, Si4713Hub, Si4713Listener, si4713_ns

DEPENDENCIES = [DOMAIN]

common_classes = [switch.Switch, cg.Parented.template(Si4713Hub)]
Si4713EnableSwitch = si4713_ns.class_("Si4713EnableSwitch", *common_classes)
Si4713ChannelMuteSwitch = si4713_ns.class_(
    "Si4713ChannelMuteSwitch", *common_classes, Si4713Listener
)
# Si4713StereoSwitch = si4713_ns.class_("Si4713StereoSwitch", *common_classes, cg.Component)
# Si4713RDSSwitch = si4713_ns.class_("Si4713RDSSwitch", *common_classes, cg.Component)

CONF_ENABLE_TX = "enable_tx"
CONF_ENABLE_STEREO = "enable_stereo"
CONF_ENABLE_RDS = "enable_rds"
CONF_MUTE_LEFT = "mute_left"
CONF_MUTE_RIGHT = "mute_right"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SI4713_ID): cv.use_id(Si4713Hub),
        cv.Optional(CONF_ENABLE_TX): switch.switch_schema(
            Si4713EnableSwitch, device_class=DEVICE_CLASS_SWITCH
        ),
        cv.Optional(CONF_MUTE_LEFT): switch.switch_schema(
            Si4713ChannelMuteSwitch, device_class=DEVICE_CLASS_SWITCH
        ),
        cv.Optional(CONF_MUTE_RIGHT): switch.switch_schema(
            Si4713ChannelMuteSwitch, device_class=DEVICE_CLASS_SWITCH
        ),
        # cv.Optional(CONF_ENABLE_STEREO): switch.switch_schema(
        #     Si4713StereoSwitch, device_class=DEVICE_CLASS_SWITCH
        # ),
        # cv.Optional(CONF_ENABLE_RDS): switch.switch_schema(
        #     Si4713RDSSwitch, device_class=DEVICE_CLASS_SWITCH
        # ),
    }
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_SI4713_ID])
    if enable_tx_cfg := config.get(CONF_ENABLE_TX):
        enable_tx_switch = await switch.new_switch(enable_tx_cfg)
        cg.add(enable_tx_switch.set_parent(paren))
        cg.add(paren.set_enabled_switch(enable_tx_switch))

    if mute_left_cfg := config.get(CONF_MUTE_LEFT):
        mute_left_switch = await switch.new_switch(mute_left_cfg)
        cg.add(mute_left_switch.set_parent(paren))
        cg.add(mute_left_switch.set_is_left_channel(True))
        cg.add(paren.register_listener(mute_left_switch))
        # cg.add(paren.set_mute_left_switch(mute_left_switch))

    if mute_right_cfg := config.get(CONF_MUTE_RIGHT):
        mute_right_switch = await switch.new_switch(mute_right_cfg)
        cg.add(mute_right_switch.set_parent(paren))
        cg.add(mute_right_switch.set_is_left_channel(False))
        cg.add(paren.register_listener(mute_right_switch))
        # cg.add(paren.set_mute_right_switch(mute_right_switch))
