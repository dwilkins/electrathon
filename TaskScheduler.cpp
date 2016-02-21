/*
 * TaskScheduler.cpp
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

#include "TaskScheduler.hpp"

void TaskScheduler::run() {
  {
    Task **tpp = tasks;
    Serial.print("millis");
    for (int t = 0; t < numTasks; t++) {
      Task *tp = *tpp;
      Serial.print("\t");
      Serial.print(tp->getLogHeader());
      tpp++;
    }

  }
  Serial.println("");
  while (1) {
    uint32_t now = millis();
    bool log_now = false;
    Task **tpp = tasks;
    for (int t = 0; t < numTasks; t++) {
      Task *tp = tasks[t];
      if (tp->canRun(now)) {
        tp->run(now);
      }
      log_now |= tp->canLog(now);
      //      tpp++;
    }
    if(log_now) {
      Serial.print(now);
      Serial.print("\t");
      for (int t = 0; t < numTasks; t++) {
        Task *tp = *tpp;
        Serial.print("\t");
        Serial.print(tp->getLogData(now));
        tpp++;
      }
      Serial.println("");
    }
  }
}

TaskScheduler::TaskScheduler(Task **_tasks, uint8_t _numTasks) :
  tasks(_tasks),
  numTasks(_numTasks) {
}
