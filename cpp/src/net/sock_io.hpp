#ifndef SOCKET_IO_H
#define SOCKET_IO_H

#include "sock.hpp"
#include "stddef.h"

class SockIo : public Sock {
public:
    SockIo();
    ~SockIo();

    /**
     * @brief wrapper system call recv, it is equal to read while flags = 0
     * 
     * @param buf 
     * @param n 
     * @param flags 
     * @return size_t 
     */
    size_t recv(void* buf, size_t n, int flags) const;

    /**
     * @brief wrapper system call read
     * 
     * @param buf 
     * @param n 
     * @return size_t 
     */
    size_t recv(void* buf, size_t n) const;

    /**
     * @brief wrapper system call send
     * 
     * @param buf 
     * @param n 
     * @param flags 
     * @return size_t 
     */
    size_t send(const void* buf, size_t n, int flags) const;

    /**
     * @brief wrapper system call write
     * 
     * @param buf 
     * @param n 
     * @return size_t 
     */
    size_t send(const void* buf, size_t n) const;
};

#endif