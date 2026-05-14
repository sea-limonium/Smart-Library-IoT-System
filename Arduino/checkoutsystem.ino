#include <SPI.h> 
#include <MFRC522.h> 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h> 

LiquidCrystal_I2C lcd(0x27, 16, 2); 

#define SS_PIN 10 
#define RST_PIN 9 

MFRC522 rfid(SS_PIN, RST_PIN); 

unsigned long lastMessageSwitch = 0; 
int currentMessage = 0; 
bool messageDisplayed = false;  // Track if current message is already shown 

void setup() { 
  Serial.begin(9600); 
  SPI.begin(); 
  rfid.PCD_Init(); 

  lcd.init(); 
  lcd.backlight(); 
   
  lastMessageSwitch = millis(); 
  resetLCD();  // Show initial message 
} 

void resetLCD() { 
  // Alternating welcome messages every 3 seconds 
  if (millis() - lastMessageSwitch > 3000) { 
    currentMessage = 1 - currentMessage; 
    lastMessageSwitch = millis(); 
    messageDisplayed = false;  // Message changed, need to redraw 
  } 
   
  // Only redraw if message changed 
  if (!messageDisplayed) { 
    lcd.clear(); 
    if (currentMessage == 0) { 
      lcd.setCursor(1, 0); 
      lcd.print("Welcome to the"); 
      lcd.setCursor(0, 1); 
      lcd.print("Checkout System!"); 
    } else { 
      lcd.setCursor(1, 0); 
      lcd.print("Scan your book"); 
      lcd.setCursor(2, 1); 
      lcd.print("to checkout!!"); 
    } 
    messageDisplayed = true;  // Mark as displayed 
  } 
} 

String readSerialLine() { 
  String s = ""; 
  unsigned long start = millis(); 
   
  while (millis() - start < 10000) { 
    if (Serial.available()) { 
      char c = Serial.read(); 
      if (c == '\n') break; 
      s += c; 
    } 
    delay(10); 
  } 
  return s; 
} 

void showBookOnLCD(String uid, String name, String author) { 
  // Show Title 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Title:"); 
  lcd.setCursor(0, 1); 
  lcd.print(name.substring(0, 16)); 
  delay(3000); 

  // Show Author 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Author:"); 
  lcd.setCursor(0, 1); 
  lcd.print(author.substring(0, 16)); 
  delay(3000); 

  // Show UID 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("UID:"); 
  lcd.setCursor(0, 1); 
  lcd.print(uid.substring(0, 16)); 
  delay(3000); 

  // Thank you message 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Checked out! :D"); 
  lcd.setCursor(0, 1); 
  lcd.print("Have a nice day!"); 
  delay(3000); 
   
  messageDisplayed = false;  // Reset flag so welcome message redraws 
  lastMessageSwitch = millis();  // Reset timer 
  resetLCD(); 
} 

void loop() { 
  // Update alternating welcome message 
  resetLCD(); 
   
  if (!rfid.PICC_IsNewCardPresent()) return; 
  if (!rfid.PICC_ReadCardSerial()) return; 

  String cleanUID = ""; 
  for (byte i = 0; i < rfid.uid.size; i++) { 
    if (rfid.uid.uidByte[i] < 0x10) cleanUID += "0"; 
    cleanUID += String(rfid.uid.uidByte[i], HEX); 
  } 
  cleanUID.toUpperCase(); 

  Serial.print("CHECKED_OUT,"); 
  Serial.println(cleanUID); 
  Serial.flush(); 

  lcd.clear(); 
  lcd.setCursor(2, 0); 
  lcd.print("Checking out"); 
  lcd.setCursor(1, 1); 
  lcd.print("your book..."); 

  // Wait for INFO line 
  String line = ""; 
  unsigned long startWait = millis(); 
   
  while (millis() - startWait < 15000) { 
    line = readSerialLine(); 
     
    if (line.startsWith("INFO,")) { 
      break; 
    } 
     
    if (line.length() == 0) { 
      delay(100); 
    } 
  } 
   
  if (!line.startsWith("INFO,")) { 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print("Checkout failed!"); 
    lcd.setCursor(0, 1); 
    lcd.print("Try again"); 
    delay(3000); 
    resetLCD(); 
    rfid.PICC_HaltA(); 
    return; 
  } 

  // Parse CSV: INFO,BookName,Author,UID 
  int p1 = line.indexOf(','); 
  int p2 = line.indexOf(',', p1 + 1); 
  int p3 = line.indexOf(',', p2 + 1); 
   
  if (p1 > 0 && p2 > 0 && p3 > 0) { 
    String name = line.substring(p1 + 1, p2); 
    String author = line.substring(p2 + 1, p3); 
    String uid = line.substring(p3 + 1); 
     
    name.trim(); 
    author.trim(); 
    uid.trim(); 
     
    showBookOnLCD(uid, name, author); 
    rfid.PICC_HaltA(); 
    return; 
  } else { 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print("Error parsing"); 
    lcd.setCursor(0, 1); 
    lcd.print("book data"); 
    delay(3000); 
    resetLCD(); 
    rfid.PICC_HaltA(); 
    return; 
  } 
}
