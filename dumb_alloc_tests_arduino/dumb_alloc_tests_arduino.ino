/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* dumb_alloc_tests_arduino.ino : testing dumb-alloc in arduino */
/* Copyright (C) 2020 Eric Herman */
/* https://github.com/ericherman/dumb-alloc */

#include <Arduino.h>
#include <HardwareSerial.h>

/* calling this NULL function pointer will force a reset */
int (*bogus_function_crash)(void) = NULL;

/* Allocate the big resources needed by the tests */
const size_t global_buffer_len = 2048;
unsigned char global_buffer[global_buffer_len];
size_t dumb_alloc_test_global_buffer_len = global_buffer_len;
unsigned char *dumb_alloc_test_global_buffer = global_buffer;

const size_t logbuflen = 1000;
char logbuf[logbuflen];
size_t dumb_alloc_test_logbuflen = logbuflen;
char *dumb_alloc_test_logbuf = logbuf;

const size_t buf2_len = 256;
unsigned char buf2[buf2_len];
size_t dumb_alloc_medium_buffer_len = buf2_len;
unsigned char *dumb_alloc_medium_buffer = buf2;

/* prototypes for the test functions, are not needed in the hosted case
 * becase these are normally called by individual programs in parallel, but
 * we need these exported because this firmware will run the tests from each
 * test compilation unit */
extern int test_big_allocs(void);
extern int test_checkered_alloc(void);
extern int test_checkered_realloc(void);
extern int test_free(void);
extern int test_out_of_memory(void);
extern int test_pool(void);
extern int test_simple(void);
extern int test_to_string(void);
extern int test_two_alloc(void);

#include "dumb-alloc.h"

/* setup/loop globals */
uint32_t loop_count;

int crash_and_reboot(void)
{
	Serial.println();
	Serial.println("Aborting.");
	delay(3000);
	bogus_function_crash();
	return -1;
}

void setup(void)
{
	dumb_alloc_die = crash_and_reboot;

	Serial.begin(9600);

	delay(50);

	Serial.println("Begin");

	loop_count = 0;

	Serial.print("sizeof(size_t) == ");
	Serial.println(sizeof(size_t));
}

void loop(void)
{
	++loop_count;
	Serial.println("=================================================");
	Serial.print("Starting test run #");
	Serial.println(loop_count);
	Serial.println("=================================================");

	int failures = 0;

	memset(global_buffer, 0x00, global_buffer_len);

	failures += test_big_allocs();
	failures += test_checkered_alloc();
	failures += test_checkered_realloc();
	failures += test_free();
	failures += test_pool();
	failures += test_simple();
	failures += test_to_string();
	failures += test_two_alloc();

	// this test simulates allocation failure and ensures that the library
	// leaves itself in a sane state afterwards
	failures += test_out_of_memory();

	Serial.println();
	if (failures != 0) {
		Serial.print("There were ");
		Serial.print(failures);
		Serial.println(" failures.");
	} else {
		Serial.println("SUCCESS!");
	}
	Serial.println("=================================================");

	for (int i = 0; i < 20; ++i) {
		Serial.print(".");
		delay(failures ? 1000 : 100);
	}

	Serial.println();
}
