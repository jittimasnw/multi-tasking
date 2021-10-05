
#include <PubSubClient.h>  
#include <WiFiClientSecure.h>
#include <WiFi.h>   

// shared variable
TaskHandle_t t0;
TaskHandle_t t1;

const char* ssid = "";       
const char* wifi_password = "";

const char* mqtt_server = "";  
const char* mqtt_username = ""; 
const char* mqtt_password = "";  
const char* clientID = "";  

WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883,wifiClient); 

long lastReconnectAttempt = 0;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;


void setup() {
  
 pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

    //create a task to handle Mqtt (core 0)
  xTaskCreatePinnedToCore(
    tstate,        /* Task function. */
    "State",       /* name of task. */
    10000,        /* Stack size of task */
    NULL,         /* parameter of the task */
    1,            /* priority of the task */
    &t0,          /* Task handle to keep track of created task */
    1);           /* pin task to core 1 */
  delay(500);

  // create a task to handle led LED_BUILTIN (core 1)
  xTaskCreatePinnedToCore(
    tLedFunc,     /* Task function. */
    "Led",        /* name of task. */
    10000,        /* Stack size of task */
    NULL,         /* parameter of the task */
    1,            /* priority of the task */
    &t1,          /* Task handle to keep track of created task */
    0);           /* pin task to core 0 */
  delay(500);
}


void loop() {
  
  Serial.print("loop running on core ");
  Serial.println(xPortGetCoreID());
  delay(1000);

}

//setup_wifi
void tWifi(){    
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, wifi_password);

  
 while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);   
    //led turn off when can't connect Wifi access point
    Serial.println("turn off led when cannot connect Wifi");
    delay(500);
    Serial.print(".");
 }
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
   //connect MQTT Broker
   if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("set up on loop core1 : Connected to MQTT Broker!");
  }
  else {
    Serial.println("set up on loop core1 : Connection to MQTT Broker failed...");
  }
}
  

boolean reconnect() {
  Serial.println("reconnect active");
  if (!client.connected() ) {
    Serial.println("connecting...");
    if (client.connect(clientID, mqtt_username, mqtt_password)) {
      Serial.println("Connected MQTT broker waiting publish");
      
      client.subscribe("#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" Try again in 5sec");
    }
  }
  return client.connected();
}


void tstate(void *params){
  // local Variables
  bool lastState = false;
  
  Serial.print("tButtonFunc running on core ");
  Serial.println(xPortGetCoreID());

  // loop
  while (true) {
    
    if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH); 
    Serial.println("turn off led when cannot connect Wifi");
    delay(1000);
   }  
    if(WiFi.status() == WL_CONNECTED){
    Serial.println("loop core0 : Connect WIFI, Turn on led 500 ms ");  
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.println("loop core0 : Connect WIFI, Turn off led 500 ms ");  
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500); 
    }
    else if(client.connected()){
    digitalWrite(LED_BUILTIN, LOW);  
    Serial.println("loop core0 : Connect MQTT, Turn on led ");
    }

  } 
}
  

// tLedFunc: blinks every 1000ms
void tLedFunc(void *params) {
  // local variable
  
  // setup  
  Serial.print("tLedFunc running on core ");
  Serial.println(xPortGetCoreID());
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // connect WiFi
  tWifi();
  
  // Connect MQTT Broker
  //loop
  while (true) {
    if (!client.connected()) {
      
    long now = millis();
    
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      Serial.println("loop core 1 : Counting.. ,not connect");
      Serial.println("Counter is reset : 0");
      Serial.print("Counter Change to : ");
      Serial.println(lastReconnectAttempt);
      // try to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
        Serial.println("loop core 1 : Reconnecting..., turn off LED: ");    // -- led turn off when  can't connect Wifi Access point
      }
    }
  }
    else if (client.connected()) {
    client.loop();
    Serial.println("loop core1 : Connected...");
    Serial.print("IP address:  ");
    Serial.println(WiFi.localIP()); 

  //  Serial.println("loop core1 : Connect MQTT, Turn on led ");          
     client.disconnect();
     delay(50);
  }
  delay(1000);
  }
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Serial.println("loop core1 : led swap");
    delay(1000);
}
