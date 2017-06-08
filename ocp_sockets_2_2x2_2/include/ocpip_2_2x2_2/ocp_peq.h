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

#ifndef OCPIP_VERSION
  #error ocp_peq.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif


namespace OCPIP_VERSION{

struct ocp_p2p_data{
  tlm::tlm_phase phase;
  //sc_dt::uint64   address;
  unsigned int   burst_length;
  tlm::tlm_response_status response;
  bool           has_blen;//, has_req_info, has_dt_info, has_resp_info;
  unsigned int   index;
  //todo: valid data?
};

template <typename OCP_P2P_DATA>
struct ocp_peq_traits_t{
  typedef tlm::tlm_generic_payload tlm_payload_type;
  typedef OCP_P2P_DATA tlm_phase_type;
};

template <typename SOCKET, bool TAGGED, typename OCP_P2P_DATA=ocp_p2p_data, bool ACTIVATE_P2P_WRITEBACK=false>
struct ocp_peq : public tlm_utils::peq_with_cb_and_phase<ocp_peq<SOCKET,TAGGED,OCP_P2P_DATA,ACTIVATE_P2P_WRITEBACK>, ocp_peq_traits_t<OCP_P2P_DATA> >{

  typedef typename SOCKET::ext_support_type ext_support_type;
  typedef ocp_peq_traits_t<OCP_P2P_DATA> ocp_peq_traits;

  template <typename MODULE>
  ocp_peq(MODULE* mod, typename nb_cb_type_selector<MODULE, TAGGED>::peq_cb_type, SOCKET* socket);
  
  void notify(typename ocp_peq_traits::tlm_payload_type& trans, tlm::tlm_phase& phase);
  
  void notify(typename ocp_peq_traits::tlm_payload_type& trans, tlm::tlm_phase& phase, const sc_core::sc_time& time);

  void notify(unsigned int, typename ocp_peq_traits::tlm_payload_type& trans, tlm::tlm_phase& phase);
  
  void notify(unsigned int, typename ocp_peq_traits::tlm_payload_type& trans, tlm::tlm_phase& phase, const sc_core::sc_time& time);
  
  void activate_sp();
  
#if (TLM_VERSION_MAJOR>2 | TLM_VERSION_MINOR>0 | TLM_VERSION_PATCH>0)  
  void reset();
#endif
protected:
  void peq_cb(typename ocp_peq_traits::tlm_payload_type& trans, const typename ocp_peq_traits::tlm_phase_type& p2p);
  OCP_P2P_DATA p2p_tmp;
  SOCKET* m_socket;
  sc_core::sc_time m_synchronization_protection;
  typename nb_functor_type_selector<TAGGED>::peq_functor_type m_functor;
  bool m_use_sp;
};

} //end ns OCPIP_VERSION

#include __MACRO_STRINGYFY__(../src/OCPIP_VERSION/ocp_peq.tpp)
