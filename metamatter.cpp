#include "metamatter.h"

namespace metamatter {
MetaMatter* mm_singleton = NULL;

MetaMatter::MetaMatter() {
	node::config_t node_config;
	this->matter_node =
			node::create(&node_config, (attribute::callback_t)MetaMatter::internal_attribute_callback,
	                 (identification::callback_t)MetaMatter::internal_identifier_callback);
	this->endpoint = endpoint::create(this->matter_node, ENDPOINT_FLAG_NONE, NULL);

	cluster::descriptor::config_t descriptor_config;
	cluster::descriptor::create(this->endpoint, &descriptor_config, CLUSTER_FLAG_SERVER);

	cluster::identify::config_t identity_config;
	cluster_t* identify_cluster =
			cluster::identify::create(this->endpoint, &identity_config, CLUSTER_FLAG_SERVER);
	cluster::identify::command::create_trigger_effect(identify_cluster);
}

void MetaMatter::add_callback(attribute::callback_t callback) {
  if (this->num_attribute_handlers == 255) {
    ESP_LOGE("MetaMatter", "Maximum number of event handlers reached.");
    return;
  }
	this->attribute_handlers[this->num_attribute_handlers] = callback;
	this->num_attribute_handlers += 1;
}

void MetaMatter::add_capability_groups() {
  if (this->capabilities & (1 << groups)) {
    ESP_LOGE("MetaMatter", "Already has groups capability!");
    return;
  }
	cluster::groups::config_t groups_config;
	groups_config.group_name_support = 1;
	cluster::groups::create(this->endpoint, &groups_config, CLUSTER_FLAG_SERVER);
  this->capabilities |= groups;
}

void MetaMatter::add_capability_scenes() {
  if (this->capabilities & (1 << scenes)) {
    ESP_LOGE("MetaMatter", "Already has scenes capability!");
    return;
  }
	cluster::scenes::config_t scenes_config;
	scenes_config.scene_name_support = 1;
	cluster_t* scenes_cluster = cluster::scenes::create(this->endpoint, &scenes_config, CLUSTER_FLAG_SERVER);
	cluster::scenes::command::create_enhanced_add_scene(scenes_cluster);
	cluster::scenes::command::create_enhanced_view_scene(scenes_cluster);
	cluster::scenes::command::create_copy_scene(scenes_cluster);
  this->capabilities |= scenes;
}

void MetaMatter::add_capability_on_off(bool default_on) {
  if (this->capabilities & (1 << on_off)) {
    ESP_LOGE("MetaMatter", "Already has on/off capability!");
    return;
  }
	cluster::on_off::config_t on_off_config;
	on_off_config.on_off = default_on;
	// TODO: check out features
	cluster::on_off::create(this->endpoint, &on_off_config, CLUSTER_FLAG_SERVER, 0);
  this->capabilities |= on_off;
}

void MetaMatter::add_capability_level_control(uint8_t default_level) {
  if (this->capabilities & (1 << level_control)) {
    ESP_LOGE("MetaMatter", "Already has level control capability!");
    return;
  }
	cluster::level_control::config_t level_control_config;
	level_control_config.current_level = default_level;
	level_control_config.on_level = 1;
	// TODO: check out features
	cluster::level_control::create(this->endpoint, &level_control_config, CLUSTER_FLAG_SERVER, 
  cluster::level_control::feature::on_off::get_id() | 
  cluster::level_control::feature::frequency::get_id());
  this->capabilities |= level_control;
}

void MetaMatter::add_capability_switch(uint8_t num_positions, uint8_t default_state) {
  if (this->capabilities & (1 << switch_mechanism)) {
    ESP_LOGE("MetaMatter", "Already has switch capability!");
    return;
  }
	cluster::switch_cluster::config_t switch_config;
	switch_config.current_position = default_state;
	switch_config.number_of_positions = num_positions;
	cluster::switch_cluster::create(this->endpoint, &switch_config, CLUSTER_FLAG_SERVER);
  this->capabilities |= switch_mechanism;
}

void MetaMatter::add_capability_sense_brightness(uint16_t min, uint16_t max) {
  if (this->capabilities & (1 << sense_brightness)) {
    ESP_LOGE("MetaMatter", "Already has sense brightness capability!");
    return;
  }
	cluster::illuminance_measurement::config_t sense_brightness_config;
	sense_brightness_config.illuminance_min_measured_value = min;
	sense_brightness_config.illuminance_max_measured_value = max;
	cluster::illuminance_measurement::create(this->endpoint, &sense_brightness_config, CLUSTER_FLAG_SERVER);
  this->capabilities |= sense_brightness;
}

void MetaMatter::add_capability_sense_temperature(int16_t min, int16_t max) {
  if (this->capabilities & (1 << sense_temperature)) {
    ESP_LOGE("MetaMatter", "Already has sense temperature capability!");
    return;
  }
	cluster::temperature_measurement::config_t sense_temperature_config;
	sense_temperature_config.min_measured_value = min;
	sense_temperature_config.max_measured_value = max;
	cluster::temperature_measurement::create(this->endpoint, &sense_temperature_config, CLUSTER_FLAG_SERVER);
  this->capabilities |= sense_temperature;
}

void MetaMatter::add_capability_sense_pressure(uint16_t min, uint16_t max) {
  if (this->capabilities & (1 << sense_pressure)) {
    ESP_LOGE("MetaMatter", "Already has sense pressure capability!");
    return;
  }
	cluster::pressure_measurement::config_t sense_pressure_config;
	sense_pressure_config.pressure_min_measured_value = min;
	sense_pressure_config.pressure_max_measured_value = max;
	cluster::pressure_measurement::create(this->endpoint, &sense_pressure_config, CLUSTER_FLAG_SERVER);
  this->capabilities |= sense_pressure;
}

void MetaMatter::add_capability_sense_humidity(uint16_t min, uint16_t max) {
  if (this->capabilities & (1 << sense_humidity)) {
    ESP_LOGE("MetaMatter", "Already has sense pressure capability!");
    return;
  }
	cluster::relative_humidity_measurement::config_t sense_humidity_config;
	sense_humidity_config.min_measured_value = min;
	sense_humidity_config.max_measured_value = max;
	cluster::relative_humidity_measurement::create(this->endpoint, &sense_humidity_config, CLUSTER_FLAG_SERVER);
  this->capabilities |= sense_humidity;
}

void MetaMatter::add_capability_sense_occupancy() {}
void MetaMatter::add_capability_sense_alarm() {}

void MetaMatter::add_capability_sense_air_quality() {
  if (this->capabilities & (1 << sense_air_quality)) {
    ESP_LOGE("MetaMatter", "Already has sense pressure capability!");
    return;
  }
	cluster::air_quality::config_t sense_air_quality_config;
	cluster::air_quality::create(this->endpoint, &sense_air_quality_config, CLUSTER_FLAG_SERVER);
  this->capabilities |= sense_air_quality;
}

void MetaMatter::add_capability_light_color(hsv_t default_color) {
	cluster::color_control::config_t light_color_config;
	light_color_config.color_mode =
			(uint8_t)chip::app::Clusters::ColorControl::ColorMode::kCurrentHueAndCurrentSaturation;
	light_color_config.enhanced_color_mode =
			(uint8_t)chip::app::Clusters::ColorControl::ColorMode::kCurrentHueAndCurrentSaturation;
	light_color_config.hue_saturation.current_hue = default_color.hue;
	light_color_config.hue_saturation.current_saturation = default_color.saturation;
	cluster::color_control::create(this->endpoint, &light_color_config, CLUSTER_FLAG_SERVER,
	                               cluster::color_control::feature::color_temperature::get_id() |
	                                   cluster::color_control::feature::hue_saturation::get_id() |
	                                   cluster::color_control::feature::color_loop::get_id());
}

void MetaMatter::add_capability_hvac_thermostat() {}
void MetaMatter::add_capability_hvac_fan() {}

esp_err_t MetaMatter::internal_attribute_callback(attribute::callback_type_t type, uint16_t endpoint_id,
                                                  uint32_t cluster_id, uint32_t attribute_id,
                                                  esp_matter_attr_val_t* val, void* priv_data) {
	esp_err_t ret = ESP_OK;
	for (uint8_t i = 0; i < mm_singleton->num_attribute_handlers; i++) {
		ret = mm_singleton->attribute_handlers[i](type, endpoint_id, cluster_id, attribute_id, val, priv_data);
		if (ret) return ret;
	}
	return ret;
}

esp_err_t MetaMatter::internal_identifier_callback(identification::callback_type_t type, uint16_t endpoint_id,
                                                   uint8_t effect_id, uint8_t effect_variant,
                                                   void* priv_data) {
	if (mm_singleton->identifier_handler) {
		return mm_singleton->identifier_handler(type, endpoint_id, effect_id, effect_variant, priv_data);
	} else {
    ESP_LOGI("MetaMatter", "Default identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
  }
	return ESP_OK;
}

void MetaMatter::set_identify_callback(identification::callback_t identify_callback) {
  if (this->identifier_handler) {
    ESP_LOGE("MetaMatter", "Identify callback handler already registered!");
    return;
  }
  this->identifier_handler = identify_callback;
}
}  // namespace metamatter
