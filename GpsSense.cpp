#include "GpsSense.hpp"

void GpsSense::init(RunMode mode) {
  m_last_input_time = 0;
  if(mode == Task::RunMode::production) {
    Serial2.begin(9600);
  }
  memset(m_gps_data,0,sizeof(m_gps_data));
  populate_log_buffer();
  setLogTime(millis());
  Task::init(mode);
}

GpsSense::~GpsSense() {}


void GpsSense::run(uint32_t now) {
  incRunTime(m_rate);
  readInputValue(now);
  processInputValue(now);
  // Triggers a log event when the speed changes
  if(true) {
    populate_log_buffer();
    setLogTime(now);
  }
}

void GpsSense::populate_log_buffer() {
  static const char mph_str[] PROGMEM = "mph";
  static const char rpm_str[] PROGMEM = "rpm,";
}


const char *GpsSense::getLogHeader() {
  return "rpm,speed";
}

void GpsSense::processInputValue(uint32_t now) {

}


int16_t GpsSense::readInputValue(uint32_t now) {
  int16_t current_rpms = -1;
  uint32_t current_input_time;
  if(runMode == Task::RunMode::production) {

  } else if(runMode == Task::RunMode::test) {
    //
    // Storing the test values in PROGMEM saves some space
    // but makes you have to pull the values out by hand
    // rather than just accessing them like normal
    //
    static const PROGMEM uint32_t test_data[][2] = {
      {1,0},
      {10000,34},
      {20000,70},
      {30000,80},
      {40000,100},
      {45000,120},
      {50000,140},
      {55000,160},
      {60000,170},
      {65000,180},
      {70000,180},
      {75000,180},
      {85000,180}
    };
  }
  return 0;
}
