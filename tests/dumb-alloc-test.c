/*
test-dumb-alloc.c: common test functions
Copyright (C) 2012, 2017 Eric Herman <eric@freesa.org>

This work is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later
version.

This work is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License (COPYING) along with this library; if not, see:

        https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt
*/
#include "dumb-alloc-test.h"

struct dumb_alloc_log logger;
struct dumb_alloc_char_buf log_context;
struct dumb_alloc da;
size_t faux_pages_used;

void dumb_alloc_test_reset_global(void)
{
	Dumb_alloc_memset(dumb_alloc_test_global_buffer, 0x00,
			  dumb_alloc_test_global_buffer_len);
	dumb_alloc_init(&da, dumb_alloc_test_global_buffer,
			dumb_alloc_test_global_buffer_len);
	dumb_alloc_set_global(&da);
	faux_pages_used = 0;
}

/* a pretend library function which does an allocation, works like strdup */
char *dumb_alloc_test_strdup(const char *s)
{
	char *copy = (char *)d_malloc(Dumb_alloc_test_strnlen(s, INT_MAX) + 1);
	if (copy) {
		Dumb_alloc_test_strcpy(copy, s);
	}
	return copy;
}

#if Dumb_alloc_test_diy_strstr
char *dumb_alloc_test_strstr(const char *haystack, const char *needle)
{
	size_t i, j, found, len;

	if (!haystack) {
		return NULL;
	}

	len = 0;
	if (!needle || !(len = Dumb_alloc_test_strnlen(needle, INT_MAX))) {
		return (char *)haystack;
	}

	for (i = 0; haystack[i]; ++i) {
		found = 1;
		for (j = 0; found && j < len; ++j) {
			if (haystack[i + j] != needle[j]) {
				found = 0;
			}
		}
		if (found) {
			return (char *)(haystack + i);
		}
	}
	return NULL;
}
#endif

#if Dumb_alloc_test_diy_strcmp
int dumb_alloc_test_strcmp(const char *s1, const char *s2)
{
	size_t i;

	i = 0;
	if (s1 == s2) {
		return 0;
	}
	if (!s1 || !s2) {
		return s1 ? 1 : -1;
	}
	for (i = 0; s1[i] && s2[i] && s1[i] == s2[i]; ++i) ;
	return s1[i] - s2[i];
}
#endif

#if Dumb_alloc_test_diy_strcpy
char *dumb_alloc_test_strcpy(char *dest, const char *src)
{
	size_t i;
	if (!dest) {
		return NULL;
	}
	i = 0;
	if (src) {
		while (*(src + i)) {
			*(dest + i) = *(src + i);
			++i;
		}
	}
	*(dest + i) = '\0';
	return dest;
}
#endif

#if Dumb_alloc_test_diy_strnlen
size_t dumb_alloc_test_strnlen(const char *str, size_t buf_size)
{
	size_t i;

	for (i = 0; i < buf_size; ++i) {
		if (*(str + i) == '\0') {
			return i;
		}
	}

	return buf_size;
}
#endif

static char dumb_alloc_nibble_to_hex(unsigned char nibble)
{
	if (nibble < 10) {
		return '0' + nibble;
	}

	return 'A' + nibble - 10;
}

static void dumb_alloc_byte_to_hex_chars(char *hi, char *lo, unsigned char byte)
{
	*hi = dumb_alloc_nibble_to_hex((byte & 0xF0) >> 4);
	*lo = dumb_alloc_nibble_to_hex((byte & 0x0F));
}

char *dumb_alloc_u64_to_hex(char *buf, size_t len, uint64_t z)
{
	size_t i = 0;
	size_t pos = 0;
	size_t u64_bytes = sizeof(uint64_t);

	if (len < _Dumb_alloc_test_u64_hex_buf_len) {
		return NULL;
	}

	buf[pos++] = '0';
	buf[pos++] = 'x';

	for (i = 0; i < u64_bytes; ++i) {
		char h = 0;
		char l = 0;
		unsigned char byte = 0;

		byte = (0xFF & (z >> (8 * ((u64_bytes - 1) - i))));
		dumb_alloc_byte_to_hex_chars(&h, &l, byte);
		buf[pos++] = h;
		buf[pos++] = l;
	}
	buf[pos] = '\0';
	return buf;

}

char *dumb_alloc_size_to_str(char *buf, size_t len, size_t z)
{
	char tmp[_Dumb_alloc_test_size_buf_len];
	size_t i = 0;
	size_t j = 0;

	if (!buf || !len) {
		return NULL;
	} else if (len == 1) {
		buf[0] = '\0';
		return NULL;
	}

	if (!z) {
		buf[0] = '0';
		buf[1] = '\0';
		return buf;
	}

	for (i = 0; z && i < _Dumb_alloc_test_size_buf_len; ++i) {
		tmp[i] = '0' + (z % 10);
		z = z / 10;
	}
	for (j = 0; i && j < len; ++j, --i) {
		buf[j] = tmp[i - 1];
	}

	buf[j < len ? j : len - 1] = '\0';

	return buf;
}

int dumb_alloc_test_append_s(struct dumb_alloc_log *logger, const char *str)
{
	struct dumb_alloc_char_buf *ctx = NULL;
	size_t i = 0;

	ctx = (struct dumb_alloc_char_buf *)logger->context;
	i = Dumb_alloc_test_strnlen(ctx->buf, ctx->len);

	while (i < ctx->len && *str) {
		ctx->buf[i++] = *str;
		++str;
	}
	if (i == ctx->len) {
		ctx->buf[ctx->len - 1] = '\0';
		return -1;
	}
	ctx->buf[i] = '\0';
	return i;
}

int dumb_alloc_test_append_z(struct dumb_alloc_log *logger, size_t z)
{
	char buf[_Dumb_alloc_test_size_buf_len];

	dumb_alloc_size_to_str(buf, _Dumb_alloc_test_size_buf_len, z);

	return dumb_alloc_test_append_s(logger, buf);
}

int dumb_alloc_test_append_v(struct dumb_alloc_log *logger, const void *v)
{
	char buf[_Dumb_alloc_test_u64_hex_buf_len];
	uint64_t u64 = (uint64_t) ((size_t)v);

	dumb_alloc_u64_to_hex(buf, _Dumb_alloc_test_u64_hex_buf_len, u64);

	return dumb_alloc_test_append_s(logger, buf);
}

int dumb_alloc_test_append_eol(struct dumb_alloc_log *logger)
{
	return dumb_alloc_test_append_s(logger, "\n");
}

void dumb_alloc_log_init(struct dumb_alloc_log *logger,
			 struct dumb_alloc_char_buf *log_context,
			 char *buf, size_t len)
{
	size_t i = 0;
	for (i = 0; i < len; ++i) {
		buf[i] = 0x00;
	}

	log_context->buf = buf;
	log_context->len = len;

	logger->context = log_context;

	logger->puts = dumb_alloc_test_append_s;
	logger->putz = dumb_alloc_test_append_z;
	logger->putv = dumb_alloc_test_append_v;
	logger->puteol = dumb_alloc_test_append_eol;
}

void *d_malloc(size_t size)
{
	void *ptr = NULL;

	dumb_alloc_log_init(&logger, &log_context, dumb_alloc_test_logbuf,
			    dumb_alloc_test_logbuflen);

	ptr = dumb_malloc(size);
	if (!ptr) {
		Dumb_alloc_debug_prints("\n");
		Dumb_alloc_debug_prints("alloc returned NULL\n");
		dumb_alloc_to_string(dumb_alloc_get_global(), &logger);
		Dumb_alloc_debug_prints(dumb_alloc_test_logbuf);
		Dumb_alloc_debug_prints("\nFAIL\n");
	}
	return ptr;
}

void *d_calloc(size_t num, size_t size)
{
	void *ptr = NULL;

	dumb_alloc_log_init(&logger, &log_context, dumb_alloc_test_logbuf,
			    dumb_alloc_test_logbuflen);

	ptr = dumb_calloc(num, size);
	if (!ptr) {
		Dumb_alloc_debug_prints("\n");
		Dumb_alloc_debug_prints("alloc returned NULL\n");
		dumb_alloc_to_string(dumb_alloc_get_global(), &logger);
		Dumb_alloc_debug_prints(dumb_alloc_test_logbuf);
		Dumb_alloc_debug_prints("\nFAIL\n");
	}
	return ptr;
}

int dumb_alloc_test_compare_strings(const char *actual, const char *expected)
{
	int rv;

	dumb_alloc_log_init(&logger, &log_context, dumb_alloc_test_logbuf,
			    dumb_alloc_test_logbuflen);

	rv = Dumb_alloc_test_strcmp(actual, expected);
	if (rv == 0) {
		return 0;
	}

	Dumb_alloc_debug_prints("\n");
	Dumb_alloc_debug_prints("expected (");
	Dumb_alloc_debug_prints(expected);
	Dumb_alloc_debug_prints("' but was '");
	Dumb_alloc_debug_prints(actual);
	Dumb_alloc_debug_prints("'\n");
	dumb_alloc_to_string(dumb_alloc_get_global(), &logger);
	Dumb_alloc_debug_prints(dumb_alloc_test_logbuf);
	Dumb_alloc_debug_prints("\nFAIL\n");
	return 1;
}

#if FAUXFREESTANDING
extern int printf(const char *format, ...);
int print(const char *str)
{
	return printf("%s", str);
}

int printz(size_t size)
{
	return printf("%lu", (unsigned long)size);
}
#endif
