#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>
#include "FS.h"
#include "SD_MMC.h"



void writeString(char add, String data)
{
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.write(add + i, data[i]);
  }
  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
  EEPROM.commit();
}

String read_String(char add)
{
  int i;
  char data[100]; //Max 100 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < 500) //Read until null character
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
}
void setup_wifi()
{

  delay(10);
  Serial.println(read_String(0));
  Serial.println(read_String(50));

  String ssid = read_String(0);
  String password = read_String(50);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  //WiFi.begin("Sokrates","suav3m3nt3b3sam3");
  uint8_t WrongSSIDPassCounter = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    WrongSSIDPassCounter += 1;
    if (WrongSSIDPassCounter == 20)
    {
      
      EEPROM.write(100, 1); //CANNOT CONNECT
      EEPROM.commit();
      delay(100);
      ESP.restart();
    }
  }
 

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 void setup_acces_point(const char* ssidd, const char* passwordd){
  
  Serial.println();

  Serial.print("Setting soft-AP ... ");
  boolean result = WiFi.softAP(ssidd, passwordd);
  if(result == true)
  {
    Serial.println("Ready");
  }
  else
  {
    Serial.println("Failed!");
  }
  
}
const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>Watering lolololo</h2>
<form action="/action_page1">
  <label for="SSID">SSID:</label>
  <input type="text" id="SSID" name="SSID"> <br>
  <label for="Password">Password:</label>
  <input type="text" id="Password" name="Password"><br>
  <label for="Token">Token</label>
  <input type="text" id="Token" name="Token"><br>
  <input type="submit" value="Submit"><br><br>
 </form>

</body>

</html>)rawliteral";

void appendFile(fs::FS &fs, String path, String message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
}

String printFiles(File dir, int numTabs)
{

  String output;
  while (true)
  {
    File entry =  dir.openNextFile();
    if (! entry)
    {
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++)
    {
      Serial.print('\t');
    }
    ///Serial.println(entry.name());
    output += entry.name();
    output += "\n";
    entry.close();
  }
  return output;
}