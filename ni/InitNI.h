#ifndef NOCINITIF_H
#define NOCINITIF_H

#include <systemc>
#include <stdexcept>
#include <iostream>

#include "tlm.h"
#ifdef MTI_SYSTEMC
#include "simple_initiator_socket.h"
#include "simple_target_socket.h"
#else
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#endif

#include "ocpip.h"

//#include "NetworkPayload.h"
#include "SimpleAddressMap.h"
#include "mm.h"

#define NTS_4_DRAMSIM2_crossbar

using namespace std;

namespace ispa
{
template<unsigned int data_width_g = 32>
class InitNI:
  public sc_core::sc_module
{

//--- Public methods ---//
public:
    //Constructor & destructor
    SC_HAS_PROCESS(InitNI);
    InitNI(sc_core::sc_module_name name,  
              int id, 
              SimpleAddressMap *map,
              sc_core::sc_time cycle_time = sc_core::sc_time(20000,sc_core::SC_PS)
              );
    ~InitNI(){};
 
    //request part
		tlm::tlm_sync_enum iterm_ni_fw(
				tlm::tlm_generic_payload &trans,
				tlm::tlm_phase           &phase,
				sc_core::sc_time         &delay);

		void req_tx_ni_thread();

		tlm::tlm_sync_enum req_rou_ni_bw(
				tlm::tlm_generic_payload &noc_trans,
				tlm::tlm_phase           &phase,
				sc_core::sc_time         &delay);

    //response part
		void rsp_rx_ni_thread();

		tlm::tlm_sync_enum rsp_noc_ni_fw(
				tlm::tlm_generic_payload &trans,
				tlm::tlm_phase           &phase,
				sc_core::sc_time         &delay);

//--- Public fields ---//
public:
    //terminals connect to IP
    ocpip::ocp_slave_socket_tl3<data_width_g, 1>  targetSocket;

    //terminals connect to router
		tlm_utils::simple_initiator_socket <InitNI, data_width_g> req_init_; 
		tlm_utils::simple_target_socket    <InitNI, data_width_g> rsp_target_; 

    //payload event queue for slave sockets.
		tlm_utils::peq_with_get<tlm::tlm_generic_payload> req_ip_peq_;
		tlm_utils::peq_with_get<tlm::tlm_generic_payload> rsp_noc_peq_;


//--- Private fields ---//
private:
    //payload buffer 
		tlm::tlm_generic_payload* rsp_rx_trans_buffer_;

    //events
    sc_core::sc_event req_rouAckEvent_;
		sc_core::sc_event rsp_rx_trans_buffer_fill_;
		sc_core::sc_event rsp_rx_trans_buffer_empty_;

		sc_core::sc_time cycleTime_;
		const unsigned int HEADER_LENGTH_IN_BYTE;
    sc_dt::uint64 m_mask_;
    unsigned int id_;
    SimpleAddressMap *m_addr_map_;
    ocpip::ocp_parameters ocp_parameters_;

    //memory management 
    mm m_mm;

//--- Private methods ---//
private:
    std::string req_ip_peq_name();
    std::string rsp_noc_peq_name();
    std::string targetSocket_name();
	unsigned int decode(const sc_dt::uint64& address);
	unsigned int crossbar_decode(const sc_dt::uint64& address, unsigned int id);

};

}
#endif /* !NOCINITIF_H */

