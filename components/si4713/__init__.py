from esphome import pins
import esphome.codegen as cg
from esphome.components import i2c
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_RESET_PIN

DEPENDENCIES = ["i2c"]

DOMAIN = "si4713"
CONF_SI4713_ID = f"{DOMAIN}_id"

si4713_ns = cg.esphome_ns.namespace(DOMAIN)
Si4713Component = si4713_ns.class_("Si4713Component", cg.Component, i2c.I2CDevice)

DEFAULT_POLLING_INTERVAL = "60s"

CONFIG_SCHEMA = (
    cv.COMPONENT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_ID): cv.declare_id(Si4713Component),
            cv.Required(CONF_RESET_PIN): pins.gpio_output_pin_schema,
        }
    )
    .extend(i2c.i2c_device_schema(0x63))
    .extend(cv.polling_component_schema(DEFAULT_POLLING_INTERVAL))
)

# Datasheet: https://cdn-shop.adafruit.com/datasheets/Si4712-13-B30.pdf
# Control Guide: https://cdn-shop.adafruit.com/datasheets/SiLabs%20Programming%20guide%20AN332.pdf

# Actions
# Si4713SetFrequencyAction = si4713_ns.class_("Si4713SetFrequencyAction", automation.Action)
# Si4713SetRDSAction = si4713_ns.class_("Si4713SetRDSAction", automation.Action)
# Si4713GetPropertyAction = si4713_ns.class_("Si4713GetPropertyAction", automation.Action)
# Si4713SetPropertyAction = si4713_ns.class_("Si4713SetPropertyAction", automation.Action)
# Si4713GetASQAction = si4713_ns.class_("Si4713GetASQAction", automation.Action)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if reset_pin_config := config.get(CONF_RESET_PIN):
        pin = await cg.gpio_pin_expression(reset_pin_config)
        cg.add(var.set_reset_pin(pin))
