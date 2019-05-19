#include<iostream>
#include "ringbuffer.h"

using namespace std;

class Temp {
public:
	Temp(int count) {
		m_count = count;
		cout << "call construct" << endl;
	}

	int m_count;
};

int main() {
	Temp* t = new Temp(1);
	Temp* pt = new(t) Temp(3);
	cout << " : " << pt->m_count << endl;
}
