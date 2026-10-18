"""Plant care: a tile that says how long ago a plant was watered and fertilized, set by a tap on the screen.

The two dates are ESPHome date entities of plugin.yaml; this component reads and writes them and registers the tile
(plant_care.cpp). register_plugin() takes the id, version, tile memory and the screen's texts from the manifest and
translations/ beside this folder."""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import datetime, smart_display
from esphome.const import CONF_ID

DEPENDENCIES = ["smart_display"]

CONF_WATERED = "watered"
CONF_FERTILIZED = "fertilized"

plant_care_ns = cg.esphome_ns.namespace("plant_care")
PlantCare = plant_care_ns.class_("PlantCare", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(PlantCare),
        cv.Required(CONF_WATERED): cv.use_id(datetime.DateEntity),
        cv.Required(CONF_FERTILIZED): cv.use_id(datetime.DateEntity),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await smart_display.register_plugin(var, __file__)
    cg.add(var.set_watered(await cg.get_variable(config[CONF_WATERED])))
    cg.add(var.set_fertilized(await cg.get_variable(config[CONF_FERTILIZED])))
