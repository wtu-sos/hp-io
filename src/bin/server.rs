use std::net::{TcpListener, TcpStream};
use std::thread;
use std::io::{Write, Read, Error};

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

fn main() {
    let listener = TcpListener::bind("0.0.0.0:8888").expect("could not bind");
    println!("listenning ..... ");

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
