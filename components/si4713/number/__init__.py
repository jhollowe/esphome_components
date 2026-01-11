import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_FREQUENCY,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_CONFIG,
    UNIT_MILLIVOLT,
)

from .. import CONF_SI4713_ID, DOMAIN, Si4713Hub, Si4713Listener, si4713_ns

DEPENDENCIES = [DOMAIN]

common_classes = [number.Number, cg.Parented.template(Si4713Hub)]
Si4713FrequencyNumber = si4713_ns.class_("Si4713FrequencyNumber", *common_classes, cg.Component)
Si4713PowerNumber = si4713_ns.class_("Si4713PowerNumber", *common_classes, cg.Component)
Si4713MaxLineLevelNumber = si4713_ns.class_(
    "Si4713MaxLineLevelNumber", *common_classes, Si4713Listener
)

CONF_FREQUENCY = "frequency"
CONF_POWER = "power_level"
CONF_MAX_LINE_LEVEL = "max_line_level"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SI4713_ID): cv.use_id(Si4713Hub),
        cv.Optional(CONF_FREQUENCY): number.number_schema(
            Si4713FrequencyNumber,
            unit_of_measurement="MHz",  # TODO try to use a standard unit
            device_class=DEVICE_CLASS_FREQUENCY,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_POWER): number.number_schema(
            Si4713PowerNumber,
            unit_of_measurement="dBµV",  # TODO try to use a standard unit (UNIT_DECIBEL_MILLIWATT?)
            device_class=DEVICE_CLASS_EMPTY,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_MAX_LINE_LEVEL): number.number_schema(
            Si4713MaxLineLevelNumber,
            device_class=DEVICE_CLASS_VOLTAGE,
            unit_of_measurement=UNIT_MILLIVOLT,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
    }
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_SI4713_ID])
    if freq_cfg := config.get(CONF_FREQUENCY):
        freq_num = await number.new_number(
            freq_cfg,
            min_value=freq_cfg.get("min_value", 76.0),
            max_value=freq_cfg.get("max_value", 108.0),
            step=freq_cfg.get("step", 0.05),
        )
        await cg.register_component(freq_num, config)
        cg.add(freq_num.set_parent(paren))

    if power_cfg := config.get(CONF_POWER):
        power_num = await number.new_number(
            power_cfg,
            min_value=power_cfg.get("min_value", 88),
            max_value=power_cfg.get("max_value", 115),
            step=power_cfg.get("step", 1.0),
        )
        await cg.register_component(power_num, config)
        cg.add(power_num.set_parent(paren))

    if max_line_cfg := config.get(CONF_MAX_LINE_LEVEL):
        max_line_num = await number.new_number(
            max_line_cfg,
            min_value=max_line_cfg.get("min_value", 0),
            max_value=max_line_cfg.get("max_value", 636),
            step=max_line_cfg.get("step", 1.0),
        )
        # await cg.register_component(max_line_num, config)
        cg.add(max_line_num.set_parent(paren))
        cg.add(paren.register_listener(max_line_num))
