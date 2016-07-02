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
#include "SocketPortable.h"

#ifdef _WIN32
int inet_pton( int af, const char *src, void *dst ) {
    struct sockaddr_storage ss;
    int size = sizeof( ss );
    char src_copy[INET6_ADDRSTRLEN + 1];

    ZeroMemory( &ss, sizeof( ss ) );
    /* stupid non-const API */
    strncpy ( src_copy, src, INET6_ADDRSTRLEN + 1 );
    src_copy[INET6_ADDRSTRLEN] = 0;

    if ( WSAStringToAddress( src_copy, af, nullptr, ( struct sockaddr * )&ss, &size ) == 0 ) {
        switch( af ) {
        case AF_INET:
            *( struct in_addr * )dst = ( ( struct sockaddr_in * )&ss )->sin_addr;
            return 1;
        case AF_INET6:
            *( struct in6_addr * )dst = ( ( struct sockaddr_in6 * )&ss )->sin6_addr;
            return 1;
        }
    }
    return 0;
}

const char *inet_ntop( int af, const void *src, char *dst, socklen_t size ) {
    struct sockaddr_storage ss;
    unsigned long s = size;

    ZeroMemory( &ss, sizeof( ss ) );
    ss.ss_family = af;

    switch( af ) {
    case AF_INET:
        ( ( struct sockaddr_in * )&ss )->sin_addr = *( struct in_addr * )src;
        break;
    case AF_INET6:
        ( ( struct sockaddr_in6 * )&ss )->sin6_addr = *( struct in6_addr * )src;
        break;
    default:
        return NULL;
    }
    /* cannot direclty use &size because of strict aliasing rules */
    return ( WSAAddressToString( ( struct sockaddr * )&ss, sizeof( ss ), nullptr, dst, &s ) == 0 ) ?
           dst : NULL;
}

SocketPortable::SocketPortable() {
    WSADATA wsaData;
    WSAStartup( MAKEWORD( 2, 0 ), &wsaData );
    sockfd = INVALID_SOCKET;
}
SocketPortable::~SocketPortable() {
    close();
    WSACleanup();
}
string SocketPortable::getLastErrorMessage() {
    DWORD errorMessageID = ::GetLastError();
    if( errorMessageID == 0 )
        return ""; //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                  FORMAT_MESSAGE_IGNORE_INSERTS,
                                  nullptr, errorMessageID, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPSTR )&messageBuffer, 0,
                                  nullptr );

    string message( messageBuffer, size );
    LocalFree( messageBuffer );

    return message;
}
bool SocketPortable::connect( const struct sockaddr *addr, int addrlen ) {
    if( ::connect( sockfd, addr, addrlen ) != 0 ) {
        close();
        return false;
    }
    return true;
}
bool SocketPortable::socket( int domain, int type, int protocol ) {
    sockfd = ::socket( domain, type, protocol );
    if( sockfd == INVALID_SOCKET ) {
        close();
        return false;
    }
    return true;
}
bool SocketPortable::setNonBlock() {
    u_long iMode = 1;
    if( ioctlsocket( sockfd, FIONBIO, &iMode ) == NO_ERROR ) {
        return true;
    }
    return false;
}
bool SocketPortable::nonBlockNoError() {
    return WSAGetLastError() == WSAEWOULDBLOCK;
}
void SocketPortable::close() {
    ::closesocket( sockfd );
}
int SocketPortable::recv( char *buf, int len, int flags ) {
    return ::recv( sockfd, buf, len, flags );
}
int SocketPortable::send( const char *buf, int len, int flags ) {
    return ::send( sockfd, buf, len, flags );
}
bool SocketPortable::setsockopt( int level, int optname, const void *optval, int optlen ) {
    return ::setsockopt( sockfd, level, optname, ( char* )optval, optlen ) >= 0 ;
}
bool SocketPortable::bind( const struct sockaddr *addr, int addrlen ) {
    return ::bind( sockfd, addr, addrlen ) >= 0 ;
}
int SocketPortable::recvfrom( char *buf, int len, int flags, struct sockaddr *src_addr,
                              int *addrlen ) {
    return ::recvfrom( sockfd, buf, len, flags, src_addr, addrlen );
}
#else
SocketPortable::SocketPortable() {}
SocketPortable::~SocketPortable() {
    ::close( sockfd );
}
string SocketPortable::getLastErrorMessage() {
    return strerror( errno );
}
bool SocketPortable::connect( const struct sockaddr *addr, socklen_t addrlen ) {
    if( ::connect( sockfd, addr, addrlen ) < 0 ) {
        ::close( sockfd );
        return false;
    }
    return true;
}
bool SocketPortable::socket( int domain, int type, int protocol ) {
    sockfd = ::socket( domain, type, protocol );
    if( sockfd < 0 ) {
        ::close( sockfd );
        return false;
    }
    return true;
}
bool SocketPortable::setNonBlock() {
    int flags = fcntl( sockfd, F_GETFL );
    flags = flags | O_NONBLOCK;
    if( fcntl( sockfd, F_SETFL, flags ) < 0 ) {
        return false;
    }
    return true;
}
bool SocketPortable::nonBlockNoError() {
    return errno == EAGAIN || errno == EWOULDBLOCK;
}
void SocketPortable::close() {
    ::close( sockfd );
}
ssize_t SocketPortable::recv( void *buf, size_t len, int flags ) {
    return ::recv( sockfd, buf, len, flags );
}
ssize_t SocketPortable::send( const void *buf, size_t len, int flags ) {
    return ::send( sockfd, buf, len, flags );
}
bool SocketPortable::setsockopt( int level, int optname, const void *optval, socklen_t optlen ) {
    return ::setsockopt( sockfd, level, optname, optval, optlen ) >= 0 ;
}
bool SocketPortable::bind( const struct sockaddr *addr, socklen_t addrlen ) {
    return ::bind( sockfd, addr, addrlen ) >= 0 ;
}
ssize_t SocketPortable::recvfrom( void *buf, size_t len, int flags, struct sockaddr *src_addr,
                                  socklen_t *addrlen ) {
    return ::recvfrom( sockfd, buf, len, flags, src_addr, addrlen );
}
#endif
bool SocketPortable::connect( const char *node, const char *service,
                              const struct addrinfo *hints ) {
    struct addrinfo *res, *rp;

    if ( getaddrinfo( node, service, hints, &res ) != 0 ) {
        freeaddrinfo( res );
        return false;
    }

    for ( rp = res; rp != nullptr; rp = rp->ai_next ) {
        if ( socket( rp->ai_family, rp->ai_socktype, rp->ai_protocol ) ) {
            if ( connect( rp->ai_addr, rp->ai_addrlen ) ) {
                break;
            }
        }
        close();
    }

    if ( rp == nullptr ) {
        freeaddrinfo( res );
        return false;
    }
    freeaddrinfo( res );
    return true;
}
