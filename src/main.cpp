#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "secrets.h"

// #define DEBUG // Décommenter pour ajouter des affichages via le moniteur série

// Volets
#define V1_HAUT 23
#define V1_BAS 22
#define V2_HAUT 21
#define V2_BAS 19
#define V3_HAUT 18
#define V3_BAS 5
// Eclairages ON OFF
#define E1 17
#define E2 16
// Eclairages variables
#define EV1 27
#define EV2 26
#define EV3 25
#define EV4 33

#define RAZ_VOLETS 30 // Arrêt des volets après X secondes
#define FREQ_PWM 1000
#define RES_PWM 8

bool e1 = false; // Active LOW donc inverse
bool e2 = false; // Active LOW donc inverse
uint8_t ev1 = 0;
uint8_t ev2 = 0;
uint8_t ev3 = 0;
uint8_t ev4 = 0;
bool v1_monte = false;
bool v1_descend = false;
bool v2_monte = false;
bool v2_descend = false;
bool v3_monte = false;
bool v3_descend = false;
int8_t razV1 = -1;
int8_t razV2 = -1;
int8_t razV3 = -1;
volatile bool interrupt = false;
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onInterrupt()
{
  portENTER_CRITICAL_ISR(&timerMux);
  interrupt = true;
  portEXIT_CRITICAL_ISR(&timerMux);
}

AsyncWebServer server(80);

void ledcAnalogWrite(uint8_t channel, uint8_t value, uint8_t valueMax = 100) {
  uint8_t duty = (255 / valueMax) * min(value, valueMax);
  ledcWrite(channel, duty);
}

void setup() {
#ifdef DEBUG
    // Initialisation Serial
    Serial.begin(115200);
    delay(10);
    Serial.println();
#endif

  
  // Initialisation des Pins
  pinMode(V1_BAS, OUTPUT);
  digitalWrite(V1_BAS, HIGH);
  pinMode(V1_HAUT, OUTPUT);
  digitalWrite(V1_HAUT, HIGH);
  pinMode(V2_BAS, OUTPUT);
  digitalWrite(V2_BAS, HIGH);
  pinMode(V2_HAUT, OUTPUT);
  digitalWrite(V2_HAUT, HIGH);
  pinMode(V3_BAS, OUTPUT);
  digitalWrite(V3_BAS, HIGH);
  pinMode(V3_HAUT, OUTPUT);
  digitalWrite(V3_HAUT, HIGH);
  pinMode(E1, OUTPUT);
  digitalWrite(E1, HIGH);
  pinMode(E2, OUTPUT);
  digitalWrite(E2, HIGH);
  pinMode(EV1, OUTPUT);
  ledcSetup(0, FREQ_PWM, RES_PWM);
  ledcAttachPin(EV1, 0);
  pinMode(EV2, OUTPUT);
  ledcSetup(1, FREQ_PWM, RES_PWM);
  ledcAttachPin(EV2, 1);
  pinMode(EV3, OUTPUT);
  ledcSetup(2, FREQ_PWM, RES_PWM);
  ledcAttachPin(EV3, 2);
  pinMode(EV4, OUTPUT);
  ledcSetup(3, FREQ_PWM, RES_PWM);
  ledcAttachPin(EV4, 3);

  // Initialisation du timer pour les volets
  timer = timerBegin(2, 80, true); // Utilisation du timer n°3, prescaler de 80 => 1 000 000 tics/secondes car CPU à 80 MHz
  timerAttachInterrupt(timer, &onInterrupt, true);
  timerAlarmWrite(timer, 1000000, true); // Interrupt toutes les secondes
  timerAlarmEnable(timer);

  // Initialisation SPIFFS pour les pages web
  if(!SPIFFS.begin())
  {
#ifdef DEBUG
    Serial.println("Erreur SPIFFS...");
#endif
    return;
  }

  // Initialisation WIFI
  WiFi.mode(WIFI_STA);
  //WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname("Po_esp32");
  WiFi.begin(ssid, password);
#ifdef DEBUG
    Serial.print("Connexion à ");
    Serial.print(ssid);
    Serial.print("...");
#endif
	
	while(WiFi.status() != WL_CONNECTED)
	{
#ifdef DEBUG
        Serial.print(".");
#endif
		delay(500);
	}
	
#ifdef DEBUG
      Serial.println("OK");
	  Serial.print("Adresse IP : ");
	  Serial.println(WiFi.localIP());
#endif

  //----------------------------------------------------SERVER
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });

  server.on("/init", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String etatInit = String(e1) + ";" + String(ev1);
#ifdef DEBUG
      Serial.print("Init : ");
      Serial.println(etatInit);
#endif
    request->send(200, "text/plain", etatInit);
  });

  server.on("/e1", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    digitalWrite(E1, e1);
    e1 = !e1;
#ifdef DEBUG
      Serial.print("E1 : ");
      Serial.println(e1);
#endif
    request->send(200);
  });

  server.on("/e2", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    digitalWrite(E2, e2);
    e2 = !e2;
#ifdef DEBUG
      Serial.print("E2 : ");
      Serial.println(e2);
#endif
    request->send(200);
  });

  server.on("/ev1", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if(request->hasParam("v")){
      AsyncWebParameter* p = request->getParam("v");
      ev1 = p->value().toInt();
      if(ev1 < 0) ev1 = 0;
      if(ev1 > 100) ev1 = 100;
      ledcAnalogWrite(0, ev1);
#ifdef DEBUG
        Serial.print("EV1 : ");
        Serial.println(ev1);
#endif
    }
    request->send(200);
  });

  server.on("/ev2", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if(request->hasParam("v")){
      AsyncWebParameter* p = request->getParam("v");
      ev2 = p->value().toInt();
      if(ev2 < 0) ev2 = 0;
      if(ev2 > 100) ev2 = 100;
      ledcAnalogWrite(1, ev2);
#ifdef DEBUG
        Serial.print("EV2 : ");
        Serial.println(ev2);
#endif
    }
    request->send(200);
  });

  server.on("/ev3", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if(request->hasParam("v")){
      AsyncWebParameter* p = request->getParam("v");
      ev3 = p->value().toInt();
      if(ev3 < 0) ev3 = 0;
      if(ev3 > 100) ev3 = 100;
      ledcAnalogWrite(2, ev3);
#ifdef DEBUG
        Serial.print("EV3 : ");
        Serial.println(ev3);
#endif
    }
    request->send(200);
  });

  server.on("/ev4", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if(request->hasParam("v")){
      AsyncWebParameter* p = request->getParam("v");
      ev4 = p->value().toInt();
      if(ev4 < 0) ev4 = 0;
      if(ev4 > 100) ev4 = 100;
      ledcAnalogWrite(3, ev4);
#ifdef DEBUG
        Serial.print("EV4 : ");
        Serial.println(ev4);
#endif
    }
    request->send(200);
  });

  server.on("/v1", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if(request->hasParam("haut")){
      if(v1_descend){
        digitalWrite(V1_BAS, HIGH);
        v1_descend = false;
#ifdef DEBUG
        Serial.println("Arrêt V1 !");
#endif
        delay(500);
      }
      digitalWrite(V1_HAUT, LOW);
      v1_monte = true;
      razV1 = RAZ_VOLETS;
#ifdef DEBUG
        Serial.print("V1 monte pendant ");
        Serial.print(RAZ_VOLETS);
        Serial.println(" secondes !");
#endif
    }
    else if(request->hasParam("bas")){
      if(v1_monte){
        digitalWrite(V1_HAUT, HIGH);
        v1_monte = false;
#ifdef DEBUG
        Serial.println("Arrêt V1 !");
#endif
        delay(500);
      }
      digitalWrite(V1_BAS, LOW);
      v1_descend = true;
      razV1 = RAZ_VOLETS;
#ifdef DEBUG
        Serial.print("V1 descend pendant ");
        Serial.print(RAZ_VOLETS);
        Serial.println(" secondes !");
#endif
    }
    else {
      digitalWrite(V1_BAS, HIGH);
      v1_descend = false;
      digitalWrite(V1_HAUT, HIGH);
      v1_monte = false;
      razV1 = -1;
#ifdef DEBUG
      Serial.println("Arrêt V1 !");
#endif
    }
    request->send(200);
  });

  server.on("/v2", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if(request->hasParam("haut")){
      if(v2_descend){
        digitalWrite(V2_BAS, HIGH);
        v2_descend = false;
#ifdef DEBUG
        Serial.println("Arrêt V2 !");
#endif
        delay(500);
      }
      digitalWrite(V2_HAUT, LOW);
      v2_monte = true;
      razV2 = RAZ_VOLETS;
#ifdef DEBUG
        Serial.print("V2 monte pendant ");
        Serial.print(RAZ_VOLETS);
        Serial.println(" secondes !");
#endif
    }
    else if(request->hasParam("bas")){
      if(v2_monte){
        digitalWrite(V2_HAUT, HIGH);
        v2_monte = false;
#ifdef DEBUG
        Serial.println("Arrêt V2 !");
#endif
        delay(500);
      }
      digitalWrite(V2_BAS, LOW);
      v2_descend = true;
      razV2 = RAZ_VOLETS;
#ifdef DEBUG
        Serial.print("V2 descend pendant ");
        Serial.print(RAZ_VOLETS);
        Serial.println(" secondes !");
#endif
    }
    else {
      digitalWrite(V2_BAS, HIGH);
      v2_descend = false;
      digitalWrite(V2_HAUT, HIGH);
      v2_monte = false;
      razV2 = -1;
#ifdef DEBUG
      Serial.println("Arrêt V2 !");
#endif
    }
    request->send(200);
  });

  server.on("/v3", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if(request->hasParam("haut")){
      if(v3_descend){
        digitalWrite(V3_BAS, HIGH);
        v3_descend = false;
#ifdef DEBUG
        Serial.println("Arrêt V3 !");
#endif
        delay(500);
      }
      digitalWrite(V3_HAUT, LOW);
      v3_monte = true;
      razV3 = RAZ_VOLETS;
#ifdef DEBUG
        Serial.print("V3 monte pendant ");
        Serial.print(RAZ_VOLETS);
        Serial.println(" secondes !");
#endif
    }
    else if(request->hasParam("bas")){
      if(v3_monte){
        digitalWrite(V3_HAUT, HIGH);
        v3_monte = false;
#ifdef DEBUG
        Serial.println("Arrêt V3 !");
#endif
        delay(500);
      }
      digitalWrite(V3_BAS, LOW);
      v3_descend = true;
      razV3 = RAZ_VOLETS;
#ifdef DEBUG
        Serial.print("V3 descend pendant ");
        Serial.print(RAZ_VOLETS);
        Serial.println(" secondes !");
#endif
    }
    else {
      digitalWrite(V3_BAS, HIGH);
      v3_descend = false;
      digitalWrite(V3_HAUT, HIGH);
      v3_monte = false;
      razV3 = -1;
#ifdef DEBUG
      Serial.println("Arrêt V3 !");
#endif
    }
    request->send(200);
  });
  
  server.begin();
#ifdef DEBUG
  Serial.println("ESP 32 actif !");
#endif
}

void loop() {
  if (interrupt) {
    portENTER_CRITICAL(&timerMux);
    interrupt = false;
    portEXIT_CRITICAL(&timerMux);

    if (razV1 == 0) {
      digitalWrite(V1_BAS, HIGH);
      v1_descend = false;
      digitalWrite(V1_HAUT, HIGH);
      v1_monte = false;
      razV1--;
#ifdef DEBUG
      Serial.println("Arrêt V1 !");
#endif
    }
    else if (razV1 > 0) razV1--;

    if (razV2 == 0) {
      digitalWrite(V2_BAS, HIGH);
      v2_descend = false;
      digitalWrite(V2_HAUT, HIGH);
      v2_monte = false;
      razV2--;
#ifdef DEBUG
      Serial.println("Arrêt V2 !");
#endif
    }
    else if (razV2 > 0) razV2--;

    if (razV3 == 0) {
      digitalWrite(V3_BAS, HIGH);
      v3_descend = false;
      digitalWrite(V3_HAUT, HIGH);
      v3_monte = false;
      razV3--;
#ifdef DEBUG
      Serial.println("Arrêt V3 !");
#endif
    }
    else if (razV3 > 0) razV3--;
  }

  while (WiFi.status() != WL_CONNECTED) {
#ifdef DEBUG
    Serial.println("Reconnexion WIFI !");
#endif
    WiFi.disconnect();
    WiFi.reconnect();
    delay(500);
  }
}
