#ifndef APP_GEN_H
#define APP_GEN_H

#include "ns3/object.h"

namespace ns3{

class DCTopology;
class PacketGenerator;
  
class  AppGen : public Object
{
  
public:

  static const int BULK_APP   = 0; //TCP Bulk Application
  static const int ONOFF_APP  = 1; //UDP ON OFF Application
  
  AppGen(Ptr<DCTopology> topo);
  virtual ~AppGen();

  /*
  void GenFlow(int appType, int src, int dst, uint16_t port, uint32_t maxBytes,
	       float startTime, float stopTime);
  */
  
  /* Generate packets according to the packet trace
   */
  void SetPacketTrace(const char* filename, float endTime);

  void GenRandomUDPFlow (int flowCnt);
  
  void GenTCPFlow(int src, int dst, uint16_t port, uint32_t maxBytes,
		  float startTime);
  
  void GenUDPFlow(int src, int dst, uint16_t port, uint32_t maxBytes,
		  float startTime, float stopTime);

private:
  AppGen(const AppGen&);
  AppGen& operator=(const AppGen&);

  Ptr<DCTopology>                     m_topo;
  std::vector<Ptr<PacketGenerator> >  m_packetGenerators;
};

}

#endif
