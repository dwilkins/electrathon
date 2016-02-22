/*
 * MotorControl.hpp
 *
 * (C) Copyright 2016
 * David H. Wilkins  <dwilkins@conecuh.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#ifndef MOTOR_CONTROL_HPP
#define MOTOR_CONTROL_HPP
#include "Task.hpp"

#include "CurrentSense.hpp"
#include "ThrottleSense.hpp"

class MotorControl : public Task {
public:
  MotorControl(uint32_t i2c_addr,
               CurrentSense *current_sense,
               ThrottleSense *throttle_sense,
               uint32_t log_rate = 10000
               ) : Task(),
                   m_i2c_addr(i2c_addr),
                   m_current_sense(current_sense),
                   m_throttle_sense(throttle_sense)
  {
    // Do some setup stuff?
    m_samples.next_free = 0;
    for(int i = 0; i< sizeof(m_samples.samples) / sizeof(m_samples.samples[0]); i++) {
      m_samples.samples[i].sample_time = 0;
      m_samples.samples[i].amps = m_samples.samples[i].throttle_level = 0.0;
    }
    for(int i = 0; i< sizeof(m_commands.commands) / sizeof(m_commands.commands[0]); i++) {
      m_commands.commands[i].command_time = 0;
      m_commands.commands[i].motor_level = 0;
      m_commands.commands[i].transmission_level = 0;
      m_commands.commands[i].completed = true;
    }
    m_target_speed = m_current_speed = 0;
    m_target_transmission = m_current_transmission = 0;
    m_amps = 0.0;
    m_throttle = 0.0;
    m_amps_resolution = 0.5;
    m_throttle_resolution = 0.5;
    m_throttle_threshold = 75;
  }

  virtual ~MotorControl();

  virtual bool canRun(uint32_t now);
  virtual void run(uint32_t now);
  virtual const char *getLogHeader();
  virtual void init(RunMode mode);

  bool readInputValues(uint32_t now);
  void processInputValues(uint32_t now);

  bool processCommands(uint32_t now);


  struct input_sample_t {
    uint32_t sample_time;
    float amps;
    float throttle_level;
  };

  struct input_samples_t {
    int next_free;
    uint32_t last_input_time; // timestamp when input values fetched last
    uint32_t next_input_time;
    input_sample_t samples[10];
  };

  struct motor_command_t {
    uint32_t command_time;
    int motor_level;
    int transmission_level;
    bool completed;
  };

  struct motor_commands_t {
    int next_free;
    int next_to_process;
    uint32_t last_comand_time; // timestamp when command completed last
    uint32_t next_command_time;
    motor_command_t commands[10];
  };

private:
  CurrentSense *m_current_sense;
  ThrottleSense *m_throttle_sense;
  motor_commands_t m_commands;
  input_samples_t m_samples;

  int m_i2c_addr;
  int m_current_speed;
  int m_target_speed;
  int m_current_transmission;
  int m_target_transmission;

  float m_amps;
  float m_throttle;

  float m_amps_resolution;
  float m_throttle_resolution;

  float m_throttle_threshold;

  void populate_log_buffer();

};

#endif
