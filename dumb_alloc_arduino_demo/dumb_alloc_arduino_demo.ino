/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* dumb_alloc_arduino_demo.ino : showing dumb-alloc in arduino */
/* Copyright (C) 2020 Eric Herman */
/* https://github.com/ericherman/dumb-alloc */

#include <Arduino.h>
#include <HardwareSerial.h>

#include "dumb-alloc.h"

// Allocate the buffer used by dumb_alloc
const size_t global_buffer_len = 512;
unsigned char global_buffer[global_buffer_len];

// the global dumb_alloc struct used by dumb_alloc() and dumb_free()
struct dumb_alloc da;

// the buffers[] array will be where we stash allocated memory
const size_t buffers_len = 10;
char *buffers[buffers_len];

// we will allocate buffers of more than 70 characters in order to
// fit nicely on an 80 column screen. Note: 70 * 10 is intentionally
// larger than 512, thus we can not possibly allocate all of the
// buffers at max length; we should expect some allocation requrests
// to be denied by dumb_malloc()
const size_t max_str_len = 70;

uint16_t loop_delay_ms = (2 * 1000);
uint32_t loops_completed;

void setup(void)
{
	Serial.begin(9600);
	// wait to ensure stable Serial initialziaiton
	delay(50);

	// struct initialization includes using some of the buffer
	// space for internal data structures; the usable space will
	// be slightly less than this
	dumb_alloc_init(&da, global_buffer, global_buffer_len);

	// for the global dumb_alloc() and dumb_free() to use our
	// struct:
	dumb_alloc_set_global(&da);

	loops_completed = 0;

	Serial.println();
	Serial.println("==================================================");
	Serial.println();
	Serial.println("Begin");
	Serial.println();
	Serial.println("==================================================");
}

void loop(void)
{
	size_t idx = loops_completed % buffers_len;

	Serial.print(" Starting run number ");
	Serial.print(loops_completed);
	Serial.print(" using b[");
	Serial.print(idx);
	Serial.println("]");
	Serial.println("==================================================");
	Serial.println();

	// if there is a pre-existing string at this index,
	// free the old string
	if (buffers[idx]) {
		Serial.print("freeing ");
		Serial.print(our_strlen(buffers[idx]) + 1);
		Serial.println(" bytes");
		dumb_free(buffers[idx]);
		buffers[idx] = NULL;
		Serial.println();
	}
	// choose an arbitrary size ... after several loops, we can
	// expect some pretty fragmented memory
	size_t buflen = (millis() % max_str_len);
	if (buflen < 2) {
		buflen = 2;
	}
	Serial.print("requesting allocation of ");
	Serial.print(buflen);
	Serial.println(" bytes");

	// Make space for a new buffer by calling dumb_malloc()
	// however, since the total global buffer size is less than
	// the total size of all the strings if they are maxium
	// length, the call to malloc may fail:
	char *buf = (char *)dumb_malloc(buflen);
	if (!buf) {
		Serial.println();
		Serial.print("COULD NOT ALLOCATE ");
		Serial.print(buflen);
		Serial.println(" BYTES");
	} else {
		Serial.print("allocated ");
		Serial.print(buflen);
		Serial.println(" bytes");

		// we will fill our allocated buffers with a string
		// of a single character repeating, this will help
		// give us some confidence that the data is not
		// getting over-written by other allocations
		char c = char_for(loops_completed);
		for (size_t i = 0; i < buflen; ++i) {
			buf[i] = c;
		}
		buf[buflen - 1] = '\0';
		buffers[idx] = buf;
	}

	size_t bytes_used = 0;
	size_t buffers_used = 0;
	Serial.println();
	Serial.println("Currently allocated buffers: ");
	// print all the strings
	for (size_t i = 0; i < buffers_len; ++i) {
		Serial.print(" b[");
		Serial.print(i);
		Serial.print("] = ");
		if (buffers[i]) {
			bytes_used += (1 + our_strlen(buffers[i]));
			Serial.println(buffers[i]);
			++buffers_used;
		} else {
			Serial.println("(null)");
		}
	}

	Serial.println();

	Serial.print(bytes_used);
	Serial.print(" bytes allocated across ");
	Serial.print(buffers_used);
	Serial.println(" active allocations");

	Serial.println();
	for (size_t i = 0; i < 50; ++i) {
		Serial.print("=");
		delay(loop_delay_ms / 50);
	}
	Serial.println();

	++loops_completed;
}

char char_for(size_t n)
{
	n = (n % 36);
	if (n < 10) {
		return '0' + n;
	} else {
		n -= 10;
		return 'a' + n;
	}
}

// a DIY version of the standard C library "strlen" so that we do not need to
// pull in the bloat of a libc into our firmware just to get this function,
// this version is simple, even if it is perhaps a bit less efficient than the
// glibc version.
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
