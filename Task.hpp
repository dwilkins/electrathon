/*
 * Task.hpp
 *
 * (C) Copyright 2016
 * David H. Wilkins  <dwilkins@conecuh.com>
 * Portions derived from Alan Burlison
 * <http://bleaklow.com/2010/07/20/a_very_simple_arduino_task_manager.html>
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

#ifndef TASK_HPP
#define TASK_HPP

#include <Arduino.h>
#define TASK_LOG_BUFFER_SIZE 100

class Task {
public:
  inline Task(uint32_t _logRate = 10000) { logRate = _logRate; }
  enum RunMode { test,production };

  virtual bool canRun(uint32_t now) = 0;
  virtual void run(uint32_t now) = 0;

  // Logging
  virtual bool canLog(uint32_t now);
  inline void setLogTime(uint32_t when) { logTime = when; }
  inline void incLogTime(uint32_t inc) { logTime += inc; }
  inline uint32_t getLogTime() { return logTime; }
  inline uint32_t getLogRate() { return logRate; }

  virtual const char *getLogHeader() = 0;
  virtual char *getLogData(uint32_t now);

  virtual void init(RunMode mode) {setRunMode(mode);};
  virtual void setRunMode(RunMode mode) { runMode = mode;}

  uint32_t logRate;
protected:
  char logBuffer[TASK_LOG_BUFFER_SIZE];
  uint32_t logTime;
  RunMode runMode;
};

class TimedTask : public Task {
public:
    inline TimedTask(uint32_t when) { runTime = when; }
    virtual bool canRun(uint32_t now);
    inline void setRunTime(uint32_t when) { runTime = when; }
    inline void incRunTime(uint32_t inc) { runTime += inc; }
    inline uint32_t getRunTime() { return runTime; }
protected:
    uint32_t runTime;
};

#endif
