//************************************************************
// Motion Sensor
//
//
//************************************************************
#include "painlessMesh.h"

#define   MESH_PREFIX     "nazwaMesha"
#define   MESH_PASSWORD   "hasłoMesha"
#define   MESH_PORT       5555
#define timeSeconds 5

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
int motionSensor = 2;
int motionSensorId = 1;

// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastSend = 0;

// User stub
void sendMessage();
void sendValue();

// Checks if motion was detected, sets LED HIGH and starts a timer
ICACHE_RAM_ATTR void detectsMovement() {
  Serial.println("MOTION DETECTED!!!");
  sendValue();
}




Task taskSendMessage( TASK_MILLISECOND * 500 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  sendValue();
}

void sendValue() {
  // Current time
  now = millis();
  // Turn off the LED after the number of seconds defined in the timeSeconds variable
  if (digitalRead(motionSensor) == HIGH) {
    if (now - lastSend > (timeSeconds * 1000)) {
      Serial.printf("--> send \n");
      DynamicJsonDocument obj(1024);
      obj["objectId"] = motionSensorId;
      obj["type"] = "read-value";
      obj["value"] = 1;
      String  sPayload = "";
      serializeJson(obj, sPayload);
      char* cPayload = &sPayload[0u];
      mesh.sendBroadcast( cPayload );
      lastSend = millis();
    }
  }
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  StaticJsonDocument<256> doc;
  deserializeJson(doc, msg);
  if (doc["objectId"] == motionSensorId) {
    if (doc["type"] == "read") {
      Serial.printf("Read value");
      sendValue();
    }
    else if (doc["type"] == "change") {
      Serial.printf("Unsupported action type");
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
  pinMode(motionSensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 5 );
  mesh.onReceive(&receivedCallback);
  mesh.setContainsRoot(true);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
}
