#include "RES.h"
#include "TM1637_Display.h"

// Define the pins for TM1637
const int CLK = 19;  // CLK pin to D5 on TM1637
const int DIO = 18;  // DIO pin to D7 on TM1637

// Create a display object
TM1637Display display(CLK, DIO);

/*
index 0 = Thermostat State ( 0 - 1)
1 = Fan Button (0 - 2)
2 = Setting Button( 0-2)
3 = Set Temperature (value divided by 100 to get float )
4 = Not Used
5 = Minutes time (0-60) 
6 = Hour time (0- 23)
7 = Not Used
8 = Live Temperature (value divided by 100 to get float )
9 = Thermostat Output (0-1)
*/

#define DEBUG true
#define MODBUS true
#define CANBUS  false

// #ifndef
// endif



const int Firmware = 0;

uint16_t calculateCRC16(uint8_t *data, uint16_t length) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

void sendModbusRequest(uint8_t slaveId, uint8_t functionCode, uint16_t startAddress, uint16_t registerCount) {
  uint8_t request[8];
  request[0] = slaveId;
  request[1] = functionCode;
  request[2] = startAddress >> 8;
  request[3] = startAddress & 0xFF;
  request[4] = registerCount >> 8;
  request[5] = registerCount & 0xFF;
  uint16_t crc = calculateCRC16(request, 6);
  request[6] = crc & 0xFF;
  request[7] = crc >> 8;
  RS485Serial.write(request, sizeof(request));
}

bool readModbusResponse(uint8_t *response, uint16_t length) {
  uint32_t timeout = millis() + 1500;
  uint16_t index = 0;
  while (millis() < timeout) {
    if (RS485Serial.available()) {
      response[index++] = RS485Serial.read();
      if (index >= length) {
        return true;
      }
    }
  }
  return false;
}

bool readHoldingRegisters(uint8_t slaveId, uint16_t startAddress, uint16_t registerCount, uint16_t *buffer) {
  digitalWrite(RS485_DE_RE_PIN, HIGH);  // Set DE/RE pin high for transmit mode
  sendModbusRequest(slaveId, MODBUS_FUNCTION_READ_HOLDING_REGISTERS, startAddress, registerCount);
  RS485Serial.flush();                 // Ensure all data is sent
  digitalWrite(RS485_DE_RE_PIN, LOW);  // Set DE/RE pin low for receive mod
  uint8_t response[5 + 2 * registerCount];
  delay(250);
  if (readModbusResponse(response, sizeof(response))) {
    uint16_t crc = (response[sizeof(response) - 1] << 8) | response[sizeof(response) - 2];
    if (calculateCRC16(response, sizeof(response) - 2) == crc) {
      for (int i = 0; i < registerCount; i++) {
        buffer[i] = (response[3 + i * 2] << 8) | response[4 + i * 2];
      }
      return true;
    } else {
      Serial.println("CRC error!");
    }
  } else {
    Serial.println("No response or timeout!");
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  RS485Serial.begin(MODBUS_BAUD_RATE, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW);  // Set DE/RE pin low for receive mode
  // Initialize the display
  display.setBrightness(0x0a);  // set the brightness (0x00 to 0x0f)

  // Test display by showing '8888'
  uint8_t segs_7[] = { 0x00, 0x80, 0x00, 0x00 };  // Turn on DP on digit 3
  display.setSegments(segs_7);
  delay(500);
  delay(2000);
}

void loop() {


  if (readHoldingRegisters(MODBUS_SLAVE_ID, MODBUS_REGISTER_ADDRESS, MODBUS_REGISTER_COUNT, registers)) {
    for (int i = 0; i < MODBUS_REGISTER_COUNT; i++) {
      Serial.print("Register ");
      Serial.print(MODBUS_REGISTER_ADDRESS + i);
      Serial.print(": ");
      Serial.println(registers[i]);
    }
  } else {
    Serial.println("Failed to read registers");
  }

  uint8_t segs[] = { 0x1, display.encodeDigit(registers[8] / 100) | 0x80, display.encodeDigit((registers[8] / 10) % 10), 0x01 };
  display.setSegments(segs);
  delay(500);

  uint8_t segs_1[] = { 0x8, display.encodeDigit(registers[8] / 100), display.encodeDigit((registers[8] / 10) % 10), 0x08 };
  display.setSegments(segs_1);
  delay(500);

  uint8_t segs_2[] = { 0x40, display.encodeDigit(registers[8] / 100) | 0x80, display.encodeDigit((registers[8] / 10) % 10), 0x40 };
  display.setSegments(segs_2);
  delay(500);

  // delay(2000);
}
