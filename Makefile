MISCFLAGS=-fstack-protector -fcf-protection -fvisibility=hidden
DEVFLAGS=-ggdb -DDEBUG -D_FORTIFY_SOURCE=3 -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function

ifdef OPTIMIZED
# On mingw, -static avoids dep on libssp-0.dll when built with -fstack-protector
	MISCFLAGS+=-DNDEBUG -Werror -static
else
	MISCFLAGS+=$(DEVFLAGS)
endif

CFLAGS=-std=gnu11 -O3 -Wall -Wextra -march=native -msse4.2 $(MISCFLAGS)

.PHONY: bench clean

bench_hashes: bench_hashes.c hash/hash_*.c
	$(CC) $(CFLAGS) $< -o $@

bench: bench_hashes
	./bench_hashes /usr/share/dict/american-english 32 4 25

clean:
	rm -f bench_hashes
