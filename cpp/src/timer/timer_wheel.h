#include <vector>
#include <list>
#include "timer_base.h"

/* 
 * Event timer code 
 */ 
enum
{
    WHEEL_BUCKETS = 4,

    TVN_BITS = 6,                   // time vector level shift bits
    TVR_BITS = 8,                   // timer vector shift bits
    TVN_SIZE = (1 << TVN_BITS),     // wheel slots of level vector
    TVR_SIZE = (1 << TVR_BITS),     // wheel slots of vector
    TVN_MASK = (TVN_SIZE - 1),      //
    TVR_MASK = (TVR_SIZE - 1),      //

    MAX_TVAL = ((uint64_t)((1ULL << (TVR_BITS + 4 * TVN_BITS)) - 1)),
};

class TimerNode {
public:
    TimerNode() {
        id = 0;
        expires = 0;
        data = 0;
    }

    unsigned int id;
    unsigned long expires;
    unsigned long data;
    TimerCallback callback;
};

typedef std::vector<std::list<TimerNode*>> TimerGroup;
class TimerVec {
public:
    TimerVec(size_t size): index(0) {
        group = std::vector(size, std::list<TimerNode*>());
    }

    int index;
    TimerGroup group; 
};

class TimerWheel {
public:
    TimerWheel();
    void add_timer(TimerNode* node);

protected:
    void inner_add_timer(TimerNode* node);
    void cascade(size_t wheel, size_t index); 

private:
    int size_ = 0;
    int64_t current_ = 0;
    int64_t jiffies_ = 0;   // 全局时间戳
    TimerVec tvs[4];
    TimerVec tv_root;
};