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
#include "Poll.h"

Poll::Poll( size_t capacity ) {
    length = 0;
    sockets.sp = new SocketPortable*[capacity];
    sockets.poll_fd = new pollfd[capacity];
}
bool Poll::add( SocketPortable* sock, short events, short revents ) {
    if ( length == capacity ) {
        return false;
    } else {
        sockets.sp[length] = sock;
        sockets.poll_fd[length] = {sock->getFD(), events, revents};
        length++;
        return true;
    }
}

size_t Poll::size() {
    return length;
}

SocketPortable* Poll::getSocketPortable( size_t pos ) {
    if( pos >= length ) {
        return nullptr;
    } else {
        return sockets.sp[pos];
    }
}

bool Poll::checkRevents( size_t pos, short flag ) {
    return ( sockets.poll_fd[pos].revents & flag ) == flag;
}

bool Poll::remove( size_t pos ) {
    sockets.sp[pos] = sockets.sp[--length];
    sockets.poll_fd[pos] = sockets.poll_fd[length];
    return true;
}
#ifdef _WIN32
int Poll::poll ( int timeout ) {
    struct timeval tv;
    fd_set read, write, except;
    SOCKET n;
    int ret;
    FD_ZERO ( &read );
    FD_ZERO ( &write );
    FD_ZERO ( &except );
    n = INVALID_SOCKET;
    for ( size_t i = 0; i < length; i++ ) {
        if ( sockets.poll_fd[i].fd == INVALID_SOCKET )
            continue;
        if ( sockets.poll_fd[i].events & POLLIN )
            FD_SET ( sockets.poll_fd[i].fd, &read );
        if ( sockets.poll_fd[i].events & POLLOUT )
            FD_SET ( sockets.poll_fd[i].fd, &write );
        if ( sockets.poll_fd[i].events & POLLERR )
            FD_SET ( sockets.poll_fd[i].fd, &except );
        if ( sockets.poll_fd[i].fd > n )
            n = sockets.poll_fd[i].fd;
        if ( n == INVALID_SOCKET )
            n = sockets.poll_fd[i].fd;
    }
    if ( n == INVALID_SOCKET )
        return ( 0 );
    if ( timeout < 0 )
        ret = select ( n + 1, &read, &write, &except, nullptr );
    else {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = 1000 * ( timeout % 1000 );
        ret = select ( n + 1, &read, &write, &except, &tv );
    }
    for ( size_t i = 0; ret >= 0 && i < length; i++ ) {
        sockets.poll_fd[i].revents = 0;
        if ( FD_ISSET ( sockets.poll_fd[i].fd, &read ) )
            sockets.poll_fd[i].revents |= POLLIN;
        if ( FD_ISSET ( sockets.poll_fd[i].fd, &write ) )
            sockets.poll_fd[i].revents |= POLLOUT;
        if ( FD_ISSET ( sockets.poll_fd[i].fd, &except ) )
            sockets.poll_fd[i].revents |= POLLERR;
    }
    return ( ret );
}
#else
int Poll::poll ( int timeout ) {
    return ::poll( sockets.poll_fd, length, timeout );
}
#endif
