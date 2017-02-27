#ifndef FLOW_ENCODER_H
#define FLOW_ENCODER_H

#include "ns3/object.h"
#include "ns3/net-device.h"

namespace ns3 {
  
class FlowEncoder : public Object
{
public:
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
  bool ReceiveFromOpenFlowSwtch (Ptr<NetDevice> ofswtch, Ptr<const Packet> packet,
				uint16_t protocol,
				const Address& src, const Address& dst,
				NetDevice::PacketType packetType);

private:
  int  m_id;  //id of the switch node the flow encoder is installed on
};

}

#endif
