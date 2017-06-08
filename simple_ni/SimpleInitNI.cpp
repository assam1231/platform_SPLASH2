#include <string>
#include <sstream>
#include <stdexcept>

#include "SimpleInitNI.h"



/*
 * Constructor
 *
 */
template <unsigned int g_data_width>
SimpleInitNI<g_data_width>::SimpleInitNI(
    sc_core::sc_module_name name, 
    int id, 
    SimpleAddressMap* address_map, 
    sc_core::sc_time cycle_time):

  sc_core::sc_module(name),
  targetSocketIP(input_socket_name().c_str()),
  m_id(id),
  m_addressMap(*address_map),
  m_ipPeq(ip_peq_name().c_str()),
  m_cycleTime(cycle_time),
  m_niDelay(7)
{
  SC_HAS_PROCESS(SimpleInitNI);

  //Set up input and output sockets
  targetSocketIP.set_ocp_config(m_ocp_parameters);
  targetSocketIP.register_nb_transport_fw (this, &SimpleInitNI::input_nb_transport_fw);
  initSocketNoC.register_nb_transport_bw (this, &SimpleInitNI::output_nb_transport_bw);

  //Register thread
  SC_THREAD(input_ni_process);
}

/*
 * Input Socket nb_transport function
 *
 */

template <unsigned int g_data_width>
tlm::tlm_sync_enum SimpleInitNI<g_data_width>::input_nb_transport_fw(
      tlm::tlm_generic_payload &trans,
      tlm::tlm_phase           &phase,
      sc_core::sc_time         &delay)
{
  if(trans.get_command() != tlm::TLM_WRITE_COMMAND)
  {
    std::ostringstream oss;
    oss << "SimpleInitNI::iterm_ni_fw " << m_id << ": only write command is supported";
    throw std::runtime_error(oss.str().c_str());
  }

  if(phase == tlm::BEGIN_REQ)
  {
    trans.acquire();
    m_ipPeq.notify(trans, delay); // put data to queue and ask queue to wait
  }
  else if(phase == tlm::END_RESP)
  {
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
    return tlm::TLM_COMPLETED;
  }
  else
  {
    std::ostringstream oss;
    oss << "SimpleInitNI::iterm_ni_fw " << m_id << ": got invalid PHASE";
    throw std::runtime_error(oss.str().c_str());
  }
  trans.set_response_status( tlm::TLM_OK_RESPONSE );
  return tlm::TLM_ACCEPTED;
}

/*
 * Input Network Interface Process
 *
 */

template <unsigned int g_data_width>
void SimpleInitNI<g_data_width>::input_ni_process()
{
  while(true)
  {
    tlm::tlm_generic_payload* trans = get_trans_from_peq();
    send_ack_to_PE(trans);

    tlm::tlm_generic_payload *noc_trans = wrap_trans(trans);
    foward_trans_to_router(noc_trans);

    trans->release();
  } 
}

/*
 * Output Socket nb tranport backward function
 *
 */
template <unsigned int g_data_width>
tlm::tlm_sync_enum SimpleInitNI<g_data_width>::output_nb_transport_bw(
				tlm::tlm_generic_payload &trans,
				tlm::tlm_phase           &phase,
				sc_core::sc_time         &delay)
{
  if(phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
  {
    std::ostringstream oss;
    oss << "SimpleInitNI: " << m_id << " got wrong phase";
    throw std::runtime_error(oss.str().c_str());
  }

  m_outputAckEvent.notify(delay);

  trans.set_response_status( tlm::TLM_OK_RESPONSE );
  return tlm::TLM_COMPLETED;
}

/*
 * Other handy functions
 *
 */

template <unsigned int g_data_width>
std::string SimpleInitNI<g_data_width>::ip_peq_name()
{
  std::ostringstream oss;
  oss << "SimpleInitNI_" << m_id << "_InputPeq";
  return oss.str();
}

template<unsigned int g_data_width> 
std::string SimpleInitNI<g_data_width>::input_socket_name()
{
    std::ostringstream oss;
      oss << "SimpleInitNI_" << m_id << "targetSocketIP";
        return oss.str();
}


template<unsigned int g_data_width>
tlm::tlm_generic_payload* SimpleInitNI<g_data_width>::get_trans_from_peq() 
{
  tlm::tlm_generic_payload *trans = NULL;
  trans = m_ipPeq.get_next_transaction();
  if( trans == 0)
  { 		  
    wait(m_ipPeq.get_event());
    trans = m_ipPeq.get_next_transaction();
  }
  return trans;
}

template<unsigned int g_data_width>
void SimpleInitNI<g_data_width>::send_ack_to_PE(tlm::tlm_generic_payload *trans) 
{
  tlm::tlm_phase phase = tlm::END_REQ;
  sc_core::sc_time delay = m_cycleTime * (trans->get_data_length() / trans->get_streaming_width());
  tlm::tlm_sync_enum retval = targetSocketIP->nb_transport_bw(*trans, phase, delay);	    

  if(retval != tlm::TLM_COMPLETED)
  {
    std::ostringstream oss;
    oss << "SimpleInitNIr::ip_thread : Not supporting responses";
    throw std::runtime_error(oss.str().c_str());
  }
}


template<unsigned int g_data_width>
tlm::tlm_generic_payload* SimpleInitNI<g_data_width>::wrap_trans(tlm::tlm_generic_payload *trans)
{

  tlm::tlm_generic_payload *noc_trans = m_mm.allocate(); //use memory management 
  unsigned destID = m_addressMap.decode(trans->get_address());
  noc_trans->set_address(destID);
  noc_trans->set_write();
  noc_trans->set_data_ptr( reinterpret_cast<unsigned char*>(trans) ); 
  noc_trans->set_streaming_width(g_data_width / 8);

  return noc_trans;
}

template<unsigned int g_data_width>
void SimpleInitNI<g_data_width>::foward_trans_to_router(tlm::tlm_generic_payload *noc_trans)
{
  tlm::tlm_phase phase = tlm::BEGIN_REQ;
  sc_core::sc_time delay = m_cycleTime * m_niDelay; // Constant delay models the Network interface (NI)
  tlm::tlm_sync_enum retval = initSocketNoC->nb_transport_fw(*noc_trans, phase, delay);

  if(retval == tlm::TLM_ACCEPTED || retval == tlm::TLM_UPDATED)
  {
    if(phase == tlm::BEGIN_REQ)
    {	
      wait(m_outputAckEvent);		
    }
    else if(phase == tlm::END_REQ)
    {
      std::ostringstream oss;
      oss << "SimpleInitNI::thread : END_REQ not supported";
      throw std::runtime_error(oss.str().c_str());
    }
    else if(phase == tlm::BEGIN_RESP)
    {
      std::ostringstream oss;
      oss << "SimpleInitNI::thread : BEGIN_RESP not supported";
      throw std::runtime_error(oss.str().c_str());
    }
    else
    {
      std::ostringstream oss;
      oss << "SimpleInitNI::thread : invalid PHASE";
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
    oss << "SimpleInitNI::thread : invalid SYNC_ENUM";
    throw std::runtime_error(oss.str().c_str());
  }
}

template class SimpleInitNI<32>;
