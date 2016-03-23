#include "CurrentSense.hpp"

void CurrentSense::init(RunMode mode) { 

  // do some stuff to initialize it
  populate_log_buffer();
  setLogTime(millis());

  Task::init(mode);

  if (mode == Task::RunMode::production) {

   ads.setGain(GAIN_EIGHT);        // 015625mV   8x gain   +/- 0.512V  1 bit =  0.015625mV
    
   // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.0078125mV
   // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 0.1875mV (default)  
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
  strcpy(logBuffer,String(m_amps,2).c_str());
  strcat(logBuffer,",");
  strcat(logBuffer,String(m_input_value).c_str());
}

const char *CurrentSense::getLogHeader() {
  return "current_amps,amps_input_value";
}

void CurrentSense::processInputValue(uint32_t now) {
   // 50mv over 200amps  = .25mv per amp so multiply x4;
   // m_amps = (double) m_input_value * 0.0078125F  * 4.0F;   // 16x gain

    m_amps = (float) m_input_value * 0.015625F  *  4.0F;   // 8x gain
}


int16_t CurrentSense::readInputValue(uint32_t now) {
  int16_t input_value = m_input_value;
  
  if(runMode == Task::RunMode::production) {
    
      input_value = ads.readADC_Differential_2_3(); // spicer

    //  input_value = ads.readADC_SingleEnded(1);
    
  //  Serial.print("adc read: ");
  //  Serial.println(input_value);

  } else if(runMode == Task::RunMode::test) {
    //
    // Storing the test values in PROGMEM saves some space
    // but makes you have to pull the values out by hand
    // rather than just accessing them like normal
    //
    static const PROGMEM uint32_t test_data[][2] = {
      {0,0},
      {6002,25},
      {10003,29},
      {11004,33},
      {16005,35},
      {20006,33},
      {21007,49},
      {22007,59},
      {23007,69},
      {24007,79},
      {25007,89},
      {26007,99},
      {27007,109},
      {28007,219},
      {29007,329},
      {30007,439},
      {31007,549},
      {32007,659},
      {33007,769},
      {34007,879},
      {35007,889},
      {36007,1899},
      {37007,4899},
      {38007,5899},
      {39007,6899}
    };


    for(int i = 0;i < sizeof(test_data) / sizeof(test_data[0]);i++) {
      uint32_t test_time = pgm_read_dword_near((uint16_t)&test_data[i][0]);
      uint32_t test_value = pgm_read_dword_near((uint16_t)&test_data[i][1]);
      if(m_last_input_time <= test_time && now >= test_time) {
        input_value = test_value;
        break;
      }
    }
  }
  m_last_input_time = now;
  return (input_value);
}
