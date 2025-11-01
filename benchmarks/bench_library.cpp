#include "library.h"
#include <benchmark/benchmark.h>

static void BM_fn1(benchmark::State& state) {
	for (auto _ : state) {
		fn1(10, 20);
	}
}
BENCHMARK(BM_fn1);
BENCHMARK_MAIN();