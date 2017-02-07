/*
 * SpeedSense.cpp
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

#include "SpeedSense.hpp"

volatile uint16_t v_current_rpm = 0;
volatile uint32_t v_pulse_times[2] = {0,0};
volatile uint32_t v_current_rpm_pulse_micros = 0;
volatile uint32_t v_last_rpm_pulse_micros = 0;
volatile uint32_t v_last_rpm_pulse_millis = 0;
volatile uint32_t v_current_rpm_pulse_millis = 0;
void rpm_interrupt();


void SpeedSense::init(RunMode mode) {
  m_last_input_time = 0;
  if(mode == Task::RunMode::production) {
    attachInterrupt (digitalPinToInterrupt(m_pin), rpm_interrupt, FALLING);
  }
  populate_log_buffer();
  setLogTime(millis());
  Task::init(mode);
}

SpeedSense::~SpeedSense() {}


void SpeedSense::run(uint32_t now) {
  incRunTime(m_rate);
  m_rpm = readInputValue(now);
  processInputValue(now);
  populate_log_buffer();
}

void SpeedSense::populate_log_buffer() {
  static const char mph_str[] PROGMEM = "mph";
  static const char rpm_str[] PROGMEM = "rpm, ";
  strcpy(logBuffer,String(m_rpm).c_str());
  strcat_P(logBuffer,rpm_str);
  strcat(logBuffer,String(m_speed,2).c_str());
  strcat_P(logBuffer,mph_str);
}


const char *SpeedSense::getLogHeader() {
  return "rpm, speed";
}

void SpeedSense::processInputValue(uint32_t now) {
  m_speed = (m_rpm * 60.0) * m_wheel_circumference_in_miles;
}


int16_t SpeedSense::readInputValue(uint32_t now) {
  uint16_t rpm = 0;
  uint32_t last_interrupt_millis = 0;
  int16_t current_rpms = -1;
  if(runMode == Task::RunMode::production) {
    noInterrupts();
    last_interrupt_millis = v_last_rpm_pulse_millis;
    rpm = v_current_rpm;
    interrupts();
  } else if(runMode == Task::RunMode::test) {
    //
    // Storing the test values in PROGMEM saves some space
    // but makes you have to pull the values out by hand
    // rather than just accessing them like normal
    //
    // This speed test deck is setup so that the last known
    // RPM / MPH will stick - i.e. it won't go down when the
    // test data runs out.
    //
    static const PROGMEM uint32_t test_data[][2] = {
      {1,0},
      {10000,34},
      {20000,70},
      {30000,80},
      {40000,100},
      {45000,120},
      {50000,140},
      {55000,160},
      {60000,170},
      {65000,180},
      {70000,180},
      {75000,180},
      {85000,180}
    };
    for(int i = 0;i < sizeof(test_data) / sizeof(test_data[0]);i++) {
      uint32_t test_time = pgm_read_dword_near((uint16_t)&test_data[i][0]);
      uint32_t test_value = pgm_read_dword_near((uint16_t)&test_data[i][1]);
      if((m_last_input_time < test_time) && (now >= test_time)) {
        last_interrupt_millis = now;
        rpm = test_value;
        break;
      }
    }
  }
  if((now > last_interrupt_millis) && ((now - last_interrupt_millis) > MAX_PULSE_AGE_MILLIS)) {
    rpm = 0;
  }
  m_last_input_time = now;
  return rpm;
}

// Arduino micros() rolls over every 70 seconds (MAX_PULSE_AGE_MILLIS)
// Arduino millis() rolls over every 56 days (we won't worry 'bout this)
// If last interrupt was > MAX_PULSE_AGE_MILLIS ago, then ignore it
// subtraction of unsigned values works across a rollover of micros()
// so the pulse_time calculation below should work as long as just
// one rollover event has happened, hence the check for > MAX_PULSE_AGE_MILLIS
//
void rpm_interrupt() {
  v_current_rpm_pulse_millis = millis();
  if((v_current_rpm_pulse_millis - v_last_rpm_pulse_millis) > (MAX_PULSE_AGE_MILLIS)) {
    v_last_rpm_pulse_micros = v_current_rpm_pulse_micros = v_current_rpm = 0;
  } else {
    uint32_t pulse_time = 0;
    v_current_rpm_pulse_micros = micros();
    v_pulse_times[0] = v_pulse_times[1];
    v_pulse_times[1] = v_current_rpm_pulse_micros - v_last_rpm_pulse_micros;
    pulse_time = (v_pulse_times[0] + v_pulse_times[1]) / 2;
    if(v_last_rpm_pulse_micros != 0) {
      if(v_last_rpm_pulse_micros < v_current_rpm_pulse_micros) {
        v_current_rpm = (((1000UL * 1000UL * 60UL) / PULSES_PER_WHEEL_REVOLUTION)/(pulse_time));
      }
    }
  }
  v_last_rpm_pulse_micros = v_current_rpm_pulse_micros;
  v_last_rpm_pulse_millis = v_current_rpm_pulse_millis;
}
