/*****************************************************/
/*                                                   */
/*                   TinyMQTTClient                  */
/*      (c) Yvan Régeard - All rights reserved       */
/*                                                   */
/* Licensed under the MIT license. See LICENSE file  */
/* in the project root for full license information. */
/*                                                   */
/*****************************************************/



// Includes
#include <Arduino.h>
#include <Client.h>
#include "MQTTClient.h"



// MQTTClient public functions

  // Constructor
  MQTTClient::MQTTClient(Client* p_client,void (*callback)(char* topic,uint8_t* message,uint16_t length),uint8_t* buffer,uint16_t buffer_size)
  {
    // Store TCP client pointer
    m_p_client=p_client;

    // Store callback
    m_callback=callback;

    // Allocate internal buffer
    if ((buffer)&&(buffer_size))
    {
      m_buffer=buffer;
      m_buffer_size=buffer_size;
    }
    else
    {
      m_internal_buffer=true;
      m_buffer=(uint8_t*) malloc(MQTT_INTERNAL_BUFFER_SIZE);
      m_buffer_size=MQTT_INTERNAL_BUFFER_SIZE;
    }
  }



  // Destructor
  MQTTClient::~MQTTClient()
  {
    // Disconnect from server
    disconnect();

    // Free transmission buffers
    if (m_internal_buffer) free(m_buffer);
  }



  // Connect to server
  bool MQTTClient::connect(const char* server_address,const uint16_t server_port,const char* id,const char* user,const char* password)
  {
    // Return when TCP client is not defined
    if (!m_p_client) return false;

    // Get lengths
    uint16_t id_length=strlen(id);
    uint16_t user_length=strlen(user);
    uint16_t password_length=strlen(password);
    uint16_t payload_length=10+2+id_length;
    if (user) payload_length+=2+user_length;
    if (password) payload_length+=2+password_length;

    // Exit on wrong parameters
    if ((!server_address)||(!server_port)||(!id)) return false;
    if ((!user)&&(password)) return false;
    if ((4+payload_length)>m_buffer_size) return false;

    // Close any previous connection
    m_p_client->flush();
    m_p_client->stop();

    // Connect WiFi client
    if (!m_p_client->connect(server_address,server_port)) return false;

    // Create CONNECT message

      // Type
      uint16_t index=0;
      m_buffer[index++]=MQTT_CONNECT;

      // Length
      set_length(index,payload_length);

      // Version
      m_buffer[index++]=0x00;
      m_buffer[index++]=0x04;
      m_buffer[index++]='M';
      m_buffer[index++]='Q';
      m_buffer[index++]='T';
      m_buffer[index++]='T';

      // Configuration
      m_buffer[index++]=0x04;
      m_buffer[index++]=0x02|(user?0x80:0x00)|(password?0x40:0x00);
      m_buffer[index++]=(MQTT_KEEP_ALIVE>>8);
      m_buffer[index++]=(MQTT_KEEP_ALIVE>>0);

      // ID
      m_buffer[index++]=(id_length>>8);
      m_buffer[index++]=(id_length>>0);
      for (uint16_t i=0; i<id_length; i++) m_buffer[index++]=id[i];

      // User
      if (user)
      {
        m_buffer[index++]=(user_length>>8);
        m_buffer[index++]=(user_length>>0);
        for (uint16_t i=0; i<user_length; i++) m_buffer[index++]=user[i];

        // Password
        if (password)
        {
          m_buffer[index++]=(password_length>>8);
          m_buffer[index++]=(password_length>>0);
          for (uint16_t i=0; i<password_length; i++) m_buffer[index++]=password[i];
        }
      }

    // Send message to broker
    if (!send_buffer(m_buffer,index)) return false;

    // Get response message
    uint16_t response_length=0;
    if (get_response(response_length))
    {
      // Check response
      if ((response_length==4)
        &&(m_buffer[0]==MQTT_CONNECT_ACK)
        &&(m_buffer[1]==0x02)
        &&(m_buffer[2]==0x00)
        &&(m_buffer[3]==0x00)) return true;
    }

    // Wrong response : close connection
    m_p_client->flush();
    m_p_client->stop();

    // Return : not connected
    return false;
  }



  // Disconnect from server
  bool MQTTClient::disconnect()
  {
    // Return when TCP client is not defined or disconnected
    if ((!m_p_client)||(!m_p_client->connected())) return false;

    // Send DISCONNECT message to broker
    m_buffer[0]=MQTT_DISCONNECT;
    m_buffer[1]=0;
    send_buffer(m_buffer,2);

    // Reset WiFi client
    m_p_client->flush();
    m_p_client->stop();

    // Return : disconnected
    return true;
  }



  // Subscribe to top
  bool MQTTClient::subscribe(const char* topic)
  {
    // Return when TCP client is not defined or disconnected
    if ((!m_p_client)||(!m_p_client->connected())) return false;

    // Get lengths
    uint16_t topic_length=strlen(topic);
    uint16_t payload_length=3+2+topic_length;

    // Return on wrong parameters
    if (!topic) return false;
    if ((4+payload_length)>m_buffer_size) return false;

    // Create SUBSCRIBE message

      // Type
      uint16_t index=0;
      m_buffer[index++]=MQTT_SUBSCRIBE|MQTT_QOS1;

      // Length
      set_length(index,payload_length);

      // Message ID
      if ((m_message_id++)==0) m_message_id=1;
      m_buffer[index++]=(m_message_id>>8);
      m_buffer[index++]=(m_message_id>>0);
  
      // Topic
      m_buffer[index++]=(topic_length>>8);
      m_buffer[index++]=(topic_length>>0);
      for (uint16_t i=0; i<topic_length; i++) m_buffer[index++]=topic[i];

      // QoS
      m_buffer[index++]=0x00;

    // Send message to broker
    return send_buffer(m_buffer,index);
  }



  // Unsubscribe from topic
  bool MQTTClient::unsubscribe(const char* topic)
  {
    // Return when TCP client is not defined or disconnected
    if ((!m_p_client)||(!m_p_client->connected())) return false;

    // Get lengths
    uint16_t topic_length=strlen(topic);
    uint16_t payload_length=4+topic_length;

    // Return on wrong parameters
    if (!topic) return false;
    if ((4+payload_length)>m_buffer_size) return false;

    // Create UNSUBSCRIBE message

      // Type
      uint16_t index=0;
      m_buffer[index++]=MQTT_UNSUBSCRIBE|MQTT_QOS1;

      // Length
      set_length(index,payload_length);

      // Message ID
      if ((m_message_id++)==0) m_message_id=1;
      m_buffer[index++]=(m_message_id>>8);
      m_buffer[index++]=(m_message_id>>0);
  
      // Topic
      m_buffer[index++]=(topic_length>>8);
      m_buffer[index++]=(topic_length>>0);
      for (uint16_t i=0; i<topic_length; i++) m_buffer[index++]=topic[i];

    // Send message to broker
    return send_buffer(m_buffer,index);
  }



  // Publish
  bool MQTTClient::publish(const char* topic,const char* message)
  {
    // Return when TCP client is not defined or disconnected
    if ((!m_p_client)||(!m_p_client->connected())) return false;

    // Get lengths
    uint16_t topic_length=strlen(topic);
    uint16_t message_length=strlen(message);
    uint16_t payload_length=2+topic_length+message_length;

    // Return on wrong parameters
    if ((!topic)||(!message)) return false;
    if ((4+payload_length)>m_buffer_size) return false;

    // Create PUBLISH message

      // Type
      uint16_t index=0;
      m_buffer[index++]=MQTT_PUBLISH;

      // Length
      set_length(index,payload_length);

      // Topic
      m_buffer[index++]=(topic_length>>8);
      m_buffer[index++]=(topic_length>>0);
      for (uint16_t i=0; i<topic_length; i++) m_buffer[index++]=topic[i];
  
    // Send message to broker
    if (send_buffer(m_buffer,index))
    {
      return send_buffer((uint8_t*) message,message_length);
    }

    // Return : error
    return false;
  }



  // Main loop
  void MQTTClient::loop()
  {
    // Return when TCP client is not defined or disconnected
    if ((!m_p_client)||(!m_p_client->connected())) return;

    // Check if message is available
    if (m_p_client->available())
    {
      // Load buffer
      uint16_t data_length=0;
      bool overflow=false;
      uint32_t timeout=millis()+MQTT_ACTIVITY_TIMEOUT;
      while(m_p_client->available())
      {
        // Read byte received
        uint8_t byte_received=m_p_client->read();
        if (data_length<m_buffer_size) m_buffer[data_length++]=byte_received;
        else overflow=true;

        // Process background tasks
        yield();

        // Timeout : close connection and ignore data
        if (((int32_t)(millis()-timeout))>0)
        {
          m_p_client->flush();
          m_p_client->stop();
          return;
        }
      }

      // Overflow : flush and ignore data
      if (overflow)
      {
        m_p_client->flush();
        return;
      }

      // Update ping process
      m_ping_timeout=millis()+MQTT_ACTIVITY_TIMEOUT;
      m_ping_sent=false;

      // Process message received
      switch(m_buffer[0]&0xF0)
      {
        // Publish
        case MQTT_PUBLISH:
          {
            // Parse length
            uint16_t index=1;
            while((index<5)&&((m_buffer[index++]&0x80)!=0));

            // Parse topic
            uint16_t topic_length=(m_buffer[index++]<<8)+(m_buffer[index]<<0);
            char* topic=(char*) &m_buffer[index];
            for(uint16_t i=0; i<topic_length; i++) m_buffer[index++]=m_buffer[index+1];
            m_buffer[index++]=0x00;

            // End message with 0x00
            if (data_length<m_buffer_size) m_buffer[data_length]=0x00;
            else
            {
              // Overflow : flush and ignore data
              m_p_client->flush();
              return;
            }

            // QoS 0
            if ((m_buffer[0]&0x06)==MQTT_QOS0)
            {
              // Execute callback
              if (m_callback) m_callback(topic,&m_buffer[index],data_length-index);
            }

            // QoS 1
            if ((m_buffer[0]&0x06)==MQTT_QOS1)
            {
              // Extract message ID
              uint16_t message_id=(m_buffer[index++]<<8)+(m_buffer[index++]<<0);

              // Execute callback
              if (m_callback) m_callback(topic,&m_buffer[index],data_length-index);

              // Send PUBLISH_ACK to broker
              m_buffer[0]=MQTT_PUBLISH_ACK;
              m_buffer[1]=0x00;
              m_buffer[2]=(message_id>>8);
              m_buffer[3]=(message_id>>0);
              send_buffer(m_buffer,4);
            }
          }
          break;

        // Ping acknoledgement : send another ping
        case MQTT_PING_ACK:
          m_ping_time=millis()-m_ping_start_time;
          m_ping_sent=false;
          break;

        // Ping request : send back a PING response
        case MQTT_PING:
          m_buffer[0]=MQTT_PING_ACK;
          m_buffer[1]=0x00;
          send_buffer(m_buffer,2);
          break;
      }
    }
    else
    {
      // Process ping
      if (((int32_t)(millis()-m_ping_timeout))>0)
      {
        // Check if a ping was already sent
        if (m_ping_sent)
        {
          // Reset WiFi client
          m_p_client->flush();
          m_p_client->stop();
        }
        else
        {
          // Start ping time count
          m_ping_start_time=millis();

          // Send PING to broker
          m_buffer[0]=MQTT_PING;
          m_buffer[1]=0x00;
          m_ping_sent=send_buffer(m_buffer,2);
        }
      }
    }
  }



  // Get ping time
  uint32_t MQTTClient::get_ping_time()
  {
    // Return ping time
    return m_ping_time;
  }



// MQTTClient private functions

  // Set length in TX buffer
  void MQTTClient::set_length(uint16_t& index,uint16_t length)
  {
    do
    {
      // Get 7-bits length byte
      uint8_t length_byte=length&0x7F;
      length>>=7;
      if (length>0) length_byte|=0x80;

      // Add length byte to TX buffer
      m_buffer[index++]=length_byte;
    }
    while(length>0);
  }



  // Send buffer
  bool MQTTClient::send_buffer(uint8_t* buffer,uint16_t length)
  {
    // Return when TCP client is not defined
    if (!m_p_client) return false;

    // Send buffer to broker
    for(uint16_t i=0; i<length; i++)
    {
      // Send byte
      if (m_p_client->write(buffer[i])!=1)
      {
        // Reset WiFi connection
        m_p_client->flush();
        m_p_client->stop();

        // Return : connection failed
        return false;
      }

      // Process background tasks
      yield();
    }

    // Update ping process
    m_ping_timeout=millis()+MQTT_ACTIVITY_TIMEOUT;
    m_ping_sent=false;

    // Return
    return true;
  }



  // Get response
  bool MQTTClient::get_response(uint16_t& length)
  {
    // Return when TCP client is not defined
    if (!m_p_client) return false;

    // Wait for response from broker
    uint32_t timeout=millis()+MQTT_ACTIVITY_TIMEOUT;
    while (!m_p_client->available())
    {
      // Process background tasks
      delay(150);

      // Return : timeout
      if (((int32_t)(millis()-timeout))>0) return false;
    }

    // Load buffer
    uint16_t index=0;
    bool overflow=false;
    timeout=millis()+MQTT_ACTIVITY_TIMEOUT;
    while(m_p_client->available())
    {
      // Read byte received
      uint8_t byte_received=m_p_client->read();
      if (index<m_buffer_size) m_buffer[index++]=byte_received;
      else overflow=true;

      // Process background tasks
      yield();

      // Return : timeout
      if (((int32_t)(millis()-timeout))>0) return false;
    }

    // Overflow : flush and ignore data
    if (overflow)
    {
      m_p_client->flush();
      return false;
    }

    // Update ping process
    m_ping_timeout=millis()+MQTT_ACTIVITY_TIMEOUT;
    m_ping_sent=false;

    // Return : response received
    length=index;
    return true;
  }
