

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h" 
#include "ns3/ipv4-global-routing-helper.h" 
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/animation-interface.h"

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"

using namespace ns3;


uint32_t checkTimes;
double avgQueueSize;

// The times
double global_start_time;
double global_stop_time;
double sink_start_time;
double sink_stop_time;
double client_start_time;
double client_stop_time;

NodeContainer n0n2;
NodeContainer n1n2;
NodeContainer n2n3;
NodeContainer w0w1;

Ipv4InterfaceContainer i0i2;
Ipv4InterfaceContainer i1i2;
Ipv4InterfaceContainer i2i3;
Ipv4InterfaceContainer i3i4;
Ipv4InterfaceContainer i3i5;

std::stringstream filePlotQueue;
std::stringstream filePlotQueueAvg;


int
main (int argc, char *argv[])
{
      
   std::cout<<"anand1\n";   
  // std::string tcp_variant = "TcpNewReno";
  //std::string bandwidth = "5Mbps";
  std::string bandwidth = "10Mbps";
  //std::string delay = "2ms";
  std::string delay = "1ms";
  //double error_rate = 0.000001;
 //int queuesize = 10; //packets
  //int simulation_time = 20; //seconds
  bool tracing = true; 
    
  //uint32_t redTest;
 

  //bool printRedStats = true;

  global_start_time = 0.0;
  global_stop_time = 11; 
  sink_start_time = global_start_time;
  sink_stop_time = global_stop_time + 3.0;
  client_start_time = sink_start_time + 0.2;
  client_stop_time = global_stop_time - 2.0;

 
  NS_LOG_INFO ("Create nodes");
  NodeContainer c,w;
  c.Create (4);
  Names::Add ( "N0", c.Get (0));
  Names::Add ( "N1", c.Get (1));
  Names::Add ( "N2", c.Get (2));
  Names::Add ( "N3", c.Get (3));
  //Names::Add ( "N4", c.Get (4));
  //Names::Add ( "N5", c.Get (5));
  n0n2 = NodeContainer (c.Get (0), c.Get (2));
  n1n2 = NodeContainer (c.Get (1), c.Get (2));
  n2n3 = NodeContainer (c.Get (2), c.Get (3));
  //n3n4 = NodeContainer (c.Get (3), c.Get (4));
  //n3n5 = NodeContainer (c.Get (3), c.Get (5));
  w.Create(2);
  NodeContainer wifiApNode = c.Get (0);
  
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue (bandwidth));
  p2p.SetChannelAttribute ("Delay", StringValue (delay));
      
  NetDeviceContainer d_link[3];
  d_link[0] = p2p.Install(n0n2);
  d_link[1] = p2p.Install(n1n2);
  d_link[2] = p2p.Install(n2n3);
  
  std::cout<<"anand2\n";
  
  
  
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;          //similar to PointToPointHelper
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, w);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (w);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  
  InternetStackHelper stack;
  stack.Install(w);
  stack.Install(c);
  

  Ipv4AddressHelper address;

    std::cout<<"anand3\n";
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces0;
  p2pInterfaces0 = address.Assign (d_link[0]);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces1;
  p2pInterfaces1 = address.Assign (d_link[1]);
  
  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces2;
  p2pInterfaces2 = address.Assign (d_link[2]);

  address.SetBase ("10.1.4.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (c.Get (3));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (p2pInterfaces2.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = 
    echoClient.Install (w.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
std::cout<<"anand4\n";
  Simulator::Stop (Seconds (10.0));
    //bool tracing = false;
  if (tracing == true)
    {
      p2p.EnablePcapAll ("third");
      //phy.EnablePcap ("third", apDevices.Get (0));
      //csma.EnablePcap ("third", csmaDevices.Get (0), true);
    }

  //Simulator::Run ();
  
  ns3::PacketMetadata::Enable();
    std::string animFile = "topology.xml";
  
  AnimationInterface anim(animFile);
    Ptr<Node> n = c.Get(0);
    anim.SetConstantPosition(n, 0, 20);
    n = c.Get(1);
    anim.SetConstantPosition(n, 20, 60);
    n = c.Get(2);
    anim.SetConstantPosition(n, 20, 40);
    n = c.Get(3);
    anim.SetConstantPosition(n, 80, 20);
    n = w.Get(0);
    anim.SetConstantPosition(n, 80, 60);
    n = w.Get(1);
    anim.SetConstantPosition(n, 40, 40);
  //  n = nodes.Get(6);
  //  anim.SetConstantPosition(n, 10, 0);
  //  n = nodes.Get(7);
  //  anim.SetConstantPosition(n, 10, 2);
  //  n = nodes.Get(8);
  //  anim.SetConstantPosition(n, 10, 4);
  //  n = nodes.Get(9);
  //  anim.SetConstantPosition(n, 10, 6);
  //  n = nodes.Get(10);
  //  anim.SetConstantPosition(n, 10, 8);
   // n = nodes.Get(11);
  //  anim.SetConstantPosition(n, 6, 4);
  
  
  /**flow monitor **/


// 1. Install FlowMonitor on all nodes
FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

// 2. Run simulation for 60 seconds
Simulator::Stop (Seconds (60));
Simulator::Run ();
// 3. Print per flow statistics
monitor->CheckForLostPackets ();
Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
{
Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
std::cout << "Flow " << i->first - 1 << " (" << t.sourceAddress << ":"<<t.sourcePort<< " -> " << t.destinationAddress << ":"<<t.destinationPort << ")\n";std::cout << " Tx Bytes: " << i->second.txBytes << "\n";
std::cout << " Rx Bytes: " << i->second.rxBytes << "\n";
std::cout << " Throughput: " << i->second.rxBytes * 8.0 /(i->second.timeLastRxPacket.GetSeconds()- i->second.timeFirstTxPacket.GetSeconds()) / 1024 / 1024 << " Mbps \n"; 
}
  Simulator::Destroy ();
  return 0;

}
