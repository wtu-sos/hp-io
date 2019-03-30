#include<atomic>
#include<type_traits>

template<typename T>
class BufferBase {
	private:
		std::atomic<size_t> read_idx;
		std::atomic<size_t> write_idx;
	public:
	
};

template<typename T, size_t MaxSize>
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
