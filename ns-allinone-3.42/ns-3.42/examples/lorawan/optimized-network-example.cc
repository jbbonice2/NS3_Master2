/*
 * Copyright (c) 2023 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Your Name <your.email@example.com>
 */

#include "ns3/command-line.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/end-device-lorawan-mac.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/log.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/one-shot-sender-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ToWAgent.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("OptimizedLorawanNetworkExample");

double simulationTime = 2.0;
double alpha = 0.1;
double beta = 0.2;
double A = 0.3;

int
main(int argc, char* argv[])
{
    // Command line arguments
    CommandLine cmd;
    cmd.AddValue("simulationTime", "Simulation time in hours", simulationTime);
    cmd.AddValue("alpha", "Learning rate for ToWAgent", alpha);
    cmd.AddValue("beta", "Exploration parameter for ToWAgent", beta);
    cmd.AddValue("A", "Oscillation amplitude for ToWAgent", A);
    cmd.Parse(argc, argv);

    // Set up logging
    LogComponentEnable("OptimizedLorawanNetworkExample", LOG_LEVEL_ALL);
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
    LogComponentEnable("LorawanMacHeader", LOG_LEVEL_ALL);
    LogComponentEnable("LoraFrameHeader", LOG_LEVEL_ALL);

    // Create the channel
    NS_LOG_INFO("Creating the channel...");
    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76);
    loss->SetReference(1, 7.7);
    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();
    Ptr<LoraChannel> channel = CreateObject<LoraChannel>(loss, delay);

    // Create helpers
    NS_LOG_INFO("Setting up helpers...");
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator>();
    allocator->Add(Vector(1000, 0, 0)); // End device position
    allocator->Add(Vector(0, 0, 0));    // Gateway position
    mobility.SetPositionAllocator(allocator);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    // Create ToWAgent
    ToWAgent towAgent(8, 6, alpha, beta, A); // 8 channels, 6 SFs

    // Create LoraPhyHelper
    LoraPhyHelper phyHelper = LoraPhyHelper();
    phyHelper.SetChannel(channel);

    // Create LorawanMacHelper
    LorawanMacHelper macHelper = LorawanMacHelper();

    // Create LoraHelper
    LoraHelper helper = LoraHelper();

    // Create End Devices
    NS_LOG_INFO("Creating the end device...");
    NodeContainer endDevices;
    endDevices.Create(1);
    mobility.Install(endDevices);

    // Create Gateways
    NS_LOG_INFO("Creating the gateway...");
    NodeContainer gateways;
    gateways.Create(1);
    mobility.Install(gateways);

    // Install devices
    phyHelper.SetDeviceType(LoraPhyHelper::ED);
    macHelper.SetDeviceType(LorawanMacHelper::ED_A);
    helper.Install(phyHelper, macHelper, endDevices);

    phyHelper.SetDeviceType(LoraPhyHelper::GW);
    macHelper.SetDeviceType(LorawanMacHelper::GW);
    helper.Install(phyHelper, macHelper, gateways);

    // Install applications
    OneShotSenderHelper oneShotSenderHelper;
    oneShotSenderHelper.SetSendTime(Seconds(2));
    oneShotSenderHelper.Install(endDevices);

    // Simulation loop
    Simulator::Stop(Hours(simulationTime));
    while (Simulator::IsFinished() == false) {
        // Select optimal channel and SF using ToWAgent
        auto [channel, sf] = towAgent.Select();
        
        // Configure MAC layer with selected parameters
        Config::SetDefault("ns3::LorawanMac::Channel", UintegerValue(channel));
        Config::SetDefault("ns3::LorawanMac::SpreadingFactor", UintegerValue(sf));

        // Run simulation for one time step
        Simulator::Run();
        
        // Check if packet was successfully received
        bool success = false;
        // TODO: Implement packet success check
        
        // Update ToWAgent with feedback
        towAgent.Feedback(success);
    }

    Simulator::Destroy();
    return 0;
}
