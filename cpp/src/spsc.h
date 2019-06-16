#include <atomic>
#include <memory>
#include <iostream>

template<typename T> 
T load_consume(T const* addr) 
{ 
    // hardware fence is implicit on c11 
    T v = *const_cast<T const volatile*>(addr); 
    std::atomic_thread_fence(std::memory_order_acquire); // compiler fence 
    return v; 
} 

// store with 'release' memory ordering 
template<typename T> 
void store_release(T* addr, T v) 
{ 
    // hardware fence is implicit c11
    std::atomic_thread_fence(std::memory_order_release); // compiler fence 
    *const_cast<T volatile*>(addr) = v; 
} 

template<typename T>
class poster;

template<typename T>
class recver;

template<typename T>
class spsc_queue : std::enable_shared_from_this< spsc_queue<T> > {
public:

    typedef std::unique_ptr<poster<T>> PostType;
    typedef std::unique_ptr<recver<T>> RecvType;

    spsc_queue() /* : is_drop(ATOMIC_FLAG_INIT) */ {
        std::cout << "spsc construct " << std::endl;
        node* n = new node;
        n->next_ = nullptr;
        tail_ = head_ = first_ = tail_copy_ = n;
    }

    ~spsc_queue() {
        node* n = first_;
        do {
            node* next = n->next_;
            delete n;
            n = next;
        } while(n);
    }

    void enqueue(T v) {
        //std::cout << "trace : enqueue" << std::endl;
        if (is_drop.load()) {
            return;
        }

        node* n = alloc_node();
        n->next_ = nullptr;
        n->value_ = v;
        store_release(&head_->next_, n);
        head_ = n;
    }

    bool dequeue(T& v) {
        //std::cout << "trace : dequeue" << std::endl;
        if (load_consume(&tail_->next_)) {
            v = std::move(tail_->next_->value_);
            store_release(&tail_, tail_->next_);
            return true;
        } else {
            return false;
        }
    }

    void release_endpoint() {
        is_drop.store(true);
    }

    static std::tuple<PostType, RecvType> split(std::shared_ptr<spsc_queue<T>> sp) {
        std::cout << "split use count: " << sp.use_count() << std::endl;
        //auto weak = this->weak_from_this();
        //auto sp = std::shared_ptr<spsc_queue<T>>(weak);
        //std::shared_ptr<spsc_queue<T>> sp = this->std::enable_shared_from_this<spsc_queue<T>>::shared_from_this();
        return std::make_tuple(std::make_unique<poster<T>>(sp), std::make_unique<recver<T>>(sp));
    }

    static std::tuple<PostType, RecvType> channel() {
		std::shared_ptr<spsc_queue<T>> q (new spsc_queue<T>);
        return spsc_queue<T>::split(q);
    }

private:
    struct node
    {
        node* next_;
        T value_;
    };

    node*  tail_;  // tail of the queue    
    // delimiter between consumer part and producer part,
    // so that they situated on different cache line
    char cache_line_pad_[64];

    // producer part
    node* head_; // hard of the queue
    node* first_; // last unused node (tail of node cache)
    node* tail_copy_; // helper (pointer somewhere between first_ 
    //and tail_)

    // is this spsc should destruct
    std::atomic<bool> is_drop = ATOMIC_FLAG_INIT;

    node* alloc_node() {
        // first tries to allocate node from internal node cache
        // if attempt fails, allocates node via ::operator new()
        if (first_ != tail_copy_) {
            node* n = first_;
            first_ = n->next_;
            return n;
        }

        tail_copy_ = load_consume(&tail_);
        if (first_ != tail_copy_) {
            node* n = first_;
            first_ = n->next_;
            return n;
        }

        node* n = new node; 
        return n; 
    }

    spsc_queue(spsc_queue const &) = delete;
    spsc_queue& operator = (spsc_queue const&) = delete;
};

template<typename T>
class poster {
public:
    poster(std::shared_ptr<spsc_queue<T>> queue): queue_(queue) {}
    ~poster() {
        std::cout << "release poster: " << std::endl;
        queue_->release_endpoint();
    }

    void post(T v) {
        queue_->enqueue(v);
    }
private:
    std::shared_ptr<spsc_queue<T>> queue_;
};

template<typename T>
class recver {
public:
    recver(std::shared_ptr<spsc_queue<T>> queue): queue_(queue) {}
    ~recver() {
        std::cout << "release recver: " << std::endl;
        queue_->release_endpoint();
    }

    bool recv(T& v) {
        return queue_->dequeue(v);
    }
private:
    std::shared_ptr<spsc_queue<T>> queue_;
};
