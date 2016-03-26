#include "MotorControl.hpp"

void MotorControl::init(RunMode mode) {
  populate_log_buffer();
  setLogTime(millis());
  Task::init(mode);

  if(mode == Task::RunMode::production) {
    dac.begin(0x60);  // banggood version
    delay(1);
    dac.setVoltage(0,false);  //spicer
    esc.write(0);
  }
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
  if(values_changed || now > m_next_evaluation) {
    values_changed = true;
    m_next_evaluation = now + 250;
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
// to achieve that end.
//
void MotorControl::processInputValues(uint32_t now) {
  int new_transmission_level = 0;
  uint32_t command_time = 0;
  float decision_amps = TARGET_AMPS - 1.0;
  static uint32_t ignore_amps_time  = 0;
  static uint32_t shift_ignore_time = 0;

  // If we just shifted, prolly want to wait
  // a while before shifting again (SHIFT_GRACE_PERIOD)

   if(now < shift_ignore_time) {
     m_shift_ignore = true;
     return;
   } else {
     m_shift_ignore = false;
   }

  // if we just shifted *up*, then the amps
  // will spike, so we won't factor the amps
  // into the equation for a while (AMPS_SPIKE_PERIOD)
  if(now > ignore_amps_time) {
    decision_amps = m_amps;
    m_amps_ignore = false;
  } else {
    m_amps_ignore = true;
  }

  new_transmission_level = shiftPosition(decision_amps,
                                         m_speed_sense->getSpeed(),
                                         m_throttle);
  if(new_transmission_level != m_current_transmission_level) {
    if(new_transmission_level > m_current_transmission_level) {
      ignore_amps_time = now + AMPS_SPIKE_PERIOD;
    }
    shift_ignore_time = now + SHIFT_GRACE_PERIOD;
    addCommand(now,0,new_transmission_level);
  }
}

/*
 * shift_table is {(integer)(speed * 10.0), transmission_position}
 */
static const PROGMEM int32_t shift_table[][2] = {
  {0    ,0},
  {179 	,0},
  {215 	,900},
  {251 	,1400},
  {286 	,1700},
  {322 	,2000},
  {358 	,2300},
  {394 	,2400},
  {430 	,2800},
  {465 	,2900},
  {465 	,3000},
  {501 	,3300},
  {537 	,3400},
  {573 	,3700},
  {609 	,3800},
  {644 	,4000}
};

/*
 *  Shift up
 *    Amps below cutoff
 *    Speed table position within 10% of maximum
 *    Throttle above 90%
 *  Shift down
 *    Amps too high
 *    Only shift down 1 position from current speed table position
 *  Otherwise
 *    No change
 */

uint32_t MotorControl::shiftPosition(float amps, float speed, float throttle) {
  int shift_position = -1;
  int current_speed = ceil(speed * 10.0);
  uint32_t previous_speed = 0;
  int32_t positions[3] = {-1,-1,-1};
  int32_t speeds[3] =  {-1,-1,-1};
  for(int i = 0;i < sizeof(shift_table) / sizeof(shift_table[0]);i++) {
    uint32_t speed = pgm_read_dword_near((uint16_t)&shift_table[i][0]);
    uint32_t position = pgm_read_dword_near((uint16_t)&shift_table[i][1]);
    uint32_t boundary_speed = ceil(speed * 0.9);
    if(current_speed > boundary_speed) {   // 1 click below where the mph indicates
      positions[0] = position;
      speeds[0] = boundary_speed;
    } else if(positions[1] < 0) { // the position we should be in
      positions[1] = position;
      speeds[1] = boundary_speed;
    } else {                      // 1 click above where the mph indicates
      positions[2] = position;
      speeds[2] = speed;
      break;
    }
  }
  // for(int i = 0;i < 3;i++) {
  //   Serial1.print(positions[i]);
  //   Serial1.print("x - ");
  //   Serial1.print(speeds[i]);
  //   Serial1.print("m  ,  ");
  // }
  // Serial1.println(' ');

  if(amps > TARGET_AMPS) {
    shift_position = 0;
  } else if (throttle > m_throttle_threshold && (current_speed > speeds[1])) {
    shift_position = 2;
  } else {
    shift_position = 1;
  }

  while(positions[shift_position] < 0 && shift_position >= 0) {
    shift_position--;
  }

  return positions[shift_position];
}


//
// Return true if command was added or removed
// update current / target for speed / transmission
//
bool MotorControl::processCommands(uint32_t now) {
  for(int i=m_commands.next_to_process;i<MOTOR_COMMAND_COUNT;i++) {
    if(!m_commands.commands[i].completed && m_commands.commands[i].command_time < now) {
      change_transmission_level(m_commands.commands[i].transmission_level);
      m_commands.commands[i].completed = true;
    }
  }
  return(false);
}


void MotorControl::change_transmission_level(int transmission_level) {
  m_current_transmission_level = transmission_level;
  dac.setVoltage(transmission_level,false);  //spicer
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

  index++;
  if(index > MOTOR_COMMAND_COUNT) {
    index = 0;
  }
  for(int8_t i=index;i < MOTOR_COMMAND_COUNT;i++) {
    if(m_commands.commands[i].completed == false) {
      m_commands.commands[i].completed = true;
    }
  }
  if(index < 0) {
    for(int8_t i=0;i < index;i++) {
      if(m_commands.commands[i].completed == true) {
        m_commands.commands[i].completed = true;
      }
    }
  }
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

  amps     = m_current_sense->getAmps();

  // Shove these values into the array of samples.
  m_samples.samples[m_samples.next_free].sample_time = now;
  m_samples.samples[m_samples.next_free].amps = amps;
  m_samples.samples[m_samples.next_free].throttle_level = throttle;
  m_samples.last_input_time = now;
  m_samples.next_input_time = now + 50;  // TODO: make this better
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
  static const char tab_str[] PROGMEM = ",";
  strcpy(logBuffer,"*");
  strcat(logBuffer,String(m_current_motor_level).c_str());
  strcat(logBuffer,"*");
  strcat_P(logBuffer,tab_str);
  strcat(logBuffer,String(m_target_motor_level).c_str());
  strcat_P(logBuffer,tab_str);
  strcat(logBuffer,"x");
  strcat(logBuffer,String(m_current_transmission_level).c_str());
  strcat(logBuffer,"x");
  strcat_P(logBuffer,tab_str);
  strcat(logBuffer,"x");
  strcat(logBuffer,String(m_target_transmission_level).c_str());
  strcat(logBuffer,"x");
  strcat_P(logBuffer,tab_str);
  strcat(logBuffer,String(m_amps,2).c_str());
  strcat_P(logBuffer,tab_str);
  strcat(logBuffer,"t");
  strcat(logBuffer,String(m_throttle,2).c_str());
  strcat(logBuffer,"t");
  if(m_amps_ignore) {
    strcat(logBuffer,",A");
  } else {
    strcat(logBuffer,",a");
  }
  if(m_shift_ignore) {
    strcat(logBuffer,",S");
  } else {
    strcat(logBuffer,",s");
  }
}

const char *MotorControl::getLogHeader() {
  return "current_motor,target_motor,current_gear,target_gear,amps,throttle";
}
