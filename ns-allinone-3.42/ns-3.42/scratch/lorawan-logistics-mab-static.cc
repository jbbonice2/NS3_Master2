/*
 * Simulation LoRaWAN logistique statique (inspirée de lorawan-logistics-mab-mobile.cc)
 * Tous les end-devices sont statiques (position fixe aléatoire dans la zone)
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
    int sf;
    int bw;
    int payload;
    int cr;
    double rssi;
    double snr;
    bool success;
    double energyConsumed;
    double timeOnAir;
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
    struct Params { int sf, txPower, payload, bw; };
    static TypeId GetTypeId(void) {
        static TypeId tid = TypeId("LogisticsEndDevice")
            .SetParent<Object>()
            .AddConstructor<LogisticsEndDevice>();
        return tid;
    }
    LogisticsEndDevice() : m_deviceId(0), m_txPower(14.0), m_energyConsumed(0.0), m_totalTx(0), m_totalRx(0), m_paramIndex(0), m_channelIndex(0) {
        m_rng = CreateObject<UniformRandomVariable>();
    }
    void Setup(uint32_t id, Ptr<LogisticsGateway> gw, Ptr<MobilityModel> mob, double txPower) {
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
    void AddChannel(uint32_t freq) { m_channels.push_back(freq); }
    void SetParamCombos(const std::vector<Params>& combos) {
        m_paramCombos = combos;
        m_paramScores = std::vector<double>(combos.size(), 0.0); // Score initial 0
    }

    void StartSending(Time interval, double simTime, double startOffset = 0.0) {
        m_interval = interval;
        m_simTime = simTime;
        Simulator::Schedule(Seconds(startOffset), &LogisticsEndDevice::SendPacket, this);
    }
    void SendPacket() {
        // Sélection intelligente (epsilon-greedy)
        if (!m_paramCombos.empty()) {
            uint32_t idx = 0;
            double randVal = m_rng->GetValue(0.0, 1.0);
            if (randVal < m_epsilon) {
                // Exploration : choisir une combinaison au hasard
                idx = m_rng->GetInteger(0, m_paramCombos.size() - 1);
            } else {
                // Exploitation : choisir la meilleure combinaison
                idx = std::distance(m_paramScores.begin(), std::max_element(m_paramScores.begin(), m_paramScores.end()));
            }
            const Params& params = m_paramCombos[idx];
            m_sf = params.sf;
            m_txPower = params.txPower;
            m_payload = params.payload;
            m_bw = params.bw;
            m_lastParamUsed = idx;
        }
        // Sélection cyclique du canal EU868
        uint32_t freq = 0;
        if (!m_channels.empty()) {
            freq = m_channels[m_channelIndex % m_channels.size()];
            m_channelIndex++;
        }
        std::cout << "[PERM] Device " << m_deviceId
                  << " | TX#" << (m_totalTx+1)
                  << " | SF=" << m_sf
                  << " | TxPower=" << m_txPower << " dBm"
                  << " | Payload=" << m_payload << " octets"
                  << " | BW=" << m_bw << " Hz"
                  << " | Channel=" << freq << " Hz"
                  << std::endl;
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
        ReceptionLog log = {m_deviceId, now, pos.x, pos.y, pos.z, distance, m_txPower, m_sf, m_bw, m_payload, m_cr, rssi, snr, success, m_energyConsumed, timeOnAir};
        m_trace.push_back(log);
        m_totalTx++;
        if (success) m_totalRx++;
        // Mise à jour du score de la combinaison utilisée
        // Récompense simple : +1 si succès, -1 sinon
        if (!m_paramScores.empty()) {
            m_paramScores[m_lastParamUsed] += (success ? 1.0 : -1.0);
        }
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
    std::vector<uint32_t> m_channels;
    std::vector<Params> m_paramCombos;
    std::vector<double> m_paramScores; // Score pour chaque combinaison
    uint32_t m_paramIndex;
    uint32_t m_channelIndex;
    double m_epsilon = 0.1; // Taux d'exploration (epsilon-greedy)
    uint32_t m_lastParamUsed = 0; // Pour savoir quel index a été utilisé au dernier envoi


};

int main(int argc, char *argv[]) {
    std::cout << "[SIMU] Début simulation LoRaWAN logistique statique" << std::endl;
    // Paramètres dynamiques
    std::vector<int> sfList = {7,8,9,10,11}; // 5 valeurs
    std::vector<int> txPowerList = {2,8};    // 2 valeurs
    std::vector<int> payloadList = {50,100,150,200,250}; // 5 valeurs
    std::vector<int> bwList = {125000, 250000}; // 2 valeurs
    uint32_t nDevices = 1000;
    int cr = 1;
    double packetIntervalSeconds = 15.0;
    uint32_t nMessages = 20;
    double simTime = nMessages * packetIntervalSeconds;
    double areaRadius = 5000.0;
    std::string outDir = "lorawan_static_results";
    mkdir(outDir.c_str(), 0777);
    // Génération de toutes les combinaisons
std::vector<LogisticsEndDevice::Params> allCombos;
for (int sf : sfList)
    for (int tx : txPowerList)
        for (int pl : payloadList)
            for (int bw : bwList)
                allCombos.push_back({sf, tx, pl, bw});

std::vector<uint32_t> eu868Frequencies = {868100000, 868300000, 868500000, 867100000, 867300000, 867500000, 867700000, 867900000};

for (int configIdx = 0; configIdx < 1; ++configIdx) { // exécution unique, toutes les configs sont cyclées par chaque device
    Ptr<LogisticsGateway> gateway = CreateObject<LogisticsGateway>();
    gateway->SetPosition(Vector(0, 0, 20));
    NodeContainer nodes;
    nodes.Create(nDevices);
    MobilityHelper mobility;
    Ptr<RandomDiscPositionAllocator> positionAlloc = CreateObject<RandomDiscPositionAllocator>();
    positionAlloc->SetX(0.0);
    positionAlloc->SetY(0.0);
    positionAlloc->SetRho(CreateObjectWithAttributes<UniformRandomVariable>("Min", DoubleValue(0.0), "Max", DoubleValue(areaRadius)));
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);
    std::vector<Ptr<LogisticsEndDevice>> devices;
    Ptr<UniformRandomVariable> offsetRng = CreateObject<UniformRandomVariable>();
    offsetRng->SetAttribute("Min", DoubleValue(0.0));
    offsetRng->SetAttribute("Max", DoubleValue(packetIntervalSeconds));
    for (uint32_t i = 0; i < nDevices; ++i) {
        Ptr<LogisticsEndDevice> dev = CreateObject<LogisticsEndDevice>();
        Ptr<MobilityModel> mob = nodes.Get(i)->GetObject<MobilityModel>();
        dev->Setup(i + 1, gateway, mob, (double)txPowerList[0]); // txPower initial, sera cyclé
        dev->SetLoRaParams(sfList[0], bwList[0], cr, payloadList[0]); // params initiaux, seront cyclés
        dev->SetParamCombos(allCombos);
        for (auto freq : eu868Frequencies) dev->AddChannel(freq);
        double offset = offsetRng->GetValue();
        dev->StartSending(Seconds(packetIntervalSeconds), simTime, offset);
        devices.push_back(dev);
    }
    Simulator::Stop(Seconds(simTime));
    std::cout << "        [SIMU] Lancement de la simulation pour permutation cyclique de tous les paramètres..." << std::endl;
    Simulator::Run();
    std::cout << "        [SIMU] Simulation terminée pour permutation cyclique." << std::endl;
    std::ostringstream fname;
    fname << outDir << "/lorawan-static_ALL.csv";
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
        auto trace = devices[i]->GetTrace();
        uint64_t totalTx = devices[i]->GetTotalTx();
        uint64_t totalRx = devices[i]->GetTotalRx();
        for (uint32_t j = 0; j < trace.size(); ++j) {
            const auto& log = trace[j];
            // Conversion time en date/heure
std::time_t base = 1752177514; // 2025-07-10 18:18:34
std::time_t t = base + static_cast<std::time_t>(log.time);
char buffer[20];
std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
out << log.deviceId << "," << (j+1) << "," << buffer << "," << log.x << "," << log.y << "," << log.z << "," << log.distance << "," << log.txPower << "," << log.sf << "," << log.bw << "," << log.cr << "," << log.payload << "," << nDevices << "," << log.rssi << "," << log.snr << "," << log.success << "," << log.energyConsumed << "," << log.timeOnAir << "," << totalTx << "," << totalRx << "\n";
        }
    }
    out.close();
    Simulator::Destroy();
}
std::cout << "[SIMU] Fin de la simulation LoRaWAN logistique statique" << std::endl;
return 0;
}
                    