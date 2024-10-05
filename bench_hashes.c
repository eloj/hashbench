/*
	Simple Hash Benchmark
	Copyright (c) 2024, Eddy L O Jansson. Licensed under The MIT License.

	See https://github.com/eloj/hashbench
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <float.h>
#include <assert.h>

//
// This is the One True prototype for a hash function.
//
typedef uint32_t hash32_t(const void *data, size_t len, void *state);
typedef uint64_t hash64_t(const void *data, size_t len, void *state);

static size_t n_iter = 100;
static int do_dump_buckets = 0;

#include "hash/hash_murmur3.c"
#include "hash/hash_djb2.c"
#include "hash/hash_jenkins.c"
#include "hash/hash_fnv1a.c"
#include "hash/hash_crc32c.c"
#include "hash/hash_siphash.c"

static struct hash_t {
	const char *name;
	hash32_t *func32; // Only one ...
	hash64_t *func64; // ... of these should be defined
	void *state;
} hashes[] = {
	{ "murmur3_32", hash_murmur3_32, NULL, (void*)0x9747b28c },
	{ "djb2",       hash_djb2, NULL, NULL },
	{ "jenkins",    hash_jenkins, NULL, NULL },
	{ "fnv1a_32",   hash_fnv1a_32, NULL, NULL },
	{ "crc32c",     hash_crc32c, NULL, NULL },
	// { "siphash32",  hash_siphash_32, NULL, "randomsecretkey!" },
	{ "siphash64",  NULL, hash_siphash_64, "randomsecretkey!" },
};
static const int NUM_HASHES = sizeof(hashes)/sizeof(hashes[0]);

struct hash_bench_result {
	int hash_idx;
	int n_buckets;
	size_t bytes;
	double tot_timer;
	double min_timer;
	double score;
	uint64_t hashval_acc;
	size_t buckets[];
};

#define START_TIMER() do { clock_gettime(CLOCK_MONOTONIC_RAW, &tp_start); } while (0)
#define STOP_TIMER(timer) do { \
	clock_gettime(CLOCK_MONOTONIC_RAW, &tp_end); \
	struct timespec tp_res; \
	timespec_diff(&tp_start, &tp_end, &tp_res); \
	timer += (tp_res.tv_sec * 1000) + (tp_res.tv_nsec / 1.0e6f); } while (0)

void timespec_diff(struct timespec *start, struct timespec *stop,
				struct timespec *result)
{
	if ((stop->tv_nsec - start->tv_nsec) < 0) {
		result->tv_sec = stop->tv_sec - start->tv_sec - 1;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
	} else {
		result->tv_sec = stop->tv_sec - start->tv_sec;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec;
	}
}

struct hash_bench_result* create_hash_bench_result(int idx, int buckets) {
	struct hash_bench_result *res = malloc(sizeof(struct hash_bench_result) + (buckets * sizeof(res->buckets[0])));
	assert(res);

	res->hash_idx = idx;
	res->n_buckets = buckets;
	res->min_timer = DBL_MAX;
	res->tot_timer = 0;

	return res;
}

void bench_hash(hash32_t func32, hash64_t func64, void *hash_state, const char *words, struct hash_bench_result *res, int n_iter) {
	struct timespec tp_start;
	struct timespec tp_end;
	char *ptr;

	res->bytes = 0;
	res->tot_timer = 0;

	assert(!(func32 && func64));

	for (int i = 0 ; i < n_iter ; ++i) {
		const char *word = words;
		memset(res->buckets, 0, sizeof(res->buckets[0])*res->n_buckets);
		res->hashval_acc = 0;
		double local_timer = 0;

		if (func32) {
			START_TIMER();
			while ((ptr = strchr(word, '\n')) != NULL) {
				size_t len = ptr - word;
				uint32_t hashval = func32(word, len, hash_state);
				res->bytes += len;
				res->hashval_acc += hashval;
				if (res->n_buckets)
					++res->buckets[hashval % res->n_buckets];
				word = ptr + 1;
			}
			STOP_TIMER(local_timer);
		} else if (func64) {
			START_TIMER();
			while ((ptr = strchr(word, '\n')) != NULL) {
				size_t len = ptr - word;
				uint64_t hashval = func64(word, len, hash_state);
				res->bytes += len;
				res->hashval_acc += hashval;
				if (res->n_buckets)
					++res->buckets[hashval % res->n_buckets];
				word = ptr + 1;
			}
			STOP_TIMER(local_timer);
		}

		res->tot_timer += local_timer;
		if (local_timer < res->min_timer) {
			res->min_timer = local_timer;
		}
	}
}

char *read_entire_file(const char *filename, size_t *len) {
	FILE *f = fopen(filename, "rb");

	if (!f)
		return NULL;

	fseek(f, 0, SEEK_END);
	long bytes = ftell(f);
	if (bytes < 0) {
		fclose(f);
		return NULL;
	}
	rewind(f);

	char *buf = malloc(bytes + 1);
	if (!buf) {
		fclose(f);
		return NULL;
	}

	size_t rnum = fread(buf, bytes, 1, f);
	fclose(f);

	if (bytes && rnum != 1) {
		free(buf);
		return NULL;
	}

	buf[bytes] = 0; // always zero-terminate.

	if (len)
		*len = bytes;

	return buf;
}

void print_results_human(struct hash_bench_result *res[]) {

	// Extract winners and losers
	int fastest = -1;
	int slowest = -1;
	int bestscore = -1;
	int worstscore = -1;
	for (int i = 0 ; i < NUM_HASHES ; ++i) {
		if (fastest == -1 || res[i]->min_timer < res[fastest]->min_timer) {
			fastest = i;
		}
		if (slowest == -1 || res[i]->min_timer > res[slowest]->min_timer) {
			slowest = i;
		}
		if (bestscore == -1 || res[i]->score > res[bestscore]->score) {
			bestscore = i;
		}
		if (worstscore == -1 || res[i]->score < res[worstscore]->score) {
			worstscore = i;
		}
	}

	// Output result table
	for (int i = 0 ; i < NUM_HASHES ; ++i) {
		struct hash_bench_result *bres = res[i];
		struct hash_t *hash = &hashes[bres->hash_idx];

		if (hash->func32) {
			printf("%s:\n%.2f/%.2f ms (min/avg), ~%.2f MiB/s (avg) (hash:0x%" PRIx32 "), score=%.2f", hash->name,
					bres->min_timer, bres->tot_timer / n_iter,
					((bres->bytes / bres->tot_timer)*1000)/(1024*1024),
					(uint32_t)bres->hashval_acc,
					bres->score
			);
		} else if (hash->func64) {
			printf("%s:\n%.2f/%.2f ms (min/avg), ~%.2f MiB/s (avg) (hash:0x%" PRIx64 "), score=%.2f", hash->name,
					bres->min_timer, bres->tot_timer / n_iter,
					((bres->bytes / bres->tot_timer)*1000)/(1024*1024),
					bres->hashval_acc,
					bres->score
			);
		}

		if (i == fastest) {
			printf(" best time! ");
		}
		if (i == slowest) {
			printf(" slowest. ");
		}
		if (i == bestscore) {
			printf(" best score! ");
		}
		if (i == worstscore) {
			printf(" worst score. ");
		}

		printf("\n");
	}
}

void dump_buckets(struct hash_bench_result *res[], size_t word_count) {
	for (int i = 0 ; i < NUM_HASHES ; ++i) {
		struct hash_bench_result *bres = res[i];
		struct hash_t *hash = &hashes[bres->hash_idx];

		if (i == 0)
			printf("# %zu words, %d buckets\n", word_count, bres->n_buckets);
		printf("\"%s\",", hash->name);
		for (int j = 0 ; j < bres->n_buckets ; ++j) {
			printf("%zu,", bres->buckets[j]);
		}
		printf("\n");
	}
}

void run_benchmarks(struct hash_bench_result *res[], const char *words, size_t word_count, size_t buckets) {
	for (int i = 0 ; i < NUM_HASHES ; ++i) {
		res[i] = create_hash_bench_result(i, buckets);
		struct hash_bench_result *bres = res[i];

		struct hash_t *hash = &hashes[i];

		bench_hash(hash->func32, hash->func64, hash->state, words, bres, n_iter);

		bres->score = 0;
		// Calculate the sum of differences squared. Closer to zero is better ~= more uniform dist.
		double expected = word_count / buckets;
		for (size_t j=0 ; j < buckets ; ++j) {
			double deviance = expected - bres->buckets[j];
			bres->score += deviance*deviance;
			// printf("[%d]=%zu, dev=%.4f\n", j, bres.buckets[j], deviance);
		}
		if (buckets) {
			bres->score /= buckets; // normalize over buckets
			bres->score = -bres->score;
		}
	}
}

int main(int argc, char *argv[])
{
	n_iter = argc > 3 ? atoi(argv[3]) : 100;
	size_t n_buckets = argc > 2 ? atoi(argv[2]) : 32;
	const char *dict = argc > 1 ? argv[1] : "/usr/share/dict/american-english";

	size_t words_len = 0;
	printf("Benchmarking hashes using '%s' with %zu buckets, %zu iterations.\n", dict, n_buckets, n_iter);

	char *words = read_entire_file(dict, &words_len);
	if (words == NULL) {
		fprintf(stderr, "Error loading dictionary: %m\n");
		return EXIT_FAILURE;
	}

	size_t word_count = 0;
	size_t word_size = 0;

	struct timespec tp_start;
	struct timespec tp_end;

	double timer_iterate = 0;

	// Null-Hypothesis (also cache-warming and word counting)
	char *prev = words;
	char *ptr;
	START_TIMER();
	while ((ptr = strchr(prev, '\n')) != NULL) {
		size_t len = ptr - prev;
		// printf("%s (%zu)\n", prev, len);
		word_size += len;
		prev = ptr + 1;
		++word_count;
	}
	STOP_TIMER(timer_iterate);

	printf("time to iterate: %.2f ms (%zu words, %zu bytes)\n", timer_iterate, word_count, word_size);

	struct hash_bench_result *res[NUM_HASHES] = { };

	run_benchmarks(res, words, word_count, n_buckets);

	print_results_human(res);

	if (do_dump_buckets) {
		dump_buckets(res, word_count);
	}

	for (size_t i = 0 ; i < NUM_HASHES ; ++i) {
		free(res[i]);
	}
	free(words);

	return EXIT_SUCCESS;
}
