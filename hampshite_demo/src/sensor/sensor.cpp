/**
 * Currently the EEPROM is really front loaded.
 * The time address is overwritten the most, and
 * the high addresses will never be used.figure out a way to balance the data load!
 */
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "SHT2x.h" //https://github.com/misenso/SHT2x-Arduino-Library
#include "Sleepy.h" //https://github.com/kinasmith/Sleepy
#include "RFM69_ATC.h" //https://www.github.com/lowpowerlab/rfm69

#define NODEID 9   //unique for each node on same network
#define NETWORKID 100  //the same on all nodes that talk to each other
#define GATEWAYID 0       //The address of the datalogger
#define FREQUENCY RF69_433MHZ //frequency of radio
#define ATC_RSSI -70 //ideal Signal Strength of trasmission
#define ACK_WAIT_TIME 100 // # of ms to wait for an ack
#define ACK_RETRIES 10 // # of attempts before giving up
#define SERIAL_BAUD 9600 // Serial comms rate
#define LED 8 // UI LED

#define SERIAL_EN //Comment this out to remove Serial comms and save a few kb's of space

#ifdef SERIAL_EN
#define DEBUG(input)   {Serial.print(input); delay(1);}
#define DEBUGln(input) {Serial.println(input); delay(1);}
#define DEBUGFlush() { Serial.flush(); }
#else
#define DEBUG(input);
#define DEBUGln(input);
#define DEBUGFlush();
#endif

// Functions
float getBatVoltage();
void Blink(int);

ISR(WDT_vect) {
	Sleepy::watchdogEvent();
}                                          //watchdog for sleeping timer

RFM69_ATC radio; //init radio

//Data Structure for transmitting data packets to datalogger (10 bytes)
struct Payload {
	float temp;
	float humidity;
	float voltage;
};
Payload thePayload;

const uint8_t BAT_EN = 3;

void setup() {
  #ifdef SERIAL_EN
		Serial.begin(SERIAL_BAUD);
	#endif
  Wire.begin();
  pinMode(BAT_EN, OUTPUT);
	pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(LED, LOW);
	randomSeed(analogRead(0)); //set random seed
	Wire.begin(); // Begin i2c
	radio.initialize(FREQUENCY,NODEID,NETWORKID);
	radio.setHighPower();
	radio.encrypt(null);
	radio.enableAutoPower(ATC_RSSI); //Test to see if this is actually working at some point
	DEBUG("-- Network Address: "); DEBUG(NETWORKID); DEBUG("."); DEBUGln(NODEID);
}

void loop() {

  thePayload.temp = SHT2x.GetTemperature();
  thePayload.humidity = SHT2x.GetHumidity();
  thePayload.voltage = getBatVoltage();

  DEBUG(thePayload.temp);
  DEBUG(",");
  DEBUG(thePayload.humidity);
  DEBUG(",");
  DEBUG(thePayload.voltage);
  DEBUGln();

	digitalWrite(LED, HIGH);
	if(radio.sendWithRetry(GATEWAYID, (const void*)(&thePayload), sizeof(thePayload)), ACK_RETRIES, ACK_WAIT_TIME) {
		DEBUG("data - snd - "); DEBUG('['); DEBUG(GATEWAYID); DEBUG("] ");
		DEBUG(sizeof(thePayload)); DEBUGln(" bytes  ");
		digitalWrite(LED, LOW);
	} else {
		DEBUGln("data - snd - Failed . . . no ack");
    Blink(100);
    Blink(100);
    Blink(100);
    Blink(100);
	}

  DEBUG("sleep - sleeping for 1 "); DEBUG(" seconds"); DEBUGln();
	DEBUGFlush();
	radio.sleep();

	Sleepy::loseSomeTime(500);
}

/**
 * [getBatVoltage description]
 * @return [description]
 */
float getBatVoltage() {
	/*
	    Gets battery voltage from sensor. Does 10x average on the analog reading
	 */
	uint8_t BAT = A0;
	uint16_t readings = 0;
	digitalWrite(BAT_EN, HIGH);
	delay(10);
	for (byte i=0; i<3; i++)
		readings += analogRead(BAT);
	readings /= 3;
	float v = 3.3 * (readings/1023.0) * (4300.0/2700.0); //Calculate battery voltage
	digitalWrite(BAT_EN, LOW);
	return v;
}

/**
 * [Blink description]
 * @param PIN      [description]
 * @param DELAY_MS [description]
 */
void Blink(int DELAY_MS) {
	digitalWrite(LED,HIGH);
	delay(DELAY_MS);
	digitalWrite(LED,LOW);
	delay(DELAY_MS);
}
