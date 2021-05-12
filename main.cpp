#include <iostream>
#include <imtui/imtui.h>
#include <imtui/imtui-impl-ncurses.h>
#include <imgui/imgui.h>
#include <tins/tins.h>

using namespace Tins;

// sniff a packet from my computer
void snifferOne(){
    Sniffer sniffer("en0");
    sniffer.set_filter("ip src 192.168.1.2");
    PDU *some_pdu = sniffer.next_packet();
    delete some_pdu;
}

struct foo {
    void bar() {
        SnifferConfiguration config;
        config.set_promisc_mode(true);
        config.set_filter("ip src 192.168.1.2");
        Sniffer sniffer("en0", config);

        /* Uses the helper function to create a proxy object that 
         * call this->handle. If you're using boost or c++11,
         * you could use boost::bind or std::bind, that
         * will also work.
         */
        sniffer.sniff_loop(make_sniffer_handler(this, &foo::handle));
    }

    bool handle(PDU& pdu) {
        std::cout << "hello world" << std::endl;
        std::cout << pdu.IP << std::endl;
        // Don't process anything
        return true;
    }
};

std::vector<Packet> getPackets(){
    std::vector<Packet> vt;
    SnifferConfiguration config;
    config.set_promisc_mode(true);
    config.set_filter("ip src 192.168.1.2");
    Sniffer sniffer("en0", config);
    while (vt.size() != 20) {
        vt.push_back(sniffer.next_packet());
    }
    std::cout << vt.size() << std::endl;
    return vt;
}

void drawIPPackets() {
    // for(Packet packet : getPackets()){
    //     // ImGui::Text(packet.pdu()->rfind_pdu<IP>().src_addr().to_string().c_str());
    //     ImGui::Text("Here1");
    // }
    // ImGui::Text("IPPacket1");
    // ImGui::Text("IPPacket2");
}

auto createScreen() {

    ImGui::CreateContext();

    auto screen = ImTui_ImplNcurses_Init(true);
    ImTui_ImplText_Init();
    return screen;
}

void destroyScreen() {}

int main() {
    auto screen = createScreen();

    int nframes = 0;
    float fval = 1.23f;

    auto packets = getPackets();

    while (true) {
        ImTui_ImplNcurses_NewFrame();
        ImTui_ImplText_NewFrame();

        ImGui::NewFrame();


        ImGui::SetNextWindowPos(ImVec2(4, 2), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(100.0, 100.0), ImGuiCond_Once);
        ImGui::Begin("Hello, world!");

        for(Packet packet : packets){
            ImGui::Text(packet.pdu()->rfind_pdu<IP>().dst_addr().to_string().c_str());
            // ImGui::Text(packet.pdu()->rfind_pdu<IP>().id().c_string());
            // ImGui::Text("Here1");
        }
        ImGui::Text("NFrames = %d", nframes++);
        // ImGui::Text("Mouse Pos : x = %g, y = %g", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
        // ImGui::Text("Time per frame %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        // ImGui::Text("Float:");
        // ImGui::SameLine();
        // ImGui::SliderFloat("##float", &fval, 0.0f, 10.0f);
        drawIPPackets();

        // for(int n=0; n < vt.size(); n++) {
        //     const char* value = vt[n].pdu()->rfind_pdu<IP>().src_addr().to_string().c_str();
        //     ImGui::Text(value);
        // }
            
        ImGui::End();


        ImGui::Render();

        ImTui_ImplText_RenderDrawData(ImGui::GetDrawData(), screen);
        ImTui_ImplNcurses_DrawScreen();
    }
    ImTui_ImplText_Shutdown();
    ImTui_ImplNcurses_Shutdown();
    return 0;
}
