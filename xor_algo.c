/*
 * Implementation of a XOR ciper algorithm using a circular XOR key with an optional start and interval
 * Copyright, copyleft, copywrong Antoniu-George Savu <santoniu@gmail.com+spambotremovethis>
 */
#include <stddef.h>

#include "xor_algo.h"

unsigned int xor_algo(void *buffer, long long int len, unsigned int xor_key_interval_start, unsigned int xor_key_interval_len) {
    if ( buffer == NULL )
	return XOR_ALGO_ERROR_BUFFER_NULL;

    if ( len <= 0 )
	return XOR_ALGO_ERROR_BUFFER_INVALID_LENGTH;

    if ( xor_key_interval_start > XOR_KEY_LENGTH_IN_BYTES )
	return XOR_ALGO_ERROR_INTERVAL_START;

    if ( (xor_key_interval_start + xor_key_interval_len) > XOR_KEY_LENGTH_IN_BYTES )
	return XOR_ALGO_ERROR_INTERVAL;

    char xor_key_bytes[] = XOR_KEY_BYTES;
    char *xor_key_interval = (char *)(xor_key_bytes + xor_key_interval_start);
    char xor_key_byte;
    for (long long int i = 0; i < len; i++) {
	xor_key_byte = *(char *)(xor_key_interval + (i % xor_key_interval_len));
	*(char *)(buffer+i) = *(char *)(buffer+i) ^ xor_key_byte;
    }
    return XOR_ALGO_OK;
}

unsigned int xor_algo_key_crc(void) {
    char xor_key_bytes[] = XOR_KEY_BYTES;
    unsigned int xor_key_crc = 0;
    for (unsigned int i = 0; i < XOR_KEY_LENGTH_IN_BYTES; i++) {
	xor_key_crc += *(char *)(xor_key_bytes + i) * i;
    }
    return xor_key_crc * (XOR_KEY_LENGTH_IN_BYTES*XOR_KEY_LENGTH_IN_BYTES);
}
