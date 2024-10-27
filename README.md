
# Simple Hash Benchmark

[![License](https://img.shields.io/github/license/eloj/hashbench)](LICENSE)

A _basic_, fully self-contained hash benchmark written in C, that you can easily extend and plug new hash functions into.

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
* The number of rounds/word.
* The number of total iterations.

Increasing the number of rounds, i.e. repeatedly hashing the same data n times,
puts more weight onto the computational complexity of the hash function, since the data will be cache hot.

Decreasing the rounds puts more weight on memory bandwidth. Setting it to 1 simulates streaming the
whole input through the hash function, almost as if you're hashing one large input.

Iterations are how many times to repeat the whole benchmark run. More repeats means more stable timings.

## Example output

```console
$ make bench
./bench_hashes /usr/share/dict/american-english 32 4 25
Benchmarking hashes using '/usr/share/dict/american-english' with 32 buckets, 4 rounds/word, 25 iterations.
time to iterate: 0.65 ms (104334 words, 880750 bytes)
murmur3_32:
0.51/0.55 ms (min/avg), ~1524.04 MiB/s (avg) (hash:0x5418b0fc), score=-3126.19
djb2:
0.55/0.55 ms (min/avg), ~1517.42 MiB/s (avg) (hash:0x32f810c4), score=-4321.12 worst score.
jenkins:
0.75/0.76 ms (min/avg), ~1101.59 MiB/s (avg) (hash:0x87ce45ec), score=-2555.56 best score!
fnv1a_32:
0.59/0.60 ms (min/avg), ~1405.95 MiB/s (avg) (hash:0xede5c6cc), score=-3994.62
crc32c:
0.46/0.47 ms (min/avg), ~1794.01 MiB/s (avg) (hash:0x1d7f823c), score=-2923.44 best time!
siphash64:
1.60/1.62 ms (min/avg), ~520.06 MiB/s (avg) (hash:0x5fdb8afa3136f544), score=-3940.50 slowest.
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

## See Also

* If you want bigger/more: [smhasher](https://github.com/rurban/smhasher) and/or [smhasher3](https://gitlab.com/fwojcik/smhasher3).
