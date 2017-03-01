#ifndef FLOW_DECODER_H
#define FLOW_DECODER_H

#include "ns3/object.h"

#include "flow-encoder.h"

namespace ns3
{

class FlowEncoder;

class FlowDecoder : public Object
{
public:
  
  FlowDecoder ();
  virtual ~FlowDecoder ();

  void AddEncoder (Ptr<FlowEncoder> encoder);

  void DecodeFlows ();

  /* Schedule the decode event
   */
  void Init ();
  
private:
  FlowDecoder(const FlowDecoder&);
  FlowDecoder& operator=(const FlowDecoder&);

  typedef std::map<FlowEncoder::FlowField, uint32_t>   FlowInfo_t;
  //k: flow field, v: packet count
  typedef std::map<int, FlowInfo_t>                    NodeFlowInfo_t;
  //k: switch node id,  v: Flow info on this node

  bool SingleDecode (Ptr<FlowEncoder> target, NodeFlowInfo_t& nodeFlowInfo);

  std::vector<Ptr<FlowEncoder> > m_encoders; 
};

}

#endif
