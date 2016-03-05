#include "SpeedSense.hpp"

volatile uint16_t rpm_counter = 0;
void rpm_interrupt();


void SpeedSense::init(RunMode mode) {
  // do some stuff to initialize it
  // setup the interrupts
  rpm_counter = 0;
  m_last_input_time = 0;
  if(mode == Task::RunMode::production) {
    attachInterrupt (digitalPinToInterrupt(m_pin), rpm_interrupt, FALLING);
  }
  populate_log_buffer();
  setLogTime(millis());
  Task::init(mode);
}

SpeedSense::~SpeedSense() {}


void SpeedSense::run(uint32_t now) {
  incRunTime(m_rate);
  m_rpm = readInputValue(now);
  processInputValue(now);
  // Triggers a log event when the speed changes
  if(m_speed != m_old_speed || m_rpm != m_old_rpm) {
    populate_log_buffer();
    setLogTime(now);
    m_old_speed = m_speed;
    m_old_rpm = m_rpm;
  }
}

void SpeedSense::populate_log_buffer() {
  static const char mph_str[] PROGMEM = "mph";
  static const char rpm_str[] PROGMEM = "rpm";
  strcat(logBuffer,String(m_rpm).c_str());
  strcat_P(logBuffer,rpm_str);
  strcpy(logBuffer,String(m_speed,2).c_str());
  strcat_P(logBuffer,mph_str);
}


const char *SpeedSense::getLogHeader() {
  return "rpm,speed";
}

void SpeedSense::processInputValue(uint32_t now) {
  m_speed = (m_rpm * 60.0) * m_wheel_circumference_in_miles;
}


int16_t SpeedSense::readInputValue(uint32_t now) {
  float rpm = m_rpm;
  int16_t current_rpms = -1;
  uint32_t current_input_time;
  if(runMode == Task::RunMode::production) {
    current_input_time = millis();
    noInterrupts();
    current_rpms = rpm_counter;
    rpm_counter = 0;
    interrupts();
  } else if(runMode == Task::RunMode::test) {
    //
    // Storing the test values in PROGMEM saves some space
    // but makes you have to pull the values out by hand
    // rather than just accessing them like normal
    //
    // This speed test deck is setup so that the last known
    // RPM / MPH will stick - i.e. it won't go down when the
    // test data runs out.
    //
    static const PROGMEM uint32_t test_data[][2] = {
      {1,0},
      {10000,34},
      {20000,70},
      {30000,80},
      {40000,100}
    };
    // Serial.print(F("m_last_input_time="));
    // Serial.print(m_last_input_time);
    // Serial.print(F(",now="));
    // Serial.println(now);
    for(int i = 0;i < sizeof(test_data) / sizeof(test_data[0]);i++) {
      uint32_t test_time = pgm_read_dword_near((uint16_t)&test_data[i][0]);
      uint32_t test_value = pgm_read_dword_near((uint16_t)&test_data[i][1]);
      // Serial.print(F("test_time="));
      // Serial.print(test_time);
      // Serial.print(F(",test_value="));
      // Serial.println(test_value);
      if((m_last_input_time < test_time) && (now >= test_time)) {
        current_input_time = test_time;
        current_rpms = test_value;
        break;
      }
    }
  }
  if(current_rpms > -1) {
    // Serial.print(F("\nCalculating from current_rpms="));
    // Serial.print(current_rpms);
    // Serial.print(F("current_input_time="));
    // Serial.print(current_input_time);
    // Serial.print(F("m_last_input_time="));
    // Serial.print(m_last_input_time);
    // Serial.print(F("elapsed="));
    // Serial.println(current_input_time - m_last_input_time);
    rpm = ceil(((float)current_rpms / ((float)(current_input_time - m_last_input_time) / (1000.0 * 30.0) ) )); // spicer
    m_last_input_time = current_input_time;
  }
  return rpm;
}

void rpm_interrupt(){
  ++rpm_counter;
}
