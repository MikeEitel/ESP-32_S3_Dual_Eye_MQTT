// DualEye LCD Module (2x GC9D01, 160x160) for ESP32 boards using Arduino_GFX.
// Board target and pin map are selected automatically for ESP32-S3 or classic ESP32.
// Tutorial : https://youtu.be/thkDA29aLhQ

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <pgmspace.h>

#include <cstdlib>
#include <WiFi.h>
#include <PubSubClient.h>

//#define Me     // If defined ( Me .. MeIOT ..  ..   ) use private network for testing, otherwise use IOT standard
//#define TEST    // Testmodus
#define OTA       // If defined use OTA programming posibility

/* XIAO ESP32-S3 normal board layout
#define MyMOSI           9    // GPIO pin for HSPI MOSI
#define MyMISO          -1    // GPIO pin for HSPI MISO not used
#define MySCK            7    // GPIO pin for HSPI clock
#define DC               8
#define CS1              1 
#define CS2              2 
#define RST1             3
#define RST2             4 
#define LED_BUILTIN     21
*/

// This files contain the device definitions for different networks
// In the credential files the fixed IP has to be defined
#if defined(M)                       
    #include <Me_credentials.h>      
#else
    #include <credentials.h>         
#endif
// Setup the background classes  
WiFiClient   espClient;                       // Get Wifi access
PubSubClient mqttclient;                      // MQTT protokol handler


// Declare global variables and constants
unsigned long currentMillis;                  // Actual timer 
unsigned long prevRMQTTMillis = 2764472319;   // Stores last MQTT time value was published 2764472319->FASTER START
unsigned long prevSMQTTMillis = 2764472319;   // Stores last MQTT time value was published 2764472319->FASTER START
unsigned long prevMinMillis = 2764472319;     // Stores last minutes time value ->  2764472319->FASTER START
const int readSensorsinterval  = 3000;        // How often the sensors are read

String Sendme;                                // Used for clear text messages in MQTT
String MySensors;                             // A list of usable sensors
char debug_buf[96];                           // Placeholder for MQTT text messages
int receivedlenght;                           // How long is mqtt message
char lastreceived;                            // Stores the last received status
char receivedChar[10];                        //  = "";
bool received;                                // Actual received status
int watchdogW = 1;                            // Counter if there is no wifi connection
int watchdogM = 1;                            // Counter if there is no MQTT connection
int mqttstatus;                               // Helper to see whats going on
bool watchdog = true;                         // Signal via mqtt that device is still ok
bool statusreset = false;                     // Used to minimize error 0 sendouts
int looped = 1;                               // Loop counter as debug helper

uint Device_Set = 0x00;                       // Commandstatus for all backlights set by mqtt
uint Crumb_1 = 0;                             // Commandstatus for backlights 
uint Crumb_2 = 0;                             // Commandstatus for wink / disable eye
uint Crumb_3 = 0;                             // Commandstatus mode modi
uint Crumb_4 = 0;                             // Commandstatus for spare 2
bool wink[2] = {true, true};                  // A adition to possible wink pin 1 and 2

// XXXXXXXXXXXXXXXXXXXXXXXX ORG PROGRAM  START XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

// Select one eye style by uncommenting exactly one line below.
// If none are uncommented, default_full_screen is used.
#define EYE_STYLE_DEFAULT_NORMAL
// #define EYE_STYLE_DEFAULT_FULL_SCREEN
// #define EYE_STYLE_CAT_NORMAL
// #define EYE_STYLE_CAT_FULL_SCREEN
// #define EYE_STYLE_DOE_NORMAL
// #define EYE_STYLE_DOE_FULL_SCREEN
// #define EYE_STYLE_DRAGON_NORMAL
// #define EYE_STYLE_DRAGON_FULL_SCREEN
// #define EYE_STYLE_GOAT_NORMAL
// #define EYE_STYLE_GOAT_FULL_SCREEN
//            #define EYE_STYLE_NAUGA_NORMAL
// #define EYE_STYLE_NAUGA_FULL_SCREEN
// #define EYE_STYLE_NEWT_NORMAL
// #define EYE_STYLE_NEWT_FULL_SCREEN
// #define EYE_STYLE_NO_SCLERA_NORMAL
// #define EYE_STYLE_NO_SCLERA_FULL_SCREEN
//      #define EYE_STYLE_OWL_NORMAL
// #define EYE_STYLE_OWL_FULL_SCREEN
// #define EYE_STYLE_TERMINATOR_NORMAL
// #define EYE_STYLE_TERMINATOR_FULL_SCREEN

// Select one mood preset by uncommenting exactly one line below.
// If none are uncommented, curious is used.
#define MOOD_SLEEPY
// #define MOOD_SURPRISED
// #define MOOD_SCARED
// #define MOOD_CURIOUS

#if (defined(EYE_STYLE_DEFAULT_NORMAL) + defined(EYE_STYLE_DEFAULT_FULL_SCREEN) + defined(EYE_STYLE_CAT_NORMAL) + defined(EYE_STYLE_CAT_FULL_SCREEN) + defined(EYE_STYLE_DOE_NORMAL) + defined(EYE_STYLE_DOE_FULL_SCREEN) + defined(EYE_STYLE_DRAGON_NORMAL) + defined(EYE_STYLE_DRAGON_FULL_SCREEN) + defined(EYE_STYLE_GOAT_NORMAL) + defined(EYE_STYLE_GOAT_FULL_SCREEN) + defined(EYE_STYLE_NAUGA_NORMAL) + defined(EYE_STYLE_NAUGA_FULL_SCREEN) + defined(EYE_STYLE_NEWT_NORMAL) + defined(EYE_STYLE_NEWT_FULL_SCREEN) + defined(EYE_STYLE_NO_SCLERA_NORMAL) + defined(EYE_STYLE_NO_SCLERA_FULL_SCREEN) + defined(EYE_STYLE_OWL_NORMAL) + defined(EYE_STYLE_OWL_FULL_SCREEN) + defined(EYE_STYLE_TERMINATOR_NORMAL) + defined(EYE_STYLE_TERMINATOR_FULL_SCREEN)) > 1
#error "Uncomment only one EYE_STYLE_* line."
#endif

#if defined(EYE_STYLE_DEFAULT_NORMAL)
#include "data/defaultEye_normal.h"
#elif  defined(EYE_STYLE_DEFAULT_FULL_SCREEN)
#include "data/defaultEye_full_screen.h"
#elif defined(EYE_STYLE_CAT_NORMAL)
#include "data/catEye_normal.h"
#elif defined(EYE_STYLE_CAT_FULL_SCREEN)
#include "data/catEye_full_screen.h"
#elif defined(EYE_STYLE_DOE_NORMAL)
#include "data/doeEye_normal.h"
#elif defined(EYE_STYLE_DOE_FULL_SCREEN)
#include "data/doeEye_full_screen.h"
#elif defined(EYE_STYLE_DRAGON_NORMAL)
#include "data/dragonEye_normal.h"
#elif defined(EYE_STYLE_DRAGON_FULL_SCREEN)
#include "data/dragonEye_full_screen.h"
#elif defined(EYE_STYLE_GOAT_NORMAL)
#include "data/goatEye_normal.h"
#elif defined(EYE_STYLE_GOAT_FULL_SCREEN)
#include "data/goatEye_full_screen.h"
#elif defined(EYE_STYLE_NAUGA_NORMAL)
#include "data/naugaEye_normal.h"
#elif defined(EYE_STYLE_NAUGA_FULL_SCREEN)
#include "data/naugaEye_full_screen.h"
#elif defined(EYE_STYLE_NEWT_NORMAL)
#include "data/newtEye_normal.h"
#elif defined(EYE_STYLE_NEWT_FULL_SCREEN)
#include "data/newtEye_full_screen.h"
#elif defined(EYE_STYLE_NO_SCLERA_NORMAL)
#include "data/noScleraEye_normal.h"
#elif defined(EYE_STYLE_NO_SCLERA_FULL_SCREEN)
#include "data/noScleraEye_full_screen.h"
#elif defined(EYE_STYLE_OWL_NORMAL)
#include "data/owlEye_normal.h"
#elif defined(EYE_STYLE_OWL_FULL_SCREEN)
#include "data/owlEye_full_screen.h"
#elif defined(EYE_STYLE_TERMINATOR_NORMAL)
#include "data/terminatorEye_normal.h"
#elif defined(EYE_STYLE_TERMINATOR_FULL_SCREEN)
#else
#include "data/defaultEye_full_screen.h"
#endif


#if (defined(MOOD_SLEEPY) + defined(MOOD_SURPRISED) + defined(MOOD_SCARED) + defined(MOOD_CURIOUS)) > 1
#error "Uncomment only one MOOD_* line."
#endif

namespace BoardTarget
{
#if defined(CONFIG_IDF_TARGET_ESP32S3)
  constexpr const char *kName = "ESP32-S3";
#elif defined(CONFIG_IDF_TARGET_ESP32)
  constexpr const char *kName = "ESP32";
#else
  #error "This sketch currently supports only ESP32-S3 and classic ESP32 targets."
#endif
} // namespace BoardTarget

namespace Pins{
#if defined(CONFIG_IDF_TARGET_ESP32S3)
  /* XIAO ESP32-S3 normal board layout
  #define MyMOSI           9    // GPIO pin for HSPI MOSI
  #define MyMISO          -1    // GPIO pin for HSPI MISO not used
  #define MySCK            7    // GPIO pin for HSPI clock
  #define DC               8
  #define CS1              1 
  #define CS2              2 
  #define RST1             3
  #define RST2             4 
  #define BL1              5
  #define BL2              6
  #define BLINK           43
  #define SPARE           44*/
  constexpr int8_t kDin = 9;            // SPI data output from the active ESP32 target to both displays.
  constexpr int8_t kSclk = 7;           // SPI clock shared by both displays.
  constexpr int8_t kDc = 8;
  //8; // Data/command control pin shared by both displays.

  constexpr int8_t kLeftCs = 1;         // Chip-select pin for the left display.
  constexpr int8_t kRightCs = 2;        // Chip-select pin for the right display.

  constexpr int8_t kLeftReset = 3;      // Hardware reset pin for the left display.
  constexpr int8_t kRightReset = 4;     // Hardware reset pin for the right display.

  constexpr int8_t kLeftBacklight = 5;  // Backlight control pin for the left display.
  constexpr int8_t kRightBacklight = 6; // Backlight control pin for the right display.
#elif defined(CONFIG_IDF_TARGET_ESP32)
  constexpr int8_t kDin = 23;           // SPI data output from the active ESP32 target to both displays.
  constexpr int8_t kSclk = 18;          // SPI clock shared by both displays.
  constexpr int8_t kDc = 27;            // Data/command control pin shared by both displays.

  constexpr int8_t kLeftCs = 21;        // Chip-select pin for the left display.
  constexpr int8_t kRightCs = 22;       // Chip-select pin for the right display.

  constexpr int8_t kLeftReset = 16;     // Hardware reset pin for the left display.
  constexpr int8_t kRightReset = 17;    // Hardware reset pin for the right display.

  constexpr int8_t kLeftBacklight = 32; // Backlight control pin for the left display.
  constexpr int8_t kRightBacklight = 33; // Backlight control pin for the right display.
#endif
} // namespace Pins

namespace DisplayConfig
{
constexpr uint16_t kPanelWidth = 160;         // Physical width of each round display panel in pixels.
constexpr uint16_t kPanelHeight = 160;        // Physical height of each round display panel in pixels.
constexpr uint8_t kLeftRotation = 1;          // Rotation used for the left display panel.
constexpr uint8_t kRightRotation = 3;         // Rotation used for the right display panel.
constexpr int16_t kEyeXOffset = 20;           // Horizontal offset that positions the eye image on each screen.
constexpr uint32_t kSpiFrequency = 40000000;  // SPI bus speed used to send pixels to the displays.
constexpr uint16_t kPanelBackground = 0x0000; // Background color used when clearing the panels.
} // namespace DisplayConfig

constexpr uint8_t NUM_EYES = 2;        // Number of eye panels driven by this sketch.
constexpr uint32_t SERIAL_BAUD = 115200; // Serial monitor speed used for status messages.
constexpr int8_t BLINK_PIN = 43;       // Optional shared blink input pin, disabled when set to -1.
constexpr int8_t LEFT_WINK_PIN = -1;   // Optional wink input pin for the left eye, disabled when set to -1.
constexpr int8_t RIGHT_WINK_PIN = -2;  // Optional wink input pin for the right eye, disabled when set to -1.

constexpr uint16_t kDefaultIrisMin = 90;  // Default minimum iris size when the eye data does not override it.
constexpr uint16_t kDefaultIrisMax = 130; // Default maximum iris size when the eye data does not override it.

#ifdef IRIS_MIN
constexpr uint16_t kIrisMin = IRIS_MIN;
#undef IRIS_MIN
#else
constexpr uint16_t kIrisMin = kDefaultIrisMin;
#endif

#ifdef IRIS_MAX
constexpr uint16_t kIrisMax = IRIS_MAX;
#undef IRIS_MAX
#else
constexpr uint16_t kIrisMax = kDefaultIrisMax;
#endif

constexpr size_t PIXEL_BUFFER_SIZE = 1024; // Number of pixels buffered before sending a batch to the display.

constexpr uint32_t BLINK_MIN_US = 36000;         // Shortest automatic blink duration in microseconds.
constexpr uint32_t BLINK_MAX_US = 72000;         // Longest automatic blink duration in microseconds.
constexpr uint32_t MOVE_PAUSE_MAX_US = 3000000;  // Maximum pause between eye movements in microseconds.
constexpr uint32_t MOVE_TIME_MIN_US = 72000;     // Shortest eye movement duration in microseconds.
constexpr uint32_t MOVE_TIME_MAX_US = 144000;    // Longest eye movement duration in microseconds.
constexpr uint32_t IRIS_TRANSITION_US = 10000000; // Time for one iris size transition in microseconds.

#ifdef EYE_FILL_PANEL
constexpr uint16_t kRenderWidth = DisplayConfig::kPanelWidth;   // Output width when the selected eye should fill the panel.
constexpr uint16_t kRenderHeight = DisplayConfig::kPanelHeight; // Output height when the selected eye should fill the panel.
constexpr int16_t kRenderXOffset = 0;                           // Full-screen styles start at the left edge of the panel.
#else
constexpr uint16_t kRenderWidth = SCREEN_WIDTH;                 // Output width from the selected eye asset.
constexpr uint16_t kRenderHeight = SCREEN_HEIGHT;               // Output height from the selected eye asset.
constexpr int16_t kRenderXOffset = DisplayConfig::kEyeXOffset;  // Horizontal offset for non-full-screen eye assets.
#endif

struct MoodPreset
{
  const char *name;
  uint8_t irisMinPercent;
  uint8_t irisMaxPercent;
  uint32_t blinkMinUs;
  uint32_t blinkMaxUs;
  uint32_t blinkGapMinUs;
  uint32_t blinkGapMaxUs;
  uint32_t movePauseMaxUs;
  uint32_t moveTimeMinUs;
  uint32_t moveTimeMaxUs;
  uint32_t irisTransitionUs;
  int16_t upperLidOffset;
  int16_t lowerLidOffset;
};

MoodPreset kMoodSleepy = {"sleepy", 45, 75, 70000, 140000, 1000000, 2500000, 5000000, 120000, 240000, 16000000, 35, 15};
MoodPreset kMoodSurprised = {"surprised", 20, 95, 25000, 50000, 3500000, 7000000, 1600000, 50000, 100000, 5000000, -20, -20};
MoodPreset kMoodScared = {"scared", 10, 45, 25000, 45000, 600000, 1800000, 800000, 40000, 90000, 3000000, -35, -35};
MoodPreset kMoodCurious = {"curious", 30, 80, BLINK_MIN_US, BLINK_MAX_US, 1500000, 3500000, MOVE_PAUSE_MAX_US, MOVE_TIME_MIN_US, MOVE_TIME_MAX_US, IRIS_TRANSITION_US, -5, -5};

#if defined(MOOD_SLEEPY)
    MoodPreset kActiveMood = kMoodSleepy;
#elif defined(MOOD_SURPRISED)
MoodPreset kActiveMood = kMoodSurprised;
#elif defined(MOOD_SCARED)
  MoodPreset kActiveMood = kMoodScared;
#else
  MoodPreset kActiveMood = kMoodCurious;
#endif

uint16_t kBaseIrisRange = (kIrisMax > kIrisMin) ? static_cast<uint16_t>(kIrisMax - kIrisMin) : 1;
uint16_t kMoodIrisMin = kIrisMin + ((kBaseIrisRange * kActiveMood.irisMinPercent) / 100);
uint16_t kMoodIrisMaxCandidate = kIrisMin + ((kBaseIrisRange * kActiveMood.irisMaxPercent) / 100);
uint16_t kMoodIrisMax = (kMoodIrisMaxCandidate > kMoodIrisMin) ? kMoodIrisMaxCandidate : kIrisMax;

enum BlinkState : uint8_t
{
  NOBLINK = 0,
  ENBLINK = 1,
  DEBLINK = 2,
};

struct EyeBlink
{
  BlinkState state;
  uint32_t duration;
  uint32_t startTime;
};

struct EyeState
{
  Arduino_GC9D01 *panel;
  int8_t winkPin;
  EyeBlink blink;
  int16_t xposition;
};

Arduino_ESP32SPI leftBus(Pins::kDc, Pins::kLeftCs, Pins::kSclk, Pins::kDin, GFX_NOT_DEFINED);
Arduino_ESP32SPI rightBus(Pins::kDc, Pins::kRightCs, Pins::kSclk, Pins::kDin, GFX_NOT_DEFINED);

Arduino_GC9D01 leftPanel(&leftBus, Pins::kLeftReset, DisplayConfig::kLeftRotation);
Arduino_GC9D01 rightPanel(&rightBus, Pins::kRightReset, DisplayConfig::kRightRotation);

EyeState eyes[NUM_EYES] = {
    {&leftPanel, LEFT_WINK_PIN, {NOBLINK, 0, 0}, kRenderXOffset},
    {&rightPanel, RIGHT_WINK_PIN, {NOBLINK, 0, 0}, kRenderXOffset},
};

uint16_t pixelBuffer[PIXEL_BUFFER_SIZE];
uint16_t oldIris = (kMoodIrisMin + kMoodIrisMax) / 2;
uint16_t newIris = oldIris;
uint32_t sketchStartMs = 0;
uint32_t timeOfLastBlink = 0;
uint32_t timeToNextBlink = 0;

const uint8_t ease[] PROGMEM = {
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 3,
    3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 9, 9, 10, 10,
    11, 12, 12, 13, 14, 15, 15, 16, 17, 18, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 27, 28, 29, 30, 31, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 44, 45, 46, 47, 48, 50, 51, 52, 53, 54, 56, 57, 58,
    60, 61, 62, 63, 65, 66, 67, 69, 70, 72, 73, 74, 76, 77, 78, 80,
    81, 83, 84, 85, 87, 88, 90, 91, 93, 94, 96, 97, 98, 100, 101, 103,
    104, 106, 107, 109, 110, 112, 113, 115, 116, 118, 119, 121, 122, 124, 125, 127,
    128, 130, 131, 133, 134, 136, 137, 139, 140, 142, 143, 145, 146, 148, 149, 151,
    152, 154, 155, 157, 158, 159, 161, 162, 164, 165, 167, 168, 170, 171, 172, 174,
    175, 177, 178, 179, 181, 182, 183, 185, 186, 188, 189, 190, 192, 193, 194, 195,
    197, 198, 199, 201, 202, 203, 204, 205, 207, 208, 209, 210, 211, 213, 214, 215,
    216, 217, 218, 219, 220, 221, 222, 224, 225, 226, 227, 228, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 237, 238, 239, 240, 240, 241, 242, 243, 243, 244,
    245, 245, 246, 246, 247, 248, 248, 249, 249, 250, 250, 251, 251, 251, 252, 252,
    252, 253, 253, 253, 254, 254, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255};

// Optional one-time hook for user custom setup code.
void user_setup(void) {}

// Optional per-frame hook for user custom behavior.
void user_loop(void) {}

// XXXXXXXXXXXXXXXXXXXXXXXXXX My Subroutines XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void setup_wifi() {
  WiFi.disconnect(false);                     // Stack reset ohne SSID zu löschen
  delay(100);
  WiFi.hostname(iamclient);
  WiFi.config(staticIP, gateway, subnet);
  
  Serial.println("");
  Serial.print("Try connect to: ");
  Serial.println(wifi_ssid);
  Serial.print("With IP address: ");
  Serial.println(WiFi.localIP());

  WiFi.begin(wifi_ssid, wifi_password);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  while ((WiFi.status()!= WL_CONNECTED) && (watchdogW <= WiFi_timeout)) {
    delay(250);
    Serial.print(".");
    Serial.print(watchdogW);
    watchdogW++;
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("No connection -> Restart");
      WiFi.disconnect(true);
      delay(500);
      ESP.restart();
    }
  else {
    Serial.println(""); Serial.println("");
    Serial.print("Successfull connected Wifi with RSSI: "); 
    Serial.println(WiFi.RSSI());
   }
}

#if defined(OTA)                                    // Augment by OTA posibility
  #include <ArduinoOTA.h>
  bool otaInProgress = false;

void setupOTA(){                                    
    ArduinoOTA.end();                               // Bestehenden OTA-Server beenden falls aktiv
    delay(100);
    ArduinoOTA.setPort(3232);                       // Port defaults to 3232
    // Hostname defaults to esp32-[ChipID] Set this BEFORE calling begin()
    ArduinoOTA.setHostname(iamclient);              // Choose a unique name
    // No authentication by default Password can be set with MD5 hash as well MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    ArduinoOTA.setPassword("admin");

    // OTA event handlers
    ArduinoOTA.onStart([]() {
      otaInProgress = true;
      mqttclient.disconnect();                      // ← MQTT Socket freigeben vor OTA
      espClient.stop();                             // ← WiFiClient Socket explizit schliessen
      delay(100);
      Serial.println("OTA Started");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("OTA Progress: %u%%\n", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf(" IP:%s Port:%u\n", ArduinoOTA.getHostname().c_str(), 3232);
        Serial.printf("OTA Error[%u]: ", error);
        if      (error == OTA_AUTH_ERROR)    Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)   Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)     Serial.println("End Failed");
        
    });

    ArduinoOTA.onEnd([]() {
        otaInProgress = false;
        Serial.println("OTA End");
    });
    ArduinoOTA.begin();
    Serial.println("OTA Initialized");
  }

void checkOTA(){
    ArduinoOTA.handle();
  }
#endif

void reconnect() {
  delay(100);
  // Loop until we're reconnected to MQTT server
  while (!mqttclient.connected() && (watchdogM <= mqtt_timeout)) {
    mqttclient.clearWriteError();                   // Cleaning MQTT write buffer
    mqttclient.flush();                             // Cleaning MQTT data buffer
    mqttstatus = mqttclient.state();                // Decoding of MQTT status 
    Serial.println("");
    Serial.print("Attempting MQTT connection try Nr. ");
    Serial.println(watchdogM);
  
    const char *reason;
    switch (mqttstatus) {
      case -4 : {reason = "MQTT server didn't respond within the keepalive time"; break;}
      case -3 : {reason = "MQTT network connection was broken"; break;}
      case -2 : {reason = "MQTT network connection failed"; break;}
      case -1 : {reason = "MQTT client is disconnected cleanly"; break;}
      case  0 : {reason = "MQTT client is connected"; break;}
      case  1 : {reason = "MQTT server doesn't support the requested version of MQTT"; break;}
      case  2 : {reason = "MQTT server rejected the client identifier"; break;}
      case  3 : {reason = "MQTT server was unable to accept the connection"; break;}
      case  4 : {reason = "MQTT username/password were rejected"; break;}
      case  5 : {reason = "MQTT client was not authorized to connect"; break;}
      default: {   }                                // Wrong               
   }
    Serial.println(reason);
    if (mqttclient.connect(iamclient, mqtt_user, mqtt_password)) {
      Serial.print("MQTT connected as: ");
      Serial.println(iamclient);
      Serial.println("");
      watchdogM = 1;
      // Send status after start
      #if defined(TEST)                               // Send status after start
        mqttclient.publish(out_status, "1" ,false);
      #else
        mqttclient.publish(out_status, "MQTT Connected" ,false);
        delay(500);
      #endif
    }
    Serial.print("MQTT with RSSI: ");   Serial.println(WiFi.RSSI());
    Serial.println("Retry MQTT in 4 seconds ... ");
    // Wait 4 seconds before retrying with OTA-frindlye wait loop
    for (int i = 0; i < 40; i++) {
      delay(100);
      #if defined(OTA)
        checkOTA();
      #endif
    }
    watchdogM++;
    Serial.println("_");
    }
    if (watchdogM >= mqtt_timeout) {
    Serial.println("NO MQTT available -> RESTART");
    RestartDevice();                                  // REBOOT of the system !!!!!
  }
}

void callback(char* topic, uint8_t* payload, unsigned int length) {
  receivedlenght = length;
  Serial.println("");
  Serial.print("Message for [");
  Serial.print(topic);
  Serial.print("] arrived = L:(");
  Serial.print(length);
  Serial.print(")->");
  for (unsigned int i = 0; i < length; i++) {
    receivedChar[i] = payload[i];    
    Serial.print(receivedChar[i]);                  //     Received from mqtt text
  }
  Serial.println();
  #if defined(TEST)
      mqttclient.publish(out_status, "3" ,false);
    #else
      Sendme = "Command  ";
      Sendme = Sendme + (receivedChar[0]);
      Sendme = Sendme + " received";
      mqttclient.publish(out_status, (String(Sendme).c_str()) ,false);
      statusreset = true;
      delay(1000);
  #endif
 
  switch (receivedChar[0]) {          // Detecition what command is detected 
    case '?' : {                       // request parameters
     Sendme = "RSSI: ";
      Sendme = Sendme + (WiFi.RSSI());
      mqttclient.publish(out_param,   (String(Sendme).c_str()), false);       // Send the wifi signal strenght
      mqttclient.publish(out_sensors, (String(MySensors).c_str()), false);    // Send list of the usable sensors
      break;
    }
    case 'I' : {
      int V; 
      V = x2i(receivedChar,1,2); 
      Device_Set = V; 
      DeviceControll();
      break;
    }      // Set intensity of display

    // Received from mqtt new update rate 
    case 'U' : {     
      int Va; 
      Va = x2i(receivedChar,1,2);                                             
      if (Va <= 5){ Va = 5; };
      Sendme = "Update : ";
      Sendme = Sendme + (Va);
      mqttclient.publish(out_status, (String(Sendme).c_str()), false);        // Send the wifi signal strenght
      readinterval = Va *  500;
      sendinterval = Va * 1000;
      break;
    }
    // RESTART command 
    case 'X' : {                       // restart device
      mqttclient.publish(out_status, "Restart requested", false);
      RestartDevice();
      break;
      }
    default: {                        // Wrong command
      #if defined(TEST)
        mqttclient.publish(out_status, "-1" ,false);
      #else
        mqttclient.publish(out_status, "No valid command" ,false);
        statusreset = true;
        delay(1000);
      #endif
      break;
    }
  }
}

void RestartDevice(){
  mqttclient.publish(out_status, "Restart Device", false);
  Serial.println("Restart Device");
  delay(1000);
  mqttclient.disconnect();
  Serial.println("MQTT disconnect");
  delay(1000);
  WiFi.disconnect(true);                                                  // true = auch gespeicherte SSID löschen
  Serial.println("Wifi disconnect");
  delay(1000);
  ESP.restart();                                                          // REMOTE RESTART
}

// Helper function for substring to hex conversion 
int x2i(const char *s, int from_a, int to_b) {
  char temp[33];                                    // Enough for 32-bit hex + null terminator
  int len = to_b - from_a + 1;
  strncpy(temp, s + from_a, len);
  temp[len] = '\0';
  return (int)strtol(temp, nullptr, 16);
}

void DeviceControll(){
  Serial.printf("DeviceControll Set: %x \t", Device_Set);
  Crumb_1 = (Device_Set & 0b11);           // Extracting 2 bits starting from the rightmost side (Bit0 and Bit1)
  Crumb_2 = ((Device_Set >> 2) & 0b11);    // Shifting 2 bits to the right and then extracting the next 2 bits
  Crumb_3 = ((Device_Set >> 4) & 0b11);    // Shifting 4 bits to the right and then extracting the next 2 bits
  Crumb_4 = ((Device_Set >> 6) & 0b11);    // Shifting 6 bits to the right and then extracting the next 2 bits
  Serial.printf("%x %x %x %x\n", Crumb_1, Crumb_2, Crumb_3, Crumb_4);
  switch (Crumb_1) {   // Mode control
    case 0:  {
      kActiveMood = kMoodSleepy ;    // = {"sleepy", 45, 75, 70000, 140000, 1000000, 2500000, 5000000, 120000, 240000, 16000000, 35, 15};
      Serial.printf("Mode-0: Sleepy\t\t");  break; }
    case 1 : {
      kActiveMood = kMoodCurious ;    //= {"curious", 30, 80, BLINK_MIN_US, BLINK_MAX_US, 1500000, 3500000, MOVE_PAUSE_MAX_US, MOVE_TIME_MIN_US, MOVE_TIME_MAX_US, IRIS_TRANSITION_US, -5, -5};
      Serial.printf("Mode-1: Curious\t\t"); break;}
    case 2 : {
      kActiveMood = kMoodSurprised ;    //= {"surprised", 20, 95, 25000, 50000, 3500000, 7000000, 1600000, 50000, 100000, 5000000, -20, -20};
      Serial.printf("Mode-2: Surprised\t"); break;}
    case 3 : {
      kActiveMood = kMoodScared ;    //= {"scared", 10, 45, 25000, 45000, 600000, 1800000, 800000, 40000, 90000, 3000000, -35, -35};
      Serial.printf("Mode-3: Scared\t\t"); break;}
    default:{
      Serial.printf("Mode-def: %x\t\t\t", Crumb_3);
      break;
    }
  }
    switch (Crumb_2) {   // Backlight controll
    case 0:  {
      digitalWrite(Pins::kLeftBacklight,  LOW); digitalWrite(Pins::kRightBacklight, LOW);
      Serial.printf("Bkl-0: %x %x\t", digitalRead(Pins::kLeftBacklight), digitalRead(Pins::kRightBacklight)); break; }
    case 1 : {
      digitalWrite(Pins::kLeftBacklight,  HIGH); digitalWrite(Pins::kRightBacklight, LOW);
      Serial.printf("Bkl-1: %x %x\t", digitalRead(Pins::kLeftBacklight), digitalRead(Pins::kRightBacklight)); break;}
    case 2 : {
      digitalWrite(Pins::kLeftBacklight,  LOW); digitalWrite(Pins::kRightBacklight, HIGH);
      Serial.printf("Bkl-2: %x %x\t", digitalRead(Pins::kLeftBacklight), digitalRead(Pins::kRightBacklight)); break;}
    case 3 : {
      digitalWrite(Pins::kLeftBacklight,  HIGH); digitalWrite(Pins::kRightBacklight, HIGH);
      Serial.printf("Bkl-3: %x %x\t", digitalRead(Pins::kLeftBacklight), digitalRead(Pins::kRightBacklight)); break;}
    default:{
      Serial.printf("Crumb_2-def : %x\t\t", Crumb_2); break;
    }
  }
    switch (Crumb_3) {   // Close Lid controll = Eye off
    case 0:  {
      wink[0] = true; wink[1] = true;
      Serial.print("Eyes-0: ON ON \n"); break; }
    case 1 : {
      wink[0] = true; wink[1] = false;
      Serial.print("Eyes-1: ON OFF \n"); break;}
    case 2 : {
      wink[0] = false; wink[1] = true;
      Serial.print("Eyes-2: OFF ON \n"); break;}
    case 3 : {
      wink[0] = false; wink[1] = false;
      Serial.print("Eyes-3: OFF OFF \n"); break;}
    default:{
      Serial.printf("Crumb_3-def : %x\n", Crumb_3);  break;
    }
  }
  Serial.print("\n");
}

// XXXXXXXXXXXXXXXXXXXXXXXEND my subroutines XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX 

void frame(uint16_t iScale);
void split(int16_t startValue, int16_t endValue, uint32_t startTime, int32_t duration, int16_t range);

// Returns the name of the currently selected mood preset.
const char *selectedMoodName()
{
  return kActiveMood.name;
}

// Clamps a lid threshold to the valid range expected by the eye masks.
uint8_t clampThreshold(int16_t value)
{
  if (value < 0)
  {
    return 0;
  }
  if (value > 254)
  {
    return 254;
  }
  return static_cast<uint8_t>(value);
}

// Returns the name of the currently compiled eye style.
const char *selectedEyeStyleName()
{
#if defined(EYE_STYLE_DEFAULT_NORMAL)
  return "default_normal";
#elif defined(EYE_STYLE_DEFAULT_FULL_SCREEN)
  return "default_full_screen";
#elif defined(EYE_STYLE_CAT_NORMAL)
  return "cat_normal";
#elif defined(EYE_STYLE_CAT_FULL_SCREEN)
  return "cat_full_screen";
#elif defined(EYE_STYLE_DOE_NORMAL)
  return "doe_normal";
#elif defined(EYE_STYLE_DOE_FULL_SCREEN)
  return "doe_full_screen";
#elif defined(EYE_STYLE_DRAGON_NORMAL)
  return "dragon_normal";
#elif defined(EYE_STYLE_DRAGON_FULL_SCREEN)
  return "dragon_full_screen";
#elif defined(EYE_STYLE_GOAT_NORMAL)
  return "goat_normal";
#elif defined(EYE_STYLE_GOAT_FULL_SCREEN)
  return "goat_full_screen";
#elif defined(EYE_STYLE_NAUGA_NORMAL)
  return "nauga_normal";
#elif defined(EYE_STYLE_NAUGA_FULL_SCREEN)
  return "nauga_full_screen";
#elif defined(EYE_STYLE_NEWT_NORMAL)
  return "newt_normal";
#elif defined(EYE_STYLE_NEWT_FULL_SCREEN)
  return "newt_full_screen";
#elif defined(EYE_STYLE_NO_SCLERA_NORMAL)
  return "no_sclera_normal";
#elif defined(EYE_STYLE_NO_SCLERA_FULL_SCREEN)
  return "no_sclera_full_screen";
#elif defined(EYE_STYLE_OWL_NORMAL)
  return "owl_normal";
#elif defined(EYE_STYLE_OWL_FULL_SCREEN)
  return "owl_full_screen";
#elif defined(EYE_STYLE_TERMINATOR_NORMAL)
  return "terminator_normal";
#elif defined(EYE_STYLE_TERMINATOR_FULL_SCREEN)
  return "terminator_full_screen";
#else
  return "default_full_screen";
#endif
}

// Prints the wiring map used by this sketch.
void printPinMap()
{
  Serial.print(F("Pin map: DIN=GPIO"));
  Serial.print(Pins::kDin);
  Serial.print(F(", CLK=GPIO"));
  Serial.print(Pins::kSclk);
  Serial.print(F(", DC=GPIO"));
  Serial.print(Pins::kDc);
  Serial.print(F(", CS1=GPIO"));
  Serial.print(Pins::kLeftCs);
  Serial.print(F(", CS2=GPIO"));
  Serial.print(Pins::kRightCs);
  Serial.print(F(", RST1=GPIO"));
  Serial.print(Pins::kLeftReset);
  Serial.print(F(", RST2=GPIO"));
  Serial.print(Pins::kRightReset);
  Serial.print(F(", BL1=GPIO"));
  Serial.print(Pins::kLeftBacklight);
  Serial.print(F(", BL2=GPIO"));
  Serial.println(Pins::kRightBacklight);
}

// Prints targeted troubleshooting steps for one display panel.
void printPanelTroubleshooting(const char *panelName, int8_t csPin, int8_t resetPin, int8_t backlightPin)
{
  Serial.print(F("Troubleshooting "));
  Serial.print(panelName);
  Serial.println(F(":"));
  Serial.print(F("  Check power first: VCC to 3V3 or 5V, and GND to GND."));
  Serial.println();
  Serial.print(F("  Verify shared SPI lines: DIN GPIO"));
  Serial.print(Pins::kDin);
  Serial.print(F(", CLK GPIO"));
  Serial.print(Pins::kSclk);
  Serial.print(F(", DC GPIO"));
  Serial.print(Pins::kDc);
  Serial.println(F("."));
  Serial.print(F("  Verify panel-specific lines: CS GPIO"));
  Serial.print(csPin);
  Serial.print(F(", RST GPIO"));
  Serial.print(resetPin);
  Serial.print(F(", BL GPIO"));
  Serial.print(backlightPin);
  Serial.println(F("."));
  Serial.println(F("  If only one eye fails, double-check that CS and RST are not swapped between left and right."));
}

// Prints a startup summary with style, timing, and wiring details.
void printStartupSummary()
{
  Serial.println();
  Serial.println(F("==== DualEye Startup ===="));
  Serial.print(F("Dual GC9D01 eye display for "));
  Serial.println(BoardTarget::kName);
  Serial.print(F("Serial monitor baud: "));
  Serial.println(SERIAL_BAUD);
  Serial.print(F("Eye style: "));
  Serial.println(selectedEyeStyleName());
  Serial.print(F("Mood: "));
  Serial.println(selectedMoodName());
  Serial.print(F("Mood iris range: "));
  Serial.print(kMoodIrisMin);
  Serial.print(F(" to "));
  Serial.println(kMoodIrisMax);
  Serial.print(F("SPI clock: "));
  Serial.print(DisplayConfig::kSpiFrequency / 1000000UL);
  Serial.println(F(" MHz"));
  printPinMap();
  if (BLINK_PIN < 0 && LEFT_WINK_PIN < 0 && RIGHT_WINK_PIN < 0)
  {
    Serial.println(F("Blink inputs: disabled, automatic blinking is active."));
  }
}

// Prints the eye style and mood that should currently be visible on the screens.
void printCurrentEyeStyle()
{
  Serial.print(F("Current eye on screen: "));
  Serial.println(selectedEyeStyleName());
  Serial.print(F("Current mood preset: "));
  Serial.println(selectedMoodName());
}

// Enables both panel backlights after initialization.
void enableBacklights()
{
  pinMode(Pins::kLeftBacklight, OUTPUT);
  pinMode(Pins::kRightBacklight, OUTPUT);
  digitalWrite(Pins::kLeftBacklight, LOW);
  digitalWrite(Pins::kRightBacklight, LOW);
  Serial.println(F("Backlights half enabled on BL1 and BL2."));
}

// Initializes both displays and clears them to the background color.
bool beginPanels()
{
  delay(200);  // Stabile Versorgungsspannung abwarten
  Serial.println(F("Initializing display panels..."));
  Serial.print(F("  Left eye panel... "));
  if (!leftPanel.begin(DisplayConfig::kSpiFrequency))
  {
    Serial.println(F("FAILED"));
    printPanelTroubleshooting("left eye", Pins::kLeftCs, Pins::kLeftReset, Pins::kLeftBacklight);
    return false;
  }
  Serial.println(F("OK"));

  Serial.print(F("  Right eye panel... "));
  if (!rightPanel.begin(DisplayConfig::kSpiFrequency))
  {
    Serial.println(F("FAILED"));
    printPanelTroubleshooting("right eye", Pins::kRightCs, Pins::kRightReset, Pins::kRightBacklight);
    return false;
  }
  Serial.println(F("OK"));

  for (uint8_t i = 0; i < NUM_EYES; ++i)
  {
    eyes[i].panel->fillScreen(DisplayConfig::kPanelBackground);
  }

  Serial.println(F("Display memory cleared."));

  return true;
}

// Resets blink state and configures any optional input pins.
void initEyes()
{
  for (uint8_t e = 0; e < NUM_EYES; ++e)
  {
    eyes[e].blink.state = NOBLINK;
    eyes[e].blink.duration = 0;
    eyes[e].blink.startTime = 0;

    if (eyes[e].winkPin >= 0)
    {
      pinMode(eyes[e].winkPin, INPUT_PULLUP);
    }
  }

  if (BLINK_PIN >= 0)
  {
    pinMode(BLINK_PIN, INPUT_PULLUP);
  }
}

// Renders one eye image to the selected display panel.
void drawEye(
    uint8_t eyeIndex,
    uint32_t iScale,
    uint32_t scleraX,
    uint32_t scleraY,
    uint32_t upperThreshold,
    uint32_t lowerThreshold)
{
  uint32_t screenX;
  uint32_t screenY;
  uint32_t scleraXStart = scleraX;
  int32_t irisXStart = static_cast<int32_t>(scleraXStart) - ((SCLERA_WIDTH - IRIS_WIDTH) / 2);
  int32_t irisYStart = static_cast<int32_t>(scleraY) - ((SCLERA_HEIGHT - IRIS_HEIGHT) / 2);
  size_t pixels = 0;

  Arduino_GC9D01 *panel = eyes[eyeIndex].panel;
  panel->startWrite();
  panel->writeAddrWindow(eyes[eyeIndex].xposition, 0, kRenderWidth, kRenderHeight);

  for (screenY = 0; screenY < kRenderHeight; ++screenY)
  {
    uint32_t sourceScreenY = (screenY * SCREEN_HEIGHT) / kRenderHeight;
    uint32_t sourceScleraY = scleraY + sourceScreenY;
    int32_t sourceIrisY = irisYStart + static_cast<int32_t>(sourceScreenY);

    for (screenX = 0; screenX < kRenderWidth; ++screenX)
    {
      uint16_t pixel;
      uint32_t sourceScreenX = (screenX * SCREEN_WIDTH) / kRenderWidth;
      uint32_t sourceScleraX = scleraXStart + sourceScreenX;
      int32_t sourceIrisX = irisXStart + static_cast<int32_t>(sourceScreenX);
      uint16_t lidX = (eyeIndex == 0) ? (SCREEN_WIDTH - 1 - sourceScreenX) : sourceScreenX;

      if ((pgm_read_byte(lower + sourceScreenY * SCREEN_WIDTH + lidX) <= lowerThreshold) ||
          (pgm_read_byte(upper + sourceScreenY * SCREEN_WIDTH + lidX) <= upperThreshold))
      {
        pixel = 0;
      }
      else if ((sourceIrisY < 0) || (sourceIrisY >= IRIS_HEIGHT) || (sourceIrisX < 0) || (sourceIrisX >= IRIS_WIDTH))
      {
        pixel = pgm_read_word(sclera + sourceScleraY * SCLERA_WIDTH + sourceScleraX);
      }
      else
      {
        uint16_t polarValue = pgm_read_word(polar + sourceIrisY * IRIS_WIDTH + sourceIrisX);
        uint32_t distance = (iScale * (polarValue & 0x7F)) / 128;
        if (distance < IRIS_MAP_HEIGHT)
        {
          uint32_t angle = (IRIS_MAP_WIDTH * (polarValue >> 7)) / 512;
          pixel = pgm_read_word(iris + distance * IRIS_MAP_WIDTH + angle);
        }
        else
        {
          pixel = pgm_read_word(sclera + sourceScleraY * SCLERA_WIDTH + sourceScleraX);
        }
      }

      pixelBuffer[pixels++] = pixel;

      if (pixels >= PIXEL_BUFFER_SIZE)
      {
        yield();
        panel->writePixels(pixelBuffer, pixels);
        pixels = 0;
      }
    }
  }

  if (pixels != 0)
  {
    panel->writePixels(pixelBuffer, pixels);
  }

  panel->endWrite();
}

// Advances the animation state and draws the next eye frame.
void frame(uint16_t iScale)
{
  static uint32_t frames = 0;
  static uint8_t eyeIndex = NUM_EYES - 1;

  static bool eyeInMotion = false;
  static int16_t eyeOldX = 512;
  static int16_t eyeOldY = 512;
  static int16_t eyeNewX = 512;
  static int16_t eyeNewY = 512;
  static uint32_t eyeMoveStartTime = 0;
  static int32_t eyeMoveDuration = 0;

  int16_t eyeX;
  int16_t eyeY;
  const uint32_t now = micros();

  if (!(++frames & 255))
  {
    float elapsed = (millis() - sketchStartMs) / 1000.0f;
    if (elapsed > 0.0f)
    {
      Serial.print(F("Status: eye= "));
      Serial.print(selectedEyeStyleName());
      Serial.print(F(", mood= "));
      Serial.print(selectedMoodName());
      Serial.print(F(", approx "));
      Serial.print(static_cast<uint16_t>(frames / elapsed));
      Serial.println(F(" FPS"));
      MySensors = String(selectedEyeStyleName()) + " " + selectedMoodName() + " " + String(static_cast<uint16_t>(frames / elapsed)) + 
                  " FPS Decontrol = 0x" + String(static_cast<uint16_t>(Device_Set), HEX);
    }
  }
  if (++eyeIndex >= NUM_EYES)
  {
    eyeIndex = 0;
  }

  int32_t dt = static_cast<int32_t>(now - eyeMoveStartTime);
  if (eyeInMotion)
  {
    if (dt >= eyeMoveDuration)
    {
      eyeInMotion = false;
      eyeMoveDuration = random(kActiveMood.movePauseMaxUs);
      eyeMoveStartTime = now;
      eyeX = eyeOldX = eyeNewX;
      eyeY = eyeOldY = eyeNewY;
    }
    else
    {
      int16_t easeIndex = (255 * dt) / eyeMoveDuration;
      if (easeIndex < 0)
      {
        easeIndex = 0;
      }
      else if (easeIndex > 255)
      {
        easeIndex = 255;
      }

      int16_t e = ease[easeIndex] + 1;
      eyeX = eyeOldX + (((eyeNewX - eyeOldX) * e) / 256);
      eyeY = eyeOldY + (((eyeNewY - eyeOldY) * e) / 256);
    }
  }
  else
  {
    eyeX = eyeOldX;
    eyeY = eyeOldY;

    if (dt > eyeMoveDuration)
    {
      int16_t dx;
      int16_t dy;
      uint32_t distance;
      do
      {
        eyeNewX = random(1024);
        eyeNewY = random(1024);
        dx = (eyeNewX * 2) - 1023;
        dy = (eyeNewY * 2) - 1023;
        distance = static_cast<uint32_t>(dx * dx + dy * dy);
      } while (distance > (1023UL * 1023UL));

      eyeMoveDuration = random(kActiveMood.moveTimeMinUs, kActiveMood.moveTimeMaxUs);
      eyeMoveStartTime = now;
      eyeInMotion = true;
    }
  }

  if ((now - timeOfLastBlink) >= timeToNextBlink)
  {
    timeOfLastBlink = now;
    uint32_t blinkDuration = random(kActiveMood.blinkMinUs, kActiveMood.blinkMaxUs);

    for (uint8_t e = 0; e < NUM_EYES; ++e)
    {
      if (eyes[e].blink.state == NOBLINK)
      {
        eyes[e].blink.state = ENBLINK;
        eyes[e].blink.startTime = now;
        eyes[e].blink.duration = blinkDuration;
      }
    }

    timeToNextBlink = random(kActiveMood.blinkGapMinUs, kActiveMood.blinkGapMaxUs);
  }

  if (eyes[eyeIndex].blink.state != NOBLINK)
  {
    if ((now - eyes[eyeIndex].blink.startTime) >= eyes[eyeIndex].blink.duration)
    {
      bool holdClosed = false;
      if (BLINK_PIN >= 0 && digitalRead(BLINK_PIN) == LOW)
      {
        holdClosed = true;
      }
      if (eyes[eyeIndex].winkPin >= 0 && digitalRead(eyes[eyeIndex].winkPin) == LOW || wink[eyeIndex] == false)
      {
        holdClosed = true;
      }

      if (!holdClosed)
      {
        if (eyes[eyeIndex].blink.state == ENBLINK)
        {
          eyes[eyeIndex].blink.state = DEBLINK;
          eyes[eyeIndex].blink.duration *= 2;
          eyes[eyeIndex].blink.startTime = now;
        }
        else
        {
          eyes[eyeIndex].blink.state = NOBLINK;
        }
      }
    }
  }
  else
  {
    if (BLINK_PIN >= 0 && digitalRead(BLINK_PIN) == LOW)
    {
      uint32_t blinkDuration = random(kActiveMood.blinkMinUs, kActiveMood.blinkMaxUs);
      for (uint8_t e = 0; e < NUM_EYES; ++e)
      {
        if (eyes[e].blink.state == NOBLINK)
        {
          eyes[e].blink.state = ENBLINK;
          eyes[e].blink.startTime = now;
          eyes[e].blink.duration = blinkDuration;
        }
      }
    }
    else if (eyes[eyeIndex].winkPin >= 0 && digitalRead(eyes[eyeIndex].winkPin) == LOW || wink[eyeIndex] == false)
    {
      eyes[eyeIndex].blink.state = ENBLINK;
      eyes[eyeIndex].blink.startTime = now;
      eyes[eyeIndex].blink.duration = random(kActiveMood.blinkMinUs, kActiveMood.blinkMaxUs);
    }
  }

  eyeX = map(eyeX, 0, 1023, 0, SCLERA_WIDTH - SCREEN_WIDTH);
  eyeY = map(eyeY, 0, 1023, 0, SCLERA_HEIGHT - SCREEN_HEIGHT);

  if (NUM_EYES > 1)
  {
    if (eyeIndex == 1)
    {
      eyeX += 4;
    }
    else
    {
      eyeX -= 4;
    }
  }

  if (eyeX < 0)
  {
    eyeX = 0;
  }
  else if (eyeX > (SCLERA_WIDTH - SCREEN_WIDTH))
  {
    eyeX = (SCLERA_WIDTH - SCREEN_WIDTH);
  }

  static uint8_t upperThreshold = 128;
  uint8_t lowerThreshold;
  uint8_t currentUpperThreshold;

  int16_t sampleX = (SCLERA_WIDTH / 2) - (eyeX / 2);
  int16_t sampleY = (SCLERA_HEIGHT / 2) - (eyeY + (IRIS_HEIGHT / 4));
  uint8_t eyelidSample;
  if (sampleY < 0)
  {
    eyelidSample = 0;
  }
  else
  {
    eyelidSample = (pgm_read_byte(upper + sampleY * SCREEN_WIDTH + sampleX) +
                    pgm_read_byte(upper + sampleY * SCREEN_WIDTH + (SCREEN_WIDTH - 1 - sampleX))) /
                   2;
  }
  upperThreshold = (upperThreshold * 3 + eyelidSample) / 4;
  currentUpperThreshold = clampThreshold(static_cast<int16_t>(upperThreshold) + kActiveMood.upperLidOffset);
  lowerThreshold = clampThreshold(static_cast<int16_t>(254 - upperThreshold) + kActiveMood.lowerLidOffset);

  if (eyes[eyeIndex].blink.state != NOBLINK)
  {
    uint32_t blinkProgress = now - eyes[eyeIndex].blink.startTime;
    if (blinkProgress >= eyes[eyeIndex].blink.duration)
    {
      blinkProgress = 255;
    }
    else
    {
      blinkProgress = (255 * blinkProgress) / eyes[eyeIndex].blink.duration;
    }

    blinkProgress = (eyes[eyeIndex].blink.state == DEBLINK) ? (1 + blinkProgress) : (256 - blinkProgress);
    currentUpperThreshold = (currentUpperThreshold * blinkProgress + 254 * (257 - blinkProgress)) / 256;
    lowerThreshold = (lowerThreshold * blinkProgress + 254 * (257 - blinkProgress)) / 256;
  }

  drawEye(eyeIndex, iScale, eyeX, eyeY, currentUpperThreshold, lowerThreshold);

  if (eyeIndex == (NUM_EYES - 1))
  {
    user_loop();
  }
}

// Smoothly interpolates iris size changes while frames continue rendering.
void split(int16_t startValue, int16_t endValue, uint32_t startTime, int32_t duration, int16_t range)
{
  if (range >= 8)
  {
    range /= 2;
    duration /= 2;
    int16_t midValue = (startValue + endValue - range) / 2 + random(range);
    uint32_t midTime = startTime + duration;
    split(startValue, midValue, startTime, duration, range);
    split(midValue, endValue, midTime, duration, range);
  }
  else
  {
    int32_t dt;
    while ((dt = static_cast<int32_t>(micros() - startTime)) < duration)
    {
      int16_t value = startValue + (((endValue - startValue) * dt) / duration);
      if (value < kMoodIrisMin)
      {
        value = kMoodIrisMin;
      }
      else if (value > kMoodIrisMax)
      {
        value = kMoodIrisMax;
      }
      frame(value);
    }
  }
}

// Picks the next iris size target and animates toward it.
void updateEye()
{
  newIris = random(kMoodIrisMin, kMoodIrisMax);
  split(oldIris, newIris, micros(), kActiveMood.irisTransitionUs, kMoodIrisMax - kMoodIrisMin);
  oldIris = newIris;
}


//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

// Initializes serial output, displays, and the eye animation state.
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(2000);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(SERIAL_BAUD);

  Serial.println("");
  Serial.println("");
  Serial.print("I am: ");
  Serial.print(iamclient);
  Serial.println("");

  setup_wifi(); // Start the wifi connection
  delay(100);
  #if defined(OTA)                                  
    setupOTA();                                     // Start OTA posibility
  #endif

  mqttclient.setClient(espClient);
  mqttclient.setServer(mqtt_server, mqtt_port);
  mqttclient.setCallback(callback);
  mqttclient.setKeepAlive(61);      // MQTT_KEEPALIVE : keepAlive interval in seconds. Override setKeepAlive()
  mqttclient.setSocketTimeout(63);  // MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds. Override setSocketTimeout()
  reconnect();                      // Start the mqtt connection would read first meassures with 0
  mqttclient.subscribe(in_topic);   // Listen to the mqtt inputs

// XXXXXXXXXXXXX Original Setup XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

  delay(250);
  printStartupSummary();

  Serial.println(F("\n\nSeeding animation..."));
  randomSeed(micros());

  Serial.println(F("Preparing blink and wink inputs..."));
  initEyes();
  if (!beginPanels())
  {
    Serial.println(F("Startup halted because a display did not initialize."));
    Serial.println(F("Re-check the wiring above, then press reset."));
    while (true)
    {
      delay(1000);
    }
  }

  enableBacklights();
  Serial.println(F("Running user setup hook..."));
  user_setup();

  sketchStartMs = millis();
  timeOfLastBlink = micros();
  timeToNextBlink = random(kActiveMood.blinkGapMinUs, kActiveMood.blinkGapMaxUs);

  Serial.println(F("Startup complete."));
  printCurrentEyeStyle();
  Serial.println(F("Expected behavior: both eyes should open, move, and blink."));
  Serial.println(F("If the screens stay black, re-check power, backlight pins, and the pin map in the code."));
  Serial.println(F("If just on screen is working, press the reset button, check backlight pins, and the pin map in the code"));
}

// Runs the continuous eye animation.
void loop()
{
  #if defined(OTA)                                  // Augment  OTA posibility
    checkOTA();
  #endif
  if (otaInProgress) return;                        // ← Während OTA alles andere stoppen  

  unsigned long currentMillis = millis();
  // Every X number of seconds (interval = x milliseconds) it reads a new MQTT message
  if (currentMillis - prevRMQTTMillis >= readinterval) {
    prevRMQTTMillis = currentMillis;

    if (WiFi.status() == WL_CONNECTED) { Serial.print("+");}
    else {
      WiFi.begin(wifi_ssid, wifi_password);
      Serial.println("Try Wifi reconnect "); 
    }
    
    if (!mqttclient.connected()) {
      reconnect();                                  // In case no mqtt it will reconnect
      mqttclient.subscribe(in_topic);
      Serial.print("Go in loop with MQTT topic: " );
      Serial.println(in_topic);
      #if defined(TEST)
        mqttclient.publish(out_status, "2" ,false);
      #else
        mqttclient.publish(out_status, "Reconnected" ,false);
      #endif
      statusreset = true;
    }
    mqttclient.loop();                              // Request if there is a message
    watchdog = !watchdog;                           // Create toggeling watchdog signal
    mqttclient.publish(out_watchdog, String(watchdog).c_str() ,false); 

    if (statusreset){
      statusreset = false;
      delay(1000);
      #if defined(TEST)
        mqttclient.publish(out_status, "0" ,false);
      #else
        mqttclient.publish(out_status, "Normal" ,false);
      #endif
    }
  }
  // Sending results to mqtt
  if (currentMillis - prevSMQTTMillis >= sendinterval) {
    prevSMQTTMillis = currentMillis;
    mqttclient.publish(out_sensors, (String(MySensors).c_str()), false);    // Send list of the usable sensors
 
    #if defined(TEST)
      Serial.printf("Free Heap: %d\n\n", ESP.getFreeHeap());  
    #endif
  }
  // XXXXXXXXXXXXX Original Loop XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  updateEye();
}
