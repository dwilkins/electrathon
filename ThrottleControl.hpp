/*
 * ThrottleControl.hpp
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


#include "Task.hpp"

#define LOG_BUFFER_SIZE 256

class ThrottleControl : public TimedTask {
public:
  ThrottleControl(int i2c_addr, uint32_t rate) : TimedTask(millis()),
                                              m_rate(rate),
                                              m_i2c_addr(i2c_addr){
    // Do some setup stuff?
  }

  void init();

  virtual void run(uint32_t now);
  virtual bool canRun(uint32_t now);


  float getAmps(uint32_t now);
  void setLevel(uint32_t rate) {
    m_old_rate = m_rate;
    m_rate = rate;
  };
  char *getLoggingData(uint32_t now);

private:
  float m_level;
  uint32_t m_old_level; // 0 - 180???
  uint32_t m_rate; // In milliseconds
  uint32_t m_i2c_addr;
  char m_log_buffer[LOG_BUFFER_SIZE];
};
