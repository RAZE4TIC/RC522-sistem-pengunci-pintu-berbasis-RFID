/*
 * RC522 Door Access Control System
 * Author: GENJI_KAZEN
 * Date: 13/06/2023
 * Version :1.0
 */

#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

const int buzzerPin = 7;
const int successLedPin = 8;
const int alarmLedPin = 6;
const int relayPin = 4;

const String doorName = "PRODUKSI";

String authorizedIDs[] = {"43e266bf"};  // Replace "ID1", "ID2", "ID3" with your actual authorized card IDs

void setup() {
  Serial.begin(9600);

  SPI.begin();
  mfrc522.PCD_Init();

  lcd.begin();
  lcd.backlight();

  rtc.begin();

  pinMode(buzzerPin, OUTPUT);
  pinMode(successLedPin, OUTPUT);
  pinMode(alarmLedPin, OUTPUT);
  pinMode(relayPin, OUTPUT);

  digitalWrite(successLedPin, LOW);
  digitalWrite(alarmLedPin, LOW);
  digitalWrite(relayPin, LOW);

  lcd.setCursor(0, 0);
  lcd.print("RC522 Access");
  lcd.setCursor(0, 1);
  lcd.print("Door: " + doorName);
  delay(2000);
  lcd.clear();
}


void loop() {
  // Check if a new card is present
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Get the Chip ID from the UID
    String chipID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      chipID += String(mfrc522.uid.uidByte[i], HEX);
    }

    // Search for the Chip ID in the authorized IDs list
    bool isAuthorized = false;
    for (int i = 0; i < sizeof(authorizedIDs) / sizeof(authorizedIDs[0]); i++) {
      if (chipID == authorizedIDs[i]) {
        isAuthorized = true;
        break;
      }
    }

    if (isAuthorized) {
      // Unlock the door by activating the relay
      digitalWrite(relayPin, HIGH);

      // Display success message with current date and time on the LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Access Granted");
      lcd.setCursor(0, 1);
      lcd.print(rtc.now().timestamp(DateTime::TIMESTAMP_FULL));

      // Indicate successful access with an LED
      digitalWrite(successLedPin, HIGH);
      delay(2000);  // Keep the LED on for 2 seconds

      // Lock the door again
      digitalWrite(relayPin, LOW);
      digitalWrite(successLedPin, LOW);
    } else {
      // Unauthorized access detected
      digitalWrite(alarmLedPin, HIGH);  // Activate the alarm LED
      digitalWrite(buzzerPin, HIGH);    // Activate the buzzer

      // Display alarm message on the LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Unauthorized");
      lcd.setCursor(0, 1);
      lcd.print("Access!");

      delay(2000);  // Alarm duration: 2 seconds
      digitalWrite(alarmLedPin, LOW);
      digitalWrite(buzzerPin, LOW);
    }

    // Print Chip ID to Serial Monitor
    Serial.println("Chip ID: " + chipID);

    mfrc522.PICC_HaltA();      // Halt the tag
    mfrc522.PCD_StopCrypto1(); // Stop encryption
    delay(2000);  // Delay before reading the next tag

    // Clear the LCD display
    lcd.clear();
  }
  
DateTime now = rtc.now();
  lcd.setCursor(0, 0);
  lcd.print("Date: " + String((now.day() < 10 ? "0" : "") + String(now.day())) + "/" + String((now.month() < 10 ? "0" : "") + String(now.month())) + "/" + String(now.year()));
  lcd.setCursor(0, 1);
  lcd.print("Time: " + String(now.hour()) + ":" + String((now.minute() < 10 ? "0" : "") + String(now.minute())) + ":" + String((now.second() < 10 ? "0" : "") + String(now.second())));

  delay(1000);
}
