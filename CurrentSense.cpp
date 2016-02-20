#include "CurrentSense.hpp"


void CurrentSense::init() {
  // do some stuff to initialize it
  strncpy((char *)"No Logging Information",m_log_buffer,sizeof(m_log_buffer)-1);
}

CurrentSense::~CurrentSense() {}


bool CurrentSense::canRun(uint32_t now) {
  now >= runTime + m_rate;
}

void CurrentSense::run(uint32_t now) {
}

char *CurrentSense::getLoggingData(uint32_t now) {
  if(canRun(now)) {
    // TODO: calculate the buffer
    memset(m_log_buffer,0,sizeof(m_log_buffer)-1);
  }
  return m_log_buffer;
}

//
// Maybe store an array of amps along with the
// current milliseconds for debug info
//
float CurrentSense::getAmps(uint32_t now) {
  // figure out what the current amp setting is
  // and store in m_amps
  return m_amps;
}
