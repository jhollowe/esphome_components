# Si4713 Frequency Number entity for ESPHome
import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import DEVICE_CLASS_FREQUENCY, ENTITY_CATEGORY_CONFIG

from .. import CONF_SI4713_ID, DOMAIN, Si4713Component, si4713_ns

DEPENDENCIES = [DOMAIN]

Si4713FrequencyNumber = si4713_ns.class_("Si4713FrequencyNumber", number.Number, cg.Component)

CONF_FREQUENCY = "frequency"
CONF_POWER = "power_level"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SI4713_ID): cv.use_id(Si4713Component),
        cv.Optional(CONF_FREQUENCY): number.number_schema(
            Si4713FrequencyNumber,
            unit_of_measurement="MHz",  # TODO try to use a standard unit
            device_class=DEVICE_CLASS_FREQUENCY,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        # cv.Optional(CONF_POWER): number.number_schema(
        #     Si4713Number,
        #     unit_of_measurement="dBµV",  # TODO try to use a standard unit (UNIT_DECIBEL_MILLIWATT?)
        #     device_class=DEVICE_CLASS_EMPTY,
        #     entity_category=ENTITY_CATEGORY_CONFIG,
        # ),
    }
)


async def to_code(config):
    par = await cg.get_variable(config[CONF_SI4713_ID])
    if freq_cfg := config.get(CONF_FREQUENCY):
        freq_num = await number.new_number(
            freq_cfg,
            # par,
            min_value=freq_cfg.get("min_value", 76.0),
            max_value=freq_cfg.get("max_value", 108.0),
            step=freq_cfg.get("step", 0.05),
        )
        cg.add(freq_num.set_parent(par))
