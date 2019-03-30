use std::os::unix::io::RawFd;
use std::sync::atomic::{Ordering, ATOMIC_USIZE_INIT, AtomicUsize};
use std::time::Duration;
use std::io;
use std::cmp;

use crate::epoll;

static NEXT_ID: AtomicUsize = ATOMIC_USIZE_INIT;

pub struct Selector {
    id: usize,
    epfd: RawFd,
}

impl Selector {
    pub fn new() -> Result<Self, io::Error> {
        let epfd = epoll::epoll_create(true)?;
        
        let id = NEXT_ID.fetch_add(1, Ordering::Relaxed) + 1;

        Ok(Self {
            id,
            epfd
        })
    }

    pub fn id(&self) -> usize {
        self.id
    }

    pub fn select(&self, evts: &mut Vec<epoll::Event>, _except: RawFd, timeout: Option<Duration>) -> Result<usize, io::Error> {
        //println!("timeout : {:?} ", timeout.map(|to| to.as_millis() as i32).unwrap_or(-1));
        let timeout = timeout.map(|to| cmp::min(to.as_millis() as u64, i32::max_value() as u64) as i32).unwrap_or(-1);
        evts.clear();
        unsafe { evts.set_len(evts.capacity()); }
        epoll::epoll_wait(self.epfd, timeout, evts)
    }

    pub fn register(&self, fd: RawFd, opts: epoll::PollOpt, token: u64) -> Result<(), io::Error>{
        let ev = epoll::Event::new(opts, token);
        epoll::epoll_ctl(self.epfd, epoll::ControlOperates::EpollCtrAdd, fd, ev)
    }

    pub fn unregister(&self, fd: RawFd) -> Result<(), io::Error>{
        let ev = epoll::Event::default();
        epoll::epoll_ctl(self.epfd, epoll::ControlOperates::EpollCtrDel, fd, ev)
    }
}
