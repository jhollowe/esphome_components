from esphome import pins
import esphome.codegen as cg
from esphome.components import i2c
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_RESET_PIN

DEPENDENCIES = ["i2c"]

si4713_ns = cg.esphome_ns.namespace("si4713")

Si4713Component = si4713_ns.class_("Si4713Component", cg.Component, i2c.I2CDevice)

CONFIG_SCHEMA = cv.COMPONENT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(Si4713Component),
        cv.Required(CONF_RESET_PIN): pins.gpio_output_pin_schema,
    }
).extend(i2c.i2c_device_schema(0x63))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if reset_pin_config := config.get(CONF_RESET_PIN):
        pin = await cg.gpio_pin_expression(reset_pin_config)
        cg.add(var.set_reset_pin(pin))

    # if CONF_ON_FAN in config:
    #     await automation.build_automation(
    #         var.get_fan_trigger(), [(int, "speed")], config[CONF_ON_FAN]
    #     )
    # if CONF_ON_LIGHT in config:
    #     await automation.build_automation(var.get_light_trigger(), [], config[CONF_ON_LIGHT])
    # if CONF_ON_BUZZER in config:
    #     await automation.build_automation(var.get_buzzer_trigger(), [], config[CONF_ON_BUZZER])
