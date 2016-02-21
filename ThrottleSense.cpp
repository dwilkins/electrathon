#include "ThrottleSense.hpp"


void ThrottleSense::init(RunMode mode) {
  // do some stuff to initialize it
  //  Serial.println("ThrottleSense::init ");
  strncpy(logBuffer,(char *)"No ThrottleSense Logging Information",sizeof(logBuffer)-1);
  setLogTime(millis());
  Task::init(mode);
}

void ThrottleSense::run(uint32_t now) {
  incRunTime(m_rate);

  // TODO: Figure out the throttle level
  m_input_level = readInputValue(now);
  processInputValue(now);
  if(m_level != m_old_level) {
    strcpy(logBuffer,String(m_level).c_str());
    strcat(logBuffer,"\t");
    strcat(logBuffer,String(m_input_level).c_str());
    setLogTime(now);
    m_old_level = m_level;
  }
}

const char *ThrottleSense::getLogHeader() {
  return "current_throttle_level\tthrottle_input_level";
}


void ThrottleSense::processInputValue(uint32_t now) {
  m_level = m_input_level;
}

int16_t ThrottleSense::readInputValue(uint32_t now) {
  int16_t input_level = m_input_level;
  if(runMode == Task::RunMode::production) {
    // TODO: read the input value
    input_level = m_input_level;
  } else if(runMode == Task::RunMode::test) {
    static int test_data[][2] = {
      {0,0},
      {6000,25},
      {10000,29},
      {11000,33},
      {16000,35},
      {20000,33},
      {21000,29}
    };
    for(int i = 0;i < sizeof(test_data) / sizeof(test_data[0]);i++) {
      if(m_last_input_time <= test_data[i][0] && now >= test_data[i][0]) {
        input_level = test_data[i][1];
        break;
      }
    }
  }
  m_last_input_time = now;
  return (input_level);
}
