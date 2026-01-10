import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG, ICON_RESTART

from .. import CONF_SI4713_ID, DOMAIN, Si4713Hub, si4713_ns

DEPENDENCIES = [DOMAIN]

common_classes = [button.Button, cg.Parented.template(Si4713Hub)]
Si4713ResetButton = si4713_ns.class_("Si4713ResetButton", *common_classes)

CONST_RESET = "reset"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SI4713_ID): cv.use_id(Si4713Hub),
        cv.Optional(CONST_RESET): button.button_schema(
            Si4713ResetButton,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_RESTART,
        ),
    }
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_SI4713_ID])
    if reset_cfg := config.get(CONST_RESET):
        reset_button = await button.new_button(reset_cfg)
        cg.add(reset_button.set_parent(paren))
