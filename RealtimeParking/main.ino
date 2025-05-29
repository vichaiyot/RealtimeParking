//กำหนด libraly การใช้การงานของ งาน
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <SPI.h>
#include <RFID.h>
#include <Servo.h>

// Define RFID pins
#define SS_PIN 9
#define RST_PIN 16

RFID rfid(SS_PIN, RST_PIN);

// WiFi 
char ssid[] = "vivo Y21";
char password[] = "12345678";

// Firebase
#define FIREBASE_HOST "realtime-parking-bf00c-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "elLcT97PNt1vgMmAOYuObSqXvmjtrYOCJ7q6Ib0I"

// Firebase objects
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

Servo Opan, Exit;  // entry and exit 
int sensorValues[4]; // กำหนดตัวแปร ir 4 ตัว
int serNum[5] = {0}; // กำหนดตัวแปรรหัส id


#define EXIT_PATH "/Control/Exit"

// Function to update Firebase
void updateFirebase(String path, String value) {
  if (Firebase.setString(firebaseData, path, value)) {
    Serial.println("อัปเดต Firebase สำเร็จ");
  } else {
    Serial.print("Firebase Error: ");
    Serial.println(firebaseData.errorReason());
  }
}

// Function to handle exit gate control from Firebase
void handleExitControl() {
  // พยายามดึงคำสั่งจาก Firebase
  if (Firebase.getString(firebaseData, "/Exit")) {
    String exitCommand = firebaseData.stringData();
    if (exitCommand == "OPEN") {
      Serial.println("เปิดประตูทางออก");
      Exit.write(180);  // เปิดประตู
      delay(5000);
      Exit.write(0);  // ปิดประตู
      updateFirebase("/Exit", "CLOSED");  // รีเซ็ตคำสั่ง
    }
  } else {
    // หากเส้นทางนี้ไม่มีให้สร้างด้วยค่าเริ่มต้น
    Serial.println("เส้นทาง Exit ไม่มี สร้างขึ้นด้วยค่าเริ่มต้น 'CLOSED'");
    Firebase.setString(firebaseData, "/Exit", "CLOSED");
  }
}


// Handle card read event
void handleCardRead() {
  bool isNewCard = false;

  // Check if the card is new
  for (int i = 0; i < 5; i++) {
    if (rfid.serNum[i] != serNum[i]) {
      isNewCard = true;
      break;
    }
  }

  if (isNewCard) {
    memcpy(serNum, rfid.serNum, 5);

    // Convert card ID to string
    String cardID = "";
    for (int i = 0; i < 5; i++) {
      cardID += String(rfid.serNum[i], HEX);
      if (i < 4) cardID += ":";
    }

    Serial.println("ตรวจพบการ์ดใหม่: " + cardID);
    updateFirebase("/CardID", cardID);

    // ตรวจสอบ cardID ใน Firebase
    if (Firebase.getJSON(firebaseData, "/Users")) {
      FirebaseJson users = firebaseData.jsonObject();
      String userKey, userData;
      bool cardFound = false;

      int userCount = users.iteratorBegin();
      int valueType;
      for (int i = 0; i < userCount; i++) {
        users.iteratorGet(i, valueType, userKey, userData);

        // ดึงข้อมูล cardID ของผู้ใช้
        FirebaseJson userJson;
        userJson.setJsonData(userData);

        FirebaseJsonData jsonData;
        userJson.get(jsonData, "cardID");

        if (jsonData.stringValue == cardID) {
          cardFound = true;
          break;
        }
      }
      users.iteratorEnd();

      if (cardFound) {
        Serial.println("บัตรตรงกับข้อมูลในระบบ, เปิดประตู!");
        // Open the entry gate
        Opan.write(180);
        delay(5000);
        Opan.write(0);
        //updateFirebase("/CardID", "00:00:00:00:00");
      } else {
        Serial.println("บัตรไม่ตรงกับข้อมูลในระบบ");
      }
      updateFirebase("/CardID", "00:00:00:00:00");
    } else {
      Serial.print("เกิดข้อผิดพลาดในการดึงข้อมูล Firebase: ");
      Serial.println(firebaseData.errorReason());
    }
  }
}







//void createUser(String userID, String firstname, String lastname, String cardID ) {
//  String userPath = "/Users/" + userID;  // Path ของผู้ใช้
//  FirebaseJson userData;
//  
//  userData.set("firstname", firstname);
//  userData.set("lastname", lastname);
//  userData.set("cardID", cardID);
//
//  if (Firebase.setJSON(firebaseData, userPath, userData)) {
//    Serial.println("เพิ่มผู้ใช้สำเร็จ: " + userID);
//  } else {
//    Serial.print("เพิ่มผู้ใช้ล้มเหลว: ");
//    Serial.println(firebaseData.errorReason());
//  }
//}


void setup() {
  Serial.begin(115200);

  // Initialize SPI and RFID
  SPI.begin();
  rfid.init();
  
  // Initialize pins
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);

  Opan.attach(15); // D8
  Exit.attach(3);  // D9

  
  Opan.write(0);
  Exit.write(0);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());

  // Firebase setup
  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&firebaseConfig, &firebaseAuth);

  // Check Firebase connection
  if (Firebase.ready()) {
    Serial.println("Firebase connected!");
  } else {
    Serial.printf("Firebase initialization failed: %s\n", firebaseData.errorReason().c_str());
  }

  Serial.println("ระบบพร้อมใช้งาน");

   Firebase.setString(firebaseData, "/Exit", "CLOSED");

 
}


void loop() {
  // Check RFID card
  if (rfid.isCard() && rfid.readCardSerial()) {
    handleCardRead();
  }
  rfid.halt();

  // Update sensor statuses to Firebase
  sensorValues[0] = digitalRead(D1);
  sensorValues[1] = digitalRead(D2);
  sensorValues[2] = digitalRead(D3);
  sensorValues[3] = digitalRead(D4);

  updateFirebase("/Sensors/Sensor1", sensorValues[0] == LOW ? "FULL" : "EN");
  updateFirebase("/Sensors/Sensor2", sensorValues[1] == LOW ? "FULL" : "EN");
  updateFirebase("/Sensors/Sensor3", sensorValues[2] == LOW ? "FULL" : "EN");
  
  updateFirebase("/Sensors/SensorD4", digitalRead(D4) == LOW ? "OBJECT_PRESENT" : "NO_OBJECT");

  // Handle exit control (from Firebase)
  handleExitControl();  // Added the semicolon here

  delay(1000); // Prevent frequent Firebase writes
}
