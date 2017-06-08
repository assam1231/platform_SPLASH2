#ifndef SIMPLENI_H_S4ZKUT72
#define SIMPLENI_H_S4ZKUT72

#include <string>
#include <systemc>
#include <ocpip.h>
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "SimpleAddressMap.h"
#include "mm.h"

template <unsigned int g_data_width>
class SimpleInitNI: sc_core::sc_module
{
  typedef ocpip::ocp_slave_socket_tl3<g_data_width, 1> targetSocket_t ;
  typedef tlm_utils::simple_initiator_socket <SimpleInitNI, g_data_width> initSocket_t; 

public:
  SimpleInitNI(sc_core::sc_module_name name, 
               int id, 
               SimpleAddressMap* address_map, 
               sc_core::sc_time cycle_time = sc_core::sc_time(20, sc_core::SC_NS));
  ~SimpleInitNI(){;}
  
  targetSocket_t targetSocketIP;
  initSocket_t initSocketNoC;

  tlm::tlm_sync_enum input_nb_transport_fw(
				tlm::tlm_generic_payload &trans,
				tlm::tlm_phase           &phase,
				sc_core::sc_time         &delay);

  void input_ni_process();

  tlm::tlm_sync_enum output_nb_transport_bw(
				tlm::tlm_generic_payload &trans,
				tlm::tlm_phase           &phase,
				sc_core::sc_time         &delay);

private:
  std::string input_socket_name();
  std::string ip_peq_name();
  tlm::tlm_generic_payload* get_trans_from_peq();
  void send_ack_to_PE(tlm::tlm_generic_payload *trans);
  void check_ack_state(tlm::tlm_sync_enum retval);
  tlm::tlm_generic_payload* wrap_trans(tlm::tlm_generic_payload *trans);
  void foward_trans_to_router(tlm::tlm_generic_payload *noc_trans);


private:
  int m_id;
  sc_core::sc_event m_outputAckEvent;
  SimpleAddressMap& m_addressMap;
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_ipPeq;
  ocpip::ocp_parameters m_ocp_parameters;
  sc_core::sc_time m_cycleTime;
  unsigned m_niDelay; //in cycles
  mm m_mm;
};

#endif /* end of include guard: SIMPLENI_H_S4ZKUT72 */
