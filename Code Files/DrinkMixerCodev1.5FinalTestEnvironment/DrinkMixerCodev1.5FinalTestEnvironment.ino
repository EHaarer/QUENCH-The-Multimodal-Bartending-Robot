#include <NewPing.h>
#include <SPI.h>
#include <MFRC522.h>
#include "WiFiS3.h"

//Used for Wifi input
#include "arduino_secrets.h"

//Setup Variables for Webserver
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;           // your network key index number (needed only for WEP)

int led = LED_BUILTIN;
int numShotsServer = 1;
int status = WL_IDLE_STATUS;
WiFiServer server(80);

#define SS_PIN 10  // SDA pin for MFRC522
#define RST_PIN 9  // Reset pin for MFRC522

#define RED_LED_PIN 7       // Red LED pin
#define GREEN_LED_PIN 8     // Green LED pin
#define BLUE_LED_PIN A3     // Blue LED pin
unsigned long prevMil = 0;  // Stores last time Blue LED was updated
bool blueledState = false;

//Define pins and Values for Proximity Sensor
#define TRIG_PIN 6        // Trig pin of the ultrasonic sensor
#define ECHO_PIN 5        // Echo pin of the ultrasonic sensor
#define MAX_DISTANCE 25   // Maximum distance to measure (in cm bc metric is superior bby)
#define PING_INTERVAL 50  // Interval between pings in milliseconds

//Code to set up RFID scanner
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Set up scanner
byte scannedUIDs[10][4];           // Array to store up to 10 scanned Cards/UIDs
int numScanned = 0;                // Curr Number of UIDs stored

//Needed Variables to track rotations of pumps
int rotateA;
int rotateB;
int rotateC;

int customA = 0;
int customB = 0;
int customC = 0;

bool aHigh = false;
bool bHigh = false;
bool cHigh = false;

const int aPos = 2;
const int bPos = 3;
const int cPos = 4;

//The number of rotations to output 1.5oz of liquid
int shotSize = 45 * 3;

int tester = 0;
//Proximity Sensor Setup/Define Values

bool cupPresent = false;
unsigned long previousMillis = 0;
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

//TODO: Make sure that the correct optical sensor/relay/motor power is all correctly connected to the proper optical sensor

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  //RFID SPI setup Code
  SPI.begin();         // Initiate SPI bus
  mfrc522.PCD_Init();  // Initiate MFRC522
  //LEDs above RFID scanner
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  // Serial.println("Place your card on the reader...");

  //Instantiates the rotate Variable values for override
  rotateA = 0;
  rotateB = 0;
  rotateC = 0;

  pinMode(aPos, OUTPUT);
  pinMode(bPos, OUTPUT);
  pinMode(cPos, OUTPUT);

  digitalWrite(aPos, HIGH);
  digitalWrite(bPos, HIGH);
  digitalWrite(cPos, HIGH);

  //Below is test code for proximity sensor
  pinMode(TRIG_PIN, OUTPUT);  // Set the Trig pin as an output
  pinMode(ECHO_PIN, INPUT);   // Set the Echo pin as an input

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    // Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);  // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();     // start the web server on port 80
  printWifiStatus();  // you're connected now, so print out the status
}

void loop() {
  WiFiClient client = server.available();  // listen for incoming clients

  if (client) {                    // if you get a client,
    Serial.println("new client");  // print a message out the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    while (client.connected()) {   // loop while the client's connected
      if (client.available()) {    // if there's bytes to read from the client,
        char c = client.read();    // read a byte, then
        Serial.write(c);           // print it out to the serial monitor
        if (c == '\n') {           // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");

            client.println();

            // the content of the HTTP response follows the header:
            client.print("<p style=\"font-size:9vw;\">Welcome to Q.U.E.N.C.H!</p>");
            client.print("<p style=\"font-size:3vw;\">[Quick User-friendly Environment for Networked Cocktail Handling]</p>");
            client.print("<p style=\"font-size:4vw;\">Choose Portion Size: <a href=\"/0\">0</a> <a href=\"/1\">1</a> <a href=\"/2\">2</a> <a href=\"/3\">3</a><br></p>");
            client.print("<p style=\"font-size:3vw;\">Current Portion: " + String(numShotsServer) + " shots</p>");
            client.print("<p style=\"font-size:4vw;\">Choose the serving sizes and drinks below, then pour your drink!</p>");
            client.print("<p style=\"font-size:4vw;\">Click to <a href=\"/A\">Dispense " + String(numShotsServer) + " of A</a><br></p>");
            client.print("<p style=\"font-size:4vw;\">Click to <a href=\"/B\">Dispense " + String(numShotsServer) + " of B</a><br></p>");
            client.print("<p style=\"font-size:4vw;\">Click to <a href=\"/C\">Dispense " + String(numShotsServer) + " of C</a><br></p>");
            client.print("<p style=\"font-size:3vw;\">Current Drink: " + String(customA) + " shots of A, " + String(customB) + " shots of B, " + String(customC) + " shots of C"
                                                                                                                                                                   "</p>");  //" + String(customA) + " B: " + String(customB) + " C: " + String(customC)
            client.println("<p style=\"font-size:5vw;\"><a href=\"/P\">Click to Pour Your Drink!</a><br></p>");
            client.print("<p style=\"font-size:7vw;\">Master Controls:</p>");
            client.print("<p style=\"font-size:7vw;\"><a href=\"/R\">RESET ALL CARDS</a><br></p>");
            client.print("<p style=\"font-size:7vw;\"><a href=\"/S\">EMERGENCY STOP</a><br></p>");

            // The HTTP response ends with another blank line:
            client.println();


            // client.print("<p style=\"font-size:9vw;\">Welcome to Q.U.E.N.C.H!</p>");
            // client.print("<p style=\"font-size:3vw;\">[Quick User-friendly Environment for Networked Cocktail Handling]</p>");
            // client.print("<p style=\"font-size:4vw;\">Choose the serving sizes and drinks below, then pour your drink!</p>");
            // client.print("<p style=\"font-size:4vw;\">Choose Portion Size: <a href=\"/0\">0</a> <a href=\"/1\">1</a> <a href=\"/2\">2</a> <a href=\"/3\">3</a><br></p>");
            // client.print("<p style=\"font-size:3vw;\">Current Portion: " + String(numShotsServer) + " shots</p>");
            // client.print("<p style=\"font-size:4vw;\">Click to <a href=\"/A\">Dispense " + String(numShotsServer) + " of A</a><br></p>");
            // client.print("<p style=\"font-size:4vw;\">Click to <a href=\"/B\">Dispense " + String(numShotsServer) + " of B</a><br></p>");
            // client.print("<p style=\"font-size:4vw;\">Click to <a href=\"/C\">Dispense " + String(numShotsServer) + " of C</a><br></p>");
            // client.print("<p style=\"font-size:3vw;\">Current Drink: " + String(customA) + " shots of A, " + String(customB) + " shots of B, " + String(customC) + " shots of C" "</p>");//" + String(customA) + " B: " + String(customB) + " C: " + String(customC)
            // client.println("<p style=\"font-size:5vw;\"><a href=\"/P\">Click to Pour Your Drink!</a><br></p>");
            // client.print("<p style=\"font-size:7vw;\">Master Controls:</p>");
            // client.print("<p style=\"font-size:5vw;\"><a href=\"/R\">RESET ALL CARDS</a><br></p>");
            // client.print("<p style=\"font-size:5vw;\"><a href=\"/S\">EMERGENCY STOP</a><br></p>");
            // client.println();

            //Test code is above^^^

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        //TODO: add a physical emergency shut off button
        //TODO: implement selection of drink size and have the current size displayed on the website
        //TODO: Test all functionality
        //TODO: Implement Serial communication to display QR Code

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /0")) {
          numShotsServer = 0;
        }
        if (currentLine.endsWith("GET /1")) {
          numShotsServer = 1;
        }
        if (currentLine.endsWith("GET /2")) {
          numShotsServer = 2;
        }
        if (currentLine.endsWith("GET /3")) {
          numShotsServer = 3;
        }
        if (currentLine.endsWith("GET /A") && rotateA == 0 && rotateB == 0 && rotateC == 0) {
          // confirm();
          customA = numShotsServer;
          // rotateB = 0;
          // rotateC = 0;
        }
        if (currentLine.endsWith("GET /B") && rotateA == 0 && rotateB == 0 && rotateC == 0) {
          // confirm();
          // rotateA = 0;
          customB = numShotsServer;
          // rotateC = 0;
        }
        if (currentLine.endsWith("GET /C") && rotateA == 0 && rotateB == 0 && rotateC == 0) {
          // confirm();
          // rotateA = 0;
          // rotateB = 0;
          customC = numShotsServer;
        }
        if (currentLine.endsWith("GET /P") && rotateA == 0 && rotateB == 0 && rotateC == 0) {
          confirm();
          rotateA = customA * shotSize;
          rotateB = customB * shotSize;
          rotateC = customC * shotSize;
          customA = 0;
          customB = 0;
          customC = 0;
        }
        if (currentLine.endsWith("GET /R")) {
          confirm();
          delay(75);
          confirm();
          memset(scannedUIDs, 0, sizeof(scannedUIDs));
          int numScanned = 0;
        }
        if (currentLine.endsWith("GET /S")) {
          rotateA = 0;
          rotateB = 0;
          rotateC = 0;
          checkHole();
          errorBlink();
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }

  if (rotateC > 0) {
    Serial.println(rotateC);
  }

  unsigned long currentMillis = millis();  //Grab current time for comparison (bc prox sensor needs delay between fires and I dont want to slow down the program w/delay())

  if (currentMillis - previousMillis >= PING_INTERVAL) {  // Check if the ping interval has passed
    previousMillis = currentMillis;                       // Save the last ping time

    unsigned int distance = sonar.ping_cm();  // Get distance from sensor in cm

    if (distance >= 4 && distance <= 7) {  // Check if distance is within the desired range
      digitalWrite(BLUE_LED_PIN, HIGH);    // Turn on the LED
      cupPresent = true;
    } else {
      digitalWrite(BLUE_LED_PIN, LOW);  // Turn off the LED
      cupPresent = false;
    }
  }

  if (cupPresent) {  //Refix code so master card can work even if bottle is not there
    if (rotateA == 0 && rotateB == 0 && rotateC == 0) {
      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        byte uid[] = { mfrc522.uid.uidByte[0], mfrc522.uid.uidByte[1], mfrc522.uid.uidByte[2], mfrc522.uid.uidByte[3] };

        if (isUIDScanned(uid)) {
          // Serial.println("Card has already been scanned!");
          errorBlink();
        } else {
          storeUID(uid);

          // Check against known UIDs and print recognized message
          if (uidMatch(uid, 0xFE, 0xA3, 0x94, 0x17)) {
            rotateA = shotSize;
            rotateB = shotSize;
            rotateC = shotSize;
            // Serial.println("Card Recognized! Card UID is FE A3 94 17");
            confirm();
          } else if (uidMatch(uid, 0x59, 0x28, 0x0A, 0xE5)) {
            // Serial.println("Card Recognized! Card UID is 59 28 0A E5");
            rotateA = shotSize * 2;
            rotateB = shotSize * 2;
            rotateC = 0;
            confirm();
          } else if (uidMatch(uid, 0x79, 0xDA, 0x7B, 0xE9)) {
            // Serial.println("Card Recognized! Card UID is 79 DA 7B E9");
            rotateA = shotSize * 3;
            rotateB = 0;
            rotateC = 0;
            confirm();
          } else if (uidMatch(uid, 0x64, 0xA4, 0x94, 0x64)) {
            // Serial.println("Card Recognized! Card UID is 64 A4 94 64");
            rotateA = shotSize * 2;
            rotateB = shotSize;
            rotateC = shotSize * 2;
            confirm();
          } else if (uidMatch(uid, 0xF3, 0x75, 0x0D, 0x1E)) {
            // Serial.println("Card Recognized! Card UID is F3 75 0D 1E");
            rotateA = 0;
            rotateB = 0;
            rotateC = shotSize * 3;
            confirm();
          } else if (uidMatch(uid, 0x99, 0xE2, 0xFD, 0xE4)) {
            // Serial.println("Card Recognized! Card UID is 99 E2 FD E4");
            rotateA = 0;
            rotateB = shotSize * 3;
            rotateC = 0;
            confirm();
          } else if (uidMatch(uid, 0xF9, 0xB9, 0x07, 0xE3)) {
            confirm();
            delay(75);
            confirm();
            memset(scannedUIDs, 0, sizeof(scannedUIDs));
            int numScanned = 0;
          } else {
            // Serial.println("Card Error! Card not known!");
            noID();
            removeUID(findUID(uid));
          }
        }

        mfrc522.PICC_HaltA();  // Halt PICC
      }
    } else {

      int aOptical = analogRead(A0);
      int bOptical = analogRead(A1);
      int cOptical = analogRead(A2);


      aOptical = (aOptical >= 300) ? 200 : 0;
      bOptical = (bOptical >= 300) ? 200 : 0;
      cOptical = (cOptical >= 300) ? 200 : 0;

      //Serial.println(cOptical);

      // Serial.println(cOptical);

      if (rotateA > 0) {
        digitalWrite(aPos, LOW);
        if (aOptical > 100 && aHigh == false) {
          aHigh = true;
        } else if (aOptical <= 100 && aHigh == true) {
          aHigh = false;
          rotateA -= 1;
        }
      } else {
        digitalWrite(aPos, HIGH);
      }

      if (rotateB > 0) {
        digitalWrite(bPos, LOW);
        if (bOptical > 100 && bHigh == false) {
          bHigh = true;
        } else if (bOptical <= 100 && bHigh == true) {
          bHigh = false;
          rotateB -= 1;
        }
      } else {
        digitalWrite(bPos, HIGH);
      }

      if (rotateC > 0) {
        digitalWrite(cPos, LOW);
        if (cOptical > 100 && cHigh == false) {
          cHigh = true;
        } else if (cOptical <= 100 && cHigh == true) {
          cHigh = false;
          rotateC -= 1;
        }
      } else {
        digitalWrite(cPos, HIGH);
      }
      //Patch fix, checks for holes in code to ensure we stop dispensing when we want to
      checkHole();
    }
  } else {

    unsigned long currentMil = millis();
    if ((rotateA > 0 || rotateB > 0 || rotateC > 0) && currentMil - prevMil > 750) {
      prevMil = currentMil;          // Save the current time
      blueledState = !blueledState;  // Toggle the state of the LED
      digitalWrite(BLUE_LED_PIN, blueledState);
    }

    digitalWrite(aPos, HIGH);
    digitalWrite(bPos, HIGH);
    digitalWrite(cPos, HIGH);
  }

  if (digitalRead(A5) == LOW) {
    rotateA = 0;
    rotateB = 0;
    rotateC = 0;

    digitalWrite(aPos, HIGH);
    digitalWrite(bPos, HIGH);
    digitalWrite(cPos, HIGH);
    Serial.println("EMERGENCY STOP ACTIVATED");
    noID();
  }
}


///// Below are all support functions for this code to work /////
void checkHole() {
  if (rotateA <= 0) {
    digitalWrite(aPos, HIGH);
  }

  if (rotateB <= 0) {
    digitalWrite(bPos, HIGH);
  }

  if (rotateC <= 0) {
    digitalWrite(cPos, HIGH);
  }
}

void confirm() {
  digitalWrite(GREEN_LED_PIN, HIGH);
  delay(125);
  digitalWrite(GREEN_LED_PIN, LOW);
  delay(75);
  digitalWrite(GREEN_LED_PIN, HIGH);
  delay(125);
  digitalWrite(GREEN_LED_PIN, LOW);
}

void noID() {
  digitalWrite(RED_LED_PIN, HIGH);
  delay(750);
  digitalWrite(RED_LED_PIN, LOW);
}

void errorBlink() {
  digitalWrite(RED_LED_PIN, HIGH);
  delay(125);
  digitalWrite(RED_LED_PIN, LOW);
  delay(75);
  digitalWrite(RED_LED_PIN, HIGH);
  delay(125);
  digitalWrite(RED_LED_PIN, LOW);
}

void confirmWifi() {
  digitalWrite(BLUE_LED_PIN, HIGH);
  delay(250);
  digitalWrite(GREEN_LED_PIN, HIGH);
  delay(250);
  digitalWrite(RED_LED_PIN, HIGH);
  delay(100);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);
  delay(125);
  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(BLUE_LED_PIN, HIGH);
  delay(200);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);
}

bool isUIDScanned(byte *uid) {
  for (int i = 0; i < numScanned; i++) {
    if (memcmp(uid, scannedUIDs[i], 4) == 0) {
      return true;  // UID has been scanned before
    }
  }
  return false;
}

void storeUID(byte *uid) {
  if (numScanned < 10) {  // Prevent overflow of the array
    memcpy(scannedUIDs[numScanned], uid, 4);
    numScanned++;
  }
}

int findUID(byte *uid) {
  for (int i = 0; i < numScanned; i++) {
    if (memcmp(uid, scannedUIDs[i], 4) == 0) {
      return i;  // Return index of the UID in the array
    }
  }
  return -1;  // UID not found
}

void removeUID(int index) {
  if (index >= 0 && index < numScanned) {
    for (int i = index; i < numScanned - 1; i++) {
      memcpy(scannedUIDs[i], scannedUIDs[i + 1], 4);  // Shift UIDs down
    }
    numScanned--;  // Decrease count of stored UIDs
  }
}

bool uidMatch(byte *uid, byte b0, byte b1, byte b2, byte b3) {
  return uid[0] == b0 && uid[1] == b1 && uid[2] == b2 && uid[3] == b3;
}

//Used for basic debugging of web server setup, taken from SimpleWebServerWiFi Example Sketch
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  // Serial.print("http://");
  // Serial.println(ip);
  String ipStr = "http://";
  ipStr += String(ip);

  Serial1.println(ipStr);
  
  while(Serial1.available() == 0) {
    delay(100);
  }
  String response = Serial1.readString();
  Serial.println(response);

  //Code to send to Serial1 goes here
  
  Serial1.print("http://");
  Serial1.println(ip);

  confirmWifi();
}
