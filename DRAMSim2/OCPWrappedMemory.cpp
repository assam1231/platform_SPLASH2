#include "OCPWrappedMemory.h"
#include "HHsuPrintMacros.h"
#include "SystemConfiguration.h"

//#define EN_CROSSBAR_DEBUG
//#define EN_MEM_TRACE
#include "CrossbarDebug.h"


using namespace DRAMSim;
using namespace std;
using std::cout;
using std::hex;
using std::dec;
using std::endl;
using std::cerr;
using tlm::tlm_phase;
using tlm::tlm_generic_payload;


//--Constructor--//

OCPWrappedMemory::OCPWrappedMemory(
    const sc_module_name name,
    const unsigned int memory_size)
    :sc_module(name),
    ocp_slave_socket("ocp_slave_socket"),
    MEMORY_SIZE( memory_size ),
    m_memory( new unsigned int [MEMORY_SIZE] ),
    req_peq_("ExtMemoryReqPeq"),
	update_cycle(0)
    
{
    //Initialize memory 
    mem = getMemorySystemInstance("DDR3_micron_32M_8B_x8_sg25E.ini", 
                                "system.ini.example.false", ".", "resultsfilename", 1024);

    //create and register our callback functions
    TransactionCompleteCB *read_cb = 
        new Callback<OCPWrappedMemory, void, unsigned, uint64_t, uint64_t> 
        (this, &OCPWrappedMemory::read_complete_CB);
    TransactionCompleteCB *write_cb = 
        new Callback<OCPWrappedMemory, void, unsigned, uint64_t, uint64_t>
        (this, &OCPWrappedMemory::write_complete_CB);

    //mem->RegisterCallbacks(read_cb, write_cb, &power_CB);
    mem->RegisterCallbacks(read_cb, write_cb, NULL);

    // set the frequency ratio to 1:1
    mem->setCPUClockSpeed(0); 

    // handle request
    SC_THREAD(req_process);

    //Register Processes
    SC_THREAD(update_method);

    SC_METHOD(response_method);
    sensitive << send_resp_event;
    dont_initialize();
    
    ocp_slave_socket.set_ocp_config(ocp_parameters_);
    ocp_slave_socket.register_nb_transport_fw(this, &OCPWrappedMemory::nb_transport_fw);
    
}

//--Destructor--//
OCPWrappedMemory::~OCPWrappedMemory()
{
  delete mem;
}


//-- Read Complete Callback--//
//This callback function wiil copy address, data pointer 
//and data size to local member so that we can use them 
//in response_process
void OCPWrappedMemory::read_complete_CB(unsigned id, uint64_t address, uint64_t cycle)
{
  //cout << "Memory READ Complete!!! " << endl << endl;
  
  return_trans_ptr = find_trans_in_deque(address);
  unsigned int      addr = return_trans_ptr->get_address();
  unsigned int      *data_ptr = reinterpret_cast<unsigned int *>(return_trans_ptr->get_data_ptr());
  unsigned int      b_length = get_burst_length(*return_trans_ptr);
  ocpip::burst_seqs b_sequence = get_burst_sequence(*return_trans_ptr);
  
  for (unsigned i=0; i<b_length; i++) 
  {
    switch (b_sequence) 
    {
        case ocpip::INCR:
            *(data_ptr+i) = read_data(addr + i*4);
            break;

        case ocpip::STRM:
            *(data_ptr+i) = read_data(addr);
            break;

        default:
            std::ostringstream oss;
            oss << "ExtMemory::req_perocess: Received not suported burst sequence type.";
            throw std::runtime_error(oss.str().c_str());
            break;
    }
  }
  
  CROSSBAR_DEBUG(
		cout      << "  read_sc_time_stamp = " << sc_time_stamp() << endl;
        cout      << "  update_cycle = " << dec << update_cycle << endl;
        std::cout << "  addr: " << std::hex << "0x" << addr <<  dec << std::endl; 
        std::cout << "  b_sequence: " << b_sequence << std::endl;
        std::cout << "  b_length: " << b_length << std::endl;
      
      
        std::cout << "  cmd: read\n";
        std::cout << "  read data: " << std::dec;
        for (unsigned int i=0; i<b_length; i++) 
        {
            std::cout << data_ptr[i] << " ";
        }
        std::cout << std::endl << std::endl;
    );
    
    MEM_TRACE(
        cout << name() << ": ";
        cout << "read_complete_sc_time_stamp = " << sc_time_stamp() << " ";
        cout << "command = " << return_trans_ptr->get_command() << " ";
        cout << "address = " << "0x" << hex << addr << " ";
        cout << "update_cycle = " << dec << update_cycle << endl;
    );
    
  send_resp_event.notify();

}

//-- Write Complete Callback--//
void OCPWrappedMemory::write_complete_CB(unsigned int id, uint64_t address, uint64_t cycle)
{
    //cout << "Memory WRITE Complete!!! " << endl << endl;
  
    return_trans_ptr = find_trans_in_deque(address);
  
    unsigned int      addr = return_trans_ptr->get_address();
    unsigned int      *data_ptr = reinterpret_cast<unsigned int *>(return_trans_ptr->get_data_ptr());
    unsigned int      *be_ptr = reinterpret_cast<unsigned int *>(return_trans_ptr->get_byte_enable_ptr());
    unsigned int      b_length = get_burst_length(*return_trans_ptr);
    ocpip::burst_seqs b_sequence = get_burst_sequence(*return_trans_ptr);

    CROSSBAR_DEBUG(
		cout      << "  write_sc_time_stamp = " << sc_time_stamp() << endl;
		cout      << "  update_cycle = " << dec << update_cycle << endl;
        std::cout << "  addr: " << std::hex << "0x" << addr <<  dec << std::endl;
        std::cout << "  b_sequence: " << b_sequence << std::endl;
        std::cout << "  b_length: " << b_length << std::endl;
      
        std::cout << "  cmd: write\n";
        std::cout << "  write data: " << std::dec;
        for (unsigned int i=0; i < b_length; i++) 
        {
            std::cout << data_ptr[i] << " ";
        }
        std::cout << std::endl << std::endl;;
    );
	
    for (unsigned i=0; i < b_length; i++) 
    {
      switch (b_sequence) 
      {
        case ocpip::INCR:
          write_data( addr + i*4, *(data_ptr+i), *(be_ptr+i));
          break;
        
        case ocpip::STRM:
          write_data( addr, *(data_ptr+i), *(be_ptr+i));
          break;
        
        default:
          std::ostringstream oss;
          oss << "ExtMemory::req_perocess: Received not suported burst sequence type.";
          throw std::runtime_error(oss.str().c_str());
          break;
      }
    }
    
    MEM_TRACE(
        cout << name() << ": ";
        cout << "write_complete_sc_time_stamp = " << sc_time_stamp() << " ";
        cout << "command = " << return_trans_ptr->get_command() << " ";
        cout << "address = " << "0x" << hex << addr << " ";
        cout  <<"update_cycle = " << dec << update_cycle << endl;
    );    
    
    send_resp_event.notify();
  
}

//-- Update Method --//
//This process will update memory every cycle.
//and see if the return data is ready;
//if it does, notify the send_resp_event
void OCPWrappedMemory::update_method()
{
  while (1) {

      mem->update();
	  update_cycle++;
      wait(tCK, SC_NS);

  }
}

//-- Non-Blocking Transport Forward Callback --//
//Used by OCP socket.
//Will accept transaction and add to memory
//and store payload to a deque

tlm::tlm_sync_enum OCPWrappedMemory::nb_transport_fw(
    tlm::tlm_generic_payload  &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay)
{  
        
    CROSSBAR_DEBUG(
		cout  <<"  mem_sc_time_stamp = " << sc_time_stamp() << endl;
		cout  <<"  update_cycle = " << dec << update_cycle << endl;
        if(phase == tlm::END_RESP){cout << endl;};
        unsigned int    *data_ptr = reinterpret_cast<unsigned int *>(trans.get_data_ptr());
        unsigned int    b_length = get_burst_length(trans);
        cout << "  cmd: " << trans.get_command() << endl;
        cout << "  In memory nb_transport_fw " << endl;
        cout << "  phase: " << phase << endl;
        if( trans.get_command() == tlm::TLM_WRITE_COMMAND ){
            cout << "  data: " << dec;
            for(unsigned int i = 0 ; i < b_length ; i++)
            {
                cout << data_ptr[i] << " ";
            }
            cout << endl;
        }
        cout << "  mem_trans_target: " << hex << trans.get_address() << endl << endl;
    );
    
    MEM_TRACE(
        cout << name() << ": ";
        cout << "fw_sc_time_stamp = " << sc_time_stamp() << " ";
        cout << "command = " << trans.get_command() << " ";
        cout << "address = " << "0x" << hex << trans.get_address() << " ";
        cout << "update_cycle = " << dec << update_cycle << endl;
    );
    
	//cout << " MEM FW in MEMORY" << " ";
	//cout << " sc_time_stamp = " << sc_time_stamp() << endl;
	
    if(phase == tlm::BEGIN_REQ){
        req_peq_.notify(trans, delay); // put data to queue and ask queue to wait
    }
    else if(phase == tlm::END_RESP){
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
        return tlm::TLM_COMPLETED;
    }
    else {
        std::ostringstream oss;
        oss << "ExtMem::nb_transport_fw: got invalid PHASE";
        throw std::runtime_error(oss.str().c_str());
    }
    
    //trans.set_response_status( tlm::TLM_OK_RESPONSE );
    return tlm::TLM_ACCEPTED;
  
}

void OCPWrappedMemory::req_process() 
{
    tlm::tlm_generic_payload* trans;
    while (true)
    {
        trans = pop_req_peq();
        uint64_t addr = trans->get_address();
        tlm::tlm_command  cmd = trans->get_command();
        bool trans_accepted;
        
        
        tlm::tlm_phase     phase = tlm::END_REQ;
        sc_core::sc_time   delay = sc_core::SC_ZERO_TIME;
        tlm::tlm_sync_enum retval;
        
        /*cout << "  In memory req_process " << endl;
        cout << "  phase: " << phase << endl;
        cout << "  mem_trans_target: " << hex << trans->get_address() << endl << endl;*/    
        
        retval = ocp_slave_socket->nb_transport_bw(*trans, phase, delay);
        
        if (retval != tlm::TLM_COMPLETED) 
        {
            std::ostringstream oss;
            oss << "ExtMemory::req_process : invalid SYNC_ENUM";
            throw std::runtime_error(oss.str().c_str());
        }
        
        //Memory access
		
        trans->acquire(); // Increment the transaction reference count
		
		switch (cmd)
		{
			case tlm::TLM_READ_COMMAND:
			{
				CROSSBAR_DEBUG(
				cout<< "  put_addr = " << trans->get_address() << endl;
                cout<< "  sc_time_stamp = " << sc_time_stamp() << endl;
				cout<< "  update_cycle = " << dec << update_cycle << endl;
                );
                
                MEM_TRACE(
                    cout << name() << ": ";
                    cout << "radd_sc_time_stamp = " << sc_time_stamp() << " ";
                    cout << "command = " << trans->get_command() << " ";
                    cout << "address = " << "0x" << hex << trans->get_address() << " ";
                    cout << "update_cycle = " << dec << update_cycle << endl;
                );
                
				trans_accepted = mem->addTransaction(false, addr);
				CROSSBAR_DEBUG( cout << "  TLM_READ_COMMAND " << " trans_accepted: " << trans_accepted << endl << endl; );
				break;
			}
			case tlm::TLM_WRITE_COMMAND:
			{
				CROSSBAR_DEBUG(
				cout << "  put_addr = " << trans->get_address() << endl;
                cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
				cout << "  update_cycle = " << dec << update_cycle << endl;);
                
                MEM_TRACE(
                    cout << name() << ": ";
                    cout << "wadd_sc_time_stamp = " << sc_time_stamp() << " ";
                    cout << "command = " << trans->get_command() << " ";
                    cout << "address = " << "0x" << hex << addr << " ";
                    cout << "update_cycle = " << dec << update_cycle << endl;
                );
                
				trans_accepted = mem->addTransaction(true, addr);
				CROSSBAR_DEBUG( cout << "  TLM_WRITE_COMMAND " << " trans_accepted: " << trans_accepted << endl << endl; );
				break;
			}
			default: 
			{
				std::ostringstream oss;
				oss << "ExtMemory::req_process : received unsupported command.";
				throw std::runtime_error(oss.str().c_str());
			}
		} //end switch
		
        return_trans_queue.push_back(trans);
    } //end while
} 



void OCPWrappedMemory::response_method() {
  
  //cout << "  In response_method:" << endl;
	
	sc_core::sc_time cycle_time_ = sc_core::sc_time(4,sc_core::SC_NS);
    tlm::tlm_command  cmd = return_trans_ptr->get_command();
	
	//cout << " MEM BW in MEMORY" << " ";
	//cout << " sc_time_stamp = " << sc_time_stamp() << endl;
	
    if(cmd == tlm::TLM_READ_COMMAND){
	
        tlm_sync_enum status;
        tlm_phase bw_phase = tlm::BEGIN_RESP;
        sc_time delay = cycle_time_ * ((return_trans_ptr->get_data_length() / 4 + 2));
        //sc_time delay = SC_ZERO_TIME;
        status = ocp_slave_socket->nb_transport_bw(*return_trans_ptr, bw_phase, delay);
        
        MEM_TRACE(
            cout << name() << ": ";
            cout << "rsp_sc_time_stamp = " << sc_time_stamp() << " ";
            cout << "command = " << cmd << " ";
            cout << "address = " << "0x" << hex << return_trans_ptr->get_address() << " ";
            cout << "update_cycle = " << dec << update_cycle << endl;
        );

        switch (status) {
            case tlm::TLM_ACCEPTED: break;
            case tlm::TLM_UPDATED:
                if (bw_phase != tlm::END_RESP) 
            break; //ignorable phase
            case tlm::TLM_COMPLETED:
                unsigned position = get_trans_position(return_trans_ptr);
                return_trans_ptr->release();
                return_trans_queue.erase(return_trans_queue.begin() + position);
            
            /*for (std::deque < tlm::tlm_generic_payload* > ::iterator iter = return_trans_queue.begin(); iter != return_trans_queue.end(); ++ iter)
            {
                cout << std::hex << (*iter)->get_address() << endl; 
            }*/
            break;
        }
    }
    else {
        unsigned position = get_trans_position(return_trans_ptr);
        //return_trans_ptr->release();
        return_trans_queue.erase(return_trans_queue.begin() + position);
    }
}

void OCPWrappedMemory::printStats(bool finalStats)
{
  mem->printStats(finalStats); 
}

tlm_generic_payload* OCPWrappedMemory::find_trans_in_deque(uint64_t addr) 
{
  std::deque <tlm::tlm_generic_payload*>::iterator it = return_trans_queue.begin();
  while ((*it)->get_address() != addr) {
    ++ it;
    if (it == return_trans_queue.end()) {
      cerr << "Cannot find cerrosponding return transaction in return_trans_queue." << endl;
      exit(1);
    }
  }
  return (*it);
}

unsigned OCPWrappedMemory::get_trans_position(tlm_generic_payload* trans)
{
  unsigned position = 0;
  std::deque <tlm::tlm_generic_payload*>::iterator it = return_trans_queue.begin();
  while ((*it) != trans) {
    ++ it;
    ++ position;
    if (it == return_trans_queue.end()) {
      cerr << "Cannot find cerrosponding return transaction in return_trans_queue." << endl;
      exit(1);
    }
  }
  return position;
}

tlm::tlm_generic_payload* OCPWrappedMemory::pop_req_peq() 
{
  tlm::tlm_generic_payload* trans;
  /*if((trans = req_peq_.get_next_transaction()) == 0)
  {
    wait(req_peq_.get_event());
    trans = req_peq_.get_next_transaction();
  }
  return trans;*/
  
  trans = req_peq_.get_next_transaction();
  while(trans == NULL)
  { 
    wait(req_peq_.get_event());
    trans = req_peq_.get_next_transaction();
  }
  assert(trans != NULL);
  
  return trans;
  
  
}

unsigned int OCPWrappedMemory::get_burst_length(transaction_type &trans)
{
  ocpip::burst_length *b_length_ext;
  ocp_slave_socket.get_extension(b_length_ext, trans);
  return b_length_ext->value;
}

ocpip::burst_seqs OCPWrappedMemory::get_burst_sequence(transaction_type &trans)
{
  ocpip::burst_sequence *b_seq_ext;
  ocp_slave_socket.get_extension(b_seq_ext, trans);
  return b_seq_ext->value.sequence;
}

void OCPWrappedMemory::write_data (
    const unsigned int addr, 
    const unsigned int data, 
    const unsigned int data_byte_en)
{
    if (data_byte_en == 0xf)
    {
        m_memory[addr/4] = data;
    }
    else
    {
        unsigned int original_data = m_memory[addr/4];
        unsigned int temp_written_data = 0;
        unsigned int temp = 0;
        for(unsigned int j=0; j<4; j++)
        {
            if (((data_byte_en >> j) & 0x1) == 0x1)
            {
                temp = (data >> 8*j) & 0xff;
            }
            else
            {
                temp = (original_data >> 8*j) & 0xff;
            }
            temp_written_data = (temp << 8*j) | temp_written_data;
        }   
        m_memory[addr/4] = temp_written_data;
    }
}


unsigned int OCPWrappedMemory::read_data (const unsigned int addr) 
{
    if ( (addr & 0x3) != 0 )//not alinged address
    {
        cout<<"ERROR: ExtMemory::read_data( "<< addr <<" ), addr is not aligned"<<endl;
        exit(-1);
    }
    return m_memory[addr/4];
}
