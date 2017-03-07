
#include <algorithm>

#include "ns3/log.h"
#include "ns3/simulator.h"

#include "flow-decoder.h"
#include "flow-encoder.h"
#include "flow-field.h"
#include "LSXR/lsqrDense.h"

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

  const Graph::AdjList_t& adjList = m_topo->GetAdjList();
  const Graph             graph   (adjList);

  /*  1. Flow decode in this frame    */
  //the first pass;
  std::cout << "1 pass" << std::endl; 
  for (unsigned ith = 0; ith < m_encoders.size(); ++ith)
    {
      FlowSingleDecode (m_encoders[ith]);
    }

  
  int pass = 1;
  while(!m_passNewFlows.empty())
    {
        FlowSet_t::const_iterator itFlow;
	for(itFlow = m_passNewFlows.begin();
	    itFlow != m_passNewFlows.end(); ++itFlow)
	  {
	    //Hacking: the node id == the last Ipv4 address section - 1
	    int           from = ((*itFlow).ipv4srcip & 0xff) - 1;
	    int           to   = ((*itFlow).ipv4dstip & 0xff) - 1;
	    Graph::Path_t path = graph.GetPath(from, to);

	    DecodeFlowOnPath (path, *itFlow);
	  }
	
	m_passNewFlows.clear(); //clear for the next pass of decode

	std::cout << ++pass <<" pass"<< std::endl;	
	for (unsigned ith = 0; ith < m_encoders.size(); ++ith)
	  {
	    FlowSingleDecode (m_encoders[ith]);
	  }
    }

  /*   2.Counter Decode in this frame    */
  for (unsigned ith = 0; ith < m_encoders.size(); ++ith)
    {
      CounterSingleDecode (m_encoders[ith]);
    }
    
  // Schedule next decode.
  if(Simulator::Now().GetSeconds() + PERIOD < END_TIME)
    Simulator::Schedule (Seconds(PERIOD), &FlowDecoder::DecodeFlows, this);
  else
    {
      //Output current decoded flows by swID;
      SWFlowInfo_t::const_iterator itNode;
      for(itNode = m_curSWFlowInfo.begin();
	  itNode != m_curSWFlowInfo.end(); ++itNode)
	{
	  int               swID     = itNode->first;
	  std::cout << swID << std::endl;
	  const FlowInfo_t& flowInfo = itNode->second;

	  FlowInfo_t::const_iterator itFlow;
	  for(itFlow = flowInfo.begin(); itFlow != flowInfo.end(); ++itFlow)
	    {
	      std::cout << itFlow->first << std::endl;
	    }
      
	}
      
    }
}
  
void
FlowDecoder::Init()
{
  for(unsigned ith = 0; ith < m_encoders.size(); ++ith)
    {
      m_curSWFlowInfo[m_encoders[ith]->GetID()] = FlowInfo_t();
    }
  Simulator::Schedule (Seconds(PERIOD), &FlowDecoder::DecodeFlows, this);
}

void
FlowDecoder::AddEncoder (Ptr<FlowEncoder> encoder)
{
  m_encoders.push_back(encoder);
}


Ptr<FlowEncoder>
FlowDecoder::GetEncoderByID (int swID)
{
  Ptr<FlowEncoder> e;
  for(unsigned ith = 0; ith < m_encoders.size(); ++ith)
    {
      e = m_encoders[ith];
      if( e->GetID() == swID)
	break;
    }

  return e;
}
  
  
//algorithm find_if predicate
bool IsFlowCountOne(const FlowEncoder::CountTableEntry& e)
{
  return (e.flow_cnt == (uint8_t)1);
}
void
FlowDecoder::FlowSingleDecode(Ptr<FlowEncoder> target)
{

  int swID = target->GetID();
  NS_LOG_INFO ("Decoding at "<< swID);

  FlowEncoder::CountTable_t&          countTable   = target->GetCountTable();
  FlowEncoder::CountTable_t::iterator itEntry;
  while( (itEntry = std::find_if(countTable.begin(), countTable.end(),
				 IsFlowCountOne))
	!= countTable.end() )
    {
      //Find a pure cell
      FlowField flow      = (*itEntry).GetFlow();
      
      //uint32_t  packetCnt = (*itEntry).packet_cnt;
      NS_LOG_INFO ("Flow: "<< flow );
      
      m_curSWFlowInfo[swID][flow] = 0;

      /* If it's a new flow doesn't collected among switches before,
       * add to m_passNewFlows
       */
      if (m_passNewFlows.find(flow) == m_passNewFlows.end())
	{
	  m_passNewFlows.insert(flow);
	}

      target->ClearFlowInCountTable(flow);
      
    }  
}

void
FlowDecoder::DecodeFlowOnPath (const Graph::Path_t& path, const FlowField& flow)
{
  NS_LOG_FUNCTION(flow);
  
  unsigned lst = path.size() - 1;
  for(unsigned ith = 0; ith < lst; ++ith)
    {
      int          swID          = path[ith].dst;
      FlowInfo_t&  swDcdFlowInfo = m_curSWFlowInfo.at (swID);
      
      
      FlowInfo_t::iterator itFlow;
      if ( (itFlow = swDcdFlowInfo.find (flow)) == swDcdFlowInfo.end() )
	{
	  /*This switch doesn't decoded the flow before, but the flow must go 
	   *through this switch. Check the sw's flowfilter to make sure that 
	   *the flow go through this sw. If so. Update the swtich's flow set 
	   *and CountTable.
	   */

	  NS_LOG_INFO (swID<<" doesn't decode this before");
	  
	  Ptr<FlowEncoder>  swEncoder = GetEncoderByID (swID);
	  if ( swEncoder->ContainsFlow(flow) )
	    {
	      swDcdFlowInfo [flow] = 0;
	      swEncoder->ClearFlowInCountTable (flow);
	    }
  
	}
      
    }
}

void
FlowDecoder::CounterSingleDecode (Ptr<FlowEncoder> target)
{

  NS_LOG_FUNCTION(target->GetID());
  
  int      swID   = target->GetID();
  const unsigned m      = COUNT_TABLE_SIZE; //Row
  const unsigned n      = m_curSWFlowInfo.at(swID).size();

  if(n == 0)
    {
      NS_LOG_INFO("No flow decoded");
    }

  double* A[m]; //Puppet
  double  b[m];
  //double  x[n];
  
  double  AA[m*n]; 
  for(unsigned i = 0; i < m; ++i)
    {
      A[i] = &(AA[i*n]);
      for(unsigned j = 0; j < n; ++j)
	{
	  A[i][j] = 0.0;
	}
    }

  ConstructLinearEquations (A, b, m, n, target);

}

void
FlowDecoder::ConstructLinearEquations (double* A[], double b[],
				       unsigned m,  unsigned n,
				       Ptr<FlowEncoder> target)
{
 
  int                        swID           = target->GetID();
  FlowInfo_t                &swDcdFlowInfo  = m_curSWFlowInfo.at (swID);
  FlowEncoder::CountTable_t &swCountTable   = target->GetCountTable();
  
  //No more flow will be inserted, so rehashing will not happen
  FlowInfo_t::const_iterator itFlow;
  unsigned col;
  for (col = 0, itFlow = swDcdFlowInfo.begin();
      itFlow != swDcdFlowInfo.end();
      ++col, ++itFlow)
    {
      const FlowField      &flow    = itFlow->first;
      std::vector<uint32_t> rowIdxs = target->GetCountTableIdx(flow);
      for (unsigned ith = 0; ith < rowIdxs.size(); ++ith)
	{
	  A[rowIdxs[ith]][col] = 1.0;
	}
    }

  NS_ASSERT(col == n && m == COUNT_TABLE_SIZE);
  
  for(unsigned row = 0; row < m; ++row)
    {
      b[row] = swCountTable[row].packet_cnt;
    }

  for(unsigned i = 0; i < m; ++i)
  {
    for(unsigned j = 0; j < n; ++j)
      {
	std::cout << A[i][j] << " ";
      }

    std::cout << "  " << b[i] << std::endl;
  }
  
}

}

