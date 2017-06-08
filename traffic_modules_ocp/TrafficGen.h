#ifndef TRAFFIC_GEN_H_
#define TRAFFIC_GEN_H_

#include <fstream>
#include <map>
#include <systemc>
using namespace sc_core;
#include "tlm"
#include "ocpip.h"
#include "tlm_utils/peq_with_get.h"

#include "TrafficParser.h"
#include "tinyxml2.h"
#include <iostream>
using namespace std;

namespace tinyxml2
{
	class XMLElement;
}

enum TrafficGenMode { BLOCKING_MODE, NON_BLOCKING_MODE };

enum previous_phase_enum { RECEIVED_UPDATED, RECEIVED_ACCEPTED, RECEIVED_END_REQ };
typedef std::map<tlm::tlm_generic_payload*,previous_phase_enum> waiting_bw_path_map;

template<unsigned int DATA_WIDTH = 32>
class TrafficGen: public sc_core::sc_module
{
public:
	typedef union word{
		int sint;
		unsigned int uint;
		unsigned char uc[4];
	} word;

public:
	SC_HAS_PROCESS(TrafficGen);

	//initiator socket
	ocpip::ocp_master_socket_tl3<DATA_WIDTH, 1>  init_socket;
	
	//constructor
	TrafficGen( sc_core::sc_module_name name, int id, TrafficGenMode mode );
	~TrafficGen();

	void load_traffic( tinyxml2::XMLElement * xml_ele_ptr );

	int id();
    
    int complete;
    
    std::map <tlm::tlm_generic_payload*,sc_core::sc_time> mapping;

private:
  //--- Socket callback functions ---//
	tlm::tlm_sync_enum nb_transport_bw 
		(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay);

  //--- Module processes ---//
	void generate_traffic_b();
	void generate_traffic_nb();

private:
    //--- Handy methods ---//
    void setup_payload(tlm::tlm_generic_payload *trans, const tinyxml2::XMLElement *action);
    void check_nb_fw_error_response(tlm::tlm_generic_payload &trans);
	void generate_and_set_data( tlm::tlm_generic_payload* trans, MasterDataMode data_mode, const tinyxml2::XMLElement * action );
	void test_gen_wait( const tinyxml2::XMLElement * action );
	void test_gen_wait_until( const tinyxml2::XMLElement * action );
	sc_core::sc_time_unit query_time_unit( const tinyxml2::XMLElement * action );


private:
  //--- Private fields ---//
	const unsigned int MAX_DATA_LENGTH; //in byte
	int m_id;
    sc_core::sc_event m_resp_event;
	tinyxml2::XMLElement * m_xml_element;
	MasterDataMode m_data_mode;
	std::ofstream m_log_file;
	unsigned int m_transaction_count;
	ocpip::ocp_parameters m_ocp_parameters;
    sc_core::sc_time  time;
    sc_core::sc_time  send_time;
    sc_core::sc_time  back_time;
    sc_core::sc_time  request_time;
    unsigned int forward_addr;
    unsigned int back_addr;
    unsigned int addr;

};

#endif
