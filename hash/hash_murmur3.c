// https://en.wikipedia.org/wiki/MurmurHash

static inline uint32_t murmur_32_scramble(uint32_t k)
{
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

uint32_t hash_murmur3_32(const void *data, size_t len, void *state)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t hash = state ? (uintptr_t)state : 0;
    uint32_t k;

    for (size_t i = len >> 2; i; --i ) {
        memcpy(&k, bytes, sizeof(uint32_t));
        // k = SwapLE32(k);
        bytes += sizeof(uint32_t);
        hash ^= murmur_32_scramble(k);
        hash = (hash << 13) | (hash >> 19);
        hash = hash * 5 + 0xe6546b64;
    }

    // Read the rest.
	if (len & 3) {
	    k = 0;
	    for (size_t i = len & 3; i--;) {
	        k <<= 8;
	        k |= bytes[i];
	    }
		hash ^= murmur_32_scramble(k);
	}

    // Finalize.
    hash ^= len;
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;

    return hash;
}
