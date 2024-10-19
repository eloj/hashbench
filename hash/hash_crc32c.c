uint32_t hash_crc32c(const void *data, size_t len, void *state) {
	const uint8_t *src = data;
	uint32_t crc = state ? (uintptr_t)state : 0;

	// PERF: Not the optimal way to do this if arch has multiple ports for crc instr, but
	// this strikes a balance between simplicity and perf. given we assume short input.
	for (size_t i = 0 ; i < len >> 2 ; ++i) {
		/* Alignment doesn't seem to matter in practice, but leave it to the compiler */
		uint32_t buf;
		memcpy(&buf, src, sizeof(buf));
		crc = __builtin_ia32_crc32si(crc, buf);
		src += 4;
	}

	switch (len & 3) {
		case 3:
			crc = __builtin_ia32_crc32qi(crc, *src++);
			/* fallthrough */
		case 2:
			crc = __builtin_ia32_crc32qi(crc, *src++);
			/* fallthrough */
		case 1:
			crc = __builtin_ia32_crc32qi(crc, *src);
	}

	return crc;
}
