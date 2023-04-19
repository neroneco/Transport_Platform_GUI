#ifndef UART_H
#define UART_H

#include <stdio.h>
#include <stdint.h>
#include <windows.h>


void print_error(const char * context)
{
  DWORD error_code = GetLastError();
  char buffer[256];
  DWORD size = FormatMessageA(
    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
    NULL, error_code, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
    buffer, sizeof(buffer), NULL);
  if (size == 0) { buffer[0] = 0; }
  fprintf(stderr, "%s: %s\n", context, buffer);
}
 

HANDLE open_serial_port(const char * device, uint32_t baud_rate)
{
  HANDLE port = CreateFileA(device, GENERIC_READ | GENERIC_WRITE, 0, NULL,
    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (port == INVALID_HANDLE_VALUE)
  {
    print_error(device);
    return INVALID_HANDLE_VALUE;
  }
 
  // Flush away any bytes previously read or written.
  BOOL success = FlushFileBuffers(port);
  if (!success)
  {
    print_error("Failed to flush serial port");
    CloseHandle(port);
    return INVALID_HANDLE_VALUE;
  }
 
  // Configure read and write operations to time out after 5000 ms.
  COMMTIMEOUTS timeouts = {0};
  timeouts.ReadIntervalTimeout = 0;
  timeouts.ReadTotalTimeoutConstant = 5000;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.WriteTotalTimeoutConstant = 1000;
  timeouts.WriteTotalTimeoutMultiplier = 0;
 
  success = SetCommTimeouts(port, &timeouts);
  if (!success)
  {
    print_error("Failed to set serial timeouts");
    CloseHandle(port);
    return INVALID_HANDLE_VALUE;
  }
 
  // Set the baud rate and other options.
  DCB state = {0};
  state.DCBlength = sizeof(DCB);
  state.BaudRate = baud_rate;
  state.ByteSize = 8;
  state.Parity = NOPARITY;
  state.StopBits = ONESTOPBIT;
  success = SetCommState(port, &state);
  if (!success)
  {
    print_error("Failed to set serial settings");
    CloseHandle(port);
    return INVALID_HANDLE_VALUE;
  }
 
  return port;
}

// Writes bytes to the serial port, returning 0 on success and -1 on failure.
int write_port(HANDLE port, uint8_t * buffer, size_t size)
{
  DWORD written;
  BOOL success = WriteFile(port, buffer, size, &written, NULL);
  if (!success)
  {
    print_error("Failed to write to port");
    return -1;
  }
  if (written != size)
  {
    print_error("Failed to write all bytes to port");
    return -1;
  }
  return 0;
}
 
// Reads bytes from the serial port.
// Returns after all the desired bytes have been read, or if there is a
// timeout or other error.
// Returns the number of bytes successfully read into the buffer, or -1 if
// there was an error reading.
SSIZE_T read_port(HANDLE port, uint8_t * buffer, size_t size)
{
  DWORD received;
  BOOL success = ReadFile(port, buffer, size, &received, NULL);
  if (!success)
  {
    print_error("Failed to read from port");
    return -1;
  }
  return received;
}

//-----------------------------------------------------------------------------
// +--------------------------------+
// |  code    | message             |
// |  0       | no message          |
// |  d       | send data           |
// |  j+x000  | jog y + distance    |
// |  j-x000  | jog x - distance    |
// |  j+y000  | jog y + distance    |
// |  j-y000  | jog x - distance    |
// |  mx100   | mov y + distance    |
// |  mx100   | mov x - distance    |
// |  my100   | mov y + distance    |
// |  my100   | mov x - distance    |
// |  sx100   | set speed of x axis |
// |  sy100   | set speed of y axis |
// +--------------------------------+

typedef struct {
    float carts_pos_x[250];
    float carts_pos_y[250];
    float carts_vel_x[250];
    float carts_vel_y[250];
    float carts_acc_x[250];
    float carts_acc_y[250];
#if 1
    float mpu9250_acce_x[250];
    float mpu9250_acce_y[250];
    float mpu9250_acce_z[250];
    float mpu9250_gyro_x[250];
    float mpu9250_gyro_y[250];
    float mpu9250_gyro_z[250];

    float mpu6886_acce_x[250];
    float mpu6886_acce_y[250];
    float mpu6886_acce_z[250];
    float mpu6886_gyro_x[250];
    float mpu6886_gyro_y[250];
    float mpu6886_gyro_z[250];
#endif
    float pitch_no_filter[250];
    float roll_no_filter[250];
    float pitch_complementary[250];
    float roll_complementary[250];
    float pitch_alfa_beta[250];
    float roll_alfa_beta[250];
    float pitch_kalman[250];
    float roll_kalman[250];
    float pitch[250];
    float roll[250];
} data_packet_struct;

enum MSG_TYPE {
    MSG_NONE,
    MSG_DATA,
    MSG_JOG,
    MSG_MOV,
    MSG_SPEED
};

enum AXIS {
    X,
    Y
};

enum DIRECTION {
    POSITIV,
    NEGATIV
};

extern uint32_t x_cart_speed ;
extern uint32_t y_cart_speed ;

extern uint32_t x_cart_pos ;
extern uint32_t y_cart_pos ;


extern bool end_thread_01 ;
extern bool UART          ;
extern char* device       ;
extern uint32_t baud_rate ;

extern bool     direction ;
extern bool     axis      ;
extern uint32_t speed     ;
extern uint32_t position  ;

extern int msg_type   ;

extern int waiting_packet_num             ;
extern data_packet_struct Data_Packet[10] ;

void UART_communication(void)
{
    static uint8_t buf[10];
    static int seconds = 10;
    while(!end_thread_01){
        if(UART){
        
            HANDLE port = open_serial_port(device, baud_rate);
            if (port == INVALID_HANDLE_VALUE) { 
                printf("INVALID_HANDLE_VALUE for UART\n");
                UART = 0; 
            }
            memset(buf,0,sizeof(buf));
            msg_type = MSG_DATA;

            while(UART){

                switch (msg_type) {  // TODO varibles like msg_type, direct, axis should be atomic

                    case MSG_DATA: {
                        buf[0] = 'd';
                    }break;

                    case MSG_JOG: {

                        buf[0] = 'j';

                        switch (direction) {
                            case POSITIV : {
                                buf[1] = '+';
                            } break;
                            case NEGATIV : {
                                buf[1] = '-';
                            } break;
                        }

                        switch (axis) {
                            case X : {
                                buf[2] = 'x';
                            } break;
                            case Y : {
                                buf[2] = 'y';
                            } break;
                        }
                    }break;

                    case MSG_MOV: {

                        buf[0] = 'm';

                        switch (axis) {
                            case X : {
                                buf[1] = 'x';
                                position = x_cart_pos;
                            } break;
                            case Y : {
                                buf[1] = 'y';
                                position = y_cart_pos;
                            } break;
                        }
                        memcpy(buf+2,&position,sizeof(position));
                    }break;

                    case MSG_SPEED: {

                        buf[0] = 's';

                        switch (axis) {
                            case X : {
                                buf[1] = 'x';
                                speed = x_cart_speed;
                            } break;
                            case Y : {
                                buf[1] = 'y';
                                speed = y_cart_speed;
                            } break;
                        }
                        memcpy(buf+2,&speed,sizeof(speed));
                    }break;

                    case MSG_NONE:
                    default:{
                        memset(buf,0,sizeof(buf));
                    }break;
                }

                printf("%s \n",buf);
                PurgeComm(port, PURGE_RXABORT);
                PurgeComm(port, PURGE_TXABORT);
                PurgeComm(port, PURGE_RXCLEAR);
                PurgeComm(port, PURGE_TXCLEAR);
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                if(write_port(port, buf, 10)!=0){
                    printf("Error in WRITE from serial port\n");
                    UART = 0;
                    break;
                }
                memset(buf,0,sizeof(buf));
                msg_type = MSG_DATA;


                //std::this_thread::sleep_for(std::chrono::milliseconds(20));
                static int packet_num = 0;
                int data_read = read_port( port, (uint8_t*)&Data_Packet[packet_num], sizeof(data_packet_struct) );
                if( data_read != sizeof(data_packet_struct) ){
                    printf("Error in READ from serial port 1:\n data read: %d\n packet length: %d\n",data_read,sizeof(data_packet_struct));
                    UART = 0;
                    break;
                } else {
                    packet_num++;
                    packet_num %= 10;
                    printf("suprise!!! : %3.f %d \n", Data_Packet[0].carts_pos_x[7], packet_num);
                }

                waiting_packet_num++;
            }

            CloseHandle(port);
        }
        printf("hello from thread\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}


#endif