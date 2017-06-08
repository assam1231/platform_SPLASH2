#ifndef OCPWRAPPEDMEMORY_H
#define OCPWRAPPEDMEMORY_H

#include <systemc>
#include <sstream>
#include <stdexcept>
#include "ocpip.h"
#include "tlm.h"
#include "tlm_utils/peq_with_get.h"
#include <iostream>
#include <deque>
#include <DRAMSim.h>
#include "Storage.h"



using tlm::tlm_generic_payload;
using tlm::tlm_sync_enum;

class OCPWrappedMemory: public sc_module
{
public:
  SC_HAS_PROCESS(OCPWrappedMemory);
  OCPWrappedMemory(
        const sc_core::sc_module_name name,
        const unsigned int memory_size = 0x40000000 );
  //SC_CTOR(OCPWrappedMemory);
  ~OCPWrappedMemory();
  
  // along
  typedef tlm::tlm_generic_payload transaction_type;

  //OCP slave socket
  ocpip::ocp_slave_socket_tl3<32, 1> ocp_slave_socket;
  
  //OCP socket callback function
  tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&);
  
  //Memory callback functions
  void read_complete_CB (unsigned int id, uint64_t address, uint64_t cycle); //used to debug
  void write_complete_CB(unsigned int id, uint64_t address, uint64_t cycle); //used to debug

  //
  void req_process();
  
  //Per-cycle process to update memory 
  void update_method();

  //Response
  void response_method();

  void printStats(bool finalStats);
  
  void write(tlm::tlm_generic_payload* trans);
  void write_data(
  const unsigned int addr, 
  const unsigned int data, 
  const unsigned int data_byte_en );
	
  unsigned int read_data(const unsigned int addr); 
  
private:
	
  const unsigned int MEMORY_SIZE;
  unsigned int* const m_memory;
  ocpip::ocp_parameters ocp_parameters_;  
	
  //sub modules
  DRAMSim::MultiChannelMemorySystem *mem;
  std::deque<tlm_generic_payload*> return_trans_queue;
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> req_peq_;
  Storage storage;
  
  unsigned request_num;
  unsigned update_cycle;
  
  //functions
  tlm_generic_payload* find_trans_in_deque(uint64_t);
  unsigned get_trans_position(tlm_generic_payload*);

  sc_event send_resp_event;
  sc_event trans_comming;
  tlm_generic_payload* return_trans_ptr;
  
  // along
  tlm::tlm_generic_payload* pop_req_peq();
  unsigned int get_burst_length(transaction_type &trans);
  ocpip::burst_seqs get_burst_sequence(transaction_type &trans);
  
  unsigned int mem_burst_len;
  unsigned int read_burst_len;
  unsigned int write_burst_len;
  
};
#endif
