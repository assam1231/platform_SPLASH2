#ifndef TRACE_BASED_OCP_MASTER_H
#define TRACE_BASED_OCP_MASTER_H

#include <string>
#include "systemc.h"
#include "ocpip.h"
#include "SystemConfiguration.h"

class TraceBasedOCPMaster: public sc_core::sc_module
{
public:
  TraceBasedOCPMaster(sc_module_name name, int sim_cycle);
  ~TraceBasedOCPMaster();

  //OCP master socket
  ocpip::ocp_master_socket_tl3<64, 1> ocp_master_socket;

  //OCP socket callback function
  tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&);

  void send_req_thread();


private:
  uint64_t simulation_cycle;
  uint64_t current_cycle;
  enum TransType {DATA_READ, DATA_WRITE};
  //unsigned char data[32]; //64bit bus length * 4BL / 8bit per char

private:
  void setup_transaction(tlm::tlm_generic_payload*, TransType, sc_dt::uint64);
  TransType recognize_trans_type(std::string cmd);
  void check_and_release(tlm::tlm_generic_payload*);
  unsigned int log2(unsigned int);
  unsigned int alignAddress(unsigned int);
  uint64_t get_current_cycle();
  

};
#endif
