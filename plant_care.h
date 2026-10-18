#pragma once
#include "esphome/core/component.h"
#include "esphome/components/datetime/date_entity.h"
#include "esphome/components/smart_display/plugin_api.h"

namespace esphome::plant_care {

// The plugin: an ESPHome component and a tessera::Plugin. It registers the plant tile in setup() and keeps the two
// dates the tile shows: when the plant was last watered and last fertilized. The dates are ESPHome entities of
// plugin.yaml (restored after a restart, and a date picker in Home Assistant too).
class PlantCare : public Component, public tessera::Plugin {
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_watered(datetime::DateEntity *d) { watered_ = d; }
  void set_fertilized(datetime::DateEntity *d) { fertilized_ = d; }
  datetime::DateEntity *watered() const { return watered_; }
  datetime::DateEntity *fertilized() const { return fertilized_; }

 protected:
  datetime::DateEntity *watered_{}, *fertilized_{};
};

}  // namespace esphome::plant_care
