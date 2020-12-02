#include <pcap.h>
#include <tins/tins.h>

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
#include <sstream>

#define MB (1048576.0)
#define UNUSE(x) (void)(x)

using Tins::SnifferConfiguration;
using Tins::FileSniffer;
using Tins::IP;
using Tins::DNS;
using Tins::TCP;
using Tins::UDP;
using Tins::RawPDU;
using Tins::PDU;
using Tins::Packet;
using namespace Tins;
using namespace std;

bool callback(const PDU &pdu) {
  // Parse the IP header
  const IP &ip = pdu.rfind_pdu<IP>(); 

  // Parse the TCP or UDP header
  const TCP &tcp = pdu.rfind_pdu<TCP>();
  
  cout << "IP:" << ip.src_addr() << ':' << tcp.sport() << " -> " << ip.dst_addr() << ':' << tcp.dport() << endl;
    
  return true;
}

int main(int argc, char *argv[])
{
        if (argc < 2) {
                std::cout << "[*] Usage: " << argv[0] << " PCAP_files\n";
                return -1;
        }
        try {
                SnifferConfiguration config;
                config.set_filter("tcp or udp");
                config.set_promisc_mode(false);
                // config.set_snap_len(65536);

                FileSniffer sniffer(argv[1], config);
                sniffer.sniff_loop(callback);
		
                return 0;
        }
        catch (std::exception& ex) {
                std::cerr << "[X] Error: " << ex.what() << '\n';
                return -1;
        }
}
