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

    char *output_filename = NULL;
    int xor_key_interval_start = 0;
    int xor_key_interval_length = XOR_KEY_LENGTH_IN_BYTES;
    while ( (opt = getopt(argc, argv, "cg:hl:o:s:")) != -1 ) {
        switch (opt) {
	    case 'c':
		fprintf(stdout, "XOR key CRC: 0x%016X\n", xor_algo_key_crc());
		exit(EXIT_SUCCESS);
	    case 'l':
		xor_key_interval_length = atoi(optarg);
		break;
            case 's':
		xor_key_interval_start = atoi(optarg);
                break;
	    case 'o':
		output_filename = optarg;
		break;
	    case 'h': /* fall-through */
            default:
                fprintf(stderr, "Usage:\n%s [-c] [-g <segments>] [-h] [-l <interval_length>] [-o <output_filename>] [-s <interval_start_offset>] <input_filename>\n", argv[0]);
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

    char *input_filename = argv[optind];
    struct stat sb;
    if ( stat(input_filename, &sb) == -1) {
	fprintf(stderr, "ERROR: can not stat() file '%s'\n", input_filename);
	exit(EXIT_FAILURE);
    }

    if ( S_ISREG(sb.st_mode) == 0 ) {
	fprintf(stderr, "ERROR: '%s' is not a regular file.\n", input_filename);
	exit(EXIT_FAILURE);
    }

    int fd_in = open(input_filename, O_RDONLY);
    if ( fd_in == -1 ) {
	fprintf(stderr, "ERROR: can not open input file '%s' for reading\n", input_filename);
	exit(EXIT_FAILURE);
    }

    if ( output_filename == NULL ) 
	output_filename = input_filename;
    
    int fd_out = open(output_filename, O_CREAT|O_WRONLY);
    if ( fd_out == -1 ) {
	close(fd_in);
	fprintf(stderr, "ERROR: can not open output file '%s' for reading\n", output_filename);
	exit(EXIT_FAILURE);
    }

    unsigned char io_buffer[1024];
    int read_bytes;
    unsigned int xor_key_position = 0;
    while ( (read_bytes = read(fd_in, &io_buffer, sizeof(io_buffer))) > 0 ) {
	if ( xor_algo(&io_buffer, read_bytes, xor_key_interval_start, xor_key_interval_length, &xor_key_position) != XOR_ALGO_OK ) {
	    fprintf(stderr, "ERROR: xor_algo failed\n");
	    close(fd_in);
	    close(fd_out);
	    exit(EXIT_FAILURE);
	}
	if ( write(fd_out, &io_buffer, read_bytes) < 0 ) {
	    if ( input_filename == output_filename )
		fprintf(stderr, "ERROR: can not write into output filename '%s'. Your original file was lost.\n", output_filename);		
	    else
		fprintf(stderr, "ERROR: can not write into output filename '%s'\n", output_filename);
	    close(fd_in);
	    close(fd_out);
	    exit(EXIT_FAILURE);
	}
    }

    close(fd_in);
    close(fd_out);

    if ( read_bytes == -1 ) {
	fprintf(stderr, "ERROR: can not read from input filename '%s'\n", input_filename);
	exit(EXIT_FAILURE);
    }

    if ( chown(output_filename, sb.st_uid, sb.st_gid) < 0 )
	fprintf(stderr, "WARNING: can not set ownership on output filename '%s'\n", output_filename);

    if ( chmod(output_filename, sb.st_mode) < 0 )
	fprintf(stderr, "WARNING: can not set permissions on output filename '%s'\n", output_filename);

    exit(EXIT_SUCCESS);
}
