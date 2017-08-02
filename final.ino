
/* Create a WiFi access point and provide a web server on it. */
// wifiAccessPoint
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

//HTTPSRequest
#include "DHT.h"
// cổng port của nodeMCU
#define DHTPIN 2
// loại sensor
#define DHTTYPE DHT22




/* Set these to your desired credentials.
* tên wifi và pass của nodeMCU
*/
const char *n_ssid = "ESP_hai";
const char *n_password = "00000000";

const char* w_ssid = "laptop_dell";
const char* w_password = "123456789";


// địa chỉ của server nodejs
const char* host = "192.168.137.1";
const int httpsPort = 3000;

DHT dht(DHTPIN, DHTTYPE);

int time_flag = 1;
unsigned long pre_time = millis();

// Use WiFiClientSecure class to create TLS connection
WiFiClient client;

ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
* connected to this access point to see it.
*/

void setup() {
  Serial.begin(115200);
  pinMode (5, OUTPUT);
  delay(1000);
  start();
  postTime();
  dht.begin();
}

void loop() {
  if(WiFi.status() == WL_CONNECTION_LOST){
    //    turnOFF(D1);
    startAP();
    Serial.println("khong the ket noi wifi!!");
  }else{
    if(time_flag == 1)  postTime();
    if(millis() - pre_time > 5000){
        pre_time = millis();
        postData();
    }
    // delay(5000);
  }
  server.handleClient();
}

void start(){

  // code HTTPSRequest
  connectWifi(w_ssid, w_password);
  if(WiFi.status()==WL_CONNECTED){
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.on("/config", handleConfig);
    server.begin();
  } else{
    // start Access Point
    startAP();
  }
}
void postTime(){
  WiFiClient postTimeClient;
  if(!postTimeClient.connect(host,httpsPort)){
    Serial.println(String("connection failed: ")+ host );
    return;
  }
  String req_body = String("{\"time\": ") + millis() + "}";
  String url = "/time";
  postTimeClient.print(String("POST ") + url + " HTTP/1.1\r\n"
               + "Host: " + host + "\r\n"
               + "Content-Length: " + String(req_body.length()) + "\r\n"
               + "Content-Type: application/json\r\n"
               + "Connection: close\r\n\r\n"
               + req_body);
  Serial.println("Post time request send");
  unsigned long timeout = millis();
  while (postTimeClient.available() == 0) {
    if (millis() - timeout > 2000) {
      Serial.println("Client Timeout !");
      postTimeClient.stop();
      return;
    }
  }
  while (postTimeClient.connected()) {
    String line = postTimeClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Post time headers received");
      break;
    }
  }
  String line = postTimeClient.readStringUntil('\n');
  if (line.startsWith("OK")) {
    time_flag = 0;
  }
 }


// function của route "/"
void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected</h1>");
}
// function của route "/configWifi"
void handleConfig() {
  String ip = server.arg("ip");
  String pass = server.arg("pass");
  w_ssid = ip.c_str();
  w_password = pass.c_str();
  server.send(200, "text/html", "confid di!!!");
  connectWifi(w_ssid, w_password);
}
void handleData() {
  server.send(200, "application/json", readData());
}

// bật wifi của nodeMCU, cấu hình wifi, tạo web server để config
void startAP(){
  //  WiFi.disconnect();
  delay(1000);
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(n_ssid, n_password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.begin();
}
// đọc dữ liệu trên sensor DHT22
String readData(){

  float h = dht.readHumidity();
  float t = dht.readTemperature();
//  long h = random(70,100);
//  long t = random(20,30);
  if (isnan(h) || isnan(t)) {
    h = 0;
    t = 0;
  }
  String data = String("{\"h\": ") + String(h) + ", \"t\": " + String(t) + ", \"time\": " + millis() + "}";
  return data;
}

// gửi dữ liệu lên server (POST)
void postData(){
  String data = readData();
  Serial.println(data);
  String url = "/data";
  WiFiClient client;
  if(!client.connect(host,httpsPort)) return;

  client.print(String("POST ") + url + " HTTP/1.1\r\n"
               + "Host: " + host + "\r\n"
               + "Content-Length: " + String(data.length()) + "\r\n"
               + "Content-Type: application/json\r\n"
               + "Connection: close\r\n\r\n"
               + data);
  Serial.println("Request send");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 2000) {
      Serial.println("Client Timeout !");
      client.stop();
      return;
    }
  }

  while(client.available()){
    String line = client.readString();
    Serial.println(line);
    Serial.println();
  }
}
void connectWifi(const char* ip,const char* pass){
  Serial.println();
  Serial.print("connecting to ");
  // tên wifi và pass của wifi server local phát ra cần kết nối


//  Serial.println(ip);
  WiFi.disconnect();
  WiFi.begin(ip, pass);

  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);
    count++;
    Serial.print(".");
    if(count>10)  break;
    // Serial.printfn("WiFi connect failed!");
  }
}
