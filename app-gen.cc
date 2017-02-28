
#include "app-gen.h"
#include "dc-topology.h"

#include "ns3/applications-module.h"
#include "ns3/log.h"


NS_LOG_COMPONENT_DEFINE("AppGen");

namespace ns3{

AppGen::AppGen(Ptr<DCTopology> topo) : m_topo(topo)
{
}

AppGen::~AppGen()
{
}

void
AppGen::GenTCPFlow(int src, int dst, uint16_t port, uint32_t maxBytes,
		   float startTime)
{
 
}

void
AppGen::GenUDPFlow(int src, int dst, uint16_t port, uint32_t maxBytes,
		  float startTime, float stopTime)
{
  Ptr<Node>   clientNode = m_topo->GetHostNode(src);
  
  Ptr<Node>   serverNode = m_topo->GetHostNode(dst);
  Ipv4Address serverAddr = m_topo->GetHostIPAddr(dst);

  //client
  OnOffHelper onoff("ns3::UdpSocketFactory",
			  Address(InetSocketAddress(serverAddr, port)));
  onoff.SetAttribute("MaxBytes", UintegerValue(maxBytes));
  onoff.SetConstantRate(DataRate("500kb/s"));
  ApplicationContainer app = onoff.Install (clientNode);
  app.Start(Seconds(startTime));
  app.Stop(Seconds(stopTime));

  //server
  PacketSinkHelper sink("ns3::UdpSocketFactory",
			Address(InetSocketAddress(Ipv4Address::GetAny(), port)));

  app = sink.Install(serverNode);
  app.Start(Seconds(0.0));

}

}
