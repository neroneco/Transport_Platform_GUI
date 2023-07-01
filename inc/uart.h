#ifndef UART_H
#define UART_H

#include <stdio.h>
#include <stdint.h>
#include <winsock2.h>
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

enum MPU_SAMPLING_RATE {
    RATE_1kHZ,
    RATE_4kHZ,
    RATE_8kHZ,
    RATE_32kHZ
};

enum MPU_ACCEL_SCALE {
    SCALE_2g,
    SCALE_4g,
    SCALE_8g,
    SCALE_16g
};

enum MPU_GYRO_SCALE {
    SCALE_250dps,
    SCALE_500dps,
    SCALE_1000dps,
    SCALE_2000dps
};

enum MPU_DLPF {
    MPU_DLPF_NONE,
    MPU_DLPF_5HZ,
    MPU_DLPF_5_1HZ,
    MPU_DLPF_10HZ,
    MPU_DLPF_10_2HZ,
    MPU_DLPF_20HZ,
    MPU_DLPF_21_2HZ,
    MPU_DLPF_41HZ,
    MPU_DLPF_44_8HZ,
    MPU_DLPF_92HZ,
    MPU_DLPF_99HZ,
    MPU_DLPF_176HZ,
    MPU_DLPF_184HZ,
    MPU_DLPF_218_1HZ,
    MPU_DLPF_250HZ,
    MPU_DLPF_420HZ,
    MPU_DLPF_460HZ,
    MPU_DLPF_1046HZ,
    MPU_DLPF_1130HZ,
    MPU_DLPF_3281HZ,
    MPU_DLPF_3600HZ,
    MPU_DLPF_8173HZ,
    MPU_DLPF_8800HZ
};


enum IMU_SAMPLING_RATE {
    RATE_100HZ,
    RATE_200HZ,
    RATE_250HZ,
    RATE_400HZ,
    RATE_500HZ,
};

enum FILTER_TYPE {
    NONE,
    COMPLEMENTARY,
    ALFA_BETA,
    KALMAN,
    ALL
};

enum STEP_MOTOR {
    STEP_16,
    STEP_8,
    STEP_4,
    STEP_2,
    STEP_FULL
};


typedef struct {
    uint8_t sampling_rate;
    uint8_t scale_accel;
    uint8_t scale_gyro;
    uint8_t dlpf_accel;
    uint8_t dlpf_gyro;
} mpu_status;

typedef struct {
    uint8_t sampling_rate;
    uint8_t filter_type;
} imu_status;

typedef struct {
    float cart_x_mass;
    float cart_y_mass;
    int   steps;
    float max_x_position;
    float max_y_position;
    float max_speed;
    float max_accel;
} carts_status;

typedef struct {
    mpu_status      mpu9250;
    mpu_status      mpu6886;
    imu_status      imu;
    carts_status    carts;
} system_status_struct;

typedef struct {
    system_status_struct sytem_status;
    float carts_pos_adc_x[100];
    float carts_pos_x[100];
    float carts_pos_adc_y[100];
    float carts_pos_y[100];
    float carts_vel_x[100];
    float carts_vel_y[100];
    float carts_acc_x[100];
    float carts_acc_y[100];
#if 1
    float mpu9250_acce_x[100];
    float mpu9250_acce_y[100];
    float mpu9250_acce_z[100];
    float mpu9250_gyro_x[100];
    float mpu9250_gyro_y[100];
    float mpu9250_gyro_z[100];
    float mpu9250_pitch[100];
    float mpu9250_roll[100];

    float mpu6886_acce_x[100];
    float mpu6886_acce_y[100];
    float mpu6886_acce_z[100];
    float mpu6886_gyro_x[100];
    float mpu6886_gyro_y[100];
    float mpu6886_gyro_z[100];
    float mpu6886_pitch[100];
    float mpu6886_roll[100];

#endif
    float pitch_no_filter[100];
    float roll_no_filter[100];
    float pitch_complementary[100];
    float roll_complementary[100];
    float pitch_alfa_beta[100];
    float roll_alfa_beta[100];
    float pitch_kalman[100];
    float roll_kalman[100];
    float pitch[100];
    float roll[100];
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
extern const char* device       ;
extern uint32_t baud_rate ;

extern bool     direction ;
extern bool     axis      ;
extern uint32_t speed     ;
extern uint32_t position  ;

extern int msg_type   ;

extern int waiting_packet_num ;
extern data_packet_struct       Data_Packet[10]    ;
extern system_status_struct     System_Status_Data ;

void UART_communication(void)
{
    static uint8_t buf[10];
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
                    printf("Error in READ from serial port 1:\n data read: %d\n packet length: %llu\n", data_read, sizeof(data_packet_struct));
                    UART = 0;
                    break;
                } else {
                    memcpy( &System_Status_Data, &Data_Packet[packet_num].sytem_status, sizeof(system_status_struct) );
                    packet_num++;
                    packet_num %= 10;
                    printf("suprise!!! : %3.f %d \n system_status_imu_filter_type %.3f \n", Data_Packet[0].carts_pos_x[7], packet_num, System_Status_Data.carts.cart_y_mass);
                }

                waiting_packet_num++;
            }

            CloseHandle(port);
        }
        printf("hello from UART thread \n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void print_system_status( system_status_struct* system_status_data )
{
    static ImGuiTableFlags tab_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_NoHostExtendX;
    if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("MPU9250"))
        {
            if (ImGui::BeginTable("table_mpu9250", 2, tab_flags))
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("sampling rate");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%d", system_status_data->mpu9250.sampling_rate);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("accelerometer scale");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%d", system_status_data->mpu9250.scale_accel);
                
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("gyroscope scale");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%d", system_status_data->mpu9250.scale_gyro);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("accelerometer dlpf bandwidth");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%d", system_status_data->mpu9250.dlpf_accel);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("gyroscope dlpf bandwidth");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%d", system_status_data->mpu9250.dlpf_gyro);

                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("MPU6886"))
        {
            if (ImGui::BeginTable("table_mpu6886", 2, tab_flags))
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("sampling rate");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%d", system_status_data->mpu6886.sampling_rate);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("accelerometer scale");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%d", system_status_data->mpu6886.scale_accel);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("gyroscope scale");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%d", system_status_data->mpu6886.scale_gyro);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("accelerometer dlpf bandwidth");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%d", system_status_data->mpu6886.dlpf_accel);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("gyroscope dlpf bandwidth");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%d", system_status_data->mpu6886.dlpf_gyro);

                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("IMU"))
        {
            if (ImGui::BeginTable("table_imu", 2, tab_flags))
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("sampling rate");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%d", system_status_data->imu.sampling_rate);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("filter type");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%d", system_status_data->imu.filter_type);
                
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("CARTS"))
        {
            if (ImGui::BeginTable("table_carts", 2, tab_flags))
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("microsteps"); 
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%d", system_status_data->carts.steps);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("cart X max position");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%.3f", system_status_data->carts.max_x_position);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();            
                ImGui::Text("cart Y max position");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%.3f", system_status_data->carts.max_y_position);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("cart X max speed");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%.3f", system_status_data->carts.max_speed);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("cart Y max acceleration");
                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784), "%.3f", system_status_data->carts.max_accel);

                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}


#endif