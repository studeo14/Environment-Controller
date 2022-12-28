
import paho.mqtt.client as mqtt

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("$SYS")
    client.subscribe("/topic/monitor_data", qos=0)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    if msg.topic == "/topic/monitor_data":
        with open("monitor_data.dat", "ab") as datafile:
            datafile.write(msg.payload)
            datafile.write(b"\n")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.username_pw_set("scraper", "6uG2LDXMu6P9vep")
client.connect("localhost", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
