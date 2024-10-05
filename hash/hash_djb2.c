static uint32_t hash_djb2(const void *data, size_t len, void *state)
{
	uint32_t hash = state ? (uintptr_t)state : 5381;
	const uint8_t *bytes = (const uint8_t *)data;

	for (size_t i = 0 ; i < len ; ++i){
		hash = ((hash << 5) + hash) + bytes[i];
	}

	return hash;
}
