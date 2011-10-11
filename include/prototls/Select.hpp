/** prototls - Portable asynchronous client/server communications C++ library 
   
     See LICENSE for copyright information.
*/
#ifndef _prototls_select_hpp_
#define _prototls_select_hpp_
#include "prototls/Socket.hpp"
namespace prototls {
    /** wrapper for select system call to perform asynchronous IO
      on multiple sockets */
    class Select {
        /** file descriptor set */
        fd_set rfds;

        /** the highest socket descriptor 'rfds' */
        Socket::Fd max;
    public:
        /** performs reset */
        Select() {
            reset();
        }

        /** initializes the file descriptor set */
        void reset() {
            FD_ZERO(&rfds);
            max = 0;
        }

        /** marks the socket to be watched for reading */
        void input(Socket::Fd fd) {
            FD_SET(fd, &rfds);
            if (fd > max)
                max = fd;
        }

        /** \return true if the socket has data waiting to be read */
        bool canRead(Socket::Fd fd) const {
            return FD_ISSET(fd, &rfds);
        }

        /** waits 'msecs' milliseconds for the sockets in the 
          socket descriptor set for input
         \return -1 on error, 0 if no data, > 0 number of sockets 
         that have data waiting */
        int select(int msecs) {
            struct timeval tv;
            tv.tv_usec = (msecs % 1000) * 1000;
            tv.tv_sec = msecs / 1000;
            return ::select(max+1, &rfds, NULL, NULL, &tv);
        }




    };
}
#endif
