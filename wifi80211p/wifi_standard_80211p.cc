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
#include "ns3/wifi-standards.h"

using namespace ns3;

static void
SetPosition (Ptr<Node> node, Vector position)
{
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  mobility->SetPosition (position);
}

static Vector
GetPosition (Ptr<Node> node)
{
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  return mobility->GetPosition ();
}

static void
AdvancePosition (Ptr<Node> node)
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
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  Packet::EnablePrinting ();

  WifiHelper wifi;
  MobilityHelper mobility;
  NodeContainer stas;
  NodeContainer ap;
  NetDeviceContainer staDevs;
  PacketSocketHelper packetSocket;
  
  WifiStandard wifiStandard = WIFI_STANDARD_80211p;
  wifi.SetStandard(wifiStandard);


  stas.Create (2);
  ap.Create (1);

  // give packet socket powers to nodes.
  packetSocket.Install (stas);
  packetSocket.Install (ap);

  WifiMacHelper wifiMac;
  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  Ssid ssid = Ssid ("wifi-default");
  wifi.SetRemoteStationManager ("ns3::ArfWifiManager");
  // setup stas.
  wifiMac.SetType ("ns3::StaWifiMac",
                   "ActiveProbing", BooleanValue (true),
                   "Ssid", SsidValue (ssid));
  staDevs = wifi.Install (wifiPhy, wifiMac, stas);
  // setup ap.
  wifiMac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));
  wifi.Install (wifiPhy, wifiMac, ap);

  // mobility.
  mobility.Install (stas);
  mobility.Install (ap);

  Simulator::Schedule (Seconds (1.0), &AdvancePosition, ap.Get (0));

  PacketSocketAddress socket;
  socket.SetSingleDevice (staDevs.Get (0)->GetIfIndex ());
  socket.SetPhysicalAddress (staDevs.Get (1)->GetAddress ());
  socket.SetProtocol (1);

  OnOffHelper onoff ("ns3::PacketSocketFactory", Address (socket));
  onoff.SetConstantRate (DataRate ("500kb/s"));

  ApplicationContainer apps = onoff.Install (stas.Get (0));
  apps.Start (Seconds (0.5));
  apps.Stop (Seconds (43.0));

  Simulator::Stop (Seconds (44.0));

  AthstatsHelper athstats;
  athstats.EnableAthstats ("athstats-sta", stas);
  athstats.EnableAthstats ("athstats-ap", ap);

  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}