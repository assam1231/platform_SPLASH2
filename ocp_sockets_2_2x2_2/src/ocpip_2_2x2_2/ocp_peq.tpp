///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// (c) Copyright OCP-IP 2008
// OCP-IP Confidential and Proprietary
//
//
//============================================================================
//      Project : OCP SLD WG
//       Author : Robert Guenzel (from TU of Braunschweig) for Greensocs Ltd.
//
//          $Id:
//
//  Description :  This file contains the PEQ used for the PEQed nb_transport
//                 calls. Since it is based on the OSCI peq_with_cb_and_phase
//                 we cannot make it copyright OCP, right?
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

template <typename SOCKET, bool TAGGED, typename OCP_P2P_DATA, bool ACTIVATE_P2P_WRITEBACK>
template <typename MODULE>
OCPIP_VERSION::ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>::ocp_peq(
    MODULE* mod
  , typename nb_cb_type_selector<MODULE, TAGGED>::peq_cb_type cb
  , SOCKET* socket
)
  : tlm_utils::peq_with_cb_and_phase<ocp_peq, ocp_peq_traits>(this, &ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>::peq_cb)
  , m_socket(socket)
  , m_synchronization_protection(sc_core::sc_get_time_resolution())
  , m_use_sp(false)
{
  m_functor.set_function(mod, cb);
}

#if (TLM_VERSION_MAJOR>2 | TLM_VERSION_MINOR>0 | TLM_VERSION_PATCH>0)
template <typename SOCKET, bool TAGGED, typename OCP_P2P_DATA, bool ACTIVATE_P2P_WRITEBACK>
void OCPIP_VERSION::ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>::reset(){
  tlm_utils::peq_with_cb_and_phase<ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>, ocp_peq_traits>::cancel_all();
}
#endif

template <typename SOCKET, bool TAGGED, typename OCP_P2P_DATA, bool ACTIVATE_P2P_WRITEBACK>
void OCPIP_VERSION::ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>::notify(typename ocp_peq_traits::tlm_payload_type& trans, tlm::tlm_phase& phase){
  if (TAGGED){std::cerr<<"Untagged notification called on tagged PEQ of "<<m_socket->name()<<std::endl; abort();}
  p2p_tmp.phase=phase;
  //p2p_tmp.address=trans.get_address();
  p2p_tmp.response=trans.get_response_status();
  burst_length* length;
  p2p_tmp.has_blen=ext_support_type::template get_extension<burst_length>(length, trans);
  if (p2p_tmp.has_blen){ //if this is an imprecise burst we gotta save its length
    p2p_tmp.burst_length=length->value;
  }

  p2p_tmp.response=trans.get_response_status();
  //this function will never be called without dp, so we do not need to check
  trans.acquire();
  tlm_utils::peq_with_cb_and_phase<ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>, ocp_peq_traits>::notify(trans, p2p_tmp, m_synchronization_protection);
}

template <typename SOCKET, bool TAGGED, typename OCP_P2P_DATA, bool ACTIVATE_P2P_WRITEBACK>
void OCPIP_VERSION::ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>::notify(typename ocp_peq_traits::tlm_payload_type& trans, tlm::tlm_phase& phase, const sc_core::sc_time& time){
  if (TAGGED){std::cerr<<"Untagged notification called on tagged PEQ "<<m_socket->name()<<std::endl; abort();}
  p2p_tmp.phase=phase;
  //p2p_tmp.address=trans.get_address();
  p2p_tmp.response=trans.get_response_status();
  burst_length* length;
  p2p_tmp.has_blen=ext_support_type::template get_extension<burst_length>(length, trans);
  if (p2p_tmp.has_blen){ //if this is an imprecise burst we gotta save its length
    p2p_tmp.burst_length=length->value;
  }
  p2p_tmp.response=trans.get_response_status();
  trans.acquire();
  if (m_use_sp)
    tlm_utils::peq_with_cb_and_phase<ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>, ocp_peq_traits>::notify(trans, p2p_tmp, time+m_synchronization_protection);
  else
    tlm_utils::peq_with_cb_and_phase<ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>, ocp_peq_traits>::notify(trans, p2p_tmp, time);
}

template <typename SOCKET, bool TAGGED, typename OCP_P2P_DATA, bool ACTIVATE_P2P_WRITEBACK>
void OCPIP_VERSION::ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>::notify(unsigned int index, typename ocp_peq_traits::tlm_payload_type& trans, tlm::tlm_phase& phase){
  if (!TAGGED){std::cerr<<"Tagged notification called on untagged PEQ "<<m_socket->name()<<std::endl; abort();}
  p2p_tmp.phase=phase;
  //p2p_tmp.address=trans.get_address();
  p2p_tmp.response=trans.get_response_status();
  p2p_tmp.index=index;
  burst_length* length;
  p2p_tmp.has_blen=ext_support_type::template get_extension<burst_length>(length, trans);
  if (p2p_tmp.has_blen){ //if this is an imprecise burst we gotta save its length
    p2p_tmp.burst_length=length->value;
  }

  p2p_tmp.response=trans.get_response_status();
  trans.acquire();
  //this function will never be called without dp, so we do not need to check
  tlm_utils::peq_with_cb_and_phase<ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>, ocp_peq_traits>::notify(trans, p2p_tmp, m_synchronization_protection);
}

template <typename SOCKET, bool TAGGED, typename OCP_P2P_DATA, bool ACTIVATE_P2P_WRITEBACK>
void OCPIP_VERSION::ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>::notify(unsigned int index, typename ocp_peq_traits::tlm_payload_type& trans, tlm::tlm_phase& phase, const sc_core::sc_time& time){
  if (!TAGGED){std::cerr<<"Tagged notification called on untagged PEQ "<<m_socket->name()<<std::endl; abort();}
  p2p_tmp.phase=phase;
  //p2p_tmp.address=trans.get_address();
  p2p_tmp.response=trans.get_response_status();
  p2p_tmp.index=index;
  burst_length* length;
  p2p_tmp.has_blen=ext_support_type::template get_extension<burst_length>(length, trans);
  if (p2p_tmp.has_blen){ //if this is an imprecise burst we gotta save its length
    p2p_tmp.burst_length=length->value;
  }
  p2p_tmp.response=trans.get_response_status();
  trans.acquire();
  if (m_use_sp)
    tlm_utils::peq_with_cb_and_phase<ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>, ocp_peq_traits>::notify(trans, p2p_tmp, time+m_synchronization_protection);
  else
    tlm_utils::peq_with_cb_and_phase<ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>, ocp_peq_traits>::notify(trans, p2p_tmp, time);
}

template <typename SOCKET, bool TAGGED, typename OCP_P2P_DATA, bool ACTIVATE_P2P_WRITEBACK>
void OCPIP_VERSION::ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>::peq_cb(typename ocp_peq_traits::tlm_payload_type& trans, const typename ocp_peq_traits::tlm_phase_type& p2p){
  //trans.set_address(p2p.address);
  if (ACTIVATE_P2P_WRITEBACK) p2p_tmp=p2p;
  trans.set_response_status(p2p.response);
  if (p2p.has_blen){ //we gotta set the burst length and activate imprecise again
    burst_length* length;
    ext_support_type::template get_extension<burst_length>(length, trans); 
    ext_support_type::template validate_extension<burst_length>(trans); //make sure burst length is validated
    length->value=p2p.burst_length; //set the burst length
  }
  m_functor(p2p.index, trans, p2p.phase);
  trans.release();
}

template <typename SOCKET, bool TAGGED, typename OCP_P2P_DATA, bool ACTIVATE_P2P_WRITEBACK>
void OCPIP_VERSION::ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>::activate_sp(){
  m_use_sp=true;
}
