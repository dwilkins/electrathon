#include "Task.hpp"
#include "TaskScheduler.hpp"
#include "CurrentSense.hpp"
#include "ThrottleSense.hpp"
#include "SpeedSense.hpp"
#include "MotorControl.hpp"


CurrentSense current_sense(0x19,100);

ThrottleSense throttle_sense(0x18,100);

SpeedSense speed_sense(2,62.8);

MotorControl motor_control(0x20,&current_sense,&throttle_sense,&speed_sense);

Task *tasks[] = { &current_sense, &throttle_sense, &speed_sense, &motor_control };

TaskScheduler sched(tasks,4);

void setup() {

  Task::RunMode modetest = Task::RunMode::test;
  Task::RunMode modeprod = Task::RunMode::production;

  Serial.begin(57600);
  Serial1.begin(57600);
  Serial.println("Electrathon Brushed Motor");

  current_sense.init(modetest);
  throttle_sense.init(modetest);
  speed_sense.init(modetest);
  motor_control.init(modetest);
  delay(500);
}


void loop() {
  // put your setup code here, to run once:
  // put your main code here, to run repeatedly:
  sched.run();
}
