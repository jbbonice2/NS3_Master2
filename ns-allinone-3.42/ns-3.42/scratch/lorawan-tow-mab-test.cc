// scratch/lorawan-tow-mab-test.cc

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lorawan-module.h" // Assuming this includes necessary headers, might need adjustment
#include "ns3/tow-mab-selector.h" // Include the selector header
#include "ns3/point-to-point-module.h" // For potential GW backhaul
#include "ns3/internet-module.h" // For potential GW backhaul

// Définir le composant de log pour le programme principal
NS_LOG_COMPONENT_DEFINE ("LorawanTowMabTest");

// Placeholder for a minimal End Device MAC that uses TowMabSelector
// In a real scenario, this would interact with PHY layer and handle LoRaWAN MAC procedures.
// Here, it's simplified to focus on the selector integration.
namespace ns3 {
namespace lorawan {

// Structure pour stocker les paramètres de communication LoRaWAN
struct LoRaWANParams {
    double txPower;       // dBm
    uint32_t bandwidth;   // kHz
    uint8_t codingRate;   // 4/x (4/5, 4/6, 4/7, 4/8)
    uint16_t payloadSize; // bytes
    double range;         // meters
};

class MinimalLorawanEndDeviceMac : public Object {
public:
    static TypeId GetTypeId(void) {
        static TypeId tid = TypeId("ns3::lorawan::MinimalLorawanEndDeviceMac")
            .SetParent<Object>()
            .SetGroupName("Lorawan")
            .AddConstructor<MinimalLorawanEndDeviceMac>()
            .AddAttribute("TowMabSelector",
                          "Pointer to the TowMabSelector instance",
                          PointerValue(),
                          MakePointerAccessor(&MinimalLorawanEndDeviceMac::m_selector),
                          MakePointerChecker<TowMabSelector>());
        return tid;
    }

    MinimalLorawanEndDeviceMac() : m_totalTxPackets(0), m_successfulTxPackets(0), m_deviceId(0) {
        // Initialisation des paramètres LoRaWAN par défaut
        m_params.txPower = 14.0;      // 14 dBm (25 mW)
        m_params.bandwidth = 125;      // 125 kHz
        m_params.codingRate = 5;       // 4/5
        m_params.payloadSize = 20;     // 20 bytes
        m_params.range = 5000.0;       // 5 km
    }
    virtual ~MinimalLorawanEndDeviceMac() {}

    void SetSelector(Ptr<TowMabSelector> selector) {
        m_selector = selector;
    }
    
    void SetDeviceId(uint32_t id) {
        m_deviceId = id;
    }
    
    void SetTxPower(double txPower) {
        m_params.txPower = txPower;
    }
    
    void SetBandwidth(uint32_t bandwidth) {
        m_params.bandwidth = bandwidth;
    }
    
    void SetCodingRate(uint8_t codingRate) {
        m_params.codingRate = codingRate;
    }
    
    void SetPayloadSize(uint16_t payloadSize) {
        m_params.payloadSize = payloadSize;
    }
    
    void SetRange(double range) {
        m_params.range = range;
    }

    void SetNumChannelsAndSpreadingFactors(uint32_t nChannels, uint8_t nSpreadingFactors) {
        if (m_selector) {
            m_selector->SetNumberOfChannels(nChannels);
            m_selector->SetNumberOfSpreadingFactors(nSpreadingFactors);
        }
    }

    // Simulate sending a packet periodically
    void StartSending(Time interval) {
        Simulator::Schedule(interval, &MinimalLorawanEndDeviceMac::SendPacket, this, interval);
    }

    // Helper methods to convert indices to real values
    double GetChannelFrequency(uint32_t channelIndex) {
        // EU868 channel frequencies in MHz
        static const double frequencies[] = {
            868.1, // Channel 0
            868.3, // Channel 1
            868.5, // Channel 2
            867.1, // Channel 3
            867.3, // Channel 4
            867.5, // Channel 5
            867.7, // Channel 6
            867.9  // Channel 7
        };
        
        if (channelIndex < sizeof(frequencies)/sizeof(frequencies[0])) {
            return frequencies[channelIndex];
        }
        return 868.0; // Default if out of range
    }
    
    uint8_t GetRealSpreadingFactor(uint8_t sfIndex) {
        // Convert SF index to real SF value (SF7-SF12)
        return sfIndex + 7;
    }

    void SendPacket(Time interval) {
        if (!m_selector) {
            std::cout << "TowMabSelector not set!" << std::endl;
            Simulator::Schedule(interval, &MinimalLorawanEndDeviceMac::SendPacket, this, interval); // Reschedule
            return;
        }

        m_totalTxPackets++;
        std::pair<uint32_t, uint8_t> selection = m_selector->SelectChannelAndSpreadingFactor();
        uint32_t channelIndex = selection.first;
        uint8_t spreadingFactorIndex = selection.second;
        
        // Get real values
        double frequency = GetChannelFrequency(channelIndex);
        uint8_t realSF = GetRealSpreadingFactor(spreadingFactorIndex);
        
        // Calculer la portée estimée en fonction du SF
        // SF plus élevé = portée plus grande
        double estimatedRange = m_params.range * (1.0 + 0.3 * spreadingFactorIndex);
        
        // Afficher tous les paramètres de communication
        std::cout << Simulator::Now().GetSeconds() << "s: ED" << m_deviceId << " attempting Tx with parameters:" << std::endl;
        std::cout << "  Channel: " << channelIndex << " (" << frequency << " MHz)" << std::endl;
        std::cout << "  SF: " << (unsigned)spreadingFactorIndex << " (SF" << (unsigned)realSF << ")" << std::endl;
        std::cout << "  Tx Power: " << m_params.txPower << " dBm" << std::endl;
        std::cout << "  Bandwidth: " << m_params.bandwidth << " kHz" << std::endl;
        std::cout << "  Coding Rate: 4/" << (unsigned)m_params.codingRate << std::endl;
        std::cout << "  Payload Size: " << m_params.payloadSize << " bytes" << std::endl;
        std::cout << "  Estimated Range: " << estimatedRange << " meters" << std::endl;

        // *** Simplified ACK Simulation ***
        // In a real simulation, this depends on channel conditions, collisions, GW reception etc.
        // Here, we use a simple random probability for demonstration.
        Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
        bool success = rng->GetValue(0, 100) < 75; // Example: 75% base success rate (adjust as needed)
        // A more realistic simulation would involve PHY layer interactions.

        std::cout << "  -> Tx Outcome: " << (success ? "Success (ACK)" : "Failure (No ACK)") << std::endl;
        std::cout << "  --------------------------" << std::endl;
        if(success) {
            m_successfulTxPackets++;
        }

        // Update the selector
        m_selector->Update(channelIndex, spreadingFactorIndex, success);

        // Schedule next transmission
        Simulator::Schedule(interval, &MinimalLorawanEndDeviceMac::SendPacket, this, interval);
    }

    uint64_t GetTotalTxPackets() const { return m_totalTxPackets; }
    uint64_t GetSuccessfulTxPackets() const { return m_successfulTxPackets; }

private:
    Ptr<TowMabSelector> m_selector;
    uint64_t m_totalTxPackets;
    uint64_t m_successfulTxPackets;
    uint32_t m_deviceId;
    LoRaWANParams m_params;
};

NS_OBJECT_ENSURE_REGISTERED(MinimalLorawanEndDeviceMac);

} // namespace lorawan
} // namespace ns3

using namespace ns3;
using namespace ns3::lorawan;

int main (int argc, char *argv[])
{
    // Simulation parameters
    double simulationTime = 100.0; // seconds
    int nEndDevices = 5;
    double packetIntervalSeconds = 10.0; // Average interval between packets per device
    uint32_t nChannels = 8; // Example: 8 channels
    uint8_t nSpreadingFactors = 6;  // Example: SF7-SF12

    CommandLine cmd (__FILE__);
    cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
    cmd.AddValue ("nEndDevices", "Number of end devices", nEndDevices);
    cmd.AddValue ("packetInterval", "Interval between packets in seconds", packetIntervalSeconds);
    cmd.AddValue ("nChannels", "Number of available channels", nChannels);
    cmd.AddValue ("nSpreadingFactors", "Number of available Spreading Factors", nSpreadingFactors);
    cmd.Parse (argc, argv);

    // Enable logging
    // LogComponentEnable("TowMabSelector", LOG_LEVEL_ALL);
    LogComponentEnable("LorawanTowMabTest", LOG_LEVEL_INFO);

    NS_LOG_INFO("Creating network with " << nEndDevices << " devices.");

    // Create Nodes (simplified: no GW node needed for this minimal MAC test)
    NodeContainer endDevices;
    endDevices.Create (nEndDevices);

    // Install Mobility (static for simplicity)
    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (endDevices);

    // Create and install minimal MAC and Selector for each device
    std::vector<Ptr<MinimalLorawanEndDeviceMac>> macDevices;
    for (int i = 0; i < nEndDevices; ++i)
    {
        Ptr<MinimalLorawanEndDeviceMac> mac = CreateObject<MinimalLorawanEndDeviceMac>();
        Ptr<TowMabSelector> selector = CreateObject<TowMabSelector>();
        
        // Configure selector attributes
        selector->SetAttribute("Alpha", DoubleValue(0.9));
        selector->SetAttribute("Beta", DoubleValue(0.99));
        selector->SetAttribute("OscAmplitude", DoubleValue(0.1));
        selector->SetAttribute("MaxRetransmissions", UintegerValue(3));
        selector->SetAttribute("TransmissionInterval", DoubleValue(10.0));
        
        // Set device ID
        mac->SetDeviceId(i + 1);
        
        // Configure LoRaWAN parameters - varier légèrement pour chaque appareil
        mac->SetTxPower(14.0 + i);  // 14-18 dBm
        mac->SetBandwidth(125);      // 125 kHz (standard)
        mac->SetCodingRate(5);       // 4/5 (standard)
        mac->SetPayloadSize(20 + i * 5); // 20-40 bytes
        mac->SetRange(5000.0 + i * 500.0); // 5-7 km
        
        mac->SetSelector(selector);
        mac->SetNumChannelsAndSpreadingFactors(nChannels, nSpreadingFactors);
        endDevices.Get(i)->AggregateObject(mac); // Attach MAC logic to node
        macDevices.push_back(mac);

        // Start packet generation with some random offset
        Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
        Time startTime = Seconds(rng->GetValue(0.1, 1.0));
        Simulator::Schedule(startTime, &MinimalLorawanEndDeviceMac::StartSending, mac, Seconds(packetIntervalSeconds));
    }

    NS_LOG_INFO ("Starting simulation for " << simulationTime << " seconds...");
    Simulator::Stop (Seconds (simulationTime));
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Simulation finished.");

    // Collect and print results
    uint64_t totalTx = 0;
    uint64_t totalSuccess = 0;
    for (const auto& mac : macDevices)
    {
        totalTx += mac->GetTotalTxPackets();
        totalSuccess += mac->GetSuccessfulTxPackets();
    }

    double fsr = (totalTx > 0) ? (double)totalSuccess / totalTx : 0.0;
    std::cout << "\n--- Simulation Results ---" << std::endl;
    std::cout << "Total Transmission Attempts: " << totalTx << std::endl;
    std::cout << "Total Successful Transmissions: " << totalSuccess << std::endl;
    std::cout << "Overall Frame Success Rate (FSR): " << fsr * 100.0 << "%" << std::endl;

    return 0;
}
