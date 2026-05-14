#include <Servo.h> 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h> 

// === PIN CONFIGURATION === 
const int PIR_PIN = 7; 
const int TRIG_PIN = 8; 
const int ECHO_PIN = 9; 
const int RED_LED = 2; 
const int GREEN_LED = 4; 
const int SERVO_PIN = 5; 

// === DETECTION SETTINGS === 
const int DETECTION_DISTANCE = 50;   // Start tracking at 50cm 
const int CLOSE_DISTANCE = 15;       // Very close = 15cm 
const unsigned long COOLDOWN = 4000; // 4 seconds between detections 
const int GATE_OPEN_TIME = 3000; 

// === GATE POSITIONS === 
const int GATE_CLOSED = 90; 
const int GATE_OPEN = 0; 

// === OBJECTS === 
Servo gateServo; 
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// === STATE VARIABLES === 
int peopleCount = 0; 
bool isGateOpen = false; 
unsigned long lastTriggerTime = 0; 

// Direction detection variables 
bool isTracking = false; 
long firstDistance = 0; 
long lastDistance = 0; 
unsigned long trackingStartTime = 0; 
const unsigned long TRACKING_TIMEOUT = 3000; // 3 seconds to complete movement 

// ======================================== 
// SENSOR FUNCTIONS 
// ======================================== 

// Get stable distance reading (average of 3 readings) 
long getStableDistance() { 
  long total = 0; 
  int validReadings = 0; 
   
  for (int i = 0; i < 3; i++) { 
    digitalWrite(TRIG_PIN, LOW); 
    delayMicroseconds(2); 
    digitalWrite(TRIG_PIN, HIGH); 
    delayMicroseconds(10); 
    digitalWrite(TRIG_PIN, LOW); 
     
    long duration = pulseIn(ECHO_PIN, HIGH, 30000); 
    if (duration > 0) { 
      long distance = duration * 0.034 / 2; 
      if (distance < 200) {  // Ignore readings over 200cm 
        total += distance; 
        validReadings++; 
      } 
    } 
    delay(10); 
  } 
   
  if (validReadings == 0) return 999; 
  return total / validReadings; 
} 

// Detect direction of movement 
bool detectDirection(bool &isEntry) { 
  int motion = digitalRead(PIR_PIN); 
  long distance = getStableDistance(); 
  unsigned long now = millis(); 
   
  // Debug output 
  static unsigned long lastDebug = 0; 
  if (millis() - lastDebug > 1000) { 
    Serial.print("PIR: "); 
    Serial.print(motion == HIGH ? "Motion" : "Clear "); 
    Serial.print(" | Distance: "); 
    Serial.print(distance); 
    Serial.print(" cm"); 
    if (isTracking) { 
      Serial.print(" | TRACKING (First: "); 
      Serial.print(firstDistance); 
      Serial.print("cm)"); 
    } 
    Serial.println(); 
    lastDebug = millis(); 
  } 
   
  // Check for tracking timeout 
  if (isTracking && (now - trackingStartTime > TRACKING_TIMEOUT)) { 
    Serial.println("⚠ Tracking timeout - reset"); 
    isTracking = false; 
  } 
   
  // START TRACKING: Motion detected and person is at medium distance 
  if (!isTracking && motion == HIGH && distance < DETECTION_DISTANCE && distance > CLOSE_DISTANCE) { 
    isTracking = true; 
    firstDistance = distance; 
    trackingStartTime = now; 
    Serial.println("→ Started tracking movement..."); 
    Serial.print("   Initial distance: "); 
    Serial.println(firstDistance); 
    return false; 
  } 
   
  // CONTINUE TRACKING: Person got very close 
  if (isTracking && motion == HIGH && distance < CLOSE_DISTANCE) { 
    lastDistance = distance; 
     
    // Determine direction based on distance change 
    long distanceChange = firstDistance - lastDistance; 
     
    Serial.println(); 
    Serial.println("═══════════════════════════"); 
    Serial.print("First Distance: "); 
    Serial.print(firstDistance); 
    Serial.println(" cm"); 
    Serial.print("Final Distance: "); 
    Serial.print(lastDistance); 
    Serial.println(" cm"); 
    Serial.print("Change: "); 
    Serial.print(distanceChange); 
    Serial.println(" cm"); 
     
    // If distance decreased significantly = ENTRY (approaching) 
    // If distance stayed similar or increased = EXIT (person was already close) 
    if (distanceChange > 15) { 
      isEntry = true; 
      Serial.println("→→→ ENTRY: Person approached from far"); 
    } else { 
      isEntry = false; 
      Serial.println("←←← EXIT: Person was already close"); 
    } 
    Serial.println("═══════════════════════════"); 
    Serial.println(); 
     
    isTracking = false; 
    return true; 
  } 
   
  return false; 
} 

// ======================================== 
// GATE CONTROL 
// ======================================== 

void moveGate(int targetPosition) { 
  int currentPos = (targetPosition == GATE_OPEN) ? GATE_CLOSED : GATE_OPEN; 
  int step = (targetPosition > currentPos) ? 1 : -1; 
   
  for (int pos = currentPos; pos != targetPosition; pos += step) { 
    gateServo.write(pos); 
    delay(15); 
  } 
  gateServo.write(targetPosition); 
} 

void openGate(bool isEntry) { 
  isGateOpen = true; 
   
  Serial.println(); 
  Serial.println(isEntry ? "════ ENTRY ════" : "════ EXIT ════"); 
   
  // Show on LCD 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Gate Opening..."); 
  lcd.setCursor(0, 1); 
  lcd.print(isEntry ? ">>> ENTRY >>>" : "<<< EXIT <<<"); 
   
  // LEDs 
  digitalWrite(RED_LED, LOW); 
  digitalWrite(GREEN_LED, HIGH); 
   
  delay(1500); 
   
  // Open gate 
  moveGate(GATE_OPEN); 
   
  // Update count 
  if (isEntry) { 
    peopleCount++; 
    Serial.println("Person ENTERED"); 
  } else { 
    peopleCount--; 
    if (peopleCount < 0) peopleCount = 0; 
    Serial.println("Person EXITED"); 
  } 

  Serial.print("EVENT,"); 
  Serial.println(isEntry ? "ENTRY" : "EXIT"); 
   
  // Show count 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("GATE OPEN"); 
  lcd.setCursor(0, 1); 
  lcd.print("Count: "); 
  lcd.print(peopleCount); 
   
  Serial.print("People Inside: "); 
  Serial.println(peopleCount); 
   
  delay(GATE_OPEN_TIME); 
   
  // Close gate 
  moveGate(GATE_CLOSED); 
  digitalWrite(GREEN_LED, LOW); 
  digitalWrite(RED_LED, HIGH); 
   
  Serial.println("Gate Closed\n"); 
   
  isGateOpen = false; 
} 

// ======================================== 
// LCD DISPLAY 
// ======================================== 

void updateStatusDisplay() { 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
   
  if (peopleCount == 0) { 
    lcd.print("Status: EMPTY"); 
  } else if (peopleCount < 3) { 
    lcd.print("Come Visit :)"); 
  } else if (peopleCount < 5) { 
    lcd.print("One at a time ;)"); 
  } else { 
    lcd.print("Occupied-No space"); 
  } 
   
  lcd.setCursor(0, 1); 
  lcd.print("Count: "); 
  lcd.print(peopleCount); 
} 

// ======================================== 
// SETUP 
// ======================================== 

void setup() { 
  Serial.begin(9600); 
   
  lcd.init(); 
  lcd.backlight(); 
  lcd.clear(); 
  lcd.print("Direction Detect"); 
  lcd.setCursor(0, 1); 
  lcd.print("Starting..."); 
   
  pinMode(PIR_PIN, INPUT); 
  pinMode(TRIG_PIN, OUTPUT); 
  pinMode(ECHO_PIN, INPUT); 
  pinMode(RED_LED, OUTPUT); 
  pinMode(GREEN_LED, OUTPUT); 
   
  gateServo.attach(SERVO_PIN); 
  gateServo.write(GATE_CLOSED); 
   
  digitalWrite(RED_LED, HIGH); 
  digitalWrite(GREEN_LED, LOW); 
   
  delay(2000); 
  updateStatusDisplay(); 
   
  Serial.println("\n═══ DIRECTION DETECTION GATE SYSTEM ═══"); 
  Serial.println("Method: Distance change analysis"); 
  Serial.println("Entry: Approach from far → close"); 
  Serial.println("Exit: Already close → very close"); 
  Serial.println("Waiting...\n"); 
} 

// ======================================== 
// MAIN LOOP 
// ======================================== 

void loop() { 
  unsigned long now = millis(); 
   
  // Skip if in cooldown or gate is open 
  if (now - lastTriggerTime < COOLDOWN || isGateOpen) { 
    delay(200); 
    return; 
  } 
   
  // Check for direction 
  bool isEntry = true;  // Default 
  if (detectDirection(isEntry)) { 
    lastTriggerTime = now; 
    openGate(isEntry); 
    updateStatusDisplay(); 
  } 
   
  delay(200); 
}
