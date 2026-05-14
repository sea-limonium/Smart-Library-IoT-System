#include <SPI.h> 
#include <MFRC522.h> 

// ----- RFID READERS ----- 
#define SS1_PIN 10 
#define RST1_PIN 9 

#define SS2_PIN 8 
#define RST2_PIN 7 

MFRC522 rfid1(SS1_PIN, RST1_PIN); 
MFRC522 rfid2(SS2_PIN, RST2_PIN); 

// ----- TCRT SENSORS ----- 
#define TCRT_A 2 
#define TCRT_B 3 
#define TCRT_C 4 
#define TCRT_D 5 

bool lastA = LOW; 
bool lastB = LOW; 
bool lastC = LOW; 
bool lastD = LOW; 

void setup() { 
  Serial.begin(9600); 
  SPI.begin(); 

  rfid1.PCD_Init(); 
  rfid2.PCD_Init(); 

  pinMode(TCRT_A, INPUT); 
  pinMode(TCRT_B, INPUT); 
  pinMode(TCRT_C, INPUT); 
  pinMode(TCRT_D, INPUT); 

  Serial.println("READY"); 
} 

void loop() { 
  checkSlot(TCRT_A, lastA, "A", 1, rfid1, rfid2); 
  checkSlot(TCRT_B, lastB, "B", 1, rfid1, rfid2); 
  checkSlot(TCRT_C, lastC, "C", 2, rfid1, rfid2); 
  checkSlot(TCRT_D, lastD, "D", 2, rfid1, rfid2); 
} 

void checkSlot(int pin, bool &lastState, String slotName,  
               int shelfNum, MFRC522 &r1, MFRC522 &r2) { 
  bool current = digitalRead(pin); 

  if (current != lastState) { 
    if (current == HIGH) { 
      Serial.print("EVENT,INSERTED,SCAN,"); 
      Serial.print(shelfNum); 
      Serial.print(","); 
      Serial.println(slotName); 

      scanRFID(shelfNum, slotName, r1, r2); 
    } else { 
      Serial.print("EVENT,REMOVED,NO_UID,"); 
      Serial.print(shelfNum); 
      Serial.print(","); 
      Serial.println(slotName); 
    } 

    lastState = current; 
    delay(200); 
  } 
} 

void scanRFID(int shelfNum, String slotName,  
              MFRC522 &r1, MFRC522 &r2) { 
  unsigned long timeout = millis() + 1000; 

  while (millis() < timeout) { 
    // Reader 1 
    if (r1.PICC_IsNewCardPresent() && r1.PICC_ReadCardSerial()) { 
      printUID(shelfNum, slotName, r1); 
      r1.PICC_HaltA(); 
      return; 
    } 

    // Reader 2 
    if (r2.PICC_IsNewCardPresent() && r2.PICC_ReadCardSerial()) { 
      printUID(shelfNum, slotName, r2); 
      r2.PICC_HaltA(); 
      return; 
    } 
  } 

  Serial.print("EVENT,INSERTED,NO_TAG,"); 
  Serial.print(shelfNum); 
  Serial.print(","); 
  Serial.println(slotName); 
} 

void printUID(int shelfNum, String slotName, MFRC522 &reader) { 
  Serial.print("EVENT,INSERTED,"); 

  for (byte i = 0; i < reader.uid.size; i++) { 
    Serial.print(reader.uid.uidByte[i], HEX); 
  } 

  Serial.print(","); 
  Serial.print(shelfNum); 
  Serial.print(","); 
  Serial.println(slotName); 
}
