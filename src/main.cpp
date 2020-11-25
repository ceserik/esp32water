//
// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//
#include "FS.h"
#include <mojalib.h>
#include "SD_MMC.h"
#include <ezTime.h>

AsyncWebServer server(80);

File root2;
const char *SSIDforHotspot = "ConfigureESP32";
const char *PasswordforHotspot = "12345678";

const char *PARAM_MESSAGE = "message";

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

void setup()
{
    EEPROM.begin(512);
    Serial.begin(9600);
    if (EEPROM.read(100) == 1) //########################################## FIRST START ################################################
    {
        setup_acces_point(SSIDforHotspot, PasswordforHotspot);
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/html", index_html);
        });

        server.on("/action_page1", HTTP_GET, [](AsyncWebServerRequest *request) {
            String SSIDtoEEPROM = "";
            if (request->hasParam("SSID"))
            {
                SSIDtoEEPROM = request->getParam("SSID")->value();
                writeString(0, SSIDtoEEPROM);
                delay(10);
                Serial.println(SSIDtoEEPROM);
            }
            if (request->hasParam("Password"))
            {
                SSIDtoEEPROM = request->getParam("Password")->value();
                writeString(50, SSIDtoEEPROM);
                delay(10);
                Serial.println(SSIDtoEEPROM);
            }

            // EEPROM.put(0,customVar);
            EEPROM.commit();
            //request->send_P(200, "text/html", index_html);
            EEPROM.write(100, 0);
            delay(100);
            EEPROM.commit();
            delay(100);
            request->send_P(200, "text/html", " <h1>Conecting to WIFI in 3 seconds</h1>");
            delay(3000);
            ESP.restart();
        });
        server.begin();
    }
    else // ################################################## NORMAL OPERATON ##################################
    {
        
        pinMode(4, OUTPUT);

        if (!SD_MMC.begin())
        {
            Serial.println("Failed to mount card");
            //return;                                         neviem co robi tato zmenaaaa
        }

        //File file2 = SD_MMC.open("/test.txt", FILE_READ);     tiez som dao 
        setup_wifi(); //SSID AND PASSWORD from EEPROM adress 0 and 50
        waitForSync();

        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/html", index_html);
        });

        server.on("/incomdata", HTTP_GET, [](AsyncWebServerRequest *request) {
            String TMP = "";
            String NODE = "";
            String MOI = "";
            String HUM = "";
            String out = "";

            if (request->hasParam("NODE"))
            {
                String date = UTC.dateTime("kXdm");
                NODE = "/";
                NODE += request->getParam("NODE")->value();
                NODE += date;
                NODE += ".txt";
            }
            if (request->hasParam("TMP"))
            {
                //File file = SD_MMC.open(NODE, FILE_WRITE);
                TMP = request->getParam("TMP")->value();    
                appendFile(SD_MMC, NODE, TMP += ",");
            }

            if (request->hasParam("HUM"))
            {
                //File file = SD_MMC.open(NODE, FILE_WRITE);
                HUM = request->getParam("HUM")->value();
                appendFile(SD_MMC, NODE, HUM += ",");
            }

            if (request->hasParam("MOI"))
            {
                //File file = SD_MMC.open(NODE, FILE_WRITE);
                MOI = request->getParam("MOI")->value();
                appendFile(SD_MMC, NODE, MOI += ",");
            }
            //timeClient.update();
            appendFile(SD_MMC, NODE, UTC.dateTime());
            appendFile(SD_MMC, NODE, "\n");
            out += NODE;           
            out += TMP;           
            out += HUM;           
            out += MOI;
            request->send(200, "text/plain", out);
        });
        // Send a GET request to <IP>/get?message=<message>
        server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
            String message;

            request->send(200, "text/plain", "Hello, GET: " + message);
        });
        server.on("/sendInfoFiles", HTTP_GET, [](AsyncWebServerRequest *request) {
            
            root2 = SD_MMC.open("/");
            String lolo = printFiles(root2, 0);
            Serial.println(lolo);
            request->send(200, "text/plain", lolo);
            root2.close();
        });

            


        server.on("/sendvalues", HTTP_GET, [](AsyncWebServerRequest *request) {
            String NODE = "/";
            if (request->hasParam("NODE"))
            {
                //File file = SD_MMC.open(NODE, FILE_WRITE);
                NODE += request->getParam("NODE")->value();
                NODE += ".txt";
                File file2 = SD_MMC.open(NODE, FILE_READ);
                String finalString = "";
                while (file2.available())
                {
                    finalString += (char)file2.read();
                }                
                file2.close();
                
                Serial.print(NODE);
                request->send(200, "text/plain", finalString);
                finalString = "";
            }
        });

        server.on("/downloadtest", HTTP_GET, [](AsyncWebServerRequest *request) {
            String message;
            File file2 = SD_MMC.open("/123.txt", FILE_READ);

            if (!file2)
            {
                Serial.println("Opening file to read failed");
                return;
            }
            String finalString = "";
            while (file2.available())
            {
                finalString += (char)file2.read();
            }
            file2.close();
            request->send(200, "text/plain", finalString);
            finalString = "";
        });

        // Send a POST request to <IP>/post with a form field message set to <message>
        server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request) {
            String message;
            if (request->hasParam(PARAM_MESSAGE, true))
            {
                message = request->getParam(PARAM_MESSAGE, true)->value();
            }
            else
            {
                message = "No message sent";
            }
            request->send(200, "text/plain", "Hello, POST: " + message);
        });

        server.onNotFound(notFound);

        server.begin();
    }
}
void loop()
{
}

//192.168.17.39/incomdata?NODE=123&TMP=28&MOI=30&HUM=53