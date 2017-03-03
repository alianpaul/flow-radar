#ifndef FLOW_DECODER_H
#define FLOW_DECODER_H

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include "ns3/object.h"

#include "flow-field.h"
#include "dc-topology.h"
#include "graph-algo.h"

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

  /* Get encoder by swID
   */
  Ptr<FlowEncoder>  GetEncoderByID(int swID);
  
  /* 
   */
  void SingleDecode     (Ptr<FlowEncoder> target);
  
  /* Update the sw' m_curSWFlowInfo on the path
   */
  void DecodeFlowOnPath (const Graph::Path_t& path, const FlowField& flow);

  
  std::vector<Ptr<FlowEncoder> >  m_encoders;
  /* Flow decoded on single swtches in this frame
   */
  SWFlowInfo_t                    m_curSWFlowInfo;
  /* The new decoded in a pass(single decode on all sw)
   */
  FlowSet_t                       m_passNewFlows;  
  Ptr<DCTopology>                 m_topo;
};

}

#endif
