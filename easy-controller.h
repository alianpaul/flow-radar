/* easy controller
 * set the flow table entry of its's registered OpenFlowSwitchNetDevices
 */

#ifndef EASY_CONTROLLER_H
#define EASY_CONTROLLER_H

#include "openflow-interface.h"
#include "dc-topology.h"

namespace ns3 {

namespace ofi {
  
class EasyController : public Controller
{

public:
  static TypeId GetTypeId (void);

  /*Inherit from Controller
   *
   */
  void ReceiveFromSwitch (Ptr<OpenFlowSwitchNetDevice> swtch, ofpbuf* buffer);

  
  void SetTopo (Ptr<DCTopology> topo);
  
  /* Proactively modify the flow entry on the OpenFlowSwitch' flow  
   * table with the OFPT_FLOW_MOD message, the actual type of the flow modification
   * is set by @param command.The OFPT_FLOW_MOD msg is built by call BuildFlow() f
   * unction. 
   *   
   * @swtch:    the openflowswitch we config on to.
   * @command:  the flow action: OFPFC_ADD(add a flow entry), OFPFC_MODIFY, OFPFC_M
   *           DIFY_STRICT, OFPFC_DELETE, OFPFC_DELETE_STRICT.
   * @out_port: the outport of the action.
   * ----------flow param---------------
   * @
   */ 
  void ProactiveModFlow (Ptr<OpenFlowSwitchNetDevice> swtch, uint16_t command,
			 uint16_t out_port, uint16_t in_port,
			 Address mac_src, Address mac_dst,
			 Ipv4Address ip_src, Ipv4Address ip_dst,
			 uint16_t port_src, uint16_t port_dst);
  

  /* Use Dijkstra algorithm with the adj list to compute the shortest path
   * betewn each hosts. Set the switches's flow table along the path.
   *
   */
  void SetDefaultFlowTable ();

  

  
private:
  /* Extract the 5 tuple(flow info) to the open switch flow struct.
   */
  void FlowExtract (struct flow * flow_in_key,
		    uint16_t in_port,
		    Address mac_src, Address mac_dst,
		    Ipv4Address ip_src, Ipv4Address ip_dst,
		    uint16_t port_src, uint16_t port_dst);

  std::vector<int> Dijkstra(const DCTopology::AdjList_t& adjList, int from, int to);

  Ptr<DCTopology>  m_topo; //the pre read and configed network topo
  
  
};

  
}
}

#endif
