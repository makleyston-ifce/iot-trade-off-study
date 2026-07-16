#include "CommunicationProfile.h"
#include "IoTGatewayApplication.h"
#include "IoTSensorApplication.h"
#include "Scenario.h"
#include "SecurityProfile.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"

#include <chrono>
#include <iomanip>

using namespace ns3;

static double
CalculateThroughput(uint64_t rxBytes, double durationSeconds)
{
  if (durationSeconds <= 0.0) {
    return 0.0;
  }
  return (static_cast<double>(rxBytes) * 8.0) / durationSeconds / 1000.0;
}

static double
CalculateAverageDelay(Time delaySum, uint64_t rxPackets)
{
  if (rxPackets == 0) {
    return 0.0;
  }
  return delaySum.GetSeconds() * 1000.0 / static_cast<double>(rxPackets);
}

static double
CalculateAverageJitter(Time jitterSum, uint64_t rxPackets)
{
  if (rxPackets <= 1) {
    return 0.0;
  }
  return jitterSum.GetSeconds() * 1000.0 / static_cast<double>(rxPackets - 1);
}

static void
PrintMetrics(uint64_t txPackets,
             uint64_t rxPackets,
             uint64_t lostPackets,
             uint64_t rxBytes,
             double throughputKbps,
             double averageDelayMs,
             double averageJitterMs,
             double cpuTimeSeconds)
{
  std::cout << "\n========== NETWORK METRICS ==========\n";
  std::cout << "TX Packets: " << txPackets << "\n";
  std::cout << "RX Packets: " << rxPackets << "\n";
  std::cout << "Lost Packets: " << lostPackets << "\n";
  std::cout << "RX Bytes: " << rxBytes << "\n";
  std::cout << "Throughput: " << std::fixed << std::setprecision(3) << throughputKbps << " kbps\n";
  std::cout << "Average Delay: " << std::fixed << std::setprecision(3) << averageDelayMs << " ms\n";
  std::cout << "Average Jitter: " << std::fixed << std::setprecision(3) << averageJitterMs << " ms\n";
  std::cout << "Simulation CPU Time: " << std::fixed << std::setprecision(3) << cpuTimeSeconds << " s\n";
}

int
main(int argc, char *argv[])
{
  ScenarioParameters params;
  std::string protocolString = "MQTT";
  std::string securityString = "NONE";
  CommandLine cmd;
  cmd.AddValue("nodes", "Number of sensor nodes", params.nodes);
  cmd.AddValue("duration", "Simulation duration in seconds", params.duration);
  cmd.AddValue("payload", "UDP payload size in bytes", params.payload);
  cmd.AddValue("protocol", "Communication protocol to model (MQTT, COAP, AMQP)", protocolString);
  cmd.AddValue("security", "Security protocol to model (NONE, TLS, MTLS)", securityString);
  cmd.Parse(argc, argv);

  CommunicationProtocol protocol = CommunicationProtocol::MQTT;
  if (protocolString == "MQTT") {
    protocol = CommunicationProtocol::MQTT;
  } else if (protocolString == "COAP") {
    protocol = CommunicationProtocol::COAP;
  } else if (protocolString == "AMQP") {
    protocol = CommunicationProtocol::AMQP;
  } else {
    std::cerr << "Invalid protocol: " << protocolString << "\n";
    std::cerr << "Valid values are: MQTT, COAP, AMQP\n";
    return 1;
  }

  SecurityProtocol securityProtocol = SecurityProtocol::NONE;
  if (securityString == "NONE") {
    securityProtocol = SecurityProtocol::NONE;
  } else if (securityString == "TLS") {
    securityProtocol = SecurityProtocol::TLS;
  } else if (securityString == "MTLS") {
    securityProtocol = SecurityProtocol::MTLS;
  } else {
    std::cerr << "Invalid security: " << securityString << "\n";
    std::cerr << "Valid values are: NONE, TLS, MTLS\n";
    return 1;
  }

  CommunicationProfile profile = GetCommunicationProfile(protocol);
  SecurityProfile securityProfile = GetSecurityProfile(securityProtocol);

  std::cout << "========== CONFIGURATION ==========\n";
  std::cout << "Protocol: " << CommunicationProtocolToString(profile.protocol) << "\n";
  std::cout << "Transport: " << (profile.usesTcp ? "TCP" : "UDP") << "\n";
  std::cout << "Security: " << SecurityProtocolToString(securityProfile.protocol) << "\n";
  std::cout << "Sensors: " << params.nodes << "\n";
  std::cout << "Duration: " << params.duration << " s\n";
  std::cout << "Payload: " << params.payload << " bytes\n";
  std::cout << "Socket Factory: " << (profile.usesTcp ? "TcpSocketFactory" : "UdpSocketFactory") << "\n";
  std::cout << "Security Handshake Model: " << (securityProfile.requiresHandshake ? "Enabled" : "Disabled") << "\n";
  std::cout << "Mutual Authentication: " << (securityProfile.mutualAuthentication ? "Yes" : "No") << "\n";
  std::cout << "Persistent Session: " << (profile.persistentSession ? "Yes" : "No") << "\n";
  std::cout << "Keep Alive: " << (profile.keepAlive ? "Yes" : "No") << "\n";
  std::cout << "Application Header: " << profile.applicationHeaderBytes << " bytes\n";
  std::cout << "Final Packet Size: " << (params.payload + profile.applicationHeaderBytes) << " bytes\n";
  std::cout << "Protocol Control Overhead: " << GetProtocolControlOverheadBytes(profile) << " bytes\n";
  std::cout << "Application Message Overhead: " << GetApplicationMessageOverheadBytes(profile) << " bytes\n";

  NodeContainer gatewayNode;
  gatewayNode.Create(1);

  NodeContainer sensorNodes;
  sensorNodes.Create(params.nodes);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211g);
  wifi.SetRemoteStationManager("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid("iot-study-ssid");

  mac.SetType("ns3::StaWifiMac",
              "Ssid", SsidValue(ssid),
              "ActiveProbing", BooleanValue(false));
  NetDeviceContainer sensorDevices = wifi.Install(phy, mac, sensorNodes);

  mac.SetType("ns3::ApWifiMac",
              "Ssid", SsidValue(ssid));
  NetDeviceContainer gatewayDevice = wifi.Install(phy, mac, gatewayNode);

  MobilityHelper mobilitySensors;
  mobilitySensors.SetPositionAllocator("ns3::GridPositionAllocator",
                                       "MinX", DoubleValue(5.0),
                                       "MinY", DoubleValue(5.0),
                                       "DeltaX", DoubleValue(5.0),
                                       "DeltaY", DoubleValue(5.0),
                                       "GridWidth", UintegerValue(10),
                                       "LayoutType", StringValue("RowFirst"));
  mobilitySensors.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilitySensors.Install(sensorNodes);

  MobilityHelper mobilityGateway;
  Ptr<ListPositionAllocator> gatewayPosition = CreateObject<ListPositionAllocator>();
  gatewayPosition->Add(Vector(0.0, 0.0, 0.0));
  mobilityGateway.SetPositionAllocator(gatewayPosition);
  mobilityGateway.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityGateway.Install(gatewayNode);

  InternetStackHelper stack;
  stack.Install(gatewayNode);
  stack.Install(sensorNodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer sensorInterfaces = address.Assign(sensorDevices);
  Ipv4InterfaceContainer gatewayInterface = address.Assign(gatewayDevice);

  double warmup = 1.0;
  uint16_t gatewayPort = 9999;
  Address gatewayAddress = InetSocketAddress(Ipv4Address::GetAny(), gatewayPort);
  std::string sinkFactory = profile.usesTcp ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory";
  PacketSinkHelper packetSinkHelper(sinkFactory, gatewayAddress);
  ApplicationContainer sinkApps = packetSinkHelper.Install(gatewayNode.Get(0));
  sinkApps.Start(Seconds(0.0));
  sinkApps.Stop(Seconds(warmup + params.duration + 1.0));

  Ptr<IoTGatewayApplication> gatewayApp = CreateObject<IoTGatewayApplication>();
  gatewayNode.Get(0)->AddApplication(gatewayApp);
  gatewayApp->SetStartTime(Seconds(0.0));
  gatewayApp->SetStopTime(Seconds(warmup + params.duration + 1.0));

  Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApps.Get(0));
  if (sink) {
    sink->TraceConnectWithoutContext("RxWithAddresses",
                                     MakeCallback(&IoTGatewayApplication::HandlePacketTrace,
                                                  gatewayApp));
  }

  Ipv4Address remoteAddress = gatewayInterface.GetAddress(0);
  ApplicationContainer sensorApps;
  for (uint32_t index = 0; index < params.nodes; ++index) {
    Ptr<IoTSensorApplication> app = CreateObject<IoTSensorApplication>();
    InetSocketAddress remoteEndpoint(remoteAddress, gatewayPort);
    app->SetRemote(remoteEndpoint);
    app->SetPayloadSize(params.payload);
    app->SetSendInterval(Seconds(1.0));
    app->SetCommunicationProfile(profile);
    app->SetSecurityProfile(securityProfile);
    sensorNodes.Get(index)->AddApplication(app);
    app->SetStartTime(Seconds(warmup));
    app->SetStopTime(Seconds(warmup + params.duration));
    sensorApps.Add(app);
  }

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  // Simulator::Stop(Seconds(params.duration));
  Simulator::Stop(Seconds(warmup + params.duration + 1.0));

  auto startCpu = std::chrono::steady_clock::now();
  Simulator::Run();
  auto endCpu = std::chrono::steady_clock::now();

  double cpuTimeSeconds = std::chrono::duration<double>(endCpu - startCpu).count();

  monitor->CheckForLostPackets();

  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();
  uint64_t txPackets = 0;
  uint64_t rxPackets = 0;
  uint64_t lostPackets = 0;
  uint64_t rxBytes = 0;
  Time delaySum = Seconds(0.0);
  Time jitterSum = Seconds(0.0);

  for (const auto& flow : stats) {
    txPackets += flow.second.txPackets;
    rxPackets += flow.second.rxPackets;
    lostPackets += flow.second.lostPackets;
    rxBytes += flow.second.rxBytes;
    delaySum += flow.second.delaySum;
    jitterSum += flow.second.jitterSum;
  }

  double throughputKbps = CalculateThroughput(rxBytes, params.duration);
  double averageDelayMs = CalculateAverageDelay(delaySum, rxPackets);
  double averageJitterMs = CalculateAverageJitter(jitterSum, rxPackets);

  PrintMetrics(txPackets,
               rxPackets,
               lostPackets,
               rxBytes,
               throughputKbps,
               averageDelayMs,
               averageJitterMs,
               cpuTimeSeconds);

  uint64_t totalMessagesSent = 0;
  for (uint32_t i = 0; i < sensorApps.GetN(); ++i) {
    Ptr<IoTSensorApplication> sensorApp = DynamicCast<IoTSensorApplication>(sensorApps.Get(i));
    if (sensorApp) {
      totalMessagesSent += sensorApp->GetMessagesSent();
    }
  }

  uint64_t applicationMessagesReceived = gatewayApp->GetMessagesReceived();
  uint64_t applicationMessagesLost = gatewayApp->GetMessagesLost();
  double averageAppDelayMs = gatewayApp->GetAverageApplicationDelayMs();

  std::cout << "\n========== APPLICATION METRICS ==========\n";
  std::cout << "Messages Sent: " << totalMessagesSent << "\n";
  std::cout << "Messages Received: " << applicationMessagesReceived << "\n";
  std::cout << "Messages Lost: " << applicationMessagesLost << "\n";
  std::cout << "Average Application Delay: " << std::fixed << std::setprecision(3)
            << averageAppDelayMs << " ms\n";

  Simulator::Destroy();

  return 0;
}
