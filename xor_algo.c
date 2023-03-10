/*
 * Implementation of a XOR ciper algorithm using a circular XOR key with an optional start and interval
 * Copyright, copyleft, copywrong Antoniu-George Savu <santoniu@gmail.com+spambotremovethis>
 */
#include "xor_algo.h"

unsigned int xor_algo(void *buffer, long long int len, unsigned int xor_key_interval_start, unsigned int xor_key_interval_len, unsigned int *xor_key_offset) {
    if ( buffer == 0 ) /* NULL buffer ptr */
	return XOR_ALGO_OK;

    if ( len < 0 )
	return XOR_ALGO_ERROR_BUFFER_INVALID_LENGTH;

    if ( xor_key_interval_start > XOR_KEY_LENGTH_IN_BYTES )
	xor_key_interval_start = 0;

    if ( (xor_key_interval_start + xor_key_interval_len) > XOR_KEY_LENGTH_IN_BYTES ) {
	xor_key_interval_start = 0;
	xor_key_interval_len = XOR_KEY_LENGTH_IN_BYTES;
    }

    unsigned int xor_key_interval_end = xor_key_interval_start + xor_key_interval_len - 1;

    unsigned char xor_key_bytes[] = XOR_KEY_BYTES;
    for (long long int i = 0; i < len; i++) {
	if ( (xor_key_interval_start + *xor_key_offset) > xor_key_interval_end )
	    *xor_key_offset = 0;
	*(unsigned char *)(buffer + i) = *(unsigned char *)(buffer + i) ^ xor_key_bytes[xor_key_interval_start + *xor_key_offset];
	*xor_key_offset += 1;
    }
    return XOR_ALGO_OK;
}

unsigned int xor_algo_key_crc(void) {
    unsigned char xor_key_bytes[] = XOR_KEY_BYTES;
    unsigned int xor_key_crc = 0;
    for (unsigned int i = 0; i < XOR_KEY_LENGTH_IN_BYTES; i++) {
	xor_key_crc += *(unsigned char *)(xor_key_bytes + i) * i;
    }
    return xor_key_crc * (XOR_KEY_LENGTH_IN_BYTES*XOR_KEY_LENGTH_IN_BYTES);
}
