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


#define SAMPLING_FREQ        (100U) // [Hz]
#define PACKETS_PER_SECOND    (4U)
#define NUM_OF_DATA_SETS      (4U)
#define DATA_SECONDS_STORAGE (20U) // [s]


static int freq          = 100;
static int pack_per_sec  =  4;
static int data_sets_num =  4;

static int data_set_len_f32 = ( freq / pack_per_sec );              // 20
static int data_set_len_u8  = ( data_set_len_f32 * sizeof(float) ); // 20 * 4 = 80

static int pack_len_f32 = data_set_len_f32 * data_sets_num;   // 20 * 4 = 80
static int pack_len_u8  = data_set_len_u8  * data_sets_num;   // 80 * 4 = 320


static int window_width = 0;
static int window_height = 0;

uint32_t cart_speed = 10;

// UART 
static bool end_thread_01 = 0;
static bool UART = 0;
static bool new_data_ready = 0;
static char* device = "\\\\.\\COM3";
uint32_t baud_rate = 400000;

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
    ScrollingBuffer(int max_size = (2000)) {
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

void RealtimePlots(float* y_data_1, float* y_data_2, float* y_data_3, float* y_data_4) {

    static ScrollingBuffer sdata_1;
    static ScrollingBuffer sdata_2;
    static ScrollingBuffer sdata_3;
    static ScrollingBuffer sdata_4;
    static float t = 0;
    static int iter = 0;
    static int offset = 0;
    static int seconds = 10;

    // offset = offset % (seconds*pack_per_sec);
    // iter   = iter % data_set_len_f32;

    if( waiting_packet_num > 0){

        // if((waiting_packet_num > 2) ){//&& ((iter % 7) == 0)){
        //     if(iter<(data_set_len_f32-1)){
        //         for ( int i = 0; i < 2; i++) {
        //             t += ImGui::GetIO().DeltaTime;
        //             sdata_1.AddPoint(t, y_data_1[iter+(data_set_len_f32*offset)]);
        //             sdata_2.AddPoint(t, y_data_2[iter+(data_set_len_f32*offset)]);
        //             sdata_3.AddPoint(t, y_data_3[iter+(data_set_len_f32*offset)]);
        //             sdata_4.AddPoint(t, y_data_4[iter+(data_set_len_f32*offset)]);
        //             iter++;
        //         }

        //     }else if(iter == (data_set_len_f32-1)){
        //         t += ImGui::GetIO().DeltaTime;
        //         sdata_1.AddPoint(t, y_data_1[iter+(data_set_len_f32*offset)]);
        //         sdata_2.AddPoint(t, y_data_2[iter+(data_set_len_f32*offset)]);
        //         sdata_3.AddPoint(t, y_data_3[iter+(data_set_len_f32*offset)]);
        //         sdata_4.AddPoint(t, y_data_4[iter+(data_set_len_f32*offset)]);
        //         iter++;
        //     }
        //     if(iter == data_set_len_f32){
        //         waiting_packet_num--;
        //         offset++;
        //     }
        // } else {
        //     t += ImGui::GetIO().DeltaTime;
        //     sdata_1.AddPoint(t, y_data_1[iter+(data_set_len_f32*offset)]);
        //     sdata_2.AddPoint(t, y_data_2[iter+(data_set_len_f32*offset)]);
        //     sdata_3.AddPoint(t, y_data_3[iter+(data_set_len_f32*offset)]);
        //     sdata_4.AddPoint(t, y_data_4[iter+(data_set_len_f32*offset)]);
        //     iter++;
        //     if(iter == data_set_len_f32){
        //         waiting_packet_num--;
        //         offset++;
        //     }

        for ( int i = 0; i<100; i++) {
            if ( waiting_packet_num > i ) {
                t += ImGui::GetIO().DeltaTime;
                sdata_1.AddPoint(t, y_data_1[iter+(data_set_len_f32*offset)]);
                sdata_2.AddPoint(t, y_data_2[iter+(data_set_len_f32*offset)]);
                sdata_3.AddPoint(t, y_data_3[iter+(data_set_len_f32*offset)]);
                sdata_4.AddPoint(t, y_data_4[iter+(data_set_len_f32*offset)]);
                iter++;
                if(iter == data_set_len_f32){
                    waiting_packet_num--;
                    offset++;
                    offset = offset % (seconds * pack_per_sec);
                    iter   = iter   % data_set_len_f32;
                }
            } else {
                break;
            }
        }
    }

    //printf("%d\n",waiting_packet_num);

    static float history = 10.0f;
    ImGui::SetCursorPosX(267);
    ImGui::PushItemWidth(300);
    ImGui::SliderFloat("History",&history,1,30,"%.1f s",ImGuiSliderFlags_None);

    static ImPlotAxisFlags flags = ImPlotAxisFlags_None;
    if(sdata_1.Data.size()!=0){
        if (ImPlot::BeginPlot("##Scrolling_1", ImVec2(400,250))) {
            ImPlot::SetupAxes("time [s]", "Pitch", flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,-180,180);
            ImPlot::SetNextLineStyle(ImVec4(0.941, 0.0, 1.0, 0.784),2.0);
            ImPlot::PlotLine("Mouse Y", &sdata_1.Data[0].x, &sdata_1.Data[0].y, sdata_1.Data.size(), 0, sdata_1.Offset, 2*sizeof(float));
            ImPlot::EndPlot();
        }
    }
    ImGui::SameLine();
    if(sdata_2.Data.size()!=0){
        if (ImPlot::BeginPlot("##Scrolling_2", ImVec2(400,250))) {
            ImPlot::SetupAxes("time [s]", "Roll", flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,-180,180);
            ImPlot::SetNextLineStyle(ImVec4(0.447, 0.604, 0.452, 0.784),2.0);
            ImPlot::PlotLine("Mouse Y", &sdata_2.Data[0].x, &sdata_2.Data[0].y, sdata_2.Data.size(), 0, sdata_2.Offset, 2*sizeof(float));
            ImPlot::EndPlot();
        }
    }
    if(sdata_3.Data.size()!=0){
        if (ImPlot::BeginPlot("##Scrolling_3", ImVec2(400,250))) {
            ImPlot::SetupAxes("time [s]", "Cart 1 position", flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,-180,180);
            ImPlot::SetNextLineStyle(ImVec4(1.000, 0.0, 0.0, 0.784),2.0);
            ImPlot::PlotLine("Mouse Y", &sdata_3.Data[0].x, &sdata_3.Data[0].y, sdata_3.Data.size(), 0, sdata_3.Offset, 2*sizeof(float));
            ImPlot::EndPlot();
        }
    }
    ImGui::SameLine();
    if(sdata_4.Data.size()!=0){
        if (ImPlot::BeginPlot("##Scrolling_4", ImVec2(400,250))) {
            ImPlot::SetupAxes("time [s]", "Cart 2 position", flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,-180,180);
            ImPlot::SetNextLineStyle(ImVec4(0.853, 1.0, 0.0, 0.784),2.0);
            ImPlot::PlotLine("Mouse Y", &sdata_4.Data[0].x, &sdata_4.Data[0].y, sdata_4.Data.size(), 0, sdata_4.Offset, 2*sizeof(float));
            ImPlot::EndPlot();
        }
    }
    if(sdata_4.Data.size()!=0){
        if (ImGui::Button("Save data to file")) // TODO: add option of saving disaried amount of time like e.g. 1[s], 3[s], 10[s] ...
        {
            ImGui::LogToFile(-1,"data.txt");
            ImGui::LogText("time pitch roll cart_dist_1 cart_dist_2 \n");
            for (int i=0; i<sdata_4.Data.size();i++) {
                ImGui::LogText("%.3f %.3f %.3f %.3f %.3f \n", sdata_1.Data[i].x, sdata_1.Data[i].y, sdata_2.Data[i].y, sdata_3.Data[i].y, sdata_4.Data[i].y);
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
                            } break;
                            case Y : {
                                buf[1] = 'y';
                            } break;
                        }
                        memcpy(buf+2,&position,sizeof(position));
                    }break;

                    case MSG_SPEED: {

                        buf[0] = 's';

                        switch (axis) {
                            case X : {
                                buf[1] = 'x';
                            } break;
                            case Y : {
                                buf[1] = 'y';
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
                //std::this_thread::sleep_for(std::chrono::milliseconds(500));
                if(write_port(port, buf, sizeof(buf))!=0){
                    printf("Error in WRITE from serial port\n");
                    UART = 0;
                    break;
                }
                memset(buf,0,sizeof(buf));
                msg_type = MSG_DATA;




                printf("%d %d\n",pack_len_f32,pack_len_u8);

                int data_read = read_port( port, (uint8_t*)( Data_buffer + ( pack_len_f32 * UART_iter )), pack_len_u8 );
                if( data_read != pack_len_u8 ){
                    printf("Error in READ from serial port 1:\n data read: %d\n packet length: %d\n",data_read,pack_len_u8);
                    UART = 0;
                    break;
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
                static char buf_cart[66] = "";
                sprintf(buf_cart,"%d",cart_speed); 
                ImGui::InputText("cart speed [mm/s]",buf_cart, 64, ImGuiInputTextFlags_CharsDecimal);
                try {
                    cart_speed = std::stoi(buf_cart);
                } catch(std::invalid_argument& e) {
                    ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784),"Not valid argument");
                    cart_speed = 10;
                }
                if (ImGui::Button("JOG + x",ImVec2(50,50)))
                {
                    // do JOG + x direction
                    msg_type   = MSG_JOG;
                    direction  = POSITIV;
                    axis       = X;
                }
                ImGui::SameLine();
                if (ImGui::Button("JOG - x",ImVec2(50,50)))
                {
                    // do JOG - x direction
                    msg_type   = MSG_JOG;
                    direction  = NEGATIV;
                    axis       = X;
                }
                ImGui::SameLine();
                ImGui::Text("X direction");
                if (ImGui::Button("JOG + y",ImVec2(50,50)))
                {
                    // do JOG + y direction
                    msg_type   = MSG_JOG;
                    direction  = POSITIV;
                    axis       = Y;
                }
                ImGui::SameLine();
                if (ImGui::Button("JOG - y",ImVec2(50,50)))
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