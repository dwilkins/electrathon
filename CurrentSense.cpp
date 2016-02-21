#include "CurrentSense.hpp"
void CurrentSense::init(RunMode mode) {
  // do some stuff to initialize it
  Adafruit_ADS1115 ads;
  strncpy(logBuffer,(char *)"No CurrentSense Logging Information",sizeof(logBuffer)-1);
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
    strcpy(logBuffer,String(m_amps).c_str());
    strcat(logBuffer,"\t");
    strcat(logBuffer,String(m_input_value).c_str());
    setLogTime(now);
    m_old_amps = m_amps;
  }
}

const char *CurrentSense::getLogHeader() {
  return "current_amps\tamps_input_value";
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
      {6000,25},
      {10000,29},
      {11000,33},
      {16000,35},
      {20000,33},
      {21000,29}
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
