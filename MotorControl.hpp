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
#include "SpeedSense.hpp"
#include <Wire.h>
#include "Adafruit_MCP4725.h"

#include <Servo.h>  // spicer

#define MOTOR_COMMAND_COUNT ((int8_t)10)
#define TARGET_AMPS 50.0

class MotorControl : public Task {
public:
  MotorControl(uint32_t i2c_addr,
               CurrentSense *current_sense,
               ThrottleSense *throttle_sense,
               SpeedSense *speed_sense,
               uint32_t log_rate = 10000
               ) : Task(),
                   m_i2c_addr(i2c_addr),
                   m_current_sense(current_sense),
                   m_throttle_sense(throttle_sense),
                   m_speed_sense(speed_sense)
  {
    // Do some setup stuff?
    m_samples.next_free = 0;
    for(int i = 0; i< sizeof(m_samples.samples) / sizeof(m_samples.samples[0]); i++) {
      m_samples.samples[i].sample_time = 0;
      m_samples.samples[i].amps = m_samples.samples[i].throttle_level = 0.0;
    }
    for(uint8_t i = 0; i< MOTOR_COMMAND_COUNT; i++) {
      m_commands.commands[i].command_time = 0;
      m_commands.commands[i].motor_level = 0;
      m_commands.commands[i].transmission_level = 0;
      m_commands.commands[i].completed = true;
    }
    m_target_motor_level = m_current_motor_level = 0;
    m_target_transmission_level = m_current_transmission_level = 0;
    m_amps = 0.0;
    m_throttle = 0.0;
    m_amps_resolution = 0.5;
    m_throttle_resolution = 0.5;
    m_throttle_threshold = 75.0;
    m_max_motor_level = 179 - 46;
    m_max_transmission_level = 4095;
    m_next_evaluation = 0;
    m_motor_level_resolution = ceil(m_max_motor_level / 30);
    m_transmission_level_resolution = ceil(m_max_transmission_level / 30);
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
    uint8_t next_free;
    uint32_t last_input_time; // timestamp when input values fetched last
    uint32_t next_input_time;
    input_sample_t samples[MOTOR_COMMAND_COUNT];
  };

  struct motor_command_t {
    uint32_t command_time;
    int motor_level;
    int transmission_level;
    bool completed;
  };

  struct motor_commands_t {
    uint8_t next_free;
    uint8_t next_to_process;
    uint32_t last_comand_time; // timestamp when command completed last
    uint32_t next_command_time;
    motor_command_t commands[MOTOR_COMMAND_COUNT];
  };

private:
  CurrentSense *m_current_sense;
  ThrottleSense *m_throttle_sense;
  SpeedSense *m_speed_sense;
  motor_commands_t m_commands;
  input_samples_t m_samples;

  uint8_t pendingCommands(uint32_t after_when = 0);
  bool addCommand(uint32_t when, int motor_level,int transmission_level);

  void increment_next_free();

  void change_motor_level(int motor_level);
  void change_transmission_level(int transmission_level);

  int transmission_level_from_speed(int increment = 0);
  int adjusted_transmission_level(int level,int increment = 0);

  int m_i2c_addr;
  int m_current_motor_level;
  int m_target_motor_level;
  int m_current_transmission_level;
  int m_target_transmission_level;
  uint32_t m_next_evaluation;
  int m_max_motor_level;
  int m_max_transmission_level;

  float m_amps;
  float m_throttle;

  float m_amps_resolution;
  float m_throttle_resolution;
  float m_throttle_threshold;

  int m_motor_level_resolution;
  int m_transmission_level_resolution;

  Servo esc;             //spicer
  Adafruit_MCP4725 dac;  //spicer

  void populate_log_buffer();

};

#endif
