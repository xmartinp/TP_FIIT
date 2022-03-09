#include "wifi-setup.h"
#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns2-node-utility.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/propagation-module.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-address.h"
#include "ns3/athstats-helper.h"

#include "ns3/netanim-module.h"

using namespace ns3;

// Custom function to print current position of the specified Node, to be used with Simulator::Schedule() function
// Example usage: Simulator::Schedule(Seconds(0), &printCurrentPosition, nodes.Get(1)->GetObject<MobilityModel>());
void printCurrentPosition(Ptr<MobilityModel> mob){
    double x = mob->GetPosition().x;
    double y = mob->GetPosition().y;
    double z = mob->GetPosition().z;
    std::cout << "X: "<< x << " Y: "<< y << " Z: " << z << std::endl;
}

NS_LOG_COMPONENT_DEFINE("CustomLoggingExample");

int main (int argc, char *argv[])
{
    LogComponentEnable("CustomLoggingExample", LOG_ALL);
    CommandLine cmd;
    cmd.Parse (argc, argv);

    ////////////////////////////////////////////////////////////////////////////////////
    //// Install mobility of the nodes using NS2 helper
    ////////////////////////////////////////////////////////////////////////////////////

    // Mobilita z NS2
    std::string mobility_file = "scratch/P2/ns2mobility.tcl";
    Ns2NodeUtility ns2_utility (mobility_file);
    uint32_t nnodes = ns2_utility.GetNNodes();
    double sim_time = ns2_utility.GetSimulationTime();
    NodeContainer nodes;
    nodes.Create (nnodes);

    Ns2MobilityHelper sumo_trace (mobility_file);
    sumo_trace.Install();

    ////////////////////////////////////////////////////////////////////////////////////
    //// Install the required communication methods one by one
    ////////////////////////////////////////////////////////////////////////////////////

    //WIFI Installation
    // (toto asi mozeme neskor vymazat)
    std::cout<<"vypis1\n";
    Packet::EnablePrinting ();
    PacketSocketHelper packetSocket;
    packetSocket.Install(nodes);

    YansWifiPhyHelper phy;
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    phy.SetChannel(wifiChannel.Create());
    std::cout<<"vypis2\n";
    WifiMacHelper mac;
    mac.SetType ("ns3::AdhocWifiMac");
    WifiHelper wifi;
    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    //Toto nastaví udáje pre odosielatela packetov
    PacketSocketAddress socket;
    //socket.SetSingleDevice (devices.Get (1)->GetIfIndex ());
    socket.SetAllDevices();
    //socket.SetPhysicalAddress (devices.Get (5)->GetAddress ());
    socket.SetPhysicalAddress (devices.Get (4)->GetAddress ());
    socket.SetProtocol (1);
    std::cout<<"vypis4\n";
    // trieda na vytváranie komunikácie, trieda vytvára pakety
    OnOffHelper onoff ("ns3::PacketSocketFactory", Address (socket));
    onoff.SetConstantRate (DataRate ("5kb/s"));
    //    onoff.SetAttribute("PacketSize", UintegerValue(2000));
    std::cout<<"vypis5\n";
    // nastavenie posielania paketov pomocou onoff.
    ApplicationContainer apps = onoff.Install (nodes);
    apps.Start (Seconds (0));
    apps.Stop (Seconds (200));
    std::cout<<"vypis6\n";

    AthstatsHelper athstats;
    athstats.EnableAthstats ("athstats-sta", nodes);
    Simulator::Stop(Seconds (sim_time));
    std::cout<<"vypis7\n";
    AnimationInterface anim ("simulacia1.xml");
    Simulator::Run();
    std::cout<<"vypis8\n";
    Simulator::Destroy();

    std::cout << "End of Program" << std::endl;

}
