#pragma once
#include <array>
#include <atomic>
#include <concepts>
#include <ostream>
#include <type_traits>

template <size_t N>
concept power_of_two = (N > 0) && ((N & (N - 1)) == 0);

template <typename T>
concept pod = requires(T a) { { sizeof(T) } -> std::convertible_to<size_t>; } && std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

template <pod T, size_t capacity>
	requires power_of_two<capacity>
class spsc_queue {
  public:
	static constexpr size_t mask = capacity - 1;
	static constexpr size_t usable_capacity = capacity;
	bool enqueue(const T &item) noexcept {
		size_t h = head_.load(std::memory_order_relaxed);
		if (size_t t = tail_.load(std::memory_order_acquire); (h - t) >= usable_capacity) {
			return false;
		}
		ring_[h & mask] = item;
		head_.store(h + 1, std::memory_order_release);
		return true;
	}
	bool dequeue(T &out) noexcept {
		size_t t = tail_.load(std::memory_order_relaxed);
		if (size_t h = head_.load(std::memory_order_acquire); t == h) {
			return false;
		}
		out = ring_[t & mask];
		tail_.store(t + 1, std::memory_order_release);
		return true;
	}

  private:
	std::array<T, capacity> ring_{};
	alignas(64) std::atomic<size_t> head_{0};
	alignas(64) std::atomic<size_t> tail_{0};
};
