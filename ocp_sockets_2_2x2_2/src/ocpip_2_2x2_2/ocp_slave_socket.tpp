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
//  Description :  This file contains the ocp_slave_socket that ocp slave 
//                 modules shall use.
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#if (TLM_VERSION_MAJOR>2 | TLM_VERSION_MINOR>0 | TLM_VERSION_PATCH>0)  
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::reset(){
  if (m_nb_peq) m_nb_peq->reset();
}
#endif

//Ctor to be used for modules that are not interested in configuration callbacks
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::ocp_slave_socket(const char* name, ocp_layer_ids layer)
  : base_type(name)
  , ocp_config_support_type(base_type::name(), BUSWIDTH, NUM_BINDS, this, ocp_config_support_type::OCP_SLV, layer)
  , m_use_sp(false)
  , m_nb_peq(NULL)
  , m_reset_peq(NULL)
  , m_if_wrapper(this, m_nb_peq, layer)
{}

//Ctor that allows for registering a configuration listener callback
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
template <typename MODULE>
OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::ocp_slave_socket(
  const char* name, 
  ocp_layer_ids layer,
  MODULE* owner, 
  void (MODULE::*timing_cb)(ocp_tl1_master_timing)
)
  : base_type(name)
  , ocp_config_support_type(base_type::name(), BUSWIDTH, NUM_BINDS, this, ocp_config_support_type::OCP_SLV, layer)
  , m_use_sp(false)
  , m_nb_peq(NULL)
  , m_reset_peq(NULL)
  , m_if_wrapper(this, m_nb_peq, layer)
{
  m_time_cb.set_function(owner, timing_cb);
}

//assign a non default timing to that socket
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::set_slave_timing(const ocp_tl1_slave_timing& my_timing){
  m_one_for_all_timing=my_timing; //set local default timing  
  for (unsigned int i=0; i<m_timing.size(); i++){ //if there are already some bound targets, inform them
    if (my_timing!=m_timing[i]){
      m_timing[i]=my_timing;
      if (m_bound[i]) transmit_timing(i);
    }
  }
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::set_slave_timing(const ocp_tl1_slave_timing& my_timing, unsigned int index){
  if (index>=m_timing.size()) m_timing.resize(index+1, m_one_for_all_timing);
  if (index>=m_bound.size())  m_bound.resize(index+1, false);
  if (my_timing!=m_timing[index]){
    m_timing[index]=my_timing;
    if (m_bound[index]) transmit_timing(index);
  }
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::activate_delta_cycle_protection(){
  std::cerr<<"Warning: "<<base_type::name()<<" use of deprecated function activate_delta_cycle_protection() use activate_synchronization_protection() instead."<<std::endl;
  activate_synchronization_protection();
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::activate_synchronization_protection(){
  m_use_sp=true;
  if (m_nb_peq) {
    m_nb_peq->activate_sp();
    assert(m_reset_peq);
    m_reset_peq->activate_sp();
  }
  else 
  if (!base_type::m_nb_fw_functor.empty()){
    std::cerr<<"Warning: "<<base_type::name()<<" synchronization protection (SP) activated, but no peq is used. SP will not work."<<std::endl;
  }
}

//register an nb_transport like function
// It is called after a 'real' nb_transport arrived and the time
//  argument of this nb_transport has expired.
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
template <typename MODULE>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::register_nb_transport_fw(
  MODULE* mod, 
  typename nb_cb_type_selector<MODULE, NUM_BINDS!=1>::nb_cb_type cb,
  bool use_peq
){
  if (use_peq){
    if (m_nb_peq){
      std::cerr<<"Warning: "<<base_type::name()<<" nb_transport_bw registered more than once. Last one registered will be used."<<std::endl;
      delete m_nb_peq;
      assert(m_reset_peq);
      delete m_reset_peq;
    }
    if (ocp_config_support_type::m_layer > ocp_tl1){
      std::cerr<<"Warning: "<<base_type::name()<<" use of PEQs for TL2/3 enabled sockets not recommended."<<std::endl;
    }    
    m_nb_fw_functor.set_function(mod, cb);
    base_type::register_nb_transport_fw(this, &ocp_slave_socket::nb_transport_fw);
    m_nb_peq=new peq_type(this, &ocp_slave_socket::nb_transport_fw_peq, this);
    m_reset_peq=new peq_type(this, &ocp_slave_socket::nb_transport_fw_peq, this);
    if (m_use_sp) {
      m_nb_peq->activate_sp();
      m_reset_peq->activate_sp();
    }
    //m_if_wrapper.set_has_peq(true);
  }
  else{
    if (m_use_sp){
      std::cerr<<"Warning: "<<base_type::name()<<" delta cycle protection (DCP) activated, but no peq is used. DCP will not work."<<std::endl;
    } 
    if (m_nb_peq) {
      delete m_nb_peq; m_nb_peq=NULL;
      assert(m_reset_peq);
      delete m_reset_peq; m_reset_peq=NULL;
    } 
    base_type::register_nb_transport_fw(mod, cb);
    //m_if_wrapper.set_has_peq(false);
  }
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
template <typename MODULE>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::register_nb_transport_fw(
  MODULE* mod, 
  typename nb_cb_type_selector<MODULE, NUM_BINDS!=1>::nb_cb_type cb
){
  if (ocp_config_support_type::m_layer & ocp_tl1)
    register_nb_transport_fw(mod, cb, true);
  else
    register_nb_transport_fw(mod, cb, false);
}
/*
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
template<typename MODULE>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::register_nb_transport_fw( MODULE* mod
                             , tlm::tlm_sync_enum (MODULE::*cb)(unsigned int, tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&)
                             , bool use_peq)
{
  if (use_peq){
    std::cerr<<"Error: "<<base_type::name()<<" built-in PEQ cannot be used with multi sockets."<<std::endl;
    exit(1);
  }
  base_type::register_nb_transport_fw(mod, cb);
}
*/
//nb_transport_fw is overridden to either feed it into the PEQ
// when the peq nb_transport was registered or to feed it directly
//  into the slave (through the ocp base socket)
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
tlm::tlm_sync_enum OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::nb_transport_fw(tlm::tlm_generic_payload& trans,
                               tlm::tlm_phase& phase,
                               sc_core::sc_time& t){
  assert(m_nb_peq);
  assert(m_reset_peq);
  
  if (t>sc_core::SC_ZERO_TIME || m_use_sp){
    //if we have a timing annotation or use delta cycle protection we push the transport into the PEQ
    if (phase==BEGIN_RESET || phase==END_RESET) m_reset_peq->notify(trans, phase, t);
    else m_nb_peq->notify(trans, phase, t);
    if (!ocp_config_support_type::m_resolved_params[0].cmdaccept && phase==tlm::BEGIN_REQ){
      //if there is no CmdAccept control flow and we get a BEGIN_REQ, we accept it on behalf of the user code
      phase=tlm::END_REQ;
      return tlm::TLM_UPDATED;
    }
    else
    if (!ocp_config_support_type::m_resolved_params[0].dataaccept && phase==BEGIN_DATA){
      //if there is no DataAccept control flow and we get a BEGIN_DATA, we accept it on behalf of the user code
      phase=END_DATA;
      return tlm::TLM_UPDATED;
    }
    else    
      return tlm::TLM_ACCEPTED;
  }
  else
    return m_nb_fw_functor(trans, phase, t);
  
  //unreachable. Just to satisfy the compiler
  return tlm::TLM_ACCEPTED;
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
tlm::tlm_sync_enum OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::nb_transport_fw(unsigned int index, tlm::tlm_generic_payload& trans,
                               tlm::tlm_phase& phase,
                               sc_core::sc_time& t){
  assert(m_nb_peq);
  assert(m_reset_peq);
  
  if (t>sc_core::SC_ZERO_TIME || m_use_sp){
    //if we have a timing annotation or use delta cycle protection we push the transport into the PEQ
    if (phase==BEGIN_RESET || phase==END_RESET) m_reset_peq->notify(index, trans, phase, t);
    else m_nb_peq->notify(index, trans, phase, t);
    if (!ocp_config_support_type::m_resolved_params[index].cmdaccept && phase==tlm::BEGIN_REQ){
      //if there is no CmdAccept control flow and we get a BEGIN_REQ, we accept it on behalf of the user code
      phase=tlm::END_REQ;
      return tlm::TLM_UPDATED;
    }
    else
    if (!ocp_config_support_type::m_resolved_params[index].dataaccept && phase==BEGIN_DATA){
      //if there is no DataAccept control flow and we get a BEGIN_DATA, we accept it on behalf of the user code
      phase=END_DATA;
      return tlm::TLM_UPDATED;
    }
    else    
      return tlm::TLM_ACCEPTED;
  }
  else
    return m_nb_fw_functor(index, trans, phase, t);
  
  //unreachable. Just to satisfy the compiler
  return tlm::TLM_ACCEPTED;
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::nb_transport_fw_peq(tlm::tlm_generic_payload& trans,
                               const tlm::tlm_phase& phase){
  tlm::tlm_phase ph=phase;
  m_null_time=sc_core::SC_ZERO_TIME;
  tlm::tlm_sync_enum retVal=m_nb_fw_functor(trans, ph, m_null_time);
  if (!ocp_config_support_type::m_resolved_params[0].cmdaccept && phase==tlm::BEGIN_REQ)
    //in case we did an automatic accept make sure the user would have accepted as well
    assert(retVal==tlm::TLM_UPDATED && ph==tlm::END_REQ && m_null_time==sc_core::SC_ZERO_TIME);
  else
  if (!ocp_config_support_type::m_resolved_params[0].dataaccept && phase==BEGIN_DATA)
    //in case we did an automatic accept make sure the user would have accepted as well
    assert(retVal==tlm::TLM_UPDATED && ph==OCPIP_VERSION::END_DATA && m_null_time==sc_core::SC_ZERO_TIME);
  else
    //this must be an updated return we did not cover with an autoaccept
    if (retVal==tlm::TLM_UPDATED) {
      retVal=(*this)->nb_transport_bw(trans, ph, m_null_time);
      //given the OCP TLM rules, whatever we got by TLM_UPDATED, can only be accepted by the other end
      assert(retVal==tlm::TLM_ACCEPTED);
    }
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::nb_transport_fw_peq(unsigned int index, tlm::tlm_generic_payload& trans,
                               const tlm::tlm_phase& phase){
  tlm::tlm_phase ph=phase;
  m_null_time=sc_core::SC_ZERO_TIME;
  tlm::tlm_sync_enum retVal=m_nb_fw_functor(index, trans, ph, m_null_time);
  if (!ocp_config_support_type::m_resolved_params[index].cmdaccept && phase==tlm::BEGIN_REQ)
    //in case we did an automatic accept make sure the user would have accepted as well
    assert(retVal==tlm::TLM_UPDATED && ph==tlm::END_REQ && m_null_time==sc_core::SC_ZERO_TIME);
  else
  if (!ocp_config_support_type::m_resolved_params[index].dataaccept && phase==BEGIN_DATA)
    //in case we did an automatic accept make sure the user would have accepted as well
    assert(retVal==tlm::TLM_UPDATED && ph==OCPIP_VERSION::END_DATA && m_null_time==sc_core::SC_ZERO_TIME);
  else
    //this must be an updated return we did not cover with an autoaccept
    if (retVal==tlm::TLM_UPDATED) {
      retVal=(*this)[index]->nb_transport_bw(trans, ph, m_null_time);
      //given the OCP TLM rules, whatever we got by TLM_UPDATED, can only be accepted by the other end
      assert(retVal==tlm::TLM_ACCEPTED);
    }
}

//called when the master socket has changed its timing
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::set_master_timing(const ocp_tl1_master_timing& other_timing){
  if (!m_time_cb.empty()) m_time_cb(other_timing);
}

//helper to transmit the timing information to the connected master
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::transmit_timing(unsigned int index){
  assert((ocp_config_support_type::other_ocp.size()>index && ocp_config_support_type::other_ocp[index]!=NULL) 
          && "Socket used without default timing, but connected to a non OCP socket.");
  ocp_config_support_type::other_ocp[index]->set_slave_timing(m_timing[index]);
}

//called by the ocp base socket as soon as it was successfully bound to another socket
// at this time extension/phase based bindability has already been checked by the ocp base socket
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::bound_to(const std::string& other_type, bindability_base_type* other, unsigned int index){
  if (index>=m_bound.size())  m_bound.resize(index+1, false);
  if (index>=m_timing.size()) m_timing.resize(index+1, m_one_for_all_timing);
  m_bound[index]=true;
  ocp_config_support_type::check_config(other_type, other, index);
  if (m_timing[index]!=ocp_tl1_slave_timing::get_default_timing()) transmit_timing(index);
}

//get the type string of this socket. This function is required by ocp base socket
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
std::string& OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::get_type_string(){return s_type;}

//at start of simulation the ocp socket makes sure that it is not generic anymore
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::start_of_simulation(){
  unsigned int tmp;
  if (!ocp_config_support_type::is_configured()){
    std::cerr<<base_type::name()<<": The OCP socket did not receive a call to set_ocp_config until simulation started."<<std::endl;
    exit(1);
  }  
  if (ocp_config_support_type::is_generic(tmp)){
    std::cerr<<base_type::name()<<": The generic OCP socket (index"<<tmp<<") did not get a configuration until simulation stared."<<std::endl;
    exit(1);
  }
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::before_end_of_elaboration(){
  base_type::before_end_of_elaboration();
  //ocp_config_support_type::auto_set_config();
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
std::string OCPIP_VERSION::ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::s_type("ocp_slave_socket");

