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
#include "ns3/packet-sink.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-socket-address.h"
#include "ns3/athstats-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/wave-module.h"
#include <vector>
#include <limits>

using namespace ns3;

// test

// Custom function to print current position of the specified Node, to be used with Simulator::Schedule() function
// Example usage: Simulator::Schedule(Seconds(0), &printCurrentPosition, nodes.Get(1)->GetObject<MobilityModel>());
void printCurrentPosition(Ptr<MobilityModel> mob){
    double x = mob->GetPosition().x;
    double y = mob->GetPosition().y;
    double z = mob->GetPosition().z;
    std::cout << "X: "<< x << " Y: "<< y << " Z: " << z << std::endl;
}


Ptr<NetDevice> findClosestNetDevice(Ptr<NetDevice> currentDevice, NetDeviceContainer possibleDevices){
  double minimalDistanceFound = std::numeric_limits<double>::infinity();
  Ptr<NetDevice> closestFoundDevice;
  Vector currentDevicePosition = currentDevice->GetNode() -> GetObject<MobilityModel>() -> GetPosition();
  double distance = 0;
  
  for(NetDeviceContainer::Iterator i = possibleDevices.Begin(); i != possibleDevices.End(); ++i){
      Vector position = (*i) -> GetNode() -> GetObject<MobilityModel>() -> GetPosition ();
      
      distance = (currentDevicePosition - position).GetLength ();

      if (distance < minimalDistanceFound)
        {
          minimalDistanceFound = distance;
          closestFoundDevice = *i;
        }
  }

  return closestFoundDevice;
}


void printTime(){
    std::cout << "a" << std::endl;
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

    ////////////////////////////////////////////////////////////////////////////////////
    //// Install AdHoc WiFi
    ////////////////////////////////////////////////////////////////////////////////////

    

    std::cout<<"vypis1\n";
    Packet::EnablePrinting ();
    PacketSocketHelper packetSocket;
    packetSocket.Install(nodes);

    std::string phyMode ("DsssRate1Mbps");

    // The below set of helpers will help us to put together the wifi NICs we want
    WifiHelper wifi;

    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    // set it to zero; otherwise, gain will be added
    wifiPhy.Set ("RxGain", DoubleValue (-10) );
    // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
    wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
    wifiPhy.SetChannel (wifiChannel.Create ());


    // Add an upper mac and disable rate control
    WifiMacHelper wifiMac;
    wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode",StringValue (phyMode),
                                  "ControlMode",StringValue (phyMode));
    // Set it to adhoc mode
    wifiMac.SetType ("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);

    wifiPhy.EnablePcap("MarekTestPcap", devices);

    std::cout<<"vypis3\n";
    //Toto nastaví udáje pre odosielatela packetov
    PacketSocketAddress socket;
    socket.SetSingleDevice (devices.Get (0)->GetIfIndex ());
    socket.SetPhysicalAddress (devices.Get (0)->GetAddress ());
    socket.SetProtocol (1);
    std::cout<<"vypis4\n";

    /* Packet Sink pokus
    std::cout<<"Packet sink install:";
    
    Ptr<Node> testWifiNode = nodes.Get(1);
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
    ApplicationContainer sinkApp = sinkHelper.Install (testWifiNode);
    sinkHelper.Install (testWifiNode);
    //sink = StaticCast<PacketSink> (sinkApp.Get (0));
    */
    
    // trieda na vytváranie komunikácie, trieda vytvára pakety
    OnOffHelper onoff ("ns3::PacketSocketFactory", Address (socket));
    onoff.SetConstantRate (DataRate ("500kb/s"));
    onoff.SetAttribute("PacketSize", UintegerValue(2000));
    //onoff.SetAttribute("PacketPayload", "asdasdasfasfsdfsdf");
    std::cout<<"vypis5\n";
    // nastavenie posielania paketov pomocou onoff.
    ApplicationContainer apps = onoff.Install (nodes.Get (0));
    apps.Start (Seconds (0.5));
    apps.Stop (Seconds (70.0));
    std::cout<<"vypis6\n";
    Ptr<NetDevice> d0 = devices.Get(0);
    
    
    //Simulator::Schedule ( Seconds(1) , &findClosestNetDevice, d0, &devices);

    

    /////////////////////////////////////////////////////////////////////////////////
    /// WAVE
    /////////////////////////////////////////////////////////////////////////////////
    /// Setup
    /*
    


    YansWifiChannelHelper waveChannel = YansWifiChannelHelper::Default ();
    YansWavePhyHelper wavePhy =  YansWavePhyHelper::Default ();
    wavePhy.SetChannel (waveChannel.Create ());
    wavePhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO); // set the pcap to radio which gives more info in the pcap file


    // these parameters devides the transmition to 7 levels
    wavePhy.Set ("TxPowerStart", DoubleValue (7)); // min transmition power
    wavePhy.Set ("TxPowerEnd", DoubleValue (7)); // max transmition power

    QosWaveMacHelper waveMac = QosWaveMacHelper::Default (); // create MAC layer
    WaveHelper waveHelper = WaveHelper::Default ();

    //rate 6 Mbps and bandwidth 10 MHZ
    waveHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                        "DataMode", StringValue ("OfdmRate6MbpsBW10MHz"	),
                                        "ControlMode",StringValue ("OfdmRate6MbpsBW10MHz"),
                                        "NonUnicastMode", StringValue ("OfdmRate6MbpsBW10MHz")); // Datamode is for broadcasting and ControlMode is for Unicast


    NetDeviceContainer devices = waveHelper.Install (wavePhy, waveMac, nodes);

    //wavePhy.EnablePcap ("WaveTest", devices);
    wavePhy.EnablePcap ("WaveTest", devices);

    */
    /////////////// End of setup ///////////////////////////////
    /*
    //prepare a packet with a payload of 1000 Bytes. Basically it has zeros in the payload
    Ptr <Packet> packet = Create <Packet> (1000);

    //destination MAC
    Mac48Address dest = Mac48Address::GetBroadcast();

    uint16_t protocol = 0x88dc;

    //We can also set the transmission parameters at the higher layeres
    TxInfo tx;
    tx.preamble = WIFI_PREAMBLE_LONG;

    //We set the channel on which the packet is sent. The WaveNetDevice must have access to the channel
    //CCH is enabled by default.
    tx.channelNumber = CCH;

    //We can set per-packet data rate. This packet will have a rate of 12Mbps.
    tx.dataRate = WifiMode ("OfdmRate12MbpsBW10MHz");

    tx.priority = 7;	//We set the AC to highest priority. We can set this per packet.

    tx.txPowerLevel = 6; //When we define TxPowerStart & TxPowerEnd for a WifiPhy, 7 is correspond to TxPowerEnd, and 1 TxPowerStart, and numbers in between are levels.
*/
/*
    Ptr <NetDevice> d0 = devices.Get (0);
    Ptr <WaveNetDevice> wd0 = DynamicCast <WaveNetDevice> (d0); // Dynamically cast to waveNetDevice
    Simulator::Schedule ( Seconds (sim_time-1) , &WaveNetDevice::SendX, wd0, packet, dest, protocol, tx); // destination of the packet is broadcast
*/

    //Get the WaveNetDevice for the first devices, using node 0.
    //for (unsigned int i =0;i < nnodes; i++){
 /*       Ptr <Packet> packet1 = Create <Packet> (1000);
        Ptr <NetDevice> d0 = devices.Get (0);
        Ptr <WaveNetDevice> wd0 = DynamicCast <WaveNetDevice> (d0);
        Simulator::Schedule (Seconds (1) , &WaveNetDevice::SendX, wd0, packet1, dest, protocol, tx);
*/

    //Let's send a Unicast packet from n0 to n2
    //Get the MAC address of the target node
/*
    Ptr <WaveNetDevice> d0 = DynamicCast<WaveNetDevice>(devices.Get(0));
    Ptr <WaveNetDevice> d1 = DynamicCast<WaveNetDevice>(devices.Get(1));
    Ptr <WaveNetDevice> d2 = DynamicCast<WaveNetDevice>(devices.Get(2));
    Ptr <WaveNetDevice> d3 = DynamicCast<WaveNetDevice>(devices.Get(3));

    TxInfo tx_u;
    std::cout << "SIM TIME JE : " << sim_time << std::endl;

    for (int i = 1; i < sim_time - 1; i++){
        Ptr <Packet> unicast_packet0 = Create<Packet> (200);
        Ptr <Packet> unicast_packet1 = Create<Packet> (200);
        Ptr <Packet> unicast_packet2 = Create<Packet> (200);
        Ptr <Packet> unicast_packet3 = Create<Packet> (200);

        Simulator::Schedule ( Seconds(i) , &printTime);
        Simulator::Schedule ( Seconds(i) , &WaveNetDevice::SendX, d0, unicast_packet0, dest, protocol, tx_u );
        Simulator::Schedule ( Seconds(i) , &WaveNetDevice::SendX, d1, unicast_packet1, dest, protocol, tx_u );
        Simulator::Schedule ( Seconds(i) , &WaveNetDevice::SendX, d2, unicast_packet2, dest, protocol, tx_u );
        Simulator::Schedule ( Seconds(i) , &WaveNetDevice::SendX, d3, unicast_packet3, dest, protocol, tx_u );
    }

    */
    ///////////////////////////////////////////////////////////

    AthstatsHelper athstats;
    athstats.EnableAthstats ("athstats-sta", nodes);
    Simulator::Stop(Seconds (sim_time));
    std::cout<<"vypis7\n";
    //AnimationInterface anim("filename.xml");




    Simulator::Run();
    std::cout<<"vypis8\n";
    Simulator::Destroy();

    std::cout << "End of Program" << std::endl;

}
