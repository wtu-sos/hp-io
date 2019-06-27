#pragma once

#include<atomic>
#include<algorithm>
#include<type_traits>
#include<iostream>

#include "def.h"

 #define likely(x) __builtin_expect(!!(x), 1)
 #define unlikely(x) __builtin_expect(!!(x), 0)

template<typename T>
class BufferBase {
	private:
		std::atomic<std::size_t> m_read_idx;

		char padding1[CacheLineSize];

		std::atomic<std::size_t> m_write_idx;

		BufferBase(BufferBase const& ) = delete;
		BufferBase& operator = (BufferBase const &) = delete;

	protected:
		BufferBase(): m_read_idx(0), m_write_idx(0) {}

		/**
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
			const std::size_t write = m_write_idx.load(std::memory_order_acquire);
			const std::size_t read = m_read_idx.load(std::memory_order_relaxed);
			return read_available(write, read, max_size);
		}

		std::size_t write_available(std::size_t max_size) {
			const std::size_t read = m_read_idx.load(std::memory_order_acquire);
			const std::size_t write = m_write_idx.load(std::memory_order_relaxed);
			return write_available(write, read, max_size);
		}

		bool empty(std::size_t m_write_idx, std::size_t m_read_idx) {
			return m_write_idx == m_read_idx;
		}

		bool push(T const& t, T * buffer, std::size_t max_size) {
			const std::size_t write = m_write_idx.load(std::memory_order_relaxed);
			const std::size_t next = this->next_index(write, max_size);
			if (next == m_read_idx.load(std::memory_order_acquire)) {
				return false;
			}

			std::cout << "avail: " << next << " , read: " << m_read_idx.load(std::memory_order_acquire) << std::endl;
			new (buffer+write) T(t);
			m_write_idx.store(next, std::memory_order_release);

			return true;
		}

		std::size_t pop(T* output_buffer, std::size_t output_count, T* buffer, std::size_t max_size) {
			const std::size_t read = m_read_idx.load(std::memory_order_relaxed);
			const std::size_t write = m_write_idx.load(std::memory_order_acquire);
			std::size_t avail = read_available(write, read, max_size);	

			if (0 == avail) {
				return 0;
			}

			std::cout << "avail: " << avail << " read: " << read << "; write: " <<write<< std::endl;
			output_count = std::min(avail, output_count);
			std::size_t new_read_index = read + output_count;
			// todo : copy and delete 
			if (read + output_count > max_size) {
				const std::size_t seg1 = max_size - read;
				copy_and_delete(buffer+read, buffer+max_size, output_buffer);

				const std::size_t seg2 = output_count - seg1;
				copy_and_delete(buffer, buffer+seg2, output_buffer+seg1);

				new_read_index -= max_size;
			} else {
				copy_and_delete(buffer+read, buffer+output_count, output_buffer);
				if (new_read_index == max_size) {
					new_read_index = 0;
				}
			}

			m_read_idx.store(new_read_index, std::memory_order_release);

			return output_count;
		}

	public:
	    template<class OutputIterator>
		OutputIterator copy_and_delete( T * first, T * last, OutputIterator out )
		{
			if (std::is_trivially_destructible<T>::value) {
				return std::copy(first, last, out); // will use memcpy if possible
			} else {
				for (; first != last; ++first, ++out) {
					*out = *first;
					first->~T();
				}
				return out;
			}
		}
};

template<typename T, std::size_t MaxSize>
class RingBuffer: BufferBase<T> {
private:
	 static const std::size_t max_size = MaxSize + 1;

	 typedef typename std::aligned_storage<max_size * sizeof(T), alignof(T)>::type storage_type[max_size];

	 storage_type m_storage;
public:

    T * data()
    {
        return reinterpret_cast<T*>(m_storage);
    }

    const T * data() const
    {
        return reinterpret_cast<const T*>(m_storage);
    }

    std::size_t max_number_of_elements() const
    {
        return max_size;
    }

	bool push(T const & t)
    {
        return BufferBase<T>::push(t, data(), max_size);
    }

	std::size_t pop(T * ret)
    {
        return BufferBase<T>::pop(ret, 1, data(), max_size);
    }
};
