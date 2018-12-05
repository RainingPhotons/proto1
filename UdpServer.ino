#include <FastLED.h>
#include <UIPEthernet.h>

#define DEBUG_OUTPUT

#define LED_PIN_1   PB3
#define LED_PIN_2   PA15
#define NUM_LEDS    120
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds_1[NUM_LEDS];
CRGB leds_2[NUM_LEDS];
#define UPDATES_PER_SECOND 100

EthernetUDP udp;

#define IPADDRESS 192,168,1

static const uint8_t kBaseAddress = 200;
static const uint8_t kIPMask[4] = {255,255,255,0};
static const uint8_t kDNS[4] = {192,168,1,1};
static const uint8_t kGateway[4] = {192,168,1,1};

uint8_t read_address() {
  pinMode(PB11, INPUT_PULLUP);
  pinMode(PB10, INPUT_PULLUP);
  pinMode(PB1, INPUT_PULLUP);
  pinMode(PB0, INPUT_PULLUP);
  pinMode(PA3, INPUT_PULLUP);
  pinMode(PA2, INPUT_PULLUP);
  pinMode(PA1, INPUT_PULLUP);
  pinMode(PA0, INPUT_PULLUP);

  uint8_t address = 0;
  address |= (!digitalRead(PB11) & 0x1) << 0;
  address |= (!digitalRead(PB10) & 0x1) << 1;
  address |= (!digitalRead(PB1) & 0x1) << 2;
  address |= (!digitalRead(PB0) & 0x1) << 3;
  address |= (!digitalRead(PA3) & 0x1) << 4;
  address |= (!digitalRead(PA2) & 0x1) << 5;
  address |= (!digitalRead(PA1) & 0x1) << 6;
  address |= (!digitalRead(PA0) & 0x1) << 7;

  return address + kBaseAddress;
}

void setup() {
  // Not sure this is necessary
  delay(3000); // power-up safety delay

  // Necessary to use PA15 and PB3 for LED output.
  // Defaults to being used for JTAG
  disableDebugPorts();

  FastLED.addLeds<LED_TYPE, LED_PIN_1, COLOR_ORDER>(leds_1, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED_PIN_2, COLOR_ORDER>(leds_2, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  uint8_t address = read_address();

  uint8_t mac[6] = {0xde,0xad,0xbe,0xef,0x04,address};

  uint8_t ip_address[4] = {IPADDRESS, address};
  Ethernet.begin(mac, ip_address, kDNS, kGateway, kIPMask);

#ifdef DEBUG_OUTPUT
  while (!Serial);
  Serial.begin(115200);
  Serial.println("Starting up");

  Serial.println(Ethernet.localIP());
  Serial.println(Ethernet.subnetMask());
  Serial.println(Ethernet.gatewayIP());
  Serial.println(Ethernet.dnsServerIP());
#endif

  int success = udp.begin(5000);
}

char message[1024];

void loop() {
  //check for new udp-packet:
  int size = udp.parsePacket();
  if (size > 0) {
    do {
        if (size == 360) {
          udp.read(message, size + 1);
          memcpy(leds_1, message, sizeof(CRGB) * 2 * 60);
          memcpy(leds_2, message, sizeof(CRGB) * 2 * 60);
        } else {
          char* msg = (char*)malloc(size+1);
          int len = udp.read(msg,size+1);
          msg[len]=0;
#ifdef DEBUG_OUTPUT
          Serial.println(msg);
#endif
          CRGB set_value;
          switch(msg[0]) {
            case 'r' : set_value = CRGB::Red; break;
            case 'g' : set_value = CRGB::Green; break;
            case 'b' : set_value = CRGB::Blue; break;
            default : set_value = CRGB::White; break;
          }
          for (int i = 0; i < NUM_LEDS; ++i) {
            leds_1[i] = set_value;
          }
          free(msg);
        }
    } while ((size = udp.available())>0);
    FastLED.show();
    //finish reading this packet:
    udp.flush();
    // stop start is necessary for other clients to connect, but do we really need it?
    udp.stop();
    udp.begin(5000);
  }

//  FastLED.delay(1000 / UPDATES_PER_SECOND);

}
