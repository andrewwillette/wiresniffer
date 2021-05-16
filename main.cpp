#include <iostream>
#include <imtui/imtui.h>
#include <imtui/imtui-impl-ncurses.h>
#include <imgui/imgui.h>
#include <tins/tins.h>
#include <string>
#include <thread>

using namespace Tins;
using namespace std;

struct WireSnifferRow {
    string sourceAddress, destinationAddress;
};

std::vector<WireSnifferRow> snifferRows;

class PacketReader {
    public:
    static void readPackets() {
        SnifferConfiguration config;
        config.set_promisc_mode(true);
        config.set_filter("ip src 192.168.1.4");
        Sniffer sniffer("en0", config);
        
        while (true) {
            auto packet = sniffer.next_packet();
            auto destinationAddress = packet.pdu()->rfind_pdu<IP>().dst_addr().to_string();
            auto sourceAddress = packet.pdu()->rfind_pdu<IP>().src_addr().to_string();
            WireSnifferRow row;
            row.destinationAddress = destinationAddress;
            row.sourceAddress = sourceAddress;
            snifferRows.push_back(row);
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
            ImGui::BeginTable("Wiresniffer", 2);
            createHeaders();
            for(WireSnifferRow row : snifferRows){
                createRow(row);
            }
            ImGui::EndTable();
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
    void createHeaders() {
        ImGui::TableNextRow();
        // ImGui::TableHeadersRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::TableHeader("Source Address");
        ImGui::TableSetColumnIndex(1);
        ImGui::TableHeader("Destination Address");
    }

    public:
    void createRow(WireSnifferRow wireSnifferRow) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(wireSnifferRow.sourceAddress.c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(wireSnifferRow.destinationAddress.c_str());
    }
};

int main() {
    std::thread (PacketReader::readPackets).detach();
    FrontEnd frontend;
    frontend.initScreen();
    frontend.displayScreen();
    return 0;
}
