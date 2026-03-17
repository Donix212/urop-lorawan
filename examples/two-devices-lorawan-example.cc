/*
This test simulates a LoRaWAN network where two devices transmit sequentially at 2.0s and 3.0s.
Because they are separated in time, the Gateway receives both packets without interference,
even though they use the same frequency and spreading factor.
*/
#include "ns3/core-module.h"
#include "ns3/lorawan-module.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("TwoDevicesLorawanExample");

int 
main(int argc, char *argv[])
{
    // Set up Logging

    LogComponentEnable("TwoDevicesLorawanExample", LOG_LEVEL_ALL);
    LogComponentEnable("LoraChannel", LOG_LEVEL_INFO);
    LogComponentEnable("LoraPhy", LOG_LEVEL_ALL);
    LogComponentEnable("EndDeviceLoraPhy", LOG_LEVEL_ALL);
    LogComponentEnable("GatewayLoraPhy", LOG_LEVEL_ALL);
    LogComponentEnable("LoraInterferenceHelper", LOG_LEVEL_ALL);
    LogComponentEnable("LorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("EndDeviceLorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("ClassAEndDeviceLorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("GatewayLorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
    LogComponentEnable("LogicalLoraChannel", LOG_LEVEL_ALL);
    LogComponentEnable("LoraHelper", LOG_LEVEL_ALL);
    LogComponentEnable("LoraPhyHelper", LOG_LEVEL_ALL);
    LogComponentEnable("LorawanMacHelper", LOG_LEVEL_ALL);
    LogComponentEnable("OneShotSenderHelper", LOG_LEVEL_ALL);
    LogComponentEnable("OneShotSender", LOG_LEVEL_ALL);
    LogComponentEnable("LorawanMacHeader", LOG_LEVEL_ALL);
    LogComponentEnable("LoraFrameHeader", LOG_LEVEL_ALL);
    LogComponentEnableAll(LOG_PREFIX_FUNC);
    LogComponentEnableAll(LOG_PREFIX_NODE);
    LogComponentEnableAll(LOG_PREFIX_TIME);

    /************************
     *  Create the channel  *
     ************************/

    NS_LOG_INFO("Creating the channel...");

    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76);
    loss->SetReference(1, 7.7);

    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();

    Ptr<LoraChannel> channel = CreateObject<LoraChannel>(loss, delay);

    /************************
     *  Create the helpers  *
     ************************/

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator>();
    allocator->Add(Vector(500, 0, 0));   // Device 1: 500m away
    allocator->Add(Vector(1500, 0, 0));  // Device 2: 1500m away
    allocator->Add(Vector(0, 0, 0));     // Gateway: Origin
    mobility.SetPositionAllocator(allocator);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    // Create the LoraPhyHelper

    LoraPhyHelper phyHelper = LoraPhyHelper();
    phyHelper.SetChannel(channel);

    // Create the LorawanMacHelper

    LorawanMacHelper macHelper = LorawanMacHelper();

    // Create the LoraHelper

    LoraHelper helper = LoraHelper();

    /************************
     *  Create End Devices  *
     ************************/

    NS_LOG_INFO("Creating the end device...");

    // Create a set of nodes

    NodeContainer endDevices;
    endDevices.Create(2); // Two devices

    // Assign a mobility model to the nodes
    mobility.Install(endDevices);

    // Create the LoraNetDevices of the end devices
    phyHelper.SetDeviceType(LoraPhyHelper::ED);
    macHelper.SetDeviceType(LorawanMacHelper::ED_A);
    helper.Install(phyHelper, macHelper, endDevices);

    /*********************
     *  Create Gateways  *
     *********************/

    NS_LOG_INFO("Creating the gateway...");

    NodeContainer gateways;
    gateways.Create(1);

    mobility.Install(gateways);

    // Create a netdevice for each gateway
    phyHelper.SetDeviceType(LoraPhyHelper::GW);
    macHelper.SetDeviceType(LorawanMacHelper::GW);
    helper.Install(phyHelper, macHelper, gateways);

    /*********************************************
     *  Install applications on the end devices  *
     *********************************************/

    OneShotSenderHelper oneShotSenderHelper;

    // Device 1 (at 500m) sends at 2 seconds
    oneShotSenderHelper.SetSendTime(Seconds(2));
    oneShotSenderHelper.Install(endDevices.Get(0));

    // Device 2 (at 1500m) sends at 3 seconds
    oneShotSenderHelper.SetSendTime(Seconds(3));
    oneShotSenderHelper.Install(endDevices.Get(1));

    /******************
     * Set Data Rates *
     ******************/

    std::vector<int> sfQuantity(6);
    sfQuantity = LorawanMacHelper::SetSpreadingFactorsUp(endDevices, gateways, channel);

    /****************
     *  Simulation  *
     ****************/

    Simulator::Stop(Seconds(10));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}