#include <atomic>
#include <barrier>
#include <benchmark/benchmark.h>
#include <library.h>
#include <thread>

static void BM_Enqueue_Empty(benchmark::State &state) {
	spsc_queue<int, 1024> q;
	for (auto _ : state) {
		int v = 123;
		bool ok = q.enqueue(v);
		benchmark::DoNotOptimize(ok);
	}
}

static void BM_Dequeue_Full(benchmark::State &state) {
	spsc_queue<int, 1024> q;
	// Pre-fill
	for (int i = 0; i < 1023; ++i)
		q.enqueue(i);
	for (auto _ : state) {
		auto val = q.dequeue();
		benchmark::DoNotOptimize(val);
		// Refill one slot to keep it full
		q.enqueue(42);
	}
}

// Throughput with real SPSC producer/consumer
static void BM_SPSC_Throughput(benchmark::State &state) {
	spsc_queue<int, 1024> q;
	std::atomic<bool> run{true};
	std::atomic<size_t> produced{0}, consumed{0};

	std::thread producer([&] {
		while (run.load(std::memory_order_relaxed)) {
			int v = static_cast<int>(produced.load(std::memory_order_relaxed));
			if (q.enqueue(v))
				produced.fetch_add(1, std::memory_order_relaxed);
		}
	});
	std::thread consumer([&] {
		while (run.load(std::memory_order_relaxed)) {
			auto v = q.dequeue();
			if (v)
				consumed.fetch_add(1, std::memory_order_relaxed);
		}
	});

	for (auto _ : state) {
		// Optionally sample counts
		benchmark::DoNotOptimize(produced.load(std::memory_order_relaxed));
		benchmark::DoNotOptimize(consumed.load(std::memory_order_relaxed));
	}

	run.store(false, std::memory_order_relaxed);
	producer.join();
	consumer.join();

	state.counters["produced"] = produced.load();
	state.counters["consumed"] = consumed.load();
	// Report throughput (ops per second)
	state.counters["prod_per_sec"] =
		benchmark::Counter(produced.load(), benchmark::Counter::kIsRate);
	state.counters["cons_per_sec"] =
		benchmark::Counter(consumed.load(), benchmark::Counter::kIsRate);
}

BENCHMARK(BM_Enqueue_Empty);
BENCHMARK(BM_Dequeue_Full);
BENCHMARK(BM_SPSC_Throughput);

BENCHMARK_MAIN();