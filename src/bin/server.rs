use std::collections::HashMap;
use std::net::{Shutdown ,TcpListener, TcpStream};
use std::thread;
use std::io::{Write, Read, Error};
use std::os::unix::io::{AsRawFd, FromRawFd};
use std::sync::Mutex;

use state::Storage;
use hp_io::epoll::{self, Event, Events, ControlOperates};

static GLOBAL_CONN : Storage<Mutex<HashMap<i32, TcpStream>>> = Storage::new();

fn handle_client(stream: TcpStream, epfd: i32) -> Result<(), Error> {
    println!("connect from : {}", stream.peer_addr()?);
    let conn_ev = Event::new(Events::EPOLLIN | Events::EPOLLET, stream.as_raw_fd() as u64);
    epoll::epoll_ctl(epfd, ControlOperates::EpollCtrAdd, stream.as_raw_fd(), conn_ev)?;

    {
        let mut conns = GLOBAL_CONN.get().lock().unwrap();
        conns.insert(stream.as_raw_fd() as i32, stream);
        println!("global connects size : {}", conns.len());
    }
    
    Ok(())
}

fn main() -> Result<(), Error> {
    GLOBAL_CONN.set(Mutex::new(HashMap::new()));
    let listener = TcpListener::bind("0.0.0.0:23456").expect("could not bind");
    listener.set_nonblocking(true);
    println!("listenning ..... ");

    let epfd = epoll::epoll_create(true)?;

    let listenfd_u64 = listener.as_raw_fd() as u64;

    let listen_ev = Event::new(Events::EPOLLIN | Events::EPOLLET, listener.as_raw_fd() as u64);
    epoll::epoll_ctl(epfd, ControlOperates::EpollCtrAdd, listener.as_raw_fd(), listen_ev)?;

    loop {
        let mut buf = vec![Event::new(Default::default(), 0u64); 50];
        let nfds = epoll::epoll_wait(epfd, 500, &mut buf)?;

        for index in 0 .. nfds {
            if buf[index].data == listenfd_u64 {
                println!("accepting event data : {:?}", buf[index]);
                match listener.accept() {
                    Ok((stream, _addr)) => {
                        let epfd_cp = epfd;
                        thread::spawn(move || {
                            handle_client(stream, epfd_cp).unwrap_or_else(|e| println!("{:#?}", e));
                        });
                    },
                    Err(e) => {
                        println!("incomming connect failed: {}", e);
                    }
                }
            } else {
                let bits = Events::from_bits_truncate(buf[index].events);
                println!("bits {:?}", bits);
                if bits.contains(Events::EPOLLIN) {
                    let mut data = [0; 1024];
                    let mut bytes_read;
                    let fd = buf[index].data as i32;
                    {
                        let mut conns = GLOBAL_CONN.get().lock().unwrap();
                        let stream = conns.get_mut(&fd).unwrap();
                        bytes_read = stream.read(&mut data)?; 
                        if bytes_read > 0 {
                            stream.write(&data[..bytes_read])?;
                        }
                    }

                    if bytes_read == 0 {
                        println!("close fd");
                        let mut conns = GLOBAL_CONN.get().lock().unwrap();
                        let stream = conns.remove(&fd).unwrap();
                        stream.shutdown(Shutdown::Both);
                    } 
                } else if bits.contains(Events::EPOLLOUT) {
                    println!("output event {:?}", bits);
                }
            }
        }
    }

    //Ok(())
}
