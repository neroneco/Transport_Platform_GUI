#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

int init_tcp_client(SOCKET* ConnectSocket)
{
    const char* ip_address  = "192.168.1.111" ;
    const char* port        = "7"     ;
    WSADATA wsaData;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
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

    DWORD OptVal = 5000;
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

typedef struct {
    char n1;
    char n2;
    char n3;
    char n4;
} test_buf_struct;

enum control_mode {
    AUTO,
    MANUAL
};

typedef struct {
    int mode;
    int x_en;
    int y_en;
    float x_position;
    float y_position;
    float x_velocity;
    float y_velocity;
} config_packet_struct;

test_buf_struct recv_test_buf = {0};
test_buf_struct send_test_buf = {.n1='a', .n2='b', .n3='c', .n4='d'};

enum STEPPER_STATUS{
    ENABLED,
    DISABLED
};

config_packet_struct config_packet = { .x_en        = DISABLED,
                                       .y_en        = DISABLED, 
                                       .x_position  = 0.0,
                                       .y_position  = 0.0,
                                       .x_velocity  = 0.0,
                                       .y_velocity  = 0.0 
                                     };


extern bool tcp_client_run        ;
extern bool end_thread_tcp_client ;

extern int waiting_packet_num ;
extern data_packet_struct       Data_Packet[10]    ;
extern system_status_struct     System_Status_Data ;

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

                iResult = send( socket_client, (char*)&config_packet, sizeof(config_packet_struct), 0 );
                if (iResult == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    WSACleanup();
                    break;
                }
                printf("\
                  sent: x_pos: %.3f \n\
                        x_spd: %.3f \n\
                        x_en: %d \n\
                        y_pos: %.3f \n\
                        y_spd: %.3f \n\
                        y_en: %d \n", 
                        config_packet.x_position,
                        config_packet.x_velocity,
                        config_packet.x_en,
                        config_packet.y_position,
                        config_packet.y_velocity,
                        config_packet.y_en);

                static int packet_num = 0;
                iResult = recv(socket_client, (char*)&Data_Packet[packet_num], sizeof(data_packet_struct), 0);
                if ( iResult > 0 ) {
                    printf("Bytes received / expected : %d / %llu \n", iResult, sizeof(data_packet_struct));
                    if ( iResult != sizeof(data_packet_struct) ) {
                        break;
                    }
                    memcpy( &System_Status_Data, &Data_Packet[packet_num].sytem_status, sizeof(system_status_struct) );
                    packet_num++;
                    packet_num %= 10;
                    printf("suprise!!! : %3.f %d \n system_status_imu_filter_type %.3f \n", Data_Packet[0].carts_pos_x[7], packet_num, System_Status_Data.carts.cart_y_mass);

                } else if ( iResult == 0 ) {
                    printf("Connection closed\n");
                    break;
                } else {
                    if (WSAGetLastError()==10060) {
                        printf("recv TIMEDOUT\n");
                        break;
                    } else {
                        printf("recv failed with error: %d\n", WSAGetLastError());
                        break;
                    }
                }
                waiting_packet_num++;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            tcp_client_run = 0;
            shutdown_tcp_client( &socket_client );
        }
        printf("hello from TCP server thread \n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

#endif