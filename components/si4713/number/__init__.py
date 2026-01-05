# Si4713 Frequency Number entity for ESPHome
import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_STEP, DEVICE_CLASS_FREQUENCY, ENTITY_CATEGORY_CONFIG

from .. import CONF_SI4713_ID, Si4713Component, si4713_ns

Si4713FrequencyNumber = si4713_ns.class_("Si4713FrequencyNumber", number.Number)
Si4713PowerNumber = si4713_ns.class_("Si4713PowerNumber", number.Number)

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
        ).extend(
            {
                cv.Optional(CONF_STEP, default=0.05): cv.float_range(min=0.05, max=1.0),
                # TODO are these needed? the code should enforce this
                # cv.Optional("min_value", default=76.0): cv.float_range(min=76.0, max=108.0),
                # cv.Optional("max_value", default=108.0): cv.float_range(min=76.0, max=108.0),
            }
        ),
        cv.Optional(CONF_POWER): number.number_schema(
            Si4713PowerNumber,
            unit_of_measurement="dBµV",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ).extend(
            {
                cv.Optional(CONF_STEP, default=1): cv.int_range(min=1, max=10),
            }
        ),
    }
)


async def to_code(config):
    par = await cg.get_variable(config[CONF_ID])
    if freq_cfg := config.get(CONF_FREQUENCY):
        freq_num = await number.new_number(
            freq_cfg,
            min_value=freq_cfg.get("min_value", 76.0),
            max_value=freq_cfg.get("max_value", 108.0),
            step=freq_cfg.get("step", 0.05),
        )
        cg.add(freq_num.set_parent(par))
        cg.add_define("USE_SI4713_FREQUENCY_NUMBER")
    if power_cfg := config.get(CONF_POWER):
        power_num = await number.new_number(
            power_cfg,
            min_value=power_cfg.get("min_value", 88.0),
            max_value=power_cfg.get("max_value", 115.0),
            step=power_cfg.get("step", 1),
        )
        cg.add(power_num.set_parent(par))
        cg.add_define("USE_SI4713_POWER_NUMBER")
