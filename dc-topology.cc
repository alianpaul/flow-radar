
#include <fstream>
#include <string>

#include "dc-topology.h"

#include "ns3/log.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mac48-address.h"
#include "ns3/address.h"
#include "ns3/openflow-switch-helper.h"

#include "openflow-switch-net-device.h"
#include "flow-encoder.h"
#include "flow-decoder.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DCTopology");

NS_OBJECT_ENSURE_REGISTERED(DCTopology);

TypeId
DCTopology::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::DCTopology")
    .SetParent<Object>()
    .SetGroupName("Openflow");

  return tid;
}


void
DCTopology::BuildTopo (const char* filename)
{
  NS_LOG_FUNCTION(this);

  std::ifstream file;
  file.open(filename);

  if(file.fail())
    {
      NS_LOG_ERROR("file can not open");
      return;
    }

  CreateNodes (file);
  
  CreateNetDevices (file);
  
  CreateOFSwitches ();

  CreateFlowRadar (); //for flow radar
  
  /* Set the internet stack,ATTENTION: must set internet stack after all
   * NetDevices installed, or, the switch port id will start by 1 which should 
   * be 0;
   */
  SetIPAddrAndArp ();

  Init();
}

void
DCTopology::Init()
{
  m_flowRadar->Init();
  m_easyController->SetTopo(Ptr<DCTopology>(this));
  m_easyController->SetDefaultFlowTable();
}

const Graph::AdjList_t&
DCTopology::GetAdjList() const
{
  
  return m_adjList;
}

unsigned
DCTopology::GetNumHost() const
{
  return m_numHost;
}

unsigned
DCTopology::GetNumSW() const
{
  return m_numSw;
}

Ipv4Address
DCTopology::GetHostIPAddr(int hostID) const
{
  return m_hostIPInterface.GetAddress ( hostID );
}

Address
DCTopology::GetHostMacAddr(int hostID) const
{
  return m_hostDevices.Get(hostID)->GetAddress();
}

Ptr<Node>
DCTopology::GetHostNode(int hostID) const
{
  return m_hostNodes.Get(hostID);
}

int
DCTopology::GetHostID(uint32_t ipv4Addr) const
{
  /*Hacking: Need a mapping between IP addr and Host ID
   *Right now, we just induce the Host ID from the Ipv4Addr directly
   */
  return ipv4Addr & 0xff ;
}

Ptr<OpenFlowSwitchNetDevice>
DCTopology::GetOFSwtch(int SWID) const
{
  SWID -= m_numHost;
  Ptr<OpenFlowSwitchNetDevice> sw = DynamicCast<OpenFlowSwitchNetDevice, NetDevice> (m_OFSwtchDevices.Get(SWID));
  
  return sw; 
}


void
DCTopology::CreateNodes (std::ifstream& file)
{
  NS_LOG_FUNCTION(this);
  std::string key;
  file >> key >> m_numHost;
  NS_LOG_INFO (key <<" "<<m_numHost);
  file >> key >> m_numSw;
  NS_LOG_INFO (key <<" "<< m_numSw);
    
  m_hostNodes.Create ( m_numHost );
  m_switchNodes.Create ( m_numSw );

  //Init the container size;
  m_adjList.resize( m_numSw  + m_numHost );
  m_switchPortDevices.resize (m_numSw);
  
  return;
}

void
DCTopology::CreateNetDevices (std::ifstream& file)
{
  NS_LOG_FUNCTION(this);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  int src, dst;
  while(file >> src >> dst)
    {
     
      int idFrom, idTo;
      NetDeviceContainer csmaDevicesFT; 
      if(src < m_numHost)
	{
	  idFrom = src;
	  idTo   = dst - m_numHost;
	  csmaDevicesFT = csma.Install (NodeContainer(m_hostNodes.Get(idFrom),
						      m_switchNodes.Get(idTo)));
	  m_hostDevices.Add (csmaDevicesFT.Get(0));
	}
      else
	{
	  idFrom = src - m_numHost;
	  idTo   = dst - m_numHost;
	  csmaDevicesFT = csma.Install (NodeContainer(m_switchNodes.Get(idFrom),
				                      m_switchNodes.Get(idTo)));
	  m_switchPortDevices[idFrom].Add(csmaDevicesFT.Get(0));
	}

      m_switchPortDevices[idTo].Add(csmaDevicesFT.Get(1));

      NS_LOG_INFO ("id: " << src << " port: "
		    << csmaDevicesFT.Get(0)->GetIfIndex() << " addr: "
		    << csmaDevicesFT.Get(0)->GetAddress() );
      
      NS_LOG_INFO ("id: " << dst << " port: "
		    << csmaDevicesFT.Get(1)->GetIfIndex() << " addr:"
		    << csmaDevicesFT.Get(1)->GetAddress() );

      Graph::AdjNode_t adjNode;
      adjNode.from_port = csmaDevicesFT.Get(0)->GetIfIndex();
      adjNode.to_port   = csmaDevicesFT.Get(1)->GetIfIndex();
      adjNode.id        = dst;
      adjNode.weight    = 1;
      m_adjList[src].push_back(adjNode);

      adjNode.from_port = csmaDevicesFT.Get(1)->GetIfIndex(); 
      adjNode.to_port   = csmaDevicesFT.Get(0)->GetIfIndex();
      adjNode.id        = src;
      adjNode.weight    = 1;
      m_adjList[dst].push_back(adjNode);     
      
    }

  csma.EnablePcapAll("swtch", false);
   
}

void
DCTopology::CreateOFSwitches ()
{
  NS_LOG_FUNCTION(this);

  m_easyController = CreateObject<ofi::EasyController>();
  
  OpenFlowSwitchHelper ofSwtch;
  for(int idSW = 0; idSW < m_numSw; ++idSW)
    {
      m_OFSwtchDevices.Add (ofSwtch.Install (m_switchNodes.Get(idSW),
					     m_switchPortDevices[idSW],
					     m_easyController ));
      
      NS_LOG_LOGIC ( "OFSW"<< idSW <<" MacAddr: "<<
		     m_OFSwtchDevices.Get(idSW)->GetAddress() << " Port: " <<
		     m_OFSwtchDevices.Get(idSW)->GetIfIndex() );
    }
  
}

void
DCTopology::CreateFlowRadar ()
{
  NS_LOG_FUNCTION(this);

  m_flowRadar = CreateObject<FlowDecoder>(Ptr<DCTopology>(this));
  
  for(int idSW = 0; idSW < m_numSw; ++idSW)
    {
      Ptr<FlowEncoder> flowEncoder = CreateObject<FlowEncoder> ();
      flowEncoder->SetOFSwtch( m_OFSwtchDevices.Get(idSW), idSW + m_numHost);
      
      m_flowRadar->AddEncoder(flowEncoder);
      
    }

}

void
DCTopology::SetIPAddrAndArp()
{

  NS_LOG_FUNCTION(this);
  
  /*Install the internet stack on all nodes*/
  InternetStackHelper internetstack;
  internetstack.Install (m_hostNodes);
  internetstack.Install (m_switchNodes);
  
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");

  /*Set IP Addresses*/
  m_hostIPInterface    = ipv4.Assign(m_hostDevices);
  m_OFSwtchIPInterface = ipv4.Assign(m_OFSwtchDevices);

  
  for(int id_local = 0; id_local < m_numHost; ++id_local)
    {
      //Set ARP cache of the local hosts
      std::pair<Ptr<Ipv4>, uint32_t> itfLocal = m_hostIPInterface.Get(id_local);
      Ptr<Ipv4L3Protocol> ippLocal = DynamicCast<Ipv4L3Protocol, Ipv4>(itfLocal.first);
      Ptr<Ipv4Interface>  ipiLocal = ippLocal->GetInterface(itfLocal.second);
      Ptr<ArpCache>       cacheLocal = ipiLocal->GetArpCache ();
      
      for(int id_remote = 0; id_remote < m_numHost; ++id_remote)
	{
	  if(id_local == id_remote) continue;
	  Ipv4Address ipAddrRemote  = GetHostIPAddr (id_remote);
	  Address     macAddrRemote = GetHostMacAddr (id_remote);
	  
	  ArpCache::Entry *entry = cacheLocal->Add (ipAddrRemote);
	  entry->SetMacAddresss (macAddrRemote);
	  entry->MarkPermanent ();
	  
	}
      /*
      NS_LOG_LOGIC("id: " << id_local);
      Ptr<OutputStreamWrapper> osw = Create<OutputStreamWrapper> (&std::cout);
      cacheLocal->PrintArpCache (osw);
      */
    }
  
  
}


  
DCTopology::DCTopology()
{
  NS_LOG_FUNCTION (this);
}

DCTopology::~DCTopology()
{
  NS_LOG_FUNCTION (this);
}

}
