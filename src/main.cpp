#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <ctime>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include "CRC.h"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> 
#include "boost/asio.hpp"
using namespace boost::asio;
using std::string;
using namespace std;

//Conversor Hex
const unsigned char tbh[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

//Converte uma string para hexadecimal (2 caracteres)
void concathex2(char  array[], unsigned char bt)
{
    char hex[2];

    hex[0] = tbh[ bt/16];
    hex[1] = tbh[ bt%16];

    strncat(array, &hex[0], 1);
    strncat(array, &hex[1], 1);
}

//Converte uma string para hexadecimal (4 caracteres)
void concathex4(char  array[], uint16_t v)
{
    unsigned char bt;
    char hex[4];

    bt = (v/256);
    hex[0] = tbh[ bt>>4];
    hex[1] = tbh[ bt & 0xf];
    bt = v&0xff;
    hex[2] = tbh[ bt/16];
    hex[3] = tbh[ bt%16];

    for(int x=0; x<4; x++){
        strncat(array, &hex[x], 1);
    }

}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int, char**)
{

// Setup GLFW OPENGL ////////////////////////////////////////////////////////////////////////
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

  

    //Janela principal
    GLFWwindow* window = glfwCreateWindow(1280, 720, "SendUDP", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

// Config ImGui ////////////////////////////////////////////////////////////////////////

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    //Declaraçẽs p/ TCP/IP
    io_service io_service;
    ip::udp::socket socket(io_service);
    ip::udp::endpoint destination;  

    bool mode_dark = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    //Loop Principal
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        //Hendles principais
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoCollapse;

        static ImGuiInputTextFlags inputServer_flags = ImGuiInputTextFlags_CharsDecimal;
        inputServer_flags |= ImGuiInputTextFlags_CharsNoBlank;
        inputServer_flags |= ImGuiInputTextFlags_NoHorizontalScroll;

        //Tema Claro/escuro
        if(mode_dark)
            ImGui::StyleColorsDark();
        else 
            ImGui::StyleColorsLight();


        static int IpSend[] = {192, 168, 4, 1};
        static char PortSend[5] = "9292";

        static int IpLocal[] = {192,168,0,81};
        static int Gateway[] = {192,168,0,1};
        static int Netmask[] = {255,255,255,255};
        static char PortL[5];
        static char Idt[5];
        static char Key[5];

        static int IpServidor[] = {177,159,233,211};
        static char PortS[5];
        static int DNS[] = {8,8,8,8};
        static char timer[5];

        //Janela "Local"
        {

            ImGui::Begin("Local", NULL, window_flags);
            
            ImGui::SetWindowSize(ImVec2(629.0, 249.0));  
            
            ImGui::Text("Local:");
            
            ImGui::InputInt4("IpV4", IpLocal, inputServer_flags);
            for (int x = 0; x < 4; x++){
                if(IpLocal[x] > 255)
                    IpLocal[x] = 255;
            }

            ImGui::InputInt4("NetMask", Netmask, inputServer_flags);
            for (int x = 0; x < 4; x++){
                if(Netmask[x] > 255)
                    Netmask[x] = 255;
            }
            
            ImGui::InputInt4("Gateway", Gateway, inputServer_flags);
            for (int x = 0; x < 4; x++){
                if(Gateway[x] > 255)
                    Gateway[x] = 255;
            }


            ImGui::InputText("Port", PortL, 6, inputServer_flags);
            if(atoi(PortL) > 65535){
                PortL[0] = '6';
                PortL[1] = '5';
                PortL[2] = '5';
                PortL[3] = '3';
                PortL[4] = '5';
            }
            
            ImGui::Separator();
            
            ImGui::InputText("IDT", Idt, 6, inputServer_flags);
            if(atoi(Idt) > 65535){
                Idt[0] = '6';
                Idt[1] = '5';
                Idt[2] = '5';
                Idt[3] = '3';
                Idt[4] = '5';
            }
            
            ImGui::InputText("Key", Key, 6, inputServer_flags);
            if(atoi(Key) > 65535){
                Key[0] = '6';
                Key[1] = '5';
                Key[2] = '5';
                Key[3] = '3';
                Key[4] = '5';
            }

            if(ImGui::Button("Gravar"))
            {
                
                char mensagem[46] = "";
                char mensagemcp[46] = "";
                char crc[4] = "";
                int data[4] = {255, 255, 255, 255};

                strncat(mensagem, "L", 1);

                for (int x=0; x<4; x++)
                    data[x] = IpLocal[x];
                for (int x=0; x<4; x++)
                    concathex2(mensagem, data[x]);

                for (int x=0; x<4; x++)
                    data[x] = Gateway[x];
                for (int x=0; x<4; x++)
                    concathex2(mensagem, data[x]);      

                for (int x=0; x<4; x++)
                    data[x] = Netmask[x];
                for (int x=0; x<4; x++)
                    concathex2(mensagem, data[x]);    

                concathex4(mensagem, atoi(PortL));
                concathex4(mensagem, atoi(Idt)); 
                concathex4(mensagem, atoi(Key));

                strcpy(mensagemcp, mensagem);

                int crcint = CRC::Calculate(mensagem, strlen(mensagem), CRC::CRC_16_CCITTFALSE());
                concathex4(crc, crcint);

                char pacotepronto[60];
                sprintf(pacotepronto, "$%s%s#", mensagemcp, crc);

                static char ipstring[1024 * 16] = "";
                sprintf(ipstring, "%d.%d.%d.%d", IpSend[0], IpSend[1], IpSend[2], IpSend[3]);
                socket.open(ip::udp::v4());
                destination = ip::udp::endpoint(ip::address::from_string(ipstring), atoi(PortSend));
                boost::system::error_code err;
                socket.send_to(buffer(pacotepronto, strlen(pacotepronto)), destination, 0, err);
                socket.close();
                                                 
            }

            ImGui::End();
    
        }

        //Janela "Servidor"
        {

            ImGui::Begin("Servidor", NULL, window_flags);                        
            
            ImGui::SetWindowSize(ImVec2(629.0, 249.0));  
            
            ImGui::Text("Servidor:");
            
            ImGui::InputInt4("IpV4", IpServidor, inputServer_flags);
            for (int x = 0; x < 4; x++){
                if(IpServidor[x] > 255)
                    IpServidor[x] = 255;
            }

            ImGui::InputText("Port", PortS, 6, inputServer_flags);
            if(atoi(PortS) > 65535){
                PortS[0] = '6';
                PortS[1] = '5';
                PortS[2] = '5';
                PortS[3] = '3';
                PortS[4] = '5';
            }
            
            ImGui::Separator();

            ImGui::InputInt4("DNS", DNS, inputServer_flags);
            for (int x = 0; x < 4; x++){
                if(DNS[x] > 255)
                    DNS[x] = 255;
            }

            ImGui::InputText("Timer", timer, 6, inputServer_flags);
            if(atoi(timer) > 65535){
                timer[0] = '6';
                timer[1] = '5';
                timer[2] = '5';
                timer[3] = '3';
                timer[4] = '5';
            }

            if(ImGui::Button("Gravar"))
            {
                
                char mensagem[46] = "";
                char mensagemcp[46] = "";
                char crc[4] = "";
                int data[4] = {255, 255, 255, 255};

                strncat(mensagem, "S", 1);
                strncat(mensagem, "I", 1);

                concathex2(mensagem, atoi(timer));
                concathex4(mensagem, atoi(PortS));

                for (int x=0; x<4; x++)
                    data[x] = DNS[x];
                for (int x=0; x<4; x++)
                    concathex2(mensagem, data[x]);

                for (int x=0; x<4; x++)
                    data[x] = IpServidor[x];
                for (int x=0; x<4; x++)
                    concathex2(mensagem, data[x]);      

                strcpy(mensagemcp, mensagem);

                int crcint = CRC::Calculate(mensagem, strlen(mensagem), CRC::CRC_16_CCITTFALSE());
                concathex4(crc, crcint);

                char pacotepronto[60];
                sprintf(pacotepronto, "$%s%s#", mensagemcp, crc);

                static char ipstring[1024 * 16] = "";
                sprintf(ipstring, "%d.%d.%d.%d", IpSend[0], IpSend[1], IpSend[2], IpSend[3]);
                socket.open(ip::udp::v4());
                destination = ip::udp::endpoint(ip::address::from_string(ipstring), atoi(PortSend));
                boost::system::error_code err;
                socket.send_to(buffer(pacotepronto, strlen(pacotepronto)), destination, 0, err);
                socket.close();
                                                 
            }

            ImGui::End();
    
        }
        
        //Janela "Read"
        {
            ImGui::Begin("Read", NULL, window_flags);
            ImGui::SetWindowSize(ImVec2(629.0, 290.0));

            ImGui::Text("Read Memory");

            static char text[1024 * 16];

            ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_ReadOnly);

            if(ImGui::Button("Local"))
            {
                static char pLocal[8] = "$?L81EC";
                static char bufferL[100];
                
                static char ipstring[1024 * 16] = "";
                sprintf(ipstring, "%d.%d.%d.%d", IpSend[0], IpSend[1], IpSend[2], IpSend[3]);
                socket.open(ip::udp::v4());
                destination = ip::udp::endpoint(ip::address::from_string(ipstring), atoi(PortSend));
                boost::system::error_code err;
                socket.send_to(buffer(pLocal, strlen(pLocal)), destination, 0, err);

                ip::udp::endpoint sender_endpoint;
                static int endtime = 0;
                while(endtime < 50000){
                size_t len = socket.receive_from(
                boost::asio::buffer(bufferL), sender_endpoint);
                endtime ++;
                }
                socket.close();

                strncat(text, bufferL, 100);

            }
            ImGui::SameLine();
            if(ImGui::Button("Server"))
            {
                static char pServer[8] = "$?S6232";
                static char bufferS[100];

                static char ipstring[1024 * 16] = "";
                sprintf(ipstring, "%d.%d.%d.%d", IpSend[0], IpSend[1], IpSend[2], IpSend[3]);
                socket.open(ip::udp::v4());
                destination = ip::udp::endpoint(ip::address::from_string(ipstring), atoi(PortSend));
                boost::system::error_code err;
                socket.send_to(buffer(pServer, strlen(pServer)), destination, 0, err);
                
                ip::udp::endpoint sender_endpoint;
                static int endtime = 0;
                while(endtime < 50000){
                size_t len = socket.receive_from(
                boost::asio::buffer(bufferS), sender_endpoint);
                endtime ++;
                }
                
                socket.close();

                strncat(text, bufferS, 100);
            }

            ImGui::End();

        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    	
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    //Limpando ao sair
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
