#ifndef NOCTARGETIF_H
#define NOCTARGETIF_H

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
//#define NTS_4_DRAMSIM2
//#define NTS_DRAMSIM2

using namespace std;

namespace ispa
{

template<unsigned int data_width_g = 32>
class TargetNI:
  public sc_core::sc_module 
//	public tlm::tlm_mm_interface
{

//--- Public methods ---//
public:
    //Constructor & destructor
    SC_HAS_PROCESS(TargetNI);
    TargetNI(sc_core::sc_module_name name,  
              int id, 
              SimpleAddressMap *map,
              sc_core::sc_time cycle_time = sc_core::sc_time(20000,sc_core::SC_PS)
              );
    ~TargetNI(){};

    //request part
    tlm::tlm_sync_enum 
    req_noc_ni_fw(
        tlm::tlm_generic_payload &noc_trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay);

    void req_rx_ni_thread();

    //response part
    tlm::tlm_sync_enum 
    oterm_ni_bw(
        tlm::tlm_generic_payload &trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay);

    void rsp_tx_ni_thread();

    tlm::tlm_sync_enum 
    rsp_rou_ni_bw(
        tlm::tlm_generic_payload &trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay);

//--- Public fields ---//
public:
    //terminals connect to IP
    ocpip::ocp_master_socket_tl3<data_width_g, 1> initSocket;

    //terminals connect to router
		tlm_utils::simple_target_socket    <TargetNI, data_width_g> req_target_;
		tlm_utils::simple_initiator_socket <TargetNI, data_width_g> rsp_init_;

    //pay load event queue for sockets.
		tlm_utils::peq_with_get<tlm::tlm_generic_payload> req_noc_peq_; 
		tlm_utils::peq_with_get<tlm::tlm_generic_payload> rsp_ip_peq_;
        
        std::map<tlm::tlm_generic_payload*, int> trans_sourceID_map_;

//--- Private fields ---//
private:

    //events
		sc_core::sc_event otermAckEvent_; 
		sc_core::sc_event rsp_rouAckEvent_; 
		sc_core::sc_event rsp_tx_deque_event_;
    
		std::deque<unsigned int > rsp_tx_source_deque_;
		const unsigned int HEADER_LENGTH_IN_BYTE;
    sc_dt::uint64 m_mask_;
    int id_;
		sc_core::sc_time cycleTime_;
    ocpip::ocp_parameters ocp_parameters_;

    //memory management
    mm m_mm;

//--- Private methods ---//
private:
    std::string req_noc_peq_name();
    std::string rsp_ip_peq_name();
    std::string initSocket_name();


}; //end of class
} //end of namespace ispa


#endif /* end of include guard: NOCTARGETIF_H */
