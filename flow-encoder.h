#ifndef FLOW_ENCODER_H
#define FLOW_ENCODER_H

#include <iosfwd>

#include "ns3/object.h"
#include "ns3/net-device.h"

namespace ns3 {
  
class FlowEncoder : public Object
{
public:

  struct CountTableEntry
  {
    uint32_t xor_srcip;
    uint32_t xor_dstip;
    uint16_t xor_srcport;
    uint16_t xor_dstport;
    uint8_t  xor_prot;

    uint8_t  flow_cnt;
    uint32_t packet_cnt;  
  };

  struct FlowField
  {
    uint32_t ipv4srcip;
    uint32_t ipv4dstip;
    uint16_t srcport;
    uint16_t dstport;
    uint8_t  ipv4prot;

    FlowField():ipv4srcip(0), ipv4dstip(0), srcport(0), dstport(0), ipv4prot(0)
    {}
  };

  //static const int NUM_FLOW; num of flow in 10ms
  static const int NUM_COUNT_HASH;        //num of counter table hash function
  static const int COUNT_TABLE_SIZE;      //num of counter table entries
  static const int NUM_FLOW_HASH;         //num of flow filter hash function
  static const int FLOW_FILTER_SIZE;      //num of bits of flow filter
  

  /*Initialize the flow filter and count table
   */
  FlowEncoder ();
  virtual ~FlowEncoder ();

  /* Set the openflow switch's promisc receive callback to this flow encoder' 
   * recive function;
   */
  void SetOFSwtch (Ptr<NetDevice> OFswtch, int id);

  /* The call back function for openflow switch net device.
   * When openflow swtich net device receive a new packet(it's ReceiveFromDevice
   * function be called.), it will use it's m_promiscRxCallback which match to 
   * this function.
   */
  bool ReceiveFromOpenFlowSwtch (Ptr<NetDevice> ofswtch,
				 Ptr<const Packet> constPacket, uint16_t protocol,
				 const Address& src, const Address& dst,
				 NetDevice::PacketType packetType);

private:

  FlowField FlowFieldFromPacket(Ptr<Packet> packet, uint16_t protocol);
  
  int                           m_id; //id of the switch node
  std::vector<uint32_t>         m_flowFilter;
  std::vector<CountTableEntry>  m_countTable;
};

std::ostream& operator<<(std::ostream& os, const FlowEncoder::FlowField& flow);

}

#endif
