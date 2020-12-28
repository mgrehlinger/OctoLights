#include <Arduino.h>
#include <PubSubClient.h>

#include <ESP8266WiFi.h>
#define PAYLOAD_SIZE 300
char msg_payload_buffer[PAYLOAD_SIZE];

const char *ssid = "Wombat5";
const char *password = "Gallowspoint";
const char *hostname = "OctoLights";
const char mqtt_server[] = "progsci.net";
const char event_topic[] = "octoPrint/event/PrinterStateChanged";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

int cnt = 0;
bool bLightsOn = false;

void octo_callback(char *topic, byte *payload, unsigned int length)
{

    if (length < PAYLOAD_SIZE)
    {
        memset(msg_payload_buffer, 0x0, PAYLOAD_SIZE * sizeof(char));
        strncpy(msg_payload_buffer, (char *)payload, PAYLOAD_SIZE);
        printf("got: <%s> <%s>\n", topic, msg_payload_buffer);
    }
    else
        printf("**** length exceeded: %d\n", length);
}

void initWifi()
{
    WiFi.begin(ssid, password);
    WiFi.hostname(hostname);
    pinMode(LED_BUILTIN, OUTPUT);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(200);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
        printf(".");
    }
    printf("Connected...\n");
    mqttClient.setServer(mqtt_server, 1883);
    mqttClient.setCallback(octo_callback);    
}


void reconnect()
{
    while (!mqttClient.connected())
    {
        String clientId = "OctoMqtt-";
        clientId += String(random(0xffff), HEX);
        printf("OctoMqtt:reconnect() %s\n", clientId.c_str());
        if (mqttClient.connect(clientId.c_str()))
        {
            mqttClient.subscribe(event_topic);
        }
        else // wait before trying again
            delay(2000);
    }
}


#define LIGHT_PIN D7
void setup()
{
  Serial.begin(9600);
  pinMode(LIGHT_PIN, OUTPUT);
  initWifi();
}

void loop()
{
  printf("cnt: %d\n", cnt++);
  digitalWrite(LIGHT_PIN, bLightsOn ? LOW : HIGH);
  bLightsOn = !bLightsOn;
  if (!mqttClient.connected())
      reconnect();
  mqttClient.loop();  
  delay(1000);
}