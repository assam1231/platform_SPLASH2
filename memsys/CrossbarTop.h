#ifndef _CROSSBARTOP_H_
#define _CROSSBARTOP_H_

#include <systemc>
#include <sstream>
#include "Crossbar.h"

#include <stdexcept>
#include "tlm_utils/peq_with_get.h"

#include "tlm.h"
#ifdef MTI_SYSTEMC
#include "simple_initiator_socket.h"
#include "simple_target_socket.h"
#else
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#endif

#include "ocpip.h"
#include "mm.h"


using namespace std;


class CrossbarTop : public sc_core::sc_module
{
	public:
		SC_HAS_PROCESS(CrossbarTop);
		CrossbarTop (
			sc_core::sc_module_name name,
			const unsigned int channel_num = 4,
			sc_core::sc_time cycle_time = sc_core::sc_time(4000,sc_core::SC_PS)
		);
		~CrossbarTop();
		
		//request part
		tlm::tlm_sync_enum 
		req_crossbar_1_fw(
			tlm::tlm_generic_payload &noc_trans,
			tlm::tlm_phase           &phase,
			sc_core::sc_time         &delay);
		
		tlm::tlm_sync_enum 
		crossbar_1_bw(
			tlm::tlm_generic_payload &trans,
			tlm::tlm_phase           &phase,
			sc_core::sc_time         &delay);
		
		tlm::tlm_sync_enum 
		req_crossbar_2_fw(
			tlm::tlm_generic_payload &noc_trans,
			tlm::tlm_phase           &phase,
			sc_core::sc_time         &delay);
		
		tlm::tlm_sync_enum 
		crossbar_2_bw(
			tlm::tlm_generic_payload &trans,
			tlm::tlm_phase           &phase,
			sc_core::sc_time         &delay);
			
		tlm::tlm_sync_enum 
		req_crossbar_3_fw(
			tlm::tlm_generic_payload &noc_trans,
			tlm::tlm_phase           &phase,
			sc_core::sc_time         &delay);
		
		tlm::tlm_sync_enum 
		crossbar_3_bw(
			tlm::tlm_generic_payload &trans,
			tlm::tlm_phase           &phase,
			sc_core::sc_time         &delay);
			
		tlm::tlm_sync_enum 
		req_crossbar_4_fw(
			tlm::tlm_generic_payload &noc_trans,
			tlm::tlm_phase           &phase,
			sc_core::sc_time         &delay);
		
		tlm::tlm_sync_enum 
		crossbar_4_bw(
			tlm::tlm_generic_payload &trans,
			tlm::tlm_phase           &phase,
			sc_core::sc_time         &delay);
			
	public:
	
		void noc_to_crb_0_process();
		void crb_to_mem_0_process();
		void mem_to_crb_0_process();
		void crb_to_noc_0_process();
		
		void noc_to_crb_1_process();
		void crb_to_mem_1_process();
		void mem_to_crb_1_process();
		void crb_to_noc_1_process();
		
		void noc_to_crb_2_process();
		void crb_to_mem_2_process();
		void mem_to_crb_2_process();
		void crb_to_noc_2_process();
		
		void noc_to_crb_3_process();
		void crb_to_mem_3_process();
		void mem_to_crb_3_process();
		void crb_to_noc_3_process();
			
		
	public:
	
		ocpip::ocp_master_socket_tl3<32, 1> * init_crossbar_socket[4];
		ocpip::ocp_slave_socket_tl3<32, 1> * target_crossbar_socket[4];
		
		/*ocpip::ocp_master_socket_tl3<32, 1>  init_crossbar_socket_0;
		ocpip::ocp_slave_socket_tl3<32, 1>   target_crossbar_socket_0;
		ocpip::ocp_master_socket_tl3<32, 1>  init_crossbar_socket_1;
		ocpip::ocp_slave_socket_tl3<32, 1>   target_crossbar_socket_1;
		ocpip::ocp_master_socket_tl3<32, 1>  init_crossbar_socket_2;
		ocpip::ocp_slave_socket_tl3<32, 1>   target_crossbar_socket_2;
		ocpip::ocp_master_socket_tl3<32, 1>  init_crossbar_socket_3;
		ocpip::ocp_slave_socket_tl3<32, 1>   target_crossbar_socket_3;*/
		
		
		
		sc_fifo<tlm::tlm_generic_payload*> * noc_to_sch[4];
		sc_fifo<tlm::tlm_generic_payload*> * sch_to_mem[4];
		sc_fifo<tlm::tlm_generic_payload*> * mem_to_sch[4];
		sc_fifo<tlm::tlm_generic_payload*> * sch_to_noc[4];
		
		tlm_utils::peq_with_get<tlm::tlm_generic_payload> req_noc_peq_1;
		tlm_utils::peq_with_get<tlm::tlm_generic_payload> rsp_ip_peq_1;
		
		tlm_utils::peq_with_get<tlm::tlm_generic_payload> req_noc_peq_2;
		tlm_utils::peq_with_get<tlm::tlm_generic_payload> rsp_ip_peq_2;
		
		tlm_utils::peq_with_get<tlm::tlm_generic_payload> req_noc_peq_3;
		tlm_utils::peq_with_get<tlm::tlm_generic_payload> rsp_ip_peq_3;
		
		tlm_utils::peq_with_get<tlm::tlm_generic_payload> req_noc_peq_4;
		tlm_utils::peq_with_get<tlm::tlm_generic_payload> rsp_ip_peq_4;
		
	private:
		Crossbar * crossbar;
	 
	private:
		sc_core::sc_time cycleTime_;
		ocpip::ocp_parameters ocp_parameters_;
		
		std::deque<unsigned int > rsp_tx_source_deque_1;
		sc_core::sc_event rsp_tx_deque_event_1;
		
		std::deque<unsigned int > rsp_tx_source_deque_2;
		sc_core::sc_event rsp_tx_deque_event_2;
		
		std::deque<unsigned int > rsp_tx_source_deque_3;
		sc_core::sc_event rsp_tx_deque_event_3;
		
		std::deque<unsigned int > rsp_tx_source_deque_4;
		sc_core::sc_event rsp_tx_deque_event_4;
		
		const unsigned int HEADER_LENGTH_IN_BYTE;
		ispa::mm m_mm;
		
		sc_core::sc_event otermAckEvent_1;
		sc_core::sc_event otermAckEvent_2;
		sc_core::sc_event otermAckEvent_3;
		sc_core::sc_event otermAckEvent_4;
        
        std::map<tlm::tlm_generic_payload*, int> mapping0;
        std::map<tlm::tlm_generic_payload*, int> mapping1;
        std::map<tlm::tlm_generic_payload*, int> mapping2; 
        std::map<tlm::tlm_generic_payload*, int> mapping3;
		
		const unsigned int data_width_byte;
		
};


#endif
