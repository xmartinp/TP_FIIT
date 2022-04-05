// Scenár s wifi štandardom 802.11p
#include "ns3/wave-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/core-module.h"

#define TEAL_CODE "\033[36m"
#define END_CODE "\033[0m"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WaveExample1");

//Fired when a packet is Enqueued in MAC
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


int main (int argc, char *argv[]){
	CommandLine cmd;
	unit32_t nNodes = 3;//number of nodes
	cmd.AddValue("n","Number of Nodes", nNodes);
	cmd.Parse(argc, argv);
	
	double simTime = 10;
	NodeContainer nodes;
	nodes.Create(nNodes);
	//Number of nodes
	uint32_t nNodes = 3;

	cmd.AddValue ("n","Number of nodes", nNodes);

	cmd.Parse (argc, argv);


	double simTime = 10;
	NodeContainer nodes;
	nodes.Create(nNodes);
	MobilityHelper mobility; //Nodes MUST have some sort of mobility because that's needed to compute the received signal strength
	
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> (); //Create positions for the nodes
	positionAlloc->Add (Vector (0.0, 0.0, 0.0));
	positionAlloc->Add (Vector (5.0, 0.0, 0.0));
	positionAlloc->Add (Vector (5.0, 10.0, 0.0));

	mobility.SetPositionAllocator (positionAlloc);
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (nodes);
	
	YansWifiChannelHelper waveChannel = YansWifiChannelHelper::Default ();
	YansWavePhyHelper wavePhy =  YansWavePhyHelper::Default ();
	wavePhy.SetChannel (waveChannel.Create ());
	wavePhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO); // set the pcap to radio which gives more info in the pcap file
	
	// these parameters devides the transmition to 7 levels
	wavePhy.Set ("TxPowerStart", DoubleValue (8)); // min transmition power
	wavePhy.Set ("TxPowerEnd", DoubleValue (33)); // max transmition power 
	
	QosWaveMacHelper waveMac = QosWaveMacHelper::Default (); // create MAC layer
	WaveHelper waveHelper = WaveHelper::Default ();
	
	//rate 6 Mbps and bandwidth 10 MHZ
	waveHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
					"DataMode", StringValue ("OfdmRate6MbpsBW10MHz"	),
					"ControlMode",StringValue ("OfdmRate6MbpsBW10MHz"),
					"NonUnicastMode", StringValue ("OfdmRate6MbpsBW10MHz")); // Datamode is for broadcasting and ControlMode is for Unicast


	NetDeviceContainer devices = waveHelper.Install (wavePhy, waveMac, nodes);
	wavePhy.EnablePcap ("WaveTest", devices);
	
	/////////////////////////////////////////////////// End of Set-up //////////////////////////////////////
	
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
	
	/*************** Sending a packet ***************/
	
	//Get the WaveNetDevice for the first devices, using node 0.
	Ptr <NetDevice> d0 = devices.Get (0);
	Ptr <WaveNetDevice> wd0 = DynamicCast <WaveNetDevice> (d0); // Dynamically cast to waveNetDevice
	Simulator::Schedule ( Seconds (1) , &WaveNetDevice::SendX, wd0, packet, dest, protocol, tx); // destination of the packet is broadcast
	
	//Let's send a Unicast packet from n0 to n2
	//Get the MAC address of the target node
	Ptr <WaveNetDevice> d2 = DynamicCast<WaveNetDevice>(devices.Get(2));
	Mac48Address target_mac = Mac48Address::ConvertFrom (d2->GetAddress());

	Ptr <Packet> unicast_packet = Create<Packet> (200);
	TxInfo tx_u;
	
	/*
	* Schedule sending from WaveNetDevice 0.
	* Since this is a unicast, the frame will be acknowledged with an acknowledgment frame
	*/
	Simulator::Schedule ( Seconds(simTime-1) , &WaveNetDevice::SendX, wd0, unicast_packet, target_mac, protocol, tx_u );
	
	/*
	*  Trace some MAC layer details
	*/
	Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WaveNetDevice/MacEntities/*/$ns3::OcbWifiMac/*/Queue/Enqueue", MakeCallback (&EnqueueTrace));

	Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WaveNetDevice/MacEntities/*/$ns3::OcbWifiMac/*/Queue/Dequeue", MakeCallback (&DequeueTrace));
	
	Simulator::Stop(Seconds(simTime));
	Simulator::Run();
	Simulator::Destroy();
	
}

