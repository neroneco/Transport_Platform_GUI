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
#include "graphs.h"



static int window_width = 0;
static int window_height = 0;

uint32_t x_cart_speed = 10;
uint32_t y_cart_speed = 10;

uint32_t x_cart_pos = 215;
uint32_t y_cart_pos = 215;


// TODO get rid of global variables (if possible)
data_packet_struct      Data_Packet[10]     = {0};
system_status_struct    System_Status_Data  = {0};


// UART 
bool end_thread_01 = 0;
bool end_thread_02 = 0;
bool UART = 0;
char* device = "\\\\.\\COM3";
uint32_t baud_rate = 900000;


int waiting_packet_num = 0;


ScrollingBuffer sdata[28];
float t;

//bool Save_Data_Status[28];
bool Figures[3];


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



    // -+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+-
    // ------------- UART thread init --------------
    // -+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+-
    
    // UART init

    // thread creation
    std::thread thr_01(UART_communication);
    std::thread thr_02(graphs_store_data_thread, sdata, Data_Packet, 1);


    // -+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+-
    // ------------ Main Program Loop --------------
    // -+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+-
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

        // -+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+-
        // ----------------- Windows -------------------
        // -+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+--+-+-+-+-
        {
            // Figures window
            //ImGui::SetNextWindowSize(ImVec2(840,580));
            //ImGui::SetNextWindowPos(ImVec2(10,10));

            ImGui::Begin("Data to save",NULL,ImGuiWindowFlags_None);
                save_data_to_file( sdata );
            ImGui::End();


            ImGui::Begin( "Figures",NULL, ImGuiWindowFlags_None );
                ImGui::Checkbox("Carts",        &Figures[CARTS]);
                ImGui::Checkbox("Angles",       &Figures[ANGLES]);
                ImGui::Checkbox("Raw Sensors",  &Figures[RAW_SENSORS]);
                //graphs_store_data( sdata, Data_Packet );
            ImGui::End();

            if ( Figures[CARTS] ) {
                ImGui::Begin( "Carts",NULL, ImGuiWindowFlags_None );
                    graphs_carts( sdata );

                ImGui::End();
            }
            if ( Figures[ANGLES] ) {
                ImGui::Begin( "Angles",NULL, ImGuiWindowFlags_None );
                    graphs_angles( sdata );
                ImGui::End();
            }
            if ( Figures[RAW_SENSORS] ) {
                ImGui::Begin( "Raw Sensors",NULL, ImGuiWindowFlags_None );
                    graphs_raw_sensors( sdata );
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

            ImGui::Begin("System Status",NULL,ImGuiWindowFlags_None);
                print_system_status( &System_Status_Data );
            ImGui::End();

            // Cart control window
            ImGui::Begin("Cart Control",NULL,ImGuiWindowFlags_None);
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
            static float f = 0.0f;
            static int counter = 0;
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
    end_thread_02 = 1;

    UART = 0;
    while(!thr_01.joinable());
    if(thr_01.joinable()){
        printf("Ending UART thread\n");
        thr_01.join();
    }
    while(!thr_02.joinable());
    if(thr_02.joinable()){
        printf("Ending graph_store_data thread\n");
        thr_02.join();
    }

    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}
