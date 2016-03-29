#include "Task.hpp"
#include "TaskScheduler.hpp"
#include "CurrentSense.hpp"
#include "ThrottleSense.hpp"
#include "SpeedSense.hpp"
#include "MotorControl.hpp"
#include "GpsSense.hpp"

CurrentSense current_sense(0x19,100);

ThrottleSense throttle_sense(0x18,100);

SpeedSense speed_sense(2,62.8);
GpsSense gps_sense(2);

MotorControl motor_control(0x20,&current_sense,&throttle_sense,&speed_sense);

Task *tasks[] = { &current_sense, &throttle_sense, &speed_sense, &gps_sense, &motor_control };

TaskScheduler sched(tasks,5);

void setup() {

  Task::RunMode modetest = Task::RunMode::test;
  Task::RunMode modeprod = Task::RunMode::production;

  Serial.begin(57600);
  Serial1.begin(57600);
  Serial.println("Electrathon Brushed Motor");

  current_sense.init(modeprod);
  throttle_sense.init(modeprod);
  speed_sense.init(modeprod);
  motor_control.init(modeprod);
  delay(500);
}


void loop() {
  // put your setup code here, to run once:
  // put your main code here, to run repeatedly:
  sched.run();
}
