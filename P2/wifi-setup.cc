#include "wifi-setup.h"

namespace ns3
{

WifiSetup::WifiSetup(){}
WifiSetup::~WifiSetup () {}

NetDeviceContainer WifiSetup::ConfigureDevices (NodeContainer& nodes)
{
  /*
    Setting up WAVE devices. With PHY & MAC using default settings. 
  */
    YansWifiPhyHelper phy;
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    phy.SetChannel(wifiChannel.Create());
    WifiMacHelper mac;
    mac.SetType ("ns3::AdhocWifiMac");
    WifiHelper wifi;
    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    return devices;
}

}
