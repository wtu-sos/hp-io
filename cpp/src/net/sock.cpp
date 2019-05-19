#include "unistd.h"
#include "sys/socket.h"
#include "errno.h"
#include "sys/types.h"
#include "sock.hpp"

Sock::Sock(
    int type, 
    int protocol_family, 
    int protocol, 
    int reuse_addr
) {
    int r = this->open(type, protocol_family, protocol, reuse_addr);
    if (r == -1) {
        // todo: add error handle
    }
}

Sock::~Sock() {

}

int Sock::open(
    int type,
    int protocal_family,
    int protocol,
    int reuse_addr
) {
    int one = 1;
    int fd = ::socket(protocal_family, type, protocol);
    this->set_fd(fd);

    if (fd == -1) {
        return -1;
    }

    if (protocal_family != PF_UNIX && reuse_addr) {
        if (this->set_option(SOL_SOCKET, SO_REUSEADDR, 
                             &one, sizeof(one)) == -1) {
            this->close();
            return -1; 
        }
    }

    return 0;
}

int Sock::close() {
    int result = 0;
    int fd = this->get_fd();
    if (fd != -1) {
        result == ::close(fd);
    }

    return result;
}

int Sock::set_option(
    int level, 
    int option, 
    void* optval,
    int optlen
) const {
    int r = ::setsockopt(this->get_fd(), level, option, optval, optlen);
    if (-1 == r) {
        errno = ENOTSUP;
    }

    return r;
}

int Sock::get_option(
    int level, 
    int option, 
    void* optval,
    int optlen
) const {
    return ::setsockopt(this->get_fd(), 
                level, option, optval, optlen);
}
