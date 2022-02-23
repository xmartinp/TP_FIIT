#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H
#include "ns3/core-module.h"
#include "ns3/wave-module.h"
#include "ns3/network-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/ssid.h"

namespace ns3
{
/** \brief This is a "utility class". It does not an extension to ns3. 
 */
  class WifiSetup
  {
    public:
      WifiSetup ();
      virtual ~WifiSetup ();

      NetDeviceContainer ConfigureDevices (NodeContainer &n);
  };
}

#endif 
