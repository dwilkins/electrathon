#include "Task.hpp"
#include "TaskScheduler.hpp"
#include "CurrentSense.hpp"
#include "ThrottleSense.hpp"

CurrentSense current_sense(0x19,200);
ThrottleSense throttle_sense(0x19,500);

Task *tasks[] = { &current_sense, &throttle_sense };

TaskScheduler sched(tasks,2);

void setup() {
  Serial.begin(38400);
    Task::RunMode mode = Task::RunMode::test;
  // put your setup code here, to run once:
  current_sense.init(mode);
  throttle_sense.init(mode);
}

void loop() {
  // put your main code here, to run repeatedly:
  sched.run();
}
