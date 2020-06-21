#include <Adafruit_PN532.h>
#include "Wire.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <Servo.h>
#include <string.h>
#include <LiquidCrystal_I2C.h>

#define SCK  (14)
#define MOSI (13)
#define SS   (2)
#define MISO (12)
Adafruit_PN532 nfc(SCK, MISO, MOSI, SS);

const char *ssid = "student one 2";  //ENTER YOUR WIFI SETTINGS
const char *password = "sto3ns22";

const char *fingerprint = "3983389be45e7ee3a78c2c997e31e17ba3626d07";
String deviceId = "5e20254aec957600011abb0d";
String apiUrl = "https://api.solusinegeri.com/search/attendance/" + deviceId + "/";

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
Servo servo;
const int buzz = 0;

void setup(void) {
  Serial.begin(9600);
  delay(1000);
  nfc.begin();
  nfc.SAMConfig();
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  lcd.init();// initialize LCD
  lcd.backlight();  // turn on LCD backlight
  lcd.clear();
  lcd.setCursor(0, 0); // lcd cursor one
  lcd.print(ssid);
  lcd.setCursor(0, 1); // lcd cursor one
  lcd.print("Connected");
  servo.attach(16); //D0
  servo.write(0);
  pinMode(buzz, OUTPUT);
  delay(1000);
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  String CardID = "";
  String Link = "";
  if (success) {
    unsigned int hex_num;
    hex_num =  uid[3] << 24;
    hex_num += uid[2] << 16;
    hex_num += uid[1] <<  8;
    hex_num += uid[0];
    CardID = String(hex_num);
    if(CardID.length()==9){
      CardID = '0'+CardID;
    }
    Serial.println(CardID);     //Print Card ID
    lcd.clear();
    lcd.setCursor(0, 0); // lcd cursor one
    lcd.print(CardID);
    lcd.setCursor(0, 1);
    lcd.print("Checking...");
    digitalWrite(buzz, HIGH);
    delay(500);        // ...for 1 sec
    digitalWrite(buzz, LOW);
    delay(200);        // ...for 1sec
    HTTPClient http;    //Declare object of class HTTPClient
    http.setTimeout(6000);
    Link =  apiUrl + CardID;
    Serial.println(Link);
    http.begin(Link, fingerprint);
    int httpCode = http.GET();
    delay(200);
    if (httpCode > 0) {
      String payload = http.getString();    //Get the response payload
      Serial.println(payload);
      String valid = payload.substring(0, 2);
      //  Serial.println("response: " + payload);    //Print request response payload
      Serial.println("response: " + valid);

      if (valid == "OK") {
        String userName = payload.substring(3, 19);
        String userBalance = payload.substring(20, 36);
        Serial.println(userName);
        Serial.println(userBalance);

        lcd.clear();
        lcd.setCursor(0, 0); // lcd cursor one
        lcd.print(userName);
        lcd.setCursor(0, 1);
        lcd.print(userBalance);

        servo.write(60); //servo
        delay(2000);
        servo.write(0);

      }
      else if (valid == "NO") {
        String message = payload.substring(3, 19);
        Serial.println(message);
        lcd.clear();
        lcd.setCursor(0, 0); // lcd cursor one
        lcd.print(message);
        delay(2000);
      }
      else {
        lcd.clear();
        lcd.print("error...");
      }
    } else {
      Serial.println(http.errorToString(httpCode).c_str());
      lcd.clear();
      lcd.setCursor(0, 0); // lcd cursor one
      lcd.print(http.errorToString(httpCode).c_str());
    }
    http.end();
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" TAP YOUR CARD ");

  } else {
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" TAP YOUR CARD ");
  }
  delay(700);
}
