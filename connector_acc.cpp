#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <iostream>
#include <string>
#include "mqtt/async_client.h"

// Replace with your accelerometer I2C address
const int ACCELEROMETER_ADDRESS = 0x68;

// MQTT broker settings
const std::string BROKER_ADDRESS = "tcp://mqtt_broker_address:1883";
const std::string CLIENT_ID = "accelerometer_publisher";

// MQTT topic to publish the data
const std::string TOPIC = "accelerometer_data";

int main()
{
    // Initialize wiringPi and set up I2C communication with the accelerometer
    wiringPiSetup();
    int fd = wiringPiI2CSetup(ACCELEROMETER_ADDRESS);

    if (fd == -1)
    {
        std::cerr << "Failed to initialize I2C communication." << std::endl;
        return 1;
    }

    // Create an MQTT client
    mqtt::async_client client(BROKER_ADDRESS, CLIENT_ID);

    // Connect to the MQTT broker using MQTT protocol
    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);

    try
    {
        // Connect to the MQTT broker
        client.connect(connOpts)->wait();

        while (1)
        {
            // Read accelerometer data using I2C protocol
            int16_t x = wiringPiI2CReadReg16(fd, 0x3B);
            int16_t y = wiringPiI2CReadReg16(fd, 0x3D);
            int16_t z = wiringPiI2CReadReg16(fd, 0x3F);

            // Convert data to string
            std::string data = std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z);

            // Publish data to MQTT broker using MQTT protocol
            mqtt::message_ptr pubmsg = mqtt::make_message(TOPIC, data);
            client.publish(pubmsg)->wait();

            delay(100);  // Delay between readings
        }

        // Disconnect from the MQTT broker
        client.disconnect()->wait();
    }
    catch (const mqtt::exception& exc)
    {
        std::cerr << "Error: " << exc.what() << std::endl;
        return 1;
    }

    return 0;
}
