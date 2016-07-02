#include <iostream>
#include <cstdint>

#include "../../lib/SocketPortable.h"

#define BUFFER_SIZE 256

using namespace std;

int main() {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in addr;
    struct ip_mreq mreq;
    int totalBytes;
    uint16_t port;
    string group;
    SocketPortable sp;
    socklen_t addrlen;
    socklen_t optval = 1;

    if( !sp.socket( AF_INET, SOCK_DGRAM, 0 ) ) {
        cout << sp.getLastErrorMessage() << endl;
        return -1;
    }

    if( !sp.setsockopt( SOL_SOCKET, SO_REUSEADDR, &optval, sizeof( optval ) ) ) {
        cout << sp.getLastErrorMessage() << endl;
        return -1;
    }

    port = 12345;
    group = "224.0.1.100";

    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl( INADDR_ANY );
    addr.sin_port = htons( port );

    if( !sp.bind( ( struct sockaddr * ) &addr, sizeof( addr ) ) ) {
        cout << sp.getLastErrorMessage() << endl;
        return -1;
    }

    inet_pton(AF_INET, group.c_str(), &mreq.imr_multiaddr);
    mreq.imr_interface.s_addr = htonl( INADDR_ANY );
    if ( !sp.setsockopt( IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof( mreq ) ) < 0 ) {
        cout << sp.getLastErrorMessage() << endl;
        return -1;
    }
    cout << "Multicast group: " << group << " Port: " << port << endl;
    cout << "Waiting messages. . ." << endl;
    while ( true ) {
        memset( &addr, 0, sizeof( addr ) );
        addrlen = sizeof( addr );
        totalBytes = sp.recvfrom( buffer, BUFFER_SIZE, 0, ( struct sockaddr * ) &addr, &addrlen );
        if ( totalBytes < 0 ) {
            cout << sp.getLastErrorMessage() << endl;
            return -1;
        }
        buffer[totalBytes] = 0;
        cout << buffer << endl;
    }
    return 0;
}
