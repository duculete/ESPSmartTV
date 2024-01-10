#include <FS.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#include <IRrecv.h>
#include <WiFiManager.h> 

#ifdef ESP32
  #include <SPIFFS.h>
#endif

#include <ArduinoJson.h>

#define TV_STATUS_PIN D0
#define IR_RECV_PIN D5
#define IR_SEND_PIN D1

#define KEY_UP 0x00FE30CF
#define KEY_DOWN 0x00FEB04F
#define KEY_CH_UP 0x00FE9867
#define KEY_CH_DOWN 0x00FE18E7
#define KEY_VOL_UP 0x00FED827
#define KEY_VOL_DOWN 0x00FE58A7
#define KEY_RIGHT 0x00FE708F
#define KEY_LEFT 0x00FEF00F
#define KEY_INFO 0x00FE48B7
#define KEY_PLAY_PAUSE 0x00FE52AD
#define KEY_BACK 0x00FEA25D
#define KEY_SELECT 0x00FE08F7
#define KEY_ON_OFF 0x00FEA857
#define KEY_MUTE 0x00FE6897
#define KEY_D_TV 0x00FED02F
#define KEY_SOURCE 0x00FE48B7
#define KEY_MENU 0x00FE8877
#define KEY_0 0x00FE00FF
#define KEY_1 0x00FE807F
#define KEY_2 0x00FE40BF
#define KEY_3 0x00FEC03F
#define KEY_4 0x00FE20DF
#define KEY_5 0x00FEA05F
#define KEY_6 0x00FE609F
#define KEY_7 0x00FEE01F
#define KEY_8 0x00FE10EF
#define KEY_9 0x00FE906F

IRsend irsend(IR_SEND_PIN);
IRrecv irrecv(IR_RECV_PIN);
decode_results results;

int tv_state = 0;
int scan_ir = 0;
String tv_source = "";

WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(2)
char msg[MSG_BUFFER_SIZE];
int value = 0;

char mqtt_server[40];
char mqtt_port[6] = "1883";
char tv_name[34] = "home/room/tv";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("CONFIG: should save config");
  shouldSaveConfig = true;
}

void sendIR(int data){
  irsend.sendNEC(data);
}

void reset_tv_input(){
  delay(1000);
  sendIR(KEY_SOURCE);
  delay(500);
  sendIR(KEY_UP);
  delay(500);
  sendIR(KEY_SELECT);
  delay(500);
  sendIR(KEY_D_TV);
  tv_source = "tv";
}

void callback(char* _topic, byte* payload, unsigned int length) {
  String topic = String(_topic);
  payload[length] = '\0';
  String message = String((char*)payload);
  Serial.print("MQTT: new message on ");
  Serial.print(topic);
  Serial.print(" :");
  Serial.println(message);

  if (topic == String(tv_name) + "/cmd/power") {

    if (message == "1" && tv_state == 0) {
      sendIR(KEY_ON_OFF);
      Serial.println("REMOTE: sending TV ON_OFF IR command");
      delay(1000);
    }

    if (message == "0" && tv_state == 1) {
      sendIR(KEY_ON_OFF);
      Serial.println("REMOTE: sending TV ON_OFF IR command");
    }
  
    if (message == "FACTORY_RESET") {
      SPIFFS.format();
      wifiManager.resetSettings();
      WiFi.disconnect();
      ESP.reset();
      delay(1000);
      Serial.println("CONFIG: factory reset requested ...");
    }

    if (message == "SCAN_IR") {
      scan_ir = 1 - scan_ir;
      if (scan_ir == 1) {
        Serial.println("IrRECV: waiting for IR code ...");
        client.publish( (String(tv_name) + "/info").c_str(), "IrRECV: scanning" );
        irrecv.enableIRIn();
      } else {
        client.publish( (String(tv_name) + "/info").c_str(), "IrRECV: stop scanning" );
        irrecv.disableIRIn();
      }
    }

  }

  if (topic == String(tv_name) + "/input") {
    // someone publish tv_source on /input
    if (tv_source == "" && message != "") {
      tv_source = message;
    }
    // someone publish "" on /input - it means they need the tv_source value
    if (tv_source != "" && message == "") {
      client.publish( (String(tv_name) + "/input").c_str(), tv_source.c_str(), true );
    }
  }
  if (topic == String(tv_name) + "/cmd/input") {
    /*
    Receive words:
      TV
      HDMIx
    do IR commands
    */

    if (message != tv_source) {

      if (message == "tv_news") {
        if (tv_source != "TV") {
          sendIR(KEY_D_TV);
          delay(500);
        }
        sendIR(KEY_4);
        delay(500);
        sendIR(KEY_2);
        delay(500);
        sendIR(KEY_SELECT);
        tv_source = "TV";
      }

      if (message == "protv") {
        if (tv_source != "TV") {
          sendIR(KEY_D_TV);
          delay(500);
        }
        sendIR(KEY_3);
        delay(500);
        sendIR(KEY_2);
        delay(500);
        sendIR(KEY_SELECT);
        tv_source = "TV";
      }

      if (message == "comedy") {
        if (tv_source != "TV") {
          sendIR(KEY_D_TV);
          delay(500);
        }
        sendIR(KEY_9);
        delay(500);
        sendIR(KEY_2);
        delay(500);
        sendIR(KEY_SELECT);
        tv_source = "TV";
      }
      
      if (message == "bbcearth") {
        if (tv_source != "TV") {
          sendIR(KEY_D_TV);
          delay(500);
        }
        sendIR(KEY_1);
        delay(500);
        sendIR(KEY_7);
        delay(500);
        sendIR(KEY_2);
        delay(500);
        sendIR(KEY_SELECT);
        tv_source = "TV";
      }

      if (message == "TV") {
        sendIR(KEY_D_TV);
        tv_source = "TV";
      }

      if (message == "HDMI1") {
        if (tv_source != "TV") {
          sendIR(KEY_D_TV);
          delay(1000);
        }
        for (int i=0; i< 7; i++) {
          sendIR(KEY_SOURCE);
          delay(500);
        }
        sendIR(KEY_SELECT);
        tv_source = "HDMI1";
      }

      if (message == "HDMI3") {
        if (tv_source != "TV") {
          sendIR(KEY_D_TV);
          delay(1000);
        }
        for (int i=0; i< 9; i++) {
          sendIR(KEY_SOURCE);
          delay(500);
        }
        sendIR(KEY_SELECT);
        tv_source = "HDMI3";
      }
      client.publish( (String(tv_name) + "/input").c_str(), tv_source.c_str(), true );
    }
  }

  if (topic == String(tv_name) + "/cmd/remote") {
    
    if (message == "UP" && tv_state == 1) {
      sendIR(KEY_UP);
      Serial.println("REMOTE: Sending TV KEY_UP command");
    }

    if (message == "DOWN" && tv_state == 1) {
      sendIR(KEY_DOWN);
      Serial.println("REMOTE: Sending TV KEY_DOWN command");
    }

    if (message == "LEFT" && tv_state == 1) {
      sendIR(KEY_LEFT);
      Serial.println("REMOTE: Sending TV KEY_LEFT command");
    }

    if (message == "RIGHT" && tv_state == 1) {
      sendIR(KEY_RIGHT);
      Serial.println("REMOTE: Sending TV KEY_RIGHT command");
    }

    if (message == "INFO" && tv_state == 1) {
      sendIR(KEY_SOURCE);
      Serial.println("REMOTE: Sending TV KEY_SOURCE command");
    }

    if (message == "BACK" && tv_state == 1) {
      sendIR(KEY_BACK);
      Serial.println("REMOTE: Sending TV KEY_BACK command");
    }

    if (message == "SELECT" && tv_state == 1) {
      sendIR(KEY_SELECT);
      Serial.println("REMOTE: Sending TV KEY_SELECT command");
    }

    if (message == "KEY_PLAY_PAUSE" && tv_state == 1) {
      sendIR(KEY_PLAY_PAUSE);
      Serial.println("REMOTE: Sending TV KEY_PLAY_PAUSE command");
    }

    if (message == "VOLUME_UP" && tv_state == 1) {
      sendIR(KEY_CH_UP);
      Serial.println("REMOTE: Sending TV KEY_VOL_UP command");
    }

    if (message == "VOLUME_DOWN" && tv_state == 1) {
      sendIR(KEY_CH_DOWN);
      Serial.println("REMOTE: Sending TV KEY_VOL_DOWN command");
    }

  Serial.println("");

  }
}

void publish_info() {
  client.publish((String(tv_name) + "/info").c_str(), ( "IP Address: " + WiFi.localIP().toString() ).c_str() );
  client.publish((String(tv_name) + "/input").c_str(), tv_source.c_str());
  client.publish((String(tv_name) + "/power").c_str(), String(tv_state).c_str());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("MQTT: Attempting MQTT connection...");
    String clientId = String(tv_name);
    clientId.replace("/", "-");

    if (client.connect(clientId.c_str(), "", "", (String(tv_name) + "/LWT").c_str(), 1, true, "0",false)) {
      Serial.println("MQTT: connected");
      publish_info();

      client.publish((String(tv_name) + "/LWT").c_str(), "1" );

      client.subscribe((String(tv_name) + "/cmd/power").c_str());
      Serial.println("MQTT: Subscribed to: " + String(tv_name) + "/cmd/power");

      client.subscribe((String(tv_name) + "/cmd/remote").c_str());
      Serial.println("MQTT: Subscribed to: " + String(tv_name) + "/cmd/remote");

      client.subscribe((String(tv_name) + "/cmd/input").c_str());
      Serial.println("MQTT: Subscribed to: " + String(tv_name) + "/cmd/input");

      client.subscribe((String(tv_name) + "/input").c_str());
      Serial.println("MQTT: Subscribed to: " + String(tv_name) + "/input");
    } else {
      Serial.print("MQTT: connection failed, rc=");
      Serial.println(client.state());
      Serial.println("MQTT: trying again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  //read configuration from FS json
  Serial.println("CONFIG: mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("CONFIG: mounted file system");
    if (SPIFFS.exists("/config.json")) {
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("CONFIG: opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);

 #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if ( ! deserializeError ) {
#else
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
#endif
          Serial.println("\nparsed json");
          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(tv_name, json["tv_name"]);
        } else {
          Serial.println("CONFIG ERROR: failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("CONFIG ERROR: failed to mount FS");
  }
  WiFiManagerParameter custom_mqtt_server("server", "MQTT server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT port", mqtt_port, 6);
  WiFiManagerParameter custom_tv_name("tv_name", "TV Name", tv_name, 32);

  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  // wifiManager.setSTAStaticIPConfig(IPAddress(10, 0, 1, 99), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));
  wifiManager.setTimeout(120);

  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_tv_name);

  // Setup auto AP named ESP Smart TV-*/12345678
  if (!wifiManager.autoConnect(("ESP Smart TV-" + String(ESP.getChipId())).c_str(), "12345678")) {
    Serial.println("ERROR: failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  Serial.print("Connected to wifi:)");
  
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(tv_name, custom_tv_name.getValue());
  Serial.println("\tmqtt_server : " + String(mqtt_server));
  Serial.println("\tmqtt_port : " + String(mqtt_port));
  Serial.println("\ttv_name : " + String(tv_name));

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
 #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    DynamicJsonDocument json(1024);
#else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
#endif
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["tv_name"] = tv_name;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("CONFIG: error failed to open config file for writing");
    }

#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    serializeJson(json, Serial);
    serializeJson(json, configFile);
#else
    json.printTo(Serial);
    json.printTo(configFile);
#endif
    configFile.close();
  }
  Serial.println("=============================");
  Serial.print("\tLocal ip: ");
  Serial.println(WiFi.localIP());
  irsend.begin();
  pinMode(TV_STATUS_PIN, INPUT_PULLDOWN_16);
  client.setServer(mqtt_server, atoi(mqtt_port));
  client.setCallback(callback);
  Serial.println("=============================");
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();

  if (now - lastMsg > 5000) {
    lastMsg = now;
    int state = digitalRead(TV_STATUS_PIN);
    tv_state = state;
    snprintf (msg, MSG_BUFFER_SIZE, "%ld", tv_state);
    Serial.print("STATUS: TV POWER State is ");
    Serial.println(msg);
    client.publish((String(tv_name) + "/power").c_str(), msg);
  }

  if (irrecv.decode(&results) && scan_ir == 1) {
    if (results.decode_type != UNKNOWN) {
      Serial.print("IrRECV: ");
      serialPrintUint64(results.value, HEX);
      Serial.println();
      client.publish( (String(tv_name) + "/ircode").c_str(), uint64ToString(results.value).c_str() );
    }
    irrecv.resume();
  }

}
