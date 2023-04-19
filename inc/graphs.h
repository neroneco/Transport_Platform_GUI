#ifndef GRAPHS_H
#define GRAPHS_H

// utility structure for realtime plot
struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    ImVector<ImVec2> Data;
    ScrollingBuffer(int max_size = (7500)) {
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

extern ScrollingBuffer sdata[28];
extern float t;

extern bool Save_Data_Status[28];
extern bool Figures[3];

void graphs_store_data( ScrollingBuffer *scroll_data, data_packet_struct* data_pack ) {

    static int iter = 0;
    static int packet_num = 0;
    static int seconds = 10;
    static uint64_t time_u64 = 0;
    static float time_real = 0;


    if( waiting_packet_num > 0){
        for ( int i = 0; i<100; i++) {
            if ( 2*waiting_packet_num > i ) {
                time_u64 += 4;
                time_real = ((float)time_u64)*(0.001f);
                t = time_real;

                scroll_data[CARTS_POS_X].AddPoint(time_real,          data_pack[packet_num].carts_pos_x[iter]);
                scroll_data[CARTS_POS_Y].AddPoint(time_real,          data_pack[packet_num].carts_pos_y[iter]);
                scroll_data[CARTS_VEL_X].AddPoint(time_real,          data_pack[packet_num].carts_vel_x[iter]);
                scroll_data[CARTS_VEL_Y].AddPoint(time_real,          data_pack[packet_num].carts_vel_y[iter]);
                scroll_data[CARTS_ACC_X].AddPoint(time_real,          data_pack[packet_num].carts_acc_x[iter]);
                scroll_data[CARTS_ACC_Y].AddPoint(time_real,          data_pack[packet_num].carts_acc_y[iter]);
                scroll_data[MPU9250_ACCE_X].AddPoint(time_real,       data_pack[packet_num].mpu9250_acce_x[iter]);
                scroll_data[MPU9250_ACCE_Y].AddPoint(time_real,       data_pack[packet_num].mpu9250_acce_y[iter]);
                scroll_data[MPU9250_ACCE_Z].AddPoint(time_real,       data_pack[packet_num].mpu9250_acce_z[iter]);
                scroll_data[MPU9250_GYRO_X].AddPoint(time_real,       data_pack[packet_num].mpu9250_gyro_x[iter]);
                scroll_data[MPU9250_GYRO_Y].AddPoint(time_real,       data_pack[packet_num].mpu9250_gyro_y[iter]);
                scroll_data[MPU9250_GYRO_Z].AddPoint(time_real,       data_pack[packet_num].mpu9250_gyro_z[iter]);
                scroll_data[MPU6886_ACCE_X].AddPoint(time_real,       data_pack[packet_num].mpu6886_acce_x[iter]);
                scroll_data[MPU6886_ACCE_Y].AddPoint(time_real,       data_pack[packet_num].mpu6886_acce_y[iter]);
                scroll_data[MPU6886_ACCE_Z].AddPoint(time_real,       data_pack[packet_num].mpu6886_acce_z[iter]);
                scroll_data[MPU6886_GYRO_X].AddPoint(time_real,       data_pack[packet_num].mpu6886_gyro_x[iter]);
                scroll_data[MPU6886_GYRO_Y].AddPoint(time_real,       data_pack[packet_num].mpu6886_gyro_y[iter]);
                scroll_data[MPU6886_GYRO_Z].AddPoint(time_real,       data_pack[packet_num].mpu6886_gyro_z[iter]);
                scroll_data[PITCH_NO_FILTER].AddPoint(time_real,      data_pack[packet_num].pitch_no_filter[iter]);
                scroll_data[ROLL_NO_FILTER].AddPoint(time_real,       data_pack[packet_num].roll_no_filter[iter]);
                scroll_data[PITCH_COMPLEMENTARY].AddPoint(time_real,  data_pack[packet_num].pitch_complementary[iter]);
                scroll_data[ROLL_COMPLEMENTARY].AddPoint(time_real,   data_pack[packet_num].roll_complementary[iter]);
                scroll_data[PITCH_ALFA_BETA].AddPoint(time_real,      data_pack[packet_num].pitch_alfa_beta[iter]);
                scroll_data[ROLL_ALFA_BETA].AddPoint(time_real,       data_pack[packet_num].roll_alfa_beta[iter]);
                scroll_data[PITCH_KALMAN].AddPoint(time_real,         data_pack[packet_num].pitch_kalman[iter]);
                scroll_data[ROLL_KALMAN].AddPoint(time_real,          data_pack[packet_num].roll_kalman[iter]);
                scroll_data[PITCH].AddPoint(time_real,                data_pack[packet_num].pitch[iter]);
                scroll_data[ROLL].AddPoint(time_real,                 data_pack[packet_num].roll[iter]);

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

void graphs_carts( ScrollingBuffer* scroll_data ) {
    static float history = 10.0f;
    ImGui::PushItemWidth(300);
    ImGui::SliderFloat("History",&history,1,60,"%.1f s",ImGuiSliderFlags_None);

    static ImPlotAxisFlags flags = ImPlotAxisFlags_None;
    if(scroll_data[CARTS_POS_X].Data.size()!=0){
        if (ImPlot::BeginPlot("##Scrolling_11", ImVec2(400,250))) {
            ImPlot::SetupAxes("time [s]", "Position [mm]", flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 365);
            //ImPlot::SetNextLineStyle(ImVec4(1.000, 0.0, 0.0, 0.784),2.0);
            ImPlot::PlotLine("Cart X", &scroll_data[CARTS_POS_X].Data[0].x, &scroll_data[CARTS_POS_X].Data[0].y, scroll_data[PITCH].Data.size(), 0, scroll_data[PITCH].Offset, 2*sizeof(float));
            ImPlot::PlotLine("Cart Y", &scroll_data[CARTS_POS_Y].Data[0].x, &scroll_data[CARTS_POS_Y].Data[0].y, scroll_data[PITCH].Data.size(), 0, scroll_data[PITCH].Offset, 2*sizeof(float));
            ImPlot::EndPlot();
        }
    }
    if(scroll_data[CARTS_VEL_X].Data.size()!=0){
        if (ImPlot::BeginPlot("##Scrolling_12", ImVec2(400,250))) {
            ImPlot::SetupAxes("time [s]", "Velocity [mm/s]", flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,0,165);
            //ImPlot::SetNextLineStyle(ImVec4(0.853, 1.0, 0.0, 0.784),2.0);
            ImPlot::PlotLine("Cart X", &scroll_data[CARTS_VEL_X].Data[0].x, &scroll_data[CARTS_VEL_X].Data[0].y, scroll_data[PITCH].Data.size(), 0, scroll_data[PITCH].Offset, 2*sizeof(float));
            ImPlot::PlotLine("Cart Y", &scroll_data[CARTS_VEL_Y].Data[0].x, &scroll_data[CARTS_VEL_Y].Data[0].y, scroll_data[PITCH].Data.size(), 0, scroll_data[PITCH].Offset, 2*sizeof(float));
            ImPlot::EndPlot();
        }
    }
    if(scroll_data[CARTS_ACC_X].Data.size()!=0){
        if (ImPlot::BeginPlot("##Scrolling_13", ImVec2(400,250))) {
            ImPlot::SetupAxes("time [s]", "Acceleration [mm/s^2]", flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,0,165);
            //ImPlot::SetNextLineStyle(ImVec4(0.853, 1.0, 0.0, 0.784),2.0);
            ImPlot::PlotLine("Cart X", &scroll_data[CARTS_ACC_X].Data[0].x, &scroll_data[CARTS_ACC_X].Data[0].y, scroll_data[PITCH].Data.size(), 0, scroll_data[PITCH].Offset, 2*sizeof(float));
            ImPlot::PlotLine("Cart Y", &scroll_data[CARTS_ACC_Y].Data[0].x, &scroll_data[CARTS_ACC_Y].Data[0].y, scroll_data[PITCH].Data.size(), 0, scroll_data[PITCH].Offset, 2*sizeof(float));
            ImPlot::EndPlot();
        }
    }
}

void graphs_angles( ScrollingBuffer* sdata ) {
    static float history = 10.0f;
    ImGui::PushItemWidth(300);
    ImGui::SliderFloat("History",&history,1,60,"%.1f s",ImGuiSliderFlags_None);

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
}

void graphs_raw_sensors( ScrollingBuffer* sdata ) {
    static float history = 10.0f;
    ImGui::PushItemWidth(300);
    ImGui::SliderFloat("History",&history,1,60,"%.1f s",ImGuiSliderFlags_None);

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
}

void save_data_to_file( ScrollingBuffer* sdata )
{
    static bool Save_Data_Status[28];
    ImGui::Checkbox("carts_pos_x",          &Save_Data_Status[CARTS_POS_X]);
    ImGui::Checkbox("carts_pos_y",          &Save_Data_Status[CARTS_POS_Y]);
    ImGui::Checkbox("carts_vel_x",          &Save_Data_Status[CARTS_VEL_X]);
    ImGui::Checkbox("carts_vel_y",          &Save_Data_Status[CARTS_VEL_Y]);
    ImGui::Checkbox("carts_acc_x",          &Save_Data_Status[CARTS_ACC_X]);
    ImGui::Checkbox("carts_acc_y",          &Save_Data_Status[CARTS_ACC_Y]);
    ImGui::Checkbox("mpu9250_acce_x",       &Save_Data_Status[MPU9250_ACCE_X]);
    ImGui::Checkbox("mpu9250_acce_y",       &Save_Data_Status[MPU9250_ACCE_Y]);
    ImGui::Checkbox("mpu9250_acce_z",       &Save_Data_Status[MPU9250_ACCE_Z]);
    ImGui::Checkbox("mpu9250_gyro_x",       &Save_Data_Status[MPU9250_GYRO_X]);
    ImGui::Checkbox("mpu9250_gyro_y",       &Save_Data_Status[MPU9250_GYRO_Y]);
    ImGui::Checkbox("mpu9250_gyro_z",       &Save_Data_Status[MPU9250_GYRO_Z]);
    ImGui::Checkbox("mpu6886_acce_x",       &Save_Data_Status[MPU6886_ACCE_X]);
    ImGui::Checkbox("mpu6886_acce_y",       &Save_Data_Status[MPU6886_ACCE_Y]);
    ImGui::Checkbox("mpu6886_acce_z",       &Save_Data_Status[MPU6886_ACCE_Z]);
    ImGui::Checkbox("mpu6886_gyro_x",       &Save_Data_Status[MPU6886_GYRO_X]);
    ImGui::Checkbox("mpu6886_gyro_y",       &Save_Data_Status[MPU6886_GYRO_Y]);
    ImGui::Checkbox("mpu6886_gyro_z",       &Save_Data_Status[MPU6886_GYRO_Z]);
    ImGui::Checkbox("pitch_no_filter",      &Save_Data_Status[PITCH_NO_FILTER]);
    ImGui::Checkbox("roll_no_filter",       &Save_Data_Status[ROLL_NO_FILTER]);
    ImGui::Checkbox("pitch_complementary",  &Save_Data_Status[PITCH_COMPLEMENTARY]);
    ImGui::Checkbox("roll_complementary",   &Save_Data_Status[ROLL_COMPLEMENTARY]);
    ImGui::Checkbox("pitch_alfa_beta",      &Save_Data_Status[PITCH_ALFA_BETA]);
    ImGui::Checkbox("roll_alfa_beta",       &Save_Data_Status[ROLL_ALFA_BETA]);
    ImGui::Checkbox("pitch_kalman",         &Save_Data_Status[PITCH_KALMAN]);
    ImGui::Checkbox("roll_kalman",          &Save_Data_Status[ROLL_KALMAN]);
    ImGui::Checkbox("pitch",                &Save_Data_Status[PITCH]);
    ImGui::Checkbox("roll",                 &Save_Data_Status[ROLL]);
    
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
}


#endif