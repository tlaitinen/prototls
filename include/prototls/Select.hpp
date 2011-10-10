#ifndef _prototls_select_hpp_
#define _prototls_select_hpp_
#include "prototls/Socket.hpp"
namespace prototls {
    class Select {
        fd_set rfds;
        Socket::Fd max;
    public:
        Select() {
            reset();
        }
        void reset() {
            FD_ZERO(&rfds);
            max = 0;
        }
        void input(Socket::Fd fd) {
            FD_SET(fd, &rfds);
            if (fd > max)
                max = fd;
        }
        bool canRead(Socket::Fd fd) const {
            return FD_ISSET(fd, &rfds);
        }
        int select(int msecs) {
            struct timeval tv;
            tv.tv_usec = (msecs % 1000) * 1000;
            tv.tv_sec = msecs / 1000;
            return ::select(max+1, &rfds, NULL, NULL, &tv);
        }




    };
}
#endif
