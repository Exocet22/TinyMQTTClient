/*****************************************************/
/*                                                   */
/*             Example for TinyMQTTClient            */
/*      (c) Yvan Régeard - All rights reserved       */
/*                                                   */
/* Licensed under the MIT license. See LICENSE file  */
/* in the project root for full license information. */
/*                                                   */
/*****************************************************/



// Includes
#ifdef ARDUINO_ARCH_ESP8266
  #include <ESP8266WiFi.h>
#endif
#ifdef ARDUINO_ARCH_ESP32
  #include <WiFi.h>
  #include <WiFiClient.h>
#endif
#include <MQTTClient.h>



// Constants
#define WIFI_SSID                               ""
#define WIFI_PASS                               ""
#define MQTT_BROKER_ADDRESS                     ""
#define MQTT_BROKER_PORT                        1883
#define MQTT_USER_ID                            ""
#define MQTT_USER_NAME                          ""
#define MQTT_USER_PASSWORD                      ""
#define MQTT_BUFFER_SIZE                        2048
#define MQTT_PUBLISH_TOPIC                      "MQTTClient"
#define MQTT_MESSAGE_TOPIC                      "MQTTClient/message"



// MQTT callback
void mqtt_callback(char* topic,uint8_t* message,uint16_t length)
{
  // Print message received
  Serial.printf("Received message on topic: %s (%d bytes)\n",topic,length);
  for(uint16_t i=0; i<length; i++) Serial.print((char) message[i]);
  Serial.println();
}



// Global variables
WiFiClient tcp_client;
uint8_t g_mqtt_buffer[MQTT_BUFFER_SIZE];
MQTTClient g_mqtt_client(&tcp_client,mqtt_callback,g_mqtt_buffer,MQTT_INTERNAL_BUFFER_SIZE);



// Initialization
void setup()
{
  // Wait for stable signal
  delay(150);

  // Initialize serial port
  Serial.begin(115200);
  Serial.println("\n****** Test application for TinyMQTTClient Library ******");

  // Connect to WiFi hotspot
  WiFi.begin( WIFI_SSID, WIFI_PASS );
  Serial.println("Connecting to Wifi hotspot...");
  while (WiFi.status()!=WL_CONNECTED) { Serial.print("o"); delay(500); }
  Serial.println("\nConnected");
}



// Main loop
void loop()
{
  // Connect to the broker if not connected
  static uint32_t connect_timeout=0;
  if (millis()>connect_timeout)
  {
    // Update timeout
    connect_timeout=millis()+2000;

    // Check if the client is still connected to the broker
    if ((WiFi.status()==WL_CONNECTED)&&(!g_mqtt_client.connected()))
    {
      // Connect to the broker
      if (g_mqtt_client.connect(MQTT_BROKER_ADDRESS,MQTT_BROKER_PORT,MQTT_USER_ID,MQTT_USER_NAME,MQTT_USER_PASSWORD))
      {
        Serial.println("Connected to MQTT broker");

        // Subscribe to message topic
        if (g_mqtt_client.subscribe(MQTT_MESSAGE_TOPIC)) Serial.println("Subscribed to MQTT message topic");
        else Serial.println("/!\\ Failed to subscribe to MQTT message topic");
      }
      else Serial.println("/!\\ Failed to connect to MQTT broker");
    }
  }

  // Publish a message every 25 seconds
  static uint32_t publish_timeout=0;
  if (millis()>publish_timeout)
  {
    // Update timeout
    publish_timeout=millis()+25000;

    // Publish message
    char message[32];
    sprintf(message,"Message: %ld",millis());
    if (g_mqtt_client.publish(MQTT_PUBLISH_TOPIC,message)) Serial.println("Message published");
  }

  // MQTT client loop
  g_mqtt_client.loop();
}
