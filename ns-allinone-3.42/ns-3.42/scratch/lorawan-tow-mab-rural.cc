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

// Structure pour stocker les statistiques de liaison radio
struct LinkStats {
    uint32_t deviceId;         // ID du dispositif
    Vector position;           // position du dispositif
    double distance;           // distance à la gateway
    uint64_t txPackets;        // paquets transmis
    uint64_t rxPackets;        // paquets reçus
    double lastRssi;           // dernier RSSI (dBm)
    double lastSnr;            // dernier SNR (dB)
    uint8_t sf;                // facteur d'étalement
    uint32_t bw;               // bande passante (kHz)
    uint8_t cr;                // coding rate
    double txPower;            // puissance de transmission (dBm)
    double pathLoss;           // perte de chemin (dB)
    double energyConsumed;     // énergie consommée (en joules)
};

// Classe Gateway LoRaWAN simplifiée pour environnement rural
class MinimalLorawanGateway : public Object {
public:
    static TypeId GetTypeId(void) {
        static TypeId tid = TypeId("ns3::lorawan::MinimalLorawanGateway")
            .SetParent<Object>()
            .SetGroupName("lorawan")
            .AddConstructor<MinimalLorawanGateway>();
        return tid;
    }
    
    MinimalLorawanGateway() : m_totalRxPackets(0) {
        // Définir la sensibilité du récepteur par SF (dBm)
        // Valeurs typiques pour un récepteur LoRa SX1276
        m_sensitivity[7] = -123.0;  // SF7
        m_sensitivity[8] = -126.0;  // SF8
        m_sensitivity[9] = -129.0;  // SF9
        m_sensitivity[10] = -132.0; // SF10
        m_sensitivity[11] = -133.0; // SF11
        m_sensitivity[12] = -136.0; // SF12
        
        // Générateur de nombres aléatoires
        m_rng = CreateObject<UniformRandomVariable>();
    }
    
    void SetPosition(Vector position) {
        m_position = position;
    }
    
    Vector GetPosition() const {
        return m_position;
    }
    
    // Recevoir un paquet d'un dispositif
    bool ReceivePacket(uint32_t deviceId, const Vector& devicePosition, double txPowerDbm, 
                       uint8_t sf, uint32_t bw, uint8_t cr, double& rssi, double& snr) {
        // Calculer la distance entre le dispositif et la gateway
        double distance = CalculateDistance(devicePosition, m_position);
        
        // Calculer la perte de chemin (modèle simplifié pour environnement rural)
        // Version simplifiée pour 868 MHz:
        double pathLoss = 100.0 + 20.0 * std::log10(distance / 1000.0); // Environnement rural (moins d'atténuation)
        
        // Calculer le RSSI
        rssi = txPowerDbm - pathLoss;
        
        // Calculer le SNR (valeurs typiques entre -20 et +10 dB pour LoRa)
        // En milieu rural, le SNR est généralement plus élevé qu'en milieu urbain
        snr = m_rng->GetValue(-5.0, 15.0); // Valeurs plus élevées pour milieu rural
        
        // Déterminer si le paquet est reçu avec succès en fonction du RSSI et de la sensibilité
        bool success = (rssi > m_sensitivity[sf]) && (snr > -5.0); // -5.0 dB est le SNR minimal pour la démodulation LoRa
        
        if (success) {
            m_totalRxPackets++;
            m_lastRxInfo[deviceId] = {rssi, snr, pathLoss};
        }
        
        return success;
    }
    
    uint64_t GetTotalRxPackets() const {
        return m_totalRxPackets;
    }
    
    struct RxInfo {
        double rssi;
        double snr;
        double pathLoss;
    };
    
    RxInfo GetLastRxInfo(uint32_t deviceId) const {
        auto it = m_lastRxInfo.find(deviceId);
        if (it != m_lastRxInfo.end()) {
            return it->second;
        }
        return {-200.0, -200.0, 0.0}; // Valeurs par défaut si aucune information n'est disponible
    }
    
    // Récupérer les statistiques de liaison pour un dispositif
    LinkStats GetLinkStats(uint32_t deviceId) const {
        LinkStats stats;
        stats.deviceId = deviceId;
        
        auto it = m_lastRxInfo.find(deviceId);
        if (it != m_lastRxInfo.end()) {
            stats.lastRssi = it->second.rssi;
            stats.lastSnr = it->second.snr;
            stats.pathLoss = it->second.pathLoss;
        } else {
            stats.lastRssi = -200.0;
            stats.lastSnr = -200.0;
            stats.pathLoss = 0.0;
        }
        
        // Initialiser les autres champs pour éviter les valeurs indéfinies
        stats.txPackets = 0;
        stats.rxPackets = 0;
        stats.sf = 0;
        stats.bw = 0;
        stats.cr = 0;
        stats.txPower = 0.0;
        stats.distance = 0.0;
        stats.energyConsumed = 0.0;
        
        return stats;
    }
    
private:
    Vector m_position;                     // Position de la gateway
    uint64_t m_totalRxPackets;            // Nombre total de paquets reçus
    std::map<uint8_t, double> m_sensitivity; // Sensibilité du récepteur par SF
    std::map<uint32_t, RxInfo> m_lastRxInfo; // Dernière information de réception par dispositif
    Ptr<UniformRandomVariable> m_rng;      // Générateur de nombres aléatoires
    
    // Calculer la distance euclidienne entre deux points
    double CalculateDistance(const Vector& a, const Vector& b) const {
        return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2) + std::pow(a.z - b.z, 2));
    }
};

NS_OBJECT_ENSURE_REGISTERED(MinimalLorawanGateway);

class MinimalLorawanEndDeviceMac : public Object {
public:
    static TypeId GetTypeId(void) {
        static TypeId tid = TypeId("ns3::lorawan::MinimalLorawanEndDeviceMac")
            .SetParent<Object>()
            .SetGroupName("lorawan")
            .AddConstructor<MinimalLorawanEndDeviceMac>();
        return tid;
    }

    MinimalLorawanEndDeviceMac() : m_totalTxPackets(0), m_successfulTxPackets(0), m_deviceId(0), m_energyConsumed(0.0) {
        // Initialisation des paramètres LoRaWAN par défaut - Environnement Rural
        m_params.txPower = 14.0;      // 14 dBm (25 mW)
        m_params.bandwidth = 125;      // 125 kHz
        m_params.codingRate = 5;       // 4/5
        m_params.payloadSize = 20;     // 20 bytes
        m_params.range = 10000.0;      // 10 km (rural)
        
        // Initialiser le générateur de nombres aléatoires
        m_rng = CreateObject<UniformRandomVariable>();
        
        // Initialiser la position du dispositif
        m_position = Vector(0, 0, 0);
    }
    virtual ~MinimalLorawanEndDeviceMac() {}

    void SetSelector(Ptr<TowMabSelector> selector) {
        m_selector = selector;
    }

    void SetPosition(Vector position) {
        m_position = position;
    }

    Vector GetPosition() const {
        return m_position;
    }

    void SetGateway(Ptr<MinimalLorawanGateway> gateway) {
        m_gateway = gateway;
    }
    
    void SetDeviceId(uint32_t id) {
        m_deviceId = id;
    }
    
    uint32_t GetDeviceId() const {
        return m_deviceId;
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
        
        if (!m_gateway) {
            std::cerr << "Erreur: Aucune gateway définie pour ce MAC" << std::endl;
            return;
        }

        // Use the ToW MAB selector to choose channel and spreading factor
        std::pair<uint32_t, uint8_t> selection = m_selector->SelectChannelAndSpreadingFactor();
        uint32_t channelIndex = selection.first;
        uint8_t spreadingFactorIndex = selection.second;

        // Get real values
        double frequency = GetChannelFrequency(channelIndex);
        uint8_t realSF = GetRealSpreadingFactor(spreadingFactorIndex);
        
        // Calcul du temps de transmission (ToA) en fonction des paramètres
        double timeOnAir = CalculateTimeOnAir(m_params.payloadSize, realSF, m_params.bandwidth, m_params.codingRate);
        
        // Calcul de l'énergie consommée en fonction du temps d'émission et de la puissance
        double txPowerMw = std::pow(10, m_params.txPower / 10.0); // Conversion dBm en mW
        double energyConsumed = (txPowerMw / 1000.0) * (timeOnAir / 1000.0); // Énergie en joules (W*s)
        m_energyConsumed += energyConsumed;
        
        // Distance entre le dispositif et la gateway
        double distance = CalculateDistance(m_position, m_gateway->GetPosition());
        
        // Afficher tous les paramètres de communication
        std::cout << Simulator::Now().GetSeconds() << "s: ED" << m_deviceId << " attempting Tx with parameters:" << std::endl;
        std::cout << "  Position: (" << m_position.x << ", " << m_position.y << ", " << m_position.z << ")" << std::endl;
        std::cout << "  Distance to Gateway: " << distance << " meters" << std::endl;
        std::cout << "  Channel: " << channelIndex << " (" << frequency << " MHz)" << std::endl;
        std::cout << "  SF: " << (unsigned)spreadingFactorIndex << " (SF" << (unsigned)realSF << ")" << std::endl;
        std::cout << "  Tx Power: " << m_params.txPower << " dBm" << std::endl;
        std::cout << "  Bandwidth: " << m_params.bandwidth << " kHz" << std::endl;
        std::cout << "  Coding Rate: 4/" << (unsigned)m_params.codingRate << std::endl;
        std::cout << "  Payload Size: " << m_params.payloadSize << " bytes" << std::endl;
        std::cout << "  Time on Air: " << timeOnAir << " ms" << std::endl;
        std::cout << "  Energy Consumed: " << energyConsumed * 1000 << " mJ" << std::endl;
        std::cout << "  Environment: Rural" << std::endl;

        // Communication avec la gateway
        double rssi = 0.0;
        double snr = 0.0;
        bool success = m_gateway->ReceivePacket(m_deviceId, m_position, m_params.txPower, realSF, 
                                              m_params.bandwidth, m_params.codingRate, rssi, snr);
        
        m_totalTxPackets++;
        if (success) {
            m_successfulTxPackets++;
            std::cout << "  -> Tx SUCCESS: RSSI=" << rssi << " dBm, SNR=" << snr << " dB" << std::endl;
        } else {
            std::cout << "  -> Tx FAILURE: RSSI=" << rssi << " dBm, SNR=" << snr << " dB" << std::endl;
        }
        std::cout << "  --------------------------" << std::endl;

        // Update the selector with the outcome
        m_selector->Update(channelIndex, spreadingFactorIndex, success);
        
        // Mettre à jour les stats par transmission
        m_lastTransmissionStats.txPower = m_params.txPower;
        m_lastTransmissionStats.sf = realSF;
        m_lastTransmissionStats.bw = m_params.bandwidth;
        m_lastTransmissionStats.cr = m_params.codingRate;
        m_lastTransmissionStats.distance = distance;
        m_lastTransmissionStats.rssi = rssi;
        m_lastTransmissionStats.snr = snr;
        m_lastTransmissionStats.success = success;
        m_lastTransmissionStats.energyConsumed = energyConsumed;
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
    
    double GetEnergyConsumed() const {
        return m_energyConsumed;
    }
    
    // Structure pour stocker les statistiques de transmission
    struct TransmissionStats {
        double txPower;
        uint8_t sf;
        uint32_t bw;
        uint8_t cr;
        double distance;
        double rssi;
        double snr;
        bool success;
        double energyConsumed;
    };
    
    TransmissionStats GetLastTransmissionStats() const {
        return m_lastTransmissionStats;
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

    // Calculer le temps de transmission (ToA) en millisecondes
    double CalculateTimeOnAir(uint16_t payloadSize, uint8_t sf, uint32_t bw, uint8_t cr) {
        // Facteurs qui influencent le temps de transmission
        const bool headerEnabled = true;      // Header LoRa activé
        const bool crcEnabled = true;         // CRC activé
        const uint8_t preambleLength = 8;     // Longueur du préambule
        
        // Taille totale des données à transmettre (octets)
        uint16_t payloadSymbolNb = payloadSize;
        if (headerEnabled) {
            payloadSymbolNb += 13; // Taille du header LoRaWAN (MHDR, DevAddr, FCnt, FPort...)
        }
        if (crcEnabled) {
            payloadSymbolNb += 2; // CRC sur 2 octets
        }
        
        // Nombre de bits par symbole
        double bitsPerSymbol = sf;
        
        // Bande passante en Hz
        double bandwidth = bw * 1000.0;
        
        // Calcul du Spreading Factor
        double rsymb = bandwidth / (1 << sf);
        
        // Durée d'un symbole (ms)
        double Tsymb = 1000.0 / rsymb;
        
        // Durée du préambule (ms)
        double Tpreamble = (preambleLength + 4.25) * Tsymb;
        
        // Nombre de symboles pour la charge utile
        uint16_t payloadSymbNb = 8 + (std::ceil((8.0 * payloadSymbolNb - 4.0 * sf + 28.0 + 16.0 * (crcEnabled ? 1.0 : 0.0)) / (4.0 * (sf - 2.0 * (sf <= 6 ? 0 : 1))))) * (cr + 4);
        
        // Durée de la charge utile (ms)
        double Tpayload = payloadSymbNb * Tsymb;
        
        // Temps total de transmission (ms)
        return Tpreamble + Tpayload;
    }
    
    // Calculer la distance euclidienne entre deux points
    double CalculateDistance(const Vector& a, const Vector& b) const {
        return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2) + std::pow(a.z - b.z, 2));
    }
    
    Ptr<TowMabSelector> m_selector;
    Ptr<MinimalLorawanGateway> m_gateway; // Référence à la gateway
    Vector m_position;                   // Position du dispositif
    uint64_t m_totalTxPackets;
    uint64_t m_successfulTxPackets;
    uint32_t m_deviceId;
    LoRaWANParams m_params;
    EventId m_sendEvent;
    Time m_interval;
    double m_energyConsumed;             // Énergie totale consommée en joules
    TransmissionStats m_lastTransmissionStats; // Statistiques de la dernière transmission
    Ptr<UniformRandomVariable> m_rng;    // Générateur de nombres aléatoires
};

NS_OBJECT_ENSURE_REGISTERED(MinimalLorawanEndDeviceMac);

} // namespace lorawan
} // namespace ns3

using namespace ns3;
using namespace lorawan;

int main(int argc, char *argv[]) {
    // Paramètres par défaut
    int nEndDevices = 5;
    std::string outputFile = "lorawan-tow-mab-rural-characterization.txt";
    
    // Traitement des arguments de ligne de commande
    CommandLine cmd(__FILE__);
    cmd.AddValue("nDevices", "Nombre de dispositifs LoRaWAN", nEndDevices);
    cmd.AddValue("outputFile", "Fichier de sortie pour la caractérisation du réseau", outputFile);
    cmd.Parse(argc, argv);
    
    // Enable logging
    LogComponentEnable("lorawan", LOG_LEVEL_INFO);

    // Create nodes for end devices and gateway
    NodeContainer endDevices;
    endDevices.Create(nEndDevices);
    
    NodeContainer gatewayNodes;
    gatewayNodes.Create(1);

    std::cout << "Creating network with " << nEndDevices << " devices and 1 gateway." << std::endl;

    // Simulation parameters
    double simTimeSeconds = 600.0;  // 10 minutes
    double packetIntervalSeconds = 60.0; // 1 minute
    uint32_t nChannels = 8;         // EU868 band has 8 channels
    uint8_t nSpreadingFactors = 6;  // SF7 to SF12

    // Create a gateway
    Ptr<MinimalLorawanGateway> gateway = CreateObject<MinimalLorawanGateway>();
    
    // Position the gateway at the center (0,0,15) - à 15m de hauteur
    Vector gatewayPosition(0, 0, 15);
    gateway->SetPosition(gatewayPosition);
    gatewayNodes.Get(0)->AggregateObject(gateway);
    
    std::cout << "Gateway positioned at (" << gatewayPosition.x << ", " 
              << gatewayPosition.y << ", " << gatewayPosition.z << ")" << std::endl;
    
    // Random variable for positioning devices
    Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
    
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
        selector->SetAttribute("TransmissionInterval", DoubleValue(packetIntervalSeconds));
        
        // Set device ID
        mac->SetDeviceId(i + 1);
        
        // Configure LoRaWAN parameters - Rural environment
        mac->SetTxPower(14.0 + (i % 3));  // 14-16 dBm
        mac->SetBandwidth(125);      // 125 kHz (standard)
        mac->SetCodingRate(5);       // 4/5 (standard)
        mac->SetPayloadSize(20 + i * 2); // 20-28 bytes
        mac->SetRange(10000.0 + i * 500.0); // 10-12.5 km (rural)
        
        // Position the device randomly in a circle around the gateway
        // Distance between 100m and 5km
        double distance = 100.0 + rng->GetValue(0, 1) * 4900.0; // Distance en mètres (de 100m à 5km)
        double angle = rng->GetValue(0, 2 * M_PI);              // Angle en radians (0 à 2π)
        
        // Calculer la position en coordonnées cartésiennes
        double x = distance * std::cos(angle);
        double y = distance * std::sin(angle);
        double z = 1.5; // hauteur de 1.5m (typique pour un dispositif)
        
        Vector devicePosition(x, y, z);
        mac->SetPosition(devicePosition);
        
        // Set gateway reference
        mac->SetGateway(gateway);
        
        // Setup MAB selector and other parameters
        mac->SetSelector(selector);
        mac->SetNumChannelsAndSpreadingFactors(nChannels, nSpreadingFactors);
        endDevices.Get(i)->AggregateObject(mac); // Attach MAC logic to node
        macDevices.push_back(mac);

        std::cout << "Device " << (i+1) << " positioned at (" << devicePosition.x << ", " 
                  << devicePosition.y << ", " << devicePosition.z << "), distance to gateway: " 
                  << std::sqrt(std::pow(devicePosition.x - gatewayPosition.x, 2) + 
                               std::pow(devicePosition.y - gatewayPosition.y, 2) + 
                               std::pow(devicePosition.z - gatewayPosition.z, 2)) 
                  << "m" << std::endl;

        // Start packet generation with some random offset
        Time startTime = Seconds(rng->GetValue(0.1, packetIntervalSeconds));
        Simulator::Schedule(startTime, &MinimalLorawanEndDeviceMac::StartSending, mac, Seconds(packetIntervalSeconds));
    }

    std::cout << "Starting simulation for " << simTimeSeconds << " seconds..." << std::endl;
    
    // Run simulation
    Simulator::Stop(Seconds(simTimeSeconds));
    Simulator::Run();
    Simulator::Destroy();

    // Print results
    std::cout << "Simulation finished." << std::endl;
    std::cout << std::endl << "--- Simulation Results ---" << std::endl;
    
    uint64_t totalTx = 0;
    uint64_t totalSuccess = 0;
    double totalEnergy = 0.0;
    
    // Ouvrir le fichier de caractérisation réseau
    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        std::cerr << "Erreur: Impossible d'ouvrir le fichier " << outputFile << std::endl;
        return 1;
    }
    
    // Écrire l'en-tête du fichier
    outFile << "# Caractérisation du réseau LoRaWAN avec ToW-MAB - Environnement Rural" << std::endl;
    outFile << "# Date: " << std::time(nullptr) << std::endl;
    outFile << "# Nombre de dispositifs: " << nEndDevices << std::endl;
    outFile << "# Durée de simulation: " << simTimeSeconds << " secondes" << std::endl;
    outFile << "# Position de la gateway: (" << gatewayPosition.x << ", " 
            << gatewayPosition.y << ", " << gatewayPosition.z << ")" << std::endl;
    outFile << "#" << std::endl;
    outFile << "# Format: DeviceID, PositionX, PositionY, PositionZ, Distance(m), TxPackets, RxPackets, SuccessRate(%), AvgRSSI(dBm), AvgSNR(dB), EnergyConsumed(J)" << std::endl;
    outFile << "#-----------------------------------------------------------" << std::endl;
    
    // Collecter et écrire les statistiques pour chaque dispositif
    for (int i = 0; i < nEndDevices; ++i) {
        auto mac = macDevices[i];
        uint64_t txPackets = mac->GetTotalTxPackets();
        uint64_t rxPackets = mac->GetSuccessfulTxPackets();
        double successRate = txPackets > 0 ? (double)rxPackets / txPackets * 100.0 : 0.0;
        double energyConsumed = mac->GetEnergyConsumed();
        
        Vector pos = mac->GetPosition();
        double distance = std::sqrt(std::pow(pos.x - gatewayPosition.x, 2) + 
                                  std::pow(pos.y - gatewayPosition.y, 2) + 
                                  std::pow(pos.z - gatewayPosition.z, 2));
        
        // Get link stats from gateway
        auto stats = gateway->GetLinkStats(mac->GetDeviceId());
        
        // Écrire les statistiques dans le fichier
        outFile << mac->GetDeviceId() << ", " 
                << pos.x << ", " << pos.y << ", " << pos.z << ", " 
                << distance << ", " 
                << txPackets << ", " << rxPackets << ", " 
                << successRate << ", " 
                << stats.lastRssi << ", " << stats.lastSnr << ", " 
                << energyConsumed << std::endl;
        
        // Afficher les statistiques à l'écran
        std::cout << "Device " << mac->GetDeviceId() << ": " 
                  << txPackets << " transmissions, " 
                  << rxPackets << " reçus, " 
                  << successRate << "% succès, " 
                  << "énergie: " << energyConsumed << " J" << std::endl;
        
        // Accumuler les totaux
        totalTx += txPackets;
        totalSuccess += rxPackets;
        totalEnergy += energyConsumed;
    }
    
    // Statistiques globales
    double overallSuccessRate = totalTx > 0 ? (double)totalSuccess / totalTx * 100.0 : 0.0;
    double energyEfficiency = totalEnergy > 0 ? (double)totalSuccess / totalEnergy : 0.0;
    
    // Écrire les statistiques globales
    outFile << "#-----------------------------------------------------------" << std::endl;
    outFile << "# Statistiques globales:" << std::endl;
    outFile << "# Total des transmissions: " << totalTx << std::endl;
    outFile << "# Total des réceptions: " << totalSuccess << std::endl;
    outFile << "# Taux de succès global: " << overallSuccessRate << "%" << std::endl;
    outFile << "# Énergie totale consommée: " << totalEnergy << " J" << std::endl;
    outFile << "# Efficacité énergétique: " << energyEfficiency << " paquets/J" << std::endl;
    
    outFile.close();
    
    // Afficher les statistiques globales à l'écran
    std::cout << std::endl << "--- Statistiques globales ---" << std::endl;
    std::cout << "Total des transmissions: " << totalTx << std::endl;
    std::cout << "Total des réceptions: " << totalSuccess << std::endl;
    std::cout << "Taux de succès global: " << overallSuccessRate << "%" << std::endl;
    std::cout << "Énergie totale consommée: " << totalEnergy << " J" << std::endl;
    std::cout << "Efficacité énergétique: " << energyEfficiency << " paquets/J" << std::endl;
    std::cout << "Fichier de caractérisation créé: " << outputFile << std::endl;

    return 0;
}
