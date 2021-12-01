#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>

using namespace ns3;

int main (int argc, char *argv[])
{
    /**
    * This will instantiate some common objects (e.g., the Channel object)
    * and provide the methods to add eNBs and UEs and configure them.
    */
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper> (); //Create an LteHelper object 
    /*
    * Note that the Node instances at this point still don't have an LTE protocol stack installed
    * they are just empty nodes.
    */
    NodeContainer enbNodes; //Create Node objects for the eNB(s) and the UEs
    enbNodes.Create (1);
    NodeContainer ueNodes;
    ueNodes.Create (2);
    /*
    * The below code will place all nodes at the coordinates (0,0,0).
    */
    MobilityHelper mobility; // Configure the Mobility model for all the nodes
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (enbNodes);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (ueNodes);
    /*
    * Install an LTE protocol stack on the eNB(s)
    */
    NetDeviceContainer enbDevs;
    enbDevs = lteHelper->InstallEnbDevice (enbNodes);
    /*
    * Install an LTE protocol stack on the UEs
    */
    NetDeviceContainer ueDevs;
    ueDevs = lteHelper->InstallUeDevice (ueNodes);
    /*
    * Attach the UEs to an eNB. 
    * This will configure each UE according to the eNB configuration, 
    * and create an RRC connection between them
    */
    lteHelper->Attach (ueDevs, enbDevs.Get (0));
    /*
    * Activate a data radio bearer between each UE and the eNB it is attached to.
    * this method will also activate two saturation traffic generators for that bearer,
    * one in uplink and one in downlink.
    */
    enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer (q);
    lteHelper->ActivateDataRadioBearer (ueDevs, bearer);
    /*
    * Set the stop time
    * This is needed otherwise the simulation will last forever,
    * because (among others) the start-of-subframe event is scheduled repeatedly,
    * and the ns-3 simulator scheduler will hence never run out of events.
    */
    Simulator::Stop (Seconds (0.005));

    // Run the simulation
    Simulator::Run ();
    // Cleanup and exit
    Simulator::Destroy ();
    return 0;
}
