#include <SPI.h>
#include <Adafruit_PN532.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>


WebServer server(80);
// Define SPI pins
#define SCK (18)   // Clock pin
#define MISO (19)  // Master-In-Slave-Out
#define MOSI (23)  // Master-Out-Slave-In
#define SS (16)    // Slave-Select (CS/Chip Select)

// Initialize Adafruit PN532 using SPI
Adafruit_PN532 nfc(SS);

#define SELECT_RESPONSE_OK "900"
bool completed = false;
String ssid = " ";
String password = " ";
// std::vector<String> split(String s, String delimiter) {
//   int pos_start = 0, pos_end;
//   int delim_len = delimiter.length();
//   String token;
//   std::vector<String> res;

//   while ((pos_end = s.indexOf(delimiter, pos_start)) != -1) {
//     token = s.substring(pos_start, pos_end);
//     pos_start = pos_end + delim_len;
//     res.push_back(token);
//   }

//   res.push_back(s.substring(pos_start));
//   return res;
// }
std::vector<String> split(String s, char delimiter) {
  int str_len = s.length();
  std::vector<String> return_vec;
  for (int i = 0; i < str_len; i++) {
    int start_sub = i;
    int end_sub = i;
    while (s[i] != delimiter && i < str_len) {
      i++;
      end_sub = i;
    }
    if (i >= str_len) {
      continue;
    }
    if (start_sub != end_sub) {
      return_vec.push_back(s.substring(start_sub, end_sub));
    }
  }
  return return_vec;
}


void setup(void) {
  Serial.begin(9600);
  setupNFC();
  WiFi.mode(WIFI_STA); 
}

void loop() {
  if (nfc.inListPassiveTarget() && completed == false) {
    Serial.println("Found something!");

    // Step 1: Send the SELECT AID command
    uint8_t selectApdu[] = {
      0x00, 0xA4, 0x04, 0x00, 0x06,        // SELECT APDU header
      0xF2, 0x23, 0x34, 0x45, 0x56, 0x67,  // AID
      0x00                                 // Le
    };
    uint8_t response[32];
    uint8_t responseLength = sizeof(response);

    if (nfc.inDataExchange(selectApdu, sizeof(selectApdu), response, &responseLength)) {
      String str = "";
      String statusCode = "";


      // Step 2: Process the first chunk
      processResponse(response, responseLength, statusCode, str);

      // Step 3: Fetch remaining chunks
      while (statusCode != SELECT_RESPONSE_OK) {
        uint8_t nextChunkApdu[] = { 0x02 };  // GET_NEXT_CHUNK_COMMAND
        responseLength = sizeof(response);

        if (nfc.inDataExchange(nextChunkApdu, sizeof(nextChunkApdu), response, &responseLength)) {
          processResponse(response, responseLength, statusCode, str);
        } else {
          Serial.println("Failed fetching next chunk");
          break;
        }
      }

      Serial.println("Complete string: " + str);
      completed = true;
      char delimiter = '$';
      std::vector<String> v = split(str, delimiter);
      String ssid = v[0];      // First part assigned to ssid
      String password = v[1];  // Second part assigned to password

      Serial.println(ssid);
      Serial.println(password);
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Attempting to connect to Wi-Fi...");
        WiFi.begin(ssid.c_str(), password.c_str());

        unsigned long startAttemptTime = millis();

        // Wait for connection
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
          delay(500);
          Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("\nConnected to Wi-Fi!");
          Serial.println("IP Address: " + WiFi.localIP().toString());
        } else {
          Serial.println("\nFailed to connect. Retrying...");
        }
      }

      delay(1000);

      // for (const String &part : v) {
      //   Serial.println(part);
      //   Serial.println(ssid);

      // }
    } else {
      Serial.println("Failed sending SELECT AID");
    }
  } else {
    // Serial.println("Didn't find anything!");
  }

  delay(1000);
}

// Process response to append chunks
void processResponse(uint8_t *response, uint8_t responseLength, String &statusCode, String &email) {
  statusCode = "";  // Extract status code from first two bytes
  statusCode += String(response[0], HEX);
  statusCode += String(response[1], HEX);

  for (int i = 0; i < responseLength; i++) {
    email += (char)response[i];  // Append payload to email
  }

  // Serial.print("Status Code: ");
  // Serial.println(statusCode);
  // Serial.print("Email Chunk: ");
  // Serial.println(email);
}

void setupNFC() {
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();

  if (!versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1)
      ;  // halt
  }
  delay(2000);

  // Got ok data, print it out!
  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  // Configure board to read RFID tags
  nfc.SAMConfig();
}
