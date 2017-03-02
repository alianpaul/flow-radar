#ifndef FLOW_DECODER_H
#define FLOW_DECODER_H

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include "ns3/object.h"

#include "flow-field.h"
#include "dc-topology.h"

namespace ns3
{

class FlowEncoder;

class FlowDecoder : public Object
{
public:
  
  FlowDecoder (Ptr<DCTopology> topo);
  virtual ~FlowDecoder ();

  void AddEncoder (Ptr<FlowEncoder> encoder);

  void DecodeFlows ();

  /* Schedule the decode event
   */
  void Init ();
  
private:
  FlowDecoder(const FlowDecoder&);
  FlowDecoder& operator=(const FlowDecoder&);

  typedef boost::unordered_map<FlowField, uint32_t, FlowFieldBoostHash>  FlowInfo_t;
  //k: flow key,      v: packet count
  typedef std::map<int, FlowInfo_t>                           SWFlowInfo_t;
  //k: switch node id,  v: Flow info on this node
  typedef boost::unordered_set<FlowField, FlowFieldBoostHash> FlowSet_t;
  
  void SingleDecode (Ptr<FlowEncoder> target);

  std::vector<Ptr<FlowEncoder> >  m_encoders;
  SWFlowInfo_t                    m_curSWFlowInfo;
  FlowSet_t                       m_passNewFlows;  //the new decoded in this pass
  Ptr<DCTopology>                 m_topo;
};

}

#endif
