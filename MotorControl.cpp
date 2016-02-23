#include "MotorControl.hpp"
void MotorControl::init(RunMode mode) {
  // do some stuff to initialize it
  populate_log_buffer();
  setLogTime(millis());
  Task::init(mode);
}

MotorControl::~MotorControl() {}

bool MotorControl::canRun(uint32_t now) {
  return true;
}

//
// Two things need to happen in run()
//
// - Read and process the input values
// - Perform any commands that need to
//   be handled now.
//
void MotorControl::run(uint32_t now) {
  bool values_changed = readInputValues(now);
  if(values_changed) {
    processInputValues(now);
  }
  values_changed |= processCommands(now);
  if(values_changed) {
    populate_log_buffer();
    setLogTime(now);
  }
}

//
// populate the logBuffer so that the async
// logging code can pull it next time it's
// needed.
//
void MotorControl::populate_log_buffer() {
  strcpy(logBuffer,String(m_current_speed).c_str());
  strcat(logBuffer,"\t");
  strcat(logBuffer,String(m_target_speed).c_str());
  strcat(logBuffer,"\t");
  strcat(logBuffer,String(m_current_transmission).c_str());
  strcat(logBuffer,"\t");
  strcat(logBuffer,String(m_target_transmission).c_str());
  strcat(logBuffer,"\t");
  strcat(logBuffer,String(m_amps,6).c_str());
  strcat(logBuffer,"\t");
  strcat(logBuffer,String(m_throttle,6).c_str());
}

const char *MotorControl::getLogHeader() {
  return "current_speed\ttarget_speed\tcurrent_gear\ttarget_gear\tamps\tthrottle";
}

//
// processInputValues should calculate where the speed
// and transmission should be and insert some commands
// to achieve that end.   Commands are simply transmission
// and motor speed values to be updated at specific times.
// for example, if the throttle is "floored" from a dead
// stop, it may take several seconds to efficiently
// achieve maximum speed.
//
// Also, if midway through the acceleration the
// throttle is backed off, some existing unfinished commands
// will need to be amended to achieve the desired
// results.
//
// Maybe this is too complicated, but it seems like the
// only way to allow multiple inputs (Amps + Throttle + Speed)
// to generate rapid, yet smooth acceleration with
// optional shifing.
//
void MotorControl::processInputValues(uint32_t now) {
  // calculate motor speed
  // calculate transmission setting
  // calculate number of steps and step spacing to achieve
  // insert the commands that are required to make it happen
  // PROFIT
  //  if(m_throttle < )
}

//
// Return true if command was added or removed
// update current / target for speed / transmission

//
bool MotorControl::processCommands(uint32_t now) {
  return(false);
}

//
// Return true if values have changed
//
bool MotorControl::readInputValues(uint32_t now) {
  float amps;
  float throttle;
  static int max_samples = sizeof(m_samples.samples) / sizeof(m_samples.samples[0]);
  if(now < m_samples.next_input_time) {return false;}

  throttle = m_throttle_sense->getLevel();
  amps = m_current_sense->getAmps();
  // Shove these values into the array of samples.
  m_samples.samples[m_samples.next_free].sample_time = now;
  m_samples.samples[m_samples.next_free].amps = amps;
  m_samples.samples[m_samples.next_free].throttle_level = throttle;
  m_samples.last_input_time = now;
  m_samples.next_input_time = now + 100;  // TODO: make this better
  m_samples.next_free++;
  if(m_samples.next_free >= max_samples) {
    m_samples.next_free = 0;
  }

  // Calculate the average of the samples
  // and use that value for the current sample
  // the hope is that this will have a smoothing
  amps = 0.0;
  throttle = 0.0;
  for(int i=0;i<max_samples;i++) {
    amps += m_samples.samples[i].amps;
    throttle += m_samples.samples[i].throttle_level;
  }
  amps = amps / (float)max_samples;
  throttle = throttle / (float)max_samples;

  // Now, check to see if the change from the current
  // value is within the resolution of what we want to
  // see.  If so, update the member variables and
  // return true indicating that values have changed
  if((amps > (m_amps + m_amps_resolution) || amps < (m_amps - m_amps_resolution)) ||
     (throttle > (m_throttle + m_throttle_resolution) || throttle < (m_throttle - m_throttle_resolution))) {
    m_amps = amps;
    m_throttle = throttle;
    return true;
  } else {
    return false;
  }
}
