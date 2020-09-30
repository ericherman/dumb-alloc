/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* dumb_alloc_tests_arduino.ino : testing dumb-alloc in arduino */
/* Copyright (C) 2020 Eric Herman */
/* https://github.com/ericherman/libefloat */

#include <Arduino.h>
#include <HardwareSerial.h>

#include "dumb-alloc.h"

/* Allocate the buffer used by dumb_alloc */
const size_t global_buffer_len = 512;
unsigned char global_buffer[global_buffer_len];
struct dumb_alloc da;

uint16_t loop_delay_ms = (2 * 1000);
uint32_t loop_count;

const size_t max_str_len = 60;
const size_t strings_len = 10;
char *strings[strings_len];

void setup(void)
{
	Serial.begin(9600);

	// wait to ensure stable Serial
	delay(50);

	Serial.println("Begin");
	print_data_sizes();

	for (size_t i = 0; i < global_buffer_len; ++i) {
		global_buffer[i] = 0;
	}
	dumb_alloc_die = crash_and_reboot;
	dumb_alloc_init(&da, global_buffer, global_buffer_len);
	dumb_alloc_set_global(&da);

	for (size_t i = 0; i < strings_len; ++i) {
		strings[i] = NULL;
	}

	loop_count = 0;
}

size_t our_strlen(const char *s)
{
	if (!s) {
		return 0;
	}
	size_t i = 0;
	while (s[i]) {
		++i;
	}
	return i;
}

char char_for(size_t loop_count)
{
	size_t cidx = loop_count % (10 + 26);

	char c = '\0';
	if (cidx < 10) {
		c = '0' + cidx;
	} else {
		c = 'a' + (cidx - 10);
	}
	return c;
}

void loop(void)
{
	++loop_count;
	Serial.println("=================================================");
	Serial.print(" Starting run #");
	Serial.println(loop_count);
	Serial.println("=================================================");
	Serial.println();

	// free the old string
	size_t idx = loop_count % strings_len;
	if (strings[idx]) {
		Serial.print("freeing ");
		Serial.print(our_strlen(strings[idx]) + 1);
		Serial.println(" bytes");
		dumb_free(strings[idx]);
		strings[idx] = NULL;
	}
	Serial.println();
	// make a new string:
	size_t buflen = (loop_count % max_str_len) + 1;
	Serial.println("allocating ");
	Serial.print(buflen);
	Serial.println(" bytes");
	char *buf = (char *)dumb_malloc(buflen);
	if (!buf) {
		Serial.print("COULD NOT ALLOCATE ");
		Serial.print(buflen);
		Serial.println(" BYTES");
	} else {
		Serial.print("allocated ");
		Serial.print(buflen);
		Serial.println(" bytes");
		char c = char_for(loop_count);
		for (size_t i = 0; i < buflen; ++i) {
			buf[i] = c;
		}
		buf[buflen - 1] = '\0';
		strings[idx] = buf;
	}

	size_t used = 0;
	size_t objects = 0;
	Serial.println();
	Serial.println("Currently allocated objects: ");
	// print all the strings
	for (size_t i = 0; i < strings_len; ++i) {
		Serial.print(" strings[");
		Serial.print(i);
		Serial.print("] = ");
		if (strings[i]) {
			used += (1 + our_strlen(strings[i]));
			Serial.println(strings[i]);
			++objects;
		} else {
			Serial.println("(null)");
		}
	}
	Serial.print(used);
	Serial.print(" bytes allocated across ");
	Serial.print(objects);
	Serial.println(" objects");

	Serial.println();
	Serial.println("=================================================");
	Serial.println();
	for (size_t i = 0; i < 20; ++i) {
		Serial.print(".");
		delay(loop_delay_ms / 20);
	}

	Serial.println();
}

/* calling this NULL function pointer will force a reset */
int (*bogus_function_crash)(void) = NULL;

int crash_and_reboot(void)
{
	Serial.println();
	Serial.println("Aborting.");
	delay(10UL * 1000UL);
	bogus_function_crash();
	return -1;
}

void print_data_sizes(void)
{
	Serial.print("sizeof(short) == ");
	Serial.println(sizeof(short));

	Serial.print("sizeof(int) == ");
	Serial.println(sizeof(int));

	Serial.print("sizeof(long) == ");
	Serial.println(sizeof(long));

	Serial.print("sizeof(size_t) == ");
	Serial.println(sizeof(size_t));

	Serial.print("sizeof(float) == ");
	Serial.println(sizeof(float));

	Serial.print("sizeof(double) == ");
	Serial.println(sizeof(double));

	Serial.print("DUMB_ALLOC_WORDSIZE = ");
	Serial.println(DUMB_ALLOC_WORDSIZE);
}
