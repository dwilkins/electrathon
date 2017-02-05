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
  printHeaders(0);
  while (1) {
    uint32_t now = millis();
    bool log_now = false;
    for (int t = 0; t < numTasks; t++) {
      if (tasks[t]->canRun(now)) {
        tasks[t]->run(now);
      }
      log_now |= tasks[t]->canLog(now);
    }
    if(log_now) {
      int hours = now / (1000UL*60UL*60UL);
      int minutes = now / (1000UL*60UL);
      int seconds = ((now / 1000UL) % 60UL);
      char time_buffer[10];
      char millis_buffer[12];
      printHeaders(now);
      sprintf(time_buffer,"%d:%02d:%02d",hours,minutes,seconds);
      sprintf(millis_buffer,"%7lu,",now);
      Serial1.print(millis_buffer);
      Serial.print(millis_buffer);
      Serial1.print(time_buffer);
      Serial.print(time_buffer);
      for (int t = 0; t < numTasks; t++) {
        Serial1.print(", ");
        Serial1.print(tasks[t]->getLogData(now));
        Serial.print(", ");
        Serial.print(tasks[t]->getLogData(now));
      }
      Serial1.println("");
      Serial.println("");
    }
  }
}

TaskScheduler::TaskScheduler(Task **_tasks, uint8_t _numTasks) :
  tasks(_tasks),
  numTasks(_numTasks) {
  m_lastHeaderOutputMillis = millis();
}

void TaskScheduler::printHeaders(uint32_t now) {
  if(((now - m_lastHeaderOutputMillis) > HEADER_FREQENCY_MILLIS) ||
     m_lastHeaderOutputMillis == 0
     ){
    Serial1.print("millis,time");
    Serial.print("millis,time");
    for (int t = 0; t < numTasks; t++) {
      Serial1.print(",");
      Serial1.print(tasks[t]->getLogHeader());
      Serial.print(",");
      Serial.print(tasks[t]->getLogHeader());
    }
  Serial1.println("");
  Serial.println("");
  m_lastHeaderOutputMillis = now;
  }
}
