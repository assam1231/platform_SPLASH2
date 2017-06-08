#ifndef _CROSSBAR_H_
#define _CROSSBAR_H_

#include "systemc.h"
#include "tlm.h"


class Crossbar : public sc_core::sc_module
{
	
	public:
		SC_HAS_PROCESS(Crossbar);
		Crossbar (
			sc_core::sc_module_name name,
			const unsigned int channel_num = 4,
			sc_core::sc_time cycle_time = sc_core::sc_time(4,sc_core::SC_NS)
		);
		~Crossbar(){};
		
		
		sc_port<sc_fifo_in_if<tlm::tlm_generic_payload*> > * in_if_tomem[4];
		sc_port<sc_fifo_out_if<tlm::tlm_generic_payload*> > * out_if_tomem[4];
		sc_port<sc_fifo_in_if<tlm::tlm_generic_payload*> > * in_if_tonoc[4];
		sc_port<sc_fifo_out_if<tlm::tlm_generic_payload*> > * out_if_tonoc[4];
		
		sc_fifo<tlm::tlm_generic_payload*> * fore_fifo[4];
		sc_fifo<tlm::tlm_generic_payload*> * back_fifo[4];
		
		
		void fore0_router();
		void fore1_router();
		void fore2_router();
		void fore3_router();
		
		void fore0_selecter();
		void fore1_selecter();
		void fore2_selecter();
		void fore3_selecter();
		
		void back0_router();
		void back1_router();
		void back2_router();
		void back3_router();
		
		void back0_selecter();
		void back1_selecter();
		void back2_selecter();
		void back3_selecter();
	
	private:
	
		const sc_module_name name;
		const unsigned int channel_num;
		sc_core::sc_time cycleTime_;
	
};



#endif