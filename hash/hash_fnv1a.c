#define FNV_BASIS_32 0x811c9dc5

static uint32_t hash_fnv1a_32(const void *data, size_t len, void *state)
{
	uint32_t hash = state ? (uintptr_t)state : FNV_BASIS_32;
	const uint8_t *bytes = (const uint8_t *)data;

	for (size_t i = 0 ; i < len ; ++i) {
		hash ^= bytes[i];
		hash *= 0x01000193;
	}

	return hash;
}
