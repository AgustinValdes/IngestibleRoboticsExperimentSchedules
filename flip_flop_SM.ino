//Sham day every other day, alternating with inflation days

#include <List.hpp>

// For pressure sensor
#include <Wire.h>
#include "SparkFunBME280.h"
BME280 pressureSensor;

// For snoozing
#include <Snooze.h>
SnoozeAlarm alarm;
SnoozeBlock my_config(alarm);
int valveSwitchingSpeed = 255; //Change valve speed (0-255)
int valveOpenTime = 4810; //the time it takes to open the valve(ms)
int intervals = 200; //ms (*30) short intervals used when closing the valve
unsigned long sThreshold = 103000; //Threshold at which the pump stops pumping based on sensor reading
int pumpSpeed = 255; //Speed the balloon is pumped up
//int pumpTimeout = 91000; //inflation time at clamp level 4

unsigned long pumpTimeout = 60000; // TODO change sleep amts dynamically based on pumpTimeout - right now 1 minute is hard coded in


int startup_duration = ceil((30*(intervals * 2) + valveOpenTime + pumpTimeout + 1000)/1000); // about 138 seconds

const int valve_h_1 = 3;
const int valve_h_2 = 4;

const int sensor_power = 22;
const int pump_enable = 23;
const int indicator = 13;




////////////////////////////////////////////////////////////////////////////////////////////////////
//Insert the time at which we are starting the experiment in the form [hr, min] and in military time
//Ex: 15:30 -> 3:30 PM
int switch_time[2] = {10, 20};// toggle switch time
int start_time[2] = {8, 30};  // time to start first inflation
bool sham = false; // initalize to whatever it will be post delay i.e. the next day (Saturday)
bool valve_init_closed = false;
int experiment_days = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////
int state;
enum state {
  init_delay,
  fork, // select between experiment and rest days, should occur at 8:30
  rest, // sleep for 24 hours, 8:30 the next day
  inflation1, // start inflation at 8:30
  interm_delay, // delay until 1:30
  inflation2, // inflate again at 1:30
  compensate, // sleep until 8:30 the next day
  full_rest // 24 hour sleep period, sham day
};

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  delay(100); //wait for comms
  pinMode(valve_h_1, OUTPUT); //valve pin hbridge 1 part
  pinMode(valve_h_2, OUTPUT); //valve pin hbridge 2 part
  pinMode(pump_enable, OUTPUT); //pump enable pin
  pinMode(sensor_power, OUTPUT);//pin 22 is power out for the sensor
  pinMode(indicator, OUTPUT);
  pressureSensor.beginI2C(); //setting up pressure sensor wiring

  analogWrite(pump_enable, 255);
  delay(500);
  analogWrite(pump_enable, 0);
  
  
  digitalWrite(indicator, HIGH);

  
  state = init_delay;
}

void loop() {
  while(experiment_days<=3){
    flipFlopSM();
  }
}

void flipFlopSM(){
  if (state == init_delay){
   // delay for however long until 8:30 the next day 
    //Serial.println("Initial delay");
    int initial_delay = timeDiff(switch_time, start_time);
    while (initial_delay != 0){
      if(initial_delay >= 60){
        initial_delay -=60;
        wakeSleep();
      }else{
        delayRTC(0, initial_delay, 0); // only minutes left to delay by
        initial_delay = 0;
      }
    }
    state = fork;
  }
  else if(state == fork){
    //Serial.println("___________________________________________");
    //Serial.println("Fork");

    if(!sham){
      state = inflation1;
      
    }else{
      state = full_rest;
    }
  }
  else if(state == inflation1){
    //Serial.println("Inflation 1");

    fullBalloon();
    delayRTC(0,30,0);
    emptyBalloon();
    state = interm_delay;
  }
  else if(state == interm_delay){
   // Serial.println("Intermediate delay");

    // Assume held balloon until 9:01, now sleep until 13:30 - 4 hrs, 29 mins
    for (int i = 0; i<4; i++){
      wakeSleep();
    }
    delayRTC(0,29,0);
    state = inflation2;
  }
  else if(state == inflation2){
//    Serial.println("Inflation 2");

    fullBalloon();
    delayRTC(0,30,0);
    emptyBalloon();
    state = compensate;
  }
  else if(state == compensate){
   // Serial.println("Compensate");

    // Assume held balloon until 2:01, now sleep until 8:30 the next day - 18 hrs,  29 mins
    for(int i = 0; i<18; i++){
      //Serial.print("+");
      wakeSleep();
    }
    delayRTC(0,29,0);
    experiment_days++; // one more out of the three days to inflate
    sham = !sham;
    state = fork;
  }
  else if(state == full_rest){
      //Serial.println("Full rest");

    for(int i = 0; i<24; i++){
      wakeSleep();
      //Serial.print("*");

    }
    sham = !sham;
    state = fork;
  }
}
