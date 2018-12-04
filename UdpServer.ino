#include <FastLED.h>
#include <UIPEthernet.h>

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

#define MYIPADDR 192,168,1,154
#define MYIPMASK 255,255,255,0
#define MYDNS 192,168,1,1
#define MYGW 192,168,1,1

void setup() {
  uint8_t mac[6] = {0xde,0xad,0xbe,0xef,0x04,0x05};

  delay(3000); // power-up safety delay
  disableDebugPorts();

  FastLED.addLeds<LED_TYPE, LED_PIN_1, COLOR_ORDER>(leds_1, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED_PIN_2, COLOR_ORDER>(leds_2, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  while (!Serial);
  Serial.begin(115200);
  Serial.println("Starting up");

#if 1
  uint8_t myIP[4] = {MYIPADDR};
  uint8_t myMASK[4] = {MYIPMASK};
  uint8_t myDNS[4] = {MYDNS};
  uint8_t myGW[4] = {MYGW};
  Ethernet.begin(mac,myIP,myDNS,myGW,myMASK);
#else
  Ethernet.begin(mac);
#endif
  Serial.println(Ethernet.localIP());
  Serial.println(Ethernet.subnetMask());
  Serial.println(Ethernet.gatewayIP());
  Serial.println(Ethernet.dnsServerIP());

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
          Serial.println(msg);
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
