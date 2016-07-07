/***************************************************************************************************
The MIT License (MIT)

Copyright (c) 2016 Alejandro Ramírez Muñoz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
***************************************************************************************************/
#ifndef POLL_H_INCLUDED
#define POLL_H_INCLUDED

#include "SocketPortable.h"

#ifdef _WIN32
#define POLLIN 0x0001
#define POLLPRI 0x0002
#define POLLOUT 0x0004
#define POLLERR 0x0008
#define POLLHUP 0x0010
#define POLLNVAL 0x0020

struct pollfd {
    SOCKET   fd;
    short events;
    short revents;
};
#else
#include <poll.h>
#endif

class Poll {
private:
    struct Data {
        SocketPortable** sp;
        struct pollfd* poll_fd;
    };

    Data sockets;
    size_t capacity;
    size_t length;
public:
    Poll( size_t capacity );
    bool add( SocketPortable* sock, short events, short revents );
    size_t size();
    SocketPortable* getSocketPortable( size_t pos );
    bool checkRevents( size_t pos, short flag );
    bool remove( size_t pos );
    /*
        In Windows is backed by select (), take a look to the function to see the limitations
    */
    int poll ( int timeout );
};

#endif // POLL_H_INCLUDED
