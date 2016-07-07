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
#ifndef SOCKETPORTABLE_H_INCLUDED
#define SOCKETPORTABLE_H_INCLUDED

#include <iostream>

using namespace std;

#ifdef _WIN32
/**
*   To compile in MinGW use -lws2_32 linker option
*   To compile with MSVC++ use  #pragma comment(lib,"Ws2_32.lib")
**/
#define _WIN32_WINNT 0x501
#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET sp_type;

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <cstring>

typedef int sp_type;

#endif

class SocketPortable {
private:
    sp_type sockfd;

public:
    SocketPortable();
    ~SocketPortable();
    string getLastErrorMessage();
    bool socket( int domain, int type, int protocol );
    bool setNonBlock();
    bool nonBlockNoError() ;
    void close();
    bool setsockopt( int level, int optname, const void *optval, socklen_t optlen );
#ifdef _WIN32
    int recv( char *buf, int len, int flags );
    int send( const char *buf, int len, int flags );
    int recvfrom( char *buf, int len, int flags, struct sockaddr *src_addr, socklen_t *addrlen ) ;
#else
    ssize_t recv( void *buf, size_t len, int flags );
    ssize_t send( const void *buf, size_t len, int flags );
    ssize_t recvfrom( void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen );
#endif
    /* Same implementation */
    bool connect( const char *node, const char *service, const struct addrinfo *hints );
    bool connect( const struct sockaddr *addr, socklen_t addrlen );
    bool listen( int backlog );
    bool bind( const struct sockaddr *addr, socklen_t addrlen );
    SocketPortable* accept( struct sockaddr *addr, socklen_t *addrlen );
    sp_type getFD();
};

#ifdef __WIN32
int inet_pton( int af, const char *src, void *dst ) ;
const char *inet_ntop( int af, const void *src, char *dst, socklen_t size );
#endif // __WIN32

#endif // SOCKETPORTABLE_H_INCLUDED
