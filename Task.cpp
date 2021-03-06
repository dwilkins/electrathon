/*
 * Task.cpp
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

#include "Task.hpp"

bool TimedTask::canRun(uint32_t now) {
  return (now >= runTime);
}

bool Task::canLog(uint32_t now) {
  return (now >= logTime);
}

char *Task::getLogData(uint32_t now) {
  setLogTime(now + logRate);
  return logBuffer;
}
