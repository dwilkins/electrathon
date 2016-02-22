#include "Task.hpp"
#include "TaskScheduler.hpp"
#include "CurrentSense.hpp"
#include "ThrottleSense.hpp"
#include "MotorControl.hpp"
CurrentSense current_sense(0x19,100);
ThrottleSense throttle_sense(0x18,100);
MotorControl motor_control(0x20,&current_sense,&throttle_sense);
Task *tasks[] = { &current_sense, &throttle_sense, &motor_control };

TaskScheduler sched(tasks,3);

void setup() {
  Task::RunMode mode = Task::RunMode::test;

  Serial.begin(38400);
  current_sense.init(mode);
  throttle_sense.init(mode);
  motor_control.init(mode);

}

void loop() {
  // put your setup code here, to run once:
  // put your main code here, to run repeatedly:
  sched.run();
}
