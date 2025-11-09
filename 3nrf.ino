#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_event_loop.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> // Change if using different library
#include <Adafruit_NeoPixel.h>
#include <U8g2_for_Adafruit_GFX.h>
// BT
#include <BluetoothSerial.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BleKeyboard.h>
#include <BLEServer.h>
// WIFI
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
// NRF24
#include "RF24.h"
#include <SPI.h>

// NrF24 Triple Module Pin Setup
SPIClass vspi(VSPI);
// radio(CE, CS)
RF24 radio(27, 15, 16000000);   // Radio 1
RF24 radio2(26, 25, 16000000);  // Radio 2
RF24 radio3(17, 5, 16000000);   // Radio 3

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SSD1306_I2C_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// U8g2 for Adafruit GFX
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

// LED
#define LED_PIN 15
// Buttons
#define UP_BUTTON_PIN 14
#define DOWN_BUTTON_PIN 12
#define SELECT_BUTTON_PIN 13

// STATE MANAGEMENT
enum AppState {
  STATE_MENU,
  STATE_BT_JAM,
  STATE_DRONE_JAM,
  STATE_WIFI_JAM,
  STATE_MULTI_JAM,
};
// Global variable to keep track of the current state
AppState currentState = STATE_MENU;

// VARIABLES
enum MenuItem {
  BT_JAM,
  DRONE_JAM,
  WIFI_JAM,
  MULTI_JAM,
  SETTINGS,
  HELP,
  NUM_MENU_ITEMS
};

bool buttonPressed = false;
bool buttonEnabled = true;
uint32_t lastDrawTime;
uint32_t lastButtonTime;
int firstVisibleMenuItem = 0;
MenuItem selectedMenuItem = BT_JAM;

// Use this instead of delay()
void nonBlockingDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    // Allow ESP32 to handle background processes
    yield();  // Very important!
  }
}

//// ------- NRF24 SETUP ------------

void initRadios() {
  // Ensure proper SPI pin configuration for VSPI
  vspi.begin();
  //pinMode(33, INPUT_PULLUP);  // Soft pull-up
  //pinMode(26, OUTPUT);        // Radio 2 CE
  // Initialize each radio
  if (radio.begin(&vspi)) {
    Serial.println("Radio 1 Started");
    radio.setAutoAck(false);
    radio.stopListening();
    radio.setRetries(0, 0);
    radio.setPALevel(RF24_PA_MAX, true);
    radio.setDataRate(RF24_2MBPS);
    radio.setCRCLength(RF24_CRC_DISABLED);
    radio.printPrettyDetails();
    radio.startConstCarrier(RF24_PA_MAX, 45);
  } else {
    Serial.println("Radio 1 Failed to Start");
  }
  if (radio2.begin(&vspi)) {
    Serial.println("Radio 2 Started");
    radio2.setAutoAck(false);
    radio2.stopListening();
    radio2.setRetries(0, 0);
    radio2.setPALevel(RF24_PA_MAX, true);
    radio2.setDataRate(RF24_2MBPS);
    radio2.setCRCLength(RF24_CRC_DISABLED);
    radio2.printPrettyDetails();
    radio2.startConstCarrier(RF24_PA_MAX, 45);
  } else {
    Serial.println("Radio 2 Failed to Start");
  }

  if (radio3.begin(&vspi)) {
    Serial.println("Radio 3 Started");
    radio3.setAutoAck(false);
    radio3.stopListening();
    radio3.setRetries(0, 0);
    radio3.setPALevel(RF24_PA_MAX, true);
    radio3.setDataRate(RF24_2MBPS);
    radio3.setCRCLength(RF24_CRC_DISABLED);
    radio3.printPrettyDetails();
    radio3.startConstCarrier(RF24_PA_MAX, 45);
  } else {
    Serial.println("Radio 3 Failed to Start");
  }
}

/* 
---
Various options to use the 2.4ghz jammer to give you some ideas:
Channels 1-14 are wifi, 40-80 are bluetooth, 1-125 drone (test on your own)
one() - all radios on same random channel range
singleChannel() - select individual channel range for each radio
multipleChannels() - bounce between specific channels for focused tests
channelRange() - provide a specific range to transmit, i.e. bluetooth 40-80, wifi 1-14, etc
Test around & see what works best!
---
*/
void btJam() {
  ////RANDOM CHANNEL
  radio2.setChannel(random(81));
  radio3.setChannel(random(81));
  radio.setChannel(random(81));
  delayMicroseconds(random(60));  //////REMOVE IF SLOW
  /*  YOU CAN DO -----SWEEP CHANNEL
  for (int i = 0; i < 79; i++) {
    radio.setChannel(i);
*/
}

void droneJam() {
  ////RANDOM CHANNEL
  radio2.setChannel(random(126));
  radio3.setChannel(random(126));
  radio.setChannel(random(126));
  delayMicroseconds(random(60));  //////REMOVE IF SLOW
  /*  YOU CAN DO -----SWEEP CHANNEL
  for (int i = 0; i < 79; i++) {
    radio.setChannel(i);
*/
}

void singleChannel() {
  ////RANDOM CHANNEL
  radio2.setChannel(random(81));
  radio3.setChannel(random(15));
  radio.setChannel(random(15));
  delayMicroseconds(random(60));  //////REMOVE IF SLOW
}
void wifiJam() {
  // Define the set of channels you want to choose from
  int numbers[] = { 1, 6, 14 };
  int sizeOfArray = sizeof(numbers) / sizeof(numbers[0]);  // Calculate the size of the array

  // Generate a random index
  int randomIndex = random(sizeOfArray);  // random(max) generates a number from 0 to max-1

  // Select the random number from the array
  int randomNumber = numbers[randomIndex];

  radio.setChannel(randomNumber);
  radio2.setChannel(randomNumber);
  radio3.setChannel(randomNumber);

  // Output the result to the Serial Monitor
  Serial.print("Randomly selected channel: ");
  Serial.println(randomNumber);
}

void channelRange() {
  int randomNumber = random(40, 81);  // 81 because the upper bound is exclusive

  // Output the result to the Serial Monitor
  Serial.print("Randomly selected number between 40 and 80: ");
  Serial.println(randomNumber);

  // Example usage with radio.setChannel
  radio.setChannel(randomNumber);
  radio2.setChannel(randomNumber);
  radio3.setChannel(randomNumber);
}
// ------- NRF24 SETUP END ------------

// ------- GENERAL CONFIGURATION ------------

void initDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.display();
  delay(2000);
  display.clearDisplay();
}
void drawMenu() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  u8g2_for_adafruit_gfx.setFont(u8g2_font_baby_tf);  // Set back to small font


  // Title bar
  display.fillRect(0, 0, SCREEN_WIDTH, 16, SSD1306_WHITE);  // Top bar (16px)
  display.setTextColor(SSD1306_BLACK);                      // Black text on white bar
  display.setCursor(5, 4);                                  // Adjust as needed
  display.setTextSize(1);
  display.println("Home");  // Replace with dynamic title if needed

  // Edit to add/remove menu items
  const char *menuLabels[NUM_MENU_ITEMS] = {
    "BT Jammer", "Drone Jammer", "Wifi Jammer", "Multi Ch Jam", "Settings", "Help"
  };

  // Menu items below title bar
  display.setTextColor(SSD1306_WHITE);  // White text in main menu area
  for (int i = 0; i < 2; i++) {         // Show 2 menu items at a time
    int menuIndex = (firstVisibleMenuItem + i) % NUM_MENU_ITEMS;
    int16_t x = 5;
    int16_t y = 20 + (i * 20);  // Adjust vertical spacing as needed

    // Highlight the selected item
    if (selectedMenuItem == menuIndex) {
      display.fillRect(0, y - 2, SCREEN_WIDTH, 15, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);  // Black text for highlighted item
    } else {
      display.setTextColor(SSD1306_WHITE);  // White text for non-highlighted items
    }
    display.setCursor(x, y);
    display.setTextSize(1);
    display.println(menuLabels[menuIndex]);
  }

  display.display();
}

void drawBorder() {
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
}
void displayInfo(String title, String info1 = "", String info2 = "", String info3 = "") {
  display.clearDisplay();
  drawBorder();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Title
  display.setCursor(4, 4);
  display.println(title);
  display.drawLine(0, 14, SCREEN_WIDTH, 14, SSD1306_WHITE);

  // Info lines
  display.setCursor(4, 18);
  display.println(info1);
  display.setCursor(4, 28);
  display.println(info2);
  display.setCursor(4, 38);
  display.println(info3);
  display.display();
}
bool isButtonPressed(uint8_t pin) {
  if (digitalRead(pin) == LOW) {
    delay(100);  // Debounce delay
    if (digitalRead(pin) == LOW) {
      digitalWrite(LED_PIN, HIGH);  // Turn on LED
      return true;
    }
  }
  return false;
}
void handleMenuSelection() {
  static bool buttonPressed = false;

  if (!buttonPressed) {
    if (isButtonPressed(UP_BUTTON_PIN)) {
      // Wrap around if at the top
      selectedMenuItem = static_cast<MenuItem>((selectedMenuItem == 0) ? (NUM_MENU_ITEMS - 1) : (selectedMenuItem - 1));

      if (selectedMenuItem == (NUM_MENU_ITEMS - 1)) {
        // If wrapped to the bottom, make it visible
        firstVisibleMenuItem = NUM_MENU_ITEMS - 2;
      } else if (selectedMenuItem < firstVisibleMenuItem) {
        firstVisibleMenuItem = selectedMenuItem;
      }

      Serial.println("UP button pressed");
      drawMenu();
      buttonPressed = true;
    } else if (isButtonPressed(DOWN_BUTTON_PIN)) {
      // Wrap around if at the bottom
      selectedMenuItem = static_cast<MenuItem>((selectedMenuItem + 1) % NUM_MENU_ITEMS);

      if (selectedMenuItem == 0) {
        // If wrapped to the top, make it visible
        firstVisibleMenuItem = 0;
      } else if (selectedMenuItem >= (firstVisibleMenuItem + 2)) {
        firstVisibleMenuItem = selectedMenuItem - 1;
      }

      Serial.println("DOWN button pressed");
      drawMenu();
      buttonPressed = true;
    } else if (isButtonPressed(SELECT_BUTTON_PIN)) {
      Serial.println("SELECT button pressed");
      executeSelectedMenuItem();
      buttonPressed = true;
    } 
  } else {
    // If no button is pressed, reset the buttonPressed flag
    if (!isButtonPressed(UP_BUTTON_PIN) && !isButtonPressed(DOWN_BUTTON_PIN) && !isButtonPressed(SELECT_BUTTON_PIN)) {
      buttonPressed = false;
      digitalWrite(LED_PIN, LOW);  // Turn off LED
    }
  }
}

static const unsigned char PROGMEM image_EviSmile1_bits[] = { 0x30, 0x03, 0x00, 0x60, 0x01, 0x80, 0xe0, 0x01, 0xc0, 0xf3, 0xf3, 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0x7f, 0xff, 0x80, 0x7f, 0xff, 0x80, 0x7f, 0xff, 0x80, 0xef, 0xfd, 0xc0, 0xe7, 0xf9, 0xc0, 0xe3, 0xf1, 0xc0, 0xe1, 0xe1, 0xc0, 0xf1, 0xe3, 0xc0, 0xff, 0xff, 0xc0, 0x7f, 0xff, 0x80, 0x7b, 0xf7, 0x80, 0x3d, 0x2f, 0x00, 0x1e, 0x1e, 0x00, 0x0f, 0xfc, 0x00, 0x03, 0xf0, 0x00 };
static const unsigned char PROGMEM image_Ble_connected_bits[] = { 0x07, 0xc0, 0x1f, 0xf0, 0x3e, 0xf8, 0x7e, 0x7c, 0x76, 0xbc, 0xfa, 0xde, 0xfc, 0xbe, 0xfe, 0x7e, 0xfc, 0xbe, 0xfa, 0xde, 0x76, 0xbc, 0x7e, 0x7c, 0x3e, 0xf8, 0x1f, 0xf0, 0x07, 0xc0 };
static const unsigned char PROGMEM image_MHz_bits[] = { 0xc3, 0x61, 0x80, 0x00, 0xe7, 0x61, 0x80, 0x00, 0xff, 0x61, 0x80, 0x00, 0xff, 0x61, 0xbf, 0x80, 0xdb, 0x7f, 0xbf, 0x80, 0xdb, 0x7f, 0x83, 0x00, 0xdb, 0x61, 0x86, 0x00, 0xc3, 0x61, 0x8c, 0x00, 0xc3, 0x61, 0x98, 0x00, 0xc3, 0x61, 0xbf, 0x80, 0xc3, 0x61, 0xbf, 0x80 };
static const unsigned char PROGMEM image_Error_bits[] = { 0x03, 0xf0, 0x00, 0x0f, 0xfc, 0x00, 0x1f, 0xfe, 0x00, 0x3f, 0xff, 0x00, 0x73, 0xf3, 0x80, 0x71, 0xe3, 0x80, 0xf8, 0xc7, 0xc0, 0xfc, 0x0f, 0xc0, 0xfe, 0x1f, 0xc0, 0xfe, 0x1f, 0xc0, 0xfc, 0x0f, 0xc0, 0xf8, 0xc7, 0xc0, 0x71, 0xe3, 0x80, 0x73, 0xf3, 0x80, 0x3f, 0xff, 0x00, 0x1f, 0xfe, 0x00, 0x0f, 0xfc, 0x00, 0x03, 0xf0, 0x00 };
static const unsigned char PROGMEM image_Bluetooth_Idle_bits[] = { 0x20, 0xb0, 0x68, 0x30, 0x30, 0x68, 0xb0, 0x20 };
static const unsigned char PROGMEM image_off_text_bits[] = { 0x67, 0x70, 0x94, 0x40, 0x96, 0x60, 0x94, 0x40, 0x64, 0x40 };
static const unsigned char PROGMEM image_wifi_not_connected_bits[] = { 0x21, 0xf0, 0x00, 0x16, 0x0c, 0x00, 0x08, 0x03, 0x00, 0x25, 0xf0, 0x80, 0x42, 0x0c, 0x40, 0x89, 0x02, 0x20, 0x10, 0xa1, 0x00, 0x23, 0x58, 0x80, 0x04, 0x24, 0x00, 0x08, 0x52, 0x00, 0x01, 0xa8, 0x00, 0x02, 0x04, 0x00, 0x00, 0x42, 0x00, 0x00, 0xa1, 0x00, 0x00, 0x40, 0x80, 0x00, 0x00, 0x00 };
static const unsigned char PROGMEM image_volume_muted_bits[] = { 0x01, 0xc0, 0x00, 0x02, 0x40, 0x00, 0x04, 0x40, 0x00, 0x08, 0x40, 0x00, 0xf0, 0x50, 0x40, 0x80, 0x48, 0x80, 0x80, 0x45, 0x00, 0x80, 0x42, 0x00, 0x80, 0x45, 0x00, 0x80, 0x48, 0x80, 0xf0, 0x50, 0x40, 0x08, 0x40, 0x00, 0x04, 0x40, 0x00, 0x02, 0x40, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00 };
static const unsigned char PROGMEM image_network_not_connected_bits[] = { 0x82, 0x0e, 0x44, 0x0a, 0x28, 0x0a, 0x10, 0x0a, 0x28, 0xea, 0x44, 0xaa, 0x82, 0xaa, 0x00, 0xaa, 0x0e, 0xaa, 0x0a, 0xaa, 0x0a, 0xaa, 0x0a, 0xaa, 0xea, 0xaa, 0xaa, 0xaa, 0xee, 0xee, 0x00, 0x00 };
static const unsigned char PROGMEM image_microphone_muted_bits[] = { 0x87, 0x00, 0x4f, 0x80, 0x26, 0x80, 0x13, 0x80, 0x09, 0x80, 0x04, 0x80, 0x0a, 0x00, 0x0d, 0x00, 0x2e, 0xa0, 0x27, 0x40, 0x10, 0x20, 0x0f, 0x90, 0x02, 0x08, 0x02, 0x04, 0x0f, 0x82, 0x00, 0x00 };
static const unsigned char PROGMEM image_mute_text_bits[] = { 0x8a, 0x5d, 0xe0, 0xda, 0x49, 0x00, 0xaa, 0x49, 0xc0, 0x8a, 0x49, 0x00, 0x89, 0x89, 0xe0 };
static const unsigned char PROGMEM image_cross_contour_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x80, 0x51, 0x40, 0x8a, 0x20, 0x44, 0x40, 0x20, 0x80, 0x11, 0x00, 0x20, 0x80, 0x44, 0x40, 0x8a, 0x20, 0x51, 0x40, 0x20, 0x80, 0x00, 0x00, 0x00, 0x00 };

void demonSHIT() {
  display.clearDisplay();
  u8g2_for_adafruit_gfx.setFont(u8g2_font_adventurer_tr);  // Use a larger font for the title
  u8g2_for_adafruit_gfx.setCursor(20, 40);                 // Centered vertically
  display.drawBitmap(56, 40, image_EviSmile1_bits, 18, 21, 1);
  display.setTextWrap(false);
  u8g2_for_adafruit_gfx.setCursor(30, 18);
  //u8g2_for_adafruit_gfx.print("D E M O N");
  u8g2_for_adafruit_gfx.print("2.4 G H Z");
  u8g2_for_adafruit_gfx.setCursor(40, 35);
  u8g2_for_adafruit_gfx.print("J A M R");
  display.drawBitmap(106, 19, image_Ble_connected_bits, 15, 15, 1);
  display.drawBitmap(2, 50, image_MHz_bits, 25, 11, 1);
  display.drawBitmap(1, 1, image_Error_bits, 18, 18, 1);
  display.drawBitmap(25, 38, image_Bluetooth_Idle_bits, 5, 8, 1);
  display.drawBitmap(83, 55, image_off_text_bits, 12, 5, 1);
  display.drawBitmap(109, 2, image_wifi_not_connected_bits, 19, 16, 1);
  display.drawBitmap(4, 31, image_volume_muted_bits, 18, 16, 1);
  display.drawBitmap(109, 45, image_network_not_connected_bits, 15, 16, 1);
  display.drawBitmap(92, 33, image_microphone_muted_bits, 15, 16, 1);
  display.drawBitmap(1, 23, image_mute_text_bits, 19, 5, 1);
  display.drawBitmap(32, 49, image_cross_contour_bits, 11, 16, 1);
  display.display();
}
void displayTitleScreen() {
  display.clearDisplay();
  u8g2_for_adafruit_gfx.setFont(u8g2_font_adventurer_tr);  // Use a larger font for the title
  u8g2_for_adafruit_gfx.setCursor(20, 40);                 // Centered vertically
  u8g2_for_adafruit_gfx.print("CYPHER BOX");
  // u8g2_for_adafruit_gfx.setCursor(centerX, 25); // Centered vertically
  // u8g2_for_adafruit_gfx.print("NETWORK PET");
  display.display();
}
void displayInfoScreen() {
  display.clearDisplay();
  u8g2_for_adafruit_gfx.setFont(u8g2_font_baby_tf);  // Set back to small font
  u8g2_for_adafruit_gfx.setCursor(0, 22);
  u8g2_for_adafruit_gfx.print("Welcome to 2.4GHZ JAMR!");

  u8g2_for_adafruit_gfx.setCursor(0, 30);
  u8g2_for_adafruit_gfx.print("This is a cool cyber tool.");

  u8g2_for_adafruit_gfx.setCursor(0, 38);
  u8g2_for_adafruit_gfx.print("I perform 2.4ghz attacks.");

  u8g2_for_adafruit_gfx.setCursor(0, 54);
  u8g2_for_adafruit_gfx.print("Have fun & be safe ~_~;");

  display.display();
}

// Menu Functions
void executeSelectedMenuItem() {
  switch (selectedMenuItem) {

    case BT_JAM:
      currentState = STATE_BT_JAM;
      Serial.println("BT JAM button pressed");
      displayInfo("BT JAMMER", "ACTIVATING RADIOS", "Starting....");
      initRadios();
      nonBlockingDelay(2000);  // Debounce nonBlockingDelay
      displayInfo("BT JAMMER", "RADIOS ACTIVE", "Running....");
      while (!isButtonPressed(SELECT_BUTTON_PIN)) {
        btJam();
      }
      break;

    case WIFI_JAM:
      currentState = STATE_WIFI_JAM;
      Serial.println("WIFI JAM button pressed");
      displayInfo("WIFI JAMMER", "ACTIVATING RADIOS", "Starting....");
      initRadios();
      nonBlockingDelay(2000);  // Debounce nonBlockingDelay
      displayInfo("WIFI JAMMER", "RADIOS ACTIVE", "Running....");
      while (!isButtonPressed(SELECT_BUTTON_PIN)) {
        wifiJam();
      }
      break;
    case DRONE_JAM:
      currentState = STATE_DRONE_JAM;
      Serial.println("DRONE JAM button pressed");
      displayInfo("DRONE JAMMER", "ACTIVATING RADIOS", "Starting....");
      initRadios();
      nonBlockingDelay(2000);  // Debounce nonBlockingDelay
      displayInfo("DRONE JAMMER", "RADIOS ACTIVE", "Running....");
      while (!isButtonPressed(SELECT_BUTTON_PIN)) {
        droneJam();
      }
      break;
    case MULTI_JAM:
      currentState = STATE_MULTI_JAM;
      Serial.println("DRONE JAM button pressed");
      displayInfo("MULTI CH JAMMER", "ACTIVATING RADIOS", "Starting....");
      initRadios();
      nonBlockingDelay(2000);  // Debounce nonBlockingDelay
      displayInfo("MULTI CH JAMMER", "RADIOS ACTIVE", "Running....");
      while (!isButtonPressed(SELECT_BUTTON_PIN)) {
        droneJam();
      }
      break;
  }
}

// ------- GENERAL CONFIGURATION END ------------
void setup() {
  Serial.begin(115200);
  delay(10);
  initDisplay();
  // Disable unnecessary wireless interfaces
  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();

  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("buttons init'd");

  // Initialize U8g2_for_Adafruit_GFX
  u8g2_for_adafruit_gfx.begin(display);
  // Display splash screens
  demonSHIT();
  delay(5000);  // Show title screen for 3 seconds
                //displayInfoScreen();
                //delay(5000);  // Show info screen for 5 seconds
  // Initial display
  displayInfoScreen();
  delay(3000);
  drawMenu();
}

void loop() {
  switch (currentState) {
    case STATE_MENU:
      handleMenuSelection();
      break;
    case STATE_BT_JAM:
      if (isButtonPressed(SELECT_BUTTON_PIN)) {
        currentState = STATE_MENU;
        drawMenu();
        nonBlockingDelay(500);  // Debounce nonBlockingDelay
        return;
      }
      break;
    case STATE_WIFI_JAM:
      if (isButtonPressed(SELECT_BUTTON_PIN)) {
        currentState = STATE_MENU;
        drawMenu();
        nonBlockingDelay(500);  // Debounce nonBlockingDelay
        return;
      }
      break;
    case STATE_DRONE_JAM:
      if (isButtonPressed(SELECT_BUTTON_PIN)) {
        currentState = STATE_MENU;
        drawMenu();
        nonBlockingDelay(500);  // Debounce nonBlockingDelay
        return;
      }
      break;
    case STATE_MULTI_JAM:
      if (isButtonPressed(SELECT_BUTTON_PIN)) {
        currentState = STATE_MENU;
        drawMenu();
        nonBlockingDelay(500);  // Debounce nonBlockingDelay
        return;
      }
      break;
  }
}
