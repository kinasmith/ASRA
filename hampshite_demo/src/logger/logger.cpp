#include <Arduino.h>
#include <RFM69_ATC.h>//get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include "DS3231.h"
#include "SdFat.h"

#define NODEID 0
#define FREQUENCY RF69_433MHZ
#define ATC_RSSI -70
#define ACK_WAIT_TIME 100 // # of ms to wait for an ack
#define ACK_RETRIES 10 // # of attempts before giving up
#define SERIAL_BAUD 9600
#define LED 3
#define SD_CS_PIN 4
#define CARD_DETECT 5

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

void Blink(byte, int);
void checkSdCard();

/*==============|| RFM69 ||==============*/
RFM69_ATC radio; //Initialize Radio
uint8_t NETWORKID = 100; //base network address
byte lastRequesterNodeID = NODEID;
uint8_t NodeID_latch;

/*==============|| DS3231_RTC ||==============*/
DateTime now;
DS3231 rtc; //Initialize the Real Time Clock

/*==============|| SD ||==============*/
SdFat SD; //This is the sd Card
uint8_t CARD_PRESENT; //var for Card Detect sensor reading

struct TimeStamp {
	uint32_t timestamp;
};
TimeStamp theTimeStamp;

struct Payload {
	float temp;
	float humidity;
	float voltage;
};
Payload thePayload;

void setup() {
	bool sd_OK = false;
	pinMode(LED, OUTPUT);
	pinMode(CARD_DETECT, INPUT_PULLUP);
	rtc.begin();
	// rtc.adjust(DateTime((__DATE__), (__TIME__))); //sets the RTC to the computer time.
	Serial.begin(SERIAL_BAUD);
	radio.initialize(FREQUENCY,NODEID,NETWORKID);
	radio.setHighPower(); //only for RFM69HW!
	radio.enableAutoPower(ATC_RSSI);
	radio.encrypt(null);
	DEBUG(NETWORKID); DEBUG("."); DEBUG(NODEID);
	DEBUG(" is listening at 433mhz");
	DEBUGln();

	digitalWrite(LED, HIGH);
	CARD_PRESENT = digitalRead(CARD_DETECT); //read CD pin (invert it so logic stays logical)
	DEBUG("Card Present Status: "); DEBUGln(CARD_PRESENT);
	now = rtc.now();
	if(CARD_PRESENT) {
		DEBUG("SD ");
		if (SD.begin(SD_CS_PIN)) {
			DEBUG("init, ");
			File f;
			if(f.open("start.txt", FILE_WRITE)) {
				DEBUGln("file, OK");
				f.print("program started at ");
				f.print(now.unixtime());
				f.println();
				f.close();
				sd_OK = true;
			}
		}
	}
	if(!sd_OK) {
		while(1) {
			Blink(LED, 100); //blink to show an error.
			Blink(LED, 200); //blink to show an error.
		}
	}
	digitalWrite(LED, LOW);
	DEBUG("Everything OK at "); DEBUGln(now.unixtime());///Victory
	DEBUGln("==========================");
}

void loop() {
	bool writeData = false;
	bool sendData = false;
	if (radio.receiveDone()) {
		// DEBUG("rcv < "); DEBUG('['); DEBUG(radio.SENDERID); DEBUG("] ");
		lastRequesterNodeID = radio.SENDERID;
		now = rtc.now();
		theTimeStamp.timestamp = now.unixtime();
		/*=== WRITE DATA ===*/
		if (radio.DATALEN == sizeof(thePayload)) {
			thePayload = *(Payload*)radio.DATA;
			writeData = true;
			sendData = true;
		}
		if (radio.ACKRequested()) radio.sendACK();
		Blink(LED,5);
	}

	if(writeData) {
		File f;
		String address = String(String(NETWORKID) + "_" + String(lastRequesterNodeID));
		String fileName = String(address + ".csv");
		char _fileName[fileName.length() +1];
		fileName.toCharArray(_fileName, sizeof(_fileName));
		if (!f.open(_fileName, FILE_WRITE)) {
			DEBUG("sd - error opening "); DEBUG(_fileName); DEBUGln();
		}
		f.print(NETWORKID); f.print(".");
		f.print(radio.SENDERID); f.print(",");
		f.print(thePayload.temp); f.print(",");
		f.print(thePayload.humidity); f.print(",");
		f.print(thePayload.voltage);
		f.println();
		f.close();
	}
	if(sendData) {
		Serial.print("A");
		Serial.print(lastRequesterNodeID);
		Serial.print(",");
		Serial.print(thePayload.temp);
		Serial.print(",");
		Serial.print(thePayload.humidity);
		Serial.print(",");
		Serial.print(thePayload.voltage);
		Serial.println();
	}
	checkSdCard(); //Checks for card insertion
}

void checkSdCard() {
	CARD_PRESENT = digitalRead(CARD_DETECT); //invert for logic's sake
	if (!CARD_PRESENT) {
		DEBUGln("sd - card Not Present");
		while (1) {
			Blink(LED, 100);
			Blink(LED, 200); //blink to show an error.
		}
	}
}

void Blink(byte PIN, int DELAY_MS) {
	digitalWrite(PIN,HIGH);
	delay(DELAY_MS);
	digitalWrite(PIN,LOW);
	delay(DELAY_MS);
}
