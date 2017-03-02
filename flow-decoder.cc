
#include <algorithm>

#include "ns3/log.h"
#include "ns3/simulator.h"

#include "flow-decoder.h"
#include "flow-encoder.h"
#include "flow-field.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("FlowDecoder");
  
FlowDecoder::FlowDecoder (Ptr<DCTopology> topo)
  : m_topo (topo)
{
}

FlowDecoder::~FlowDecoder ()
{
}
  
void
FlowDecoder::DecodeFlows ()
{
  NS_LOG_FUNCTION(Simulator::Now().GetSeconds());

  //the first SingleDecode pass;
  std::cout << "1 pass" << std::endl; 
  for (unsigned ith = 0; ith < m_encoders.size(); ++ith)
    {
      SingleDecode (m_encoders[ith]);
    }

  /*Debug the first pass result
  SWFlowInfo_t::const_iterator itNode;
  for(itNode = m_curSWFlowInfo.begin(); itNode != m_curSWFlowInfo.end(); ++itNode)
    {
      int               swID     = itNode->first;
      std::cout << swID << std::endl;
      const FlowInfo_t& flowInfo = itNode->second;

      FlowInfo_t::const_iterator itFlow;
      for(itFlow = flowInfo.begin(); itFlow != flowInfo.end(); ++itFlow)
	{
	  std::cout << "Flow: " << itFlow->first << " Packet: " << itFlow->second <<std::endl;
	}
      
    }
  */
  int pass = 1;
  while(!m_passNewFlows.empty())
    {
        FlowSet_t::const_iterator itFlow;
	for(itFlow = m_passNewFlows.begin();
	    itFlow != m_passNewFlows.end(); ++itFlow)
	  {
	    
	    std::cout << (*itFlow) << std::endl;
	  }


	//TODO:Flow  Decode
	
	m_passNewFlows.clear(); //clear for the next pass of decode

	std::cout << ++pass <<" pass"<< std::endl;	
	for (unsigned ith = 0; ith < m_encoders.size(); ++ith)
	  {
	    SingleDecode (m_encoders[ith]);
	  }
    }
  
  // Schedule next decode.
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

//algorithm find_if predicate
bool IsFlowCountOne(const FlowEncoder::CountTableEntry& e)
{
  return (e.flow_cnt == (uint8_t)1);
}

void
FlowDecoder::SingleDecode(Ptr<FlowEncoder> target)
{

  int swID = target->GetID();
  NS_LOG_INFO ("Decoding at "<< swID);

  FlowEncoder::CountTable_t&          countTable   = target->GetCountTable();
  int                                 numDecoded   = 0; //flows decoded
  FlowEncoder::CountTable_t::iterator itEntry;
  while( (itEntry = std::find_if(countTable.begin(), countTable.end(),
				 IsFlowCountOne))
	!= countTable.end() )
    {
      //Find a pure cell
      FlowField flow      = (*itEntry).GetFlow();
      uint32_t  packetCnt = (*itEntry).packet_cnt;
      numDecoded++;
      
      NS_LOG_INFO ("Flow: "<< flow <<" Pakcet: " << packetCnt );
      m_curSWFlowInfo[swID][flow] = packetCnt;

      /* If it's a new flow doesn't collected among switches before,
       * add to m_passNewFlows
       */
      if (m_passNewFlows.find(flow) == m_passNewFlows.end())
	{
	  m_passNewFlows.insert(flow);
	}
      
      //Clear this flow in the count table
      std::vector<uint32_t> tableIdxs = FlowEncoder::GetCountTableIdx (flow);
      for(unsigned ith = 0; ith < NUM_COUNT_HASH; ++ith )
	{     
	  FlowEncoder::CountTableEntry& entry = countTable[tableIdxs[ith]];
	  entry.XORFlow (flow);
	  entry.flow_cnt   --;
	  entry.packet_cnt -= packetCnt;
	}
    }  
}

}

