
#include "flow-encoder.h"
#include "openflow-switch-net-device.h"

#include "ns3/log.h"


namespace ns3 {
  
NS_LOG_COMPONENT_DEFINE("FlowEncoder");

NS_OBJECT_ENSURE_REGISTERED(FlowEncoder);

FlowEncoder::FlowEncoder()
{
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
				      Ptr<const Packet> packet, uint16_t protocol,
				      const Address& src, const Address& dst,
				      NetDevice::PacketType packetType)
{
  NS_LOG_INFO("FlowEncoder ID is " <<m_id);

  return true;
}

}

