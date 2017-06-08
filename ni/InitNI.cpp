#include <stdexcept>
#include "InitNI.h"
//#include "NetworkPayload.h"
#include "NocPayloadExtension.h"

#include "OCPWrappedMemory.h"

//#define EN_CROSSBAR_DEBUG
#define EN_TRACE
#include "CrossbarDebug.h"

namespace ispa
{


//Constructor
template<unsigned int data_width_g> 
InitNI<data_width_g>::
InitNI(sc_core::sc_module_name name, int id, SimpleAddressMap *map, sc_core::sc_time cycle_time)
  : sc_core::sc_module(name)
  , targetSocket(targetSocket_name().c_str())
  , req_ip_peq_(req_ip_peq_name().c_str())
  , rsp_noc_peq_(rsp_noc_peq_name().c_str()) 
  , cycleTime_(cycle_time) // 20ns = 50 MHz
  , HEADER_LENGTH_IN_BYTE( data_width_g/8 ) 
  , m_mask_(0x1ffffull) //no external memory now
  , id_(id)
  
{
  rsp_rx_trans_buffer_ = NULL;
  m_addr_map_ = map;
  targetSocket.set_ocp_config(ocp_parameters_);

  //register callback functions for sockets
  targetSocket.register_nb_transport_fw (this, &InitNI::iterm_ni_fw);
  req_init_.register_nb_transport_bw    (this, &InitNI::req_rou_ni_bw);
  rsp_target_.register_nb_transport_fw  (this, &InitNI::rsp_noc_ni_fw);

  SC_THREAD (req_tx_ni_thread);
  SC_THREAD (rsp_rx_ni_thread);

}

/*
 * Callback function. Gets data from targetSocket (input
 * terminal=iterm) and puts it into payload event queue (peq)
 * of NI.
 */
template<unsigned int data_width_g> 
tlm::tlm_sync_enum 
InitNI<data_width_g>::
iterm_ni_fw(
		tlm::tlm_generic_payload &trans,
		tlm::tlm_phase           &phase,
		sc_core::sc_time         &delay)
{
		CROSSBAR_DEBUG(
			cout<<  "  init_sc_time_stamp = " << sc_time_stamp() << endl;
			cout << "  In InitNI iterm_ni_fw" << endl;
			cout << "  cmd: " << trans.get_command() << endl;
			cout << "  phase: " << phase << endl;
			cout << "  InitNI_id: " << dec << id_ << endl;
			cout << "  InitNI_trans_target: " << hex << trans.get_address() << endl << endl;
		);
		
	if(phase == tlm::BEGIN_REQ)
	{
		req_ip_peq_.notify(trans, delay); // put data to queue and ask queue to wait
	}
	else if(phase == tlm::END_RESP)
	{
		trans.set_response_status(tlm::TLM_OK_RESPONSE);
		return tlm::TLM_COMPLETED;
	}
	else
	{
		std::ostringstream oss;
		oss << "NocInitIf::iterm_ni_fw " << id_
			<< ": got invalid PHASE";
		throw std::runtime_error(oss.str().c_str());
	}
	trans.set_response_status( tlm::TLM_OK_RESPONSE );
	return tlm::TLM_ACCEPTED;
}

template<unsigned int data_width_g>
tlm::tlm_sync_enum 
InitNI<data_width_g>::
req_rou_ni_bw(
    tlm::tlm_generic_payload &noc_trans,
    tlm::tlm_phase           &phase,
    sc_core::sc_time         &delay)
{

  if(phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
  {
    std::ostringstream oss;
    oss << "TlmMesh2D::req_rou_ni_bw " << id_ << " got wrong phase";
    throw std::runtime_error(oss.str().c_str());
  }

  req_rouAckEvent_.notify(delay);
  
  noc_trans.set_response_status( tlm::TLM_OK_RESPONSE );
  noc_trans.release();
  return tlm::TLM_COMPLETED;
}
    
/*
 * Models network interface (NI) at the sender side. Takes
 * data from peq and gives it to socket "req_init_" which is
 * connected to router.
 */
template<unsigned int data_width_g>
void 
InitNI<data_width_g>::
req_tx_ni_thread()
{
	tlm::tlm_generic_payload* noc_trans = 0;
	tlm::tlm_generic_payload* trans = 0;
	tlm::tlm_phase            phase;
	sc_core::sc_time          delay;
	tlm::tlm_sync_enum        retval;

	while(true)
	{
		// Check for pending transactions coming from PEs
		/*if((trans = req_ip_peq_.get_next_transaction()) == 0)
		{ 	
			
			wait(req_ip_peq_.get_event());
			trans = req_ip_peq_.get_next_transaction();
		}*/
		trans = req_ip_peq_.get_next_transaction();
        while(trans == NULL)
        { 
            wait(req_ip_peq_.get_event());
            trans = req_ip_peq_.get_next_transaction();
        }
        assert(trans != NULL);
        
		if ( trans->is_write() ) {
			phase = tlm::END_REQ; 

			// Send acknowledge to PE (=its adapter)
			delay = cycleTime_ * 
				((trans->get_data_length() / (data_width_g/8)) + 2 );

			retval = targetSocket->nb_transport_bw(*trans, phase, delay);	    
			if(retval != tlm::TLM_COMPLETED)
			{
				std::ostringstream oss;
				oss << "NocInitIfr::ip_thread : Not supporting responses";
				throw std::runtime_error(oss.str().c_str());
			}
		}
		
    //Allocate a payload and wrap the incomming transaction
    //as a NoC transaction (always write)
		noc_trans = m_mm.allocate();
		noc_trans->set_streaming_width(data_width_g / 8);
		noc_trans->set_write();
		noc_trans->set_extension( new ip_trans_ext(trans) );
		noc_trans->set_extension( new source_id_ext(id_) );
		//noc_trans->set_address( decode( trans->get_address() ) );

#ifdef NTS_4_DRAMSIM2_crossbar	
		noc_trans->set_address( crossbar_decode( trans->get_address(), id_) );
#else
		noc_trans->set_address( decode( trans->get_address() ) );
#endif
		CROSSBAR_DEBUG(
			cout<<  "  init_sc_time_stamp = " << sc_time_stamp() << endl;
			cout << "  InitNI_id: " << dec << id_ << endl;
			cout << "  cmd: " << trans->get_command() << endl;
			cout << "  InitNI_trans_target: " << hex << trans->get_address() << endl;	
			cout << "  InitNI_noc_trans_target: " << dec << noc_trans->get_address() << endl << endl;
		);
		
		if ( trans->is_read() ) {
			trans->acquire(); //need to wait read data back
			/*if ( rsp_rx_trans_buffer_ != NULL ) { wait( rsp_rx_trans_buffer_empty_ ); }
			rsp_rx_trans_buffer_ = trans;
			rsp_rx_trans_buffer_fill_.notify( sc_core::SC_ZERO_TIME );*/
			noc_trans->set_data_length( HEADER_LENGTH_IN_BYTE ); //header!
		}
		else //write
		{
			noc_trans->set_data_length( HEADER_LENGTH_IN_BYTE+trans->get_data_length() ); //header+data
		}

		// Forward the transaction to routers
		phase = tlm::BEGIN_REQ;
		delay = cycleTime_ * 7; // Constant delay models the Network interface (NI)
		retval = req_init_->nb_transport_fw(*noc_trans, phase, delay);
		
        TRACE(
		cout << "NoC FW ToN in InitNI ";
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
				wait(req_rouAckEvent_);            
			}
			else if(phase == tlm::END_REQ)
			{
				std::ostringstream oss;
				oss << "NocInitIf::thread : END_REQ not supported";
				throw std::runtime_error(oss.str().c_str());
			}
			else if(phase == tlm::BEGIN_RESP)
			{
				std::ostringstream oss;
				oss << "NocInitIf::thread : BEGIN_RESP not supported";
				throw std::runtime_error(oss.str().c_str());
			}
			else
			{
				std::ostringstream oss;
				oss << "NocInitIf::thread : invalid PHASE";
				throw std::runtime_error(oss.str().c_str());
			}	       
		}
		else if(retval == tlm::TLM_COMPLETED)
		{
			if(delay != sc_core::SC_ZERO_TIME)
			{
				//wait(delay);
                wait(SC_ZERO_TIME);  //along
			}
		}
		else
		{
			std::ostringstream oss;
			oss << "NocInitIf::thread : invalid SYNC_ENUM";
			throw std::runtime_error(oss.str().c_str());
		}


	} // end of while(true)
}      

/*
 * Callback function. Gets data from socket rsp_target_ 
 * and puts it into payload event queue (peq)
 * of NI.
 */
template<unsigned int data_width_g>
tlm::tlm_sync_enum 
InitNI<data_width_g>::
rsp_noc_ni_fw(
		tlm::tlm_generic_payload &noc_trans,
		tlm::tlm_phase           &phase,
		sc_core::sc_time         &delay)
{
	CROSSBAR_DEBUG(
		cout << "  InitNI_sc_time_stamp = " << sc_time_stamp() << endl;
        tlm::tlm_generic_payload* trans = 0;
        trans = NocPayloadExt::get_ip_trans( noc_trans );
        cout << "  In InitNI rsp_noc_ni_fw " << endl;
        cout << "  InitNI_id: " << dec << id_ << endl;
        cout << "  cmd: " << trans->get_command() << endl;
        cout << "  phase: " << phase << endl;
        cout << "  InitNI_noc_trans_target: " << dec << noc_trans.get_address() << endl << endl;
    );


	// Only write command is supported
	if(noc_trans.get_command() != tlm::TLM_WRITE_COMMAND)
	{
		std::ostringstream oss;
		oss << "NocInitIf::rsp_noc_ni_fw " << id_
			<< ": only write command is supported";
		throw std::runtime_error(oss.str().c_str());
	}

	if(phase == tlm::BEGIN_REQ)
	{
		noc_trans.acquire();
		rsp_noc_peq_.notify(noc_trans, delay);
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
		oss << "NocInitIf::rsp_noc_ni_fw " << id_
			<< ": got invalid PHASE";
		throw std::runtime_error(oss.str().c_str());
	}
	noc_trans.set_response_status( tlm::TLM_OK_RESPONSE );
	return tlm::TLM_ACCEPTED;
}


template<unsigned int data_width_g>
void 
InitNI<data_width_g>::
rsp_rx_ni_thread()
{
	tlm::tlm_generic_payload* noc_trans = 0;
	tlm::tlm_generic_payload* trans = 0;
	tlm::tlm_phase            phase;
	sc_core::sc_time          delay;
	tlm::tlm_sync_enum        retval;
	
	while(true)
	{
		// Check for pending transactions in RX peq
		/*if((noc_trans = rsp_noc_peq_.get_next_transaction()) == 0)
		{ 
			wait(rsp_noc_peq_.get_event());
			noc_trans = rsp_noc_peq_.get_next_transaction();
		}*/
		
		noc_trans = rsp_noc_peq_.get_next_transaction();
        while(noc_trans == NULL)
        { 
            wait(rsp_noc_peq_.get_event());
            noc_trans = rsp_noc_peq_.get_next_transaction();
        }
        assert(noc_trans != NULL);
		
		
		/*if ( rsp_rx_trans_buffer_ == NULL ) { wait( rsp_rx_trans_buffer_fill_ ); }
		trans = rsp_rx_trans_buffer_;
		rsp_rx_trans_buffer_ = NULL;
		rsp_rx_trans_buffer_empty_.notify( sc_core::SC_ZERO_TIME );*/
		
		CROSSBAR_DEBUG(
			cout << "  init_sc_time_stamp = " << sc_time_stamp() << endl;
			cout << "  In InitNI rsp_rx_ni_thread" << endl;
			cout << "  InitNI_id: " << dec << id_ << endl;
			cout << "  InitNI_trans_target: " << hex << noc_trans->get_address() << endl << endl;
		);
        
        tlm::tlm_generic_payload* trans = 0;
        trans = NocPayloadExt::get_ip_trans( noc_trans );

		phase = tlm::END_REQ; 

		// Acknowledge
		delay = cycleTime_ * ((noc_trans->get_data_length() / (data_width_g/8) + 3));
		retval = rsp_target_->nb_transport_bw(*noc_trans, phase, delay);
		

		if(retval != tlm::TLM_COMPLETED)
		{
			std::ostringstream oss;
			oss << "TlmMesh2Dr::noc_thread : Not supporting responses";
			throw std::runtime_error(oss.str().c_str());
		}
		
        TRACE(
		cout << "NoC BW ToComm in InitNI ";
        if ( trans->is_read() ) cout << "R ";
        else                    cout << "W ";
		cout << "Source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << " ";
		cout << "Target: " << noc_trans->get_address() << " ";
		cout << "sc_time_stamp= " << sc_time_stamp() << " ";
        cout << hex << "0x"<< trans->get_address() << endl;
        );
        
		noc_trans->release();

		// Forward the transaction to IP
		phase = tlm::BEGIN_RESP;
		delay = cycleTime_ * ((trans->get_data_length() / (data_width_g/8) + 2));	    
		retval = targetSocket->nb_transport_bw(*trans, phase, delay);
		
		/*if(trans->get_command() == tlm::TLM_READ_COMMAND){
		
			unsigned int *data_ptr = reinterpret_cast<unsigned int *>(trans->get_data_ptr());
			cout << "  init_sc_time_stamp = " << sc_time_stamp() << endl;
			cout << "  In InitNI rsp_rx_ni_thread" << endl;
			cout << "  phase: " << phase << endl;
			cout << "  data: " << dec << data_ptr[0] << endl;
			cout << "  InitNI_id: " << dec << id_ << endl;
			cout << "  InitNI_trans_target: " << hex << trans->get_address() << endl << endl;
		}*/
		
		//cout<<"init_sc_time_stamp = " << sc_time_stamp() << endl;

		if(retval != tlm::TLM_COMPLETED)
		{
			std::ostringstream oss;
			oss << "TlmMesh2D::thread : invalid SYNC_ENUM";
			throw std::runtime_error(oss.str().c_str());
		}
		if(delay != sc_core::SC_ZERO_TIME)
		{
			wait(delay);
		}

	} // end of while(true)
}      

template<unsigned int data_width_g> 
std::string  
InitNI<data_width_g>::rsp_noc_peq_name()
{
  std::ostringstream oss;
  oss << "InitNI_" << id_ << "rsp_noc_peq_";
  return oss.str();
}

template<unsigned int data_width_g> 
std::string  
InitNI<data_width_g>::req_ip_peq_name()
{
  std::ostringstream oss;
  oss << "InitNI_" << id_ << "req_ip_peq_";
  return oss.str();
}

template<unsigned int data_width_g> 
std::string  
InitNI<data_width_g>::targetSocket_name()
{
  std::ostringstream oss;
  oss << "InitNI_" << id_ << "targetSocket";
  return oss.str();
}

template<unsigned int data_width_g> 
unsigned int 
InitNI<data_width_g>::decode(const sc_dt::uint64& address)
{
  return m_addr_map_->decode(address);
}

template<unsigned int data_width_g> 
unsigned int 
InitNI<data_width_g>::crossbar_decode(const sc_dt::uint64& address, unsigned int id)
{
  return m_addr_map_->crossbar_decode(address, id);
}


template class InitNI<32>;

} //end of nemaspace ispa



