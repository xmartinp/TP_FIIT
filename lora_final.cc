#include "ns3/core-module.h"
 #include "ns3/network-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/point-to-point-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/spectrum-module.h"
#include "ns3/propagation-module.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/internet-apps-module.h"

//Lora scenar - Martin, funkcny, z jiry prekopirovany

 using namespace ns3;

 NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

 int
 main (int argc, char *argv[])
 {
//   CommandLine cmd (__FILE__);
//   cmd.Parse (argc, argv);

   Time::SetResolution (Time::NS);
   LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
   LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
   LogComponentEnable ("LrWpanNetDevice", LOG_LEVEL_ALL);
   LogComponentEnable ("LrWpanPhy", LOG_LEVEL_ALL);

   NodeContainer nodes;
   nodes.Create (2);

   LrWpanHelper lrWpanHelper;
   NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(nodes);
   lrWpanHelper.AssociateToPan (lrwpanDevices, 1);

   PointToPointHelper pointToPoint;
   pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
   pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

   NetDeviceContainer devices;
   devices = pointToPoint.Install (nodes);

   InternetStackHelper stack;
   stack.Install (nodes);

   Ipv4AddressHelper address;
   address.SetBase ("10.1.1.0", "255.255.255.0");

   Ipv4InterfaceContainer interfaces = address.Assign (devices);

   UdpEchoServerHelper echoServer (9);

   ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
   serverApps.Start (Seconds (1.0));
   serverApps.Stop (Seconds (10.0));

   UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
   echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
   echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
   echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

   ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
   clientApps.Start (Seconds (2.0));
   clientApps.Stop (Seconds (10.0));

   Simulator::Run ();
   Simulator::Destroy ();
   return 0;
 }