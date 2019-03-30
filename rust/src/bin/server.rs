use std::net::{TcpListener, TcpStream};
//use std::thread;
use std::io::{Write, Read, Error};
use std::os::unix::io::{AsRawFd, FromRawFd};
use std::time::Duration;

use hp_io::epoll::PollOpt;
use hp_io::selector;

fn main() -> Result<(), Error> {
    let listener = TcpListener::bind("0.0.0.0:23456").expect("could not bind");
    listener.set_nonblocking(true)?;

    let poll = selector::Selector::new()?;

    let listenfd_u64 = listener.as_raw_fd() as u64;

    println!("listenning fd:{} ..... ", listenfd_u64 as i32);
    poll.register(listener.as_raw_fd(), PollOpt::EPOLLIN | PollOpt::EPOLLET, listenfd_u64)?;

    loop {
        let mut buf = Vec::with_capacity(100);
        let nfds = poll.select(&mut buf, listener.as_raw_fd(), Some(Duration::from_millis(500)))?;

        for index in 0 .. nfds {
            if buf[index].data as i32 == listenfd_u64 as i32 {
                println!("accepting event data : {:?}", buf[index]);
                match listener.accept() {
                    Ok((stream, _addr)) => {
                        poll.register(stream.as_raw_fd(), PollOpt::EPOLLIN | PollOpt::EPOLLET, stream.as_raw_fd() as u64)?;
                        std::mem::forget(stream);
                    },
                    Err(e) => {
                        println!("incomming connect failed: {}", e);
                        return Ok(());
                    }
                }
            } else {
                let bits = PollOpt::from_bits_truncate(buf[index].events);
                println!("bits {:?}, index: {}, Event:{:?}", bits, index, buf[index]);
                if bits.contains(PollOpt::EPOLLIN) {
                    let mut data = [0; 1024];

                    let fd = buf[index].data as i32;
                    let mut stream = unsafe {TcpStream::from_raw_fd(fd)};
                    let bytes_read = stream.read(&mut data)?; 
                    if bytes_read > 0 {
                        stream.write(&data[..bytes_read])?;
                    }

                    println!("fd {:?}", fd);

                    if bytes_read == 0 {
                        println!("close fd");
                        std::mem::drop(stream);
                    } else {
                        std::mem::forget(stream);
                    }
                } else if bits.contains(PollOpt::EPOLLOUT) {
                    println!("output event {:?}", bits);
                }
            }
        }
    }

    //Ok(())
}
