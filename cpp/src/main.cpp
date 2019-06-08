#include<iostream>
#include <chrono>
#include <memory>
#include "ringbuffer.h"
#include "thread_pool.h"

using namespace std;

class Temp {
public:
	Temp(int count) {
		m_count = count;
		cout << "call construct" << endl;
	}

	int m_count;
};

std::function<void()> gen_task(int i) {
	return std::move([i]() {
		//while (true) {
			//std::cout << "********************* this is thread " << std::this_thread::get_id() 
			//	<< ", gen number: " << i << std::endl; 
			std::this_thread::sleep_for(std::chrono::seconds(10));
			//break;
		//}
	});
}

void test(std::size_t count, int size) {
	int i = 0;
	std::list<std::function<void()>> es;
	while (i < count) {
		//std::cout << "gen task " << i << std::endl;
		es.push_back(gen_task(i));
		++i;
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}


	std::shared_ptr<ThreadPool> pool = std::make_shared<ThreadPool>(size);
	pool->append_events(es);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	pool->init();
	std::cout << "begin: " << count << "  size: " << size << endl;
	auto start = std::chrono::steady_clock::now();
	while(true) {
		int s = pool->events_size();
		//std::cout<< "event size: " << s << endl;
		if (0 <= s) {
			break;
		}
	}

	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> diff = end-start;
	std::cout << "Time to fill and iterate a vector of " 
			<< size << " ints : " << diff.count() << " s\n";
}

int main() {
	test(10000000, 4);
	test(10000000, 8);
	Temp* t = new Temp(1);
	Temp* pt = new(t) Temp(3);
	cout << " : " << pt->m_count << endl;
}
