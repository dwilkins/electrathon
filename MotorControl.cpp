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
  int motor_level = ceil((m_throttle / 100.0) * m_max_motor_level);
  int motor_level_distance = abs(motor_level - m_target_motor_level);
  if(motor_level_distance < m_motor_level_resolution) {
    return;
  }
  if(m_throttle < m_throttle_threshold) {
    int new_transmission_level = 0;
    uint32_t command_time = 0;
    if(m_amps > TARGET_AMPS) {
      new_transmission_level = transmission_level_from_speed(-1);
      command_time = now;
    } else {
      new_transmission_level = transmission_level_from_speed();
    }
    // TODO: handle slowing down and shifting down...
    addCommand(now,motor_level,new_transmission_level);
  } else {
    if(m_amps > TARGET_AMPS &&
       (m_target_transmission_level > 0) &&
       pendingCommands(now + 500) == 0) {
      addCommand(now,motor_level, adjusted_transmission_level(m_target_transmission_level,-1));
      addCommand(now + 1500,motor_level, adjusted_transmission_level(m_target_transmission_level,-1));
    } else if(m_amps < TARGET_AMPS &&
              m_current_transmission_level == m_target_transmission_level &&
              pendingCommands() == 0) {
      for(int8_t i=0;(m_amps + (TARGET_AMPS / 5.0) * (float)i) < TARGET_AMPS,i<3;i++) {
        addCommand(now+(1000 * i),motor_level, adjusted_transmission_level(m_target_transmission_level,1 + i));
      }
    } else {
    }
    int motor_level_step = motor_level_distance > m_motor_level_resolution ? motor_level_distance / 3 : motor_level_distance;
    int target_motor_level = m_target_motor_level;
    if(motor_level < m_target_motor_level) {
      motor_level_step = -motor_level_step;
    }
    for(int i = 0;i<3;i++) {
      addCommand(now + (500 * i),target_motor_level += motor_level_step,m_current_transmission_level);
    }
  }
}

//
// Return true if command was added or removed
// update current / target for speed / transmission

//
bool MotorControl::processCommands(uint32_t now) {
  for(int i=m_commands.next_to_process;i<MOTOR_COMMAND_COUNT;i++) {
    if(!m_commands.commands[i].completed && m_commands.commands[i].command_time < now) {
      change_motor_level(m_commands.commands[i].motor_level);
      change_transmission_level(m_commands.commands[i].transmission_level);
      m_commands.commands[i].completed = true;
    }
  }
  return(false);
}
//
// Calculate what the transmission level should be based on the current
// speed. Useful for low-throttle situations
//
int MotorControl::transmission_level_from_speed(int increment) {
  return(adjusted_transmission_level(m_speed_sense->getRelativeSpeed() * m_max_transmission_level),increment);
}

//
// Turn a raw level into an adjusted level so that we're not
// making micro adjustments to the transmission (unless we want to)

//
int MotorControl::adjusted_transmission_level(int level,int increment) {
  int new_level = ((level / m_transmission_level_resolution)  + increment) * m_transmission_level_resolution;
  return(new_level > m_max_transmission_level ?
         m_max_transmission_level :
         (new_level < 0 ? 0 : new_level));
}



void MotorControl::change_motor_level(int motor_level) {
  m_current_motor_level = motor_level;
}
void MotorControl::change_transmission_level(int transmission_level) {
  m_current_transmission_level = transmission_level;
}



bool MotorControl::addCommand(uint32_t when, int motor_level,int transmission_level) {
  bool overwrite = m_commands.commands[m_commands.next_free].completed;
  int8_t index = -1;
  for(int8_t i=m_commands.next_to_process;i < MOTOR_COMMAND_COUNT;i++) {
    if((m_commands.commands[i].completed == true) ||
       m_commands.commands[i].command_time >= when) {
      index = i;
      break;
    }
  }
  if(index < 0) {
    for(int8_t i=0;i < m_commands.next_to_process;i++) {
      if((m_commands.commands[i].completed == true) ||
         m_commands.commands[i].command_time >= when) {
        index = i;
        break;
      }
    }
  }
  if(index < 0) {return false;} // no command slots available
  m_commands.commands[index].command_time = when;
  m_commands.commands[index].motor_level = motor_level;
  m_commands.commands[index].transmission_level = transmission_level;
  m_commands.commands[index].completed = false;
  increment_next_free();
  return !overwrite;
}

void MotorControl::increment_next_free() {
  uint32_t min_ready_time = UINT32_MAX;
  uint32_t max_ready_time = 0;
  uint8_t min_ready_index = MOTOR_COMMAND_COUNT + 1;
  uint8_t max_ready_index = MOTOR_COMMAND_COUNT + 1;
  uint8_t next_free = MOTOR_COMMAND_COUNT + 1;
  for(int8_t i = 0;i < MOTOR_COMMAND_COUNT;i++) {
    if(m_commands.commands[i].completed == false) {
      if(min_ready_time > m_commands.commands[i].command_time ) {
        min_ready_time = m_commands.commands[i].command_time;
        min_ready_index = i;
      }
      if(max_ready_time < m_commands.commands[i].command_time ) {
        max_ready_time = m_commands.commands[i].command_time;
        max_ready_index = i;
      }
    } else {
      if(min_ready_index > MOTOR_COMMAND_COUNT) {
        next_free = i;
      } else if (next_free > MOTOR_COMMAND_COUNT) {
        next_free = i;
      }
    }
  }
  m_commands.next_to_process = min_ready_index;
  m_commands.next_free = next_free;
  // Since where here, populate the target motor_level and transmmission level
  m_target_motor_level = m_commands.commands[max_ready_index].motor_level;
  m_target_transmission_level = m_commands.commands[max_ready_index].transmission_level;
}


//
// Return true if values have changed
//
bool MotorControl::readInputValues(uint32_t now) {
  float amps;
  float throttle;
  static uint8_t max_samples = sizeof(m_samples.samples) / sizeof(m_samples.samples[0]);
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
  for(uint8_t i=0;i<max_samples;i++) {
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


uint8_t MotorControl::pendingCommands(uint32_t after_when) {
  uint8_t pending = 0;
  for(uint8_t i=0;i < MOTOR_COMMAND_COUNT;i++) {
    if(m_commands.commands[i].completed == false &&
       m_commands.commands[i].command_time > after_when) {
      pending++;
    }
  }
  return pending;
}


//
// populate the logBuffer so that the async
// logging code can pull it next time it's
// needed.
//
void MotorControl::populate_log_buffer() {
  static const char tab_str[] PROGMEM = "\t";
  strcpy(logBuffer,String(m_current_motor_level).c_str());
  strcat_P(logBuffer,tab_str);
  strcat(logBuffer,String(m_target_motor_level).c_str());
  strcat_P(logBuffer,tab_str);
  strcat(logBuffer,String(m_current_transmission_level).c_str());
  strcat_P(logBuffer,tab_str);
  strcat(logBuffer,String(m_target_transmission_level).c_str());
  strcat_P(logBuffer,tab_str);
  strcat(logBuffer,String(m_amps,2).c_str());
  strcat_P(logBuffer,tab_str);
  strcat(logBuffer,String(m_throttle,2).c_str());
}

const char *MotorControl::getLogHeader() {
  return "current_motor\ttarget_motor\tcurrent_gear\ttarget_gear\tamps\tthrottle";
}

