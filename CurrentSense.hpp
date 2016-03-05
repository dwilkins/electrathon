/*
 * CurrentSense.hpp
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


#ifndef CURRENT_SENSE_HPP
#define CURRENT_SENSE_HPP
#include <Wire.h>
#include "Task.hpp"
#include "Adafruit_ADS1015.h"

class CurrentSense : public TimedTask {
public:
  CurrentSense(uint32_t i2c_addr,
               uint32_t rate = 2000,
               uint32_t log_rate = 10000,
               uint32_t input_min = 0,
               uint32_t input_max = 65535,
               float output_min = 0.0,
               float output_max = 100.0
               ) : TimedTask(millis()),
                   m_rate(rate),
                   m_i2c_addr(i2c_addr),
                   m_input_min(input_min),
                   m_input_max(input_max),
                   m_output_min(output_min),
                   m_output_max(output_max)
  {
    m_amps = 0.0;
    m_old_amps = 0.0;
    m_last_input_time = 0;
    // Do some setup stuff?
  }
  virtual ~CurrentSense();

  virtual void run(uint32_t now);
  virtual const char *getLogHeader();
  //  virtual char *getLogData(uint32_t now);
  virtual void init(RunMode mode);

  float getAmps() { return m_amps; };
  void setRate(uint32_t rate) {
    m_rate = rate;
  };

  int16_t readInputValue(uint32_t now);
  void processInputValue(uint32_t now);


private:

  void read_input_value();

  int16_t m_input_value;
  float m_amps;
  float m_old_amps;
  uint32_t m_input_min;
  uint32_t m_input_max;
  float m_output_min;
  float m_output_max;

  uint32_t m_rate; // In milliseconds
  uint32_t m_i2c_addr;
  uint32_t m_last_input_time;
  Adafruit_ADS1115 ads;

  void populate_log_buffer();
};

#endif
