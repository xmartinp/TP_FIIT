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
#include "ns3/wave-module.h"
#include <ns3/lte-module.h>
#include <vector>
#include <limits>

using namespace ns3;

// Global variables
std::set<uint32_t> usedNodes;

//Create a unique string identifier (usually based on the name of the file and/or class defined within the file) and register it with a macro call
NS_LOG_COMPONENT_DEFINE("CustomLoggingExample");

// Custom function to print current position of the specified Node, to be used with Simulator::Schedule() function
// Example usage: Simulator::Schedule(Seconds(0), &printCurrentPosition, nodes.Get(1)->GetObject<MobilityModel>());
void printCurrentPosition(Ptr<MobilityModel> mob){
    double x = mob->GetPosition().x;
    double y = mob->GetPosition().y;
    double z = mob->GetPosition().z;
    std::cout << "X: "<< x << " Y: "<< y << " Z: " << z << std::endl;
}

// Function to find the closest device to the current one.
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

// Split main NodeContainer to smallest one according to the user's input
NodeContainer splitNodeSet(NodeContainer nodes, std::set<uint32_t> chosenNodes) { 
  NodeContainer splitNodes = NodeContainer();

  if(nodes.GetN() == 0){
    std::cout<<"ERROR: node container is empty\n";
    return splitNodes;
  }

  if(chosenNodes.size() == 0){
    std::cout<<"ERROR: set of nodes you are trying to attach to new container is empty\n";
    return splitNodes;
  }

  for(auto i : chosenNodes) {
    if(!nodes.Contains(i)){
      std::cout<<"ERROR: node doesnt exist in original node container\n";
      return NodeContainer();
    }

    if(!nodes.Contains(i)){
      std::cout<<"ERROR: this node is already taken\n";
      return NodeContainer();
    }

    splitNodes.Add(nodes.Get(i));
  }

  return splitNodes;
}


int main (int argc, char *argv[])
{
    LogComponentEnable("CustomLoggingExample", LOG_ALL);
    CommandLine cmd;
    cmd.Parse (argc, argv);

    ////////////////////////////////////////////////////////////////////////////////////
    ////           Install mobility of the nodes using NS2 helper                   ////
    ////////////////////////////////////////////////////////////////////////////////////

    std::string mobilityFile = "scratch/P2/ns2mobility.tcl";
    Ns2NodeUtility ns2_utility (mobilityFile);
    uint32_t nnodes = ns2_utility.GetNNodes(); 
    double simTime = ns2_utility.GetSimulationTime();
    NodeContainer nodes;
    nodes.Create (nnodes);
    
    Ns2MobilityHelper sumoTrace (mobilityFile);
    sumoTrace.Install();

    std::cout<<"sm time je" << simTime;

    // Choose which devices use which transmission technology

    std::cout<<"Total number of nodes is :" << nnodes << std::endl;

    std::set <uint32_t> wifiNodesSet = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69};
    std::set <uint32_t> lteNodesSet = {21, 22, 23, 24, 25};
    std::set <uint32_t> waveNodesSet = {};
    NodeContainer wifiNodes = splitNodeSet(nodes, wifiNodesSet);
    NodeContainer lteUeNodes = splitNodeSet(nodes, lteNodesSet);
    NodeContainer waveNodes = splitNodeSet(nodes, waveNodesSet);


    ////////////////////////////////////////////////////////////////////////////////////
    ////          Install the required communication methods one by one             ////
    ////////////////////////////////////////////////////////////////////////////////////

    // If you don't intend to use some specific transimission technology in your simulation, put the whole section of that protocol into a comment block!

    // PLEASE note that NetAnim CAN NOT be used if WAVE devices are present in the simulation!

    ////////////////////////////////////////////////////////////////////////////////////
    ////                             Install LTE                                    ////
    ////////////////////////////////////////////////////////////////////////////////////

    // Only install LTE if there are LTE devices present

    Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
    Ptr<PointToPointEpcHelper> lteEpcHelper = CreateObject<PointToPointEpcHelper> ();
    lteHelper->SetEpcHelper (lteEpcHelper);                                // Set the EpcHelper to be used to setup the EPC network in conjunction with the setup of the LTE radio access network.
    lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
    lteHelper->SetHandoverAlgorithmType ("ns3::NoOpHandoverAlgorithm"); // disable automatic handover

    Ptr<Node> ltePgw = lteEpcHelper->GetPgwNode ();                           // a pointer to the node implementing PGW functionality. 

    // Create a NodeContainer with exactly one node which has been previously instantiated as a single RemoteHost .
    NodeContainer lteRemoteHostContainer;
    lteRemoteHostContainer.Create (1);
    Ptr<Node> lteRemoteHost = lteRemoteHostContainer.Get (0);
    InternetStackHelper lteInternet;
    lteInternet.Install (lteRemoteHostContainer);
    // Create the Internet
    PointToPointHelper lteP2ph;                                            // Create a PointToPointHelper to make life easier when creating point to point networks.
    lteP2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
    lteP2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
    lteP2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
    NetDeviceContainer lteInternetDevices = lteP2ph.Install (ltePgw, lteRemoteHost);
    Ipv4AddressHelper lteIpv4h;
    lteIpv4h.SetBase ("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer lteInternetIpIfaces = lteIpv4h.Assign (lteInternetDevices);
    Ipv4Address lteRemoteHostAddr = lteInternetIpIfaces.GetAddress (1);
    // Routing of the Internet Host (towards the LTE network)
    Ipv4StaticRoutingHelper lteIpv4RoutingHelper;
    Ptr<Ipv4StaticRouting> lteRemoteHostStaticRouting = lteIpv4RoutingHelper.GetStaticRouting (lteRemoteHost->GetObject<Ipv4> ());
    // interface 0 is localhost, 1 is the p2p device
    lteRemoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

    NodeContainer lteEnbNodes;
    lteEnbNodes.Create (1);
    //NodeContainer lteUeNodes;
    //lteUeNodes.Create (nnodes);
    MobilityHelper lteMobility;

    /*lteMobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                            "Bounds", RectangleValue (Rectangle (-100, 100, -50, 100)));
    lteMobility.Install (lteUeNodes);
    */
    lteMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    lteMobility.Install (lteEnbNodes);
    //lteMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    // Install LTE Devices in eNB and UEs
    NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (lteEnbNodes);
    NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (lteUeNodes);

    // Install the IP stack on the UEs
    lteInternet.Install (lteUeNodes);
    Ipv4InterfaceContainer lteUeIpIfaces;
    lteUeIpIfaces = lteEpcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));

    lteHelper->AttachToClosestEnb (ueLteDevs, enbLteDevs);


    NS_LOG_LOGIC ("setting up applications");
    // Install and start applications on UEs and remote host
    uint16_t lteDlPort = 10000;
    uint16_t lteUlPort = 20000;

    // Randomize a bit start times to avoid simulation artifacts (e.g., buffer overflows due to packet transmissions happening exactly at the same time)
    Ptr<UniformRandomVariable> lteStartTimeSeconds = CreateObject<UniformRandomVariable> ();
    lteStartTimeSeconds->SetAttribute ("Min", DoubleValue (0));
    lteStartTimeSeconds->SetAttribute ("Max", DoubleValue (0.010)); 

    uint32_t numberOfLteNodes = lteUeNodes.GetN();
    for (uint32_t u = 0; u < numberOfLteNodes; ++u)
    {
      Ptr<Node> lteUe = lteUeNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = lteIpv4RoutingHelper.GetStaticRouting (lteUe->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (lteEpcHelper->GetUeDefaultGatewayAddress (), 1);

      //for (uint32_t b = 0; b < numBearersPerUe; ++b)
        {
          ++lteDlPort;
          ++lteUlPort;

          ApplicationContainer clientApps;
          ApplicationContainer serverApps;

          NS_LOG_LOGIC ("installing UDP DL app for UE " << u);
          UdpClientHelper dlClientHelper (lteUeIpIfaces.GetAddress (u), lteDlPort);
          clientApps.Add (dlClientHelper.Install (lteRemoteHost));
          PacketSinkHelper lteDlPacketSinkHelper ("ns3::UdpSocketFactory",
                                              InetSocketAddress (Ipv4Address::GetAny (), lteDlPort));
          serverApps.Add (lteDlPacketSinkHelper.Install (lteUe));

          NS_LOG_LOGIC ("installing UDP UL app for UE " << u);
          UdpClientHelper lteUlClientHelper (lteRemoteHostAddr, lteUlPort);
          clientApps.Add (lteUlClientHelper.Install (lteUe));
          PacketSinkHelper lteUlPacketSinkHelper ("ns3::UdpSocketFactory",
                                              InetSocketAddress (Ipv4Address::GetAny (), lteUlPort));
          serverApps.Add (lteUlPacketSinkHelper.Install (lteRemoteHost));

          Ptr<EpcTft> lteTft = Create<EpcTft> ();
          EpcTft::PacketFilter lteDlft;
          lteDlft.localPortStart = lteDlPort;
          lteDlft.localPortEnd = lteDlPort;
          lteTft->Add (lteDlft);
          EpcTft::PacketFilter lteUlpf;
          lteUlpf.remotePortStart = lteUlPort;
          lteUlpf.remotePortEnd = lteUlPort;
          lteTft->Add (lteUlpf);
          EpsBearer lteBearer (EpsBearer::GBR_CONV_VOICE);
          lteHelper->ActivateDedicatedEpsBearer (ueLteDevs.Get (u), lteBearer, lteTft);

          Time lteStartTime = Seconds (lteStartTimeSeconds->GetValue ());
          serverApps.Start (lteStartTime);
          clientApps.Start (lteStartTime);

        } 
    }
    
    // Add X2 inteface
    lteHelper->AddX2Interface (lteEnbNodes);

    // Enable tracing
    lteHelper->EnablePhyTraces ();
    lteHelper->EnableMacTraces ();
    lteHelper->EnableRlcTraces ();
    lteHelper->EnablePdcpTraces ();
    
    Ptr<RadioBearerStatsCalculator> lteRlcStats = lteHelper->GetRlcStats ();
    lteRlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (0.02)));
    Ptr<RadioBearerStatsCalculator> ltePdcpStats = lteHelper->GetPdcpStats ();
    ltePdcpStats->SetAttribute ("EpochDuration", TimeValue (Seconds (0.02)));
    


    ////////////////////////////////////////////////////////////////////////////////////
    ////                          Install AdHoc WiFi                                ////
    ////////////////////////////////////////////////////////////////////////////////////

    Packet::EnablePrinting ();
    PacketSocketHelper wifiPacketSocket;
    wifiPacketSocket.Install(wifiNodes);

    std::string wifiPhyMode ("DsssRate1Mbps");

    // The below set of helpers will help us to put together the wifi NICs we want
    WifiHelper wifi;

    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    // Set it to zero; otherwise, gain will be added
    wifiPhy.Set ("RxGain", DoubleValue (-10) );
    // Ns-3 supports RadioTap and Prism tracing extensions for 802.11b
    wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
    wifiPhy.SetChannel (wifiChannel.Create ());


    // Add an upper mac and disable rate control
    WifiMacHelper wifiMac;
    wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode",StringValue (wifiPhyMode),
                                  "ControlMode",StringValue (wifiPhyMode));
    // Set it to adhoc mode
    wifiMac.SetType ("ns3::AdhocWifiMac");
    NetDeviceContainer wifiDevices = wifi.Install (wifiPhy, wifiMac, wifiNodes);

    wifiPhy.EnablePcap("WifiTestPcap", wifiDevices);

    // Data settings for the packet sender
    PacketSocketAddress wifiSocket;
    wifiSocket.SetSingleDevice (wifiDevices.Get (0)->GetIfIndex ());
    wifiSocket.SetPhysicalAddress (wifiDevices.Get (0)->GetAddress ());
    wifiSocket.SetProtocol (1);
    
    // Communication class, class creates packets
    OnOffHelper wifiOnOff ("ns3::PacketSocketFactory", Address (wifiSocket));
    wifiOnOff.SetConstantRate (DataRate ("500kb/s"));
    
    // Set up packet sending using onoff
    ApplicationContainer wifiApps = wifiOnOff.Install (wifiNodes.Get (0));
    wifiApps.Start (Seconds (0.5));
    wifiApps.Stop (Seconds (70.0));

    Ptr<NetDevice> wifiD0 = wifiDevices.Get(0);
    

    /////////////////////////////////////////////////////////////////////////////////
    ////                                WAVE                                     ////
    /////////////////////////////////////////////////////////////////////////////////
    
    // Setup
    YansWifiChannelHelper waveChannel = YansWifiChannelHelper::Default ();
    YansWavePhyHelper wavePhy =  YansWavePhyHelper::Default ();
    wavePhy.SetChannel (waveChannel.Create ());
    wavePhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO); // Set the pcap to radio which gives more info in the pcap file

    // These parameters devides the transmition to 7 levels
    wavePhy.Set ("TxPowerStart", DoubleValue (7));                     // Min transmition power
    wavePhy.Set ("TxPowerEnd", DoubleValue (7));                       // Max transmition power

    QosWaveMacHelper waveMac = QosWaveMacHelper::Default ();           // Create MAC layer
    WaveHelper waveHelper = WaveHelper::Default ();

    // Rate 6 Mbps and bandwidth 10 MHZ
    waveHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                        "DataMode", StringValue ("OfdmRate6MbpsBW10MHz"	),
                                        "ControlMode",StringValue ("OfdmRate6MbpsBW10MHz"),
                                        "NonUnicastMode", StringValue ("OfdmRate6MbpsBW10MHz")); // Datamode is for broadcasting and ControlMode is for Unicast


    NetDeviceContainer waveDevices = waveHelper.Install (wavePhy, waveMac, waveNodes);

    wavePhy.EnablePcap ("WaveTest", waveDevices);
    // End of setup 

    
    // Prepare a packet with a payload of 1000 Bytes. Basically it has zeros in the payload
    Ptr <Packet> wavePacket = Create <Packet> (1000);

    // Destination MAC
    Mac48Address waveDest = Mac48Address::GetBroadcast();

    uint16_t waveProtocol = 0x88dc;

    // We can also set the transmission parameters at the higher layeres
    TxInfo waveTx;
    waveTx.preamble = WIFI_PREAMBLE_LONG;

    // We set the channel on which the packet is sent. The WaveNetDevice must have access to the channel
    // CCH is enabled by default.
    waveTx.channelNumber = CCH;

    // We can set per-packet data rate. This packet will have a rate of 12Mbps.
    waveTx.dataRate = WifiMode ("OfdmRate12MbpsBW10MHz");

    waveTx.priority = 7;	    //We set the AC to highest priority. We can set this per packet.

    waveTx.txPowerLevel = 6;  //When we define TxPowerStart & TxPowerEnd for a WifiPhy, 7 is correspond to TxPowerEnd, and 1 TxPowerStart, and numbers in between are levels.

    // Let's send a Unicast packet from n0 to n2
    // Get the MAC address of the target node
    std::vector<Ptr<WaveNetDevice>> pointers;
    TxInfo waveTxU;

    for (uint32_t i = 0; i < waveDevices.GetN(); i++){
      Ptr <WaveNetDevice> y = DynamicCast<WaveNetDevice>(waveDevices.Get(i));
      pointers.push_back(y);
    }

    for (uint32_t i = 1; i < simTime - 1; i++){
      for (auto y : pointers){
        Ptr <Packet> waveUnicastPacket = Create<Packet> (200);
        Simulator::Schedule ( Seconds(i) , &WaveNetDevice::SendX, y, waveUnicastPacket, waveDest, waveProtocol, waveTxU );
      }
    }
    

    Simulator::Stop(Seconds (simTime));

    // This if condition can cause undefined behaviour, if you are using WAVE devices put this in a comment block!
    // NetAnim interface CAN NOT be used with WAVE !!!
    //if (waveNodesSet.empty()){
   // AnimationInterface anim("/usr/ns-allinone-3.30/ns-3.30/scratch/P2/NetAnimOutput.xml");
    //}
    
    Simulator::Run();
    Simulator::Destroy();

    std::cout << "End of Program" << std::endl;
    return 0;
}
