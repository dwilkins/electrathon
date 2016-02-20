#include <EthernetServer.h>
#include <Dns.h>
#include <EthernetClient.h>
#include <Dhcp.h>
#include <EthernetUdp.h>
#include <Ethernet.h>

#include "Task.hpp"
#include "TaskScheduler.hpp"
#include "CurrentSense.hpp"

CurrentSense current_sense(0x19,1000);

Task *tasks[] = { &current_sense };

TaskScheduler sched(tasks,1);

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

  sched.run();

}
