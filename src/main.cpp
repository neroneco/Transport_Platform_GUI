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

// UART 
static bool end_thread_01 = 0;
static bool UART = 0;
static bool new_data_ready = 0;
const char* device = "\\\\.\\COM3";
uint32_t baud_rate = 900000;

static int UART_iter = 0;
static SSIZE_T received;
static float Pitch[610] = {0};
static float Roll[610] = {0};
static float Cart_dist_1[610] = {0};
static float Cart_dist_2[610] = {0};
static float time_data[610] = {0};

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

    offset = offset % 10;
    iter   = iter % 60;

    if( waiting_packet_num > 0){
        if((waiting_packet_num > 1) && ((iter % 7) == 0)){
            if(iter<59){
                t += ImGui::GetIO().DeltaTime;
                sdata_1.AddPoint(t, y_data_1[iter+(60*offset)]);
                sdata_2.AddPoint(t, y_data_2[iter+(60*offset)]);
                sdata_3.AddPoint(t, y_data_3[iter+(60*offset)]);
                sdata_4.AddPoint(t, y_data_4[iter+(60*offset)]);
                iter++;
                t += ImGui::GetIO().DeltaTime;
                sdata_1.AddPoint(t, y_data_1[iter+(60*offset)]);
                sdata_2.AddPoint(t, y_data_2[iter+(60*offset)]);
                sdata_3.AddPoint(t, y_data_3[iter+(60*offset)]);
                sdata_4.AddPoint(t, y_data_4[iter+(60*offset)]);
                iter++;
            }else if(iter == 59){
                t += ImGui::GetIO().DeltaTime;
                sdata_1.AddPoint(t, y_data_1[iter+(60*offset)]);
                sdata_2.AddPoint(t, y_data_2[iter+(60*offset)]);
                sdata_3.AddPoint(t, y_data_3[iter+(60*offset)]);
                sdata_4.AddPoint(t, y_data_4[iter+(60*offset)]);
                iter++;
            }
            if(iter == 60){
                waiting_packet_num--;
                offset++;
            }         
        } else {
            t += ImGui::GetIO().DeltaTime;
            sdata_1.AddPoint(t, y_data_1[iter+(60*offset)]);
            sdata_2.AddPoint(t, y_data_2[iter+(60*offset)]);
            sdata_3.AddPoint(t, y_data_3[iter+(60*offset)]);
            sdata_4.AddPoint(t, y_data_4[iter+(60*offset)]);
            iter++;
            if(iter == 60){
                waiting_packet_num--;
                offset++;
            }
        }
    }

    static float history = 10.0f;
    ImGui::SetCursorPosX(267);
    ImGui::PushItemWidth(300);
    ImGui::SliderFloat("History",&history,1,30,"%.1f s",ImGuiSliderFlags_None);

    static ImPlotAxisFlags flags = ImPlotAxisFlags_None;
    printf("%d\n",waiting_packet_num);
    if(sdata_1.Data.size()!=0){
        if (ImPlot::BeginPlot("##Scrolling_1", ImVec2(400,250))) {
            ImPlot::SetupAxes("time [s]", "Pitch", flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,-6,6);
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
            ImPlot::SetupAxisLimits(ImAxis_Y1,-6,6);
            ImPlot::SetNextLineStyle(ImVec4(0.447, 0.604, 0.452, 0.784),2.0);
            ImPlot::PlotLine("Mouse Y", &sdata_2.Data[0].x, &sdata_2.Data[0].y, sdata_2.Data.size(), 0, sdata_2.Offset, 2*sizeof(float));
            ImPlot::EndPlot();
        }
    }
    if(sdata_3.Data.size()!=0){
        if (ImPlot::BeginPlot("##Scrolling_3", ImVec2(400,250))) {
            ImPlot::SetupAxes("time [s]", "Cart 1 position", flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,-6,6);
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
            ImPlot::SetupAxisLimits(ImAxis_Y1,-6,6);
            ImPlot::SetNextLineStyle(ImVec4(0.853, 1.0, 0.0, 0.784),2.0);
            ImPlot::PlotLine("Mouse Y", &sdata_4.Data[0].x, &sdata_4.Data[0].y, sdata_4.Data.size(), 0, sdata_4.Offset, 2*sizeof(float));
            ImPlot::EndPlot();
        }
    }
}

//-----------------------------------------------------------------------------


void UART_communication(void)
{
    while(!end_thread_01){
        if(UART){
                HANDLE port = open_serial_port(device, baud_rate);
                if (port == INVALID_HANDLE_VALUE) { 
                    printf("INVALID_HANDLE_VALUE for UART\n");
                    UART = 0; 
                }

                while(UART){
                    UART_iter = UART_iter % 10; 
                    static uint8_t TX = 'x';
                    PurgeComm(port, PURGE_RXCLEAR);
                    if(write_port(port, &TX, 1)!=0){
                        printf("Error in WRITE from serial port\n");
                        UART = 0;
                        break;
                    }
                    if(read_port(port, (uint8_t*)(Pitch+(60*UART_iter)), 240) != 240){
                        printf("Error in READ from serial port\n");
                        UART = 0;
                        break;
                    }
                    if(read_port(port, (uint8_t*)(Roll+(60*UART_iter)), 240) != 240){
                        printf("Error in READ from serial port\n");
                        UART = 0;
                        break;
                    }
                    if(read_port(port, (uint8_t*)(Cart_dist_1+(60*UART_iter)), 240) != 240){
                        printf("Error in READ from serial port\n");
                        UART = 0;
                        break;
                    }
                    if(read_port(port, (uint8_t*)(Cart_dist_2+(60*UART_iter)), 240) != 240){
                        printf("Error in READ from serial port\n");
                        UART = 0;
                        break;
                    }
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


            // Figures                                                
            ImGui::SetNextWindowSize(ImVec2(840,580));
            ImGui::SetNextWindowPos(ImVec2(10,10));
            ImGui::Begin("Figures",NULL,  ImGuiWindowFlags_NoResize  |
                                          ImGuiWindowFlags_NoMove    |
                                          ImGuiWindowFlags_NoCollapse ); // Create a window called "Hello, world!" and append into it.

            //if (ImPlot::BeginSubplots("##ItemSharing", rows, cols, ImVec2(-1,-1), flags)) {
                //for (int i = 0; i < rows*cols; ++i) {
                    // if (ImPlot::BeginPlot("My Plot 1",ImVec2(200,200)),ImPlotFlags_NoTitle) {
                    //     ImPlot::SetupAxes(NULL,NULL,0,ImPlotAxisFlags_RangeFit);
                    //     ImPlot::SetupAxesLimits(1, 1000, -180, 180);
                    //     ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                    //     ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 1, 1000);
                    //     ImPlot::SetNextLineStyle(ImVec4(0.941, 0.0, 1.0, 0.784),2.0);
                    //     ImPlot::PlotLine("My Line Plot 1", time_data, Pitch, 1000);
                    //     ImPlot::EndPlot();
                    // }
                RealtimePlots(Pitch,Roll,Cart_dist_1,Cart_dist_2);
                //}
                //ImPlot::EndSubplots();
            //}
            ImGui::End();

            // UART window
            ImGui::Begin("Serial Comunication",NULL,ImGuiWindowFlags_None);
            ImGui::Checkbox("UART", &UART);
            ImGui::End();


            // Help window
            ImGui::Begin("Help",NULL,ImGuiWindowFlags_None); 
            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("ImGui Demo Window", &show_demo_ImGuiWindow);      // Edit bools storing our window open/close state
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