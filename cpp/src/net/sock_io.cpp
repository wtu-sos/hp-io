#include "sys/socket.h"
#include "unistd.h"

#include "sock_io.hpp"

SockIo::SockIo() {

}

SockIo::~SockIo() {}

size_t SockIo::recv(void* buf, size_t n, int flags) const {
    return ::recv(this->get_fd(), buf, n, flags);
}

size_t SockIo::recv(void* buf, size_t n) const {
    return ::read(this->get_fd(), buf, n);
}

size_t SockIo::send(const void* buf, size_t n, int flags) const {
    return ::send(this->get_fd(), buf, n, flags);
}

size_t SockIo::send(const void* buf, size_t n) const {
    return ::write(this->get_fd(), buf, n);
}