
#include <iostream>

#include "graph-algo.h"

#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("Graph");

namespace ns3 {

/*Use Dijkstra algorithm to calculate the shortest path 
 *and store each node's path to its path table;
 */
  
Graph::Graph (const AdjList_t& adjList)
  : m_adjList(adjList),
    m_numNodes(adjList.size()) 
{
  NS_LOG_LOGIC(this);

  //Prepare the path table's size
  m_paths.resize (m_numNodes);
  for(int i = 0; i < m_numNodes; i++)
    {
      m_paths[i].resize(m_numNodes);
      m_paths[i][i].dist    = 0;
    }   
}

void
Graph::BuildPaths ()
{
  NS_LOG_FUNCTION(this);
  
  for(int i = 0; i < m_numNodes; ++i)
    {
      Dijkstra(i);
    }
}

Graph::Path_t
Graph::GetPath (int from, int to) const
{
  const PathTable_t & pathTable = m_paths[from];
        Path_t        path;
  
  for(int i = to; i != from; i = pathTable[i].before)
    {
      Edge_t    edge;
      edge.src = pathTable[i].before;
      edge.dst = i;

      const AdjListEntry_t & adjNodes = m_adjList[edge.src];
      AdjNode_t dstNode;
      //find the dst node in src's adjlist
      for (unsigned adjid = 0; adjid < adjNodes.size(); ++adjid)
	{
	  if ( adjNodes[adjid].id == edge.dst )
	    {
	      dstNode = adjNodes[adjid];
	      break;
	    }
	}
      
      edge.spt = dstNode.from_port;
      edge.dpt = dstNode.to_port;

      path.push_back (edge); 
    }

  //reverse the path;
  Path_t pathRev;
  for(int i = path.size() - 1; i >= 0; --i)
    {
      pathRev.push_back( path[i] );
    }
  return pathRev;
}

void
Graph::Dijkstra(int root)
{
  //NS_LOG_LOGIC (i << " path table");
  PathTable_t& pathTable = m_paths[root];
  while(1)
    {
      //next node to add into the known set; 
      int next = FindSmallestUnknown (pathTable);
      if( next  == -1 )
	{
	  break;
	}
	
      pathTable[next].known = true;

      //for each next's adjcent node.
      for(unsigned i = 0; i < m_adjList[next].size(); ++i)
	{
	  int adj    = m_adjList[next][i].id;
	  int weight = m_adjList[next][i].weight;
	    
	  if ( pathTable[next].dist + weight < pathTable[adj].dist)
	    {
	      pathTable[adj].before = next;
	      pathTable[adj].dist = pathTable[next].dist + weight;
	    }
	}
    }

  /*
  for(unsigned i = 0; i < pathTable.size(); i++)
    {
      std::cout << i <<" K "<< pathTable[i].known << " D " << pathTable[i].dist
		<<" B "<< pathTable[i].before << std::endl;
    }
  */
}

int
Graph::FindSmallestUnknown(PathTable_t& pathTable)
{
  int v;
  int dist_s = MAX_DIST;
  for (unsigned i = 0; i < pathTable.size(); ++i)
    {
      if( !pathTable[i].known &&  pathTable[i].dist < dist_s )
	{
	  v = i;
	  dist_s = pathTable[i].dist;
	}
    }

  if (dist_s == MAX_DIST) return -1;
  return v;
}
  
Graph::~Graph()
{
  NS_LOG_LOGIC(this);
}



  
}
