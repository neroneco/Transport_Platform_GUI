#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "implot.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include <thread>

#define _USE_MATH_DEFINES
#include <cmath>

#include "linmath.h"
#include "uart.h"


#define SAMPLING_FREQ        (200U) // [Hz]
#define PACKETS_PER_SECOND    (4U)
#define NUM_OF_DATA_SETS      (4U)
#define DATA_SECONDS_STORAGE (20U) // [s]


static int freq          = 200;
static int pack_per_sec  =  4;
static int data_sets_num =  4;

static int data_set_len_f32 = ( freq / pack_per_sec );              // 20
static int data_set_len_u8  = ( data_set_len_f32 * sizeof(float) ); // 20 * 4 = 80

static int pack_len_f32 = data_set_len_f32 * data_sets_num;   // 20 * 4 = 80
static int pack_len_u8  = data_set_len_u8  * data_sets_num;   // 80 * 4 = 320


static int window_width = 0;
static int window_height = 0;

uint32_t x_cart_speed = 10;
uint32_t y_cart_speed = 10;

uint32_t x_cart_pos = 215;
uint32_t y_cart_pos = 215;


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


data_packet_struct      Data_Packet[10]  = {0};




// UART 
static bool end_thread_01 = 0;
static bool UART = 0;
static bool new_data_ready = 0;
static char* device = "\\\\.\\COM3";
uint32_t baud_rate = 900000;

static int UART_iter = 0;
static SSIZE_T received;

// FIXME change below values from 610 to something meaningful
static float Pitch[DATA_SECONDS_STORAGE*SAMPLING_FREQ] = {0};
static float Roll[DATA_SECONDS_STORAGE*SAMPLING_FREQ] = {0};
static float Cart_dist_1[DATA_SECONDS_STORAGE*SAMPLING_FREQ] = {0};
static float Cart_dist_2[DATA_SECONDS_STORAGE*SAMPLING_FREQ] = {0};
static float time_data[DATA_SECONDS_STORAGE*SAMPLING_FREQ] = {0};
static float Data_buffer[DATA_SECONDS_STORAGE*SAMPLING_FREQ*NUM_OF_DATA_SETS] = {0};

static bool UART_recv_status = 0;
static bool UART_send_status = 0;


static int waiting_packet_num = 0;

// utility structure for realtime plot
struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    ImVector<ImVec2> Data;
    ScrollingBuffer(int max_size = (20000)) {
        MaxSize = max_size;
        Offset  = 0;
        Data.reserve(MaxSize);
    }
    void AddPoint(float x, float y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImVec2(x,y));
        else {
            Data[Offset] = ImVec2(x,y);
            Offset =  (Offset + 1) % MaxSize;
        }
    }
    void Erase() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset  = 0;
        }
    }
};

//-----------------------------------------------------------------------------
enum Data_Names {
    CARTS_POS_X,
    CARTS_POS_Y,
    CARTS_VEL_X,
    CARTS_VEL_Y,
    CARTS_ACC_X,
    CARTS_ACC_Y,
    MPU9250_ACCE_X,
    MPU9250_ACCE_Y,
    MPU9250_ACCE_Z,
    MPU9250_GYRO_X,
    MPU9250_GYRO_Y,
    MPU9250_GYRO_Z,
    MPU6886_ACCE_X,
    MPU6886_ACCE_Y,
    MPU6886_ACCE_Z,
    MPU6886_GYRO_X,
    MPU6886_GYRO_Y,
    MPU6886_GYRO_Z,
    PITCH_NO_FILTER,
    ROLL_NO_FILTER,
    PITCH_COMPLEMENTARY,
    ROLL_COMPLEMENTARY,
    PITCH_ALFA_BETA,
    ROLL_ALFA_BETA,
    PITCH_KALMAN,
    ROLL_KALMAN,
    PITCH,
    ROLL
};


void RealtimePlots(float* y_data_1, float* y_data_2, float* y_data_3, float* y_data_4) {

    static ScrollingBuffer sdata[32];


    static float t = 0;
    static int iter = 0;
    static int packet_num = 0;
    static int seconds = 10;


    if( waiting_packet_num > 0){
        for ( int i = 0; i<100; i++) {
            if ( 2*waiting_packet_num > i ) {
                t += 0.005;

                sdata[CARTS_POS_X].AddPoint(t, Data_Packet[packet_num].carts_pos_x[iter]);
                sdata[CARTS_POS_Y].AddPoint(t, Data_Packet[packet_num].carts_pos_y[iter]);
                sdata[CARTS_VEL_X].AddPoint(t, Data_Packet[packet_num].carts_vel_x[iter]);
                sdata[CARTS_VEL_Y].AddPoint(t, Data_Packet[packet_num].carts_vel_y[iter]);
                sdata[CARTS_ACC_X].AddPoint(t, Data_Packet[packet_num].carts_acc_x[iter]);
                sdata[CARTS_ACC_Y].AddPoint(t, Data_Packet[packet_num].carts_acc_y[iter]);
                sdata[MPU9250_ACCE_X].AddPoint(t, Data_Packet[packet_num].mpu9250_acce_x[iter]);
                sdata[MPU9250_ACCE_Y].AddPoint(t, Data_Packet[packet_num].mpu9250_acce_y[iter]);
                sdata[MPU9250_ACCE_Z].AddPoint(t, Data_Packet[packet_num].mpu9250_acce_z[iter]);
                sdata[MPU9250_GYRO_X].AddPoint(t, Data_Packet[packet_num].mpu9250_gyro_x[iter]);
                sdata[MPU9250_GYRO_Y].AddPoint(t, Data_Packet[packet_num].mpu9250_gyro_y[iter]);
                sdata[MPU9250_GYRO_Z].AddPoint(t, Data_Packet[packet_num].mpu9250_gyro_z[iter]);
                sdata[MPU6886_ACCE_X].AddPoint(t, Data_Packet[packet_num].mpu6886_acce_x[iter]);
                sdata[MPU6886_ACCE_Y].AddPoint(t, Data_Packet[packet_num].mpu6886_acce_y[iter]);
                sdata[MPU6886_ACCE_Z].AddPoint(t, Data_Packet[packet_num].mpu6886_acce_z[iter]);
                sdata[MPU6886_GYRO_X].AddPoint(t, Data_Packet[packet_num].mpu6886_gyro_x[iter]);
                sdata[MPU6886_GYRO_Y].AddPoint(t, Data_Packet[packet_num].mpu6886_gyro_y[iter]);
                sdata[MPU6886_GYRO_Z].AddPoint(t, Data_Packet[packet_num].mpu6886_gyro_z[iter]);
                sdata[PITCH_NO_FILTER].AddPoint(t, Data_Packet[packet_num].pitch_no_filter[iter]);
                sdata[ROLL_NO_FILTER].AddPoint(t, Data_Packet[packet_num].roll_no_filter[iter]);
                sdata[PITCH_COMPLEMENTARY].AddPoint(t, Data_Packet[packet_num].pitch_complementary[iter]);
                sdata[ROLL_COMPLEMENTARY].AddPoint(t, Data_Packet[packet_num].roll_complementary[iter]);
                sdata[PITCH_ALFA_BETA].AddPoint(t, Data_Packet[packet_num].pitch_alfa_beta[iter]);
                sdata[ROLL_ALFA_BETA].AddPoint(t, Data_Packet[packet_num].roll_alfa_beta[iter]);
                sdata[PITCH_KALMAN].AddPoint(t, Data_Packet[packet_num].pitch_kalman[iter]);
                sdata[ROLL_KALMAN].AddPoint(t, Data_Packet[packet_num].roll_kalman[iter]);
                sdata[PITCH].AddPoint(t, Data_Packet[packet_num].pitch[iter]);
                sdata[ROLL].AddPoint(t, Data_Packet[packet_num].roll[iter]);


                iter++;
                iter %= 250; 
                if(iter == 0){
                    waiting_packet_num--;
                    packet_num++;
                    packet_num %= 10;
                }
            } else {
                break;
            }
        }
    }

    static float history = 10.0f;
    ImGui::SetCursorPosX(267);
    ImGui::PushItemWidth(300);
    ImGui::SliderFloat("History",&history,1,30,"%.1f s",ImGuiSliderFlags_None);

    static ImPlotAxisFlags flags = ImPlotAxisFlags_None;
    if(sdata[PITCH_NO_FILTER].Data.size()!=0){
        if (ImPlot::BeginPlot("##Scrolling_1", ImVec2(400,250))) {
            ImPlot::SetupAxes("time [s]", "Pitch [deg]", flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,-180,180);
            ImPlot::SetNextLineStyle(ImVec4(0.941, 0.0, 1.0, 0.784),2.0);
            ImPlot::PlotLine("no filter", &sdata[PITCH_NO_FILTER].Data[0].x, &sdata[PITCH_NO_FILTER].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
            ImPlot::PlotLine("complementary filter", &sdata[PITCH_COMPLEMENTARY].Data[0].x, &sdata[PITCH_COMPLEMENTARY].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
            ImPlot::EndPlot();
        }
    }
    ImGui::SameLine();
    if(sdata[ROLL_NO_FILTER].Data.size()!=0){
        if (ImPlot::BeginPlot("##Scrolling_2", ImVec2(400,250))) {
            ImPlot::SetupAxes("time [s]", "Roll [deg]", flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,-180,180);
            ImPlot::SetNextLineStyle(ImVec4(0.447, 0.604, 0.452, 0.784),2.0);
            ImPlot::PlotLine("no filter", &sdata[ROLL_NO_FILTER].Data[0].x, &sdata[ROLL_NO_FILTER].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
            ImPlot::PlotLine("complementary filter", &sdata[ROLL_COMPLEMENTARY].Data[0].x, &sdata[ROLL_COMPLEMENTARY].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
            ImPlot::EndPlot();
        }
    }
    if(sdata[CARTS_POS_X].Data.size()!=0){
        if (ImPlot::BeginPlot("##Scrolling_3", ImVec2(400,250))) {
            ImPlot::SetupAxes("time [s]", "Cart X position [mm]", flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,0,365);
            ImPlot::SetNextLineStyle(ImVec4(1.000, 0.0, 0.0, 0.784),2.0);
            ImPlot::PlotLine("cart x", &sdata[CARTS_POS_X].Data[0].x, &sdata[CARTS_POS_X].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
            ImPlot::EndPlot();
        }
    }
    ImGui::SameLine();
    if(sdata[CARTS_POS_X].Data.size()!=0){
        if (ImPlot::BeginPlot("##Scrolling_4", ImVec2(400,250))) {
            ImPlot::SetupAxes("time [s]", "Cart Y position [mm]", flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,0,165);
            ImPlot::SetNextLineStyle(ImVec4(0.853, 1.0, 0.0, 0.784),2.0);
            ImPlot::PlotLine("cart y", &sdata[CARTS_POS_X].Data[0].x, &sdata[CARTS_POS_X].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
            ImPlot::EndPlot();
        }
    }
    if(sdata[CARTS_POS_X].Data.size()!=0){
        if (ImGui::Button("Save data to file")) // TODO: add option of saving disaried amount of time like e.g. 1[s], 3[s], 10[s] ...
        {
            ImGui::LogToFile(-1,"data.txt");
            for (int i=0; i<sdata[CARTS_POS_X].Data.size();i++) {
                ImGui::LogText("\n");
                for ( int k=0; k<28; k++ ) {
                    if ( (k == MPU9250_ACCE_X) ||
                         (k == MPU9250_ACCE_Y) ||
                         (k == MPU9250_ACCE_Z) ||
                         (k == MPU9250_GYRO_X) ||
                         (k == MPU9250_GYRO_Y) ||
                         (k == MPU9250_GYRO_Z) ||
                         (k == MPU6886_ACCE_X) ||
                         (k == MPU6886_ACCE_Y) ||
                         (k == MPU6886_ACCE_Z) ||
                         (k == MPU6886_GYRO_X) ||
                         (k == MPU6886_GYRO_Y) ||
                         (k == MPU6886_GYRO_Z) )
                    {
                        ImGui::LogText("%.3f ",sdata[k].Data[i].y);
                    }
                }
            }
            ImGui::LogFinish();
        }
    }
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

static bool     direction     =  0;
static bool     axis          =  0;
static uint32_t speed         = 10; // [mm/s]
static uint32_t position      = 10; // [mm]

int msg_type = MSG_NONE;

uint8_t TX[10] = {'x'};

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
                UART_iter = UART_iter % (seconds*pack_per_sec);
                PurgeComm(port, PURGE_RXABORT);
                PurgeComm(port, PURGE_TXABORT);
                PurgeComm(port, PURGE_RXCLEAR);
                PurgeComm(port, PURGE_TXCLEAR);
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                static uint8_t abba[10] = {'1','2','3','4','5','6','7','8','9','0',};
                if(write_port(port, abba, 10)!=0){
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

                // memcpy((Pitch       + (20*UART_iter)), (Data_buffer +  0 + (80*UART_iter)), (20*sizeof(float)));
                // memcpy((Roll        + (20*UART_iter)), (Data_buffer + 20 + (80*UART_iter)), (20*sizeof(float)));
                // memcpy((Cart_dist_1 + (20*UART_iter)), (Data_buffer + 40 + (80*UART_iter)), (20*sizeof(float)));
                // memcpy((Cart_dist_2 + (20*UART_iter)), (Data_buffer + 60 + (80*UART_iter)), (20*sizeof(float)));

                memcpy((Pitch       + (data_set_len_f32 * UART_iter)), (Data_buffer + (0 * data_set_len_f32) + (pack_len_f32 * UART_iter)), data_set_len_u8);
                memcpy((Roll        + (data_set_len_f32 * UART_iter)), (Data_buffer + (1 * data_set_len_f32) + (pack_len_f32 * UART_iter)), data_set_len_u8);
                memcpy((Cart_dist_1 + (data_set_len_f32 * UART_iter)), (Data_buffer + (2 * data_set_len_f32) + (pack_len_f32 * UART_iter)), data_set_len_u8);
                memcpy((Cart_dist_2 + (data_set_len_f32 * UART_iter)), (Data_buffer + (3 * data_set_len_f32) + (pack_len_f32 * UART_iter)), data_set_len_u8);

                UART_iter++;
                waiting_packet_num++;
            }

            CloseHandle(port);
        }
        printf("hello from thread\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    window_width = width;
    window_height = height;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


int main()
{

    // -+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+-
    // ------------------- GLFW --------------------
    // -+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+-

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1600, 900, "Transport platform GUI", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }  
    glViewport(0, 0, 1600, 900);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetKeyCallback(window, key_callback);

    // =============================================
    // ----------------- END GLFW ------------------
    // =============================================





    // -+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+-
    // --------------  ImGui/ImPlot ----------------
    // -+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+-

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    const char* glsl_version = "#version 130";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_ImGuiWindow  = false;
    bool show_demo_ImPlotWindow = false;
    bool show_another_window    = false;

    // ImPlot setup
    float f = 0.01;
    float x_data[1000] = {0};
    float y_data[1000] = {0};
    for(int i=0;i<1000;i++){
        x_data[i] = i;
        y_data[i] = sin(2*M_PI*f*i);
    }
    for(int i=0;i<610;i++){
        time_data[i] = i;
    }

    // =============================================
    // ------------- END ImGui/ImPlot --------------
    // =============================================





    // -+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+-
    // ------------- UART thread init --------------
    // -+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+-
    
    // UART init


    // thread creation
    std::thread thr_01(UART_communication);

    // =============================================
    // ----------- END UART thread init ------------
    // =============================================





    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (show_demo_ImGuiWindow)
            ImGui::ShowDemoWindow(&show_demo_ImGuiWindow);

        if (show_demo_ImPlotWindow)
            ImPlot::ShowDemoWindow(&show_demo_ImPlotWindow);
        
        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static ImPlotSubplotFlags flags = ImPlotSubplotFlags_NoResize;
            static int rows = 2;
            static int cols = 2;
            static float f = 0.0f;
            static int counter = 0;


            // Figures window
            //ImGui::SetNextWindowSize(ImVec2(840,580));
            //ImGui::SetNextWindowPos(ImVec2(10,10));
            ImGui::Begin("Figures",NULL,  ImGuiWindowFlags_NoResize  |
                                          ImGuiWindowFlags_NoMove    |
                                          ImGuiWindowFlags_NoCollapse ); // Create a window called "Hello, world!" and append into it.
                RealtimePlots(Pitch,Roll,Cart_dist_1,Cart_dist_2);
            ImGui::End();

            // UART window
            ImGui::Begin("Serial Comunication",NULL,ImGuiWindowFlags_None);
            ImGui::SeparatorText("UART configuration");
            ImGui::LabelText("label", "Value");
            // Baud rate setup
            static char buf_uart[66] = "";
            sprintf(buf_uart,"%d",baud_rate); 
            ImGui::InputText("baud rate",buf_uart, 64, ImGuiInputTextFlags_CharsDecimal);
            try {
                baud_rate = std::stoi(buf_uart);
            } catch(std::invalid_argument& e) {
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784),"Not valid argument");
                baud_rate = 0;
            }
            // COM port setup
            static char buf_com[66] = "";
            sprintf(buf_com,device);
            ImGui::InputText("device port",buf_com, 64, ImGuiInputTextFlags_CharsDecimal);
            try {

            } catch(std::invalid_argument& e) {
                ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784),"Not valid argument");
                baud_rate = 0;
            }

            // UART enable
            ImGui::Checkbox("UART Enable", &UART);
            ImGui::Text("Waiting Packets to be ploted: %d",waiting_packet_num);
            if (UART) {
                if (ImGui::TreeNode("Status of UART connection")) {
                    if (UART_send_status) {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "TX");
                    } else {
                        ImGui::TextDisabled("TX");
                    }
                    if (UART_recv_status) {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "RX");
                    } else {
                        ImGui::TextDisabled("RX");
                    }
                    ImGui::TreePop();
                }
            }


            ImGui::End();

            // Cart control window
            ImGui::Begin("Cart control",NULL,ImGuiWindowFlags_None);
            static int ctrl_mod = 0;
            if (ImGui::Combo("Control mode", &ctrl_mod, "Automatic\0Manual\0"))
            {
                switch (ctrl_mod)
                {
                case 0: ; break;
                case 1: ; break;
                }
            }
            if (ctrl_mod == 1) {
                ImGui::SeparatorText("Motor configuration");
                ImGui::SeparatorText("SPEED");
                static char buf_cart[66] = "";
                sprintf(buf_cart,"%d",x_cart_speed); 
                ImGui::InputText("cart X speed [mm/s]",buf_cart, 64, ImGuiInputTextFlags_CharsDecimal);
                try {
                    x_cart_speed = std::stoi(buf_cart);
                } catch(std::invalid_argument& e) {
                    ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784),"Not valid argument");
                    x_cart_speed = 10;
                }
                sprintf(buf_cart,"%d",y_cart_speed);
                ImGui::InputText("cart Y speed [mm/s]",buf_cart, 64, ImGuiInputTextFlags_CharsDecimal);
                try {
                    y_cart_speed = std::stoi(buf_cart);
                } catch(std::invalid_argument& e) {
                    ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784),"Not valid argument");
                    y_cart_speed = 10;
                }
                if (ImGui::Button("X cart UPDATE SPEED",ImVec2(150,30)))
                {
                    // do JOG + x direction
                    msg_type   = MSG_SPEED;
                    axis       = X;
                }
                ImGui::SameLine();
                if (ImGui::Button("Y cart UPDATE SPEED",ImVec2(150,30)))
                {
                    // do JOG + x direction
                    msg_type   = MSG_SPEED;
                    axis       = Y;
                }

                ImGui::SeparatorText("POSITION");
                sprintf(buf_cart,"%d",x_cart_pos); 
                ImGui::InputText("cart X position [mm]",buf_cart, 64, ImGuiInputTextFlags_CharsDecimal);
                try {
                    x_cart_pos = std::stoi(buf_cart);
                } catch(std::invalid_argument& e) {
                    ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784),"Not valid argument");
                    x_cart_pos = 215;
                }
                sprintf(buf_cart,"%d",y_cart_pos);
                ImGui::InputText("cart Y position [mm]",buf_cart, 64, ImGuiInputTextFlags_CharsDecimal);
                try {
                    y_cart_pos = std::stoi(buf_cart);
                } catch(std::invalid_argument& e) {
                    ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784),"Not valid argument");
                    y_cart_pos = 215;
                }
                if (ImGui::Button("X cart UPDATE POSITION",ImVec2(170,30)))
                {
                    // do JOG + x direction
                    msg_type   = MSG_MOV;
                    axis       = X;
                }
                ImGui::SameLine();
                if (ImGui::Button("Y cart UPDATE POSITION",ImVec2(170,30)))
                {
                    // do JOG + x direction
                    msg_type   = MSG_MOV;
                    axis       = Y;
                }


                ImGui::SeparatorText("JOG");
                if (ImGui::Button("JOG + x",ImVec2(70,30)))
                {
                    // do JOG + x direction
                    msg_type   = MSG_JOG;
                    direction  = POSITIV;
                    axis       = X;
                }
                ImGui::SameLine();
                if (ImGui::Button("JOG - x",ImVec2(70,30)))
                {
                    // do JOG - x direction
                    msg_type   = MSG_JOG;
                    direction  = NEGATIV;
                    axis       = X;
                }
                ImGui::SameLine();
                ImGui::Text("X direction");
                if (ImGui::Button("JOG + y",ImVec2(70,30)))
                {
                    // do JOG + y direction
                    msg_type   = MSG_JOG;
                    direction  = POSITIV;
                    axis       = Y;
                }
                ImGui::SameLine();
                if (ImGui::Button("JOG - y",ImVec2(70,30)))
                {
                    // do JOG - y direction
                    msg_type   = MSG_JOG;
                    direction  = NEGATIV;
                    axis       = Y;
                }
                ImGui::SameLine();
                ImGui::Text("y direction");
            
            }
            ImGui::End();

            // Help window
            ImGui::Begin("Help",NULL,ImGuiWindowFlags_None); 
            ImGui::Text("This is some useful text.");                         // Display some text (you can use a format strings too)
            ImGui::Checkbox("ImGui Demo Window", &show_demo_ImGuiWindow);     // Edit bools storing our window open/close state
            ImGui::Checkbox("ImPlot Demo Window", &show_demo_ImPlotWindow);
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
            ImGui::Text("Window height: %d",window_height);
            ImGui::Text("Window width:  %d",window_width);
            // ImGui::Text("sleep time [us]: %d",sleep_time);
            // ImGui::Text("fps: %.1f",fps);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
    end_thread_01 = 1;
    UART = 0;
    while(!thr_01.joinable());
    if(thr_01.joinable()){
        printf("Ending UART thread\n");
        thr_01.join();
    }

    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}
