#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <qrcode.h>

#define SHARP_SCK 13   // CLK
#define SHARP_MOSI 11  // MOSI
#define SHARP_SS 10    // CS

Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 144, 168, 2000000);

#define BLACK 0
#define WHITE 1

int xOffset = 14;
int yOffset = 48;
bool qrGenerated = false;

String receivedData;
QRCode qrcode;

void setup() {
  Serial1.begin(9600);  // Initialize Serial1 with a baud rate of 9600
  Serial.begin(9600);

  pinMode(SHARP_SCK, OUTPUT);
  pinMode(SHARP_MOSI, OUTPUT);
  pinMode(SHARP_SS, OUTPUT);

  display.begin();
  display.clearDisplay();
}

void loop() {
  if (Serial1.available() > 0) {
    receivedData = Serial1.readString();  // Read the data from Arduino A
    // Serial.println(receivedData);         // Output the received data

    // Send acknowledgment back to Arduino A
    Serial1.println("ACK");

    // Ensure qrGenerated is reset for new data
    qrGenerated = false;
  }
  if (!qrGenerated && receivedData.length() > 0) {
    updateDisplay();
    qrGenerated = true;  // Prevent further unnecessary updates
  }
}

void updateDisplay() {
  display.clearDisplay();  // Clear the display for fresh update
  display.setTextColor(BLACK);
  display.setTextSize(2);
  display.setCursor(7, 6);
  display.println("Scan Link &");
  display.setCursor(7, 24);
  display.println("Pour Drinks");
  display.setTextSize(1);
  display.setCursor(5, 50);

  uint8_t qrcodeData[qrcode_getBufferSize(4)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, receivedData.c_str());  // Initialize QR code

  int scale = 4;  // Scaling factor for the QR code
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      int color = qrcode_getModule(&qrcode, x, y) ? BLACK : WHITE;
      for (int dy = 0; dy < scale; dy++) {
        for (int dx = 0; dx < scale; dx++) {
          display.drawPixel(scale * x + dx + xOffset, scale * y + dy + yOffset, color);
        }
      }
    }
  }
  display.refresh();  // Refresh the display to show the QR code
}

//handshake no data
// #include <Adafruit_GFX.h>
// #include <Adafruit_SharpMem.h>
// #include <qrcode.h>

// #define SHARP_SCK 13   // CLK
// #define SHARP_MOSI 11  // MOSI
// #define SHARP_SS 10    // CS

// Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 144, 168, 2000000);

// #define BLACK 0
// #define WHITE 1

// int xOffset = 14;
// int yOffset = 48;
// bool qrGenerated = false;

// String receivedData;
// QRCode qrcode;

// void setup() {
//   Serial1.begin(9600);  // Initialize Serial1 with a baud rate of 9600
//   Serial.begin(9600);

//   pinMode(SHARP_SCK, OUTPUT);
//   pinMode(SHARP_MOSI, OUTPUT);
//   pinMode(SHARP_SS, OUTPUT);

//   display.begin();
//   display.clearDisplay();
// }

// void loop() {
//   if (Serial1.available() > 0) {
//     receivedData = Serial1.readString();  // Read the data from Arduino A
//     Serial.println(receivedData);         // Output the received data

//     // Send acknowledgment back to Arduino A
//     Serial1.println("ACK");
//     delay(200);
//   }
//   if (!qrGenerated) {
//     updateDisplay();
//   }

//   // Optionally, add a delay or additional functionality here
// }

// void updateDisplay() {
//   display.clearDisplay();  // Clear the display for fresh update
//   display.setTextColor(BLACK);
//   display.setTextSize(2);
//   display.setCursor(7, 6);
//   display.println(receivedData);
//   // display.println("Scan Link &");
//   // display.setCursor(7, 24);
//   // display.println("Pour Drinks");
//   // display.setTextSize(1);
//   display.setCursor(5, 50);

//   uint8_t qrcodeData[qrcode_getBufferSize(4)];
//   qrcode_initText(&qrcode, qrcodeData, 3, 0, receivedData.c_str());

//   int scale = 4;  // Scaling factor for the QR code
//   for (uint8_t y = 0; y < qrcode.size; y++) {
//     for (uint8_t x = 0; x < qrcode.size; x++) {
//       int color = qrcode_getModule(&qrcode, x, y) ? BLACK : WHITE;
//       for (int dy = 0; dy < scale; dy++) {
//         for (int dx = 0; dx < scale; dx++) {
//           display.drawPixel(scale * x + dx + xOffset, scale * y + dy + yOffset, color);
//         }
//       }
//     }
//   }
//   display.refresh();
//   qrGenerated = true;  // Set the flag to true after generating QR code
// }

//olest
// #include <Adafruit_GFX.h>
// #include <Adafruit_SharpMem.h>
// #include <qrcode.h>

// #define SHARP_SCK 13   // CLK
// #define SHARP_MOSI 11  // MOSI
// #define SHARP_SS 10    // CS

// #define IP_ADDRESS "http://100.70.15.115"

// Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 144, 168, 2000000);

// #define BLACK 0
// #define WHITE 1

// int actualLength = 0;
// int xOffset = 14;
// int yOffset = 48;
// bool qrGenerated = false;
// bool foundIP = false;

// String receivedData;

// QRCode qrcode;

// void setup() {
//   Serial1.begin(9600);  // Initialize Serial1 with a baud rate of 9600
//   Serial.begin(9600);

//   pinMode(6, OUTPUT);
//   digitalWrite(6, LOW);

//   pinMode(SHARP_SCK, OUTPUT);
//   pinMode(SHARP_MOSI, OUTPUT);
//   pinMode(SHARP_SS, OUTPUT);

//   display.begin();
//   display.clearDisplay();
// }

// void loop() {
//   if (Serial1.available() > 0) {
//     receivedData = Serial1.readString();  // Read the data from Arduino A

//     // Process the received data (if necessary)

//     Serial.println(receivedData);

//     digitalWrite(6, HIGH);
//     delay(500);
//     digitalWrite(6, LOW);

//     foundIP = true;

//     Serial1.println("ACK");  // Send acknowledgement back to Arduino A
//   }
//   if (foundIP && !qrGenerated) {
//     updateDisplay();
//     delay(5000);
//   }

//   // display.refresh();
// }

// void updateDisplay() {
//   display.clearDisplay();  // Clear the display for fresh update

//   display.setTextColor(BLACK);
//   display.setTextSize(2);
//   display.setCursor(7, 6);
//   // display.println(receivedData);
//   display.println("Scan Link &");
//   display.setCursor(7, 24);
//   display.println("Pour Drinks");

//   display.setTextSize(1);
//   display.setCursor(5, 50);  // Adjusted for positioning

//   uint8_t qrcodeData[qrcode_getBufferSize(4)];
//   // Convert String to const char* using c_str() before passing to the function
//   qrcode_initText(&qrcode, qrcodeData, 3, 0, receivedData.c_str());

//   int scale = 4;  // Scaling factor for the QR code
//   for (uint8_t y = 0; y < qrcode.size; y++) {
//     for (uint8_t x = 0; x < qrcode.size; x++) {
//       int color = qrcode_getModule(&qrcode, x, y) ? BLACK : WHITE;
//       for (int dy = 0; dy < scale; dy++) {
//         for (int dx = 0; dx < scale; dx++) {
//           display.drawPixel(scale * x + dx + xOffset, scale * y + dy + yOffset, color);
//         }
//       }
//     }
//   }
//   display.refresh();
//   qrGenerated = true;  // Set the flag to true after generating QR code
// }
