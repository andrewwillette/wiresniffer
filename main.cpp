#include <iostream>
#include <imtui/imtui.h>
#include <imtui/imtui-impl-ncurses.h>
#include <imgui/imgui.h>
#include <tins/tins.h>
#include <string>
#include <thread>

using namespace Tins;
using namespace std;

class WireSnifferRow {

    string sourceAddress, destinationAddress;

    public:
    WireSnifferRow(string sourceAddress, string destinationAddress) {
        sourceAddress = sourceAddress;
        destinationAddress = destinationAddress;
    }
};

std::vector<Packet> globalPackets;

class PacketReader {
    public:
    static void readPackets() {
        SnifferConfiguration config;
        config.set_promisc_mode(true);
        config.set_filter("ip src 192.168.1.2");
        Sniffer sniffer("en0", config);
        
        while (true) {
            globalPackets.push_back(sniffer.next_packet());
        }
    }

    public:
    void printTest() {
        std::cout << "printTest" << std::endl;
    }
};

/**
 * Handles all the display functionality , including starting up screen and methods
 * to add data to the screen.
 */
class FrontEnd {
    const int HEIGHT = 50;
    const int WIDTH = 100;
    ImTui::TScreen* screen;

    public:
    void initScreen() {
        ImGui::CreateContext();
        auto newScreen = ImTui_ImplNcurses_Init(true);
        ImTui_ImplText_Init();
        screen = newScreen;
    }

    public:
    void displayScreen() {
        while (true) {
            ImTui_ImplNcurses_NewFrame();
            ImTui_ImplText_NewFrame();
            ImGui::NewFrame();
            bool m_visible = true;
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT), ImGuiCond_Once);
            bool x_visible = false;
            ImGui::Begin("Wiresniffer", &x_visible, ImGuiWindowFlags_NoCollapse);
            for(Packet packet : globalPackets){
                auto destinationAddresses = packet.pdu()->rfind_pdu<IP>().dst_addr().to_string();
                createRow(destinationAddresses);
            }
            ImGui::End();
            ImGui::Render();
            ImTui_ImplText_RenderDrawData(ImGui::GetDrawData(), screen);
            ImTui_ImplNcurses_DrawScreen();
        }
    }

    public:
    void shutdownScreen() {
        ImTui_ImplText_Shutdown();
        ImTui_ImplNcurses_Shutdown();
    }

    public:
    void createRow(std::string& valueOne) {
        ImGui::Text("destinationAddress");
        ImGui::SameLine();
        ImGui::Text(valueOne.c_str());
    }
};

int main() {
    std::thread (PacketReader::readPackets).detach();
    FrontEnd frontend;
    frontend.initScreen();
    frontend.displayScreen();
    return 0;
}
