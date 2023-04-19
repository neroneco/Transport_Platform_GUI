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



static int window_width = 0;
static int window_height = 0;

uint32_t x_cart_speed = 10;
uint32_t y_cart_speed = 10;

uint32_t x_cart_pos = 215;
uint32_t y_cart_pos = 215;



data_packet_struct      Data_Packet[10]  = {0};


// UART 
bool end_thread_01 = 0;
bool UART = 0;
char* device = "\\\\.\\COM3";
uint32_t baud_rate = 900000;


int waiting_packet_num = 0;

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
enum Data_Names_enum {
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

const char* Data_Names[] = {
    "carts_pos_x",
    "carts_pos_y",
    "carts_vel_x",
    "carts_vel_y",
    "carts_acc_x",
    "carts_acc_y",
    "mpu9250_acce_x",
    "mpu9250_acce_y",
    "mpu9250_acce_z",
    "mpu9250_gyro_x",
    "mpu9250_gyro_y",
    "mpu9250_gyro_z",
    "mpu6886_acce_x",
    "mpu6886_acce_y",
    "mpu6886_acce_z",
    "mpu6886_gyro_x",
    "mpu6886_gyro_y",
    "mpu6886_gyro_z",
    "pitch_no_filter",
    "roll_no_filter",
    "pitch_complementary",
    "roll_complementary",
    "pitch_alfa_beta",
    "roll_alfa_beta",
    "pitch_kalman",
    "roll_kalman",
    "pitch",
    "roll"
};


enum Figures_Names {
    CARTS,
    ANGLES,
    RAW_SENSORS
};


static ScrollingBuffer sdata[28];
static float t;

static bool Save_Data_Status[28];
static bool Figures[3];

void RealtimePlots(void) {

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
}


bool     direction     =  0;
bool     axis          =  0;
uint32_t speed         = 10;
uint32_t position      = 10;

int msg_type = MSG_NONE;


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

            ImGui::Begin("Data to save",NULL,ImGuiWindowFlags_None);
                ImGui::Checkbox("carts_pos_x", &Save_Data_Status[CARTS_POS_X]);
                ImGui::Checkbox("carts_pos_y", &Save_Data_Status[CARTS_POS_Y]);
                ImGui::Checkbox("carts_vel_x", &Save_Data_Status[CARTS_VEL_X]);
                ImGui::Checkbox("carts_vel_y", &Save_Data_Status[CARTS_VEL_Y]);
                ImGui::Checkbox("carts_acc_x", &Save_Data_Status[CARTS_ACC_X]);
                ImGui::Checkbox("carts_acc_y", &Save_Data_Status[CARTS_ACC_Y]);
                ImGui::Checkbox("mpu9250_acce_x", &Save_Data_Status[MPU9250_ACCE_X]);
                ImGui::Checkbox("mpu9250_acce_y", &Save_Data_Status[MPU9250_ACCE_Y]);
                ImGui::Checkbox("mpu9250_acce_z", &Save_Data_Status[MPU9250_ACCE_Z]);
                ImGui::Checkbox("mpu9250_gyro_x", &Save_Data_Status[MPU9250_GYRO_X]);
                ImGui::Checkbox("mpu9250_gyro_y", &Save_Data_Status[MPU9250_GYRO_Y]);
                ImGui::Checkbox("mpu9250_gyro_z", &Save_Data_Status[MPU9250_GYRO_Z]);
                ImGui::Checkbox("mpu6886_acce_x", &Save_Data_Status[MPU6886_ACCE_X]);
                ImGui::Checkbox("mpu6886_acce_y", &Save_Data_Status[MPU6886_ACCE_Y]);
                ImGui::Checkbox("mpu6886_acce_z", &Save_Data_Status[MPU6886_ACCE_Z]);
                ImGui::Checkbox("mpu6886_gyro_x", &Save_Data_Status[MPU6886_GYRO_X]);
                ImGui::Checkbox("mpu6886_gyro_y", &Save_Data_Status[MPU6886_GYRO_Y]);
                ImGui::Checkbox("mpu6886_gyro_z", &Save_Data_Status[MPU6886_GYRO_Z]);
                ImGui::Checkbox("pitch_no_filter", &Save_Data_Status[PITCH_NO_FILTER]);
                ImGui::Checkbox("roll_no_filter", &Save_Data_Status[ROLL_NO_FILTER]);
                ImGui::Checkbox("pitch_complementary", &Save_Data_Status[PITCH_COMPLEMENTARY]);
                ImGui::Checkbox("roll_complementary", &Save_Data_Status[ROLL_COMPLEMENTARY]);
                ImGui::Checkbox("pitch_alfa_beta", &Save_Data_Status[PITCH_ALFA_BETA]);
                ImGui::Checkbox("roll_alfa_beta", &Save_Data_Status[ROLL_ALFA_BETA]);
                ImGui::Checkbox("pitch_kalman", &Save_Data_Status[PITCH_KALMAN]);
                ImGui::Checkbox("roll_kalman", &Save_Data_Status[ROLL_KALMAN]);
                ImGui::Checkbox("pitch", &Save_Data_Status[PITCH]);
                ImGui::Checkbox("roll", &Save_Data_Status[ROLL]);
                
                // TODO: add option of saving disaried amount of time like e.g. 1[s], 3[s], 10[s] ...
                static bool no_data;
                if (ImGui::Button("Save data to file"))
                {
                    if(sdata[CARTS_POS_X].Data.size()!=0)
                    {
                        no_data = 0;
                        ImGui::LogToFile(-1,"data.txt");
                        for ( int k=0; k<28; k++ ) 
                        {
                            if ( Save_Data_Status[k] )
                            {
                                ImGui::LogText("%s ",Data_Names[k]);
                            }
                        }
                        for (int i=0; i<sdata[CARTS_POS_X].Data.size();i++) 
                        {
                            ImGui::LogText("\n");
                            for ( int k=0; k<28; k++ ) 
                            {
                                if ( Save_Data_Status[k] )
                                {
                                    ImGui::LogText("%.3f ",sdata[k].Data[i].y);
                                }
                            }
                        }
                        ImGui::LogFinish();
                    } else {
                        no_data = 1;
                    }
                }
                if (no_data) {
                    ImGui::TextColored(ImVec4(0.941, 0.0, 1.0, 0.784),"No data to save");
                }
            ImGui::End();


            ImGui::Begin( "Figures",NULL, ImGuiWindowFlags_None );
                ImGui::Checkbox("Carts", &Figures[CARTS]);
                ImGui::Checkbox("Angles", &Figures[ANGLES]);
                ImGui::Checkbox("Raw Sensors", &Figures[RAW_SENSORS]);
                RealtimePlots();
            ImGui::End();

            if ( Figures[CARTS] ) {
                ImGui::Begin( "Carts",NULL, ImGuiWindowFlags_None );
                static float history = 10.0f;
                ImGui::PushItemWidth(300);
                ImGui::SliderFloat("History",&history,1,30,"%.1f s",ImGuiSliderFlags_None);

                static ImPlotAxisFlags flags = ImPlotAxisFlags_None;
                if(sdata[CARTS_POS_X].Data.size()!=0){
                    if (ImPlot::BeginPlot("##Scrolling_11", ImVec2(400,250))) {
                        ImPlot::SetupAxes("time [s]", "Position [mm]", flags, flags);
                        ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
                        ImPlot::SetupAxisLimits(ImAxis_Y1,0,365);
                        //ImPlot::SetNextLineStyle(ImVec4(1.000, 0.0, 0.0, 0.784),2.0);
                        ImPlot::PlotLine("Cart X", &sdata[CARTS_POS_X].Data[0].x, &sdata[CARTS_POS_X].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("Cart Y", &sdata[CARTS_POS_Y].Data[0].x, &sdata[CARTS_POS_Y].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::EndPlot();
                    }
                }
                if(sdata[CARTS_VEL_X].Data.size()!=0){
                    if (ImPlot::BeginPlot("##Scrolling_12", ImVec2(400,250))) {
                        ImPlot::SetupAxes("time [s]", "Velocity [mm/s]", flags, flags);
                        ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
                        ImPlot::SetupAxisLimits(ImAxis_Y1,0,165);
                        //ImPlot::SetNextLineStyle(ImVec4(0.853, 1.0, 0.0, 0.784),2.0);
                        ImPlot::PlotLine("Cart X", &sdata[CARTS_VEL_X].Data[0].x, &sdata[CARTS_VEL_X].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("Cart Y", &sdata[CARTS_VEL_Y].Data[0].x, &sdata[CARTS_VEL_Y].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::EndPlot();
                    }
                }
                if(sdata[CARTS_ACC_X].Data.size()!=0){
                    if (ImPlot::BeginPlot("##Scrolling_13", ImVec2(400,250))) {
                        ImPlot::SetupAxes("time [s]", "Acceleration [mm/s^2]", flags, flags);
                        ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
                        ImPlot::SetupAxisLimits(ImAxis_Y1,0,165);
                        //ImPlot::SetNextLineStyle(ImVec4(0.853, 1.0, 0.0, 0.784),2.0);
                        ImPlot::PlotLine("Cart X", &sdata[CARTS_ACC_X].Data[0].x, &sdata[CARTS_ACC_X].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("Cart Y", &sdata[CARTS_ACC_Y].Data[0].x, &sdata[CARTS_ACC_Y].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::EndPlot();
                    }
                }
                ImGui::End();
            }
            if ( Figures[ANGLES] ) {
                ImGui::Begin( "Angles",NULL, ImGuiWindowFlags_None );
                static float history = 10.0f;
                ImGui::PushItemWidth(300);
                ImGui::SliderFloat("History",&history,1,30,"%.1f s",ImGuiSliderFlags_None);

                static ImPlotAxisFlags flags = ImPlotAxisFlags_None;
                if(sdata[PITCH_NO_FILTER].Data.size()!=0){
                    if (ImPlot::BeginPlot("##Scrolling_21", ImVec2(400,250))) {
                        ImPlot::SetupAxes("time [s]", "Pitch [deg]", flags, flags);
                        ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
                        ImPlot::SetupAxisLimits(ImAxis_Y1,-180,180);
                        //ImPlot::SetNextLineStyle(ImVec4(0.941, 0.0, 1.0, 0.784),2.0);
                        ImPlot::PlotLine("no filter", &sdata[PITCH_NO_FILTER].Data[0].x, &sdata[PITCH_NO_FILTER].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("complementary filter", &sdata[PITCH_COMPLEMENTARY].Data[0].x, &sdata[PITCH_COMPLEMENTARY].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("alfa-beta filter", &sdata[PITCH_ALFA_BETA].Data[0].x, &sdata[PITCH_ALFA_BETA].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("kalman filter", &sdata[PITCH_KALMAN].Data[0].x, &sdata[PITCH_KALMAN].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));

                        ImPlot::EndPlot();
                    }
                }
                ImGui::SameLine();
                if(sdata[ROLL_NO_FILTER].Data.size()!=0){
                    if (ImPlot::BeginPlot("##Scrolling_22", ImVec2(400,250))) {
                        ImPlot::SetupAxes("time [s]", "Roll [deg]", flags, flags);
                        ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
                        ImPlot::SetupAxisLimits(ImAxis_Y1,-180,180);
                        //ImPlot::SetNextLineStyle(ImVec4(0.447, 0.604, 0.452, 0.784),2.0);
                        ImPlot::PlotLine("no filter", &sdata[ROLL_NO_FILTER].Data[0].x, &sdata[ROLL_NO_FILTER].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("complementary filter", &sdata[ROLL_COMPLEMENTARY].Data[0].x, &sdata[ROLL_COMPLEMENTARY].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("alfa-beta filter", &sdata[ROLL_ALFA_BETA].Data[0].x, &sdata[ROLL_ALFA_BETA].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("kalman filter", &sdata[ROLL_KALMAN].Data[0].x, &sdata[ROLL_KALMAN].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::EndPlot();
                    }
                }
                ImGui::End();
            }
            if ( Figures[RAW_SENSORS] ) {
                ImGui::Begin( "Raw Sensors",NULL, ImGuiWindowFlags_None );
                static float history = 10.0f;
                ImGui::PushItemWidth(300);
                ImGui::SliderFloat("History",&history,1,30,"%.1f s",ImGuiSliderFlags_None);

                static ImPlotAxisFlags flags = ImPlotAxisFlags_None;
                if(sdata[MPU9250_ACCE_X].Data.size()!=0){
                    if (ImPlot::BeginPlot("##Scrolling_31", ImVec2(400,250))) {
                        ImPlot::SetupAxes("time [s]", "Acceleration X [m/s^2]", flags, flags);
                        ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
                        ImPlot::SetupAxisLimits(ImAxis_Y1,0,365);
                        //ImPlot::SetNextLineStyle(ImVec4(1.000, 0.0, 0.0, 0.784),2.0);
                        ImPlot::PlotLine("mpu9250", &sdata[MPU9250_ACCE_X].Data[0].x, &sdata[MPU9250_ACCE_X].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("mpu6886", &sdata[MPU6886_ACCE_X].Data[0].x, &sdata[MPU6886_ACCE_X].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::EndPlot();
                    }
                }
                ImGui::SameLine();
                if(sdata[MPU9250_ACCE_Y].Data.size()!=0){
                    if (ImPlot::BeginPlot("##Scrolling_32", ImVec2(400,250))) {
                        ImPlot::SetupAxes("time [s]", "Acceleration Y [m/s^2]", flags, flags);
                        ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
                        ImPlot::SetupAxisLimits(ImAxis_Y1,0,165);
                        //ImPlot::SetNextLineStyle(ImVec4(0.853, 1.0, 0.0, 0.784),2.0);
                        ImPlot::PlotLine("mpu9250", &sdata[MPU9250_ACCE_Y].Data[0].x, &sdata[MPU9250_ACCE_Y].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("mpu6886", &sdata[MPU6886_ACCE_Y].Data[0].x, &sdata[MPU6886_ACCE_Y].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::EndPlot();
                    }
                }
                ImGui::SameLine();
                if(sdata[MPU9250_ACCE_Z].Data.size()!=0){
                    if (ImPlot::BeginPlot("##Scrolling_33", ImVec2(400,250))) {
                        ImPlot::SetupAxes("time [s]", "Acceleration Z [m/s^2]", flags, flags);
                        ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
                        ImPlot::SetupAxisLimits(ImAxis_Y1,0,165);
                        //ImPlot::SetNextLineStyle(ImVec4(0.853, 1.0, 0.0, 0.784),2.0);
                        ImPlot::PlotLine("mpu9250", &sdata[MPU9250_ACCE_Z].Data[0].x, &sdata[MPU9250_ACCE_Z].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("mpu6886", &sdata[MPU6886_ACCE_Z].Data[0].x, &sdata[MPU6886_ACCE_Z].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::EndPlot();
                    }
                }
                if(sdata[MPU9250_GYRO_X].Data.size()!=0){
                    if (ImPlot::BeginPlot("##Scrolling_34", ImVec2(400,250))) {
                        ImPlot::SetupAxes("time [s]", "Gyro X [deg/s]", flags, flags);
                        ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
                        ImPlot::SetupAxisLimits(ImAxis_Y1,0,365);
                        //ImPlot::SetNextLineStyle(ImVec4(1.000, 0.0, 0.0, 0.784),2.0);
                        ImPlot::PlotLine("mpu9250", &sdata[MPU9250_GYRO_X].Data[0].x, &sdata[MPU9250_GYRO_X].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("mpu6886", &sdata[MPU6886_GYRO_X].Data[0].x, &sdata[MPU6886_GYRO_X].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::EndPlot();
                    }
                }
                ImGui::SameLine();
                if(sdata[MPU9250_GYRO_Y].Data.size()!=0){
                    if (ImPlot::BeginPlot("##Scrolling_35", ImVec2(400,250))) {
                        ImPlot::SetupAxes("time [s]", "Gyro Y [deg/s]", flags, flags);
                        ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
                        ImPlot::SetupAxisLimits(ImAxis_Y1,0,165);
                        //ImPlot::SetNextLineStyle(ImVec4(0.853, 1.0, 0.0, 0.784),2.0);
                        ImPlot::PlotLine("mpu9250", &sdata[MPU9250_GYRO_Y].Data[0].x, &sdata[MPU9250_GYRO_Y].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("mpu6886", &sdata[MPU6886_GYRO_Y].Data[0].x, &sdata[MPU6886_GYRO_Y].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::EndPlot();
                    }
                }
                ImGui::SameLine();
                if(sdata[MPU9250_GYRO_Z].Data.size()!=0){
                    if (ImPlot::BeginPlot("##Scrolling_36", ImVec2(400,250))) {
                        ImPlot::SetupAxes("time [s]", "Gyro Z [deg/s]", flags, flags);
                        ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
                        ImPlot::SetupAxisLimits(ImAxis_Y1,0,165);
                        //ImPlot::SetNextLineStyle(ImVec4(0.853, 1.0, 0.0, 0.784),2.0);
                        ImPlot::PlotLine("mpu9250", &sdata[MPU9250_GYRO_Z].Data[0].x, &sdata[MPU9250_GYRO_Z].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::PlotLine("mpu6886", &sdata[MPU6886_GYRO_Z].Data[0].x, &sdata[MPU6886_GYRO_Z].Data[0].y, sdata[PITCH].Data.size(), 0, sdata[PITCH].Offset, 2*sizeof(float));
                        ImPlot::EndPlot();
                    }
                }
                ImGui::End();
            }

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
