/*
 * GpsSense.hpp
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


#ifndef GPS_SENSE_HPP
#define GPS_SENSE_HPP
#include "Task.hpp"
#include "TinyGPS++.h"


class GpsSense : public TimedTask {
public:
  GpsSense(uint32_t pin,
           uint32_t rate = 100,
           uint32_t log_rate = 1000 // every 1 second regardless
           ) : TimedTask(millis()),
                 m_pin(pin),
                 m_rate(rate)
  {
    logRate = log_rate;
  }
  virtual ~GpsSense();

  virtual void run(uint32_t now);
  virtual const char *getLogHeader();
  virtual void init(RunMode mode);

  void setRate(uint32_t rate) {
    m_rate = rate;
  };

  int16_t readInputValue(uint32_t now);
  void processInputValue(uint32_t now);


private:
  char m_gps_data[128];
  TinyGPSPlus m_gps;
  void read_input_value();
  int16_t m_pin;
  uint32_t m_rate; // In milliseconds
  uint32_t m_last_input_time;
  void populate_log_buffer();
};

#endif
