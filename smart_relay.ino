//#define BLYNK_AUTH_TOKEN "3kMdJxdP6u8UHct_tzR26e1wDRGFAztd"
#define BLYNK_PRINT Serial
//#define telegramToken "5846409484:AAGfHRSyA6CyxnLu1_Rmu8Ni7kIgm3_Q6SM"


#include <WiFi.h>
#include "CTBot.h"
#include <BlynkSimpleEsp32.h>
#include <WebServer.h>
#include "SPIFFS.h"
#include <NTPClient.h>
#include <WiFiUdp.h>


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200);


// Global Declaration Value

bool reboot = false;

String ssid;
String pass;

//ap mode
String ssid_ap;
String pass_ap;
String ch;
String ip;
IPAddress netmask(255,255,255,0);
IPAddress local;
String BToken;
String TToken;

// Variable to store the HTTP request
String header;

int startRange;
int endRange;

String current;

String relay1State = "off";
String relay2State = "off";
String relay3State = "off";
String relay4State = "off";
String relayAll = "off";

const int relay1 = 23;
const int relay2 = 22;
const int relay3 = 21;
const int relay4 = 19;


String telegramStatus = "off";
String schedulerState = "on";
WebServer server(80);



String readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;
  }
  file.close();
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
  file.close();
}


void resetDefault() {
  writeFile(SPIFFS, "/blynk.txt", String("NULL").c_str());
  writeFile(SPIFFS, "/tele.txt", String("NULL").c_str());
  writeFile(SPIFFS, "/ssid.txt", String("My Home").c_str());
  writeFile(SPIFFS, "/pass.txt", String("NULL").c_str());
  writeFile(SPIFFS, "/ssidAP.txt", String("Smart Relay").c_str());
  writeFile(SPIFFS, "/passAP.txt", String("NULL").c_str());
  writeFile(SPIFFS, "/start.txt", String("0").c_str());
  writeFile(SPIFFS, "/end.txt", String("0").c_str());
  writeFile(SPIFFS, "/ch.txt", String("1").c_str());
  writeFile(SPIFFS, "/ip.txt", String("192.168.0.1").c_str());
  reboot = true;
}


const char headerPage[] PROGMEM = R"=====(
<style>
*{
  padding: 0;
  margin: 0;
  font-family: arial;
}
body{
  background: black;
}
header {
  background-color: #282A35;
  color: #f1f1f1;
  display: flex;
  align-items: center;
  position: relative;
  width: 100%;
  height: 50px;
}
header span{
  margin-left: 10px;
}
header h3{
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  white-space: nowrap;
}
header span a{
  color: #f1f1f1;
  text-decoration: none;
}
form{
    border: 2px solid black;
    border-radius: 10px;
    width: 300px;
    margin-top: 20px;
}
</style>
<header>
  <span><a href="/">&lt;--</a></span>
  <h3>%title%</h3>
</header>
)=====";


const char wifiHTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>

%header%
<style>
  button{
    width: 100px;
    height: 30px;
    margin-top: 10px;
  }
  input{
    outline: none;
    background: transparent;
    color: white;
    border: 1px solid white;
    border-radius: 3px;
    margin-top: 5px;
    padding: 3px;
  }
  h2, h3, td {
    color: white;
  }
</style>
<center>
  
  <form action="/wifi-submit">
    <h2>Wifi Configuration</h2>
    <h3>Current Wifi: %ssid%</h3>
    
    <input required type="text" name="ssid" placeholder="SSID"><br>
    <input required type="text" name="pass" placeholder="Password"><br>
    <button type="submit">Save</button>
  </form>

  <form action="/ap-submit">
    <h2>Web Panel Configuration</h2>
    <table>
      <tr>
        <td>Current AP Name:</td>
        <td>%APNAME%</td>
      </tr>
      <tr>
        <td>Current Password:</td>
        <td>%APPASS%</td>
      </tr>
      <tr>
        <td>Current IP:</td>
        <td>%APIP%</td>
      </tr>
      <tr>
        <td>Current Chanel:</td>
        <td>%CH%</td>
      </tr>
    </table>

    <input required type="text" name="apname" placeholder="Nama Akses Point"><br>
    <input required type="text" name="appass" placeholder="Password Akses Point"><br>
    <input required type="text" name="ch" placeholder="Channel Akses Point"><br>
    <input required type="text" name="apip" placeholder="Alamat IP Web Panel" value="192.168.0.1"><br>
    <button type="submit">Save</button>
  </form>
  
</center>
</body>
</html>
)=====";

const char apiHTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>

%header%
<style>
  button{
    width: 100px;
    height: 30px;
    margin-top: 10px;
  }
  input{
    outline: none;
    background: transparent;
    color: white;
    border: 1px solid white;
    border-radius: 3px;
    margin-top: 5px;
    padding: 3px;
  }
  h2, h3, td {
    color: white;
  }
</style>
<center>

  <form action="/api">
    <h2>API Token Manager</h2>

    <input required type="text" name="blynk" placeholder="Blynk Token" value="%BLYNK%"><br>
    <input required type="text" name="telegram" placeholder="Telegram Token" value="%TELEGRAM%"><br>
    <button type="submit">Save</button>
    
  </form>
  
</center>
</body>
</html>
)=====";


const char rootHTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
  *{
    font-family: arial;
    padding: 0;
    margin: 0;
  }
  body{
    background: black;
  }
  h2{
    color: white;
  }
  div{
    display: flex;
    flex-direction: column;
    justify-content: center;
    align-items: center;
  }
  a{
    text-decoration: none;
    color: rgb(0, 255, 0);
    border: 1px solid rgb(0, 255, 0);
    background: transparent;
    padding: 5px 10px;
    margin-top: 30px;
  }
</style>
</head>
<body>

<center>
  <h2>Smart Panel Web Interface</h2>
  
  <div>
  <a href="/wifi">Wifi Setup</a>
  <a href="/relay">Control Panel</a>
  <a href="/api">Api Manager</a>
  <a href="/scheduler">Scheduler</a>
  </div>

</center>

</body>
</html>
)=====";



const char relayHTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>
%header%
<style>
form {
  display: flex;
  flex-direction: column;
  justify-content: center;
  border: none;
}
form button {
  width: 200px;
  height: 30px;
  margin: auto;
  margin-top: 10px;
}
button{
  background: transparent;
  border: 1px solid white;
  border-radius: 5px;
}
button[value="on"] {
  color: rgb(255, 0, 0);
  border-color: rgb(255, 0, 0);
}
button[value="off"] {
  color: rgb(0, 255, 0);
  border-color: rgb(0, 255, 0);
}
</style>
<center>
<form action="">
<button type="submit" name="r1" value="{r1State}">Relay 1 {r1t}</button>
<button type="submit" name="r2" value="{r2State}">Relay 2 {r2t}</button>
<button type="submit" name="r3" value="{r3State}">Relay 3 {r3t}</button>
<button type="submit" name="r4" value="{r4State}">Relay 4 {r4t}</button>
<button type="submit" name="all" value="{rA}">Relay All {rat}</button>
<button type="submit" name="tele" value="{telegramStatus}">Telegram {teleT}</button>
<con></con>
</form>
</center>


</body>
)=====";


const char schedulerHTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
  *{
    font-family: arial;
    padding: 0;
    margin: 0;
  }
  body{
    background: black;
  }
  h2, h4{
    color: white;
  }
  input{
    outline: none;
    background: transparent;
    color: white;
    border: 1px solid white;
    border-radius: 3px;
    margin-top: 5px;
    padding: 3px;
  }
  button{
    background: transparent;
    border: 1px solid white;
    border-radius: 5px;
    padding: 2px 10px;
    color: white;
    margin-top: 15px;
  }
  button[value="on"] {
    color: rgb(255, 0, 0);
    border-color: rgb(255, 0, 0);
  }
  button[value="off"] {
    color: rgb(0, 255, 0);
    border-color: rgb(0, 255, 0);
  }
</style>
</head>
<body>
%header%
<center>
  <h2>Scheduler</h2>
  <h4>Current Time: %time%</h4>
  <h4>Current Start Range: %start%</h4>
  <h4>Current End Range: %end%</h4>
  <form action="">
  <button type="submit" name="scheduler" value="%schedulerState%">Scheduler %schedulerT%</button>
  </form>
  <form>
  <input type="text" name="start" placeholder="Start">
  <input type="text" name="end" placeholder="End"><br>
  <button type="submit">Save</button>
  </form>

</center>

</body>
</html>
)=====";


String processor (const String str, const String val1, const String val2) {
  String s = str;

  s.replace(val1, val2);

  return s;
}




void rootHandle(){
  String s = rootHTML;
  server.send(200, "text/html", s);
}

void apiHandle() {
  String s = apiHTML;
  String c = headerPage;
  s = processor(s, "%BLYNK%", BToken);
  s = processor(s, "%TELEGRAM%", TToken);
  s = processor(s, "%header%", c);
  s = processor(s, "%title%", "API Manager");
  String b = "<script>alert(\"Konfigurasi Berhasil Di Simpan\");alert(\"Jika lampu indikator tidak menyala silahkan konfigurasi ulang\");location.href=\"/\"</script>";
  
  String bt = server.arg("blynk");
  String tt = server.arg("telegram");

  if (bt != "" && tt != ""){
    writeFile(SPIFFS, "/blynk.txt", bt.c_str());
    writeFile(SPIFFS, "/tele.txt", tt.c_str());
    reboot = true;
    server.send(200, "text/html", b);
  }else{
    server.send(200, "text/html", s);
  }
  
}

void wifi() {
  String s = wifiHTML;
  String c = headerPage;
  c = processor(c, "%title%", "Wireless Configuration");
  s = processor(s, "%header%", c);
  s = processor(s, "%ssid%", ssid);
  s = processor(s, "%APNAME%", ssid_ap);
  s = processor(s, "%APPASS%", pass_ap);
  s = processor(s, "%APIP%", ip);
  s = processor(s, "%CH%", ch);
  server.send(200, "text/html", s);
}

void wifiHandle() {
  String s = "<script>alert(\"Konfigurasi Berhasil Di Simpan\");alert(\"Jika lampu indikator tidak menyala silahkan konfigurasi ulang\");location.href=\"/\"</script>";
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");

  writeFile(SPIFFS, "/ssid.txt", ssid.c_str());
  writeFile(SPIFFS, "/pass.txt", pass.c_str());
  
  server.send(200, "text/html", s);

  delay(3000);

  reboot = true;
}

void apHandle() {
  String s = "<script>alert(\"Konfigurasi Berhasil Di Simpan\");alert(\"Jika lampu indikator tidak menyala silahkan konfigurasi ulang\");location.href=\"/\"</script>";
  String ssidAP = server.arg("apname");
  String passAP = server.arg("appass");
  String ch = server.arg("ch");
  String apIP = server.arg("apip");

  writeFile(SPIFFS, "/ssidAP.txt", ssidAP.c_str());
  writeFile(SPIFFS, "/passAP.txt", passAP.c_str());
  writeFile(SPIFFS, "/ch.txt", ch.c_str());
  writeFile(SPIFFS, "/ip.txt", apIP.c_str());

  server.send(200, "text/html", s);
  delay(3000);
  reboot = true;
}

void schedulerHandle() {
  String s = schedulerHTML;
  String b = headerPage;
  s = processor(s, "%header%", b);
  s = processor(s, "%title%", "Scheduler");
  s = processor(s, "%start%", String(startRange));
  s = processor(s, "%end%", String(endRange));
  s = processor(s, "%time%", current);
  if (schedulerState == "on") {
    s = processor(s, "%schedulerState%", "off");
    s = processor(s, "%schedulerT%", "on");
  }else if (schedulerState == "off") {
    s = processor(s, "%schedulerState%", "on");
    s = processor(s, "%schedulerT%", "off");
  }
  int r = 0;

  if (server.arg("scheduler") != "") {
    r = 1;
  }else if(server.arg("start") != "" && server.arg("end") != ""){
    r = 1;
  }else{
    r = 0;
  }

  if (r == 1) {
    String state = server.arg("scheduler");
    String start = server.arg("start");
    String endS = server.arg("end");

    if (state != "") {
      schedulerState = state;
    }else{
      writeFile(SPIFFS, "/start.txt", start.c_str());
      writeFile(SPIFFS, "/end.txt", endS.c_str());
      reboot = true;
    }
  }

  server.send(200, "text/html", s);
}

void relayHandle() {
  String s = relayHTML;
  String c = headerPage;
  c = processor(c, "%title%", "Relay Control");
  s = processor(s, "%header%", c);
  int r = 0;

  if (relay1State == "on"){
    s = processor(s, "{r1State}", "off");
    s = processor(s, "{r1t}", "on");
  }
  if (relay1State == "off") {
    s = processor(s, "{r1State}", "on");
    s = processor(s, "{r1t}", "off");
  }

  if (relay2State == "on"){
    s = processor(s, "{r2State}", "off");
    s = processor(s, "{r2t}", "on");
  }
  if (relay2State == "off"){
    s = processor(s, "{r2State}", "on");
    s = processor(s, "{r2t}", "off");
  }

  if (relay3State == "on"){
    s = processor(s, "{r3State}", "off");
    s = processor(s, "{r3t}", "on");
  }
  if (relay3State == "off") {
    s = processor(s, "{r3State}", "on");
    s = processor(s, "{r3t}", "off");
  }

  if (relay4State == "on"){
    s = processor(s, "{r4State}", "off");
    s = processor(s, "{r4t}", "on");
  }
  if (relay4State == "off") {
    s = processor(s, "{r4State}", "on");
    s = processor(s, "{r4t}", "off");
  }

  if (relayAll == "on"){
    s = processor(s, "{rA}", "off");
    s = processor(s, "{rat}", "on");
  }
  if (relayAll == "off") {
    s = processor(s, "{rA}", "on");
    s = processor(s, "{rat}", "off");
  }

  if (telegramStatus == "on") {
    s = processor(s, "{telegramStatus}", "off");
    s = processor(s, "{teleT}", "on");
  }
  if (telegramStatus == "off") {
    s = processor(s, "{telegramStatus}", "on");
    s = processor(s, "{teleT}", "off");
  }


  if (server.arg("r1") == "on") {
    relay1State = "on";
    digitalWrite(relay1, LOW);
    r = 1;
  }else if (server.arg("r1") == "off"){
    relay1State = "off";
    digitalWrite(relay1, HIGH);
    r = 1;
  }
  
  else if (server.arg("r2") == "on") {
    relay2State = "on";
    digitalWrite(relay2, LOW);
    r = 1;
  }else if (server.arg("r2") == "off"){
    relay2State = "off";
    digitalWrite(relay2, HIGH);
    r = 1;
  }
  
  else if (server.arg("r3") == "on") {
    relay3State = "on";
    digitalWrite(relay3, LOW);
    r = 1;
  }else if (server.arg("r3") == "off"){
    relay3State = "off";
    digitalWrite(relay3, HIGH);
    r = 1;
  }
  
  else if (server.arg("r4") == "on") {
    relay4State = "on";
    digitalWrite(relay4, LOW);
    r = 1;
  }else if (server.arg("r4") == "off"){
    relay4State = "off";
    digitalWrite(relay4, HIGH);
    r = 1;
  }else if (server.arg("all") == "on"){
    relayAll = "on";
    relay1State = "on";
    relay2State = "on";
    relay3State = "on";
    relay4State = "on";
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, LOW);
    digitalWrite(relay3, LOW);
    digitalWrite(relay4, LOW);
    r = 1;
  }else if (server.arg("all") == "off"){
    relayAll = "off";
    relay1State = "off";
    relay2State = "off";
    relay3State = "off";
    relay4State = "off";
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    digitalWrite(relay3, HIGH);
    digitalWrite(relay4, HIGH);
    r = 1;
  }else if (server.arg("tele") == "on"){
    telegramStatus = "on";
    r = 1;
  }else if(server.arg("tele") == "off"){
    telegramStatus = "off";
    r = 1;
  }

  if (r == 1) {
    s = processor(s, "<con></con>", "<script>location.href=\"/relay\"</script>");
  }
  
  server.send(200, "text/html", s);
}




void serverInit() {
  
  
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(0, INPUT);
  
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
  digitalWrite(BUILTIN_LED, LOW);

  server.on("/", rootHandle);
  server.on("/relay", relayHandle);
  server.on("/wifi", wifi);
  server.on("/wifi-submit", wifiHandle);
  server.on("/ap-submit", apHandle);
  server.on("/api", apiHandle);
  server.on("/scheduler", schedulerHandle);
  
  server.begin();

  
}


// =====================================
// Blynk
// =====================================


// https://blynk.cloud/external/api/update?token={token}&{pin}={value}


BLYNK_WRITE(V0) {
  int val = param.asInt();
  if (val == 1) {
    digitalWrite(relay1, LOW);
    relay1State = "on";
  }else{
    digitalWrite(relay1, HIGH);
    relay1State = "off";
  }
}

BLYNK_WRITE(V1) {
  int val = param.asInt();
  if (val == 1) {
    digitalWrite(relay2, LOW);
    relay2State = "on";
  }else{
    digitalWrite(relay2, HIGH);
    relay2State = "off";
  }
}

BLYNK_WRITE(V2) {
  int val = param.asInt();
  if (val == 1) {
    digitalWrite(relay3, LOW);
    relay3State = "on";
  }else{
    digitalWrite(relay3, HIGH);
    relay3State = "off";
  }
}

BLYNK_WRITE(V3) {
  int val = param.asInt();
  if (val == 1) {
    digitalWrite(relay4, LOW);
    relay4State = "on";
  }else{
    digitalWrite(relay4, HIGH);
    relay4State = "off";
  }
}

//BLYNK_CONNECTED() {
//  digitalWrite(BUILTIN_LED, HIGH);
//}

WidgetLCD lcd(V4);
WidgetLCD lcd2(V5);
BlynkTimer timer;

void displayStatus() {
  lcd.clear();
  lcd2.clear();
  lcd.print(0,0, "Relay1: " + relay1State);
  lcd.print(0,1, "Relay2: " + relay2State);
  lcd2.print(0,0, "Relay3: " + relay3State);
  lcd2.print(0,1, "Relay4: " + relay4State);
}


// =====================================




// =====================================
// TELEGRAM
// =====================================

CTBot myBot;
CTBotReplyKeyboard myKbd;


// Perintah relay
const String relay1on = "relay 1 on";
const String relay1off = "relay 1 off";

const String relay2on = "relay 2 on";
const String relay2off = "relay 2 off";

const String relay3on = "relay 3 on";
const String relay3off = "relay 3 off";

const String relay4on = "relay 4 on";
const String relay4off = "relay 4 off";

const String sambutanTelegram = "Welcome....";

void telegramInit(){
  myBot.setTelegramToken(TToken);      
  
  if (myBot.testConnection()) {   
     Serial.println("\nKoneksi Ke Telegram BOT Berhasil!"); 
  }

  myKbd.addButton("Relay 1 ON");
  myKbd.addButton("Relay 1 OFF");
  myKbd.addRow();
  myKbd.addButton("Relay 2 ON");
  myKbd.addButton("Relay 2 OFF");
  myKbd.addRow();
  myKbd.addButton("Relay 3 ON");
  myKbd.addButton("Relay 3 OFF");
  myKbd.addRow();
  myKbd.addButton("Relay 4 ON");
  myKbd.addButton("Relay 4 OFF");
  myKbd.addRow();
  myKbd.addButton("Relay ALL ON");
  myKbd.addButton("Relay ALL OFF");
  myKbd.addRow();
  myKbd.addButton("Cek Status");
  myKbd.enableResize();
}

void telegram(){
  TBMessage msg;

  if (myBot.getNewMessage(msg)) {   

    if (msg.text.equalsIgnoreCase("/start")) {
      myBot.sendMessage(msg.sender.id, sambutanTelegram, myKbd);
    }
    
    // relay 1 on
    else if (msg.text.equalsIgnoreCase(relay1on)) {
        if (relay1State == "on") {
          myBot.sendMessage(msg.sender.id, "relay 1 sudah on");
        }else {
          myBot.sendMessage(msg.sender.id, "relay 1 on");
          digitalWrite(relay1, LOW);
          relay1State = "on";
        }
    }

    // relay 1 off
    else if (msg.text.equalsIgnoreCase(relay1off)) {
        if (relay1State == "off") {
          myBot.sendMessage(msg.sender.id, "relay 1 sudah off");
        }else {
          myBot.sendMessage(msg.sender.id, "relay 1 off");
          digitalWrite(relay1, HIGH);
          relay1State = "off";
        }
    }

    // relay 2 on
    else if (msg.text.equalsIgnoreCase(relay2on)) {
        if (relay2State == "on") {
          myBot.sendMessage(msg.sender.id, "relay 2 sudah on");
        }else {
          myBot.sendMessage(msg.sender.id, "relay 2 on");
          digitalWrite(relay2, LOW);
          relay2State = "on";
        }
    }

    // relay 2 off
    else if (msg.text.equalsIgnoreCase(relay2off)) {
        if (relay2State == "off") {
          myBot.sendMessage(msg.sender.id, "relay 2 sudah off");
        }else {
          myBot.sendMessage(msg.sender.id, "relay 2 off");
          digitalWrite(relay2, HIGH);
          relay2State = "off";
        }
    }

    // relay 3 on
    else if (msg.text.equalsIgnoreCase(relay3on)) {
        if (relay3State == "on") {
          myBot.sendMessage(msg.sender.id, "relay 3 sudah on");
        }else {
          myBot.sendMessage(msg.sender.id, "relay 3 on");
          digitalWrite(relay3, LOW);
          relay3State = "on";
        }
    }

    // relay 3 off
    else if (msg.text.equalsIgnoreCase(relay3off)) {
        if (relay3State == "off") {
          myBot.sendMessage(msg.sender.id, "relay 3 sudah off");
        }else {
          myBot.sendMessage(msg.sender.id, "relay 3 off");
          digitalWrite(relay3, HIGH);
          relay3State = "off";
        }
    }

    // relay 4 on
    else if (msg.text.equalsIgnoreCase(relay4on)) {
        if (relay4State == "on") {
          myBot.sendMessage(msg.sender.id, "relay 4 sudah on");
        }else {
          myBot.sendMessage(msg.sender.id, "relay 4 on");
          digitalWrite(relay4, LOW);
          relay4State = "on";
        }
    }

    // relay 4 off
    else if (msg.text.equalsIgnoreCase(relay4off)) {
        if (relay4State == "off") {
          myBot.sendMessage(msg.sender.id, "relay 4 sudah off");
        }else {
          myBot.sendMessage(msg.sender.id, "relay 4 off");
          digitalWrite(relay4, HIGH);
          relay4State = "off";
        }
    }

    // all relay on
    else if (msg.text.equalsIgnoreCase("relay all on")) {
        myBot.sendMessage(msg.sender.id, "Semua relay on");
        digitalWrite(relay1, LOW);
        digitalWrite(relay2, LOW);
        digitalWrite(relay3, LOW);
        digitalWrite(relay4, LOW);
        relay1State = "on";
        relay2State = "on";
        relay3State = "on";
        relay4State = "on";
    }

    // all relay off
    else if (msg.text.equalsIgnoreCase("relay all off")) {
        myBot.sendMessage(msg.sender.id, "Semua relay off");
        digitalWrite(relay1, HIGH);
        digitalWrite(relay2, HIGH);
        digitalWrite(relay3, HIGH);
        digitalWrite(relay4, HIGH);
        relay1State = "off";
        relay2State = "off";
        relay3State = "off";
        relay4State = "off";
    }

    // cek status
    else if (msg.text.equalsIgnoreCase("cek status")) {
      String str = "Status Saat Ini: \n";
      str += "Relay 1:\t" + relay1State + "\n";
      str += "Relay 2:\t" + relay2State + "\n";
      str += "Relay 3:\t" + relay3State + "\n";
      str += "Relay 4:\t" + relay4State + "\n";

      myBot.sendMessage(msg.sender.id, str);
    }
  }
}

// =====================================
// =====================================


void setup() {
  Serial.begin(300);
  
  
  WiFi.mode(WIFI_AP_STA);
  
  
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
  }

  ssid = readFile(SPIFFS, "/ssid.txt");
  pass = readFile(SPIFFS, "/pass.txt");

  ssid_ap = readFile(SPIFFS, "/ssidAP.txt");
  pass_ap = readFile(SPIFFS, "/passAP.txt");

  
  ch = readFile(SPIFFS, "/ch.txt");
  ip = readFile(SPIFFS, "/ip.txt");

  local.fromString(ip);

  BToken = readFile(SPIFFS, "/blynk.txt");
  TToken = readFile(SPIFFS, "/tele.txt");


  WiFi.begin(ssid.c_str(), pass.c_str());
  WiFi.softAPConfig(local, local, netmask);
  if (pass_ap == "NULL") {
    WiFi.softAP(ssid_ap.c_str(), NULL, ch.toInt());
  }else{
    WiFi.softAP(ssid_ap.c_str(), pass_ap.c_str(), ch.toInt());
  }

  startRange = readFile(SPIFFS, "/start.txt").toInt();
  endRange = readFile(SPIFFS, "/end.txt").toInt();

  Blynk.config(BToken.c_str());

  
  
  serverInit();
  telegramInit();

  

  Serial.println("init done");
  for (int i = 0; i < 5; i++){
    delay(500);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(500);
    digitalWrite(BUILTIN_LED, LOW);
  }
  Serial.println(WiFi.softAPIP());
  timer.setInterval(1500L, displayStatus);
}

void loop() {

  server.handleClient();

  if (digitalRead(0) == LOW) {
    resetDefault();
  }
  
  if (WiFi.status() == WL_CONNECTED){
    bool con = Blynk.connected();
    Blynk.run();
    
    timer.run();
    if (telegramStatus == "on") {
      telegram();
    }

    if (con) {
      digitalWrite(BUILTIN_LED, HIGH);
      timeClient.update();
    }
    
  }else{
    digitalWrite(BUILTIN_LED, LOW);
  }

  int jam = timeClient.getHours();
  int menit = timeClient.getMinutes();

  current = timeClient.getFormattedTime();

  int asd;
  if (menit < 10) {
    asd = (String(jam) + "0" + String(menit)).toInt();
  }else{
    asd = (String(jam) + String(menit)).toInt();
  }

  if (schedulerState == "on") {
    if (asd >= startRange && asd <= endRange) {
      relay1State = "on";
      digitalWrite(relay1, LOW);
    }else{
      relay1State = "off";
      digitalWrite(relay1, HIGH);
    }
  }

  
  if(reboot){
    ESP.restart();
  }
  
  
  
}
