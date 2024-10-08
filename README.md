
# Simple Hash Benchmark

[![License](https://img.shields.io/github/license/eloj/hashbench)](LICENSE)

A basic, fully self-contained hash benchmark written in C, that you can easily extend and plug new hash functions into.

All code is provided under the [MIT License](LICENSE) unless otherwise specified.

# Built-in Hash Functions

* crc32c (using sse4.2 intrinsics)
* djb2
* fnv1a
* jenkins
* murmur3
* siphash

## Adding new hash functions.

Add a file to the [hash](hash) directory containing your implementation. You need to adhere to the One True Prototype for a hash function:

```c
uint{32,64}_t hash(const void *data, size_t len, void *state);
```

Next include your implementation in the main benchmark file, `bench_hashes.c`, and add an entry to the `hashes` array.

Done.

## Usage

The arguments to `./bench_hashes` are in order:

* A dictionary, whose strings will be hashed line-by-line.
* The number of buckets to simulate.
* The number of iterations.

## Example output

```console
$ make bench
./bench_hashes /usr/share/dict/american-english 32 100
Benchmarking hashes using '/usr/share/dict/american-english' with 32 buckets, 100 iterations.
time to iterate: 0.52 ms (104334 words, 880750 bytes)
murmur3_32:
1.06/1.07 ms (min/avg), ~784.29 MiB/s (avg) (hash:0xd5062c3f), score=-3126.19
djb2:
1.12/1.15 ms (min/avg), ~729.51 MiB/s (avg) (hash:0x4cbe0431), score=-4321.12 worst score.
jenkins:
1.15/1.16 ms (min/avg), ~724.32 MiB/s (avg) (hash:0xe1f3917b), score=-2555.56 best score!
fnv1a_32:
1.16/1.17 ms (min/avg), ~719.44 MiB/s (avg) (hash:0xbb7971b3), score=-3994.62
crc32c:
1.03/1.04 ms (min/avg), ~806.96 MiB/s (avg) (hash:0x475fe08f), score=-2923.44 best time!
siphash64:
1.94/1.96 ms (min/avg), ~429.42 MiB/s (avg) (hash:0x17f6e2be8c4dbd51), score=-3940.50 slowest.
```

The "time to iterate" measures how much time it takes just to parse through the input once, without any hashing.

The "best time!" is determined by the fastest single run, not the average.

The score is an attempt to quantify how 'uniform' the hash function distribution is, by simulating using it to pick hash buckets.
From this actual distribution we calculate the absolute differences squared compared to the ideal. A perfectly uniform distribution
would score 0.

The score is specific to the input set and the number of buckets.
It is mostly useful for a quick reality check. If one of the scores is _wildly_ smaller (which is to say, worse) than the others,
something is up. There is little statistical rigor here, this is not the place for evaluating hash function quality.

The internal flag `do_dump_buckets` can be set to generate a CSV-style table of the bucket distribution for each hash function.

```csv
# 104334 words, 3 buckets
"murmur3_32",34974,34728,34632,
"djb2",38317,32941,33076,
"jenkins",34626,34831,34877,
"fnv1a_32",35021,34516,34797,
"crc32c",34907,34601,34826,
"siphash64",34746,34586,35002,
```
