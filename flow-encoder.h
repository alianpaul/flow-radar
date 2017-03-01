#ifndef FLOW_ENCODER_H
#define FLOW_ENCODER_H

#include <iosfwd>
#include <bitset>

#include "ns3/object.h"
#include "ns3/net-device.h"

#include "flow-radar-config.h"

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

    CountTableEntry(): xor_srcip(0), xor_dstip(0), xor_srcport(0), xor_dstport(0),
		       xor_prot(0), flow_cnt(0), packet_cnt(0)
    {}
  };

  typedef std::vector<CountTableEntry>  CountTable_t;
  typedef std::bitset<FLOW_FILTER_SIZE> FlowFilter_t;

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
  friend  std::ostream& operator<<(std::ostream& os, const FlowField& flow);

  
  /*Initialize the flow filter and count table
   */
  FlowEncoder ();
  virtual ~FlowEncoder ();

  /* Set the openflow switch's promisc receive callback to this flow encoder' 
   * recive function;
   */
  void                SetOFSwtch (Ptr<NetDevice> OFswtch, int id);
  
  int                 GetID();

  CountTable_t&       GetCountTable();
  /* Clear flow filter and count table.
   */
  void                Clear();

  
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

  /* Extract the 5 tuple flow infro from packet to struct FlowField
   */
  FlowField    FlowFieldFromPacket(Ptr<Packet> packet, uint16_t protocol) const;

  /* Check the flow with the flow filter, if the flow is new, and update the 
   * flow filter and return true;
   * else return false;
   */
  bool         UpdateFlowFilter(const FlowField& flow);

  /* Update the flow according count table.
   * If the flow is new, we xor the flow fields,
   * else, we only update the packet count.
   */
  void         UpdateCountTable(const FlowField& flow, bool isNew);

  /* Calculate the flow filter idxs;
   */
  std::vector<uint32_t> GetFlowFilterIdx(const FlowField& flow) const;

  /* Calculate the count table idxs;
   */
  std::vector<uint32_t> GetCountTableIdx(const FlowField& flow) const;
  
  int               m_id;          //id of the switch node
  FlowFilter_t      m_flowFilter;  //bit
  CountTable_t      m_countTable;  //count table
};

 
}

#endif
