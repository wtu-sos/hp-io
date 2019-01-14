use std::io::{self, Error};
use std::os::unix::io::RawFd;

use libc;

pub enum ControlOperates {
    EpollCtrAdd = libc::EPOLL_CTL_ADD as isize,
    EpollCtrMod = libc::EPOLL_CTL_MOD as isize,
    EpollCtrDel = libc::EPOLL_CTL_DEL as isize,
}

bitflags! {
    #[derive(Default)]
    pub struct Events: u32 {
        const EPOLLET = libc::EPOLLET as u32;
        const EPOLLIN = libc::EPOLLIN as u32;
        const EPOLLERR = libc::EPOLLERR as u32;
        const EPOLLHUP = libc::EPOLLHUP as u32;
        const EPOLLOUT = libc::EPOLLOUT as u32;
        const EPOLLPRI = libc::EPOLLPRI as u32;
        const EPOLLRDHUP = libc::EPOLLRDHUP as u32;
        const EPOLLWAKEUP = libc::EPOLLWAKEUP as u32;
        const EPOLLONESHOT = libc::EPOLLONESHOT as u32;
    }
}

fn cvt(result: libc::c_int) -> io::Result<libc::c_int> {
    if result < 0 {
        Err(Error::last_os_error())
    } else {
        Ok(result)
    }
}

#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct Event {
    pub events : u32,
    pub data: u64
}

#[allow(unused)]
impl Event {
    pub fn new(events: Events, data: u64) -> Self {
        Event { events: events.bits(), data: data }
    }
}

#[allow(unused)]
pub fn epoll_create(cloexe: bool)  -> io::Result<RawFd> {
    let flags = if cloexe { libc::EPOLL_CLOEXEC } else { 0 };
    unsafe { cvt(libc::epoll_create1(flags)) }
}

#[allow(unused)]
pub fn epoll_ctl(epfd: RawFd, op: ControlOperates, fd: RawFd, mut event: Event) -> io::Result<()> {
    let e = &mut event as *mut _ as *mut libc::epoll_event;
    let result = unsafe { cvt(libc::epoll_ctl(epfd, op as i32, fd, e)) };
    if let Err(error) = result {
        return Err(error);
    }

    Ok(())
}

#[allow(unused)]
pub fn epoll_wait(epfd: RawFd, timeout: i32, buf: &mut Vec<Event>) -> io::Result<usize> {
    let timeout = if timeout < -1 { -1 } else { timeout };
    let events_count = unsafe { 
        cvt(libc::epoll_wait(epfd, buf.as_mut_ptr() as *mut libc::epoll_event,
                                buf.len() as i32,
                                timeout))?
    };

    Ok(events_count as usize)
}

