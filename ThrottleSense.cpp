#include "ThrottleSense.hpp"


void ThrottleSense::init(RunMode mode) {
  // do some stuff to initialize it
  //  Serial.println("ThrottleSense::init ");
  populate_log_buffer();
  setLogTime(millis());
  Task::init(mode);
}

void ThrottleSense::run(uint32_t now) {
  incRunTime(m_rate);

  // TODO: Figure out the throttle level
  m_input_level = readInputValue(now);
  processInputValue(now);
  if(m_level != m_old_level) {
    populate_log_buffer();
    setLogTime(now);
    m_old_level = m_level;
  }
}

void ThrottleSense::populate_log_buffer() {
  strcpy(logBuffer,String(m_level,2).c_str());
  strcat(logBuffer,"\t");
  strcat(logBuffer,String(m_input_level).c_str());
}


const char *ThrottleSense::getLogHeader() {
  return "current_throttle_level\tthrottle_input_level";
}


void ThrottleSense::processInputValue(uint32_t now) {
  m_level = ((float)(m_input_max - (m_input_max - m_input_level)) / (float)m_input_max) * 100.0;
}

int16_t ThrottleSense::readInputValue(uint32_t now) {
  int16_t input_level = m_input_level;
  if(runMode == Task::RunMode::production) {
    // TODO: read the input value
    input_level = m_input_level;
  } else if(runMode == Task::RunMode::test) {
    static const PROGMEM uint32_t test_data[][2] = {
      {0,0},
      {6000,25},
      {10000,29},
      {11000,33},
      {16000,35},
      {20000,33},
      {21000,29},
      {23000,200},
      {25000,300},
      {27000,400},
      {29000,500},
      {31000,600},
      {33000,700},
      {35000,800},
      {37000,900},
      {39000,1000},
      {41000,1024},
      {43000,900},
      {45000,800},
      {47000,700},
      {49000,600},
      {51000,500},
      {53000,400},
      {55000,300},
      {57000,200},
      {59000,100},
      {61000,0}
    };
    for(int i = 0;i < sizeof(test_data) / sizeof(test_data[0]);i++) {
      uint32_t test_time = pgm_read_dword_near((uint16_t)&test_data[i][0]);
      uint32_t test_value = pgm_read_dword_near((uint16_t)&test_data[i][1]);
      if(m_last_input_time <= test_time && now >= test_time) {
        input_level = test_value;
        break;
        }
      // if(m_last_input_time <= test_data[i][0] && now >= test_data[i][0]) {
      //   input_level = test_data[i][1];
      //   break;
      // }
    }
  }
  m_last_input_time = now;
  return (input_level);
}
