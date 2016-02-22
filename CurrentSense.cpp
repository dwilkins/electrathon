#include "CurrentSense.hpp"
void CurrentSense::init(RunMode mode) {
  // do some stuff to initialize it
  populate_log_buffer();
  setLogTime(millis());
  Task::init(mode);
  if(mode == Task::RunMode::production) {
    ads.begin();
  }
}

CurrentSense::~CurrentSense() {}


void CurrentSense::run(uint32_t now) {
  incRunTime(m_rate);
  m_input_value = readInputValue(now);
  processInputValue(now);
  // TODO: set m_amps to the right value
  if(m_amps != m_old_amps) {
    populate_log_buffer();
    setLogTime(now);
    m_old_amps = m_amps;
  }
}

void CurrentSense::populate_log_buffer() {
  memset(logBuffer,0,sizeof(logBuffer)-1);
  strcpy(logBuffer,String(m_amps,6).c_str());
  strcat(logBuffer,"\t");
  strcat(logBuffer,String(m_input_value).c_str());
}


char *CurrentSense::getLogHeader() {
  return (char *)"current_amps\tamps_input_value";
}

void CurrentSense::processInputValue(uint32_t now) {
   float mv = ((float) m_input_value * .0078125F);
   m_amps = mv * 1.0;
}


int16_t CurrentSense::readInputValue(uint32_t now) {
  int16_t input_value = m_input_value;
  if(runMode == Task::RunMode::production) {
    // TODO: read the input value
    input_value = ads.readADC_Differential_0_1();
  } else if(runMode == Task::RunMode::test) {
    static int test_data[][2] = {
      {0,0},
      {6002,25},
      {10003,29},
      {11004,33},
      {16005,35},
      {20006,33},
      {21007,29}
    };
    for(int i = 0;i < sizeof(test_data) / sizeof(test_data[0]);i++) {
      if(m_last_input_time <= test_data[i][0] && now >= test_data[i][0]) {
        input_value = test_data[i][1];
        break;
      }
    }
  }
  m_last_input_time = now;
  return (input_value);
}
