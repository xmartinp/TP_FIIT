// Scenar s wifi prenosovou technologiou
// Používa sa predvolený štandard wifi prenosu 802.11b
// Scenár neobsahuje komunikáciu vozidiel

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

using namespace ns3;

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
  
  std::cout<<"vypis1\n";
  Packet::EnablePrinting ();
  NodeContainer c;
  c.Create(2);
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
