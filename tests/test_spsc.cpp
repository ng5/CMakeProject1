#include <chrono>
#include <iostream>
#include <library.h>
#include <ostream>
#include <thread>
int main() {
	spsc_queue<int, 4> q;
	using namespace std::chrono_literals;
	std::jthread p([&q]() {
		int i = 0;
		while (true) {
			auto x = q.enqueue(i++);
			if (!x)
				std::cout << "failed to enqueue " << i << "\n";
			std::this_thread::sleep_for(1s);
			std::cout << q << "\n";
		}
	});
	std::jthread c([&q]() {
		while (true) {
			std::this_thread::sleep_for(1s);
			auto item = q.dequeue();
			if (!item) {
				std::cout << "queue empty\n";
				continue;
			}

			std::cout << "Dequeued: " << *item << '\n';
		}
	});
	getchar();
}