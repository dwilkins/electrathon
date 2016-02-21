/*
 * ThrottleSense.hpp
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


#ifndef THROTTLE_SENSE_HPP
#define THROTTLE_SENSE_HPP
#include "Task.hpp"

#define LOG_BUFFER_SIZE 256

class ThrottleSense : public TimedTask {
public:
  ThrottleSense(uint32_t pin,
                uint32_t rate = 200,
                uint32_t log_rate = 10000, // log every 10 seconds regardless
                uint32_t input_min = 0,
                uint32_t input_max = 1024,
                float output_min = 0.0,
                float output_max = 100.0) : TimedTask(millis()),
                                            m_pin(pin),
                                            m_rate(rate),
                                            m_input_min(input_min),
                                            m_input_max(input_max),
                                            m_output_min(output_min),
                                            m_output_max(output_max)
  {
    m_level = 0.0;
    m_old_level = 0.0;
    m_last_input_time = 0;
    // Do some setup stuff?
  }

  virtual void run(uint32_t now);
  virtual const char *getLogHeader();
  virtual void init(RunMode mode);

  float getLevel() { return m_level; };

  int16_t readInputValue(uint32_t now);
  void processInputValue(uint32_t now);


private:
  float m_level;
  int16_t m_input_level;
  uint32_t m_old_level; // 0 - 100???
  uint32_t m_rate; // In milliseconds
  uint32_t m_pin;
  uint32_t m_input_min;
  uint32_t m_input_max;
  float m_output_min;
  float m_output_max;

  uint32_t m_last_input_time;
};

#endif
