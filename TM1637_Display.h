// #include "RES.h"
uint8_t m_pinClk;
uint8_t m_pinDIO;
uint8_t m_brightness;
unsigned int m_bitDelay;
#define SEG_A 0b00000001
#define SEG_B 0b00000010
#define SEG_C 0b00000100
#define SEG_D 0b00001000
#define SEG_E 0b00010000
#define SEG_F 0b00100000
#define SEG_G 0b01000000
#define SEG_DP 0b10000000

#define TM1637_I2C_COMM1 0x40
#define TM1637_I2C_COMM2 0xC0
#define TM1637_I2C_COMM3 0x80

//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D
const uint8_t digitToSegment[] = {
  // XGFEDCBA
  0b00111111,  // 0
  0b00000110,  // 1
  0b01011011,  // 2
  0b01001111,  // 3
  0b01100110,  // 4
  0b01101101,  // 5
  0b01111101,  // 6
  0b00000111,  // 7
  0b01111111,  // 8
  0b01101111,  // 9
  0b01110111,  // A
  0b01111100,  // b
  0b00111001,  // C
  0b01011110,  // d
  0b01111001,  // E
  0b01110001   // F
};

static const uint8_t minusSegments = 0b01000000;

#define DEFAULT_BIT_DELAY 100

class TM1637Display {

public:

  //! Initialize a TM1637Display object, setting the clock and
  TM1637Display(uint8_t pinClk, uint8_t pinDIO, unsigned int bitDelay = DEFAULT_BIT_DELAY) {
    // Copy the pin numbers
    m_pinClk = pinClk;
    m_pinDIO = pinDIO;
    m_bitDelay = bitDelay;

    // Set the pin direction and default value.
    // Both pins are set as inputs, allowing the pull-up resistors to pull them up
    pinMode(m_pinClk, INPUT);
    pinMode(m_pinDIO, INPUT);
    digitalWrite(m_pinClk, LOW);
    digitalWrite(m_pinDIO, LOW);
  }

  //! Sets the brightness of the display.
  void setBrightness(uint8_t brightness, bool on = true) {
    m_brightness = (brightness & 0x7) | (on ? 0x08 : 0x00);
  }

  //! Display arbitrary data on the module
  void setSegments(const uint8_t segments[], uint8_t length = 4, uint8_t pos = 0) {
    // Write COMM1
    start();
    writeByte(TM1637_I2C_COMM1);
    stop();

    // Write COMM2 + first digit address
    start();
    writeByte(TM1637_I2C_COMM2 + (pos & 0x03));

    // Write the data bytes
    for (uint8_t k = 0; k < length; k++)
      writeByte(segments[k]);

    stop();

    // Write COMM3 + brightness
    start();
    writeByte(TM1637_I2C_COMM3 + (m_brightness & 0x0f));
    stop();
  }

  //! Clear the display
  void clear() {
    uint8_t data[] = { 0, 0, 0, 0 };
    setSegments(data);
  }

  //! Translate a single digit into 7 segment code
  static uint8_t encodeDigit(uint8_t digit) {
    return digitToSegment[digit & 0x0f];
  }

  void bitDelay() {
    delayMicroseconds(m_bitDelay);
  }

  void start() {
    pinMode(m_pinDIO, OUTPUT);
    bitDelay();
  }

  void stop() {
    pinMode(m_pinDIO, OUTPUT);
    bitDelay();
    pinMode(m_pinClk, INPUT);
    bitDelay();
    pinMode(m_pinDIO, INPUT);
    bitDelay();
  }

  bool writeByte(uint8_t b) {
    uint8_t data = b;

    // 8 Data Bits
    for (uint8_t i = 0; i < 8; i++) {
      // CLK low
      pinMode(m_pinClk, OUTPUT);
      bitDelay();

      // Set data bit
      if (data & 0x01)
        pinMode(m_pinDIO, INPUT);
      else
        pinMode(m_pinDIO, OUTPUT);

      bitDelay();

      // CLK high
      pinMode(m_pinClk, INPUT);
      bitDelay();
      data = data >> 1;
    }

    // Wait for acknowledge
    // CLK to zero
    pinMode(m_pinClk, OUTPUT);
    pinMode(m_pinDIO, INPUT);
    bitDelay();

    // CLK to high
    pinMode(m_pinClk, INPUT);
    bitDelay();
    uint8_t ack = digitalRead(m_pinDIO);
    if (ack == 0)
      pinMode(m_pinDIO, OUTPUT);


    bitDelay();
    pinMode(m_pinClk, OUTPUT);
    bitDelay();

    return ack;
  }
};
