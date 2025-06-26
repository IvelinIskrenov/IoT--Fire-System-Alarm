#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Si7021.h>
#include <Adafruit_NeoPixel.h>

// WI-FI (hotspot) params
const char* apSSID     = "ESP32-AP";
const char* apPassword = "iot2025";

// Pins
// senesor for temperature
#define GY_SDA       27
#define GY_SCL       26
// buzzer
#define BUZZER_PIN   17
// led ring with 16 led diods
#define LED_PIN      22
#define LED_COUNT    16


WebServer   server(80); // 80 is the port for web
Adafruit_Si7021 sensor;
Adafruit_NeoPixel ring(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// returns the current color of the diod
String ledColor(int idx, int g, int y, int r) {
  if (idx < g)        return "rgb(0,255,0)";
  else if (idx < g+y) return "rgb(255,191,0)";
  else if (idx < g+y+r) return "rgb(255,0,0)";
  else                return "rgb(200,200,200)";
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // === Стартиране на Wi-Fi AP ===
  WiFi.softAP(apSSID, apPassword); // Create the Wifi name with password
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP()); // return the adr.

  // === HTTP маршрут “/” ===
  server.on("/", [](){
    float t = sensor.readTemperature();

    // Calculate the leds
    int g=0,y=0,r=0;
    if      (t>=40.0){ g=6; y=6; r=4; }
    else if (t>=38.0){ g=6; y=6; r=2; }
    else if (t>=36.0){ g=6; y=6; }
    else if (t>=34.0){ g=6; y=4; }
    else if (t>=32.0){ g=6; y=2; }
    else if (t>=30.0){ g=6; }
    else if (t>=28.0){ g=4; }
    else if (t>=26.0){ g=2; }

    // Генерираме HTML с кръгчета
    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>"
                  "<title>ESP32 Fire/System alarm</title>"
                  "<script>setTimeout(()=>location.reload(),1200);</script>" // reloading the page
                  "<style>"
                  "body{background:#fff;font-family:Arial,sans-serif;"
                  "text-align:center;padding:20px;}"
                  ".led{display:inline-block;width:25px;height:25px;"
                  "border-radius:50%;margin:5px;}"
                  "</style></head><body>";
    html += "<h1>Temp: " + String(t,2) + " &deg;C</h1><div>";
    for (int i = 0; i < LED_COUNT; i++) {
      String c = ledColor(i, g, y, r);
      html += "<span class='led' style='background:" + c + ";'></span>"; // add circles
    }
    html += "</div></body></html>";

    server.send(200, "text/html", html); // (html status code, sending html content, content)
  });
  server.begin(); //starts listening to requests
  Serial.println("HTTP server started");

 
 // start connection with the sensor
  Wire.begin(GY_SDA, GY_SCL);
  if (!sensor.begin()) {
    Serial.println("ERROR: Si7021 init failed!");
    while (1) delay(10);
  }
  pinMode(BUZZER_PIN, OUTPUT);

  ring.begin();
  ring.show();  
  ring.setBrightness(50);
}

void loop() {
  server.handleClient();

  float temp = sensor.readTemperature();
  Serial.print("Temp: "); Serial.println(temp,2);

  // Buzzer sound above 40 C.
  if (temp > 40.0) tone(BUZZER_PIN,1000);
  else             noTone(BUZZER_PIN);

  // turn off all diods
  for (int i = 0; i < LED_COUNT; i++) {
    ring.setPixelColor(i, 0);
  }

  // Calculate the led lamps
  int g=0,y=0,r=0;
  if      (temp>=40.0){ g=6; y=6; r=4; }
  else if (temp>=38.0){ g=6; y=6; r=2; }
  else if (temp>=36.0){ g=6; y=6; }
  else if (temp>=34.0){ g=6; y=4; }
  else if (temp>=32.0){ g=6; y=2; }
  else if (temp>=30.0){ g=6; }
  else if (temp>=28.0){ g=4; }
  else if (temp>=26.0){ g=2; }

  // set the leds
  for (int i = 0; i < g; i++)          ring.setPixelColor(i, ring.Color(0,255,0));
  for (int i = g; i < g+y; i++)        ring.setPixelColor(i, ring.Color(255,191,0));
  for (int i = g+y; i < g+y+r; i++)    ring.setPixelColor(i, ring.Color(255,0,0));

  ring.show();

  // set the red leds blinking
  if (r > 0) {
    delay(300);
    for (int i = g+y; i < g+y+r; i++) ring.setPixelColor(i, 0); // turn off the diods (for every i diod set rgb to 0) 
    ring.show();
    delay(300);
  } else {
    delay(1300);
  }
}