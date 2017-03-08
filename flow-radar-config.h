#ifndef FLOW_RADAR_CONFIG_H
#define FLOW_RADAR_CONFIG_H

namespace ns3
{

/*************Flow radar config*****************/
/*Flow Decoder config
 */
static const float PERIOD   = 10; //10ms
static const float END_TIME = 0.0;  //10s

/*Flow Encoder config
 *The count table entry has been divided into NUM_COUNT_HASH sections.
 *Each section has COUNT_TABLE_SUB_SIZE entries.
 */
static const int NUM_COUNT_HASH   = 4;      //num of counter table hash function
static const int COUNT_TABLE_SIZE = 8000;     //num of counter table total entries
static const int COUNT_TABLE_SUB_SIZE = 2000;  //num of entries in a section of ct
  
static const int NUM_FLOW_HASH = 27;        //num of flow filter hash function
static const int FLOW_FILTER_SIZE = 432752;  //num of bits of flow filter

}
#endif
