
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
DCTopology::BuildTopo (const char* filename, Ptr<ns3::ofi::Controller> controller)
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
  
  CreateOFSwitches (controller);
  
  SetIPAddrAndArp ();
}

const DCTopology::AdjList_t&
DCTopology::GetAdjList() const
{
  NS_LOG_FUNCTION(this);
  
  return m_adjList;
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
  
  /*Install the internet stack on all nodes*/
  InternetStackHelper internetstack;
  internetstack.Install (m_hostNodes);
  internetstack.Install (m_switchNodes);
  
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
      NS_LOG_LOGIC(src<<" "<<dst);
      
      if(src < m_numHost) 
	{
	  //link from host to switch
	  int idH  = src;
	  int idSW = dst - m_numHost;

	  NetDeviceContainer csmaDevicesHS;
	  csmaDevicesHS = csma.Install(NodeContainer(m_hostNodes.Get(idH),
						     m_switchNodes.Get(idSW)));
	  m_hostDevices.Add (csmaDevicesHS.Get(0));
	  m_switchPortDevices[idSW].Add (csmaDevicesHS.Get(1));

	  NS_LOG_LOGIC ( csmaDevicesHS.Get(0)->GetIfIndex() << " " <<
			 csmaDevicesHS.Get(0)->GetAddress() );
	  NS_LOG_LOGIC ( csmaDevicesHS.Get(1)->GetIfIndex() << " " <<
			 csmaDevicesHS.Get(1)->GetAddress() );

	  AdjNode adjNode;
	  adjNode.from_port = csmaDevicesHS.Get(0)->GetIfIndex();
	  adjNode.to_port   = csmaDevicesHS.Get(1)->GetIfIndex();
	  adjNode.id        = dst;
	  adjNode.weight    = 1;
	  m_adjList[src].push_back(adjNode);

	  adjNode.from_port = csmaDevicesHS.Get(1)->GetIfIndex(); 
	  adjNode.to_port   = csmaDevicesHS.Get(0)->GetIfIndex();
	  adjNode.id        = src;
	  adjNode.weight    = 2;
	  m_adjList[dst].push_back(adjNode);
	}
      else
	{
	  //link from switch to switch

	  int idSW1 = src - m_numHost;
	  int idSW2 = dst - m_numHost;

	  NetDeviceContainer csmaDevicesSS;
	  csmaDevicesSS = csma.Install(NodeContainer(m_switchNodes.Get(idSW1),
						     m_switchNodes.Get(idSW2)));
	  m_switchPortDevices[idSW1].Add (csmaDevicesSS.Get(0));
	  m_switchPortDevices[idSW2].Add (csmaDevicesSS.Get(1));

	  NS_LOG_LOGIC ( csmaDevicesSS.Get(0)->GetIfIndex() << " " <<
			 csmaDevicesSS.Get(0)->GetAddress() );
	  NS_LOG_LOGIC ( csmaDevicesSS.Get(1)->GetIfIndex() << " " <<
			 csmaDevicesSS.Get(1)->GetAddress() );

	  AdjNode adjNode;
	  adjNode.from_port = csmaDevicesSS.Get(0)->GetIfIndex();
	  adjNode.to_port   = csmaDevicesSS.Get(1)->GetIfIndex();
	  adjNode.id        = dst;
	  adjNode.weight    = 1;
	  m_adjList[src].push_back(adjNode);

	  adjNode.from_port = csmaDevicesSS.Get(1)->GetIfIndex(); 
	  adjNode.to_port   = csmaDevicesSS.Get(0)->GetIfIndex();
	  adjNode.id        = src;
	  adjNode.weight    = 1;
	  m_adjList[dst].push_back(adjNode);
	}

    }
   
}

void
DCTopology::CreateOFSwitches (Ptr<ns3::ofi::Controller> controller)
{
  NS_LOG_FUNCTION(this);

  OpenFlowSwitchHelper ofSwtch;
  for(int idSW = 0; idSW < m_numSw; ++idSW)
    {
      m_OFSwtchDevices.Add (ofSwtch.Install (m_switchNodes.Get(idSW),
					     m_switchPortDevices[idSW],
					     controller ));
      
      NS_LOG_LOGIC ( "OFSW"<< idSW <<" MacAddr: "<<
		     m_OFSwtchDevices.Get(idSW)->GetAddress() << " Port: " <<
		     m_OFSwtchDevices.Get(idSW)->GetIfIndex() );
    }
}

void
DCTopology::SetIPAddrAndArp()
{
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
	  Ipv4Address ipAddrRemote  = m_hostIPInterface.GetAddress(id_remote);
	  Address     macAddrRemote = m_hostDevices.Get(id_remote)->GetAddress();
	  
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
