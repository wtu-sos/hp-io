#ifndef SOCK_H
#define SOCK_H

class Sock {
protected:
    Sock(){} 

    Sock(int type, 
         int protocol_family,
         int protocol = 0, 
         int reuse_addr = 0);

    ~Sock();

public:
    // wrapper c system call
    int set_option(int level, 
                   int option, 
                   void* optval,
                   int optlen) const;

    int get_option(int level, 
                   int option, 
                   void* optval,
                   int optlen) const;


    /**
     * @brief  close the socket
     * 
     * @return 0 success 
     */
    int close();

    /**
     * @brief Get the local addr object
     * 
     * @return int 0 if successful else -1
     */
    int get_local_addr() const;

    /**
     * @brief Get the remote addr object
     * 
     * @param addr 
     * @return int 0 if successful else -1
     */
    int get_remote_addr(void* addr) const;

    /**
     * @brief  wrapper c socket system call
     * 
     * @param type 
     * @param protocal_family 
     * @param protocol 
     * @param reuse_addr 
     * @return int 0 if successful else -1
     */
    int open(int type,
             int protocal_family,
             int protocol,
             int reuse_addr);

    int get_fd() const {
        return this->m_sock;
    }

    void set_fd(int fd) {
        this->m_sock = fd;
    }

private:
    int m_sock;
};

#endif