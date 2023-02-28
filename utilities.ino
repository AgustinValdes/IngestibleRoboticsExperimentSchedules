void fullBalloon() {
  // closing the valve
  for (int i = 0; i <= 30; i++ ) {
    valveBack();//make it like a pause
    delay(intervals);
    valveOff();
    delay(intervals);
  }
  //valve closed
  pump(pumpSpeed); //
  //Serial.println("Conclude pumping, now sleeping");
  delay(2000);
}
void emptyBalloon() {
  //  Serial.println("Begin emptying");
  valveFor();
  delay(valveOpenTime);
  valveOff();
  //  Serial.println("Conclude emptying, now sleeping");
}

void pump(int val) {
  unsigned long timer = millis(); //get the time when pump starts
  //  Serial.println("Pump Started");
  analogWrite(pump_enable, val);
  int sval = 0; //the moving average
  List<int> bucket; //the bucket to get the moving average
  //  Serial.println(bucket.getSize());
  while (sval < sThreshold && (millis() - timer < pumpTimeout)) {
    //keep pumping and reading sensor values if the moving average doesn't exeed the threshold 
    // and the pumping time doesn't reach pumpTimeout 
      
    int nval = pressureSensor.readFloatPressure(); //get pressure value from the sensor
    
    if (!(nval > 130000 or nval < 80000)) {
      //if the sensor value isn't an outlier (>1300000 or <80000) then take it into the average calculation
      if (bucket.getSize() >= 10) { 
        //keep the bucket size to be 10
        bucket.removeFirst();
      }
      bucket.add(nval);
    }
    
    int sum = 0;
    int bucketSize = bucket.getSize();
    for (int i = 0; i < bucketSize; i++) {
      //sum over the bucket
      sum += bucket[i];
    }
    sval = sum / bucketSize; 
    
    delay(1000);
  }
  analogWrite(pump_enable, 0);
}
void valveFor() { //open to empty balloon
  analogWrite(valve_h_1, valveSwitchingSpeed);
  analogWrite(valve_h_2, 0);
}
void valveBack() { //close to let pump
  analogWrite(valve_h_2, valveSwitchingSpeed);
  analogWrite(valve_h_1, 0);
}
void valveOff() {
  analogWrite(valve_h_1, 0);
  analogWrite(valve_h_2, 0);
}
void delayRTC(int hr, int mi, int sc) {
  //let the device snooze for (hr, mi, sc)
  alarm.setRtcTimer(hr, mi, sc);
  Snooze.hibernate(my_config); //let device snooze
}

void wakeSleep(){
  /* after the board wakes from the last cycle, let it stay awake for 10 seconds <- why? 
   *  and go to snooze again. The whole duration is 1 hr.
  */
  // need to wake up every hour; sleeping for over 2 hours may not work
  delay(10000);
  delayRTC(0,59,50);
}

// Given two times in military time, return the number of minutes between time a (initial) and b (final)
// Example input: [15,30], [1,20] -> 9hrs 50 mins
int timeDiff(int a[2], int b[2]){
  int fill_hrs;
  int fill_mins = 60- a[1];
  if (a[0] > b[0]){
    fill_hrs = 24 - a[0] - 1 + b[0];
  }
  else{
    fill_hrs = b[0] - a[0] - 1; // [10:30], [8:30]
        
  }
  return fill_mins + 60*(fill_hrs) + b[1];  
  
}


void printDay(int d){
  //For debugging
  
  String out = "";

  switch(d){
    case 0:
      out = "Monday";
      break;
    case 1:
      out = "Tuesday";
      break;
    case 2:
      out = "Wednesday";
      break;
    case 3:
      out = "Thursday";
      break;
    case 4:
      out = "Friday";
      break;
    case 5:
      out = "Saturday";
      break;
    case 6:
      out = "Sunday";
      break; 
    default:
      out = "Whoops something went wrongn with current_day";
  
  }
  

 // Serial.println(out);
}
