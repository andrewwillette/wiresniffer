#include <iostream>
#include <imtui/imtui.h>
#include <imtui/imtui-impl-ncurses.h>
#include <imgui/imgui.h>
#include <tins/tins.h>
#include <string>
#include <thread>
#include <cstring>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fmt/core.h>
#include <spdlog/sinks/rotating_file_sink.h>

using namespace Tins;
using namespace std;
auto max_size = 1048576 * 5;
auto max_files = 3;
auto logger = spdlog::rotating_logger_mt("wiresniffer_log", "logs/wiresniffer.log", max_size, max_files);

static string getProtocol(uint8_t ipProtocolNumber) {
    if(ipProtocolNumber == 6) {
        return "TCP";
    } else if (ipProtocolNumber == 17) {
        return "UDP";
    } else if (ipProtocolNumber == 1) {
        return "ICMP";
    } else if (ipProtocolNumber == 2) {
        return "IGMP";
    } else {
        return fmt::format("Unknown protocol: {}", std::to_string(ipProtocolNumber));
    }
}

struct WireSnifferRow {
    string sourceAddress, destinationAddress, headerSize, protocol, timestamp, number, info, size;

    WireSnifferRow(PtrPacket& ptrPacket, int packetNumber) {
        number = std::to_string(packetNumber);
        timestamp = std::to_string(ptrPacket.timestamp().seconds());
        sourceAddress = ptrPacket.pdu()->rfind_pdu<IP>().src_addr().to_string();
        destinationAddress = ptrPacket.pdu()->rfind_pdu<IP>().dst_addr().to_string();
        headerSize = std::to_string(ptrPacket.pdu()->rfind_pdu<IP>().header_size());
        size = std::to_string(ptrPacket.pdu()->size());
        info = "some info";
        protocol = getProtocol(ptrPacket.pdu()->rfind_pdu<IP>().protocol());
        try {
            auto testing = ptrPacket.pdu()->rfind_pdu<UDP>();
            if(testing.sport() == 53 || testing.dport() == 53) {
                DNS dns = ptrPacket.pdu()->rfind_pdu<RawPDU>().to<DNS>();
                for(const auto &query : dns.queries()) {
                    logger->info(sourceAddress);
                    logger->info(destinationAddress);
                    logger->info(protocol);
                    logger->info(query.dname());
                }
            }
        } catch (pdu_not_found pduNotFound) {
            logger->info("pdu not found");
        }
    }
};

std::vector<WireSnifferRow> sniffedPackets;

std::string getLocalIPAddress() {
    const char* google_dns_server = "8.8.8.8";
    int dns_port = 53;

    struct sockaddr_in serv;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0) {
        std::cout << "Socket error" << std::endl;
    }

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(google_dns_server);
    serv.sin_port = htons(dns_port);

    int err = connect(sock, (const struct sockaddr*)&serv, sizeof(serv));
    if (err < 0) {
        std::cout << "Error number: " << errno
            << ". Error message: " << strerror(errno) << std::endl;
    }

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (struct sockaddr*)&name, &namelen);

    char buffer[80];
    const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, 80);
    if(p == NULL) {
        std::cout << "Error number: " << errno
            << ". Error message: " << strerror(errno) << std::endl;
    }

    close(sock);
    return buffer;
}

class PacketReader {
    /**
     * Reads packets and pushes entries to sniffed packets
     */
    public:
    [[noreturn]] static void readPackets(string sourceAddress) {
        SnifferConfiguration config;
        config.set_promisc_mode(true);
        Sniffer sniffer("en0", config);
        int packetNumber = 1;
        while (true) {
            auto packet = sniffer.next_packet();
            if(packet.pdu()->find_pdu<IP>() != 0 || packet.pdu()->find_pdu<DNS>() != 0) {
                WireSnifferRow wireSnifferRow = WireSnifferRow(packet, packetNumber);
                sniffedPackets.push_back(wireSnifferRow);
                packetNumber++;
            }
        }
    }
};

/**
 * Handles all the display functionality , including starting up screen and methods
 * to add data to the screen.
 */
class FrontEnd {
    const int HEIGHT = 50;
    const int WIDTH = 200;
    const string headers[8] = {"No.", "Time", "Source Address", "Destination Address", "Protocol", "Length", "Info", "Header Length"};
    ImTui::TScreen* screen;

    public:
    void initScreen() {
        ImGui::CreateContext();
        auto newScreen = ImTui_ImplNcurses_Init(true);
        ImTui_ImplText_Init();
        screen = newScreen;
    }

    public:
    [[noreturn]] void displayScreen() {
        while (true) {
            ImTui_ImplNcurses_NewFrame();
            ImTui_ImplText_NewFrame();
            ImGui::NewFrame();
            bool m_visible = true;
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT), ImGuiCond_Once);
            bool x_visible = false;
            ImGui::Begin("Wiresniffer", &x_visible, ImGuiWindowFlags_NoCollapse);
            ImGui::BeginTable("Wiresniffer", sizeof(headers)/sizeof(headers[0]));
            createHeaders();
            for(WireSnifferRow row : sniffedPackets){
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
        ImGui::TableHeadersRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TableHeader(headers[0].c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::TableHeader(headers[1].c_str());
        ImGui::TableSetColumnIndex(2);
        ImGui::TableHeader(headers[2].c_str());
        ImGui::TableSetColumnIndex(3);
        ImGui::TableHeader(headers[3].c_str());
        ImGui::TableSetColumnIndex(4);
        ImGui::TableHeader(headers[4].c_str());
        ImGui::TableSetColumnIndex(5);
        ImGui::TableHeader(headers[5].c_str());
        ImGui::TableSetColumnIndex(6);
        ImGui::TableHeader(headers[6].c_str());
        ImGui::TableSetColumnIndex(7);
        ImGui::TableHeader(headers[7].c_str());
    }

    public:
    void createRow(WireSnifferRow wireSnifferRow) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", wireSnifferRow.number.c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", wireSnifferRow.timestamp.c_str());
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%s", wireSnifferRow.sourceAddress.c_str());
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%s", wireSnifferRow.destinationAddress.c_str());
        ImGui::TableSetColumnIndex(4);
        ImGui::Text("%s", wireSnifferRow.protocol.c_str());
        ImGui::TableSetColumnIndex(5);
        ImGui::Text("%s", wireSnifferRow.size.c_str());
        ImGui::TableSetColumnIndex(6);
        ImGui::Text("%s", wireSnifferRow.info.c_str());
        ImGui::TableSetColumnIndex(7);
        ImGui::Text("%s", wireSnifferRow.headerSize.c_str());
    }
};


int main() {
    string personalIPAddress = getLocalIPAddress();
    std::thread (PacketReader::readPackets, personalIPAddress).detach();
    FrontEnd frontend;
    frontend.initScreen();
    frontend.displayScreen();
    return 0;
}
