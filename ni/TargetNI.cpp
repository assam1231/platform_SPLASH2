#include "TargetNI.h"
#include "pe_mmap.h"
#include "NocPayloadExtension.h"

#include "OCPWrappedMemory.h"

//#define EN_TARGET_NI_DEBUG

//#define EN_CROSSBAR_DEBUG
#define EN_TRACE
#include "TargetNIDebug.h"

namespace ispa
{

template<unsigned int data_width_g> 
TargetNI<data_width_g>::
TargetNI(sc_core::sc_module_name name, int id, SimpleAddressMap *map, sc_core::sc_time cycle_time)
  : sc_core::sc_module(name)
  , initSocket(initSocket_name().c_str())
  , req_noc_peq_(req_noc_peq_name().c_str())
  , rsp_ip_peq_(rsp_ip_peq_name().c_str()) 
  , HEADER_LENGTH_IN_BYTE( data_width_g/8 ) 
  , m_mask_(0x1ffffull) //no external memory now
  //, m_mask_(0xffffffffull)
  , id_(id)
  , cycleTime_(cycle_time) // 10ns = 100 MHz
  
{
  initSocket.set_ocp_config(ocp_parameters_);
  //register callback functions for sockets
  initSocket.register_nb_transport_bw  (this, &TargetNI::oterm_ni_bw);  // give data to PEs
  rsp_init_.register_nb_transport_bw   (this, &TargetNI::rsp_rou_ni_bw); 
  req_target_.register_nb_transport_fw (this, &TargetNI::req_noc_ni_fw);
  SC_THREAD(req_rx_ni_thread);
  SC_THREAD(rsp_tx_ni_thread);
}

/*
 * Callback function. Output terminal (oterm) acknowledges
 * the receiving NI that data was accepted.
 */
template<unsigned int data_width_g>
tlm::tlm_sync_enum 
TargetNI<data_width_g>::
oterm_ni_bw(
        tlm::tlm_generic_payload &trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay)
{
        CROSSBAR_DEBUG(
			cout << "  bw_sc_time_stamp = " << sc_time_stamp() << endl;
            cout << "  In Target oterm_ni_bw " << endl;
            cout << "  cmd: " << trans.get_command() << endl;
            cout << "  phase: " << phase << endl;
            cout << "  TargetNI_id: " << dec << id_ << endl;
            cout << "  targetNI_trans_target: " << hex << trans.get_address() << endl << endl;
        );
		
    if(phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
    {
        std::ostringstream oss;
        oss << "TlmMesh2D::oterm_ni_bw " << id_ << " got wrong phases: " << phase;
        throw std::runtime_error(oss.str().c_str());
    }
    else if (phase == tlm::END_REQ) //end of request
    {
        otermAckEvent_.notify(delay);
    }
    else if (phase == tlm::BEGIN_RESP)
    {
		//cout<<"rsp_bw_sc_time_stamp = " << sc_time_stamp() << endl;
        rsp_ip_peq_.notify(trans, delay); // put data to queue and ask queue to wait
    }
    else 
    {
        std::ostringstream oss;
        oss << "TlmMesh2D::oterm_ni_bw " << id_ << " got invalid phases: " << phase;
        throw std::runtime_error(oss.str().c_str());
    }

    trans.set_response_status( tlm::TLM_OK_RESPONSE );
    return tlm::TLM_COMPLETED;
}


template<unsigned int data_width_g>
tlm::tlm_sync_enum 
TargetNI<data_width_g>::
rsp_rou_ni_bw(
        tlm::tlm_generic_payload &noc_trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay)
{
    if(phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
    {
        std::ostringstream oss;
        oss << "TlmMesh2D::rsp_rou_ni_bw " << id_ << " got wrong phase";
        throw std::runtime_error(oss.str().c_str());
    }

    rsp_rouAckEvent_.notify(delay);

    noc_trans.set_response_status( tlm::TLM_OK_RESPONSE );
    noc_trans.release(); 
    return tlm::TLM_COMPLETED;
}



/*
 * Callback function. Puts the data from router's IP output
 * into RX peq of network interface (NI)
 */
template<unsigned int data_width_g>
tlm::tlm_sync_enum 
TargetNI<data_width_g>::
req_noc_ni_fw(
        tlm::tlm_generic_payload &noc_trans,
        tlm::tlm_phase           &phase,
        sc_core::sc_time         &delay)
{
    CROSSBAR_DEBUG(
		cout << "  target_sc_time_stamp = " << sc_time_stamp() << endl;
        tlm::tlm_generic_payload* trans = 0;
        trans = NocPayloadExt::get_ip_trans( noc_trans );
        cout << "  In Target req_noc_ni_fw " << endl;
        cout << "  TargetNI_id: " << dec << id_ << endl;
        cout << "  cmd: " << trans->get_command() << endl;
        cout << "  phase: " << phase << endl;
        cout << "  targetNI_noc_trans_target: " << dec << noc_trans.get_address() << endl << endl;
    );
    
    // Only write command is supported
    if(noc_trans.get_command() != tlm::TLM_WRITE_COMMAND)
    {
        std::ostringstream oss;
        oss << "TlmMesh2D::req_noc_ni_fw " << id_
            << ": only write command is supported";
        throw std::runtime_error(oss.str().c_str());
    }

    if(phase == tlm::BEGIN_REQ)
    {
        noc_trans.acquire();
        req_noc_peq_.notify(noc_trans, delay);
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
        oss << "TlmMesh2D::req_noc_ni_fw " << id_
            << ": got invalid PHASE";
        throw std::runtime_error(oss.str().c_str());
    }
    noc_trans.set_response_status( tlm::TLM_OK_RESPONSE );
    return tlm::TLM_ACCEPTED;
}


/*
 * Models the network interface (NI) at the receiver (rx
 * side). Gets data from rx peq and puts it into init socket
 * (output terminal).
 */
template<unsigned int data_width_g>
void 
TargetNI<data_width_g>::
req_rx_ni_thread()
{
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_generic_payload* noc_trans = 0;
    //NetworkPayload* received_payload = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    tlm::tlm_sync_enum        retval;
    
    
    while(true)
    {       // Check for pending transactions in RX peq
        /*if((noc_trans = req_noc_peq_.get_next_transaction()) == 0)
        { 
            wait(req_noc_peq_.get_event());
            noc_trans = req_noc_peq_.get_next_transaction();
        }*/
        
        noc_trans = req_noc_peq_.get_next_transaction();
        while(noc_trans == NULL)
        { 
            wait(req_noc_peq_.get_event());
            noc_trans = req_noc_peq_.get_next_transaction();
        }
        assert(noc_trans != NULL);
        
        
    trans = NocPayloadExt::get_ip_trans( noc_trans );
    unsigned int source_id = NocPayloadExt::get_source_id( noc_trans );
    
    /*  
    if (trans->get_address() < pe_global_mmap::GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN) {
          trans->set_address( trans->get_address() & m_mask_ );
    } else {
          trans->set_address( trans->get_address() & 0xfffffff );
    }
    */  
#ifdef NTS_4_DRAMSIM2_crossbar
    // access each memory address should be redefine crossbar  by along
    if (trans->get_address() < pe_global_mmap::GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN) {
        trans->set_address( trans->get_address() & m_mask_ );
    }
#endif

#ifdef NTS_4_DRAMSIM2
    if (trans->get_address() < pe_global_mmap::GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN) 
    {
        trans->set_address( trans->get_address() & m_mask_ );
    } 
    else if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY2_BEGIN)
    {
        trans->set_address( trans->get_address() & 0xfffffff );
    } 
    else if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY3_BEGIN)
    {
        trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY2_BEGIN);
    }
    else if(trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY4_BEGIN)
    {
        trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY3_BEGIN);
    } 
    else {
        trans->set_address( trans->get_address() - pe_global_mmap::EXTERNAL_MEMORY4_BEGIN);
    }
#endif


#ifdef NTS_DRAMSIM2
     if (trans->get_address() < pe_global_mmap::GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN) {
          trans->set_address( trans->get_address() & m_mask_ );
    } else {
          trans->set_address( trans->get_address() & 0xfffffff );
    }
#endif
        
        if ( trans->is_read() ) { //only read command has response: model m_writeresp_enable==false in OCP. 
            //rsp_tx_source_deque_.push_back( source_id );
            trans_sourceID_map_.insert(std::pair<tlm::tlm_generic_payload*, int>(trans, source_id));
            rsp_tx_deque_event_.notify();
        }
        
        
        // Send Acknowledge 
        phase = tlm::END_REQ; 
        delay = cycleTime_ * ((noc_trans->get_data_length() / (data_width_g/8) + 3));
        retval = req_target_->nb_transport_bw(*noc_trans, phase, delay);

        if(retval != tlm::TLM_COMPLETED)
        {
            std::ostringstream oss;
            oss << "TlmMesh2Dr::noc_thread : Not supporting responses";
            throw std::runtime_error(oss.str().c_str());
        }

        //noc_trans->release();

        // Forward the transaction to IP
        phase = tlm::BEGIN_REQ;
//        if(trans->is_write())
            delay = cycleTime_ * ((trans->get_data_length() / (data_width_g/8) + 2));
//        else
//            delay = cycleTime_ * (HEADER_LENGTH_IN_BYTE/(data_width_g/8));


#ifdef  NTS_4_DRAMSIM2_crossbar 
        // sent noc_trans to crossbar because the source id should be used by along
        if(id_ == 16 || id_ == 17 || id_ == 18 || id_ == 19){
            retval = initSocket->nb_transport_fw(*noc_trans, phase, delay);
        }
        else {
            noc_trans->release();
            retval = initSocket->nb_transport_fw(*trans, phase, delay);
        }
#else
        noc_trans->release();
        retval = initSocket->nb_transport_fw(*trans, phase, delay);
#endif      
		
        TRACE(
		cout << "NoC FW ToC in TargetNI ";
        if ( trans->is_read() ) cout << "R ";
        else                    cout << "W ";
		cout << "Source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << " ";
		cout << "Target: " << noc_trans->get_address() << " ";
		cout << "sc_time_stamp= " << sc_time_stamp() << " ";
        cout << hex << "0x"<< trans->get_address() << endl;
		);
        
        if ( trans->is_write() ) { //read is responsed through other path with rsp_ip_peq_ in oterm_ni_bw.
            if(retval == tlm::TLM_ACCEPTED || retval == tlm::TLM_UPDATED)
            {
                if(phase == tlm::BEGIN_REQ)
                { 
                    wait(otermAckEvent_);
                }
                else if(phase == tlm::END_REQ)
                {
                    std::ostringstream oss;
                    oss << "TlmMesh2D::thread : END_REQ not supported";
                    throw std::runtime_error(oss.str().c_str());
                }
                else if(phase == tlm::BEGIN_RESP)
                {
                    std::ostringstream oss;
                    oss << "TlmMesh2D::thread : BEGIN_RESP not supported";
                    throw std::runtime_error(oss.str().c_str());
                }
                else
                {
                    std::ostringstream oss;
                    oss << "TlmMesh2D::thread : invalid PHASE";
                    throw std::runtime_error(oss.str().c_str());
                }          
            }
            else if(retval == tlm::TLM_COMPLETED)
            {
                if(delay != sc_core::SC_ZERO_TIME)
                {
                    wait(delay);
                }
            }
            else
            {
                std::ostringstream oss;
                oss << "TlmMesh2D::thread : invalid SYNC_ENUM";
                throw std::runtime_error(oss.str().c_str());
            }
        }

    } // end of while(true)
}      


template<unsigned int data_width_g>
void 
TargetNI<data_width_g>::
rsp_tx_ni_thread()
{
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_generic_payload* noc_trans = 0;
    tlm::tlm_phase            phase;
    sc_core::sc_time          delay;
    tlm::tlm_sync_enum        retval;
    unsigned int              response_to = 0;

    while(true)
    {
        /*if( rsp_tx_source_deque_.empty() ) { wait(rsp_tx_deque_event_); }
        response_to = rsp_tx_source_deque_.front();
        rsp_tx_source_deque_.pop_front();*/
    
    
        // Check for pending transactions coming from PEs
        trans = rsp_ip_peq_.get_next_transaction();
        while(trans == NULL)
        { 
            wait(rsp_ip_peq_.get_event());
            trans = rsp_ip_peq_.get_next_transaction();
        }
        
        if( trans_sourceID_map_.empty() ) { wait(rsp_tx_deque_event_); }
        response_to = trans_sourceID_map_[trans];
        trans_sourceID_map_.erase(trans);

        
        TARGET_NI_DEBUG (
            std::cout << name() << "::rsp_tx_ni_thread: Respounding transaction" << std::endl;
        );

        noc_trans = m_mm.allocate();
        noc_trans->set_streaming_width(data_width_g / 8);
        noc_trans->set_write();
        noc_trans->set_extension( new ip_trans_ext(trans) );
        noc_trans->set_extension( new source_id_ext(id_) );
        noc_trans->set_address( response_to ); 
        noc_trans->set_data_length( trans->get_data_length() + HEADER_LENGTH_IN_BYTE );

		CROSSBAR_DEBUG(
			cout<< "  back_bw_sc_time_stamp = " << sc_time_stamp() << endl << endl;
			//cout<< "  ---------------------------------------------" << endl;
		);
        // Forward the transaction to routers
        phase = tlm::BEGIN_REQ;
        delay = cycleTime_ * 7; // Constant delay models the Network interface (NI)
        retval = rsp_init_->nb_transport_fw(*noc_trans, phase, delay);
		
		TRACE(
		cout << "NoC BW ToN in TargetNI ";
        if ( trans->is_read() ) cout << "R ";
        else                    cout << "W ";
		cout << "Source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << " ";
		cout << "Target: " << noc_trans->get_address() << " ";
		cout << "sc_time_stamp= " << sc_time_stamp() << " ";
        cout << hex << "0x"<< trans->get_address() << endl;
		);
		
        if(retval == tlm::TLM_ACCEPTED || retval == tlm::TLM_UPDATED)
        {
            if(phase == tlm::BEGIN_REQ)
            {   
                wait(rsp_rouAckEvent_);     
            }
            else if(phase == tlm::END_REQ)
            {
                std::ostringstream oss;
                oss << "TlmMesh2D::thread : END_REQ not supported";
                throw std::runtime_error(oss.str().c_str());
            }
            else if(phase == tlm::BEGIN_RESP)
            {
                std::ostringstream oss;
                oss << "TlmMesh2D::thread : BEGIN_RESP not supported";
                throw std::runtime_error(oss.str().c_str());
            }
            else
            {
                std::ostringstream oss;
                oss << "TlmMesh2D::thread : invalid PHASE";
                throw std::runtime_error(oss.str().c_str());
            }          
        }
        else if(retval == tlm::TLM_COMPLETED)
        {
            if(delay != sc_core::SC_ZERO_TIME)
            {
                //wait(delay);
                wait(SC_ZERO_TIME); //along
            }
        }
        else
        {
            std::ostringstream oss;
            oss << "TlmMesh2D::thread : invalid SYNC_ENUM";
            throw std::runtime_error(oss.str().c_str());
        }
    } // end of while(true)
}      



//----- Other private functions -----///

template<unsigned int data_width_g> 
std::string  
TargetNI<data_width_g>::req_noc_peq_name()
{
  std::ostringstream oss;
  oss << "TargetNI_" << id_ << "req_noc_peq_";
  return oss.str();
}


template<unsigned int data_width_g> 
std::string  
TargetNI<data_width_g>::rsp_ip_peq_name()
{
  std::ostringstream oss;
  oss << "TargetNI_" << id_ << "rsp_ip_peq_";
  return oss.str();
}

template<unsigned int data_width_g> 
std::string  
TargetNI<data_width_g>::initSocket_name()
{
  std::ostringstream oss;
  oss << "TargetNI_" << id_ << "initSocket";
  return oss.str();
}


template class TargetNI<32>;

} //end of namespace ispa
