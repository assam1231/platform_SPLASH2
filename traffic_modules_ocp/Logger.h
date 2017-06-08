#ifndef LOGGER_H__
#define LOGGER_H__
#include <fstream>
#include <systemc>

#include "tlm"
#include "ocpip.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_get.h"

#include "TrafficParser.h"

namespace tinyxml2
{
	class XMLElement;
}

using namespace sc_core;

template<unsigned int DATA_WIDTH = 32>
class Logger: public sc_module
{
public:
	SC_HAS_PROCESS( Logger );
	typedef union word{
		int sint;
		unsigned int uint;
		unsigned char uc[4];
	} word;

	Logger( sc_module_name n, int id );
	~Logger();
	
	ocpip::ocp_slave_socket_tl3<DATA_WIDTH, 1> target_socket;
	
	void load_traffic( tinyxml2::XMLElement * xml_ele_ptr );
	int id() { return m_id; }

private:
  //--- Socket transport functions ---//
  void b_transport( tlm::tlm_generic_payload &trans, sc_core::sc_time &delay );
	tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload &trans, tlm::tlm_phase& phase, sc_time & delay);

  //--- Module process ---//
  void begin_resp_process();

private:
	void operate(tlm::tlm_generic_payload &trans);
  void do_read(tlm::tlm_generic_payload &trans);
  void do_write(tlm::tlm_generic_payload &trans);  
  void print_transaction
       (const tlm::tlm_generic_payload & trans, 
        const sc_core::sc_time delay=sc_core::sc_time(0, SC_NS));

private:
	const unsigned int MEM_SIZE; //in byte
	ocpip::ocp_parameters m_ocp_parameters; 
	int m_id;
  sc_core::sc_time m_read_latency;
  sc_core::sc_time m_write_latency;
	tinyxml2::XMLElement *m_slave_element;
	bool m_print;
	unsigned int m_transaction_count;
	std::ofstream log_file;
	tlm_utils::peq_with_get <tlm::tlm_generic_payload> m_resp_peq;
  unsigned char *m_memory;

};
#endif
