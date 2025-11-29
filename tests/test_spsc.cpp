#include <chrono>
#include <library.h>
#include <ostream>
#include <print>
#include <thread>
int main() {
	spsc_queue<int, 4> q;
	using namespace std::chrono_literals;
	std::jthread p([&q]() {
		int i = 0;
		while (true) {
			std::this_thread::sleep_for(10ms);
			q.enqueue(i++);
		}
	});
	std::jthread c([&q]() {
		int t;
		while (true) {
			std::this_thread::sleep_for(1s);
			if (!q.dequeue(t)) {
				std::print("queue empty\n");
				continue;
			}
			std::print("Dequeued {}\n", t);
		}
	});
	getchar();
}