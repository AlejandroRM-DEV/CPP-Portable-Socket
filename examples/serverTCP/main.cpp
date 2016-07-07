#include <iostream>
#include <limits>
#include <cstdio>

#include "../../lib/SocketPortable.h"
#include "../../lib/Poll.h"

using namespace std;

#define TAMANO_BUFFER 1000

bool inputError();

int main() {
    int conexion, cliente_len, resRead;
    uint16_t puerto;
    struct sockaddr_in servidor, cliente;
    char buffer[TAMANO_BUFFER + 1];

    do {
        cout << "Dame el puerto (0 - 65535): ";
        cin >> puerto;
    } while ( inputError() );

    SocketPortable sp;

    if( !sp.socket( AF_INET, SOCK_STREAM, 0 ) ) {
        cout << sp.getLastErrorMessage() << endl;
        return -1;
    }
    cout << "INFO: Socket creado" << endl;

    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = htonl( INADDR_ANY );
    servidor.sin_port = htons( puerto );

    if( !sp.bind( ( struct sockaddr * ) &servidor, sizeof( servidor ) ) ) {
        cout << sp.getLastErrorMessage() << endl;
        return -1;
    }

    if( !sp.listen( 5 ) ) {
        cout << sp.getLastErrorMessage() << endl;
        return -1;
    }
    cout << "Escuchando en el puerto: " << puerto << endl;

    cout << "Esperando clientes. . ." << endl;
    Poll poll( 50 );
    poll.add( &sp, POLLIN, 0 );

    do {
        conexion = poll.poll( 50 );
        if ( conexion < 0 ) {
            perror( "Error en POll" );
            break;
        } else if ( conexion > 0 ) {
            if ( poll.checkRevents( 0, POLLIN ) ) {
                cliente_len = sizeof( cliente );
                SocketPortable* newsp = sp.accept( ( struct sockaddr * )&cliente, ( socklen_t* )&cliente_len );
                cout << "Nuevo cliente IP: " << inet_ntoa( cliente.sin_addr ) << " Puerto: " << ntohs ( cliente.sin_port ) << endl;
                poll.add( newsp, POLLIN, 0 );
            }
            for ( size_t i = 1; i < poll.size(); i++ ) {
                if ( poll.checkRevents( i,  POLLIN ) ) {
                    resRead = poll.getSocketPortable( i )->recv( buffer, TAMANO_BUFFER, 0 );
                    if ( resRead < 0 ) {
                        perror( "ERROR FATAL" );
                        cout << "El cliente (fd: " << poll.getSocketPortable( i )->getFD() << ") ha sido cerrado" << endl;
                        poll.getSocketPortable( i )->close();
                        poll.remove( i );
                        i--; // El ultimo esta en la posicion actual ahora, atender desde ahi
                        return -1;
                    } else if ( resRead == 0 ) {
                        cout << "El cliente (fd: " << poll.getSocketPortable( i )->getFD() << ") ha sido cerrado" << endl;
                        poll.getSocketPortable( i )->close();
                        poll.remove( i );
                        i--; // El ultimo esta en la posicion actual ahora, atender desde ahi
                    } else {
                        buffer[resRead] = 0;
                        cout << endl << "fd: " << poll.getSocketPortable( i )->getFD() << " MENSAJE: " << endl << buffer << endl << endl;
                    }
                }
            }
        } 	// "else" No ocurrio ningun evento
    } while ( true );
    sp.close();
    return 0;
}

bool inputError() {
    if ( cin.fail() ) {
        cin.clear();
        cin.ignore( numeric_limits<streamsize>::max(), '\n' );
        return true;
    } else {
        return false;
    }
}
