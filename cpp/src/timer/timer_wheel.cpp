#include "timer_wheel.h"

TimerWheel::TimerWheel() {
    for (std::size_t i = 0; i < 4; ++i) {
        tvs[i] = TimerVec(TVN_SIZE);
    }
    tv_root = TimerVec(TVR_SIZE);
}

void TimerWheel::add_timer(TimerNode* node) {
    // todo: lock here
    this->inner_add_timer(node);
}

void TimerWheel::inner_add_timer(TimerNode* node) {
    unsigned long expires = node->expires;
    unsigned long idx = expires - this->jiffies_;

    if (idx < TVR_SIZE) {   // idx < 256
        int i = expires & TVR_MASK;
        tv_root.group[i].push_back(node);
    } else if (idx < TVR_SIZE + TVN_SIZE) {
        int i = (expires >> TVR_BITS) & TVN_MASK;
        tvs[0].group[i].push_back(node);
    } else if (idx < (TVR_SIZE + 2 * TVN_SIZE)) {
        int i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
        tvs[1].group[i].push_back(node);
    } else if (idx < (TVR_SIZE + 3 * TVN_SIZE)) {
        int i = (expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
        tvs[2].group[i].push_back(node);
    } else if (static_cast<long>(idx) < 0) {
        tv_root.group[tv_root.index].push_back(node);
    } else if (idx <= 0xffffffff) {
        int i = (expires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
        tvs[3].group[i].push_back(node);
    } else {
        // If the timeout is larger than MAX_TVAL on 64-bit
        // architectures then we use the maximum timeout
        node->expires = MAX_TVAL;
        int i = (MAX_TVAL >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
        tvs[3].group[i].push_back(node);
    }
}

void TimerWheel::cascade(size_t wheel, size_t index) {
    std::list<TimerNode*> prepare;
    this->tvs[wheel].group[index].swap(prepare);
    for (auto tn : prepare) {
        this->inner_add_timer(tn);
    }
}