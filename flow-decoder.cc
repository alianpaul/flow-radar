
#include <algorithm>

#include "ns3/log.h"
#include "ns3/simulator.h"

#include "flow-decoder.h"


namespace ns3
{

NS_LOG_COMPONENT_DEFINE("FlowDecoder");
  
FlowDecoder::FlowDecoder ()
{
}

FlowDecoder::~FlowDecoder ()
{
}
  
void
FlowDecoder::DecodeFlows ()
{
  NS_LOG_FUNCTION(Simulator::Now().GetSeconds());

  NodeFlowInfo_t nodeFlowInfo;

  for (unsigned ith = 0; ith < m_encoders.size(); ++ith)
    {
      SingleDecode (m_encoders[ith], nodeFlowInfo);
    }
  
  if(Simulator::Now().GetSeconds() + PERIOD < END_TIME)
    Simulator::Schedule (Seconds(PERIOD), &FlowDecoder::DecodeFlows, this);
}
  
void
FlowDecoder::Init()
{
  Simulator::Schedule (Seconds(PERIOD), &FlowDecoder::DecodeFlows, this);
}

void
FlowDecoder::AddEncoder(Ptr<FlowEncoder> encoder)
{
  m_encoders.push_back(encoder);
}

//
bool IsFlowCountOne(const FlowEncoder::CountTableEntry& e)
{
  return (e.flow_cnt == 1);
}

bool
FlowDecoder::SingleDecode(Ptr<FlowEncoder> target, NodeFlowInfo_t& nodeFlowInfo)
{

  NS_LOG_FUNCTION (target->GetID());

  FlowEncoder::CountTable_t& countTable = target->GetCountTable();

  FlowEncoder::CountTable_t::iterator it;
  while((it = std::find_if(countTable.begin(), countTable.end(), IsFlowCountOne))
	!= countTable.end())
    {
      NS_LOG_INFO( "flow cnt:" << it->flow_cnt-- );
      NS_LOG_INFO( "packet cnt:" << it->packet_cnt-- );
    }
  
  return false;
}

}

