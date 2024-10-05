static uint32_t hash_jenkins(const void *data, size_t len, void *state)
{
	uint32_t hash = state ? (uintptr_t)state : 0;
	const uint8_t *bytes = (const uint8_t *)data;

	for (size_t i = 0 ; i < len ; ++i) {
		hash += bytes[i];
		hash += hash << 10;
		hash ^= hash >> 6;
	}

	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;

	return hash;
}
