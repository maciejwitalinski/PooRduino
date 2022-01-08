
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#ifndef STASSID
#define STASSID "WIFI_SSID"
#define STAPSK  "WIFI_PWD"
#endif

#define sonarPWRPin D4 // Sonar Relay trig pin
#define echoPin D7 // Echo Pin
#define trigPin D6 // Trigger Pin
long duration, distance;

const char* ssid = STASSID;
const char* password = STAPSK;

const char* host = "RASPBERRY_EXTERNAL_IP";
const int httpsPort = 3003;
int retryCount = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(sonarPWRPin, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(sonarPWRPin, HIGH);

  delay(1000);
  digitalWrite(sonarPWRPin, LOW);
  delay(1000);
  digitalWrite(sonarPWRPin, HIGH);
    delay(1000);
  digitalWrite(sonarPWRPin, LOW);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void sendReading(int distance) {
  WiFiClient client;
  Serial.print("Connecting to ");
  Serial.println(host);


  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return;
  }

  String url = "/reading/";
  Serial.print("Requesting URL: ");
  Serial.println(url + "reading/" + distance);
  

  client.print(String("GET ") + url + distance + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: PooReader v2\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("Request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  Serial.println("Reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("Closing connection");
}

void loop() {
  digitalWrite(sonarPWRPin, LOW);
  delay(1000);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
 
  duration = pulseIn(echoPin, HIGH);
  distance = duration / 58;
  Serial.println(distance);
  
  digitalWrite(sonarPWRPin, HIGH);
  if(distance == 0) {
    if(retryCount > 4) {
      sendReading(distance);
      ESP.deepSleep(7200000000);
    }
    delay(4000);
    retryCount = retryCount + 1;
  } else {
    if(distance < 400) {
      retryCount = 0;
      sendReading(distance);
      ESP.deepSleep(7200000000);
    } else {
      delay(1000);
    }
  }
}
