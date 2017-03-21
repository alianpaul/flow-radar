
#include "ns3/log.h"
#include "ns3/csma-net-device.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/udp-header.h"
#include "ns3/tcp-header.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/arp-l3-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/arp-cache.h"

#include "packet-gen.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("PacketGenerator");

NS_OBJECT_ENSURE_REGISTERED(PacketGenerator);

PacketGenerator::PacketGenerator(int                hostid,  
				 Ptr<NetDevice>     netdev,
				 Ptr<Node>          node,
				 const Address&     macAddr,
				 const Ipv4Address& ipv4Addr)
  : m_hostID(hostid), m_macAddr(macAddr), m_ipv4Addr(ipv4Addr),
    m_device(netdev), m_node(node)
{
  NS_LOG_FUNCTION( hostid );
}

PacketGenerator::~PacketGenerator()
{
}

void 
PacketGenerator::ScheduleNextTx()
{
  //1. Read one packet information from you pcap file
  //   If no more packetm, just return to stop.

  //2. Transform the information to current network

  //3. Schedule the transmit event according to the time
  //   Simulator::Schedule(time, &SendPacket, this, ...)
  //   ...means other arguments like packet size, packet type, port etc.
}

void
PacketGenerator::SendPacket(Ipv4Address ipdst,
			    uint16_t    portsrc,
			    uint16_t    portdst,
			    uint8_t     type,
			    uint32_t    size)
{
  //1. Generate the packet according to size

  //2. Use Send() to send the packet.

  //3. Schedule next transmit event.
}
  
void
PacketGenerator::Send( Ptr<Packet>        packet,
		       const Ipv4Address& ipdst,
		       uint16_t           portsrc,
		       uint16_t           portdst,
		       uint8_t            type)
{
  // 1.Add Transport Layer header
  if(type == UdpL4Protocol::PROT_NUMBER)
    {
      UdpHeader udpHeader;

      if( Node::ChecksumEnabled() )
	{
	  udpHeader.EnableChecksums ();
	  udpHeader.InitializeChecksum(m_ipv4Addr,
				       ipdst,
				       UdpL4Protocol::PROT_NUMBER);
	}
      udpHeader.SetDestinationPort (portdst);
      udpHeader.SetSourcePort      (portsrc);
      packet->AddHeader(udpHeader);
    }
  else if(type == TcpL4Protocol::PROT_NUMBER)
    {
      TcpHeader tcpHeader;

      tcpHeader.SetDestinationPort (portdst);
      tcpHeader.SetSourcePort      (portsrc);
      packet->AddHeader(tcpHeader);  
    }
  else
    {
      NS_FATAL_ERROR("Transport Layer Protocol Unknown");
      return;
    }

  Ptr<Ipv4>           ipv4     = m_node->GetObject<Ipv4> ();
  Ptr<Ipv4L3Protocol> ipv4Impl = DynamicCast<Ipv4L3Protocol, Ipv4>(ipv4);
  
  //2. Add IP Layer header
  Ipv4Header          ipHeader = ipv4Impl->BuildHeader(m_ipv4Addr, ipdst,
						       type,
						       packet->GetSize(),
						       32, //ttl
						       0, //tos
						       false); //mayFragment
  packet->AddHeader(ipHeader);

  
  Address hwDst = GetHardwareDstAddr(ipv4Impl, ipdst);
  //3. Send the Packet through net device
  m_device->Send(packet, hwDst, Ipv4L3Protocol::PROT_NUMBER);
}

Address
PacketGenerator::GetHardwareDstAddr(Ptr<Ipv4L3Protocol> ipv4Impl,
				    const Ipv4Address& ipdst)
{

  if (ipdst.IsBroadcast())
    {
      return m_device->GetBroadcast();
    }
  else if(ipdst.IsMulticast())
    {
      return m_device->GetMulticast(ipdst);
    }
  else
    {
      int32_t interface = ipv4Impl->GetInterfaceForDevice(m_device);
      NS_ASSERT( interface >= 0 );
      Ptr<Ipv4Interface> outInterface = ipv4Impl->GetInterface (interface);
      Ptr<ArpCache>      arpCache     = outInterface->GetArpCache();
      ArpCache::Entry *entry = arpCache->Lookup(ipdst);

      //We set the arp cache to be permanent, it should exist.
      if(entry != 0)
	{
	  return entry->GetMacAddress();
	}
    }
  
  NS_FATAL_ERROR("Arp cache error");
  return Address();
  
}
  
}
