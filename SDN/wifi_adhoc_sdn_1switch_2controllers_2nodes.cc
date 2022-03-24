#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/double.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-address.h"
#include "ns3/athstats-helper.h"
#include "ns3/ofswitch13-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"

using namespace ns3;

class Controller0;
class Controller1;

static void SetPosition (Ptr<Node> node, Vector position)
{
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  mobility->SetPosition (position);
}

static Vector GetPosition (Ptr<Node> node)
{
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  return mobility->GetPosition ();
}

static void AdvancePosition (Ptr<Node> node)
{
  Vector pos = GetPosition (node);
  pos.x += 5.0;
  if (pos.x >= 210.0)
  {
    return;
  }
  SetPosition (node, pos);

  Simulator::Schedule (Seconds (1.0), &AdvancePosition, node);
}


int main (int argc, char *argv[])
{

  OFSwitch13Helper::EnableDatapathLogs ();
  LogComponentEnable ("OFSwitch13Interface", LOG_LEVEL_ALL);
  LogComponentEnable ("OFSwitch13Device", LOG_LEVEL_ALL);
  LogComponentEnable ("OFSwitch13Port", LOG_LEVEL_ALL);
  LogComponentEnable ("OFSwitch13Queue", LOG_LEVEL_ALL);
  LogComponentEnable ("OFSwitch13SocketHandler", LOG_LEVEL_ALL);
  LogComponentEnable ("OFSwitch13Controller", LOG_LEVEL_ALL);
  LogComponentEnable ("OFSwitch13LearningController", LOG_LEVEL_ALL);
  LogComponentEnable ("OFSwitch13Helper", LOG_LEVEL_ALL);
  LogComponentEnable ("OFSwitch13InternalHelper", LOG_LEVEL_ALL);
  
  std::cout<<"vypis1\n";
  Packet::EnablePrinting ();
	
  //enable chceck sum computations (reuqired!)
  GlobalValue::Bind("ChecksumEnabled", BooleanValue (true));

  NodeContainer c;
  c.Create(2);

  //create the switch node
  Ptr<Node> switchNode = CreateObject<Node> ();

  //
  CsmaHelper csmaHelper;
  csmaHelper.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
  csmaHelper.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  NetDeviceContainer hostDevices;
  NetDeviceContainer switchPorts;

  for (size_t i=0; i< c.GetN (); i++)
  	{
		NodeContainer pair (c.Get(i), switchNode);
		NetDeviceContainer link = csmaHelper.Install (pair);
		hostDevices.Add (link.Get (0));
		switchPorts.Add (link.Get (1));

  	}

  // Create two controller nodes
  NodeContainer controllers;
  controllers.Create (2);
  
  // Configure the OpenFlow network domain
  
  Ptr<OFSwitch13InternalHelper> of13Helper = CreateObject<OFSwitch13InternalHelper> ();
  Ptr<Controller0> ctrl0 = CreateObject<Controller0> ();
  Ptr<Controller1> ctrl1 = CreateObject<Controller1> ();
  of13Helper->InstallController (controllers.Get (0), ctrl0);
  of13Helper->InstallController (controllers.Get (1), ctrl1);

  of13Helper->InstallSwitch (switchNode, switchPorts);
  of13Helper->CreateOpenFlowChannels ();

  
  PacketSocketHelper packetSocket;
  packetSocket.Install(c);

  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel", "MinLoss",DoubleValue(250));
  phy.SetChannel(wifiChannel.Create());
      std::cout<<"vypis2\n";
  WifiMacHelper mac;
      mac.SetType ("ns3::AdhocWifiMac");
      WifiHelper wifi;
      NetDeviceContainer devices = wifi.Install(phy, mac, c);

  MobilityHelper mobility;

  Ptr < ListPositionAllocator > positionAlloc = CreateObject < ListPositionAllocator > ();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(5.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  mobility.Install(c);
      std::cout<<"vypis3\n";

  Simulator::Schedule (Seconds (1.0), &AdvancePosition, c.Get (1));

  //Toto nastaví udáje pre odosielatela packetov
  PacketSocketAddress socket;
  socket.SetSingleDevice (devices.Get (0)->GetIfIndex ());
  socket.SetPhysicalAddress (devices.Get (1)->GetAddress ());
  socket.SetProtocol (1);
  std::cout<<"vypis4\n";
  // trieda na vytváranie komunikácie, trieda vytvára pakety
  OnOffHelper onoff ("ns3::PacketSocketFactory", Address (socket));
  onoff.SetConstantRate (DataRate ("500kb/s"));
  //    onoff.SetAttribute("PacketSize", UintegerValue(2000));
  
  
  of13Helper->EnableOpenFlowPcap ("openflow");
  of13Helper->EnableDatapathStats ("switch-stats");
  csmaHelper.EnablePcap ("switch", switchPorts, true);
  csmaHelper.EnablePcap ("host", hostDevices);
  

  // nastavenie posielania paketov pomocou onoff.
  ApplicationContainer apps = onoff.Install (c.Get (0));
  apps.Start (Seconds (0.5));
  apps.Stop (Seconds (43.0));

  Simulator::Stop (Seconds (44.0));

  AthstatsHelper athstats;
  athstats.EnableAthstats ("athstats-sta", c);

  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}


/** Controller 0 installs the rule to forward packets from host 0 to 1 (port 1 to 2). */
class Controller0 : public OFSwitch13Controller
{
protected:
  // Inherited from OFSwitch13Controller
  void HandshakeSuccessful (Ptr<const RemoteSwitch> swtch);
};

void
Controller0::HandshakeSuccessful (Ptr<const RemoteSwitch> swtch)
{
  DpctlExecute (swtch->GetDpId (), "flow-mod cmd=add,table=0,prio=1 in_port=1 write:output=2");
}

/** Controller 1 installs the rule to forward packets from host 1 to 0 (port 2 to 1). */
class Controller1 : public OFSwitch13Controller
{
protected:
  // Inherited from OFSwitch13Controller
  void HandshakeSuccessful (Ptr<const RemoteSwitch> swtch);
};

void
Controller1::HandshakeSuccessful (Ptr<const RemoteSwitch> swtch)
{
  DpctlExecute (swtch->GetDpId (), "flow-mod cmd=add,table=0,prio=1 in_port=2 write:output=1");
}
