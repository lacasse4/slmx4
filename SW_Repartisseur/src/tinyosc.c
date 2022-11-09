/**
 * Copyright (c) 2015-2018, Martin Roth (mhroth@gmail.com)
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define tosc_strncpy(_dst, _src, _len) strncpy(_dst, _src, _len)
#include "tinyosc.h"

#define BUNDLE_ID 0x2362756E646C6500L // "#bundle"

// http://opensoundcontrol.org/spec-1_0
int tosc_parseMessage(tosc_message *o, char *buffer, const int len) {
  // NOTE(mhroth): if there's a comma in the address, that's weird
  int i = 0;
  while (buffer[i] != '\0') ++i; // find the null-terimated address
  while (buffer[i] != ',') ++i; // find the comma which starts the format string
  if (i >= len) return -1; // error while looking for format string
  // format string is null terminated
  o->format = buffer + i + 1; // format starts after comma

  while (i < len && buffer[i] != '\0') ++i;
  if (i == len) return -2; // format string not null terminated

  i = (i + 4) & ~0x3; // advance to the next multiple of 4 after trailing '\0'
  o->marker = buffer + i;

  o->buffer = buffer;
  o->len = len;

  return 0;
}

// check if first eight bytes are '#bundle '
bool tosc_isBundle(const char *buffer) {
  return ((*(const int64_t *) buffer) == htonl(BUNDLE_ID));
}

void tosc_parseBundle(tosc_bundle *b, char *buffer, const int len) {
  b->buffer = (char *) buffer;
  b->marker = buffer + 16; // move past '#bundle ' and timetag fields
  b->bufLen = len;
  b->bundleLen = len;
}

uint64_t tosc_getTimetag(tosc_bundle *b) {
  return ntohl(*((uint64_t *) (b->buffer+8)));
}

uint32_t tosc_getBundleLength(tosc_bundle *b) {
  return b->bundleLen;
}

bool tosc_getNextMessage(tosc_bundle *b, tosc_message *o) {
  if ((b->marker - b->buffer) >= b->bundleLen) return false;
  uint32_t len = (uint32_t) ntohl(*((int32_t *) b->marker));
  tosc_parseMessage(o, b->marker+4, len);
  b->marker += (4 + len); // move marker to next bundle element
  return true;
}

char *tosc_getAddress(tosc_message *o) {
  return o->buffer;
}

char *tosc_getFormat(tosc_message *o) {
  return o->format;
}

uint32_t tosc_getLength(tosc_message *o) {
  return o->len;
}

int32_t tosc_getNextInt32(tosc_message *o) {
  // convert from big-endian (network btye order)
  const int32_t i = (int32_t) ntohl(*((uint32_t *) o->marker));
  o->marker += 4;
  return i;
}

int64_t tosc_getNextInt64(tosc_message *o) {
  const int64_t i = (int64_t) ntohl(*((uint64_t *) o->marker));
  o->marker += 8;
  return i;
}

uint64_t tosc_getNextTimetag(tosc_message *o) {
  return (uint64_t) tosc_getNextInt64(o);
}

float tosc_getNextFloat(tosc_message *o) {
  // convert from big-endian (network btye order)
  const uint32_t i = ntohl(*((uint32_t *) o->marker));
  o->marker += 4;
  return *((float *) (&i));
}

double tosc_getNextDouble(tosc_message *o) {
  const uint64_t i = ntohl(*((uint64_t *) o->marker));
  o->marker += 8;
  return *((double *) (&i));
}

const char *tosc_getNextString(tosc_message *o) {
  int i = (int) strlen(o->marker);
  if (o->marker + i >= o->buffer + o->len) return NULL;
  const char *s = o->marker;
  i = (i + 4) & ~0x3; // advance to next multiple of 4 after trailing '\0'
  o->marker += i;
  return s;
}

void tosc_getNextBlob(tosc_message *o, const char **buffer, int *len) {
  int i = (int) ntohl(*((uint32_t *) o->marker)); // get the blob length
  if (o->marker + 4 + i <= o->buffer + o->len) {
    *len = i; // length of blob
    *buffer = o->marker + 4;
    i = (i + 7) & ~0x3;
    o->marker += i;
  } else {
    *len = 0;
    *buffer = NULL;
  }
}

unsigned char *tosc_getNextMidi(tosc_message *o) {
  unsigned char *m = (unsigned char *) o->marker;
  o->marker += 4;
  return m;
}

tosc_message *tosc_reset(tosc_message *o) {
  int i = 0;
  while (o->format[i] != '\0') ++i;
  i = (i + 4) & ~0x3; // advance to the next multiple of 4 after trailing '\0'
  o->marker = o->format + i - 1; // -1 to account for ',' format prefix
  return o;
}


void tosc_printOscBuffer(char *buffer, const int len) {
  // parse the buffer contents (the raw OSC bytes)
  // a return value of 0 indicates no error
  tosc_message m;
  const int err = tosc_parseMessage(&m, buffer, len);
  if (err)
  	  printf("Error while reading OSC buffer: %i\n", err);
}

int tosc_printMessage(tosc_message *osc, void* buffer) {
  char _add[32] = {0};
  strcpy(_add, tosc_getAddress(osc));

  switch(_add[0])
  {
  case '@':
	  memmove(_add, _add+1, strlen(_add));
	  strcpy(buffer, _add);
	  return 1;
	  break;
  case '/':
	  memmove(_add, _add+1, strlen(_add));
	  strcpy(buffer, _add);
	  return 2;
  }

  return 0;

}
