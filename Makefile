CFLAGS = -g -O2 -Wall

all: clean
	# compile and generate a random xor key
	gcc -o $(CURDIR)/xor_algo_header_generator $(CURDIR)/xor_algo_header_generator.c
	$(CURDIR)/xor_algo_header_generator > $(CURDIR)/xor_algo.h

	# binaries for x86_64
	[ -d $(CURDIR)/x86_64 ] || mkdir -p $(CURDIR)/x86_64
	gcc $(CFLAGS) -o $(CURDIR)/x86_64/xor_algo.o -c $(CURDIR)/xor_algo.c
	gcc $(CFLAGS) -o $(CURDIR)/x86_64/xor_file.o -c $(CURDIR)/xor_file.c
	gcc $(CFLAGS) -o $(CURDIR)/x86_64/xor_cipher $(CURDIR)/x86_64/xor_file.o $(CURDIR)/x86_64/xor_algo.o

	# binaries for arm64
	[ -d $(CURDIR)/arm64 ] || mkdir -p $(CURDIR)/arm64
	aarch64-linux-gnu-gcc $(CFLAGS) -o $(CURDIR)/arm64/xor_algo.o -c $(CURDIR)/xor_algo.c
	aarch64-linux-gnu-gcc $(CFLAGS) -o $(CURDIR)/arm64/xor_file.o -c $(CURDIR)/xor_file.c
	aarch64-linux-gnu-gcc $(CFLAGS) -o $(CURDIR)/arm64/xor_cipher $(CURDIR)/arm64/xor_file.o $(CURDIR)/arm64/xor_algo.o


clean:
	rm -rf $(CURDIR)/xor_algo_header_generator $(CURDIR)/xor_algo.h $(CURDIR)/arm64/ $(CURDIR)/x86_64/
