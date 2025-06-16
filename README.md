# TinyMQTTClient

A tiny [Arduino](https://arduino.cc/)/[PlatformIO](https://platformio.org/) library for a MQTT client running on ESP8266 and ESP32.

## Installation

### Using the Arduino IDE Library Manager

1. Choose `Sketch` -> `Include Library` -> `Manage Libraries...`
2. Type `TinyMQTTClient` into the search box.
3. Click the row to select the library.
4. Click the `Install` button to install the library.

### Using PlatformIO Library Manager

1. Choose `PIO Home` -> `Libraries` -> `Registry`
2. Type `TinyMQTTClient` into the search box.
3. Click the row to select the library.
4. Click the `Install` button to install the library.

### Using Git

```sh
cd ~/Documents/Arduino/libraries/
git clone https://github.com/exocet22/TinyMQTTClient TinyMQTTClient
```

## Examples

See [examples](examples) folder.

## Limitations

 - MQTT version 3.1.1.
 - No TLS/SSL encryption.
 - SUBSCRIBE/UNSUBSCRIBE in QoS 1.
 - PUBLISH in QoS 0.
 - Topic and message sizes limited to 16 bits.
 - Keepalive set to 15 seconds.
 - Ping set to 10 seconds.
 - Supports external TCP client.

## License

This libary is [licensed](LICENSE) under the [MIT Licence](https://en.wikipedia.org/wiki/MIT_License).
