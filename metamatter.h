#pragma once

#include "color/color.h"

#include <esp_err.h>
#include <esp_matter.h>

using namespace esp_matter;

namespace metamatter {
  enum capability {
    groups = 0,
    scenes = 1,
    on_off = 2,
    level_control = 3,
    switch_mechanism = 4,
    sense_brightness = 5,
    sense_temperature = 6,
    sense_pressure = 7,
    sense_humidity = 8,
    sense_occupancy = 9,
    sense_alarm = 10,
    sense_air_quality = 11,
    light_color = 12,
    hvac_thermostat = 13,
    hvac_fan = 14,
  };

  class MetaMatter {
    public:
      MetaMatter();
      void add_callback(attribute::callback_t attribute_callback);
      void set_identify_callback(identification::callback_t identify_callback);
      
      void add_capability_groups();
      void add_capability_scenes();
      void add_capability_on_off(bool default_on);
      void add_capability_level_control(uint8_t default_level);
      void add_capability_switch(uint8_t num_positions, uint8_t default_state);
      void add_capability_sense_brightness(uint16_t min, uint16_t max);
      void add_capability_sense_temperature(int16_t min, int16_t max);
      void add_capability_sense_pressure(uint16_t min, uint16_t max);
      void add_capability_sense_humidity(uint16_t min, uint16_t max);
      void add_capability_sense_occupancy();
      void add_capability_sense_alarm();
      void add_capability_sense_air_quality();
      void add_capability_light_color(hsv_t default_color);
      void add_capability_hvac_thermostat();
      void add_capability_hvac_fan();
    private:
      uint64_t capabilities = 0;
      node_t* matter_node;
      endpoint_t* endpoint;

      uint8_t num_attribute_handlers = 0;
      attribute::callback_t attribute_handlers[256];

      identification::callback_t identifier_handler = NULL;

      static esp_err_t internal_attribute_callback(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data);
      static esp_err_t internal_identifier_callback(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id, uint8_t effect_variant, void *priv_data);
  };
  
  extern MetaMatter* mm_singleton;
}