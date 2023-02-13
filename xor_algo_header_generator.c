#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
    int opt;
    extern char *optarg;
    extern int optind, opterr, optopt;

    int xor_key_length_in_bytes = 0;
    while ( (opt = getopt(argc, argv, "l:")) != -1 ) {
	switch (opt) {
	    case 'l':
		xor_key_length_in_bytes = atoi(optarg);
		break;
	    default:
		fprintf(stderr, "Usage: %s [-l key_length_in_bytes]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
    }

    time_t now = time(NULL);
    srand(now);

    /* if optional key length was not specified, generate a random key 
     * length between 63 and up to 256 bytes.
     */
    if ( xor_key_length_in_bytes == 0 )
	xor_key_length_in_bytes = 63 + rand()%194;

    printf("/* XOR algo cipher header generated @%lu */\n", now);
    printf("#if !defined(_XOR_ALGO_HEADER)\n");
    printf("#define _XOR_ALGO_HEADER\n\n");
    printf("#define\tXOR_ALGO_OK\t\t\t\t0\n");
    printf("#define\tXOR_ALGO_ERROR_BUFFER_NULL\t\t1\n");
    printf("#define\tXOR_ALGO_ERROR_BUFFER_INVALID_LENGTH\t2\n");
    printf("#define\tXOR_ALGO_ERROR_INTERVAL_START\t\t3\n");
    printf("#define\tXOR_ALGO_ERROR_INTERVAL\t\t4\n\n");
    printf("/* Random generated XOR key, %u bytes long */\n", xor_key_length_in_bytes);
    printf("#define\tXOR_KEY_LENGTH_IN_BYTES\t%u\n", xor_key_length_in_bytes);
    printf("#define\tXOR_KEY_BYTES\t\t{");

    for (unsigned i = 0; i < xor_key_length_in_bytes; i++) {
	printf("0x%02x", ((unsigned char)rand()| i));
	if ( i <= (xor_key_length_in_bytes-2) ) printf(", ");
    }
    printf("}\n\n");
    printf("unsigned int xor_algo(void *, long long int, unsigned int, unsigned int);\n");
    printf("unsigned int xor_algo_key_crc(void);\n");

    printf("#endif /* !defined(_XOR_ALGO_HEADER) */\n");

    exit(EXIT_SUCCESS);
}
