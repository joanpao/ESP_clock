// ----------------------------------------
//  ESP32_OLEDaf_NTPclock
//  Author: EA5JTT
//  Data: 20250118
//  Tested: 
//  - Lilygo ESP32 LoRa T3_V1.6.1 (ESP PICO-D4 in Arduino iDE)
//  - ESP32-WROMM-32D + OLED 0.9  (ESP WROOM DA in Arduino iDE))
//  Procedure:
//  - Read Internet NTP time
//  - Display time in OLED screen
//  Next Projects:
//  - Read GPS/GNSS time with ESP32 LoRa GPS/GNSS board
//  - LoRa transmision time 
//  More projects and ideeas in https://sonotrigger-software.blogspot.com/
//  You can use this program, but you must keep this announcement
// ----------------------------------------
//
// Include section: wifi, time,spi, wire, adafruit OLED
#include <WiFi.h>
#include <time.h>
struct tm timeinfo;
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Inicialize section: OLED 0.95 B/W
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Define section
// Debug ON = true Debug OFF = false 
#define USE_SERIAL true
// WIFI
//const char* wifi_ssid = "<WiFi SSID here>";   // WiFi SSID
//const char* wifi_pwd = "<WiFi password here>";    // WiFi password
const char* wifi_ssid = "TP-Link_7254";   // WiFi SSID
const char* wifi_pwd = "63341379";    // WiFi password
// NTP https://www.ntppool.org/es/use.html
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;       // GMT +/- in seconds
const int   daylightOffset_sec = 0;  // Daylight savings 0|3600
// Global variables
long every1s;
long every10s;

//  Serial print (debug)
void debug(String text, bool newline = true)
{
  #if USE_SERIAL
    if (newline) { Serial.println(text); }
    else  { Serial.print(text); }
  #endif
}
//          Setup   section                                                         
void setup()
{
  // Serial monitor 9600, if debuger=true and serial not OK waiting
  #if USE_SERIAL
    delay(1000);
    Serial.begin(9600);
    while (!Serial) {
      ; 
    }
    delay(500);
    debug(" Serial monitor OK");
  #endif
  // OLED screnn
  // if OLED not OK waiting forever
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    debug("SSD1306 allocation failed");
    for(;;); 
    display.clearDisplay();
  }
  // if OLED = OK display LOGO
  display.clearDisplay();             // Sreen clear
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        //  white color
  display.setCursor(10,32);           // Draw white text X=10,Y=32
  display.println(F("NTP Clock"));    // TEXT
  display.display();
  // WiFi and NTPconexion, if NOT OK then restart ESP32
  if (WiFiConnect())  
  {
    NTPConnect();  
  }
  else
  {
    delay(5000);
    ESP.restart();  
  }
  every1s = millis();
  every10s = millis();
}

// MAIN Section
void loop()
{
  // Every 1s
  if (millis() > every1s + 1000)
  {
    // Print the local time
    display.clearDisplay();
    printLocalTime();
    every1s = millis();
  }
  // Every 10s check wiFi if not OK reconnect
  if (millis() > every10s + 10000)
  {
    display.clearDisplay();
    if (WiFi.status() != WL_CONNECTED)
    {

      debug("WiFi down, trying to re-connect");
      WiFi.reconnect();
    }
    every10s = millis();
  }
}

// Funtion section
// Connect to AP
bool WiFiConnect()
{
  int NAcounts = 0;
  WiFi.begin(wifi_ssid, wifi_pwd); // SSID, password
    
  #if USE_SERIAL
    WiFi.printDiag(Serial);
  #endif

  debug("Connecting to WiFi ", false);
  while (WiFi.status() != WL_CONNECTED && (NAcounts < 10))
  {
    debug(".", false);
    delay(500);
    NAcounts++;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    debug(" could not connect");
    return false;
  }
  else
  {
    debug("Connected with IP", false);
    debug(WiFi.localIP().toString().c_str());
    /*
      true: module will try to reestablish lost connection to the AP
      false: module will stay disconnected
      Note: running setAutoReconnect(true) when module is already disconnected will not make it reconnect to the access point. Instead reconnect() should be used.
    */
    WiFi.setAutoReconnect(true);
    // WiFi.persistent(true);
    return true;
  }
}

// Connect to NTP server
bool NTPConnect()
{
  int NAcounts = 0;
  debug("Connecting to NTP server ", false);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  while ( (!getLocalTime(&timeinfo)) && (NAcounts < 3) )
  {
    debug(".", false);
    delay(500);
    NAcounts++;
  }
  if (getLocalTime(&timeinfo))
  {
    debug(" connected");
    return true;
  }
  else
  {
    debug(" could not connect");
    return false;
  }
}

// Print local time on the display
void printLocalTime() {
  // time.h: https://www.tutorialspoint.com/c_standard_library/time_h.htm
  // Time formats: http://www.cplusplus.com/reference/ctime/strftime/
  // https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/

  getLocalTime(&timeinfo);

  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  char timeMinutes[3];
  strftime(timeMinutes, 3, "%M", &timeinfo);
  char timeSeconds[3];
  strftime(timeSeconds, 3, "%S", &timeinfo);

  char dateYear[5];
  strftime(dateYear, 5, "%Y", &timeinfo);
  char dateMonth[3];
  strftime(dateMonth, 3, "%m", &timeinfo);
  char dateDay[3];
  strftime(dateDay, 3, "%d", &timeinfo);

  String timeString = timeHour;
  timeString = timeString + ":";
  timeString = timeString + timeMinutes;
  timeString = timeString + ":";
  timeString = timeString + timeSeconds;

  String dateString = dateDay;
  dateString = dateString + "-";
  dateString = dateString + dateMonth;
  dateString = dateString + "-";
  dateString = dateString + dateYear;

  debug(timeString, false);
  debug(" - ", false);
  debug(dateString);

  // strftime(timePrint,3, "%A, %B %d %Y %H:%M:%S", &timeinfo);

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(15,15);             // Start at top-left corner
  display.println(timeString);

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,45);             // Start at top-left corner
  display.println(dateString);

  
  display.display();
}
