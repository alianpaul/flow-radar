/*
 * Build the topology, Also work as a container to store the created nodes
 * , csmaNetDevices, openswitchNetDevices, Edges   
 */

#ifndef DC_TOPOLOGY_H
#define DC_TOPOLOGY_H

#include <iosfwd>
#include <vector>

#include "ns3/object.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/ipv4-interface-container.h"

#include "openflow-interface.h"
#include "graph-algo.h"


namespace ns3 {

class FlowDecoder;
  
class DCTopology : public Object {
  
public:
  
  
  //==========================Public Interface=================
  
  static TypeId GetTypeId( void );

  DCTopology();
  virtual ~DCTopology();

    
  /*Build the Nodes, NetDevices according to the Topofile
   *OpenFlowNetDevices and Controller are excluded.
   */
  void BuildTopo (const char* filename, Ptr<ns3::ofi::Controller> controller);
  
  const Graph::AdjList_t&      GetAdjList () const;
  unsigned                     GetNumHost () const;
  unsigned                     GetNumSW   () const;
  Ipv4Address                  GetHostIPAddr  (int hostID) const;
  Address                      GetHostMacAddr (int hostID) const;
  Ptr<Node>                    GetHostNode    (int hostID) const;
  //TODO: GetHostID need a more concrete version.
  int                          GetHostID      (uint32_t ipv4Addr) const; 
  Ptr<OpenFlowSwitchNetDevice> GetOFSwtch (int SWID) const;

private:
  DCTopology(const DCTopology&);
  DCTopology& operator=(const DCTopology&);
  
  /*Create ns3 Nodes in the NodeContainer according to the Topofile.
   *Also intall internet stack on all nodes
   *Called by BuildTopo
   *file: input topo file
   */
  void CreateNodes (std::ifstream& file);

  /*Create ns3 port NetDevices on the nodes(host and swtiches)
   *Create the adjcent list of the topo;
   *Called by BuildTopo
   */
  void CreateNetDevices (std::ifstream& file);

  /*Create ns3 OpenFlowNetDevices(openflowswitches) with the provided
   *switch nodes and swtich port netdevices
   *Called by BuildTopo
   */
  void CreateOFSwitches (Ptr<ns3::ofi::Controller> controller);

  /* Create FlowRadar
   * Install a flow encoder on each openflow switch.
   *   Call flow encoder's SetOFSwtch function register the openflow swtich's
   *   promisc receive call back function.
   * Register each flow encoder to a central flow decoder.
   */
  void CreateFlowRadar ();

  /* Set IP addresses of the hosts and openflow switches
   * and also set ARP cache permantly, because this simulation is focused on
   * ethernet IP packetets. We don't want any ethernet ARP broadcast packets flow 
   * in the network. In this way, it's easy to set flow tables of the switch.
   */
  void SetIPAddrAndArp ();
    
  int                             m_numHost; 
  NodeContainer                   m_hostNodes;
  NetDeviceContainer              m_hostDevices;
  Ipv4InterfaceContainer          m_hostIPInterface; //host ip interfaces
  
  int                             m_numSw;
  NodeContainer                   m_switchNodes;
  NetDeviceContainer              m_OFSwtchDevices;
  Ipv4InterfaceContainer          m_OFSwtchIPInterface; //OFSW ip interfaces
  std::vector<NetDeviceContainer> m_switchPortDevices;

  Ptr<FlowDecoder>                m_flowRadar;

  Graph::AdjList_t                m_adjList;  
};

  
}

#endif
