/*
 * Simulation LoRaWAN logistique mobile (modèle robuste, style EE-MAB urban mobile)
 * Tous les end-devices sont mobiles (RandomWaypointMobilityModel)
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
    uint32_t messageId;
    double time;
    double x, y, z;
    double distance;
    double txPower;
    int sf;
    int bw;
    int payload;
    int cr;
    double rssi;
    double snr;
    bool success;
    double energyConsumed;
    double timeOnAir;
    uint32_t gatewayId;
};

class LogisticsGateway : public Object {
public:
    static TypeId GetTypeId(void) {
        static TypeId tid = TypeId("LogisticsGateway")
            .SetParent<Object>()
            .AddConstructor<LogisticsGateway>();
        return tid;
    }
    LogisticsGateway() {
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
    // txPowerDbm: puissance d'émission, sf: spreading factor, bw: bandwidth
    bool ReceivePacket(uint32_t deviceId, const Vector& devicePosition, double txPowerDbm, int sf, int bw, double& rssi, double& snr) {
        double distance = CalculateDistance(devicePosition, m_position);
        // Path loss rural plus doux
        double pathLoss = 120.0 + 30.0 * std::log10(std::max(distance, 1.0) / 1000.0);
        rssi = txPowerDbm - pathLoss;
        // SNR réaliste LoRa
        snr = m_rng->GetValue(-20.0, 10.0);
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

class LogisticsEndDevice : public Object {
public:
    static TypeId GetTypeId(void) {
        static TypeId tid = TypeId("LogisticsEndDevice")
            .SetParent<Object>()
            .AddConstructor<LogisticsEndDevice>();
        return tid;
    }
    LogisticsEndDevice() : m_deviceId(0), m_txPower(14.0), m_energyConsumed(0.0), m_totalTx(0), m_totalRx(0) {
        m_rng = CreateObject<UniformRandomVariable>();
    }
    void Setup(uint32_t id, Ptr<LogisticsGateway> gw, Ptr<MobilityModel> mob, double txPower) {
        m_deviceId = id;
        m_gateway = gw;
        m_mobility = mob;
        m_txPower = txPower;
    }
    void StartSending(Time interval, double simTime, double startOffset = 0.0) {
        m_interval = interval;
        m_simTime = simTime;
        Simulator::Schedule(Seconds(startOffset), &LogisticsEndDevice::SendPacket, this);
    }
    // Ajout des paramètres LoRa pour chaque device
    void SetLoRaParams(int sf, int bw, int cr, int payload) {
        m_sf = sf;
        m_bw = bw;
        m_cr = cr;
        m_payload = payload;
    }
    void SendPacket() {
        double now = Simulator::Now().GetSeconds();
        if (now > m_simTime) return;
        Vector pos = m_mobility ? m_mobility->GetPosition() : Vector(0,0,0);
        double rssi = 0.0, snr = 0.0;
        // Appel ReceivePacket avec SF/BW pour sensibilité réaliste
        bool success = m_gateway->ReceivePacket(m_deviceId, pos, m_txPower, m_sf, m_bw, rssi, snr);
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
        ReceptionLog log = {m_deviceId, static_cast<uint32_t>(m_totalTx+1), now, pos.x, pos.y, pos.z, distance, m_txPower, m_sf, m_bw, m_payload, m_cr, rssi, snr, success, m_energyConsumed, timeOnAir, 1};
        m_trace.push_back(log);
        m_totalTx++;
        if (success) m_totalRx++;
        Simulator::Schedule(m_interval, &LogisticsEndDevice::SendPacket, this);
    }
    std::vector<ReceptionLog> GetTrace() const { return m_trace; }
    uint32_t GetDeviceId() const { return m_deviceId; }
    uint64_t GetTotalTx() const { return m_totalTx; }
    uint64_t GetTotalRx() const { return m_totalRx; }
private:
    uint32_t m_deviceId;
    double m_txPower;
    double m_energyConsumed;
    Ptr<LogisticsGateway> m_gateway;
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

// Fonction utilitaire pour calculer le timeOnAir LoRa (en ms)
double CalculateLoRaTimeOnAir(int sf, int pl, int cr, int bw, int preamble = 8, bool ih = false, bool de = false) {
    // Formule simplifiée LoRa (datasheet Semtech SX1276)
    double tSym = std::pow(2, sf) / (double)bw * 1000.0; // ms
    double payloadSymbNb = 8 + std::max(
        (int)std::ceil((8.0 * pl - 4.0 * sf + 28 + 16 - 20 * ih) /
        (4.0 * (sf - 2 * de)) * (cr + 4)), 0);
    double tPreamble = (preamble + 4.25) * tSym;
    double tPayload = payloadSymbNb * tSym;
    return tPreamble + tPayload;
}

int main(int argc, char *argv[]) {
    std::cout << "[SIMU] Début simulation LoRaWAN logistique mobile" << std::endl;
    // Création du dossier de résultats si besoin
    std::string resultsDir = "lorawan_mobile_results";
    struct stat st = {0};
    if (stat(resultsDir.c_str(), &st) == -1) {
        mkdir(resultsDir.c_str(), 0700);
    }
    // Paramètres dynamiques
    std::vector<int> sfList = {7,8,9,10,12}; // 5 valeurs
    std::vector<int> txPowerList = {2,8};    // 2 valeurs
    std::vector<int> payloadList = {50,100,150,200,250}; // 5 valeurs
    std::vector<int> bwList = {125000, 250000}; // 2 valeurs
    uint32_t nDevices = 50; // Correction ici : int -> uint32_t
    int cr = 1; // 4/5 => cr=1 (car CR=4/(4+cr))
    double packetIntervalSeconds = 15.0;
    uint32_t nMessages = 20;
    double simTime = nMessages * packetIntervalSeconds;
    double areaRadius = 3000.0;
    Ptr<LogisticsGateway> gateway = CreateObject<LogisticsGateway>();
    gateway->SetPosition(Vector(0, 0, 20));
    NodeContainer nodes;
    nodes.Create(nDevices);
    MobilityHelper mobility;
    Ptr<RandomDiscPositionAllocator> positionAlloc = CreateObject<RandomDiscPositionAllocator>();
    positionAlloc->SetX(0.0);
    positionAlloc->SetY(0.0);
    positionAlloc->SetRho(CreateObjectWithAttributes<UniformRandomVariable>("Min", DoubleValue(0.0), "Max", DoubleValue(areaRadius)));
    mobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
        "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"),
        "PositionAllocator", PointerValue(positionAlloc));
    mobility.Install(nodes);
    std::vector<Ptr<LogisticsEndDevice>> devices;
    Ptr<UniformRandomVariable> offsetRng = CreateObject<UniformRandomVariable>();
    offsetRng->SetAttribute("Min", DoubleValue(0.0));
    offsetRng->SetAttribute("Max", DoubleValue(packetIntervalSeconds));
    for (uint32_t i = 0; i < nDevices; ++i) {
        // Paramètres individuels affichés pour chaque device si besoin
    
        int sf = sfList[i % sfList.size()];
        int txPower = txPowerList[i % txPowerList.size()];
        int payload = payloadList[i % payloadList.size()];
        int bw = bwList[i % bwList.size()];
        Ptr<LogisticsEndDevice> dev = CreateObject<LogisticsEndDevice>();
        Ptr<MobilityModel> mob = nodes.Get(i)->GetObject<MobilityModel>();
        dev->Setup(i + 1, gateway, mob, (double)txPower);
        dev->SetLoRaParams(sf, bw, cr, payload);
        double offset = offsetRng->GetValue();
        dev->StartSending(Seconds(packetIntervalSeconds), simTime, offset);
        devices.push_back(dev);
    }
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    std::cout << "[SIMU] Simulation terminée." << std::endl;
    // --- Génération du fichier CSV ---
    std::ostringstream fname;
    fname << resultsDir << "/lorawan-logistics-mab-mobile_dynamic_ALL.csv";
    std::ofstream out;
    bool writeHeader = false;
    if (!std::ifstream(fname.str())) {
        out.open(fname.str(), std::ios::out);
        writeHeader = true;
    } else {
        out.open(fname.str(), std::ios::app);
    }
    if (writeHeader) {
        out << "deviceId,messageId,time,x,y,z,distance,txPower,sf,bw,cr,payload,nDevices,rssi,snr,success,energyConsumed,timeOnAir,totalTx,totalRx\n";
    }
    for (uint32_t i = 0; i < devices.size(); ++i) {
        // Affichage possible pour chaque device si besoin
    
        auto trace = devices[i]->GetTrace();
        uint64_t totalTx = devices[i]->GetTotalTx();
        uint64_t totalRx = devices[i]->GetTotalRx();
        for (const auto& log : trace) {
            // Conversion time en date/heure
std::time_t base = 1752177514; // 2025-07-10 18:18:34
std::time_t t = base + static_cast<std::time_t>(log.time);
char buffer[20];
std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
out << log.deviceId << "," << log.messageId << "," << buffer << "," << log.x << "," << log.y << "," << log.z << "," << log.distance << "," << log.txPower << "," << log.sf << "," << log.bw << "," << log.cr << "," << log.payload << "," << nDevices << "," << log.rssi << "," << log.snr << "," << log.success << "," << log.energyConsumed << "," << log.timeOnAir << "," << totalTx << "," << totalRx << "\n";
        }
    }
    out.close();
    Simulator::Destroy();
    std::cout << "[SIMU] Fin simulation LoRaWAN logistique mobile" << std::endl;
    return 0;
}
