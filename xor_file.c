#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>

#include "xor_algo.h"

#define	IO_BUFFER_SIZE	1024

int main(int argc, char **argv) {
    int opt;
    extern char *optarg;
    extern int optind, opterr, optopt;

    char *output_filename = NULL;
    unsigned int xor_key_interval_start = 0;
    unsigned int xor_key_interval_length = XOR_KEY_LENGTH_IN_BYTES;
    unsigned char xor_use_header = 0;
    while ( (opt = getopt(argc, argv, "cg:hl:o:s:w")) != -1 ) {
        switch (opt) {
	    case 'c':
		fprintf(stdout, "XOR key CRC: 0x%08X\n", xor_algo_key_crc());
		break;
	    case 'l':
		xor_key_interval_length = atoi(optarg);
		break;
            case 's':
		xor_key_interval_start = atoi(optarg);
                break;
	    case 'o':
		output_filename = optarg;
		break;
	    case 'w':
		xor_use_header = 1;
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
    struct stat sb_in;
    if ( stat(input_filename, &sb_in) == -1) {
	fprintf(stderr, "ERROR: can not stat() file '%s'\n", input_filename);
	exit(EXIT_FAILURE);
    }

    if ( S_ISREG(sb_in.st_mode) == 0 ) {
	fprintf(stderr, "ERROR: '%s' is not a regular file.\n", input_filename);
	exit(EXIT_FAILURE);
    }

    int fd_in = open(input_filename, O_RDONLY);
    if ( fd_in == -1 ) {
	fprintf(stderr, "ERROR: can not open input file '%s' for reading\n", input_filename);
	exit(EXIT_FAILURE);
    }

    if ( output_filename == NULL ) {
	if ( xor_use_header ) {
	    fprintf(stderr, "ERROR: -o <output_filename> is required in conjunction with -w\n");
	    close(fd_in);
	    exit(EXIT_FAILURE);
	}
	output_filename = input_filename;
    }

    int fd_out = open(output_filename, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
    if ( fd_out == -1 ) {
	close(fd_in);
	fprintf(stderr, "ERROR: can not open output file '%s' for writting\n", output_filename);
	exit(EXIT_FAILURE);
    }

    unsigned char io_buffer[IO_BUFFER_SIZE];
    size_t io_buffer_size = IO_BUFFER_SIZE;
    int read_bytes, already_read_bytes = 0;
    unsigned int xor_key_position = 0;

    /*
     * TL;DR:
     * If we use the XOR header, determine if we are crypting or decrypting the input file.
     * - First, check the file size; if it's less than our xor_algo_header, it's either an unecrypted file
     * or it was crypted without a header. In this case, we can not determine if we are encrypting or
     * descrypting the file and we assume that the user knows what he wants to do.
     * - Second, read the XOR header, check the magic. If it matches, we are decypting, else, we are
     * encrypting the input file. If we are encrypting, add xor_algo_header. If we are decrypting, check
     * stored XOR key crc against current XOR key crc.
     */
    if ( xor_use_header ) {
	XOR_ALGO_HEADER xor_algo_header;
	if ( sb_in.st_size > (off_t)sizeof(xor_algo_header) ) {
	    read_bytes = read(fd_in, &io_buffer, sizeof(xor_algo_header));
	    if ( read_bytes != sizeof(xor_algo_header) ) {
		fprintf(stderr, "ERROR: can not read from input filename '%s' %d\n", input_filename, read_bytes);
		close(fd_in);
		close(fd_out);
		exit(EXIT_FAILURE);
	    }
	    memcpy(&xor_algo_header, &io_buffer, sizeof(xor_algo_header));
	    /* Check XOR header magic. */
	    if ( xor_algo_header.xor_algo_magic != XOR_ALGO_HEADER_MAGIC ) {
		/*
		 * No match, we are encrypting.
		 * - move read data from xor_algo_header into io_buffer
		 * - write XOR header into output file.
		 */
		already_read_bytes = sizeof(xor_algo_header);

		xor_algo_header.xor_algo_magic = XOR_ALGO_HEADER_MAGIC;
		xor_algo_header.xor_key_interval_start = xor_key_interval_start;
		xor_algo_header.xor_key_interval_length = xor_key_interval_length;
		xor_algo_header.xor_algo_key_crc = xor_algo_key_crc();
		if ( write(fd_out, &xor_algo_header, sizeof(xor_algo_header)) != sizeof(xor_algo_header) ) {
		    fprintf(stderr, "ERROR: can not write XOR header into output filename '%s'\n", output_filename);
		    close(fd_in);
		    close(fd_out);
		    exit(EXIT_FAILURE);
		}
	    } else {
		/* xor header magic matches, we are decrypting. Check XOR key CRC */
		if ( xor_algo_header.xor_algo_key_crc != xor_algo_key_crc() ) {
		    fprintf(stderr, "ERROR: can not decrypt input filename '%s'. Stored XOR key CRC doesn't match current XOR key CRC. (stored=0x%08X, current=0x%08X)\n", input_filename, xor_algo_header.xor_algo_key_crc, xor_algo_key_crc());
		    close(fd_in);
		    close(fd_out);
		    exit(EXIT_FAILURE);
		}
		/* read stored XOR key interval start and length. */
		xor_key_interval_start = xor_algo_header.xor_key_interval_start;
		xor_key_interval_length = xor_algo_header.xor_key_interval_length;
	    }
	}
    }

    while ( (read_bytes = read(fd_in, &io_buffer[already_read_bytes], io_buffer_size)) > 0 ) {
	if ( xor_algo(&io_buffer, read_bytes+already_read_bytes, xor_key_interval_start, xor_key_interval_length, &xor_key_position) != XOR_ALGO_OK ) {
	    fprintf(stderr, "ERROR: xor_algo failed\n");
	    close(fd_in);
	    close(fd_out);
	    exit(EXIT_FAILURE);
	}
	if ( write(fd_out, &io_buffer, read_bytes+already_read_bytes) < 0 ) {
	    if ( input_filename == output_filename )
		fprintf(stderr, "ERROR: can not write into output filename '%s'. Your original file might have been lost.\n", output_filename);
	    else
		fprintf(stderr, "ERROR: can not write into output filename '%s'\n", output_filename);
	    close(fd_in);
	    close(fd_out);
	    exit(EXIT_FAILURE);
	}
	already_read_bytes = 0 ; /* reset already_read_bytes */
    }

    close(fd_in);
    close(fd_out);

    if ( read_bytes == -1 ) {
	if ( input_filename == output_filename )
	    fprintf(stderr, "ERROR: can not read from input filename '%s', Your original file might have been lost.\n", input_filename);
	else
	    fprintf(stderr, "ERROR: can not read from input filename '%s' %d\n", input_filename, read_bytes);
	exit(EXIT_FAILURE);
    }

    if ( chown(output_filename, sb_in.st_uid, sb_in.st_gid) < 0 )
	fprintf(stderr, "WARNING: can not set ownership on output filename '%s'\n", output_filename);

    if ( chmod(output_filename, sb_in.st_mode) < 0 )
	fprintf(stderr, "WARNING: can not set permissions on output filename '%s'\n", output_filename);

    exit(EXIT_SUCCESS);
}
