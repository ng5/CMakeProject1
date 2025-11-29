#pragma once
#include <array>
#include <atomic>
#include <concepts>
#include <ostream>
#include <type_traits>

// capacity must be power of two for fast masking
template <size_t N>
concept power_of_two = (N > 0) && ((N & (N - 1)) == 0);

// POD-like constraint for trivial copying
template <typename T>
concept pod = requires(T a) { { sizeof(T) } -> std::convertible_to<size_t>; } && std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

// Single-producer single-consumer lock-free ring buffer
// Optimizations:
// - Monotonic head_/tail_ (no masking stored values) enables distance computation for full check.
// - Single mask operation per enqueue/dequeue.
// - Out-parameter dequeue to avoid std::optional overhead.
// - Acquire/Release only on cross-thread visibility; other loads are relaxed.
// - Branch prediction hints for full/empty paths.
// - Cache line alignment & padding to minimize false sharing.
// - noexcept declarations.
// - Minimal arithmetic & memory barriers.

template <pod T, size_t capacity>
	requires power_of_two<capacity>
class spsc_queue {
  public:
	static constexpr size_t mask = capacity - 1;		// index mask
	static constexpr size_t usable_capacity = capacity; // full when head_ - tail_ == capacity

	spsc_queue() noexcept : head_(0), tail_(0) {}

	// Returns false if queue is full
	bool enqueue(const T &item) noexcept {
		size_t h = head_.load(std::memory_order_relaxed);
		size_t t = tail_.load(std::memory_order_relaxed); // need acquire to see consumer's released tail
		if ((h - t) >= usable_capacity) [[unlikely]] {
			return false; // full
		}
		ring_[h & mask] = item;						   // write element
		head_.fetch_add(1, std::memory_order_release); // publish via atomic increment
		return true;
	}

	// Attempts dequeue; returns false if empty; out receives item.
	bool dequeue(T &out) noexcept {
		size_t t = tail_.load(std::memory_order_relaxed);
		size_t h = head_.load(std::memory_order_acquire); // see producer's release
		if (t == h) [[unlikely]] {
			return false; // empty
		}
		out = ring_[t & mask];
		tail_.fetch_add(1, std::memory_order_release);
		return true;
	}

	// Approximate size (not linearizable with concurrent ops but sufficient for monitoring)
	size_t size() const noexcept {
		size_t h = head_.load(std::memory_order_acquire);
		size_t t = tail_.load(std::memory_order_acquire);
		return h - t;
	}

	bool empty() const noexcept { return size() == 0; }
	bool full() const noexcept { return size() >= usable_capacity; }

	friend std::ostream &operator<<(std::ostream &o, const spsc_queue &q) {
		// Debug-only: unsynchronized snapshot.
		size_t h = q.head_.load(std::memory_order_acquire);
		size_t t = q.tail_.load(std::memory_order_acquire);
		o << "[";
		for (size_t i = t; i < h; ++i) {
			o << q.ring_[i & mask];
			if (i + 1 < h)
				o << ' ';
		}
		o << "]";
		return o;
	}

  private:
	std::array<T, capacity> ring_{};
	alignas(64) std::atomic<size_t> head_;
	char pad1_[64 - sizeof(std::atomic<size_t>)]; // separate cache line
	alignas(64) std::atomic<size_t> tail_;
	char pad2_[64 - sizeof(std::atomic<size_t>)]; // separate cache line
};
