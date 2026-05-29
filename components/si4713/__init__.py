from esphome import pins
import esphome.codegen as cg
from esphome.components import i2c
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_RESET_PIN

DEPENDENCIES = ["i2c"]

# TODO see if this can support multiple devices (up to 2 since there are only 2 I2C addresses)
# MULTI_CONF = True

# Datasheet: https://cdn-shop.adafruit.com/datasheets/Si4712-13-B30.pdf
# Control Guide: https://cdn-shop.adafruit.com/datasheets/SiLabs%20Programming%20guide%20AN332.pdf

DOMAIN = "si4713"
CONF_SI4713_ID = f"{DOMAIN}_id"

si4713_ns = cg.esphome_ns.namespace(DOMAIN)
Si4713Hub = si4713_ns.class_("Si4713Hub", cg.PollingComponent, i2c.I2CDevice)
Si4713Listener = si4713_ns.class_("Si4713Listener", cg.Parented.template(Si4713Hub))

DEFAULT_POLLING_INTERVAL = "5s"


# New options for initial frequency and power
CONF_INITIAL_FREQUENCY = "initial_frequency"
CONF_INITIAL_POWER = "initial_power"
CONF_PTY = "pty"
CONF_PROGRAM_ID = "program_id"
CONF_INITIAL_PS = "initial_ps"

# Range constants number entities/values
FREQ_BOUNDS = (76.0, 108.0)  # MHz
POWER_BOUNDS = (88, 115)
THRESHOLD_BOUNDS = (-70, 0)

CONFIG_SCHEMA = (
    cv.COMPONENT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_ID): cv.declare_id(Si4713Hub),
            cv.Required(CONF_RESET_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_INITIAL_FREQUENCY, default=93.3): cv.float_range(
                min=FREQ_BOUNDS[0], max=FREQ_BOUNDS[1]
            ),
            cv.Optional(CONF_INITIAL_POWER, default=100): cv.int_range(
                min=POWER_BOUNDS[0], max=POWER_BOUNDS[1]
            ),
            cv.Optional(CONF_PTY, default=0): cv.int_range(min=0, max=31),
            cv.Optional(CONF_PROGRAM_ID, default=0): cv.int_range(min=0, max=0xFFFF),
            cv.Optional(CONF_INITIAL_PS, default="ESPHome"): cv.All(
                cv.string, cv.ByteLength(max=88)
            ),
        }
    )
    .extend(i2c.i2c_device_schema(0x63))
    .extend(cv.polling_component_schema(DEFAULT_POLLING_INTERVAL))
)

# Actions
# Si4713SetRDSAction = si4713_ns.class_("Si4713SetRDSAction", automation.Action)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if reset_pin_config := config.get(CONF_RESET_PIN):
        pin = await cg.gpio_pin_expression(reset_pin_config)
        cg.add(var.set_reset_pin(pin))

    # Set initial frequency and power if provided
    if CONF_INITIAL_FREQUENCY in config:
        # Convert MHz float to kHz int (e.g., 12.3 -> 1230)
        freq_khz = int(config[CONF_INITIAL_FREQUENCY] * 100)
        cg.add(var.set_initial_frequency(freq_khz))
    if CONF_INITIAL_POWER in config:
        cg.add(var.set_initial_power(config[CONF_INITIAL_POWER]))
    if CONF_PTY in config:
        cg.add(var.set_pty_stored(config[CONF_PTY]))
    if CONF_PROGRAM_ID in config:
        cg.add(var.set_program_id_stored(config[CONF_PROGRAM_ID]))
    if CONF_INITIAL_PS in config:
        cg.add(var.set_initial_ps(config[CONF_INITIAL_PS]))
