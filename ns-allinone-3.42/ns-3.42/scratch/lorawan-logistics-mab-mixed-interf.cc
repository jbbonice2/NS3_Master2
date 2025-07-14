/*
 * Simulation LoRaWAN logistique mixte (statique + mobile) avec interférences
 * Interférences : pluie, vent, bâtiments, arbres, réseaux voisins
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/random-variable-stream.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <cmath>
#include <map>
#include <sys/stat.h>
#include <sys/types.h>

using namespace ns3;

struct ReceptionLog {
    uint32_t deviceId;
    double time;
    double x, y, z;
    double distance;
    double txPower;
    double rssi;
    double snr;
    bool success;
    double energyConsumed;
    double interferenceLoss;
};

class LogisticsGatewayInterf : public Object {
public:
    static TypeId GetTypeId(void) {
        static TypeId tid = TypeId("LogisticsGatewayInterf")
            .SetParent<Object>()
            .AddConstructor<LogisticsGatewayInterf>();
        return tid;
    }
    LogisticsGatewayInterf() {
        m_position = Vector(0,0,20);
        // Table de sensibilité LoRa (BW 125kHz)
        m_sfSensitivity[7] = -123.0;
        m_sfSensitivity[8] = -126.0;
        m_sfSensitivity[9] = -129.0;
        m_sfSensitivity[10] = -132.0;
        m_sfSensitivity[11] = -133.0;
        m_sfSensitivity[12] = -136.0;
        m_rng = CreateObject<UniformRandomVariable>();
    }
    void SetPosition(Vector pos) { m_position = pos; }
    Vector GetPosition() const { return m_position; }
    // Ajout d'interférences, txPowerDbm: puissance d'émission, sf: spreading factor, bw: bandwidth
    bool ReceivePacket(uint32_t deviceId, const Vector& devicePosition, double txPowerDbm, int sf, int bw, double& rssi, double& snr, double& interfLoss) {
        double distance = CalculateDistance(devicePosition, m_position);
        // Path loss rural plus doux
        double pathLoss = 120.0 + 30.0 * std::log10(std::max(distance, 1.0) / 1000.0);
        // Modélisation simple des interférences
        double rainLoss = m_rng->GetValue(1.0, 3.0); // pluie (dB)
        double windLoss = m_rng->GetValue(0.5, 2.0); // vent (dB)
        double buildingLoss = m_rng->GetValue(5.0, 15.0); // bâtiments (dB)
        double treeLoss = m_rng->GetValue(2.0, 8.0); // arbres (dB)
        double neighborNetLoss = m_rng->GetValue(1.0, 5.0); // réseaux voisins (dB)
        interfLoss = rainLoss + windLoss + buildingLoss + treeLoss + neighborNetLoss;
        rssi = txPowerDbm - pathLoss - interfLoss;
        // SNR réaliste LoRa
        snr = m_rng->GetValue(-20.0, 10.0) - interfLoss/10.0;
        // Sensibilité dépend du SF (BW 125kHz)
        double sensitivity = m_sfSensitivity.count(sf) ? m_sfSensitivity[sf] : -130.0;
        // Critère de succès assoupli : SNR > -20 dB
        return (rssi > sensitivity) && (snr > -20.0);
    }
private:
    Vector m_position;
    std::map<int, double> m_sfSensitivity;
    Ptr<UniformRandomVariable> m_rng;
    double CalculateDistance(const Vector& a, const Vector& b) const {
        return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2) + std::pow(a.z - b.z, 2));
    }
};

class LogisticsEndDeviceInterf : public Object {
public:
    static TypeId GetTypeId(void) {
        static TypeId tid = TypeId("LogisticsEndDeviceInterf")
            .SetParent<Object>()
            .AddConstructor<LogisticsEndDeviceInterf>();
        return tid;
    }
    LogisticsEndDeviceInterf() : m_deviceId(0), m_txPower(14.0), m_energyConsumed(0.0), m_totalTx(0), m_totalRx(0) {
        m_rng = CreateObject<UniformRandomVariable>();
    }
    void Setup(uint32_t id, Ptr<LogisticsGatewayInterf> gw, Ptr<MobilityModel> mob, double txPower) {
        m_deviceId = id;
        m_gateway = gw;
        m_mobility = mob;
        m_txPower = txPower;
    }
    void SetLoRaParams(int sf, int bw, int cr, int payload) {
        m_sf = sf;
        m_bw = bw;
        m_cr = cr;
        m_payload = payload;
    }
    void StartSending(Time interval, double simTime, double startOffset = 0.0) {
        m_interval = interval;
        m_simTime = simTime;
        Simulator::Schedule(Seconds(startOffset), &LogisticsEndDeviceInterf::SendPacket, this);
    }
    void SendPacket() {
        double now = Simulator::Now().GetSeconds();
        if (now > m_simTime) return;
        Vector pos = m_mobility ? m_mobility->GetPosition() : Vector(0,0,0);
        double rssi = 0.0, snr = 0.0, interfLoss = 0.0;
        // Appel ReceivePacket avec SF/BW pour sensibilité réaliste
        bool success = m_gateway->ReceivePacket(m_deviceId, pos, m_txPower, m_sf, m_bw, rssi, snr, interfLoss);
        double distance = std::sqrt(pos.x*pos.x + pos.y*pos.y + pos.z*pos.z);
        // Calcul timeOnAir LoRa
        double tSym = std::pow(2, m_sf) / (double)m_bw * 1000.0;
        double payloadSymbNb = 8 + std::max(
            (int)std::ceil((8.0 * m_payload - 4.0 * m_sf + 28 + 16 - 20 * 0) /
            (4.0 * (m_sf - 2 * 0)) * (m_cr + 4)), 0);
        double tPreamble = (8 + 4.25) * tSym;
        double tPayload = payloadSymbNb * tSym;
        double timeOnAir = tPreamble + tPayload; // ms
        double txPowerMw = std::pow(10, m_txPower / 10.0);
        double energy = (txPowerMw / 1000.0) * (timeOnAir / 1000.0);
        m_energyConsumed += energy;
        ReceptionLog log = {m_deviceId, now, pos.x, pos.y, pos.z, distance, m_txPower, rssi, snr, success, m_energyConsumed, interfLoss};
        m_trace.push_back(log);
        m_totalTx++;
        if (success) m_totalRx++;
        Simulator::Schedule(m_interval, &LogisticsEndDeviceInterf::SendPacket, this);
    }
    std::vector<ReceptionLog> GetTrace() const { return m_trace; }
    uint32_t GetDeviceId() const { return m_deviceId; }
    uint64_t GetTotalTx() const { return m_totalTx; }
    uint64_t GetTotalRx() const { return m_totalRx; }
private:
    uint32_t m_deviceId;
    double m_txPower;
    double m_energyConsumed;
    Ptr<LogisticsGatewayInterf> m_gateway;
    Ptr<MobilityModel> m_mobility;
    std::vector<ReceptionLog> m_trace;
    uint64_t m_totalTx;
    uint64_t m_totalRx;
    Ptr<UniformRandomVariable> m_rng;
    Time m_interval;
    double m_simTime;
    int m_sf = 7;
    int m_bw = 125000;
    int m_cr = 1;
    int m_payload = 30;
    double m_timeOnAir;
};

int main(int argc, char *argv[]) {
    std::cout << "[SIMU] Début simulation LoRaWAN logistique mixte avec interférences" << std::endl;
    // Création du dossier de résultats si besoin
    std::string resultsDir = "lorawan_mixed_results_interf";
    struct stat st = {0};
    if (stat(resultsDir.c_str(), &st) == -1) {
        mkdir(resultsDir.c_str(), 0700);
    }
    // Définition des listes de paramètres
    std::vector<int> sfList = {7,8,9,10,11}; // 5 valeurs
    std::vector<int> txPowerList = {2,8};    // 2 valeurs
    std::vector<int> payloadList = {50,100,150,200,250}; // 5 valeurs
    std::vector<int> bwList = {125000, 250000}; // 2 valeurs
    int cr = 1;
    
    // Génération de toutes les combinaisons possibles
    struct ParamCombo {
        int sf;
        int txPower;
        int payload;
        int bw;
    };
    
    std::vector<ParamCombo> paramCombinations;
    for(int sf : sfList) {
        for(int txPower : txPowerList) {
            for(int payload : payloadList) {
                for(int bw : bwList) {
                    paramCombinations.push_back({sf, txPower, payload, bw});
                }
            }
        }
    }
    
    std::cout << "[SIMU] Nombre total de combinaisons: " << paramCombinations.size() << std::endl;
    
    uint32_t nDevices = 1000;
    double mobileRatio = 0.5;
    double packetIntervalSeconds = 15.0;
    uint32_t nMessages = 20;
    double simTime = nMessages * packetIntervalSeconds;
    
    // Attribution des combinaisons aux dispositifs
    for (uint32_t comboIdx = 0; comboIdx < paramCombinations.size(); ++comboIdx) {
        ParamCombo combo = paramCombinations[comboIdx];
        std::cout << "[SIMU] Configuration " << (comboIdx + 1) << "/" << paramCombinations.size() 
                  << " - SF=" << combo.sf << ", TxPower=" << combo.txPower 
                  << "dBm, Payload=" << combo.payload << " octets, BW=" << combo.bw << "Hz" << std::endl;
        
        // Calcul du nombre de dispositifs pour cette combinaison
        uint32_t devicesPerCombo = nDevices / paramCombinations.size();
        uint32_t startDeviceId = comboIdx * devicesPerCombo;
        uint32_t endDeviceId = (comboIdx == paramCombinations.size() - 1) ? 
                               nDevices : startDeviceId + devicesPerCombo;
        
        double areaRadius = 1000.0;
        Ptr<LogisticsGatewayInterf> gateway = CreateObject<LogisticsGatewayInterf>();
        gateway->SetPosition(Vector(0, 0, 20));
        
        NodeContainer nodes;
        nodes.Create(endDeviceId - startDeviceId);
        
        MobilityHelper mobility;
        Ptr<RandomDiscPositionAllocator> positionAlloc = CreateObject<RandomDiscPositionAllocator>();
        positionAlloc->SetX(0.0);
        positionAlloc->SetY(0.0);
        positionAlloc->SetRho(CreateObjectWithAttributes<UniformRandomVariable>("Min", DoubleValue(0.0), "Max", DoubleValue(areaRadius)));
        mobility.SetPositionAllocator(positionAlloc);
        
        for (uint32_t i = 0; i < (endDeviceId - startDeviceId); ++i) {
            if (i < (endDeviceId - startDeviceId) * mobileRatio) {
                mobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                    "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"),
                    "PositionAllocator", PointerValue(positionAlloc));
            } else {
                mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
            }
            mobility.Install(nodes.Get(i));
        }
        
        std::vector<Ptr<LogisticsEndDeviceInterf>> devices;
        Ptr<UniformRandomVariable> offsetRng = CreateObject<UniformRandomVariable>();
        offsetRng->SetAttribute("Min", DoubleValue(0.0));
        offsetRng->SetAttribute("Max", DoubleValue(packetIntervalSeconds));
        
        for (uint32_t i = 0; i < (endDeviceId - startDeviceId); ++i) {
            Ptr<LogisticsEndDeviceInterf> dev = CreateObject<LogisticsEndDeviceInterf>();
            Ptr<MobilityModel> mob = nodes.Get(i)->GetObject<MobilityModel>();
            dev->Setup(startDeviceId + i + 1, gateway, mob, (double)combo.txPower);
            dev->SetLoRaParams(combo.sf, combo.bw, cr, combo.payload);
            double offset = offsetRng->GetValue();
            dev->StartSending(Seconds(packetIntervalSeconds), simTime, offset);
            devices.push_back(dev);
        }
        
        Simulator::Stop(Seconds(simTime));
        Simulator::Run();
        
        std::cout << "[SIMU] Simulation terminée pour la configuration " << (comboIdx + 1) << std::endl;
        
        std::ostringstream fname;
        fname << resultsDir << "/lorawan-logistics-mab-mixed_ALL.csv";
        std::ofstream out;
        bool writeHeader = false;
        if (!std::ifstream(fname.str())) {
            out.open(fname.str(), std::ios::out);
            writeHeader = true;
        } else {
            out.open(fname.str(), std::ios::app);
        }
        if (writeHeader) {
            out << "deviceId,messageId,time,x,y,z,distance,txPower,sf,bw,cr,payload,nDevices,rssi,snr,success,energyConsumed,timeOnAir,totalTx,totalRx,interferenceLoss\n";
        }
        
        int globalMessageId = 1;
        for (auto dev : devices) {
            auto trace = dev->GetTrace();
            uint64_t totalTx = dev->GetTotalTx();
            uint64_t totalRx = dev->GetTotalRx();
            for (const auto& log : trace) {
                double tSym = std::pow(2, combo.sf) / (double)combo.bw * 1000.0;
                double payloadSymbNb = 8 + std::max(
                    (int)std::ceil((8.0 * combo.payload - 4.0 * combo.sf + 28 + 16 - 20 * 0) /
                    (4.0 * (combo.sf - 2 * 0)) * (cr + 4)), 0);
                double tPreamble = (8 + 4.25) * tSym;
                double tPayload = payloadSymbNb * tSym;
                double timeOnAir = tPreamble + tPayload; // ms
                
                // Conversion time en date/heure
                std::time_t base = 1752177514; // 2025-07-10 18:18:34
                std::time_t t = base + static_cast<std::time_t>(log.time);
                char buffer[20];
                std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
                
                out << log.deviceId << "," << globalMessageId << "," << buffer << "," 
                    << log.x << "," << log.y << "," << log.z << "," << log.distance << "," 
                    << log.txPower << "," << combo.sf << "," << combo.bw << "," << cr << "," 
                    << combo.payload << "," << nDevices << "," << log.rssi << "," << log.snr << "," 
                    << log.success << "," << log.energyConsumed << "," << timeOnAir << "," 
                    << totalTx << "," << totalRx << "," << log.interferenceLoss << "\n";
                globalMessageId++;
            }
        }
        out.close();
        Simulator::Destroy();
    }
    std::cout << "[SIMU] Fin simulation LoRaWAN logistique mixte avec interférences" << std::endl;
    return 0;
}
