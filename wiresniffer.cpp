#include <iostream>
#include <imtui/imtui.h>
#include <imtui/imtui-impl-ncurses.h>
#include <imgui/imgui.h>
#include <tins/tins.h>

using namespace Tins;

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

    bool handle(PDU&) {
        // Don't process anything
        return false;
    }
};

int main() {
    foo f;
    f.bar();
}