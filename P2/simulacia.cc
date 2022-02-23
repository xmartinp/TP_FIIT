#include "custom-application.h"
#include "wave-setup.h"
#include "wifi-setup.h"
#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns2-node-utility.h"

#define TEAL_CODE "\033[36m"
#define END_CODE "\033[0m"

using namespace ns3;


void EnqueueTrace(std::string context, Ptr<const WifiMacQueueItem> item){
	std::cout << TEAL_CODE << "A Packet was enqueued : " << context << END_CODE << std::endl;

	Ptr <const Packet> p = item->GetPacket();
	/*
	 * Do something with the packet, like attach a tag. ns3 automatically attaches a timestamp for enqueued packets;
	 */
}

//Fired when a packet is Dequeued from MAC layer. A packet is dequeued before it is transmitted.
void DequeueTrace(std::string context, Ptr<const WifiMacQueueItem> item){
	std::cout << TEAL_CODE << "A Packet was dequeued : " << context << END_CODE << std::endl;

	Ptr <const Packet> p = item->GetPacket();
	Time queue_delay = Simulator::Now() - item->GetTimeStamp();

	//Keep in mind that a packet might get dequeued (dropped_ if it exceeded MaxDelay (default is 500ms)
	std::cout << "\tQueuing delay=" << queue_delay << std::endl;
}

int main (int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse (argc, argv);

    std::string mobility_file = "scratch/P2/ns2mobility.tcl";

    //A tool I created so that we only start the applications within nodes when they actually enter the simulation.
    Ns2NodeUtility ns2_utility (mobility_file);

    uint32_t nnodes = ns2_utility.GetNNodes();
    double sim_time = ns2_utility.GetSimulationTime();

    NodeContainer nodes;
    nodes.Create (nnodes);

    //Using the bulit-in ns-2 mobility helper
    Ns2MobilityHelper sumo_trace (mobility_file);
    sumo_trace.Install(); //install ns-2 mobility in all nodes

    //WAVE
    WaveSetup wave;
    wave.ConfigureDevices(nodes);

    //WIFI
    WifiSetup wifi;
    wifi.ConfigureDevices(nodes);

    std::cout << "NS2 mobility & devices configured..." << std::endl;
    //Let's install my CustomApplication to all nodes and start them at the appropriate time using my utilitiy.
    	//prepare a packet with a payload of 1000 Bytes. Basically it has zeros in the payload
	Ptr <Packet> packet = Create <Packet> (1000);

	//destination MAC
	Mac48Address dest = Mac48Address::GetBroadcast();
	
	/*
	* 0x88dc is the ethertype corresponding to WSMP. IPv4's etherType is 0x0800, and IPv6 is 0x86DD
	* The standard doesn't allow sending IP packets over CCH channel
	*/
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
	
	tx.txPowerLevel = 3; //When we define TxPowerStart & TxPowerEnd for a WifiPhy, 7 is correspond to TxPowerEnd, and 1 TxPowerStart, and numbers in between are levels.
	
    for (uint32_t i=1 ; i<nnodes; i++)
    {
        Ptr<Node> n = nodes.Get(i);
        std::cout << "fuck marek 1" << n <<std::endl;
        Ptr <WaveNetDevice> wd0 = DynamicCast <WaveNetDevice> (n);
        std::cout << "fuck marek 2" << n <<std::endl;
        Simulator::Schedule ( Seconds (1) , &WaveNetDevice::SendX, wd0, packet, dest, protocol, tx); 
        std::cout << "fuck marek 3" << n <<std::endl;
        Ptr<Node> m = nodes.Get(0);
        Ptr <WaveNetDevice> d0 = DynamicCast<WaveNetDevice>(m);
        std::cout << "fuck marek 4_d0 " << d0 <<std::endl;
        std::cout << "fuck marek 4_m " << m <<std::endl;
	    Mac48Address target_mac = Mac48Address::ConvertFrom (d0->GetAddress());
        std::cout << "fuck marek 5" << n <<std::endl;
        Ptr <Packet> unicast_packet = Create<Packet> (200);
        TxInfo tx_u;
        std::cout << "fuck marek 6" << n <<std::endl;
        /*
        * Schedule sending from WaveNetDevice 0.
        * Since this is a unicast, the frame will be acknowledged with an acknowledgment frame
        */
        
        Simulator::Schedule ( Seconds(sim_time-1) , &WaveNetDevice::SendX, wd0, unicast_packet, target_mac, protocol, tx_u );
        std::cout << "fuck marek 7" << n <<std::endl;
        //Ptr<CustomApplication> app = CreateObject <CustomApplication>  ();
        //app->SetStartTime (Seconds (0));
        //app->SetStopTime (Seconds (sim_time));
        //app->SetStartTime(Seconds (ns2_utility.GetEntryTimeForNode(i)));
        //app->SetStopTime (Seconds (ns2_utility.GetExitTimeForNode(i)));

        //n->AddApplication(app);

    }
    std::cout << "Applications setup done!" << std::endl;
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WaveNetDevice/MacEntities/*/$ns3::OcbWifiMac/*/Queue/Enqueue", MakeCallback (&EnqueueTrace));
    std::cout << "fuck marek 10" <<std::endl;
	Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WaveNetDevice/MacEntities/*/$ns3::OcbWifiMac/*/Queue/Dequeue", MakeCallback (&DequeueTrace));
    Simulator::Stop(Seconds (sim_time)); //because this is the last timestamp in your ns-2 trace
    Simulator::Run ();

    std::cout << "End of Program" << std::endl;

}
