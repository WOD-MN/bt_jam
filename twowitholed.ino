#include "RF24.h"
#include <SPI.h>
#include <ezButton.h>
#include "esp_bt.h"
#include "esp_wifi.h"

/* OLED 
SDA D21
SCL D22
*/

//OLED
#include <Arduino.h>
#include <U8x8lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
U8X8_SSD1306_128X32_UNIVISION_SW_I2C u8x8( SCL, SDA, U8X8_PIN_NONE);

SPIClass *sp = nullptr;
SPIClass *hp = nullptr;

RF24 radio(26, 15, 16000000);   //NFR24-1 HSPI CAN SET SPI SPEED TO 16000000 BY DEFAULT ITS 10000000
RF24 radio1(4, 5, 16000000);  //NRF24-2 VSPI CAN SET SPI SPEED TO 16000000 BY DEFAULT ITS 10000000


//NRF24-1 HSPI=SCK = 14, MISO = 12, MOSI = 13, CS = 15 , CE = 26
//NRF24-2 VSPI=SCK = 18, MISO = 19, MOSI = 23 ,CS = 5 , CE = 4

unsigned int flag = 0;   //HSPI// Flag variable to keep track of direction
unsigned int flagv = 0;  //VSPI// Flag variable to keep track of direction
int ch = 45;    // Variable to store value of ch
int ch1 = 45;   // Variable to store value of ch

ezButton toggleSwitch(33);

void two() {
  if (flagv == 0) {  // If flag is 0, increment ch by 4 and ch1 by 1

    ch1 += 4;
  } else {  // If flag is not 0, decrement ch by 4 and ch1 by 1

    ch1 -= 4;
  }

  if (flag == 0) {  // If flag is 0, increment ch by 4 and ch1 by 1
    ch += 2;

  } else {  // If flag is not 0, decrement ch by 4 and ch1 by 1
    ch -= 2;
  }

  // Check if ch1 is greater than 79 and flag is 0
  if ((ch1 > 79) && (flagv == 0)) {
    flagv = 1;                             // Set flag to 1 to change direction
  } else if ((ch1 < 2) && (flagv == 1)) {  // Check if ch1 is less than 2 and flag is 1
    flagv = 0;                             // Set flag to 0 to change direction
  }

  // Check if ch is greater than 79 and flag is 0
  if ((ch > 79) && (flag == 0)) {
    flag = 1;                            // Set flag to 1 to change direction
  } else if ((ch < 2) && (flag == 1)) {  // Check if ch is less than 2 and flag is 1
    flag = 0;                            // Set flag to 0 to change direction
  }
  radio.setChannel(ch);
  radio1.setChannel(ch1);
  
}

void one() {
  ////RANDOM CHANNEL
  radio1.setChannel(random(80));
  radio.setChannel(random(80));
  delayMicroseconds(random(60));//////REMOVE IF SLOW
}

void setup() {
  Serial.begin(115200);
  // Initialize OLED
   u8x8.begin();
   u8x8.setPowerSave(0);
   u8x8.setFlipMode(1);
        u8x8.setFont(u8x8_font_chroma48medium8_r);
        u8x8.drawString(0,0,"BLE JAMMER WT24");
        u8x8.drawString(0,1,"...............");
        u8x8.drawString(0,3,"EducationalOnly");
        //u8x8.drawString(0,3,"...");
        u8x8.refreshDisplay();    // only required for SSD1606/7  
        delay(5000);
  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();
  esp_wifi_disconnect();
  toggleSwitch.setDebounceTime(50);

  initHP();
  initSP();
}

void initSP() {
  sp = new SPIClass(VSPI);
  sp->begin();
  if (radio1.begin(sp)) {
    Serial.println("VSPI Jammer Started !!!");
    //u8x8.clear(); //to clear display
        u8x8.setFont(u8x8_font_chroma48medium8_r);
        u8x8.drawString(0,0,"VSPI Jammer ON!");
        u8x8.refreshDisplay();    // only required for SSD1606/7 
    radio1.setAutoAck(false);
    radio1.stopListening();
    radio1.setRetries(0, 0);
    radio1.setPALevel(RF24_PA_MAX, true);
    radio1.setDataRate(RF24_2MBPS);
    radio1.setCRCLength(RF24_CRC_DISABLED);
    radio1.printPrettyDetails();
    radio1.startConstCarrier(RF24_PA_MAX, ch1);
  } else {
    Serial.println("VSPI Jammer couldn't start !!!");
     //u8x8.clear(); //to clear display
        u8x8.setFont(u8x8_font_chroma48medium8_r);
        u8x8.drawString(0,0,"VSPI Jammer fail");
        u8x8.refreshDisplay();    // only required for SSD1606/7 
  }
}
void initHP() {
  hp = new SPIClass(HSPI);
  hp->begin();
  if (radio.begin(hp)) {
    Serial.println("HSPI Started !!!");
    //u8x8.clear(); //to clear display
        u8x8.setFont(u8x8_font_chroma48medium8_r);
        u8x8.drawString(0,1,"HSPI Jammer ON!");
        u8x8.refreshDisplay();    // only required for SSD1606/7
    radio.setAutoAck(false);
    radio.stopListening();
    radio.setRetries(0, 0);
    radio.setPALevel(RF24_PA_MAX, true);
    radio.setDataRate(RF24_2MBPS);
    radio.setCRCLength(RF24_CRC_DISABLED);
    radio.printPrettyDetails();
    radio.startConstCarrier(RF24_PA_MAX, ch);
  } else {
    Serial.println("HSPI couldn't start !!!");
    u8x8.clear(); //to clear display
        u8x8.setFont(u8x8_font_chroma48medium8_r);
        u8x8.drawString(0,1,"HSPI Jammer fail");
        u8x8.refreshDisplay();    // only required for SSD1606/7
  }
}

void loop() {

  toggleSwitch.loop();  // MUST call the loop() function first
  
  int state = toggleSwitch.getState();


  if (state == HIGH)
    two();

  else {
    one();
  }
}
