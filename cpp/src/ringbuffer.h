#include<atomic>
#include<type_traits>

 #define likely(x) __builtin_expect(!!(x), 1)
 #define unlikely(x) __builtin_expect(!!(x), 0)

template<typename T>
class BufferBase {
	private:
		std::atomic<std::size_t> read_idx;
		std::atomic<std::size_t> write_idx;

		BufferBase(BufferBase const& ) = delete;
		BufferBase& operator = (BufferBase const &) = delete;

	protected:
		BufferBase(): read_idx(0), write_idx(0) {}

		/**
		 *
		 *  #define likely(x) __builtin_expect(!!(x), 1)
		 *	#define unlikely(x) __builtin_expect(!!(x), 0)
		 *  这里的__built_expect()函数是gcc(version >= 2.96)的内建函数,提供给程序员使用的，目的是将"分支转移"的信息提供给编译器，这样编译器对代码进行优化，以减少指令跳转带来的性能下降。
		 *  __buildin_expect((x), 1)表示x的值为真的可能性更大.
		 *  __buildin_expect((x), 0)表示x的值为假的可能性更大.
		 *
		 * */
		static std::size_t next_index(std::size_t index, std::size_t max_size) {
			std::size_t next = index + 1;
			if (unlikely(next >= max_size)) {
				next -= max_size;
			}

			return next;
		}

		static std::size_t read_available(std::size_t write, std::size_t read, std::size_t max_size) {
			if (write >= read) {
				return write - read;
			}

			const std::size_t ret = write + max_size - read;
			return ret;
		}

		static std::size_t write_available(std::size_t write, std::size_t read, std::size_t max_size) {
			if (read >= write) {
				return read - write - 1;
			}

			const std::size_t ret = max_size - write + read - 1;
			return ret;
		}

		/**
		 * read_available中write使用acquire,因为该函数仅用于读线程，write可能在写线程中被修改，此处需要获取
		 * 该操作之前最新的数据，
		 * write_available同理
		 **/
		std::size_t read_available(std::size_t max_size) {
			const std::size_t write = write_idx.load(std::memory_order_acquire);
			const std::size_t read = read_idx.load(std::memory_order_relaxed);
			return read_available(write, read, max_size);
		}

		std::size_t write_available(std::size_t max_size) {
			const std::size_t read = read_idx.load(std::memory_order_acquire);
			const std::size_t write = write_idx.load(std::memory_order_relaxed);
			return write_available(write, read, max_size);
		}

		bool empty(std::size_t write_idx, std::size_t read_idx) {
			return write_idx == read_idx;
		}

	public:
};

template<typename T, std::size_t MaxSize>
class RingBuffer {
private:
	 static const std::size_t max_size = MaxSize + 1;

	 typedef typename std::aligned_storage<max_size * sizeof(T), alignof(T)>::type storage_type;

	 storage_type storage;
public:
	 T* data() {
		 return reinterpret_cast<T*>(&storage);
	 }
};
