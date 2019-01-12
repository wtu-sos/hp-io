use std::net::{TcpListener, TcpStream};
use std::thread;
use std::io::{Write, Read, Error};
use std::os::unix::io::{AsRawFd};

use hp_io::epoll::{self, Event, Events, ControlOperates};

fn handle_client(mut stream: TcpStream) -> Result<(), Error> {
    println!("connect from : {}", stream.peer_addr()?);
    let mut buf = [0; 1024];
    loop {
        let bytes_read = stream.read(&mut buf)?; 
        if bytes_read == 0 {
            return Ok(());
        }

        stream.write(&buf[..bytes_read])?;
    }
}

fn main() -> Result<(), Error> {
    let listener = TcpListener::bind("0.0.0.0:8888").expect("could not bind");
    println!("listenning ..... ");

    let epfd = epoll::epoll_create(true)?;

    let listen_ev = Event::new(Events::EPOLLIN | Events::EPOLLET, 0u64);
    epoll::epoll_ctl(epfd, ControlOperates::EpollCtrAdd, listener.as_raw_fd(), listen_ev)?;

    loop {
        let mut buf = vec![Event::new(Default::default(), 0u64); 50];
        let nfds = epoll::epoll_wait(epfd, 500, &mut buf)?;

        for stream in listener.incoming() {
            match stream {
                Ok(stream) => {
                    thread::spawn(move || {
                        handle_client(stream).unwrap_or_else(|e| println!("{:#?}", e));
                    });
                },
                Err(e) => {
                    println!("incomming connect failed: {}", e);
                }
            }
        }
    }

    //Ok(())
}
