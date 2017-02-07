/*
 * SpeedSense.hpp
 *
 * (C) Copyright 2016-2017
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


#ifndef SPEED_SENSE_HPP
#define SPEED_SENSE_HPP
#include "Task.hpp"

#define PULSES_PER_WHEEL_REVOLUTION 2UL
#define MAX_PULSE_AGE_MILLIS 2000UL

class SpeedSense : public TimedTask {
public:
  SpeedSense(uint32_t pin,
             float wheel_circumference,
             uint32_t rate = 1000,
             uint32_t log_rate = 1000 // every 1 second regardless
             ) : TimedTask(millis()),
                 m_pin(pin),
                 m_rate(rate),
                 m_wheel_circumference(wheel_circumference)
  {
    logRate = log_rate;
    m_speed = 0.0;
    m_rpm = 0.0;
    m_last_input_time = 0;
    m_wheel_circumference_in_miles = m_wheel_circumference / 63360.0;
    m_top_speed = 42.0;
  }
  virtual ~SpeedSense();

  virtual void run(uint32_t now);
  virtual const char *getLogHeader();
  virtual void init(RunMode mode);

  float getSpeed() { return m_speed; };
  float getRelativeSpeed() { return m_speed / m_top_speed;}
  void setRate(uint32_t rate) {
    m_rate = rate;
  };

  int16_t readInputValue(uint32_t now);
  void processInputValue(uint32_t now);


private:

  void read_input_value();
  int16_t m_pin;
  uint16_t m_rpm;
  float m_wheel_circumference;
  float m_wheel_circumference_in_miles;
  float m_speed;
  float m_top_speed;
  uint32_t m_pulse_duration;
  uint32_t m_rate; // In milliseconds
  uint32_t m_last_input_time;
  void populate_log_buffer();
};

#endif
