
#include <PubSubClient.h>  
#include <WiFiClientSecure.h>
#include <WiFi.h>   

// shared variable
TaskHandle_t t0;
TaskHandle_t t1;
TaskHandle_t t2;

const char* mqtt_server = "";  //IP the mqtt broker
const char* mqtt_username = ""; 
const char* mqtt_password = "";  
const char* clientID = "";  


void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  //create a task to handle Wifi (core 0)
  xTaskCreatePinnedToCore(
    tWifi,        /* Task function. */
    "Wifi",       /* name of task. */
    10000,        /* Stack size of task */
    NULL,         /* parameter of the task */
    1,            /* priority of the task */
    &t0,          /* Task handle to keep track of created task */
    1);           /* pin task to core 0 */
  delay(500);

    //create a task to handle Mqtt (core 0)
  xTaskCreatePinnedToCore(
    tMqtt,        /* Task function. */
    "Mqtt",       /* name of task. */
    10000,        /* Stack size of task */
    NULL,         /* parameter of the task */
    1,            /* priority of the task */
    &t1,          /* Task handle to keep track of created task */
    1);           /* pin task to core 0 */
  delay(500);

  // create a task to handle led LED_BUILTIN (core 1)
  xTaskCreatePinnedToCore(
    tLedFunc,     /* Task function. */
    "Led",        /* name of task. */
    10000,        /* Stack size of task */
    NULL,         /* parameter of the task */
    1,            /* priority of the task */
    &t2,          /* Task handle to keep track of created task */
    0);           /* pin task to core 1 */
  delay(500);
}

void loop() {
  
  Serial.print("loop running on core ");
  Serial.println(xPortGetCoreID());
  delay(1000);

}

void tWifi(void *params){
  
  const char* ssid = "rsipenya";       
  const char* wifi_password = "12345678";

  WiFi.begin(ssid, wifi_password);

 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  delay(10);
  
}

void tMqtt(void *params){
  

WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883,wifiClient); 

   //connect MQTT Broker
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
  delay(10);
  }
  

void tLedFunc(void *params) {
  
  // setup  
  Serial.print("tLedFunc running on core ");
  Serial.println(xPortGetCoreID());
  
  WiFiClient wifiClient;
  PubSubClient client(mqtt_server, 1883,wifiClient); 
  
  // loop
  while (true) {
    if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH); 
    Serial.println("turn off led");
    delay(1000);
   }  
    if(WiFi.status() == WL_CONNECTED){
    Serial.println("Connect WIFI");  
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.println("Connect WIFI ");  
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500); 
    }
    else if(client.connected()){
    digitalWrite(LED_BUILTIN, LOW); 
    Serial.println("Connect MQTT");
    }
 }
}
