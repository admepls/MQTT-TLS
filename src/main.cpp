#include <Arduino.h>
#include <SPIFFS.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>

LOOK FOR README

WiFiClientSecure espClient;
PubSubClient client(espClient);

String root_ca;

bool shouldSaveConfig = false;


char static_ip[16] = "4.4.4.64";
char static_gw[16] = "4.4.4.1";
char static_sn[16] = "255.255.255.0";

String clientId = "ESP32-";
const char *mqtt_server = "4.4.4.44";
const char *mqtt_user = "admepls";
const char *mqtt_password = "pausal192";

unsigned long previousMillis = millis();

// put function declarations here:
int myFunction(int, int);
String readFile(fs::FS &fs, const char * path);
void callback(char *topic, byte *message, unsigned int length);
void saveConfigCallback ();
void reconnect();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  if(!SPIFFS.begin()){
    Serial.println("SPIFFS FAILED");
    return;
  }

  WiFiManager wm;
  wm.setSaveConfigCallback(saveConfigCallback);

  IPAddress _ip, _gw, _sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);

  wm.setSTAStaticIPConfig(_ip, _gw, _sn);

  if (!wm.autoConnect("MQTT"))
  {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    // reset and try again, or maybe put it to deep sleep
    // ESP.restart();
  }
   root_ca = readFile(SPIFFS,"/ca-root-cert.crt");

  espClient.setCACert(root_ca.c_str());
  client.setServer(mqtt_server, 8883);
  client.setCallback(callback);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected())
  {
    reconnect();
  }
  if (!client.loop())
    client.connect("ESP32-");

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= (1000 * 3))
  {
    previousMillis = currentMillis;
    client.publish("esp32tls","it works!");
  }
} 



//Functions
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

String readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\r\n", path);
  File file = SPIFFS.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- N/A");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  file.close();
  Serial.println(fileContent);
  return fileContent;
} 

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "test/topic")
  {
    Serial.println("MSG");
  }
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password))
    {
      Serial.println("connected");
      // for this example not necessary
      client.subscribe("test/topic");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
