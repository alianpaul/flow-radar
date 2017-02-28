#ifndef APP_GEN_H
#define APP_GEN_H

#include "ns3/object.h"

namespace ns3{

class Node;
class DCTopology;
  
class  AppGen : public Object
{
  
public:
  AppGen(Ptr<DCTopology> topo);
  virtual ~AppGen();
  /*
   * Simply use the bulk-send-application.
   */
  void GenTCPFlow(int src, int dst, uint16_t port, uint32_t maxBytes,
		  float startTime);
  
  void GenUDPFlow(int src, int dst, uint16_t port, uint32_t maxBytes,
		  float startTime, float stopTime);

private:
  AppGen(const AppGen&);
  AppGen& operator=(const AppGen&);
  
  Ptr<DCTopology> m_topo;
};

}

#endif
