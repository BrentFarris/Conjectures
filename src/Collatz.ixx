export module Collatz;

import <vector>;
import <thread>;
import <iostream>;

struct ThreadInfo {
	size_t number;		// The number that the thread is currently on
	size_t skip;		// The number to skip by to get the next number to check
	size_t limit;		// The upper bound of the range to check
	size_t readNumber;	// Read only by the main thread to know what to print
	bool done;			// Lets the main thread know when this thread is done
};

static void process_number(ThreadInfo* info) {
	while (info->number < info->limit) {
		size_t n = info->number;
		while (n != 1) {
			if ((n & 0x01) == 0)	// If the number is even
				n /= 2;
			else					// If the number is odd
				n = 3 * n + 1;
		}
		info->number += info->skip;
	}
	info->done = true;
}

export void Collatz(size_t range) {
	size_t threadCount = std::thread::hardware_concurrency();
	std::vector<std::thread> threads;
	threads.reserve(threadCount);
	std::vector<ThreadInfo> states(threadCount, { 0, threadCount, range, 0, false });
	// Setup the initial states for the threads
	for (size_t i = 0; i < threadCount; ++i) {
		states[i].number = i + 1;
		states[i].readNumber = i + 1;
		threads.push_back(std::thread(process_number, &states[i]));
	}
	int doneCount = 0;
	while (doneCount != threadCount) {
		doneCount = 0;
		for (auto& s : states) {
			// Count up to see if all of the threads have completed yet
			doneCount += s.done;
			// Main thread should print out stuff so we know things are happening
			size_t readNum = s.number;
			size_t last = s.readNumber;
			if (last < readNum) {
				s.readNumber = readNum;
				std::cout << "All numbers verified from " << last << " to " << readNum << " skipping every " << threadCount << std::endl;
				s.readNumber = readNum;
			}
		}
	}
	for (auto& t : threads)
		if (t.joinable())
			t.join();
}
