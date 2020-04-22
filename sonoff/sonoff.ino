//************************************************************
// Sonoff basic
//
//************************************************************
#include "painlessMesh.h"

#define   MESH_PREFIX     "nazwaMesha"
#define   MESH_PASSWORD   "hasłoMesha"
#define   MESH_PORT       5555
#define timeSeconds 5

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
int relay = 12;
int led = 13;
int switchId = 1;
int value = 0;

// Timer: Auxiliary variables

// User stub
void sendValue();

// Checks if motion was detected, sets LED HIGH and starts a timer
ICACHE_RAM_ATTR void detectsMovement() {
  Serial.println("MOTION DETECTED!!!");
  if (value == 0) {
    value = 1;
  }
  else {
    value = 0;
  }
  digitalWrite(led, value);
  digitalWrite(relay, value);
  sendValue();
}


void sendValue() {

  Serial.printf("--> send value  \n");
  DynamicJsonDocument obj(1024);
  obj["objectId"] = switchId;
  obj["type"] = "read-value";
  obj["value"] = value;
  String  sPayload = "";
  serializeJson(obj, sPayload);
  char* cPayload = &sPayload[0u];
  mesh.sendBroadcast( cPayload );
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  StaticJsonDocument<256> doc;
  deserializeJson(doc, msg);
  if (doc["objectId"] == switchId) {
    if (doc["type"] == "read") {
      Serial.printf("Read value");
      sendValue();
    }
    else if (doc["type"] == "change") {
      value = doc["value"];
      digitalWrite(led, value);
      digitalWrite(relay, value);
      sendValue();
    }
  }
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(0, INPUT);
  digitalWrite(led, value);
  digitalWrite(relay, value);
  attachInterrupt(digitalPinToInterrupt(0), detectsMovement, FALLING);
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 5 );
  mesh.onReceive(&receivedCallback);
  mesh.setContainsRoot(true);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
}
