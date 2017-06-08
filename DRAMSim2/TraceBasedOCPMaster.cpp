#include "TraceBasedOCPMaster.h"
#include "HHsuPrintMacros.h"
#include "SystemConfiguration.h"
#include <sstream>

SC_HAS_PROCESS(TraceBasedOCPMaster);
TraceBasedOCPMaster::TraceBasedOCPMaster(sc_module_name name, int sim_cycle):
    sc_module(name),
    ocp_master_socket("ocp_master_socket", ocpip::ocp_master_socket_tl3<>::mm_txn_with_data())
{

  SC_THREAD(send_req_thread);
  ocp_master_socket.register_nb_transport_bw(this, &TraceBasedOCPMaster::nb_transport_bw);

  simulation_cycle = sim_cycle;
  current_cycle = 0;
}    

TraceBasedOCPMaster::~TraceBasedOCPMaster(){

}

void TraceBasedOCPMaster::send_req_thread() 
{

  //Read address, command and cycle from trace file
  ifstream trace_file;
  //trace_file.open("k6_aoe_02_short.trc");
  trace_file.open("k6_aoe_02_short_half.trc");
	if (!trace_file) {
		std::cerr << sc_core::sc_module::name() << ": Error - Could not open trace file"<<endl;
		exit(1);
	}

  tlm::tlm_sync_enum status;
  tlm::tlm_generic_payload *trans;
  tlm::tlm_phase phase;
  sc_time delay(0, SC_NS);

  std::string addr_str, cmd_str;
  sc_time cycle_time(tCK, SC_NS);
  unsigned int addr;
  uint64_t cycle;
  TransType trans_type;
  bool print_cycle=true;

  while (current_cycle <= simulation_cycle &&  trace_file >> addr_str >> cmd_str >> cycle) {
    current_cycle = get_current_cycle();
    HHSU_DEBUG("")
    if (print_cycle)
      HHSU_DEBUG( "---------- ["  << std::dec << current_cycle << "] ----------")
    print_cycle = true;
    
    //Handle address
    std::istringstream addr_ss(addr_str.substr(2)); //get rid of "0x"
    addr_ss >> hex >> addr;
    addr = alignAddress(addr);
   
    trans_type = recognize_trans_type(cmd_str);

    trans = ocp_master_socket.get_transaction();
    setup_transaction(trans, trans_type, addr);

    if (cycle > current_cycle) {
      //std::cout << "Wait " <<  (cycle - current_cycle) << " cycle" << std::endl;
      wait((cycle - current_cycle) * tCK, SC_NS);
      print_cycle = false;
      current_cycle = get_current_cycle();
      HHSU_DEBUG("---------- ["  << std::dec << current_cycle << "] ----------")
    }

    status = ocp_master_socket->nb_transport_fw(*trans, phase, delay);
    HHSU_DEBUGN("[" << sc_core::sc_module::name() << "] Sent transaction: ")

    if (trans_type) 
      HHSU_DEBUG("write to " << hex << addr << endl)
    else 
      HHSU_DEBUG("read from " << hex << addr << endl)

    switch (status) {
      case tlm::TLM_ACCEPTED: 
        break;

      case tlm::TLM_UPDATED: //expected status for read command 
        if (phase != tlm::END_REQ) {
          std::cerr << sc_core::sc_module::name() << ": Unexpected phase." << std::endl;
          exit(1);
        }
        break;

      case tlm::TLM_COMPLETED: //expeced status for write command
          check_and_release(trans); 
          break;

      default:
          std::cerr << sc_core::sc_module::name() << ": Unexpected status." << std::endl;
          exit(1);
    }
  }
	trace_file.close();
}

tlm::tlm_sync_enum TraceBasedOCPMaster::nb_transport_bw(
    tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay) 
{
  if (phase == tlm::BEGIN_RESP) {  //expected for read command  
    check_and_release(&trans);
    phase = tlm::END_RESP;
  }
  else {
    HHSU_ERROR(sc_core::sc_module::name() << ": Unexpected phase.")
    exit(1);
  }

  return tlm::TLM_UPDATED;
}

void TraceBasedOCPMaster::setup_transaction(
    tlm::tlm_generic_payload *trans, TransType type, sc_dt::uint64 addr)
{
  unsigned transactionSize = (JEDEC_DATA_BUS_BITS/8)*BL;
  ocp_master_socket.reserve_data_size(*trans, transactionSize);
  if (type == DATA_READ) 
    trans->set_command(tlm::TLM_READ_COMMAND);
  else { 
    trans->set_command(tlm::TLM_WRITE_COMMAND);
  }
  trans->set_address(addr);
  //trans->set_data_ptr(data);
  //trans->set_data_length(transactionSize);
  trans->set_byte_enable_ptr(NULL); 
  trans->set_streaming_width(sizeof(uint64_t)); 
  trans->set_dmi_allowed(false);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  if (type == DATA_WRITE) {
    unsigned char *data = trans->get_data_ptr();
    for (unsigned i=0; i<transactionSize; i++)
    {
      data[i]=1; 
    }
  }
  
}

TraceBasedOCPMaster::TransType TraceBasedOCPMaster::recognize_trans_type(std::string cmd) 
{
  if (cmd == "P_MEM_WR" || cmd == "BOFF") {
    return DATA_WRITE;
  }
  else if (cmd == "P_FETCH" || cmd == "P_MEM_RD" ||
           cmd == "P_LOCK_RD" || cmd == "P_LOCK_WR") {
    return DATA_READ;
  }
  else {
    std::cerr << sc_core::sc_module::name() << ": Unknown command." << std::endl;
    exit(1);
  }
}

void TraceBasedOCPMaster::check_and_release(tlm::tlm_generic_payload *trans)
{
  if (trans->is_response_error())
  {   
    std::cerr << sc_core::sc_module::name() << ": Transaction returned with error, response status = ";
    std::cerr << trans->get_response_string().c_str() << std::endl;
    exit(1);
  }   

  tlm::tlm_command cmd = trans->get_command();
  sc_dt::uint64    adr = trans->get_address();
  //uint64_t*        ptr = reinterpret_cast<uint64_t *>( trans->get_data_ptr() );

  HHSU_DEBUG("["<< sc_core::sc_module::name() << "] Released trans  addr=" << hex << adr << " cmd=" << (cmd ? 'W' : 'R') 
  << " @" << dec << get_current_cycle()<< endl)
  ocp_master_socket.release_transaction(trans);
}

unsigned int TraceBasedOCPMaster::alignAddress(unsigned int address) 
{

  unsigned throwAwayBits = log2((BL*JEDEC_DATA_BUS_BITS/8));
  address >>= throwAwayBits;
  address <<= throwAwayBits;
  return address;

}

unsigned int TraceBasedOCPMaster::log2(unsigned int value)
{
  unsigned logbase2 = 0;
  unsigned orig = value;
  value>>=1;
  while (value>0)
  {
    value >>= 1;
    logbase2++;
  }
  if ((unsigned)1<<logbase2<orig)logbase2++;
  return logbase2;
}

uint64_t TraceBasedOCPMaster::get_current_cycle()
{
  return static_cast<uint64_t>( (sc_time_stamp().to_double() / tCK));
}
