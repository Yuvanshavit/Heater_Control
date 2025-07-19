#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into GPIO 4
#define ONE_WIRE_BUS 4
#define HEATER_PIN 2
#define STATUS_LED 13

// Temperature thresholds in Celsius
#define TARGET_TEMP 45
#define STABILIZING_MARGIN 2
#define OVERHEAT_TEMP 60
#define INTERVAL 1000  // ms

enum HeaterState {
  IDLE,
  HEATING,
  STABILIZING,
  TARGET_REACHED,
  OVERHEAT
};

HeaterState currentState = IDLE;

unsigned long lastMillis = 0;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float currentTemp = 0;

void setup() {
  Serial.begin(115200);
  sensors.begin();
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  digitalWrite(STATUS_LED, LOW);
  Serial.println("Heater Control System Initialized");
}

void loop() {
  if (millis() - lastMillis >= INTERVAL) {
    lastMillis = millis();
    sensors.requestTemperatures();
    currentTemp = sensors.getTempCByIndex(0);

    updateState(currentTemp);
    actOnState(currentState);
    logStatus(currentTemp, currentState);
  }
}

void updateState(float temp) {
  switch (currentState) {
    case IDLE:
      if (temp < TARGET_TEMP - STABILIZING_MARGIN) {
        currentState = HEATING;
      }
      break;
    case HEATING:
      if (temp >= TARGET_TEMP - STABILIZING_MARGIN && temp < TARGET_TEMP + STABILIZING_MARGIN) {
        currentState = STABILIZING;
      } else if (temp >= OVERHEAT_TEMP) {
        currentState = OVERHEAT;
      }
      break;
    case STABILIZING:
      if (temp >= TARGET_TEMP && temp < OVERHEAT_TEMP) {
        currentState = TARGET_REACHED;
      } else if (temp >= OVERHEAT_TEMP) {
        currentState = OVERHEAT;
      }
      break;
    case TARGET_REACHED:
      if (temp >= OVERHEAT_TEMP) {
        currentState = OVERHEAT;
      } else if (temp < TARGET_TEMP - STABILIZING_MARGIN) {
        currentState = HEATING;
      }
      break;
    case OVERHEAT:
      if (temp < TARGET_TEMP - 5) {
        currentState = HEATING;
      }
      break;
  }
}

void actOnState(HeaterState state) {
  switch (state) {
    case HEATING:
      digitalWrite(HEATER_PIN, HIGH);
      digitalWrite(STATUS_LED, HIGH);
      break;
    case STABILIZING:
      digitalWrite(HEATER_PIN, HIGH);
      digitalWrite(STATUS_LED, LOW);
      break;
    case TARGET_REACHED:
    case IDLE:
      digitalWrite(HEATER_PIN, LOW);
      digitalWrite(STATUS_LED, LOW);
      break;
    case OVERHEAT:
      digitalWrite(HEATER_PIN, LOW);
      digitalWrite(STATUS_LED, HIGH); // constant ON for alert
      break;
  }
}

void logStatus(float temp, HeaterState state) {
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" C | TEMP State: ");
  
  // switch (state) {
  //   case IDLE: Serial.println("IDLE"); break;
  //   case HEATING: Serial.println("HEATING"); break;
  //   case STABILIZING: Serial.println("STABILIZING"); break;
  //   case TARGET_REACHED: Serial.println("TARGET REACHED"); break;
  //   case OVERHEAT: Serial.println("OVERHEAT!"); break;
  // }
  if (state == IDLE ){
    Serial.print("IDLE");
    Serial.println(" | HEATER State: ON");
  }
  else if(state == HEATING){
    Serial.print("HEATING");
    Serial.println(" | HEATER State: ON");
  }
  else if(state == STABILIZING){
    Serial.print("STABILIZING");
    Serial.println(" | HEATER State: ON");
  }
  else if(state == TARGET_REACHED){
    Serial.print("TARGET REACHED");
    Serial.println(" | HEATER State: OFF");
  }
  else if(state == OVERHEAT){
    Serial.print("OVERHEAT");
    Serial.println(" | HEATER State: OFF");
  }
}
