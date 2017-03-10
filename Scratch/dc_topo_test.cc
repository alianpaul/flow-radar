
#include "ns3/openflow-module.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/log.h"

#include <iostream>


using namespace ns3;

int main(int argc, char* argv[])
{
  //LogComponentEnable ("DCTopology", LOG_LEVEL_ALL);
  //LogComponentEnable ("EasyController", LOG_LEVEL_ALL);
  //LogComponentEnable ("Graph", LOG_LEVEL_ALL);
  //LogComponentEnable ("OpenFlowSwitchNetDevice", LOG_LEVEL_INFO);
  //LogComponentEnable ("OpenFlowInterface", LOG_LEVEL_INFO);
  //LogComponentEnable ("FlowEncoder", LOG_LEVEL_INFO);
  //LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  //LogComponentEnable ("OnOffApplication",LOG_LEVEL_INFO);
  LogComponentEnable ("FlowDecoder", LOG_LEVEL_ALL);

  
  Ptr<DCTopology> topo   = CreateObject<DCTopology>();
  /* 1 : pcap
   * 2 : ascii
   */
  //topo->BuildTopo ("test_topo.txt", 2);
  topo->BuildTopo("fattree_k4.txt", 2);
  
  Ptr<AppGen>     appGen = CreateObject<AppGen>(topo);

  /*
  appGen->GenUDPFlow(0, 3, 10, 512*10, 0.0,  0.5);
  appGen->GenUDPFlow(0, 3, 16, 512*9,  0.01, 0.5);
  appGen->GenUDPFlow(3, 2, 18, 512*13, 0.02, 0.5);
  appGen->GenUDPFlow(2, 1, 20, 512*5,  0.03, 0.5);
  */

  appGen->GenRandomUDPFlow (10000);
  
  Simulator::Run();
  Simulator::Destroy();
  
}
