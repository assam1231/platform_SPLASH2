#include "CrossbarTop.h"
#include "NocPayloadExtension.h"
#include "pe_mmap.h"

//#define EN_CROSSBAR_DEBUG
#define EN_TRACE
#include "CrossbarDebug.h"

CrossbarTop::CrossbarTop(
    sc_core::sc_module_name name, 
    const unsigned int channel_num,
    sc_core::sc_time cycle_time
)
    : sc_core::sc_module(name)
    , req_noc_peq_1("req_noc_peq_1")
    , rsp_ip_peq_1("rsp_ip_peq_1")
    , req_noc_peq_2("req_noc_peq_2")
    , rsp_ip_peq_2("rsp_ip_peq_2")
    , req_noc_peq_3("req_noc_peq_3")
    , rsp_ip_peq_3("rsp_ip_peq_3")
    , req_noc_peq_4("req_noc_peq_4")
    , rsp_ip_peq_4("rsp_ip_peq_4")
    , cycleTime_(cycle_time)
    , HEADER_LENGTH_IN_BYTE(32/8)
	, data_width_byte(32/8)
    
{
    // OCP initiator and target socket
    
    std::ostringstream oss;
    std::ostringstream oss1;
    for(unsigned int i = 0 ; i < 4 ; i++){
        oss << "Crossbar" << i << "initSocket";
        oss1 << "Crossbar" << i << "targetSocket";
        init_crossbar_socket[i] = new ocpip::ocp_master_socket_tl3<32, 1>(oss.str().c_str());
        target_crossbar_socket[i] = new ocpip::ocp_slave_socket_tl3<32, 1>(oss1.str().c_str());
        
        init_crossbar_socket[i]->set_ocp_config(ocp_parameters_);
        target_crossbar_socket[i]->set_ocp_config(ocp_parameters_);
    }
    
    (*init_crossbar_socket[0]).register_nb_transport_bw(this, &CrossbarTop::crossbar_1_bw);
    (*target_crossbar_socket[0]).register_nb_transport_fw(this, &CrossbarTop::req_crossbar_1_fw);
    (*init_crossbar_socket[1]).register_nb_transport_bw(this, &CrossbarTop::crossbar_2_bw);
    (*target_crossbar_socket[1]).register_nb_transport_fw(this, &CrossbarTop::req_crossbar_2_fw);
    (*init_crossbar_socket[2]).register_nb_transport_bw(this, &CrossbarTop::crossbar_3_bw);
    (*target_crossbar_socket[2]).register_nb_transport_fw(this, &CrossbarTop::req_crossbar_3_fw);
    (*init_crossbar_socket[3]).register_nb_transport_bw(this, &CrossbarTop::crossbar_4_bw);
    (*target_crossbar_socket[3]).register_nb_transport_fw(this, &CrossbarTop::req_crossbar_4_fw);
    
    //Crossbar
    crossbar = new Crossbar("crossbar", 4, cycleTime_);
    
    //between crossbar and noc FIFO
    
    for(unsigned int i = 0 ; i < 4 ; i++)
    {
        noc_to_sch[i] = new sc_fifo<tlm::tlm_generic_payload*>(1);
        sch_to_mem[i] = new sc_fifo<tlm::tlm_generic_payload*>(1);
        mem_to_sch[i] = new sc_fifo<tlm::tlm_generic_payload*>(1);
        sch_to_noc[i] = new sc_fifo<tlm::tlm_generic_payload*>(1);
    }
    for(unsigned int i = 0; i < 4; i++)
    {
        (*crossbar->in_if_tomem[i])(*noc_to_sch[i]);
        (*crossbar->out_if_tomem[i])(*sch_to_mem[i]);
        (*crossbar->in_if_tonoc[i])(*mem_to_sch[i]);
        (*crossbar->out_if_tonoc[i])(*sch_to_noc[i]);
    }
    
    SC_THREAD(noc_to_crb_0_process);
    SC_THREAD(crb_to_mem_0_process);
    SC_THREAD(mem_to_crb_0_process);
    SC_THREAD(crb_to_noc_0_process);
    
    SC_THREAD(noc_to_crb_1_process);
    SC_THREAD(crb_to_mem_1_process);
    SC_THREAD(mem_to_crb_1_process);
    SC_THREAD(crb_to_noc_1_process);
    
    SC_THREAD(noc_to_crb_2_process);
    SC_THREAD(crb_to_mem_2_process);
    SC_THREAD(mem_to_crb_2_process);
    SC_THREAD(crb_to_noc_2_process);
    
    SC_THREAD(noc_to_crb_3_process);
    SC_THREAD(crb_to_mem_3_process);
    SC_THREAD(mem_to_crb_3_process);
    SC_THREAD(crb_to_noc_3_process);
    

}

//////////////////// Crossbar Channel 1 //////////////////////////
//////////////////// Crossbar Channel 1 //////////////////////////


tlm::tlm_sync_enum 
CrossbarTop::req_crossbar_1_fw(
        tlm::tlm_generic_payload &noc_trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay)

{

    CROSSBAR_DEBUG(
        cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
        cout << "  got noc_trans in CrossbarTop_channel_1 req_crossbar_1_fw " << endl;
        cout << "  phase: " << phase << endl;
        cout << "  source: " << NocPayloadExt::get_source_id( noc_trans ) << endl;
        cout << "  noc_trans target address: " << dec << noc_trans.get_address() << endl << endl;
    );
    
    if(noc_trans.get_command() != tlm::TLM_WRITE_COMMAND)
    {
        std::ostringstream oss;
        oss << "CrossbarTop::req_crossbar_1_fw "
            << ": only write command is supported";
        throw std::runtime_error(oss.str().c_str());
    }
    
    if(phase == tlm::BEGIN_REQ)
    {
        req_noc_peq_1.notify(noc_trans, delay);
    }
    else if(phase == tlm::END_RESP)
    {
        noc_trans.set_response_status(tlm::TLM_OK_RESPONSE);
        noc_trans.release();
        return tlm::TLM_COMPLETED;
    }
    else
    {
        std::ostringstream oss;
        oss << "CrossbarTop::req_crossbar_1_fw "
            << ": got invalid PHASE";
        throw std::runtime_error(oss.str().c_str());
    }
    noc_trans.set_response_status( tlm::TLM_OK_RESPONSE );
    return tlm::TLM_ACCEPTED;

}


// Callback function. receive data from Crossbar


tlm::tlm_sync_enum 
CrossbarTop::crossbar_1_bw(
        tlm::tlm_generic_payload &trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay)
{
	
    if(phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
    {
        std::ostringstream oss;
        oss << "CrossbarTop::crossbar_1_bw " << " got wrong phases: " << phase;
        throw std::runtime_error(oss.str().c_str());
    }
    else if (phase == tlm::END_REQ) //end of request
    {
        otermAckEvent_1.notify(delay);
    }
    else if (phase == tlm::BEGIN_RESP)
    {
        rsp_ip_peq_1.notify(trans, delay); // put data to queue and ask queue to wait   
    }
    else 
    {
        std::ostringstream oss;
        oss << "CrossbarTop::crossbar_1_bw " << " got invalid phases: " << phase;
        throw std::runtime_error(oss.str().c_str());
    }

    trans.set_response_status( tlm::TLM_OK_RESPONSE );
    return tlm::TLM_COMPLETED;
}


void CrossbarTop::noc_to_crb_0_process()
{
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    tlm::tlm_sync_enum        retval;
    
    while(true)
    {
        noc_trans = req_noc_peq_1.get_next_transaction();
        while(noc_trans == NULL)
        {
            wait(req_noc_peq_1.get_event());
            noc_trans = req_noc_peq_1.get_next_transaction();
        }
        assert(noc_trans != NULL);
        
        CROSSBAR_DEBUG(
			trans = NocPayloadExt::get_ip_trans( noc_trans );
            cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got noc_trans in noc_to_crb_0_process" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
            cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
            cout << "  target: " << dec << noc_trans->get_address() << endl << endl;
        );
        
        phase = tlm::END_REQ;
        delay = cycleTime_ * ((noc_trans->get_data_length() / data_width_byte + 2));
        retval = (*target_crossbar_socket[0])->nb_transport_bw(*noc_trans, phase, delay);
        
        if(retval != tlm::TLM_COMPLETED)
        {
            std::ostringstream oss;
            oss << "CrossbarTop::noc_to_crb_0_process : Not supporting responses";
            throw std::runtime_error(oss.str().c_str());
        }
        
        TRACE(
            cout << "CRO FW ToC in CROSSBAR_1 ";
            tlm::tlm_generic_payload* trans = 0;
            trans = NocPayloadExt::get_ip_trans( noc_trans );
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;
         );
        
        while(true)
        {
            if((*noc_to_sch[0]).num_free() > 0)
            {
				CROSSBAR_DEBUG( cout << "  write noc_trans in noc_to_sch[0]" << endl << endl; );
                wait( cycleTime_ );
                (*noc_to_sch[0]).write(noc_trans);                   
                break;
            } 
            else 
            {
				//cout << "  wait_1_sc_time_stamp = " << sc_time_stamp() << endl;
                wait( SC_ZERO_TIME );
            }
        }
    }
}


void CrossbarTop::crb_to_mem_0_process()
{
    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    tlm::tlm_sync_enum        retval;
    
    while(true)
    {
        if((*sch_to_mem[0]).num_available() > 0)
        {
            noc_trans = (*sch_to_mem[0]).read();
			//wait( SC_ZERO_TIME );
        }
        else 
        {
            wait((*sch_to_mem[0]).data_written_event());
            noc_trans = (*sch_to_mem[0]).read();
			//wait( SC_ZERO_TIME );
        }
        
        trans = NocPayloadExt::get_ip_trans( noc_trans );
        unsigned int source_id = NocPayloadExt::get_source_id( noc_trans );
		
        
        CROSSBAR_DEBUG(
            cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got noc_trans in crb_to_mem_0_process" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
            cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
            cout << "  target: " << dec << noc_trans->get_address() << endl << endl;    
        );
        
        TRACE(
		cout << "CRO FW ToM in CROSSBAR_1 ";
        if ( trans->is_read() ) cout << "R ";
        else                    cout << "W ";
		cout << "Source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << " ";
		cout << "Target: " << noc_trans->get_address() << " ";
		cout << "sc_time_stamp= " << sc_time_stamp() << " "; // address delay
        cout << hex << "0x"<< trans->get_address() << endl;
        );
        
        if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY2_BEGIN){
            trans->set_address( trans->get_address() & 0xfffffff );
        } else if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY3_BEGIN){
            trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY2_BEGIN);
        } else if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY4_BEGIN){
            trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY3_BEGIN);
        } else {
            trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY4_BEGIN);
        }
        
        if ( trans->is_read() ) { //only read command has response: model m_writeresp_enable==false in OCP. 
            //rsp_tx_source_deque_1.push_back( source_id );
            mapping0.insert(std::pair<tlm::tlm_generic_payload*, int>(trans, source_id));
            rsp_tx_deque_event_1.notify();
        }
        
        noc_trans->release();
        
        phase = tlm::BEGIN_REQ;
        delay = SC_ZERO_TIME;
        retval = (*init_crossbar_socket[0])->nb_transport_fw(*trans, phase, delay);
        
        
        if ( trans->is_write() ) {
            if(retval == tlm::TLM_ACCEPTED || retval == tlm::TLM_UPDATED)
            {
                if(phase == tlm::BEGIN_REQ)
                {   
                    wait(otermAckEvent_1);      
                }
                else if(phase == tlm::END_REQ)
                {
                    std::ostringstream oss;
                    oss << "CrossbarTop::crb_to_mem_0_process : END_REQ not supported";
                    throw std::runtime_error(oss.str().c_str());
                }
                else if(phase == tlm::BEGIN_RESP)
                {
                    std::ostringstream oss;
                    oss << "CrossbarTop::crb_to_mem_0_process : BEGIN_RESP not supported";
                    throw std::runtime_error(oss.str().c_str());
                }
                else
                {
                    std::ostringstream oss;
                    oss << "CrossbarTop::crb_to_mem_0_process : invalid PHASE";
                    throw std::runtime_error(oss.str().c_str());
                }          
            }
            else
            {
                std::ostringstream oss;
                oss << "CrossbarTop::crb_to_mem_0_process : invalid SYNC_ENUM";
                throw std::runtime_error(oss.str().c_str());
            }
        }
    }
}


void CrossbarTop::mem_to_crb_0_process()
{
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    //tlm::tlm_sync_enum        retval;
    unsigned int              response_to = 0;
    
    while(true)
    {
        /*if( rsp_tx_source_deque_1.empty() ) 
        {
            wait(rsp_tx_deque_event_1); 
        }
        response_to = rsp_tx_source_deque_1.front();
        rsp_tx_source_deque_1.pop_front();*/
        
        trans = rsp_ip_peq_1.get_next_transaction();
        while(trans == NULL)
        { 
            wait(rsp_ip_peq_1.get_event());
            trans = rsp_ip_peq_1.get_next_transaction();
        }
        assert(trans != NULL);
        
        
        if( mapping0.empty() ) { wait(rsp_tx_deque_event_1); }
        response_to = mapping0[trans];
        mapping0.erase(trans);
        

        CROSSBAR_DEBUG(
            cout<<  "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got trans in mem_to_crb_0_process (read back)" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
        );
        
        /*phase = tlm::END_RESP;
        delay = cycleTime_; // Constant delay models the Network interface (NI)
        retval = (*init_crossbar_socket[0])->nb_transport_fw(*trans, phase, delay);
        
        if(retval != tlm::TLM_COMPLETED)
        {    
            std::ostringstream oss;
            oss << "CrossbarTop::mem_to_crb_0_process : invalid SYNC_ENUM";
            throw std::runtime_error(oss.str().c_str());
        }*/
        
        noc_trans = m_mm.allocate();
        noc_trans->set_streaming_width(32 / 8);
        noc_trans->set_write();
        noc_trans->set_extension( new ip_trans_ext(trans) );
        noc_trans->set_extension( new source_id_ext(response_to) );
        noc_trans->set_address( 16 ); 
        noc_trans->set_data_length( trans->get_data_length() + HEADER_LENGTH_IN_BYTE );
        
        TRACE(
            cout << "CRO BW ToC in CROSSBAR_1 ";
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( *noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;
        );
        
        while(true)
        {
            if((*mem_to_sch[0]).num_free() > 0)
            {   
				CROSSBAR_DEBUG( cout << "  write noc_trans in mem_to_sch[0]" << endl << endl; );
                (*mem_to_sch[0]).write(noc_trans);
				wait( SC_ZERO_TIME );
                break;
            } 
            else 
            {
				//cout << "  wait_1_sc_time_stamp = " << sc_time_stamp() << endl;
                wait( SC_ZERO_TIME );
            }
        }
    }
}



void CrossbarTop::crb_to_noc_0_process()
{

    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    tlm::tlm_sync_enum        retval;
    
    while(true)
    {
        if((*sch_to_noc[0]).num_available() > 0)
        {   
            noc_trans = (*sch_to_noc[0]).read();
			//wait( SC_ZERO_TIME );
        }
        else 
        {
            wait((*sch_to_noc[0]).data_written_event());
            noc_trans = (*sch_to_noc[0]).read();
			//wait( SC_ZERO_TIME );
        }
        
        trans = NocPayloadExt::get_ip_trans( noc_trans );
        
        CROSSBAR_DEBUG(
            cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got noc_trans in crb_to_noc_0_process" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
            cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
            cout << "  target: " << dec << noc_trans->get_address() << endl << endl;    
        );
        
        phase = tlm::BEGIN_RESP;
        delay = SC_ZERO_TIME ;
        retval = (*target_crossbar_socket[0])->nb_transport_bw(*trans, phase, delay);
        
        TRACE(
            cout << "CRO BW ToN in CROSSBAR_1 ";
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( *noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;
        );
        
        noc_trans->release();

        if(retval != tlm::TLM_COMPLETED)
        {
            std::ostringstream oss;
            oss << "CrossbarTop::crb_to_noc_0_process : invalid SYNC_ENUM";
            throw std::runtime_error(oss.str().c_str());
        }
    }
}

//////////////////// Crossbar Channel 1 //////////////////////////
//////////////////// Crossbar Channel 1 //////////////////////////

//////////////////// Crossbar Channel 2 //////////////////////////
//////////////////// Crossbar Channel 2 //////////////////////////



tlm::tlm_sync_enum 
CrossbarTop::req_crossbar_2_fw(
        tlm::tlm_generic_payload &noc_trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay)
        
{
    CROSSBAR_DEBUG(
        cout<<  "  sc_time_stamp = " << sc_time_stamp() << endl;
        cout << "  got noc_trans in CrossbarTop_channel_2 req_crossbar_2_fw " << endl;
        cout << "  phase: " << phase << endl;
        cout << "  source: " << NocPayloadExt::get_source_id( noc_trans ) << endl;
        cout << "  noc_trans target address: " << dec << noc_trans.get_address() << endl << endl;
    );
    
    if(noc_trans.get_command() != tlm::TLM_WRITE_COMMAND)
    {
        std::ostringstream oss;
        oss << "CrossbarTop::req_crossbar_2_fw "
            << ": only write command is supported";
        throw std::runtime_error(oss.str().c_str());
    }
    
    if(phase == tlm::BEGIN_REQ)
    {
        req_noc_peq_2.notify(noc_trans, delay);
    }
    else if(phase == tlm::END_RESP)
    {
        noc_trans.set_response_status(tlm::TLM_OK_RESPONSE);
        noc_trans.release();
        return tlm::TLM_COMPLETED;
    }
    else
    {
        std::ostringstream oss;
        oss << "CrossbarTop::req_crossbar_2_fw "
            << ": got invalid PHASE";
        throw std::runtime_error(oss.str().c_str());
    }
    noc_trans.set_response_status( tlm::TLM_OK_RESPONSE );
    return tlm::TLM_ACCEPTED;

}


// Callback function. receive data from Crossbar


tlm::tlm_sync_enum 
CrossbarTop::crossbar_2_bw(
        tlm::tlm_generic_payload &trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay)
{

    if(phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
    {
        std::ostringstream oss;
        oss << "CrossbarTop::crossbar_2_bw " << " got wrong phases: " << phase;
        throw std::runtime_error(oss.str().c_str());
    }
    else if (phase == tlm::END_REQ) //end of request
    {
        otermAckEvent_2.notify(delay);
    }
    else if (phase == tlm::BEGIN_RESP)
    {
        rsp_ip_peq_2.notify(trans, delay); // put data to queue and ask queue to wait
    }
    else 
    {
        std::ostringstream oss;
        oss << "CrossbarTop::crossbar_2_bw " << " got invalid phases: " << phase;
        throw std::runtime_error(oss.str().c_str());
    }

    trans.set_response_status( tlm::TLM_OK_RESPONSE );
    return tlm::TLM_COMPLETED;
}


void CrossbarTop::noc_to_crb_1_process()
{
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    tlm::tlm_sync_enum        retval;
    
    while(true)
    {
        noc_trans = req_noc_peq_2.get_next_transaction();
        while(noc_trans == NULL)
        { 
            wait(req_noc_peq_2.get_event());
            noc_trans = req_noc_peq_2.get_next_transaction();
        }
        assert(noc_trans != NULL);
        
        CROSSBAR_DEBUG(
			trans = NocPayloadExt::get_ip_trans( noc_trans );
            cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got noc_trans in noc_to_crb_1_process" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
            cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
            cout << "  target: " << dec << noc_trans->get_address() << endl;    
        );
        
        phase = tlm::END_REQ;
        delay = cycleTime_ * ((noc_trans->get_data_length() / data_width_byte + 2));
        retval = (*target_crossbar_socket[1])->nb_transport_bw(*noc_trans, phase, delay);
        
        if(retval != tlm::TLM_COMPLETED)
        {
            std::ostringstream oss;
            oss << "CrossbarTop::noc_to_crb_1_process : Not supporting responses";
            throw std::runtime_error(oss.str().c_str());
        }
        
        TRACE(
            cout << "CRO FW ToC in CROSSBAR_2 ";
            tlm::tlm_generic_payload* trans = 0;
            trans = NocPayloadExt::get_ip_trans( noc_trans );
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;
        );
        
        while(true)
        {
            if((*noc_to_sch[1]).num_free() > 0)
            {
				CROSSBAR_DEBUG( cout << "  write noc_trans in noc_to_sch[1]" << endl << endl; );
                wait( cycleTime_ );
                (*noc_to_sch[1]).write(noc_trans);
                break;
            } 
            else 
            {
                //cout << "  wait_2_sc_time_stamp = " << sc_time_stamp() << endl;
                wait( SC_ZERO_TIME );
            }
        }
    }
}


void CrossbarTop::crb_to_mem_1_process()
{
    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    tlm::tlm_sync_enum        retval;
    
    while(true)
    {
        if((*sch_to_mem[1]).num_available() > 0)
        {
            noc_trans = (*sch_to_mem[1]).read();
			//wait( SC_ZERO_TIME );
        }
        else 
        {
            wait((*sch_to_mem[1]).data_written_event());
            noc_trans = (*sch_to_mem[1]).read();
			//wait( SC_ZERO_TIME );
        }
        
        
        trans = NocPayloadExt::get_ip_trans( noc_trans );
        unsigned int source_id = NocPayloadExt::get_source_id( noc_trans );
        
        CROSSBAR_DEBUG(
            cout<<"init_sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got noc_trans in crb_to_mem_1_process" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
            cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
            cout << "  target: " << dec << noc_trans->get_address() << endl << endl;    
        );
        
        TRACE(
            cout << "CRO FW ToM in CROSSBAR_2 ";
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;
        );
        
        if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY2_BEGIN){
            trans->set_address( trans->get_address() & 0xfffffff );
        } else if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY3_BEGIN){
            trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY2_BEGIN);
        } else if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY4_BEGIN){
            trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY3_BEGIN);
        } else {
            trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY4_BEGIN);
        }
        
        if ( trans->is_read() ) { //only read command has response: model m_writeresp_enable==false in OCP. 
            //rsp_tx_source_deque_2.push_back( source_id );
            mapping1.insert(std::pair<tlm::tlm_generic_payload*, int>(trans, source_id));
            rsp_tx_deque_event_2.notify();
        }
        
        noc_trans->release();
        
        phase = tlm::BEGIN_REQ;
        delay = SC_ZERO_TIME;
        retval = (*init_crossbar_socket[1])->nb_transport_fw(*trans, phase, delay);
        
        
        if ( trans->is_write() ) {
            if(retval == tlm::TLM_ACCEPTED || retval == tlm::TLM_UPDATED)
            {
                if(phase == tlm::BEGIN_REQ)
                {   
                    wait(otermAckEvent_2);      
                }
                else if(phase == tlm::END_REQ)
                {
                    std::ostringstream oss;
                    oss << "CrossbarTop::crb_to_mem_1_process : END_REQ not supported";
                    throw std::runtime_error(oss.str().c_str());
                }
                else if(phase == tlm::BEGIN_RESP)
                {
                    std::ostringstream oss;
                    oss << "CrossbarTop::crb_to_mem_1_process : BEGIN_RESP not supported";
                    throw std::runtime_error(oss.str().c_str());
                }
                else
                {
                    std::ostringstream oss;
                    oss << "CrossbarTop::crb_to_mem_1_process : invalid PHASE";
                    throw std::runtime_error(oss.str().c_str());
                }          
            }
            else
            {
                std::ostringstream oss;
                oss << "CrossbarTop::crb_to_mem_1_process : invalid SYNC_ENUM";
                throw std::runtime_error(oss.str().c_str());
            }
        }
    }
}


void CrossbarTop::mem_to_crb_1_process()
{
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    //tlm::tlm_sync_enum        retval;
    unsigned int              response_to = 0;
    
    while(true)
    {
        /*if( rsp_tx_source_deque_2.empty() ) 
        {
            wait(rsp_tx_deque_event_2); 
        }
        response_to = rsp_tx_source_deque_2.front();
        rsp_tx_source_deque_2.pop_front();*/
        
        trans = rsp_ip_peq_2.get_next_transaction();
        while(trans == NULL)
        { 
            wait(rsp_ip_peq_2.get_event());
            trans = rsp_ip_peq_2.get_next_transaction();
        }
        assert(trans != NULL);
            
        if( mapping1.empty() ) { wait(rsp_tx_deque_event_2); }
        response_to = mapping1[trans];
        mapping1.erase(trans);
  
        CROSSBAR_DEBUG(
            cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got trans in mem_to_crb_1_process (read back)" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
        );
        
        /*phase = tlm::END_RESP;
        delay = cycleTime_; // Constant delay models the Network interface (NI)
        retval = (*init_crossbar_socket[1])->nb_transport_fw(*trans, phase, delay);
        
        if(retval != tlm::TLM_COMPLETED)
        {
            std::ostringstream oss;
            oss << "CrossbarTop::mem_to_crb_1_process : invalid SYNC_ENUM";
            throw std::runtime_error(oss.str().c_str());
        }*/
        
        noc_trans = m_mm.allocate();
        noc_trans->set_streaming_width(32 / 8);
        noc_trans->set_write();
        noc_trans->set_extension( new ip_trans_ext(trans) );
        noc_trans->set_extension( new source_id_ext(response_to) );
        noc_trans->set_address( 17 ); 
        noc_trans->set_data_length( trans->get_data_length() + HEADER_LENGTH_IN_BYTE );
        
        TRACE(
            cout << "CRO BW ToC in CROSSBAR_2 ";
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( *noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;
        ); 
  
        while(true)
        {
            if((*mem_to_sch[1]).num_free() > 0)
            {
				CROSSBAR_DEBUG( cout << "  write noc_trans in mem_to_sch[1]" << endl << endl; );
                (*mem_to_sch[1]).write(noc_trans);
				wait( SC_ZERO_TIME );
                break;
            } 
            else 
            {
                //cout << "  wait_2_sc_time_stamp = " << sc_time_stamp() << endl;
                wait( SC_ZERO_TIME );
            }
        }
    }
}



void CrossbarTop::crb_to_noc_1_process()
{

    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    tlm::tlm_sync_enum        retval;
    
    while(true)
    {
        if((*sch_to_noc[1]).num_available() > 0)
        {   
            noc_trans = (*sch_to_noc[1]).read();
			//wait( SC_ZERO_TIME );
        }
        else 
        {
            wait((*sch_to_noc[1]).data_written_event());
            noc_trans = (*sch_to_noc[1]).read();
			//wait( SC_ZERO_TIME );
        }
        
        trans = NocPayloadExt::get_ip_trans( noc_trans );
        
        CROSSBAR_DEBUG(
            cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got noc_trans in crb_to_noc_1_process" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
            cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
            cout << "  target: " << dec << noc_trans->get_address() << endl << endl;    
        );
        
        phase = tlm::BEGIN_RESP;
        delay = SC_ZERO_TIME;
        retval = (*target_crossbar_socket[1])->nb_transport_bw(*trans, phase, delay);
        
        TRACE(
            cout << "CRO BW ToN in CROSSBAR_2 ";
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( *noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;
		);
        
        noc_trans->release();

        if(retval != tlm::TLM_COMPLETED)
        {
            std::ostringstream oss;
            oss << "CrossbarTop::crb_to_noc_1_process : invalid SYNC_ENUM";
            throw std::runtime_error(oss.str().c_str());
        }
    }
}


//////////////////// Crossbar Channel 2 //////////////////////////
//////////////////// Crossbar Channel 2 //////////////////////////


//////////////////// Crossbar Channel 3 //////////////////////////
//////////////////// Crossbar Channel 3 //////////////////////////



tlm::tlm_sync_enum 
CrossbarTop::req_crossbar_3_fw(
        tlm::tlm_generic_payload &noc_trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay)
        
{
    CROSSBAR_DEBUG(
        cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
        cout << "  got noc_trans in CrossbarTop_channel_3 req_crossbar_3_fw " << endl;
        cout << "  phase: " << phase << endl;
        cout << "  source: " << NocPayloadExt::get_source_id( noc_trans ) << endl;
        cout << "  noc_trans target address: " << dec << noc_trans.get_address() << endl << endl;
    );
	
    if(noc_trans.get_command() != tlm::TLM_WRITE_COMMAND)
    {
        std::ostringstream oss;
        oss << "CrossbarTop::req_crossbar_3_fw "
            << ": only write command is supported";
        throw std::runtime_error(oss.str().c_str());
    }
    
    if(phase == tlm::BEGIN_REQ)
    {
        req_noc_peq_3.notify(noc_trans, delay);
    }
    else if(phase == tlm::END_RESP)
    {
        noc_trans.set_response_status(tlm::TLM_OK_RESPONSE);
        noc_trans.release();
        return tlm::TLM_COMPLETED;
    }
    else
    {
        std::ostringstream oss;
        oss << "CrossbarTop::req_crossbar_3_fw "
            << ": got invalid PHASE";
        throw std::runtime_error(oss.str().c_str());
    }
    noc_trans.set_response_status( tlm::TLM_OK_RESPONSE );
    return tlm::TLM_ACCEPTED;

}


// Callback function. receive data from Crossbar


tlm::tlm_sync_enum 
CrossbarTop::crossbar_3_bw(
        tlm::tlm_generic_payload &trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay)
{
	
    if(phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
    {
        std::ostringstream oss;
        oss << "CrossbarTop::crossbar_3_bw " << " got wrong phases: " << phase;
        throw std::runtime_error(oss.str().c_str());
    }
    else if (phase == tlm::END_REQ) //end of request
    {
        otermAckEvent_3.notify(delay);
    }
    else if (phase == tlm::BEGIN_RESP)
    {
        rsp_ip_peq_3.notify(trans, delay); // put data to queue and ask queue to wait
    }
    else 
    {
        std::ostringstream oss;
        oss << "CrossbarTop::crossbar_3_bw " << " got invalid phases: " << phase;
        throw std::runtime_error(oss.str().c_str());
    }

    trans.set_response_status( tlm::TLM_OK_RESPONSE );
    return tlm::TLM_COMPLETED;
}


void CrossbarTop::noc_to_crb_2_process()
{
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    tlm::tlm_sync_enum        retval;
    
    while(true)
    {
        noc_trans = req_noc_peq_3.get_next_transaction();
        while(noc_trans == NULL)
        { 
            wait(req_noc_peq_3.get_event());
            noc_trans = req_noc_peq_3.get_next_transaction();
        }
        assert(noc_trans != NULL);
        
        CROSSBAR_DEBUG(
			trans = NocPayloadExt::get_ip_trans( noc_trans );
            cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got noc_trans in noc_to_crb_2_process" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
            cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
            cout << "  target: " << dec << noc_trans->get_address() << endl;    
        );
        
        phase = tlm::END_REQ;
        delay = cycleTime_ * ((noc_trans->get_data_length() / data_width_byte + 2));
        retval = (*target_crossbar_socket[2])->nb_transport_bw(*noc_trans, phase, delay);
        
        if(retval != tlm::TLM_COMPLETED)
        {
            std::ostringstream oss;
            oss << "CrossbarTop::noc_to_crb_2_process : Not supporting responses";
            throw std::runtime_error(oss.str().c_str());
        }
        
        TRACE(
            cout << "CRO FW ToC in CROSSBAR_3 ";
            tlm::tlm_generic_payload* trans = 0;
            trans = NocPayloadExt::get_ip_trans( noc_trans );
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;
        );
        
        while(true)
        {
            if((*noc_to_sch[2]).num_free() > 0)
            {
				CROSSBAR_DEBUG( cout << "  write noc_trans in noc_to_sch[2]" << endl << endl; );
                wait( cycleTime_ );
                (*noc_to_sch[2]).write(noc_trans);
                break;
            } 
            else 
            {
                //cout << "  wait_3_sc_time_stamp = " << sc_time_stamp() << endl;
                wait( SC_ZERO_TIME );
            }
        }
    }
}


void CrossbarTop::crb_to_mem_2_process()
{
    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    tlm::tlm_sync_enum        retval;
    
    while(true)
    {
        if((*sch_to_mem[2]).num_available() > 0)
        {
            noc_trans = (*sch_to_mem[2]).read();
			//wait( SC_ZERO_TIME );
        }
        else 
        {
            wait((*sch_to_mem[2]).data_written_event());
            noc_trans = (*sch_to_mem[2]).read();
			//wait( SC_ZERO_TIME );
        }
        
        trans = NocPayloadExt::get_ip_trans( noc_trans );
        unsigned int source_id = NocPayloadExt::get_source_id( noc_trans );

        CROSSBAR_DEBUG(
            cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got noc_trans in crb_to_mem_2_process" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
            cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
            cout << "  target: " << dec << noc_trans->get_address() << endl << endl;    
        );
        
        TRACE(
            cout << "CRO FW ToM in CROSSBAR_3 ";
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;
        );
        
        if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY2_BEGIN){
            trans->set_address( trans->get_address() & 0xfffffff );
        } else if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY3_BEGIN){
            trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY2_BEGIN);
        } else if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY4_BEGIN){
            trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY3_BEGIN);
        } else {
            trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY4_BEGIN);
        }
        
        if ( trans->is_read() ) { //only read command has response: model m_writeresp_enable==false in OCP. 
            //rsp_tx_source_deque_3.push_back( source_id );
            mapping2.insert(std::pair<tlm::tlm_generic_payload*, int>(trans, source_id));
            rsp_tx_deque_event_3.notify();
        }
        
        noc_trans->release();
        
        phase = tlm::BEGIN_REQ;
        delay = SC_ZERO_TIME;
        retval = (*init_crossbar_socket[2])->nb_transport_fw(*trans, phase, delay);
        
        
        if ( trans->is_write() ) {
            if(retval == tlm::TLM_ACCEPTED || retval == tlm::TLM_UPDATED)
            {
                if(phase == tlm::BEGIN_REQ)
                {   
                    wait(otermAckEvent_3);      
                }
                else if(phase == tlm::END_REQ)
                {
                    std::ostringstream oss;
                    oss << "CrossbarTop::crb_to_mem_2_process : END_REQ not supported";
                    throw std::runtime_error(oss.str().c_str());
                }
                else if(phase == tlm::BEGIN_RESP)
                {
                    std::ostringstream oss;
                    oss << "CrossbarTop::crb_to_mem_2_process : BEGIN_RESP not supported";
                    throw std::runtime_error(oss.str().c_str());
                }
                else
                {
                    std::ostringstream oss;
                    oss << "CrossbarTop::crb_to_mem_2_process : invalid PHASE";
                    throw std::runtime_error(oss.str().c_str());
                }          
            }
            else
            {
                std::ostringstream oss;
                oss << "CrossbarTop::crb_to_mem_2_process : invalid SYNC_ENUM";
                throw std::runtime_error(oss.str().c_str());
            }
        }
    }
}


void CrossbarTop::mem_to_crb_2_process()
{
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    //tlm::tlm_sync_enum        retval;
    unsigned int              response_to = 0;
    
    while(true)
    {
        /*if( rsp_tx_source_deque_3.empty() ) 
        {
            wait(rsp_tx_deque_event_3); 
        }
        response_to = rsp_tx_source_deque_3.front();
        rsp_tx_source_deque_3.pop_front();*/
        
        trans = rsp_ip_peq_3.get_next_transaction();
        while(trans == NULL)
        { 
            wait(rsp_ip_peq_3.get_event());
            trans = rsp_ip_peq_3.get_next_transaction();
        }
        assert(trans != NULL);
        
        if( mapping2.empty() ) { wait(rsp_tx_deque_event_3); }
        response_to = mapping2[trans];
        mapping2.erase(trans);
        
        CROSSBAR_DEBUG(
            cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got trans in mem_to_crb_2_process (read back)" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
        );
        
       /*phase = tlm::END_RESP;
        delay = cycleTime_; // Constant delay models the Network interface (NI)
        retval = (*init_crossbar_socket[2])->nb_transport_fw(*trans, phase, delay);
        
        if(retval != tlm::TLM_COMPLETED)
        {
            std::ostringstream oss;
            oss << "CrossbarTop::mem_to_crb_2_process : invalid SYNC_ENUM";
            throw std::runtime_error(oss.str().c_str());
        }*/
        
        noc_trans = m_mm.allocate();
        noc_trans->set_streaming_width(32 / 8);
        noc_trans->set_write();
        noc_trans->set_extension( new ip_trans_ext(trans) );
        noc_trans->set_extension( new source_id_ext(response_to) );
        noc_trans->set_address( 18 ); 
        noc_trans->set_data_length( trans->get_data_length() + HEADER_LENGTH_IN_BYTE );
       
        TRACE(
            cout << "CRO BW ToC in CROSSBAR_3 ";
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( *noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;
        );
       
        while(true)
        {
            if((*mem_to_sch[2]).num_free() > 0)
            {
				CROSSBAR_DEBUG( cout << "  write noc_trans in mem_to_sch[2]" << endl << endl; );
                (*mem_to_sch[2]).write(noc_trans);
				wait( SC_ZERO_TIME );
                break;
            } 
            else 
            {
                //cout << "  wait_3_sc_time_stamp = " << sc_time_stamp() << endl;
                wait( SC_ZERO_TIME );
            }
        }
    }
}



void CrossbarTop::crb_to_noc_2_process()
{

    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    tlm::tlm_sync_enum        retval;
    
    while(true)
    {
        if((*sch_to_noc[2]).num_available() > 0)
        {   
            noc_trans = (*sch_to_noc[2]).read();
			//wait( SC_ZERO_TIME );
        }
        else 
        {
            wait((*sch_to_noc[2]).data_written_event());
            noc_trans = (*sch_to_noc[2]).read();
			//wait( SC_ZERO_TIME );
        }
        
        trans = NocPayloadExt::get_ip_trans( noc_trans );
        
        CROSSBAR_DEBUG(
            cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got noc_trans in crb_to_noc_2_process" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
            cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
            cout << "  target: " << dec << noc_trans->get_address() << endl << endl;    
        );

		TRACE(	
            cout << "CRO BW ToN in CROSSBAR_3 ";
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( *noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;
		);
        
        phase = tlm::BEGIN_RESP;
        delay = SC_ZERO_TIME;
        retval = (*target_crossbar_socket[2])->nb_transport_bw(*trans, phase, delay);
        
        noc_trans->release();

        if(retval != tlm::TLM_COMPLETED)
        {
            std::ostringstream oss;
            oss << "CrossbarTop::crb_to_noc_2_process : invalid SYNC_ENUM";
            throw std::runtime_error(oss.str().c_str());
        }
    }
}


//////////////////// Crossbar Channel 3 //////////////////////////
//////////////////// Crossbar Channel 3 //////////////////////////


//////////////////// Crossbar Channel 4 //////////////////////////
//////////////////// Crossbar Channel 4 //////////////////////////


tlm::tlm_sync_enum 
CrossbarTop::req_crossbar_4_fw(
        tlm::tlm_generic_payload &noc_trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay)
        
{
    CROSSBAR_DEBUG(
        cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
        cout << "  got noc_trans in CrossbarTop_channel_4 req_crossbar_4_fw " << endl;
        cout << "  phase: " << phase << endl;
        cout << "  source: " << NocPayloadExt::get_source_id( noc_trans ) << endl;
        cout << "  noc_trans target address: " << dec << noc_trans.get_address() << endl << endl;
    );
    
    if(noc_trans.get_command() != tlm::TLM_WRITE_COMMAND)
    {
        std::ostringstream oss;
        oss << "CrossbarTop::req_crossbar_4_fw "
            << ": only write command is supported";
        throw std::runtime_error(oss.str().c_str());
    }
    
    if(phase == tlm::BEGIN_REQ)
    {
        req_noc_peq_4.notify(noc_trans, delay);
    }
    else if(phase == tlm::END_RESP)
    {
        noc_trans.set_response_status(tlm::TLM_OK_RESPONSE);
        noc_trans.release();
        return tlm::TLM_COMPLETED;
    }
    else
    {
        std::ostringstream oss;
        oss << "CrossbarTop::req_crossbar_4_fw "
            << ": got invalid PHASE";
        throw std::runtime_error(oss.str().c_str());
    }
    noc_trans.set_response_status( tlm::TLM_OK_RESPONSE );
    return tlm::TLM_ACCEPTED;

}


// Callback function. receive data from Crossbar


tlm::tlm_sync_enum 
CrossbarTop::crossbar_4_bw(
        tlm::tlm_generic_payload &trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay)
{
    if(phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
    {
        std::ostringstream oss;
        oss << "CrossbarTop::crossbar_4_bw " << " got wrong phases: " << phase;
        throw std::runtime_error(oss.str().c_str());
    }
    else if (phase == tlm::END_REQ) //end of request
    {
        otermAckEvent_4.notify(delay);
    }
    else if (phase == tlm::BEGIN_RESP)
    {
        rsp_ip_peq_4.notify(trans, delay); // put data to queue and ask queue to wait
    }
    else 
    {
        std::ostringstream oss;
        oss << "CrossbarTop::crossbar_4_bw " << " got invalid phases: " << phase;
        throw std::runtime_error(oss.str().c_str());
    }

    trans.set_response_status( tlm::TLM_OK_RESPONSE );
    return tlm::TLM_COMPLETED;
}


void CrossbarTop::noc_to_crb_3_process()
{
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    tlm::tlm_sync_enum        retval;
    
    while(true)
    {
        noc_trans = req_noc_peq_4.get_next_transaction();
        while(noc_trans == NULL)
        { 
            wait(req_noc_peq_4.get_event());
            noc_trans = req_noc_peq_4.get_next_transaction();
        }
        assert(noc_trans != NULL);
        
        CROSSBAR_DEBUG(
			trans = NocPayloadExt::get_ip_trans( noc_trans );
            cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got noc_trans in noc_to_crb_3_process" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
            cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
            cout << "  target: " << dec << noc_trans->get_address() << endl;  
        );
        
        phase = tlm::END_REQ;
        delay = cycleTime_ * ((noc_trans->get_data_length() / data_width_byte + 2));
        retval = (*target_crossbar_socket[3])->nb_transport_bw(*noc_trans, phase, delay);
        
        if(retval != tlm::TLM_COMPLETED)
        {
            std::ostringstream oss;
            oss << "CrossbarTop::noc_to_crb_3_process : Not supporting responses";
            throw std::runtime_error(oss.str().c_str());
        }
        
        TRACE(
            cout << "CRO FW ToC in CROSSBAR_4 ";
            tlm::tlm_generic_payload* trans = 0;
            trans = NocPayloadExt::get_ip_trans( noc_trans );
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;
        );

        while(true)
        {
            if((*noc_to_sch[3]).num_free() > 0)
            {
				CROSSBAR_DEBUG( cout << "  write noc_trans in noc_to_sch[3]" << endl << endl; );
                wait( cycleTime_ );
                (*noc_to_sch[3]).write(noc_trans);    
                break;
            } 
            else 
            {
                //cout << "  wait_4_sc_time_stamp = " << sc_time_stamp() << endl;
                wait( SC_ZERO_TIME );
            }
        }
    }
}


void CrossbarTop::crb_to_mem_3_process()
{
    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    tlm::tlm_sync_enum        retval;
    
    while(true)
    {
        if((*sch_to_mem[3]).num_available() > 0)
        {
            noc_trans = (*sch_to_mem[3]).read();
			//wait( SC_ZERO_TIME );
        }
        else 
        {
            wait((*sch_to_mem[3]).data_written_event());
            noc_trans = (*sch_to_mem[3]).read();
			//wait( SC_ZERO_TIME );
        }
        
        trans = NocPayloadExt::get_ip_trans( noc_trans );
        unsigned int source_id = NocPayloadExt::get_source_id( noc_trans );

        CROSSBAR_DEBUG(
            cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got noc_trans in crb_to_mem_3_process" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
            cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
            cout << "  target: " << dec << noc_trans->get_address() << endl << endl;    
        );
        
        TRACE(
            cout << "CRO FW ToM in CROSSBAR_3 ";
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;
		);
        
        if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY2_BEGIN){
            trans->set_address( trans->get_address() & 0xfffffff );
        } else if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY3_BEGIN){
            trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY2_BEGIN);
        } else if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY4_BEGIN){
            trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY3_BEGIN);
        } else {
            trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY4_BEGIN);
        }
        
        if ( trans->is_read() ) { //only read command has response: model m_writeresp_enable==false in OCP. 
            //rsp_tx_source_deque_4.push_back( source_id );
            mapping3.insert(std::pair<tlm::tlm_generic_payload*, int>(trans, source_id));
            rsp_tx_deque_event_4.notify();
        }
        
        noc_trans->release();
        
        phase = tlm::BEGIN_REQ;
        delay = SC_ZERO_TIME;
        retval = (*init_crossbar_socket[3])->nb_transport_fw(*trans, phase, delay);
        
        
        if ( trans->is_write() ) {
            if(retval == tlm::TLM_ACCEPTED || retval == tlm::TLM_UPDATED)
            {
                if(phase == tlm::BEGIN_REQ)
                {   
                    wait(otermAckEvent_4);      
                }
                else if(phase == tlm::END_REQ)
                {
                    std::ostringstream oss;
                    oss << "CrossbarTop::crb_to_mem_3_process : END_REQ not supported";
                    throw std::runtime_error(oss.str().c_str());
                }
                else if(phase == tlm::BEGIN_RESP)
                {
                    std::ostringstream oss;
                    oss << "CrossbarTop::crb_to_mem_3_process : BEGIN_RESP not supported";
                    throw std::runtime_error(oss.str().c_str());
                }
                else
                {
                    std::ostringstream oss;
                    oss << "CrossbarTop::crb_to_mem_3_process : invalid PHASE";
                    throw std::runtime_error(oss.str().c_str());
                }          
            }
            else
            {
                std::ostringstream oss;
                oss << "CrossbarTop::crb_to_mem_3_process : invalid SYNC_ENUM";
                throw std::runtime_error(oss.str().c_str());
            }
        }
    }
}


void CrossbarTop::mem_to_crb_3_process()
{
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    //tlm::tlm_sync_enum        retval;
    unsigned int              response_to = 0;
    
    while(true)
    {
        /*if( rsp_tx_source_deque_4.empty() ) 
        {
            wait(rsp_tx_deque_event_4); 
        }
        response_to = rsp_tx_source_deque_4.front();
        rsp_tx_source_deque_4.pop_front();*/
        
        trans = rsp_ip_peq_4.get_next_transaction();
        while(trans == NULL)
        { 
            wait(rsp_ip_peq_4.get_event());
            trans = rsp_ip_peq_4.get_next_transaction();
        }
        assert(trans != NULL);
        
        CROSSBAR_DEBUG(
            cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got trans in mem_to_crb_3_process (read back)" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
        );
        
        if( mapping3.empty() ) { wait(rsp_tx_deque_event_4); }
        response_to = mapping3[trans];
        mapping3.erase(trans);
        
        /*phase = tlm::END_RESP;
        delay = cycleTime_; // Constant delay models the Network interface (NI)
        retval = (*init_crossbar_socket[3])->nb_transport_fw(*trans, phase, delay);
        
        if(retval != tlm::TLM_COMPLETED)
        {
            std::ostringstream oss;
            oss << "CrossbarTop::mem_to_crb_3_process : invalid SYNC_ENUM";
            throw std::runtime_error(oss.str().c_str());
        }*/
        
        noc_trans = m_mm.allocate();
        noc_trans->set_streaming_width(32 / 8);
        noc_trans->set_write();
        noc_trans->set_extension( new ip_trans_ext(trans) );
        noc_trans->set_extension( new source_id_ext(response_to) );
        noc_trans->set_address( 19 ); 
        noc_trans->set_data_length( trans->get_data_length() + HEADER_LENGTH_IN_BYTE );

        TRACE(
            cout << "CRO BW ToC in CROSSBAR_4 ";
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( *noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;
        );
        
        while(true)
        {
            if((*mem_to_sch[3]).num_free() > 0)
            {
				CROSSBAR_DEBUG( cout << "  write noc_trans in mem_to_sch[3]" << endl << endl; );
                (*mem_to_sch[3]).write(noc_trans);       
				wait( SC_ZERO_TIME );
                break;
            } 
            else 
            {
                //cout << "  wait_4_sc_time_stamp = " << sc_time_stamp() << endl;
                wait( SC_ZERO_TIME );
            }
        }
    }
}


void CrossbarTop::crb_to_noc_3_process()
{

    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    tlm::tlm_sync_enum        retval;
    
    while(true)
    {
        if((*sch_to_noc[3]).num_available() > 0)
        {   
            noc_trans = (*sch_to_noc[3]).read();
			//wait( SC_ZERO_TIME );
        }
        else 
        {
            wait((*sch_to_noc[3]).data_written_event());
            noc_trans = (*sch_to_noc[3]).read();
			//wait( SC_ZERO_TIME );
        }
        
        trans = NocPayloadExt::get_ip_trans( noc_trans );
        
        CROSSBAR_DEBUG(
            cout << "  sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  got noc_trans in crb_to_noc_3_process" << endl;
            cout << "  cmd: " << trans->get_command() << endl;
            cout << "  address: " << hex << trans->get_address() << endl;
            cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
            cout << "  target: " << dec << noc_trans->get_address() << endl << endl;    
        );
        
        TRACE(
            cout << "CRO BW ToN in CROSSBAR_4 ";
            if ( trans->is_read() ) cout << "R ";
            else                    cout << "W ";
            cout << "Source: " << dec << NocPayloadExt::get_source_id( *noc_trans ) << " ";
            cout << "Target: " << noc_trans->get_address() << " ";
            cout << "sc_time_stamp= " << sc_time_stamp() << " ";
            cout << hex << "0x"<< trans->get_address() << endl;            
        );
        
        phase = tlm::BEGIN_RESP;
        delay = SC_ZERO_TIME;
        retval = (*target_crossbar_socket[3])->nb_transport_bw(*trans, phase, delay);
        
        noc_trans->release();

        if(retval != tlm::TLM_COMPLETED)
        {
            std::ostringstream oss;
            oss << "CrossbarTop::crb_to_noc_3_process : invalid SYNC_ENUM";
            throw std::runtime_error(oss.str().c_str());
        }
    }
}

CrossbarTop::
~CrossbarTop()
{
}

//////////////////// Crossbar Channel 4 //////////////////////////
//////////////////// Crossbar Channel 4 //////////////////////////
