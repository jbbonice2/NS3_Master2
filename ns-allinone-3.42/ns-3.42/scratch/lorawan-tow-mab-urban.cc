#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/random-variable-stream.h"
#include "../src/lorawan/model/tow-mab-selector.h"

NS_LOG_COMPONENT_DEFINE("lorawan");

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
            .SetGroupName("lorawan")
            .AddConstructor<MinimalLorawanEndDeviceMac>();
        return tid;
    }

    MinimalLorawanEndDeviceMac() : m_totalTxPackets(0), m_successfulTxPackets(0), m_deviceId(0) {
        // Initialisation des paramètres LoRaWAN par défaut - Environnement Urbain
        m_params.txPower = 14.0;      // 14 dBm (25 mW)
        m_params.bandwidth = 125;      // 125 kHz
        m_params.codingRate = 5;       // 4/5
        m_params.payloadSize = 20;     // 20 bytes
        m_params.range = 2000.0;       // 2 km (urbain)
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
            // InitializeVectors est appelé automatiquement dans le sélecteur
        }
    }

    void StartSending(Time interval) {
        m_sendEvent = Simulator::Schedule(Seconds(0), &MinimalLorawanEndDeviceMac::SendPacket, this);
        m_interval = interval;
    }

    void SendPacket() {
        // Schedule next transmission first
        m_sendEvent = Simulator::Schedule(m_interval, &MinimalLorawanEndDeviceMac::SendPacket, this);

        if (!m_selector) {
            std::cerr << "Erreur: Aucun sélecteur défini pour ce MAC" << std::endl;
            return;
        }

        // Use the ToW MAB selector to choose channel and spreading factor
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
        std::cout << "  Environment: Urban" << std::endl;

        // *** Simplified ACK Simulation ***
        // In a real simulation, this depends on channel conditions, collisions, GW reception etc.
        // Here we use a simple probability model
        Ptr<UniformRandomVariable> rv = CreateObject<UniformRandomVariable>();
        
        // Probabilité de succès plus faible en milieu urbain (plus d'obstacles et d'interférences)
        // SF plus élevé = meilleure sensibilité = meilleure probabilité de succès
        double successProbability = 0.60 + (0.05 * spreadingFactorIndex);
        
        // Limiter à 85% max (environnement urbain plus difficile)
        successProbability = std::min(successProbability, 0.85);
        
        // Ajouter un facteur aléatoire pour simuler les interférences urbaines
        double urbanInterference = rv->GetValue(0.0, 0.15);
        successProbability -= urbanInterference;
        
        bool success = (rv->GetValue(0.0, 1.0) < successProbability);
        
        m_totalTxPackets++;

        std::cout << "  -> Tx Outcome: " << (success ? "Success (ACK)" : "Failure (No ACK)") << std::endl;
        std::cout << "  --------------------------" << std::endl;
        if(success) {
            m_successfulTxPackets++;
        }

        // Update the selector with the outcome
        m_selector->Update(channelIndex, spreadingFactorIndex, success);
    }

    double GetFrameSuccessRate() const {
        if (m_totalTxPackets == 0) return 0.0;
        return (double)m_successfulTxPackets / m_totalTxPackets * 100.0;
    }

    uint64_t GetTotalTxPackets() const {
        return m_totalTxPackets;
    }

    uint64_t GetSuccessfulTxPackets() const {
        return m_successfulTxPackets;
    }

private:
    // Helper function to convert channel index to actual frequency (MHz)
    double GetChannelFrequency(uint32_t channelIndex) {
        // EU868 band frequencies
        const double frequencies[] = {
            868.1, // Channel 0
            868.3, // Channel 1
            868.5, // Channel 2
            867.1, // Channel 3
            867.3, // Channel 4
            867.5, // Channel 5
            867.7, // Channel 6
            867.9  // Channel 7
        };
        
        if (channelIndex < 8) {
            return frequencies[channelIndex];
        }
        return 868.0; // Default if out of range
    }
    
    // Helper function to convert SF index to actual SF value
    uint8_t GetRealSpreadingFactor(uint8_t sfIndex) {
        // LoRaWAN SF values
        const uint8_t spreadingFactors[] = {
            7,  // SF7 - highest data rate, shortest range
            8,  // SF8
            9,  // SF9
            10, // SF10
            11, // SF11
            12  // SF12 - lowest data rate, longest range
        };
        
        if (sfIndex < 6) {
            return spreadingFactors[sfIndex];
        }
        return 7; // Default if out of range
    }

    Ptr<TowMabSelector> m_selector;
    uint64_t m_totalTxPackets;
    uint64_t m_successfulTxPackets;
    uint32_t m_deviceId;
    LoRaWANParams m_params;
    EventId m_sendEvent;
    Time m_interval;
};

NS_OBJECT_ENSURE_REGISTERED(MinimalLorawanEndDeviceMac);

} // namespace lorawan
} // namespace ns3

using namespace ns3;
using namespace lorawan;

int main(int argc, char *argv[]) {
    // Enable logging
    LogComponentEnable("lorawan", LOG_LEVEL_INFO);

    // Create nodes
    int nEndDevices = 10; // Plus de dispositifs en milieu urbain
    NodeContainer endDevices;
    endDevices.Create(nEndDevices);

    std::cout << "Creating urban network with " << nEndDevices << " devices." << std::endl;

    // Simulation parameters
    double simTimeSeconds = 100.0;
    double packetIntervalSeconds = 10.0;
    uint32_t nChannels = 8;         // EU868 band has 8 channels
    uint8_t nSpreadingFactors = 6;  // SF7 to SF12

    std::cout << "Starting simulation for " << simTimeSeconds << " seconds..." << std::endl;

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
        
        // Configure LoRaWAN parameters - Urban environment
        mac->SetTxPower(14.0 + i % 5);  // 14-18 dBm
        mac->SetBandwidth(125);      // 125 kHz (standard)
        mac->SetCodingRate(5);       // 4/5 (standard)
        mac->SetPayloadSize(20 + i * 3); // 20-47 bytes
        mac->SetRange(2000.0 + i * 200.0); // 2-4 km (urbain)
        
        mac->SetSelector(selector);
        mac->SetNumChannelsAndSpreadingFactors(nChannels, nSpreadingFactors);
        endDevices.Get(i)->AggregateObject(mac); // Attach MAC logic to node
        macDevices.push_back(mac);

        // Start packet generation with some random offset
        Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
        Time startTime = Seconds(rng->GetValue(0.1, 1.0));
        Simulator::Schedule(startTime, &MinimalLorawanEndDeviceMac::StartSending, mac, Seconds(packetIntervalSeconds));
    }

    // Run simulation
    Simulator::Stop(Seconds(simTimeSeconds));
    Simulator::Run();
    Simulator::Destroy();

    // Print results
    std::cout << "Simulation finished." << std::endl;
    std::cout << std::endl << "--- Simulation Results ---" << std::endl;
    
    uint64_t totalTx = 0;
    uint64_t totalSuccess = 0;
    
    for (auto mac : macDevices) {
        totalTx += mac->GetTotalTxPackets();
        totalSuccess += mac->GetSuccessfulTxPackets();
    }
    
    std::cout << "Total Transmission Attempts: " << totalTx << std::endl;
    std::cout << "Total Successful Transmissions: " << totalSuccess << std::endl;
    std::cout << "Overall Frame Success Rate (FSR): " << (double)totalSuccess / totalTx * 100.0 << "%" << std::endl;

    return 0;
}
