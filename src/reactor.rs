/* * 
 *
 * use epoll 
 *
 * */
pub trait Reactor {
    /* * 
     *
     * epoll :check wait event count and update events list
     *
     * */
    fn pending(timeout: u32) -> Result<(), &'static str>; 
}

pub struct Token(usize);

impl From<usize> for Token {
    fn from(index: usize) -> Token {
        Token(index)
    }
}

pub struct ReactorImpl {
    // epfd
    // events_list 
    // handlers: Slab<Handler>,
}
