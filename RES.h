// Define modbus parameters and vaiables
#define MODBUS_SLAVE_ID 0x01
#define MODBUS_FUNCTION_READ_HOLDING_REGISTERS 0x03
#define MODBUS_REGISTER_ADDRESS 0x0000
#define MODBUS_REGISTER_COUNT 10
#define MODBUS_BAUD_RATE 9600
#define RS485_TX_PIN 17
#define RS485_RX_PIN 16
#define RS485_DE_RE_PIN 5  // DE and RE pins tied together and connected to GPIO 5
HardwareSerial RS485Serial(1);
uint16_t registers[MODBUS_REGISTER_COUNT];



// float Te
// int Mode = 0;
