// Access control: CalSTAR MR

#include <avr/io.h>
#include <util/delay.h>
#include <Arduino.h>
#include "Wire.h"
#include "SparkFunMPL3115A2.h"

#define F_CPU 16000000
#define RECEIVER_IO DDRD
#define RECEIVER_PORT PIND
#define RECEIVER_PIN 3
#define CHARGE_IO DDRC
#define CHARGE_PORT PORTC
#define CHARGE_PIN 1
#define LED_IO DDRD
#define LED_PORT PORTD
#define LED_PIN_RED 5
#define LED_PIN_GREEN 6
#define LED_PIN_BLUE 7
#define BUZZER_IO DDRB
#define BUZZER_PORT PORTB
#define BUZZER_PIN 1
#define CONTINUITY_IO DDRD
#define CONTINUITY_PORT PIND
#define CONTINUITY_PIN 4
#define ACCEL_TOLERANCE 1
#define ALT_LAND 50
#define ALT_LAUNCHED 200
#define ACCEL_ADDR 0x00
#define ALT_ADDR 0x60
#define ALT_WRITE_ADDR 0xC0
#define ALT_READ_ADDR 0xC1
#define BASE_ALT 0

int main() {
	/* Pseudocode for deployment sequence:
	1. Verify vehicle has launched with altimeter
	2. Wait until ADC input exceeds ADC threshold
	3. Verify vehicle is at rest on the ground with altimeter
	4. Actuate solenoid. */

	// Setup
	// Signal receiver and continuity default to input
	LED_IO = (1 << LED_PIN_RED) | (1 << LED_PIN_GREEN) | (1 << LED_PIN_BLUE); // configure LEDs as output
	CHARGE_IO = 0xFF; // configure black powder as output
	BUZZER_IO = 0xFF; // configure buzzer as output
	LED_PORT = 0x00; // set LEDs to low
	CHARGE_PORT = 0x00; // set charge to low
	BUZZER_PORT = 0x00; // set buzzer to low
	MPL3115A2 altimeter;

	// Configure accelerometer
	Wire.begin();
	altimeter.begin();
	altimeter.setModeAltimeter();
	altimeter.setOversampleRate(7);
	altimeter.enableEventFlags();

	// Initialize timer vars
	unsigned long start_time;
	unsigned long buzzer_time;
	start_time = millis();
	buzzer_time = micros();

	// Main program thread
	while (altimeter.readAltitudeFt() - BASE_ALT < ALT_LAUNCHED) { // wait for vehicle to launch
		beep(start_time, buzzer_time);
	}
	LED_PORT = (1 << LED_PIN_RED); // set LED to red to indicate launch
	while (waitForSignal() == 0) { // wait for ejection signal and cross-check with sensor
		beep(start_time, buzzer_time);
	}
	LED_PORT = (1 << LED_PIN_GREEN); // set LED to green to indicate receipt of signal

	CHARGE_PORT = (1 << CHARGE_PIN); // trigger black powder
	LED_PORT = (1 << LED_PIN_BLUE); // set LED to blue to indicate completion of program
	while (true) {
		beep(start_time, buzzer_time);
	}
	return 0;
}

char waitForSignal() {
	while(!(RECEIVER_PORT & (1 << RECEIVER_PIN))); // wait for signal from ejection
	if (altimeter.readAltitudeFt() - BASE_ALT > ALT_LAND) { // if not on ground
		return 0;
	}
	return 1; // landed!
}

void beep(unsigned long& start_time, unsigned long& buzzer_time) {
	static short buzzer = 0;
	unsigned long current_time_ms = millis();
	unsigned long current_time_us = micros();
	if (current_time_ms - start_time >= 1000) { // Counts up to one second then resets
		start_time = current_time_ms;
	}
	if (current_time_us - buzzer_time >= 500) { // Run every 500us
		buzzer_time = current_time_us;
		// buzzer frequency = 1/(2*500us) = 1000 Hz
		if ((current_time_ms - start_time <= 400) && (detectContinuity())) { // If in first 400ms of the second and still detecting continuity
			buzzer = !buzzer;
			BUZZER_PORT = (buzzer << BUZZER_PIN);
		}
	}
}

short detectContinuity() {
	return (CONTINUITY_PORT & (1 << CONTINUITY_PIN));
}
