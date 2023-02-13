#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>

#include "xor_algo.h"

int main(int argc, char **argv) {
    int opt;
    extern char *optarg;
    extern int optind, opterr, optopt;

    int xor_key_interval_start = 0;
    int xor_key_interval_length = XOR_KEY_LENGTH_IN_BYTES;
    while ( (opt = getopt(argc, argv, "cdg:hl:s:")) != -1 ) {
        switch (opt) {
	    case 'l':
		xor_key_interval_length = atoi(optarg);
		break;
            case 's':
		xor_key_interval_start = atoi(optarg);
                break;
	    case 'h': /* fall-through */
            default:
                fprintf(stderr, "Usage:\n%s [-h] [-s <interval_start_offset> ] [-l <interval_length>] [-g <segments>] <file_name>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
	fprintf(stderr, "ERROR: expected file name\n");
	exit(EXIT_FAILURE);
    }

    /* sanity */
    if ( (xor_key_interval_start > 0) && (xor_key_interval_length > (XOR_KEY_LENGTH_IN_BYTES-xor_key_interval_start)) ) {
	fprintf(stderr, "WARNING: interval of %u bytes, starting at %u, is larger than our key, adjusting interval length.\n", xor_key_interval_length, xor_key_interval_start);
	xor_key_interval_length = XOR_KEY_LENGTH_IN_BYTES-xor_key_interval_start;
    }

    if ( xor_key_interval_length == 0 ) {
	xor_key_interval_length = XOR_KEY_LENGTH_IN_BYTES-xor_key_interval_start;
    }

    const char *filename = argv[optind];
    struct stat sb;
    if ( stat(filename, &sb) == -1) {
	fprintf(stderr, "ERROR: can not stat() file '%s'\n", filename);
	exit(EXIT_FAILURE);
    }

    if ( S_ISREG(sb.st_mode) == 0 ) {
	fprintf(stderr, "ERROR: '%s' is not a regular file.\n", filename);
	exit(EXIT_FAILURE);
    }

    int fin = open(filename, O_RDONLY);
    if ( fin == -1 ) {
	fprintf(stderr, "ERROR: can not open file '%s' for reading\n", filename);
	exit(EXIT_FAILURE);
    }

    char *tmp_filename = (char *) malloc(strlen(filename)+5);
    memset(tmp_filename, 0, strlen(filename)+5);
    snprintf(tmp_filename, strlen(filename)+5, "%s.tmp", filename);

    int fout = creat(tmp_filename, sb.st_mode);
    if ( fout == -1 ) {
	close(fin);
	fprintf(stderr, "ERROR: can not create temporary output file\n");
	exit(EXIT_FAILURE);
    }

    unsigned char io_buffer[1024];
    int read_bytes;
    unsigned int xor_key_position = 0;
    while ( (read_bytes = read(fin, &io_buffer, sizeof(io_buffer))) > 0 ) {
	if ( xor_algo(&io_buffer, read_bytes, xor_key_interval_start, xor_key_interval_length, &xor_key_position) != XOR_ALGO_OK ) {
	    fprintf(stderr, "ERROR: xor_algo failed\n");
	    close(fin);
	    close(fout);
	    unlink(tmp_filename);
	    exit(EXIT_FAILURE);
	}
	write(fout, &io_buffer, read_bytes);
    }

    close(fin);
    close(fout);

    if ( unlink(filename) < 0 ) {
	fprintf(stderr, "ERROR: can not remove filename '%s'\n", filename);
	unlink(tmp_filename);
	exit(EXIT_FAILURE);
    }

    rename(tmp_filename, filename);

    fprintf(stdout, "XOR key CRC: 0x%016X\n", xor_algo_key_crc());
    exit(EXIT_SUCCESS);
}
