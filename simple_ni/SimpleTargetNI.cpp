#include <stdexcept>
#include "SimpleTargetNI.h"


template<unsigned int g_data_width>
SimpleTargetNI<g_data_width>::SimpleTargetNI(sc_core::sc_module_name name,  
    int id, 
    SimpleAddressMap *map,
    sc_core::sc_time cycle_time):
  sc_module(name),
  initSocketIP(output_socket_name().c_str()),
  m_id(id),
  m_addressMap(*map),
  m_cycleTime(cycle_time),
  m_niDelay(3),
  m_nocPeq(noc_peq_name().c_str())
{
  SC_HAS_PROCESS(SimpleTargetNI);

  initSocketIP.set_ocp_config(m_ocpParameters);
  initSocketIP.register_nb_transport_bw(this, &SimpleTargetNI::output_nb_transport_bw);

	targetSocketNoC.register_nb_transport_fw (this, &SimpleTargetNI::input_nb_transport_fw);

  SC_THREAD(output_ni_process);
}



template<unsigned int g_data_width>
tlm::tlm_sync_enum 
SimpleTargetNI<g_data_width>::input_nb_transport_fw(
    tlm::tlm_generic_payload &noc_trans,
    tlm::tlm_phase           &phase,
    sc_core::sc_time         &delay)
{
  // Only write command in NoC transaction
  if(noc_trans.get_command() != tlm::TLM_WRITE_COMMAND)
  {
    std::ostringstream oss;
    oss << "SimpleTargetNI::noc_ni_fw " << m_id 
      << ": only write command is supported";
    throw std::runtime_error(oss.str().c_str());
  }

  if(phase == tlm::BEGIN_REQ)
  {
    noc_trans.acquire();
    m_nocPeq.notify(noc_trans, delay);
    noc_trans.set_response_status( tlm::TLM_OK_RESPONSE );
    return tlm::TLM_ACCEPTED;
  }
  else if(phase == tlm::END_RESP)
  {
    noc_trans.set_response_status(tlm::TLM_OK_RESPONSE);
    return tlm::TLM_COMPLETED;
  }
  else
  {
    std::ostringstream oss;
    oss << "SimpleTargetNI::noc_ni_fw " << m_id 
      << ": got invalid PHASE";
    throw std::runtime_error(oss.str().c_str());
  }

}

template<unsigned int g_data_width>
void SimpleTargetNI<g_data_width>::output_ni_process()
{
  while(true)
  {
    tlm::tlm_generic_payload *noc_trans = get_noc_trans_from_peq();
    tlm::tlm_generic_payload *trans = unwrap_noc_trans(noc_trans);
    noc_trans->release();

    send_ack_to_NoC(noc_trans);

    remap_trans_address(trans);
    foward_trans_to_ip(trans);
    trans->release();
  } 
}

template<unsigned int g_data_width>
tlm::tlm_sync_enum 
SimpleTargetNI<g_data_width>::output_nb_transport_bw(
    tlm::tlm_generic_payload &noc_trans,
    tlm::tlm_phase           &phase,
    sc_core::sc_time         &delay)
{
  if(phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
  {
    std::ostringstream oss;
    oss << "SimpleTargetNI::oterm_ni_bw " << m_id << " got wrong phase";
    throw std::runtime_error(oss.str().c_str());
  }

  m_outputAckEvent.notify(delay);

  noc_trans.set_response_status( tlm::TLM_OK_RESPONSE );
  return tlm::TLM_COMPLETED;
}

/*
 * Other handy functions
 *
 */

template <unsigned int g_data_width>
std::string SimpleTargetNI<g_data_width>::noc_peq_name()
{
  std::ostringstream oss;
  oss << "SimpleTargetNI_" << m_id << "_nocPeq";
  return oss.str();
}

template<unsigned int g_data_width> 
std::string SimpleTargetNI<g_data_width>::output_socket_name()
{
  std::ostringstream oss;
  oss << "SimpleTargetNI_" << m_id << "output_socket_name";
  return oss.str();
}

template<unsigned int g_data_width> 
tlm::tlm_generic_payload* SimpleTargetNI<g_data_width>::get_noc_trans_from_peq()
{
  tlm::tlm_generic_payload *noc_trans;
  if((noc_trans = m_nocPeq.get_next_transaction()) == 0)
  { 
    wait(m_nocPeq.get_event());
    noc_trans = m_nocPeq.get_next_transaction();
  }
  return noc_trans;
}

template<unsigned int g_data_width> 
tlm::tlm_generic_payload* SimpleTargetNI<g_data_width>::unwrap_noc_trans(tlm::tlm_generic_payload *noc_trans)
{
    return reinterpret_cast<tlm::tlm_generic_payload*>(noc_trans->get_data_ptr());
}

template<unsigned int g_data_width> 
void SimpleTargetNI<g_data_width>::send_ack_to_NoC(tlm::tlm_generic_payload *noc_trans)
{
  tlm::tlm_phase phase = tlm::END_REQ;
  sc_core::sc_time delay = m_cycleTime * ((noc_trans->get_data_length() / noc_trans->get_streaming_width()));
  tlm::tlm_sync_enum retval = targetSocketNoC->nb_transport_bw(*noc_trans, phase, delay);

  if(retval != tlm::TLM_COMPLETED)
  {
    std::ostringstream oss;
    oss << "TargetNIr::noc_thread : Not supporting responses";
    throw std::runtime_error(oss.str().c_str());
  }
}

template<unsigned int g_data_width> 
void SimpleTargetNI<g_data_width>::remap_trans_address(tlm::tlm_generic_payload *trans) {
  unsigned int oriAddr = trans->get_address();
  unsigned int portNum = m_addressMap.decode(oriAddr);
  trans->set_address(oriAddr - m_addressMap.getBaseAddress(portNum));
}

template<unsigned int g_data_width> 
void SimpleTargetNI<g_data_width>::foward_trans_to_ip(tlm::tlm_generic_payload *trans)
{
  tlm::tlm_phase phase = tlm::BEGIN_REQ;
  sc_core::sc_time delay = m_cycleTime* ((trans->get_data_length() / trans->get_streaming_width() + m_niDelay));	    
  tlm::tlm_sync_enum retval = initSocketIP->nb_transport_fw(*trans, phase, delay);

  if(retval == tlm::TLM_ACCEPTED || retval == tlm::TLM_UPDATED)
  {
    if(phase == tlm::BEGIN_REQ)
    { 
      wait(m_outputAckEvent);		
    }
    else if(phase == tlm::END_REQ)
    {
      std::ostringstream oss;
      oss << "SimpleTargetNI::thread : END_REQ not supported";
      throw std::runtime_error(oss.str().c_str());
    }
    else if(phase == tlm::BEGIN_RESP)
    {
      std::ostringstream oss;
      oss << "SimpleTargetNI::thread : BEGIN_RESP not supported";
      throw std::runtime_error(oss.str().c_str());
    }
    else
    {
      std::ostringstream oss;
      oss << "SimpleTargetNI::thread : invalid PHASE";
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
    oss << "SimpleTargetNI::thread : invalid SYNC_ENUM";
    throw std::runtime_error(oss.str().c_str());
  }
}

template class SimpleTargetNI<32>;
