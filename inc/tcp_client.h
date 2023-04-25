#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    char n1;
    char n2;
    char n3;
    char n4;
} test_buf_struct;

test_buf_struct recv_test_buf = {0};
test_buf_struct send_test_buf = {.n1='a', .n2='b', .n3='c', .n4='d'};

int init_tcp_client(SOCKET* ConnectSocket)
{
    const char* ip_address  = "127.0.0.1" ;
    const char* port        = "27015"     ;
    WSADATA wsaData;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    const char *sendbuf = "this is a test";
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(ip_address, port, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        *ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (*ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( *ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(*ConnectSocket);
            *ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(result);
    if (*ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    DWORD OptVal = 3000;
    int   OptLen = sizeof (DWORD);
    iResult = setsockopt(*ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (char *) &OptVal, OptLen);
    if (iResult == SOCKET_ERROR) {
        printf("setsockopt for SO_RCVTIMEO failed with error: %d\n", WSAGetLastError());
        return 1;
    } else
        printf("Set SO_KEEPALIVE: ON\n");
    
    return 0;
}

void shutdown_tcp_client(SOCKET* ConnectSocket)
{
    int iResult;
    // shutdown the connection since no more data will be sent
    iResult = shutdown(*ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(*ConnectSocket);
        WSACleanup();
    }

    // cleanup
    closesocket(*ConnectSocket);
    WSACleanup();
}

extern bool tcp_client_run        ;
extern bool end_thread_tcp_client ;

void tcp_client(void)
{
    int iResult;
    while ( !end_thread_tcp_client ) {
        if ( tcp_client_run ) {

            SOCKET socket_client = INVALID_SOCKET;
            if (init_tcp_client( &socket_client ) != 0){
                tcp_client_run = 0;
            }

            while ( tcp_client_run ) {

                iResult = send( socket_client, (char*)&send_test_buf, sizeof(test_buf_struct), 0 );
                if (iResult == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    WSACleanup();
                    break;
                }

                iResult = recv(socket_client, (char*)&recv_test_buf, sizeof(test_buf_struct), 0);
                if ( iResult > 0 ) {
                    printf("Bytes received: %d\n", iResult);
                } else if ( iResult == 0 ) {
                    printf("Connection closed\n");
                } else {
                    if (WSAGetLastError()==10060) {
                        printf("recv TIMEDOUT\n");
                    } else {
                        printf("recv failed with error: %d\n", WSAGetLastError());
                        break;
                    }

                }
            }

            shutdown_tcp_client( &socket_client );
        }
        printf("hello from TCP server thread \n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

#endif