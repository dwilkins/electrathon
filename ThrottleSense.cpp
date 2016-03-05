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
  strcat(logBuffer,",");
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
    analogReference(DEFAULT);
    int16_t val = analogRead(0); // spicer hack
    int throttle = map(val,185,620,0,100);
    if (throttle < 0)  throttle = 0;
    input_level = throttle;
  } else if(runMode == Task::RunMode::test) {
    static const PROGMEM uint32_t test_data[][2] = {
      {0,0},
      {6000,1},
      {10000,1},
      {11000,1},
      {16000,1},
      {20000,1},
      {21000,20},
      {23000,20},
      {25000,20},
      {27000,30},
      {29000,30},
      {31000,30},
      {33000,30},
      {35000,30},
      {37000,30},
      {39000,30},
      {41000,30},
      {43000,30},
      {45000,30},
      {47000,30},
      {49000,30},
      {51000,40},
      {53000,40},
      {55000,30},
      {57000,20},
      {59000,10},
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
