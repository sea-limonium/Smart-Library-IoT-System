#include <WiFi.h> 

#include <HTTPClient.h> 

#include <ArduinoJson.h> 

 

// ===== WiFi ===== 

const char* ssid     = "LIMONIUM 8158"; 

const char* password = "6)0H11u5"; 

 

// ===== GOOGLE WEBHOOK ===== 

String WEBHOOK_URL = "https://script.google.com/macros/s/AKfycbyUEvUcUeT-aP38nKnuYxZVvFqGnAcMkPD6S6boHqSWwCsX0r-fhzotFqQWsUET4mYL/exec"; 

 

// UART definitions 

HardwareSerial& shelfUART = Serial2;   // Arduino 1 

HardwareSerial& checkoutUART = Serial; // Arduino 2 (RX0) 

 

String shelfBuffer = ""; 

String checkoutBuffer = ""; 

 

void setup() { 

  Serial.begin(9600); 

  shelfUART.begin(9600, SERIAL_8N1, 16, 17); 

  checkoutUART.begin(9600); 

 

  Serial.println("Connecting to WiFi..."); 

  WiFi.begin(ssid, password); 

 

  while (WiFi.status() != WL_CONNECTED) { 

    Serial.print("."); 

    delay(400); 

  } 

  Serial.println("\nWiFi connected!"); 

} 

 

void loop() { 

  readShelfUART(); 

  readCheckoutUART(); 

} 

 

void readShelfUART() { 

  while (shelfUART.available()) { 

    char c = shelfUART.read(); 

    if (c == '\n') { 

      shelfBuffer.trim(); 

      if (shelfBuffer.length() > 0) handleShelfEvent(shelfBuffer); 

      shelfBuffer = ""; 

    } else { 

      shelfBuffer += c; 

    } 

  } 

} 

 

void readCheckoutUART() { 

  while (checkoutUART.available()) { 

    char c = checkoutUART.read(); 

    if (c == '\n') { 

      checkoutBuffer.trim(); 

      if (checkoutBuffer.length() > 0) handleCheckoutEvent(checkoutBuffer); 

      checkoutBuffer = ""; 

    } else { 

      checkoutBuffer += c; 

    } 

  } 

} 

 

void handleShelfEvent(String line) { 

  Serial.println("[Arduino 1] → " + line); 

 

  int p1 = line.indexOf(','); 

  int p2 = line.indexOf(',', p1 + 1); 

  int p3 = line.indexOf(',', p2 + 1); 

  int p4 = line.indexOf(',', p3 + 1); 

 

  if (p1 < 0 || p2 < 0 || p3 < 0 || p4 < 0) { 

    Serial.println("Rejected: invalid shelf format"); 

    return; 

  } 

 

  String status = line.substring(p1 + 1, p2); 

  String uid    = line.substring(p2 + 1, p3); 

  String shelf  = line.substring(p3 + 1, p4); 

  String slot   = line.substring(p4 + 1); 

 

  if (status == "INSERTED") { 

    if (uid == "SCAN" || uid == "NO_TAG" || uid == "NO_UID") { 

      Serial.println("Ignoring INSERT with no UID"); 

      return; 

    } 

    sendToGooglePOST("INSERTED", uid, shelf, slot);  // Shelf uses POST 

  } 

  else if (status == "REMOVED") { 

    sendToGooglePOST("REMOVED", "NONE", shelf, slot);  // Shelf uses POST 

  } 

} 

 

void handleCheckoutEvent(String line) { 

  Serial.println("[Checkout Arduino]: " + line); 

 

  int p = line.indexOf(','); 

  if (p < 0) { 

    Serial.println("Invalid checkout format"); 

    return; 

  } 

 

  String status = line.substring(0, p); 

  String uid    = line.substring(p + 1); 

 

  status.trim(); 

  status.toUpperCase(); 

  uid.trim(); 

 

  if (status == "CHECKED_OUT" || status == "CHECKOUT") { 

    sendToGoogleGET("CHECKED_OUT", uid, "", "");  // Checkout uses GET 

    return; 

  } 

 

  Serial.println("Checkout ignored: status=" + status); 

} 

 

// ============================== 

//   SEND TO GOOGLE (POST for shelf system) 

// ============================== 

void sendToGooglePOST(String status, String uid, String shelf, String slot) { 

  if (WiFi.status() != WL_CONNECTED) { 

    Serial.println("Event Skipped -- Is WiFi down?"); 

    return; 

  } 

 

  HTTPClient http; 

  http.begin(WEBHOOK_URL); 

  http.addHeader("Content-Type", "application/json"); 

 

  String payload = "{"; 

  payload += "\"status\":\"" + status + "\","; 

  payload += "\"uid\":\"" + uid + "\","; 

  payload += "\"shelf\":\"" + shelf + "\","; 

  payload += "\"slot\":\"" + slot + "\""; 

  payload += "}"; 

 

  Serial.println("Sending POST: " + payload); 

 

  int code = http.POST(payload); 

  Serial.println("Response code: " + String(code)); 

 

  if (code > 0) { 

    String response = http.getString(); 

    Serial.println("Response: " + response); 

  } 

 

  http.end(); 

} 

 

// ============================== 

//   SEND TO GOOGLE (GET for checkout system) 

// ============================== 

void sendToGoogleGET(String status, String uid, String shelf, String slot) { 

  if (WiFi.status() != WL_CONNECTED) { 

    // Serial.println("WiFi down → event skipped"); 

    return; 

  } 

 

  String url = WEBHOOK_URL + "?status=" + status + "&uid=" + uid + "&shelf=" + shelf + "&slot=" + slot; 

   

  // Serial.println("Sending GET: " + url); 

 

  HTTPClient http; 

  http.begin(url); 

 

  int code = http.GET(); 

  // Serial.println("Response code: " + String(code)); 

 

  String response = ""; 

   

  if (code == 302 || code == 301) { 

    response = http.getString(); 

    // Serial.println("Got redirect, extracting URL..."); 

     

    int hrefStart = response.indexOf("HREF=\""); 

    if (hrefStart > 0) { 

      hrefStart += 6; 

      int hrefEnd = response.indexOf("\"", hrefStart); 

      String redirectUrl = response.substring(hrefStart, hrefEnd); 

       

      redirectUrl.replace("&amp;", "&"); 

       

      // Serial.println("Following redirect to: " + redirectUrl); 

       

      http.end(); 

       

      http.begin(redirectUrl); 

      code = http.GET(); 

      // Serial.println("Redirect response code: " + String(code)); 

    } 

  } 

   

  if (code > 0) { 

    response = http.getString(); 

    // Serial.println("Response: " + response);  // Comment this out - it interferes! 

 

    DynamicJsonDocument doc(512); 

    DeserializationError err = deserializeJson(doc, response); 

 

    if (!err) { 

      String book   = doc["bookName"] | ""; 

      String author = doc["author"] | ""; 

      String uidR   = doc["uid"] | ""; 

 

      // Serial.println("Book: " + book); 

      // Serial.println("Author: " + author); 

      // Serial.println("UID: " + uidR); 

 

      book.replace(",", " "); 

      author.replace(",", " "); 

      uidR.replace(",", " "); 

 

      sendBackToArduino(book, author, uidR); 

    } else { 

      // Serial.print("JSON parse error: "); 

      // Serial.println(err.c_str()); 

      checkoutUART.println("ERROR,Parse Failed,,"); 

    } 

  } else { 

    // Serial.println("HTTP error"); 

    checkoutUART.println("ERROR,HTTP Failed,,"); 

  } 

 

  http.end(); 

} 

 

void sendBackToArduino(String book, String author, String uid) { 

  String msg = "INFO," + book + "," + author + "," + uid + "\n"; 

  checkoutUART.print(msg); 

  // Removed debug print to avoid interference 

} 
