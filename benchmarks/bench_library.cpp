#include "library.h"
#include <benchmark/benchmark.h>
namespace bm = benchmark;
static void BM_fn1(benchmark::State& state) {
	int32_t a = std::rand(), b = std::rand(), c = 0;
	for (auto _ : state)
		bm::DoNotOptimize(c = a + b);
}
BENCHMARK(BM_fn1);
BENCHMARK_MAIN();