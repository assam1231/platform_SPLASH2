#ifndef SIMPLETARGETNI_H_DERNPJIA
#define SIMPLETARGETNI_H_DERNPJIA

#include <systemc>
#include <ocpip.h>
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "SimpleAddressMap.h"

/*
 *
 * Socket type description:
 * For Target Network Interface, the input socket is 
 * connected to NoC. We use TLM's simple socket.
 * The ourput socket is connected to IP, which is 
 * OCP's socket.
 *
 *
 */

template<unsigned int g_data_width>
class SimpleTargetNI: sc_core::sc_module
{
  typedef tlm_utils::simple_target_socket <SimpleTargetNI, g_data_width> targetSocket_t; 
  typedef ocpip::ocp_master_socket_tl3<g_data_width, 1> initSocket_t ;

public:
  SimpleTargetNI(sc_core::sc_module_name name,  
                 int id, 
                 SimpleAddressMap *map,
                 sc_core::sc_time cycle_time = sc_core::sc_time(20,sc_core::SC_NS));
  virtual ~SimpleTargetNI (){;}

  targetSocket_t targetSocketNoC;
  initSocket_t initSocketIP;

  tlm::tlm_sync_enum 
    input_nb_transport_fw(
        tlm::tlm_generic_payload &noc_trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay);

  void output_ni_process();

  tlm::tlm_sync_enum 
    output_nb_transport_bw(
        tlm::tlm_generic_payload &noc_trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay);

private:
  std::string noc_peq_name();
  std::string output_socket_name();
  tlm::tlm_generic_payload* get_noc_trans_from_peq();
  tlm::tlm_generic_payload* unwrap_noc_trans(tlm::tlm_generic_payload *noc_trans);
  void send_ack_to_NoC(tlm::tlm_generic_payload *noc_trans);
  void remap_trans_address(tlm::tlm_generic_payload *trans);
  void foward_trans_to_ip(tlm::tlm_generic_payload *trans);

private:
  int m_id;
  SimpleAddressMap &m_addressMap;
  sc_core::sc_time m_cycleTime;
  unsigned int m_niDelay;
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_nocPeq;
  sc_core::sc_event m_outputAckEvent;
  ocpip::ocp_parameters m_ocpParameters;
};

#endif /* end of include guard: SIMPLETARGETNI_H_DERNPJIA */
