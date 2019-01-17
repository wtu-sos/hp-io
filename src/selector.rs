use std::os::unix::io::RawFd;
use std::sync::Ordering;

use crate::epoll;

static NEXT_ID: AtomicUsize = ATOMIC_USIZE_INIT;

pub struct Selector {
    id: usize,
    epfd: RawFd,
}

impl Selector {
    pub fn new() -> Result<Self> {
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

    pub fn select(&self, evts: &mut Vec<Events>, except: RawFd, timeout: Option<Duration>) -> Result<usize> {
        let timeout = timeout.map(|to| cmp::min(millis(to), i32::MAX as u64) as i32).unwrap_or(-1);
        evts.clear();
        epoll::epoll_wait(self.epfd, timeout, evts)
    }

    pub fn register(&self, fd: RawFd, opts: epoll::PollOpt, token: u64) -> Result<()>{
        let mut ev = epoll::Event::new(opt, token);
        epoll::epoll_ctl(self.epfd, ControlOperates::EPOLL_CTL_MOD, fd, mut ev)
    }

    pub fn unregister(&self, fd: RawFd) -> Result<()>{
        let mut ev = epoll::Event::default();
        epoll::epoll_ctl(self.epfd, ControlOperates::EPOLL_CTL_DEL, fd, mut ev)
    }
}
