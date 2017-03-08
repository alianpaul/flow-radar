
#include <ctime>
#include <cstdlib>
#include <string>
#include <fstream>
#include <bitset>

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
AppGen::GenRandomUDPFlow (int flowCnt)
{
  /*
  std::time_t     rawtime;
  std::time (&rawtime);
  struct std::tm* timeinfo = std::localtime(&rawtime);
  char            buffer[80];
  std::strftime( buffer, sizeof(buffer), "%d-%m-%Y", timeinfo);
  std::string     filename (buffer);
  */
  std::string     filename = "gen-flow.txt";
  std::ofstream   file;
  file.open(filename.c_str());

  if(!file)
    {
      NS_LOG_ERROR("gen flow file open failed");
    }

  std::bitset<65535> preports;
  int numHost = m_topo->GetNumHost();
  for (int ith = 0; ith < flowCnt; ++ith)
    {
      
      int src  = std::rand() % numHost;
      int dst  = std::rand() % numHost;
      while( src == dst )
	{
	  dst  = std::rand() % numHost;
	}
      
      int port = std::rand() % 65535;
      while( preports[port] )
	{
	  port = std::rand() % 65535;
	}
      preports[port] = true;

      
      int maxBytes = 1024;
      Ptr<Node>   clientNode = m_topo->GetHostNode(src);
      Ptr<Node>   serverNode = m_topo->GetHostNode(dst);
      Ipv4Address serverAddr = m_topo->GetHostIPAddr(dst);
      Ipv4Address clientAddr = m_topo->GetHostIPAddr(src);
      
      //client
      OnOffHelper onoff("ns3::UdpSocketFactory",
			Address(InetSocketAddress(serverAddr, port)));
      onoff.SetAttribute("MaxBytes", UintegerValue(maxBytes));
      onoff.SetConstantRate(DataRate("500kb/s"));
      ApplicationContainer app = onoff.Install (clientNode);
      app.Start(Seconds(0.1));
      app.Stop(Seconds(0.5));

      //server
      PacketSinkHelper sink("ns3::UdpSocketFactory",
			    Address(InetSocketAddress(Ipv4Address::GetAny(),
						      port)));
      app = sink.Install(serverNode);
      app.Start(Seconds(0.0));
	
      file << clientAddr << " " << serverAddr << " " << "UDP "<< port << std::endl;
    }

  file.close();
}
  
void
AppGen::GenTCPFlow (int src, int dst, uint16_t port, uint32_t maxBytes,
		   float startTime)
{
 
}

void
AppGen::GenUDPFlow (int src, int dst, uint16_t port, uint32_t maxBytes,
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
