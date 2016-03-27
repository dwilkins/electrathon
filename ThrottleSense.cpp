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
  strcpy(logBuffer,String(m_input_level).c_str());
}

const char *ThrottleSense::getLogHeader() {
  return "throttle_level";
}

void ThrottleSense::processInputValue(uint32_t now) {
//  m_level = ((float)(m_input_size - (m_input_size - m_input_level)) / (float)m_input_size) * 100.0;

  m_level =  (float) m_input_level / 100.0;
}

int16_t ThrottleSense::readInputValue(uint32_t now) {
  int16_t input_level = m_input_level;

  if(runMode == Task::RunMode::production) {
    uint32_t input_level_average = 0;

    int val = analogRead(0);

   // Serial1.println(val);

    input_level = map(val,0,819,0,100);  /// seems to work

    if (input_level > 100) {
        input_level = 100;
    }

   // Serial.print("throttle:");
   // Serial.println(val);

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
      {27000,91},
      {29000,91},
      {31000,91},
      {33000,91},
      {35000,91},
      {37000,91},
      {39000,91},
      {41000,91},
      {43000,91},
      {45000,91},
      {47000,91},
      {49000,91},
      {51000,40},
      {53000,40},
      {55000,91},
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
    }
  }
  m_last_input_time = now;
  return (input_level);
}
