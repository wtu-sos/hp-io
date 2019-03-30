extern crate libc;
extern crate slab;
#[macro_use]
extern crate bitflags;

pub mod epoll;
pub mod selector;
pub mod reactor;
//pub use crate::epoll::*;
