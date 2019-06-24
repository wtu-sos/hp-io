#include <iostream>
#include <chrono>
#include <memory>
#include <thread>
#include <typeinfo>

#include "ringbuffer.h"
//#include "thread_pool.h"
#include "spsc.h"

using namespace std;

class Temp
{
public:
	Temp() {
		m_count = 0;
	}

	Temp(int count) {
		m_count = count;
		cout << "call construct" << endl;
	}

	    //拷贝构造函数
    Temp(const Temp& C) {
        m_count = C.m_count;
		cout << "call copy construct" << endl;
    }

	int output() {
		return m_count;
	}

	int m_count;
};

//std::function<void()> gen_task(int i) {
//	return std::move([i]() {
//		//while (true) {
//			//std::cout << "********************* this is thread " << std::this_thread::get_id()
//			//	<< ", gen number: " << i << std::endl;
//			std::this_thread::sleep_for(std::chrono::seconds(10));
//			//break;
//		//}
//	});
//}

//void test(std::size_t count, int size) {
//	int i = 0;
//	std::list<std::function<void()>> es;
//	while (i < count) {
//		//std::cout << "gen task " << i << std::endl;
//		es.push_back(gen_task(i));
//		++i;
//		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
//	}
//
//
//	std::shared_ptr<ThreadPool> pool = std::make_shared<ThreadPool>(size);
//	pool->append_events(es);
//	std::this_thread::sleep_for(std::chrono::milliseconds(10));
//	pool->init();
//	std::cout << "begin: " << count << "  size: " << size << endl;
//	auto start = std::chrono::steady_clock::now();
//	while(true) {
//		int s = pool->events_size();
//		//std::cout<< "event size: " << s << endl;
//		if (0 <= s) {
//			break;
//		}
//	}
//
//	auto end = std::chrono::steady_clock::now();
//	std::chrono::duration<double, std::milli> diff = end-start;
//	std::cout << "Time to fill and iterate a vector of "
//			<< size << " ints : " << diff.count() << " s\n";
//}

void writer(spsc_queue<Temp>::PostType p) {
	for (int i = 0; i < 10; ++i) 
	{
		cout << "post i " << i << endl;
		p->post(Temp(i));
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

int main()
{
	//	test(10000000, 4);
	//	test(10000000, 8);

	try {
		shared_ptr<spsc_queue<int>> q (new spsc_queue<int>);
		//q->enqueue(1);
		//q->enqueue(2);
		//int v;
		//bool b = q->dequeue(v);
		//b = q->dequeue(v);
		//q->enqueue(3);
		//q->enqueue(4);
		//b = q->dequeue(v);

		////auto tmp = spsc_queue<int>::split(q);
		//b = q->dequeue(v);
		//b = q->dequeue(v);
		auto tmp = spsc_queue<Temp>::channel();
		spsc_queue<Temp>::PostType p = std::move(std::get<0>(tmp));
		spsc_queue<Temp>::RecvType r = std::move(std::get<1>(tmp));

		p->post(5);

		cout << "tpyes p: " << typeid(p).name() << "  r: " << typeid(r).name() << endl;

		cout << "use count : " << q.use_count() << endl;
		std::thread write (writer, std::move(p));

		std::thread read ([](spsc_queue<Temp>::RecvType rt) {
			Temp output;
			while (true) {
				if (rt->recv(output)) {
					cout << " recved : " << output.output() << endl;
				}
				if (output.output() > 4) {
					break;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}, std::move(r));

		write.join();
		read.join();
    } catch(const std::bad_weak_ptr& e) {
        std::cout << e.what() << '\n';
    }

	Temp *t = new Temp(1);
	Temp *pt = new (t) Temp(3);
	cout << " : " << pt->m_count << endl;

	RingBuffer<Temp, 10> rb;
	rb.push(Temp(1000));

	Temp ret;
	rb.pop(&ret);
	cout << "rb return : " << ret.m_count << endl;
}
