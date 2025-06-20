#include <Arduino.h>
#include <TimerService.h>

// XMODEM CRC16 with poly 0x1021 and initial value 0x0000
uint16_t crcXmodem(const uint8_t* data, size_t length) {
  uint16_t crc = 0x0000;
  for (size_t i = 0; i < length; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }

  // Print CRC in HEX format (e.g., 0xB7A9)
  Serial.print(printLocalTime() + " CRC16-XMODEM = ");
  Serial.print(String((const char*)data) + " - "); // MSB
  if (crc < 0x1000) Serial.print("0");
  if (crc < 0x100)  Serial.print("0");
  if (crc < 0x10)   Serial.print("0");
  Serial.println(crc, HEX);

  return crc;
}

bool isValidStringMessageWithCRC(const String* message) {
  if (message == nullptr) return false;

  size_t totalLength = message->length();
  if (totalLength < 4) return false;  // Must include at least 1 byte + 2 CRC + \r

  // Check if last character is \r
  if (message->charAt(totalLength - 1) != '\r') return false;

  // Prepare buffer
  uint8_t buffer[256];
  if (totalLength > sizeof(buffer)) return false;  // Avoid overflow

  message->getBytes(buffer, totalLength + 1); // +1 for null terminator

  // Extract CRC
  uint16_t receivedCRC = ((uint16_t)buffer[totalLength - 3] << 8) | buffer[totalLength - 2];

  // Calculate CRC on everything before the 2 CRC bytes and \r
  size_t dataLength = totalLength - 3;
  uint16_t computedCRC = crcXmodem(buffer, dataLength);

  // Compare received CRC with computed CRC
  Serial.print(printLocalTime() + " Received CRC: ");
  Serial.println(receivedCRC, HEX);
  Serial.print(printLocalTime() + " Computed CRC: ");
  Serial.println(computedCRC, HEX);

  return receivedCRC == computedCRC;
}


// Validate a message like: [data][2 CRC bytes][0x0D]
bool isValidMessageWithCRC(const uint8_t* message, size_t totalLength) {
  if (totalLength < 4) return false;

  uint8_t endChar = message[totalLength - 1];
  if (endChar != 0x0D) return false;

  uint16_t receivedCRC = ((uint16_t)message[totalLength - 3] << 8) | message[totalLength - 2];
  size_t dataLength = totalLength - 3; // Exclude CRC + \r

  uint16_t computedCRC = crcXmodem(message, dataLength);
  return receivedCRC == computedCRC;
}
