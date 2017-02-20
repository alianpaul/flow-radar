
#include <iostream>

#include "easy-controller.h"
#include "openflow-switch-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EasyController");
//NS_OBJECT_ENSURE_REGISTERED (EasyController);
namespace ofi {

TypeId
EasyController::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ofi::EasyController")
    .SetParent<Controller> ()
    .SetGroupName("Openflow")
    .AddConstructor<EasyController> ()
    ;

  return tid;
}

void
EasyController::ReceiveFromSwitch (Ptr<OpenFlowSwitchNetDevice> swtch, ofpbuf* buffer)
{
  return;
}
  
void
EasyController::SetTopo (Ptr<DCTopology> topo)
{
  NS_LOG_FUNCTION(this);
  m_topo = topo;
}

void
EasyController::SetDefaultFlowTable ()
{

  NS_LOG_FUNCTION(this);
  
  const DCTopology::AdjList_t& adjList = m_topo->GetAdjList();

  
     
}

std::vector<int>
EasyController::Dijkstra (const DCTopology::AdjList_t& adjList, int from, int to)
{
  std::vector<int> dist(adjList.size(), std::numeric_limits<int>::max());
  std::vector<int> slct;
  std::vector<int> lft;
  for(int = 0; i < adjList.size(); ++i )
    {
      lft[i] = i;
    }

 
  dist[from] = 0;

   
}
  
void
EasyController::ProactiveModFlow (Ptr<OpenFlowSwitchNetDevice> swtch,
				  uint16_t command,
				  uint16_t out_port, uint16_t in_port,
				  Address mac_src, Address mac_dst,
				  Ipv4Address ip_src, Ipv4Address ip_dst,
				  uint16_t port_src, uint16_t port_dst)
{
  //Create the matching key
  sw_flow_key key;
  key.wildcards = 0;
  FlowExtract(&key.flow, in_port, mac_src, mac_dst, ip_src, ip_dst, port_src, port_dst);

  //Create the output-to-port action
  ofp_action_output x[1];
  x[0].type = htons (OFPAT_OUTPUT);
  x[0].len  = htons (sizeof(ofp_action_output));
  x[0].port = out_port;

  //buffer_id does not exist, so set it to -1
  ofp_flow_mod * ofm = BuildFlow (key, -1, command, x, sizeof(x), OFP_FLOW_PERMANENT, OFP_FLOW_PERMANENT);
  
  SendToSwitch (swtch, ofm, ofm->header.length);  
}


void
EasyController::FlowExtract (struct flow * flow_in_key,
			     uint16_t in_port,
			     Address mac_src, Address mac_dst,
			     Ipv4Address ip_src, Ipv4Address ip_dst,
			     uint16_t port_src, uint16_t port_dst)
{

  memset(flow_in_key, 0, sizeof(*flow_in_key));


  //Input VLAN vlan disable openflow.h
  flow_in_key->dl_vlan = htons (OFP_VLAN_NONE);
  //Input switch port
  flow_in_key->in_port = htons (in_port);


  //Ethernet frame type: Ethernet IP (No Ethernet Arp, because we set the
  //host's cache permanently). So we need to set nw_src, nw_dst.
  flow_in_key->dl_type = htons (ETH_TYPE_IP);
  //Ethernet address.
  mac_src.CopyTo (flow_in_key->dl_src);
  mac_dst.CopyTo (flow_in_key->dl_dst);
  //MAH: start MPLS_INVALID_LABEL private\packet.h
  flow_in_key->mpls_label1 = htonl (MPLS_INVALID_LABEL);
  flow_in_key->mpls_label2 = htonl (MPLS_INVALID_LABEL);

  
  //IP source address
  flow_in_key->nw_src = htonl (ip_src.Get ());
  flow_in_key->nw_dst = htonl (ip_dst.Get ());
  //IP protocal
  //Ipv4Header::GetProtocol return 8
  flow_in_key->nw_proto = 8;

  //TCP UDP port
  flow_in_key->tp_src = htons (port_src);
  flow_in_key->tp_dst = htons (port_dst);

  return;
  
}

  
/*
void
EasyController::SetAdjMtx (DCTopology::AdjMatrix_t adjMtx)
{
  NS_LOG_FUNCTION(this);

  m_adjMtx = adjMtx;
}
*/

    
}

}
