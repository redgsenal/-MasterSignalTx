 #include <SPI.h>
#include <RH_RF95.h>
#include <elapsedMillis.h>

/* for feather m0 */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
#define RF95_FREQ 433.0
// D6
#define CALL_BUTTON 6
// D10
#define LED_INDICATOR 10
// D13 - ON LED
#define LED_POWER 13

#define STATE_IDLE 0xA0
#define STATE_BUTTON_PRESSED_SEND_REQUEST 0xA1
#define STATE_RESPONSE_RECEIVED 0xA2
#define STATE_TIMER_COUNT 0xA3

#define BUTTON_PRESSED 0x01
#define REPLY_RESPONSE 0x03
#define VALID_RESPONSE 0x04
#define ELAPSED_WAIT_TIME 0x05
#define WAIT_TIME 20000 // 20 seconds

#define TX_POWER_LEVEL 10

const char RESPONSE_TOKEN[20] = "1234567890123456789";
const char REQUEST_TOKEN[20] = "ABCDEFGH1JKLMNOPQRS";
unsigned long currentState = STATE_IDLE;

int callButtonState = 0;
char* requestRxToken;
RH_RF95 rf95(RFM95_CS, RFM95_INT);

elapsedMillis waitingTimeElapsed;

void setup() {
  pinMode(LED_POWER, OUTPUT);  // declare LED as output
  digitalWrite(LED_POWER, HIGH);
  pinMode(LED_INDICATOR, OUTPUT);  // declare LED as output
  pinMode(CALL_BUTTON, INPUT);    // declare pushbutton as input
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  // while (!Serial); uncomment when debugging
  Serial.begin(9600);
  delay(100);
  out("Feather LoRa TX Test!");
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  while (!rf95.init()) {
    out("LoRa radio init failed");
    while (1);
  }
  out("LoRa radio init OK!");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    out("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); out(RF95_FREQ);
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(TX_POWER_LEVEL, false);
}
int16_t packetnum = 0;  // packet counter, we increment per xmission

void out(String v){
  Serial.println(v);
}

void loop() {
  // loop here on button states
  checkButton();
  while(currentState == STATE_IDLE){
    out("waiting button");
    callButtonState = checkButton();
    if (callButtonState == HIGH){
      out("button pressed");
      currentState = STATE_BUTTON_PRESSED_SEND_REQUEST;
    }    
  }

  while(currentState == STATE_BUTTON_PRESSED_SEND_REQUEST){
    out("button pressed; send signal");
    blinkState();
    sendRequest();
  }

  while(currentState == STATE_RESPONSE_RECEIVED){    
    blinkState();
    if (waitingTimeElapsed > WAIT_TIME){
      // waiting time; reset to idle state
      waitingTimeElapsed = 0;       // reset the counter to 0 so the counting starts over...
      digitalWrite(LED_POWER, HIGH);
      currentState = STATE_IDLE;
    }   
  }
}

void blinkState(){
  int ledState = digitalRead(LED_POWER);  // read input value
  ledState = (ledState == HIGH) ? LOW : HIGH;
  digitalWrite(LED_POWER, ledState);
  delay(500);  
}

int checkButton(){
  int ledState = digitalRead(CALL_BUTTON);  // read input value
  digitalWrite(LED_INDICATOR, ledState);  // turn LED OFF
  return ledState;
}

void sendRequest() {
  out("Sending to rf95_server");
  // Send a message to rf95_server
  
  char radiopacket[] = "ABCDEFGH1JKLMNOPQRS"; //"Hello World #      ";
  //itoa(packetnum++, radiopacket+13, 10);
  Serial.print("Sending "); out(radiopacket);
  radiopacket[19] = 0;
  
  out("Sending..."); delay(10);
  rf95.send((uint8_t *)radiopacket, 20);

  out("Waiting for packet to complete..."); delay(10);
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  out("Waiting for reply..."); delay(10);
  if (rf95.waitAvailableTimeout(1000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len)) {
      Serial.print("Got reply: ");
      requestRxToken = (char*)buf;
      out(requestRxToken);
      Serial.print("RSSI: ");
      out(rf95.lastRssi(), DEC);
      if (strcmp(requestRxToken, RESPONSE_TOKEN) == 0){  
        currentState = STATE_RESPONSE_RECEIVED;
        waitingTimeElapsed = 0;
        out("valid token");
      }else{
        out("invalid token");
      }
    } else {
      out("Receive failed");
    }
  } else {
    out("No reply, is there a listener around?");
  }  
}

