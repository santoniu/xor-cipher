CFLAGS = -g -O2 -Wall

all:
	[ -f $(CURDIR)/xor_algo.h ] || make CROSS_COMPILE=$(CROSS_COMPILE) header
	$(CROSS_COMPILE)gcc $(CFLAGS) -o $(CURDIR)/xor_algo.o -c $(CURDIR)/xor_algo.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -o $(CURDIR)/xor_file.o -c $(CURDIR)/xor_file.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -o $(CURDIR)/xor_cipher $(CURDIR)/xor_file.o $(CURDIR)/xor_algo.o -static
	$(CROSS_COMPILE)strip $(CURDIR)/xor_cipher

clean:
	rm -rf *.o $(CURDIR)/xor_algo_header_generator $(CURDIR)/xor_algo.h xor_cipher

header: clean
	$(CROSS_COMPILE)gcc -o $(CURDIR)/xor_algo_header_generator $(CURDIR)/xor_algo_header_generator.c
	$(CURDIR)/xor_algo_header_generator > $(CURDIR)/xor_algo.h

install: all
	[ -d $(INSTALL_DIR) ] || mkdir -p $(INSTALL_DIR)
	cp $(CURDIR)/xor_cipher $(INSTALL_DIR)/xor_cipher
