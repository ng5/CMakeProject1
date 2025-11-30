// spsc_bench.cpp
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

#include "library.h" // your header

using clock_type = std::chrono::high_resolution_clock;

template <std::size_t Capacity>
void run_bench(std::size_t iterations) {
	spsc_queue<std::uint64_t, Capacity> q;

	std::atomic start_flag{false};
	std::atomic done_flag{false};

	// Consumer thread
	std::thread consumer([&] {
		// Spin until producer arms the start flag
		while (!start_flag.load(std::memory_order_acquire)) {
			// busy-wait
		}

		for (std::size_t i = 0; i < iterations; ++i) {
			std::uint64_t value;
			// Spin until we manage to dequeue
			while (!q.dequeue(value)) {
				// busy-wait
			}
		}

		done_flag.store(true, std::memory_order_release);
	});

	// Small barrier to help OS schedule both threads
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	auto t0 = clock_type::now();
	start_flag.store(true, std::memory_order_release);

	// Producer loop
	for (std::size_t i = 0; i < iterations; ++i) {
		auto v = i;
		// Spin until we manage to enqueue
		while (!q.enqueue(v)) {
			// busy-wait
		}
	}

	// Wait until consumer has drained all items
	while (!done_flag.load(std::memory_order_acquire)) {
		// busy-wait
	}
	auto t1 = clock_type::now();

	consumer.join();

	auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
	double seconds = static_cast<double>(ns) / 1e9;
	double msgs_per_sec = static_cast<double>(iterations) / seconds;
	double ns_per_msg = static_cast<double>(ns) / iterations;

	std::cout << "Capacity: " << Capacity << "\n";
	std::cout << "Iterations: " << iterations << "\n";
	std::cout << "Elapsed: " << ns << " ns (" << seconds << " s)\n";
	std::cout << "Throughput: " << msgs_per_sec / 1e6 << " M msg/s\n";
	std::cout << "Latency: " << ns_per_msg << " ns/msg (amortized)\n";
	std::cout << "----------------------------------------\n";
}

int main(int argc, char **argv) {
	std::size_t iterations = 10'000'000; // default

	if (argc >= 2) {
		iterations = std::stoull(argv[1]);
	}

	// Run with a few capacities (all power-of-two to satisfy your constraint)
	run_bench<1024>(iterations);
	run_bench<16 * 1024>(iterations);
	run_bench<256 * 1024>(iterations);

	return 0;
}
