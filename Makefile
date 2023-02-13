CFLAGS = -g -O2 -Wall
CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip

all: clean
	[ -f $(CURDIR)/xor_algo.h ] || make CROSS_COMPILE=$(CROSS_COMPILE) header
	$(CC) $(CFLAGS) -o $(CURDIR)/xor_algo.o -c $(CURDIR)/xor_algo.c
	$(CC) $(CFLAGS) -o $(CURDIR)/xor_file.o -c $(CURDIR)/xor_file.c
	$(CC) $(CFLAGS) -o $(CURDIR)/xor_cipher $(CURDIR)/xor_file.o $(CURDIR)/xor_algo.o -static
	$(STRIP) $(CURDIR)/xor_cipher

clean:
	rm -rf *.o 

realclean: clean
	rm -rf $(CURDIR)/xor_algo_header_generator $(CURDIR)/xor_algo.h xor_cipher

header:
	$(CC) -o $(CURDIR)/xor_algo_header_generator $(CURDIR)/xor_algo_header_generator.c
	$(CURDIR)/xor_algo_header_generator > $(CURDIR)/xor_algo.h

install: all
	[ -d $(INSTALL_DIR) ] || mkdir -p $(INSTALL_DIR)
	cp $(CURDIR)/xor_cipher $(INSTALL_DIR)/xor_cipher
