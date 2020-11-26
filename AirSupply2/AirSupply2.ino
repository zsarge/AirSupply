//
//This sketch operates a fresh air vent. The fresh air resupplies the house from exhaust fans and combustion air.
//Arduino's heat call pin should be connected to Normally Open side of the furnace switch/relay.
//When there is a furnace call for heat, the furnace relay closes and heatCallReading will read HIGH.
//Arduino also opens the vent if there is a call from exhaust fans.
//
//Vent position is determined by three magnetic switches attached to the vent door. Signals are sent to Arduino.
//Vent door overtravel is prevented by two mechanical roller microswitches (one on each side) wired with the relays.
//
//The signal from the furnace is sent from a solenoid relay, which can send "dirty" signals that need debouncing.

//When first turned on, the arduino attempts to determine if the vent is open, closed, or in the middle



//vars
//pins
//const int otherCallPin = 4;      // digital pin number, input from non-furnace needs for air. Sets the ventCall value
const int heatCallPin = 5;      // digital pin number, input from heat call relay
const int maxOpenLimitSw = 6;   // max open limit switch. Switch (NC) opens when vent is open at max. LOW when vent is open, HIGH when closed.
const int maxClosedLimitSw = 7; // max closed limit switch. Switch (NC) opens when vent is closed at max. LOW when vent is closed, HIGH when open.
//const int midLimitSw =8;        // middle limit switch. Switch is closed when vent is close to the middle.
const int rlyOpenVent = 9;      // relay control to open the vent
const int rlyCloseVent = 10;    // relay control to close the vent


//values
//int ventReading;                 // stores the present actual state of the vent door 0 = damper closed  1 = damper midway  2 = damper full open
int heatCall = 0;            // Is there a call for heat?
//int ventCall = 0;            // Other than the furnace, is there something calling for an open vent?
//int otherCallReading = 0;        // present reading from otherCallPin
int heatCallReading = 0;             // present reading from heatCallPin
int lastHeatCallReading = 0;         // stores last reading from heatCallPin
//int maxOpenLimitSwReading;   // reading from open limit switch
//int maxClosedLimitSwReading; // reading from max closed limit switch
//int midLimitSwReading;       // reading from mid limit switch
//int firstTimeRun = true;     // tracks if this is the first time this has run (ie after a power outage)

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 10000;   // the debounce time; increase if the output flickers

void setup() {
  pinMode(heatCallPin, INPUT);      // HIGH when there is a call from the furnace for heat
  //  pinMode(otherCallPin, INPUT);     // Not yet used
  pinMode(maxOpenLimitSw, INPUT);   // LOW when vent is fully open, else HIGH
  pinMode(maxClosedLimitSw, INPUT); // LOW when vent is fully closed, else HIGH
  // pinMode(midLimitSw, INPUT);       // LOW when vent is half open, else HIGH


  pinMode(rlyOpenVent, OUTPUT);
  pinMode(rlyCloseVent, OUTPUT);

  digitalWrite(rlyCloseVent, LOW); // turn off relay if in case it is on for some reason
  digitalWrite(rlyOpenVent, LOW);  // turn off relay if in case it is on for some reason

  Serial.begin(9600);   // initialize serial port for debugging

  // The first time power is turned on, or when recovering from a power outage, check to see if the vent
  // is already open. If open, close it.

  while (digitalRead(maxClosedLimitSw) == HIGH) {
    digitalWrite(rlyCloseVent, HIGH);
  }

  digitalWrite(rlyCloseVent, LOW);  // makes sure that the relay is off
}

void handleHeatCall(int heatCall) {
  Serial.print ("heatCall after debounce = ");
  Serial.println (heatCall);
  delay(2000);

  // respond to heatcall

  if (heatCall == HIGH and digitalRead(maxClosedLimitSw) == LOW) {  //call for heat, vent is closed
    Serial.print ("vent started opening at ");
    Serial.println (millis());
    while (digitalRead(maxOpenLimitSw) == HIGH) {
      digitalWrite(rlyOpenVent, HIGH);  //open the vent
    }
    Serial.print ("vent is now opened at ");
    Serial.println (millis());
    digitalWrite(rlyOpenVent, LOW);  //stop opening the vent
  }

  if (heatCall == LOW and digitalRead(maxOpenLimitSw) == LOW) {     //no call for heat, vent is open
    Serial.print ("vent started closing at ");
    Serial.println (millis());
    while (digitalRead(maxClosedLimitSw) == HIGH) {
      digitalWrite(rlyCloseVent, HIGH);  //close the vent
    }
    Serial.print ("vent is now closed at ");
    Serial.println (millis());
    digitalWrite(rlyCloseVent, LOW);  //stop closing the vent
  }

}


void loop() {

  //int ventCall= digitalRead(otherCallPin);
  //int maxOpenLimitSwReading = digitalRead(maxOpenLimitSw);
  //int maxClosedLimitSwReading = digitalRead();
  //int midLimitSwReading = digitalRead();

  // check for heat call, debounce signal

  int heatCallReading = digitalRead(heatCallPin);
  if (heatCallReading != lastHeatCallReading) {       //if the pin's reading does not match previous reading, record the time
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) { //if the new reading has remained stable for long enough

    if (heatCallReading != heatCall) {                 //see if it matches the prevous status. If it's different,
      heatCall = heatCallReading;                      //then status has really changed. Reset the status.

      Serial.print ("HeatCall is now = ");
      Serial.println (heatCall);
      // delay(1000);

      // Code to be debounced:
      handleHeatCall(heatCall);
    }
  }

  lastHeatCallReading = heatCallReading;  // reset the reading to use as future comparison
}
