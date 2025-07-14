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
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>
#include <map>
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

// Classe Gateway LoRaWAN simplifiée pour environnement urbain
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
        
        // Calculer la perte de chemin (modèle simplifié pour environnement urbain)
        // Version simplifiée de Okumura-Hata pour 868 MHz en milieu urbain:
        double pathLoss = 120.0 + 35.0 * std::log10(distance / 1000.0); // Environnement urbain (plus d'atténuation)
        
        // Calculer le RSSI
        rssi = txPowerDbm - pathLoss;
        
        // Calculer le SNR (valeurs typiques entre -20 et +10 dB pour LoRa)
        // En milieu urbain, le SNR est généralement plus faible qu'en milieu rural
        snr = m_rng->GetValue(-15.0, 8.0); // Valeurs plus faibles pour milieu urbain
        
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
        // Initialisation des paramètres LoRaWAN par défaut - Environnement Urbain
        m_params.txPower = 14.0;      // 14 dBm (25 mW)
        m_params.bandwidth = 125;      // 125 kHz
        m_params.codingRate = 5;       // 4/5
        m_params.payloadSize = 20;     // 20 bytes
        m_params.range = 2000.0;       // 2 km (urbain)
        
        // Initialiser le générateur de nombres aléatoires
        m_rng = CreateObject<UniformRandomVariable>();
        
        // Initialiser la position du dispositif
        m_position = Vector(0, 0, 0);
    }
    virtual ~MinimalLorawanEndDeviceMac() {}

    void SetSelector(Ptr<TowMabSelector> selector) {
        m_selector = selector;
    }
    
    void SetDeviceId(uint32_t id) {
        m_deviceId = id;
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
        // Planifier la prochaine transmission
        m_sendEvent = Simulator::Schedule(m_interval, &MinimalLorawanEndDeviceMac::SendPacket, this);

        if (!m_selector) {
            std::cerr << "Erreur: Aucun sélecteur défini pour ce MAC" << std::endl;
            return;
        }
        
        if (!m_gateway) {
            std::cerr << "Erreur: Aucune gateway définie pour ce MAC" << std::endl;
            return;
        }

        // Utiliser le sélecteur ToW-MAB pour choisir le canal et le facteur d'étalement
        std::pair<uint32_t, uint8_t> selection = m_selector->SelectChannelAndSpreadingFactor();
        uint32_t channelIndex = selection.first;
        uint8_t spreadingFactorIndex = selection.second;

        // Obtenir les valeurs réelles
        double frequency = GetChannelFrequency(channelIndex);
        uint8_t realSF = GetRealSpreadingFactor(spreadingFactorIndex);
        
        // Calculer la distance à la gateway
        double distance = CalculateDistance(m_position, m_gateway->GetPosition());
        
        // Afficher les paramètres de communication
        std::cout << Simulator::Now().GetSeconds() << "s: ED" << m_deviceId << " transmission avec paramètres:" << std::endl;
        std::cout << "  Position: (" << m_position.x << "," << m_position.y << "," << m_position.z << ")" << std::endl;
        std::cout << "  Distance à la gateway: " << distance << " m" << std::endl;
        std::cout << "  Canal: " << channelIndex << " (" << frequency << " MHz)" << std::endl;
        std::cout << "  SF: " << (unsigned)spreadingFactorIndex << " (SF" << (unsigned)realSF << ")" << std::endl;
        std::cout << "  Puissance Tx: " << m_params.txPower << " dBm" << std::endl;
        std::cout << "  Bande passante: " << m_params.bandwidth << " kHz" << std::endl;
        std::cout << "  Coding Rate: 4/" << (unsigned)m_params.codingRate << std::endl;
        std::cout << "  Taille de payload: " << m_params.payloadSize << " bytes" << std::endl;
        
        m_totalTxPackets++;
        
        // Envoyer le paquet à la gateway et récupérer RSSI et SNR
        double rssi = 0.0;
        double snr = 0.0;
        bool success = m_gateway->ReceivePacket(m_deviceId, m_position, m_params.txPower, 
                                              realSF, m_params.bandwidth, m_params.codingRate, 
                                              rssi, snr);
        
        // Calculer le temps de transmission (Time on Air) en ms
        double timeOnAir = CalculateTimeOnAir(m_params.payloadSize, realSF, m_params.bandwidth, m_params.codingRate);
        
        // Calculer l'énergie consommée pour cette transmission (en Joules)
        // Conversion de la puissance Tx de dBm à mW: P(mW) = 10^(P(dBm)/10)
        double txPowerMw = std::pow(10, m_params.txPower / 10.0);
        // Énergie = Puissance (W) * Temps (s) : on divise par 1000 pour convertir mW en W et ms en s
        double energy = (txPowerMw / 1000.0) * (timeOnAir / 1000.0);
        
        // Ajouter l'énergie consommée au total
        m_energyConsumed += energy;
        
        // Enregistrer les statistiques de la dernière transmission
        m_lastTxStats.txPower = m_params.txPower;
        m_lastTxStats.sf = realSF;
        m_lastTxStats.bw = m_params.bandwidth;
        m_lastTxStats.distance = distance;
        m_lastTxStats.rssi = rssi;
        m_lastTxStats.snr = snr;
        m_lastTxStats.success = success;
        m_lastTxStats.energyConsumed = energy;

        if (success) {
            m_successfulTxPackets++;
        }
        
        std::cout << "  RSSI: " << rssi << " dBm" << std::endl;
        std::cout << "  SNR: " << snr << " dB" << std::endl;
        std::cout << "  Time on Air: " << timeOnAir << " ms" << std::endl;
        std::cout << "  Énergie: " << energy << " J" << std::endl;
        std::cout << "  -> Résultat: " << (success ? "Succès" : "Échec") << std::endl;
        std::cout << "  --------------------------" << std::endl;

        // Mettre à jour le sélecteur avec le résultat
        m_selector->Update(channelIndex, spreadingFactorIndex, success);
    }
    
    // Calculer le temps de transmission (Time on Air) en millisecondes
    double CalculateTimeOnAir(uint16_t payloadSize, uint8_t sf, uint32_t bw, uint8_t cr) {
        // Nombre de symboles dans le préambule
        int nPreamble = 8;
        
        // Taille du header (1 pour explicite header mode)
        int explicitHeader = 1;
        
        // Facteur de démodulation pour différentes bandes passantes
        double bwFactor = 0.0;
        if (bw == 125) bwFactor = 1.0;
        else if (bw == 250) bwFactor = 0.5;
        else if (bw == 500) bwFactor = 0.25;
        else bwFactor = 1.0; // valeur par défaut
        
        // Nombre de bits par symbole
        double bitsPerSymbol = sf * 1.0;
        
        // Calcul du nombre de symboles
        int payloadSymbNb = 8 + std::max(std::ceil((8.0 * payloadSize - 4.0 * sf + 28.0 + 16.0 - 20.0 * explicitHeader) / (4.0 * sf)) * cr, 0.0);
        
        // Durée d'un symbole en ms
        double Tsym = std::pow(2, sf) / (bw * 1.0) * 1000;
        
        // Temps de transmission total en ms
        double Tpreamble = (nPreamble + 4.25) * Tsym;
        double Tpayload = payloadSymbNb * Tsym;
        
        return Tpreamble + Tpayload;
    }
    
    // Calculer la distance euclidienne entre deux points
    double CalculateDistance(const Vector& a, const Vector& b) const {
        return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2) + std::pow(a.z - b.z, 2));
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
    
    // Structure pour stocker les statistiques de la dernière transmission
    struct LastTxStats {
        double txPower;
        uint8_t sf;
        uint32_t bw;
        double distance;
        double rssi;
        double snr;
        bool success;
        double energyConsumed;
    };
    
    // Récupérer les statistiques de la dernière transmission
    LastTxStats GetLastTxStats() const {
        return m_lastTxStats;
    }
    
    // Génère les statistiques détaillées pour le fichier de caractérisation
    LinkStats GetDeviceStats() const {
        LinkStats stats;
        stats.deviceId = m_deviceId;
        stats.position = m_position;
        stats.distance = m_gateway ? CalculateDistance(m_position, m_gateway->GetPosition()) : 0.0;
        stats.txPackets = m_totalTxPackets;
        stats.rxPackets = m_successfulTxPackets;
        stats.lastRssi = m_lastTxStats.rssi;
        stats.lastSnr = m_lastTxStats.snr;
        stats.sf = m_lastTxStats.sf;
        stats.bw = m_lastTxStats.bw;
        stats.cr = m_params.codingRate;
        stats.txPower = m_lastTxStats.txPower;
        stats.energyConsumed = m_energyConsumed;
        return stats;
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

    Ptr<TowMabSelector> m_selector;                   // Sélecteur ToW-MAB
    Ptr<MinimalLorawanGateway> m_gateway;              // Référence à la gateway
    Vector m_position;                                 // Position 3D du dispositif
    uint64_t m_totalTxPackets;                         // Nombre total de paquets transmis
    uint64_t m_successfulTxPackets;                    // Nombre de paquets transmis avec succès
    uint32_t m_deviceId;                               // ID du dispositif
    LoRaWANParams m_params;                            // Paramètres LoRaWAN
    double m_energyConsumed;                           // Énergie totale consommée (Joules)
    LastTxStats m_lastTxStats;                         // Statistiques de la dernière transmission
    Ptr<UniformRandomVariable> m_rng;                  // Générateur de nombres aléatoires
    EventId m_sendEvent;                               // Événement de transmission planifié
    Time m_interval;                                   // Intervalle entre les transmissions
};

NS_OBJECT_ENSURE_REGISTERED(MinimalLorawanEndDeviceMac);

} // namespace lorawan
} // namespace ns3

using namespace ns3;
using namespace lorawan;

int main(int argc, char *argv[]) {
    // Activer les logs
    LogComponentEnable("lorawan", LOG_LEVEL_INFO);
    
    // Paramètres de ligne de commande
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Paramètres de simulation
    int nEndDevices = 15;           // Nombre de dispositifs LoRaWAN
    double simTimeSeconds = 600.0;  // Durée de la simulation (10 minutes)
    double packetIntervalSeconds = 60.0; // Intervalle entre les paquets (1 minute)
    uint32_t nChannels = 8;         // 8 canaux dans la bande EU868
    uint8_t nSpreadingFactors = 6;  // SF7 à SF12
    
    // Générateur aléatoire pour positionner les dispositifs
    Ptr<UniformRandomVariable> posRng = CreateObject<UniformRandomVariable>();
    
    // Création des nœuds (1 gateway + N dispositifs)
    NodeContainer gatewayNodes;
    gatewayNodes.Create(1); // Une seule gateway
    
    NodeContainer endDeviceNodes;
    endDeviceNodes.Create(nEndDevices);

    std::cout << "Création d'un réseau LoRaWAN urbain avec 1 gateway et " << nEndDevices << " dispositifs." << std::endl;
    
    // Créer et configurer la gateway
    Ptr<MinimalLorawanGateway> gateway = CreateObject<MinimalLorawanGateway>();
    
    // Positionner la gateway au centre (position légèrement surélevée pour milieu urbain)
    Vector gatewayPosition(0.0, 0.0, 20.0); // Hauteur typique pour une gateway urbaine
    gateway->SetPosition(gatewayPosition);
    gatewayNodes.Get(0)->AggregateObject(gateway);
    
    std::cout << "Gateway positionnée à " << gatewayPosition.x << ", " << gatewayPosition.y << ", " << gatewayPosition.z << " m" << std::endl;

    // Créer et installer les MACs et sélecteurs pour chaque dispositif
    std::vector<Ptr<MinimalLorawanEndDeviceMac>> macDevices;
    
    for (int i = 0; i < nEndDevices; ++i) {
        Ptr<MinimalLorawanEndDeviceMac> mac = CreateObject<MinimalLorawanEndDeviceMac>();
        Ptr<TowMabSelector> selector = CreateObject<TowMabSelector>();
        
        // Configurer les attributs du sélecteur ToW-MAB
        selector->SetAttribute("Alpha", DoubleValue(0.9));
        selector->SetAttribute("Beta", DoubleValue(0.99));
        selector->SetAttribute("OscAmplitude", DoubleValue(0.1));
        selector->SetAttribute("MaxRetransmissions", UintegerValue(3));
        selector->SetAttribute("TransmissionInterval", DoubleValue(60.0));
        
        // Définir l'ID du dispositif
        mac->SetDeviceId(i + 1);
        
        // Association à la gateway
        mac->SetGateway(gateway);
        
        // Positionner aléatoirement le dispositif dans un cercle autour de la gateway
        // Distance entre 100m et 3km (plus courte que dans le cas rural)
        double distance = 100.0 + posRng->GetValue(0.0, 2900.0); // 100m à 3km
        double angle = posRng->GetValue(0.0, 2.0 * M_PI);
        double x = distance * std::cos(angle);
        double y = distance * std::sin(angle);
        // Hauteur variable pour simuler différents étages d'immeubles (0 à 30m)
        double z = posRng->GetValue(0.0, 30.0);
        Vector devicePosition(x, y, z);
        mac->SetPosition(devicePosition);
        
        // Configurer les paramètres LoRaWAN - environnement urbain
        // Puissance variable entre 14 et 20 dBm selon la distance à la gateway
        double txPower = 14.0 + std::min(6.0, distance / 500.0);
        mac->SetTxPower(txPower);
        mac->SetBandwidth(125);      // 125 kHz (standard)
        mac->SetCodingRate(5);       // 4/5 (standard)
        mac->SetPayloadSize(20 + i % 11); // 20-30 bytes
        
        mac->SetSelector(selector);
        mac->SetNumChannelsAndSpreadingFactors(nChannels, nSpreadingFactors);
        endDeviceNodes.Get(i)->AggregateObject(mac);
        macDevices.push_back(mac);

        std::cout << "Dispositif " << (i + 1) << " positionné à " << devicePosition.x << ", " << devicePosition.y << ", " << devicePosition.z 
                 << " m (distance: " << distance << " m)" << std::endl;
        
        // Démarrer la génération de paquets avec un décalage aléatoire
        Time startTime = Seconds(posRng->GetValue(0.1, 10.0)); // Décalage entre 0.1 et 10 secondes
        Simulator::Schedule(startTime, &MinimalLorawanEndDeviceMac::StartSending, mac, Seconds(packetIntervalSeconds));
    }

    std::cout << "Démarrage de la simulation pour " << simTimeSeconds << " secondes..." << std::endl;
    
    // Exécuter la simulation
    Simulator::Stop(Seconds(simTimeSeconds));
    Simulator::Run();
    Simulator::Destroy();

    // Génération du fichier de caractérisation réseau
    std::string filename = "lorawan-tow-mab-urban-network-characterization.txt";
    std::ofstream outFile(filename);
    
    if (!outFile.is_open()) {
        std::cerr << "Erreur: Impossible d'ouvrir le fichier " << filename << std::endl;
        return 1;
    }
    
    // En-tête du fichier
    outFile << "# Caractérisation du réseau LoRaWAN ToW-MAB - Environnement Urbain" << std::endl;
    outFile << "# Simulation de " << simTimeSeconds << " secondes avec " << nEndDevices << " dispositifs et 1 gateway" << std::endl;
    outFile << "# Position de la gateway: (" << gatewayPosition.x << ", " << gatewayPosition.y << ", " << gatewayPosition.z << ") m" << std::endl;
    outFile << "#" << std::endl;
    outFile << "# Format: ID | Position(x,y,z) | Distance(m) | Paquets_Tx | Paquets_Rx | Succès(%) | RSSI(dBm) | SNR(dB) | SF | BW(kHz) | CR | Tx_Power(dBm) | Énergie(J)" << std::endl;
    outFile << "#------------------------------------------------------------------------------------------------------------------------------------------" << std::endl;
    
    // Initialisation des compteurs globaux
    uint64_t totalTx = 0;
    uint64_t totalRx = 0;
    double totalEnergy = 0.0;
    
    // Générer les statistiques pour chaque dispositif
    for (int i = 0; i < nEndDevices; ++i) {
        Ptr<MinimalLorawanEndDeviceMac> mac = macDevices[i];
        
        // Récupérer les statistiques du dispositif
        uint64_t txPackets = mac->GetTotalTxPackets();
        uint64_t rxPackets = mac->GetSuccessfulTxPackets();
        double successRate = (txPackets > 0) ? ((double)rxPackets / txPackets * 100.0) : 0.0;
        Vector position = mac->GetPosition();
        double distance = std::sqrt(std::pow(position.x - gatewayPosition.x, 2) +
                                  std::pow(position.y - gatewayPosition.y, 2) +
                                  std::pow(position.z - gatewayPosition.z, 2));
        double energy = mac->GetEnergyConsumed();
        
        // Obtenir les statistiques de la dernière transmission
        auto lastStats = mac->GetLastTxStats();
        
        // Écrire les statistiques dans le fichier
        outFile << (i + 1) << " | ("
                << position.x << "," << position.y << "," << position.z << ") | "
                << distance << " | "
                << txPackets << " | "
                << rxPackets << " | "
                << std::fixed << std::setprecision(2) << successRate << " | "
                << std::fixed << std::setprecision(1) << lastStats.rssi << " | "
                << std::fixed << std::setprecision(1) << lastStats.snr << " | "
                << (unsigned)lastStats.sf << " | "
                << lastStats.bw << " | "
                << "4/" << (unsigned)mac->GetDeviceStats().cr << " | "
                << std::fixed << std::setprecision(1) << lastStats.txPower << " | "
                << std::fixed << std::setprecision(6) << energy
                << std::endl;
                
        // Mettre à jour les compteurs globaux
        totalTx += txPackets;
        totalRx += rxPackets;
        totalEnergy += energy;
    }
    
    // Ajouter les statistiques globales
    double globalSuccessRate = (totalTx > 0) ? ((double)totalRx / totalTx * 100.0) : 0.0;
    double energyEfficiency = (totalEnergy > 0) ? ((double)totalRx / totalEnergy) : 0.0;
    
    outFile << "#------------------------------------------------------------------------------------------------------------------------------------------" << std::endl;
    outFile << "# STATISTIQUES GLOBALES" << std::endl;
    outFile << "# Total paquets transmis: " << totalTx << std::endl;
    outFile << "# Total paquets reçus: " << totalRx << std::endl;
    outFile << "# Taux de succès global: " << std::fixed << std::setprecision(2) << globalSuccessRate << "%" << std::endl;
    outFile << "# Énergie totale consommée: " << std::fixed << std::setprecision(6) << totalEnergy << " J" << std::endl;
    outFile << "# Efficacité énergétique: " << std::fixed << std::setprecision(4) << energyEfficiency << " paquets/J" << std::endl;
    
    outFile.close();
    
    // Afficher les résultats à l'écran
    std::cout << "\nSimulation terminée." << std::endl;
    std::cout << "Fichier de caractérisation généré: " << filename << std::endl;
    std::cout << "\n--- Résultats de Simulation ---" << std::endl;
    std::cout << "Paquets transmis: " << totalTx << std::endl;
    std::cout << "Paquets reçus: " << totalRx << std::endl;
    std::cout << "Taux de succès global: " << std::fixed << std::setprecision(2) << globalSuccessRate << "%" << std::endl;
    std::cout << "Énergie totale consommée: " << std::fixed << std::setprecision(6) << totalEnergy << " J" << std::endl;
    std::cout << "Efficacité énergétique: " << std::fixed << std::setprecision(4) << energyEfficiency << " paquets/J" << std::endl;
    
    return 0;
}
