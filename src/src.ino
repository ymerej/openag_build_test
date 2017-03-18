#include <openag_brain/DiagnosticArray.h>
#include <openag_brain/DiagnosticStatus.h>
#include <ros.h>
#include <openag_am2315.h>

uint32_t last_status_read = 0;
bool should_read_statuses() {
  uint32_t curr_time = millis();
  bool res = (curr_time - last_status_read) > 5000;
  if (res) {
    last_status_read = curr_time;
  }
  return res;
}
Am2315 am2315_1;
std_msgs::Float32 am2315_1_air_humidity_msg;
std_msgs::Float32 am2315_1_air_temperature_msg;
ros::NodeHandle nh;
openag_brain::DiagnosticArray status_array;
ros::Publisher pub_diagnostics("/internal_diagnostics", &status_array);
ros::Publisher pub_am2315_1_air_humidity("/sensors/am2315_1/air_humidity/raw", &am2315_1_air_humidity_msg);
ros::Publisher pub_am2315_1_air_temperature("/sensors/am2315_1/air_temperature/raw", &am2315_1_air_temperature_msg);

void setup() {
  // Setup all plugins
  Serial.begin(57600);
  nh.initNode();
  nh.advertise(pub_diagnostics);
  // Setup all modules
  am2315_1.begin();
  nh.advertise(pub_am2315_1_air_humidity);
  nh.advertise(pub_am2315_1_air_temperature);
}

void loop() {
  // Update all plugins
  nh.spinOnce();
  // Update all modules
  am2315_1.update();
  nh.spinOnce();
  if (am2315_1.get_air_humidity(am2315_1_air_humidity_msg)) {
    pub_am2315_1_air_humidity.publish(&am2315_1_air_humidity_msg);
  }
  if (am2315_1.get_air_temperature(am2315_1_air_temperature_msg)) {
    pub_am2315_1_air_temperature.publish(&am2315_1_air_temperature_msg);
  }
  // Read statuses of all modules
  if (should_read_statuses()) {
    openag_brain::DiagnosticStatus statuses[1];
    status_array.status_length = 1;
    openag_brain::DiagnosticStatus am2315_1_status;
    am2315_1_status.level = am2315_1.status_level;
    am2315_1_status.name = "am2315_1";
    am2315_1_status.code = am2315_1.status_code;
    statuses[0] = am2315_1_status;
    status_array.status = statuses;
    pub_diagnostics.publish(&status_array);
  }
}
