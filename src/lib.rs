extern crate libc;
#[macro_use]
extern crate bitflags;

pub mod epoll;
pub use crate::epoll::*;