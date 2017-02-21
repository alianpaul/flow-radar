#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <stdint.h>
#include <limits>
#include <vector>
#include <list>

namespace ns3 {

class Graph
{
  
public:
  /*The Adjacent List Node;
   */
  struct AdjNode_t
  {
    uint32_t from_port;
    uint32_t to_port;
    int      id;
    int      weight;
  };
  typedef std::vector<AdjNode_t>       AdjListEntry_t;
  typedef std::vector<AdjListEntry_t>  AdjList_t;

  /*PathTable Entry Type
   */
#define MAX_DIST std::numeric_limits<int>::max()
  
  struct PathTableEntry_t
  {
    PathTableEntry_t():
      known(false),
      dist(MAX_DIST),
      before(0)
    {
    }
    bool known;
    int  dist;
    int  before;
  };
  typedef std::vector<PathTableEntry_t>  PathTable_t;

  struct Edge_t 
  {
    int src;
    int dst;
    int spt;
    int dpt;
  };

  typedef std::vector<Edge_t> Path_t;
    
  Graph(const AdjList_t& adjList);
  
  ~Graph();

  /*Build all nodes' Path table.
   */
  void        BuildPaths();

  Path_t      GetPath (int from, int to) const;
  
private:


  /*Use Dijkstra algo to compute the short path from node root to other nodes, 
   *store the paths in root's path table.
   */
  void Dijkstra (int root);

  /*Find the unknown node in root's pathTable with the shortest smallest dist,
   *If the founded unknown node's dist == MAX_DIST, we'll say that this node is 
   *not exist.
   */
  int  FindSmallestUnknown (PathTable_t& pathTable);
  
  const AdjList_t          m_adjList;
  std::vector<PathTable_t> m_paths;
  int                      m_numNodes;
};

}

#endif