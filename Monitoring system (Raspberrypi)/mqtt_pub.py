#!/usr/bin/python3
import paho.mqtt.client as mqtt

class MQTTPublisher:
    def __init__(self, broker_ip, port=1883, client_id="Raspberrypi_Broker"):
        self.broker_ip = broker_ip
        self.port = port
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=client_id)

    def connect(self):
        try:
            self.client.connect(self.broker_ip, self.port, 60)
            self.client.loop_start()  
            print("Connected to MQTT broker.")
        except Exception as e:
            print(f"Failed to connect to MQTT broker: {e}")

    def publish(self, topic, message):
        try:
            self.client.publish(topic, message)
            print(f"Published message: {message} to topic: {topic}")
        except Exception as e:
            print(f"Failed to publish message: {e}")

    def disconnect(self):
        self.client.loop_stop()  
        self.client.disconnect()  
        print("Disconnected from MQTT broker.")
