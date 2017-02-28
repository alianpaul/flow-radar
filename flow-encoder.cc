
#include <cmath>

#include "flow-encoder.h"
#include "openflow-switch-net-device.h"

#include "ns3/log.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/tcp-l4-protocol.h"


namespace ns3 {
  
NS_LOG_COMPONENT_DEFINE("FlowEncoder");

NS_OBJECT_ENSURE_REGISTERED(FlowEncoder);

/*
const int FlowEncoder::NUM_FLOW = 100000;
const int FlowEncoder::KC       = 3;
const int FlowEncoder::MC       = 1.24 * numflow;
const double DECODE_FAILURE_RATE = pow(NUM_FLOW, 2 - KC) ; 
const double FALSE_POSITIVE      = 1.0/10.0 * DECODE_FAILURE_RATE;  
const int FlowEncoder::KF = ;
*/
const int FlowEncoder::NUM_COUNT_HASH   = 4;
const int FlowEncoder::COUNT_TABLE_SIZE = 16;
const int FlowEncoder::NUM_FLOW_HASH    = 27;
const int FlowEncoder::FLOW_FILTER_SIZE = 32768; //bits count

  
FlowEncoder::FlowEncoder()
{
  m_flowFilter.resize( FLOW_FILTER_SIZE / 32, 0);
  m_countTable.resize( COUNT_TABLE_SIZE, CountTableEntry());
}

FlowEncoder::~FlowEncoder()
{
}

void
FlowEncoder::SetOFSwtch (Ptr<NetDevice> OFswtch, int id)
{
  NS_LOG_FUNCTION(this);

  m_id = id;
  
  OFswtch->SetPromiscReceiveCallback(MakeCallback(&FlowEncoder::ReceiveFromOpenFlowSwtch, this));

}
  
bool
FlowEncoder::ReceiveFromOpenFlowSwtch(Ptr<NetDevice> ofswtch,
				      Ptr<const Packet> constPacket,
				      uint16_t protocol,
				      const Address& src, const Address& dst,
				      NetDevice::PacketType packetType)
{
  NS_LOG_INFO("FlowEncoder ID is " <<m_id);
  
  Ptr<Packet> packet = constPacket->Copy();

  FlowField   flow   = FlowFieldFromPacket (packet, protocol);
  
  std::cout << flow << std::endl;

  NS_LOG_INFO("Flow encode finish");
  
  return true;
}

FlowEncoder::FlowField
FlowEncoder::FlowFieldFromPacket(Ptr<Packet> packet, uint16_t protocol)
{
  NS_LOG_INFO("Extract flow field");
    
  FlowField flow;
  if(protocol == Ipv4L3Protocol::PROT_NUMBER)
    {
      Ipv4Header ipHd;
      if( packet->PeekHeader(ipHd) )
	{
	  NS_LOG_INFO("IP header detected");
	  
	  flow.ipv4srcip = ipHd.GetSource().Get();
	  flow.ipv4dstip = ipHd.GetDestination().Get();
	  flow.ipv4prot  = ipHd.GetProtocol ();
	  

	  if( flow.ipv4prot == TcpL4Protocol::PROT_NUMBER)
	    {
	      TcpHeader tcpHd;
	      if( packet->PeekHeader(tcpHd) )
		{
		  NS_LOG_INFO("TCP header detected");
		  
		  flow.srcport = tcpHd.GetSourcePort ();
		  flow.dstport = tcpHd.GetDestinationPort ();

		}
	    }
	  else if( flow.ipv4prot == UdpL4Protocol::PROT_NUMBER )
	    {
	      UdpHeader udpHd;
	      if( packet->PeekHeader(udpHd))
		{
		  NS_LOG_INFO("UDP header detected");
		 
		  flow.srcport = udpHd.GetSourcePort ();
		  flow.dstport = udpHd.GetDestinationPort ();

		}
	    }
	  else
	    {
	      NS_LOG_INFO("layer 4 protocol can't extract: "<< unsigned(flow.ipv4prot));
	    }
	  
	}
    }
  else
    {
      NS_LOG_INFO("packet is not an ip packet");
    }

  return flow;
}

std::ostream&
operator<<(std::ostream& os, const FlowEncoder::FlowField& flow)
{
  Ipv4Address srcip (flow.ipv4srcip);
  Ipv4Address dstip (flow.ipv4dstip);
  std::string prot;
  if (flow.ipv4prot == TcpL4Protocol::PROT_NUMBER)
    prot = "TCP";
  else if (flow.ipv4prot == UdpL4Protocol::PROT_NUMBER)
    prot = "UDP";
  else
    {
      prot = "Unknown L4 prot";
      os << flow.ipv4prot;
    }
    
  uint16_t srcport = flow.srcport;
  uint16_t dstport = flow.dstport;

  os << "flow:"<< srcip << " " << dstip << " " << prot << " "
     << srcport << " " << dstport;

  return os;
}

}

