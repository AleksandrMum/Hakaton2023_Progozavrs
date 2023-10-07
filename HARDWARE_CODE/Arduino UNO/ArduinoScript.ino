#include <RCSwitch.h>
#include <DHT.h>

using namespace std;

//max 111111 = 63
#define HUMIDITY_DEVICE_ID 27 
#define TEMPERATURE_DEVICE_ID 26 

#define TRANSMIT_PIN 10

#define DHT_PIN 2
#define DHT_TYPE DHT11

#define FIELD 28841
#define PRIVATE_KEY 22487
#define PUBLIC_KEY 14423

RCSwitch mySwitch = RCSwitch();

DHT dht(DHT_PIN, DHT_TYPE);

struct DeviceHashes {
  byte humidityHash = 0;
  byte temperatureHash = 1;
} deviceHashes;

unsigned long power(unsigned long int base, unsigned int expo, unsigned int mod) {
  unsigned long test;
  for (test = 1; expo; expo >>= 1) {
    if (expo & 1)
      test = (test * base) % mod;
    base = (base * base) % mod;
  }
  return test;
}

unsigned char lowbias32(unsigned long int x) {
  x ^= x >> 16;
  x *= 0x7feb352dU;
  x ^= x >> 15;
  x *= 0x846ca68bU;
  x ^= x >> 16;
  return (unsigned char)x;
}

unsigned long int data_construct(byte messageNumber, byte deviceID, unsigned int data) {
  unsigned long construct = 0x0;

  byte header = 0;
  header |= messageNumber;
  header <<= 6;
  header |= deviceID;

  construct |= header;
  construct <<= 16;
  construct |= data;
  construct <<= 8;
  return construct;
}

void send_float(byte deviceID, float *data, byte *deviceHash) {
  unsigned int firstPart = 0, secondPart = 0;
  byte *floatBytePtr = (byte *)(void *)data;
  
  firstPart |= floatBytePtr[0];
  firstPart <<= 8;
  firstPart |= floatBytePtr[1];

  secondPart |= floatBytePtr[2];
  secondPart <<= 8;
  secondPart |= floatBytePtr[3];

  unsigned long firstDataConstruct = data_construct(0, deviceID, firstPart);
  unsigned long secondDataConstruct = data_construct(1, deviceID, secondPart);

  firstDataConstruct |= *deviceHash;
  *deviceHash = lowbias32(firstDataConstruct);
  secondDataConstruct |= *deviceHash;
  *deviceHash = lowbias32(secondDataConstruct);

  mySwitch.send(firstDataConstruct, 32);
  Serial.print("First dataconstruct sended: ");
  Serial.println(firstDataConstruct, HEX);

  mySwitch.send(secondDataConstruct, 32);
  Serial.print("Second dataconstruct sended: ");
  Serial.println(secondDataConstruct, HEX);
}

void setup() {
  Serial.begin(9600);
  mySwitch.enableTransmit(TRANSMIT_PIN);
  mySwitch.setRepeatTransmit(4);
  dht.begin();
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  Serial.println(temperature);
  Serial.println(humidity);
  if (!isnan(temperature)) send_float(TEMPERATURE_DEVICE_ID, &temperature, &deviceHashes.temperatureHash);
  if (!isnan(humidity)) send_float(HUMIDITY_DEVICE_ID, &humidity, &deviceHashes.humidityHash);

  delay(5000);
}
