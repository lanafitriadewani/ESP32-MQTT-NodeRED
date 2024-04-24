#include <WiFi.h>  
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

const int DHT_PIN = 33;  
const int LED_MERAH = 4;
const int LED_KUNING = 2;
const int LED_HIJAU = 32;
const char* ssid = "theo"; ///  wifi ssid 
const char* password = "haaaaaaa";
const char* mqtt_server = "172.20.10.3";// mosquitto server url

DHT sensor_dht(DHT_PIN,DHT22);
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
float temp = 0;
float hum = 0;

void setup_wifi() { 
  delay(10);
  Serial.println();
  Serial.print("Wifi terkoneksi ke : ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi berhasil terkoneksi");
  Serial.print("Alamat IP : ");
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length) { 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) { 
    Serial.print((char)payload[i]);
  }}
void reconnect() { 
  while (!client.connected()) {
    Serial.print("Baru melakukan koneksi MQTT ...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }}
}
void setup() {
  //pinMode(2, OUTPUT); 
  pinMode(LED_MERAH,OUTPUT);
  pinMode(LED_KUNING,OUTPUT);
  pinMode(LED_HIJAU,OUTPUT);    
  Serial.begin(115200);
  sensor_dht.begin();
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); 

}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) { //perintah publish data
    lastMsg = now;
    
    float temp = sensor_dht.readTemperature();
    float hum = sensor_dht.readHumidity();
    float rate = (temp*2+hum)/2;
    
    if (rate<55){
      digitalWrite(LED_HIJAU,HIGH);
      digitalWrite(LED_KUNING,LOW);
      digitalWrite(LED_MERAH,LOW);
    }
    else if (rate<61){
      digitalWrite(LED_HIJAU,LOW);
      digitalWrite(LED_KUNING,HIGH);
      digitalWrite(LED_MERAH,LOW);
    }
    else{
      digitalWrite(LED_HIJAU,LOW);
      digitalWrite(LED_KUNING,LOW);
      digitalWrite(LED_MERAH,HIGH);
    }
    
    char temp_str[8];
    dtostrf(temp, 1, 2, temp_str);
    client.publish("/dht_sensor/temp", temp_str); 
    char hum_str[8];
    dtostrf(hum, 1, 2, hum_str);
    client.publish("/dht_sensor/hum", hum_str);   

    Serial.println("Data Sensor: ");
    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Humidity: ");
    Serial.println(hum);

    //Membuat doc JSON
    DynamicJsonDocument doc(100);
    doc["temperature"] = temp;
    doc["humidity"] = hum;

    // Mengonversi objek JSON menjadi string
    char jsonBuffer[100];
    serializeJson(doc, jsonBuffer);
    
    // Publish data dalam format JSON
    client.publish("/dht_sensor/data", jsonBuffer);

    Serial.println("Format JSON: ");
    Serial.println(jsonBuffer);
    Serial.println(" ");
  }
}
