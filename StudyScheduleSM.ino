
//Sham on tues, thurs, sat


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
unsigned long pumpTimeout = 120000; //estimated inflation time inside pig's stomach with clamp at level 4


int startup_duration = ceil((30*(intervals * 2) + valveOpenTime + pumpTimeout + 1000)/1000); // about 138 seconds

const int valve_h_1 = 3;
const int valve_h_2 = 4;

const int sensor_power = 22;
const int pump_enable = 23;
const int indicator = 13;


enum week {mon, tues, wed, thurs, fri, sat, sun};


////////////////////////////////////////////////////////////////////////////////////////////////////
//Insert the time at which we are starting the experiment in the form [hr, min] and in military time
//Ex: 15:30 -> 3:30 PM
int start_time[2] = {16, 45};// toggle switch time
int next_time[2] = {8, 30};  // time to start first inflation
int current_day = sat; //First day of inflation after setup, starts at 8:30 AM
////////////////////////////////////////////////////////////////////////////////////////////////////
int state;


enum state {
  init_delay,
  fork, // select between experiment and rest days, should be 8:30
  rest, // sleep for 24 hours, 8:30 the next day
  inflation1, // start inflation at 8:30
  stall, // delay until 1:30
  inflation2, // inflate again at 1:30
  compensate // sleep until 8:30 the next day
};


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  delay(100); //wait for comms
  pinMode(valve_h_1, OUTPUT); //valve pin hbridge 1 part
  pinMode(valve_h_2, OUTPUT); //valve pin hbridge 2 part
  pinMode(pump_enable, OUTPUT); //pump enable pin
  pinMode(sensor_power, OUTPUT);//pin 22 is power out for the sensor
  pinMode(indicator, OUTPUT);
  pressureSensor.beginI2C(); //setting up pressure sensor wiring


  analogWrite(pump_enable, 255);
  delay(2000);
  analogWrite(pump_enable, 0);
  digitalWrite(indicator, HIGH);
    
  state = init_delay;
}

void loop() {
  // put your main code here, to run repeatedly:

  stateMachine();
}


void stateMachine(){
  printDay(current_day);

  if (state == init_delay){
    int initial_delay = timeDiff(start_time, next_time);
    Serial.print("Initial Delay: ");

    Serial.println(initial_delay);
    while (initial_delay != 0){
      if(initial_delay >= 60){
        initial_delay -=60;
        //wakeSleep();
      }else{
        //delayRTC(0, initial_delay, 0);
        initial_delay = 0;
      }
    }
    state = fork;
    Serial.println("Its 8:30");
    
  }else if (state == fork){
    Serial.println("Fork");

    if (current_day == tues || current_day == thurs || current_day == sat){
      state = stall;
    }else{
      state = inflation1;
    }    
  }else if (state == inflation1){
    Serial.println("Inflation 1");
    fullBalloon();
    //delayRTC(0,30,0); // Hold air for 30 minutes
    emptyBalloon();
    Serial.println("Its 9:00 on an inflation day");
    state = stall;
  }else if (state == stall){
    
    Serial.println("Stall");
    int t1[2] = {0,0};
    int t2[2] = {0,0};
    if (current_day == tues || current_day == thurs || current_day == sat){
      Serial.println("Its 8:30 on a sham day");
      t1[0] = 8;
      t1[1] = 30;

      t2[0] = 14;
      t2[1] = 00;
    

      
    }else{
      Serial.println("Its 9:02 on a double inflation day");
      t1[0] = 9;
      t1[1] = 2;


      t2[0] = 13;
      t2[1] = 30;
    }
        
    
    int interm_delay = timeDiff(t1, t2); // set up delay between 9:32 AM and 1:30 PM, or 8:30 and 1:30 PM if there was no first inflation
    
    Serial.println("STALL TIME");
    Serial.println(interm_delay);
    
      while (interm_delay != 0){
        if (interm_delay >= 60){
          interm_delay -= 60;      
          //wakeSleep();
        }
        else{
          // only minutes left
          //delayRTC(0, interm_delay,0);
          interm_delay = 0;
        }
      }

    if (current_day == tues || current_day == thurs || current_day == sat){
      state = compensate;
      Serial.println("From stall to compensate");
    }
    else{
      state = inflation2;
      Serial.println("From stall to inflation 2");
    }
  }else if (state == inflation2){
    Serial.println("Inflation 2 at 1:30 PM");
    fullBalloon();
    //delayRTC(0,30,0); // hold for 30 minutes now
    emptyBalloon();    
    state = compensate;
  //inflation now ends at 2 instead of 2:30, stall another half around until 8:30 the next day
  }else if (state == compensate){
    Serial.println("Compensate, both now at 2:00");
    for (int i = 0; i < 18; i++){
      Serial.print("+");
      //wakeSleep();
    }
    //delayRTC(0,30,0);
    current_day = (current_day+1)%7;
    Serial.println("_______________8:30 the next day_____________________");
    if(current_day == 0){
      Serial.println("New Week");
    }
    state = fork;   
  }
}
