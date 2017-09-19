
#include <SPI.h>
#include <RH_RF95.h>
/* for feather m0 */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
#define RF95_FREQ 434.0
// D6
#define CALL_BUTTON 6
// D10
#define LED_INDICATOR 10
int callButtonState = 0;
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
  pinMode(LED_INDICATOR, OUTPUT);  // declare LED as output
  pinMode(CALL_BUTTON, INPUT);    // declare pushbutton as input
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  while (!Serial);
  Serial.begin(9600);
  delay(100);
  Serial.println("Feather LoRa TX Test!");
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}
int16_t packetnum = 0;  // packet counter, we increment per xmission

void loop() {
  callButtonState = checkButton();
  Serial.println("button State ");
  Serial.println(callButtonState);
}

int checkButton(){
  int currentState = digitalRead(CALL_BUTTON);  // read input value
  digitalWrite(LED_INDICATOR, currentState);  // turn LED OFF
  return currentState;
}

