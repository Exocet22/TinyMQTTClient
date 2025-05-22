/*****************************************************/
/*                                                   */
/*                   TinyMQTTClient                  */
/*      (c) Yvan Régeard - All rights reserved       */
/*                                                   */
/* Licensed under the MIT license. See LICENSE file  */
/* in the project root for full license information. */
/*                                                   */
/*****************************************************/
#ifndef _TINY_MQTTCLIENT_H_
#define _TINY_MQTTCLIENT_H_



// Constants
#define MQTT_INTERNAL_BUFFER_SIZE     256     // MQTT buffer size
#define MQTT_KEEP_ALIVE               15      // 15 seconds keep alive
#define MQTT_ACTIVITY_TIMEOUT         10000   // 10 seconds activity timeout
#define MQTT_CONNECT                  (1<<4)  // Connect
#define MQTT_CONNECT_ACK              (2<<4)  // Connect acknowledgment
#define MQTT_PUBLISH                  (3<<4)  // Publish
#define MQTT_PUBLISH_ACK              (4<<4)  // Publish acknowledgment
#define MQTT_SUBSCRIBE                (8<<4)  // Subscribe
#define MQTT_UNSUBSCRIBE              (10<<4) // Unsubscribe
#define MQTT_PING                     (12<<4) // Ping
#define MQTT_PING_ACK                 (13<<4) // Ping acknowledgment
#define MQTT_DISCONNECT               (14<<4) // Disconnect
#define MQTT_QOS0                     (0<<1)  // QoS level 0
#define MQTT_QOS1                     (1<<1)  // QoS level 1



// MQTTClient class definition
class MQTTClient
{
  // Private attributes
  private:

    // WiFi client
    WiFiClient m_client;

    // Callback
    void (*m_callback)(char* topic,uint8_t* message,uint16_t length)=NULL;

    // Transmission buffer
    bool m_internal_buffer=false;
    uint8_t* m_buffer=NULL;
    uint32_t m_buffer_size=0;

    // Ping
    uint32_t m_ping_timeout=0;
    bool m_ping_sent=false;

    // Message ID
    uint16_t m_message_id=1;



  // Private methods
  private:

    // Set length in buffer
    void set_length(uint16_t& index,uint16_t length);

    // Send buffer
    bool send_buffer(uint8_t* buffer,uint16_t length);

    // Get response
    bool get_response(uint16_t& length);



  // Public methods
  public:

    // Constructor / destructor
    MQTTClient(void (*callback)(char* topic,uint8_t* message,uint16_t length)=NULL,uint8_t* buffer=NULL,uint16_t buffer_size=0);
    ~MQTTClient();

    // Connect / disconnect
    bool connect(const char* server_address,const uint16_t server_port,const char* id,const char* user=NULL,const char* password=NULL);
    bool disconnect();
    bool connected() {return m_client.connected();};
    
  
    // Subscribe / unsubscribe
    bool subscribe(const char* topic);
    bool unsubscribe(const char* topic);

    // Publish
    bool publish(const char* topic,const char* message);

    // Main loop
    void loop();
};



#endif
