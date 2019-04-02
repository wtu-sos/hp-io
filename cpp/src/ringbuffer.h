#include<atomic>
#include<type_traits>

using namespace std;

template<typename T>
class BufferBase {
	private:
		std::atomic<std::size_t> read_idx;
		std::atomic<std::size_t> write_idx;

		BufferBase(BufferBase const& ) = delete;
		BufferBase& operator = (BufferBase const &) = delete;

	protected:
		BufferBase(): read_idx(0), write_idx(0) {}

		static size_t read_available(std::size_t write, std::size_t read, size_t max_size) {
			if (write >= read) {
				return write - read;
			}

			const size_t ret = write + max_size - read;
			return ret;
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
