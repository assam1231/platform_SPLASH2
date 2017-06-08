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
//  Description :  This file contains the ocp_master_socket that ocp master 
//                 modules shall use.
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
   
#if (TLM_VERSION_MAJOR>2 | TLM_VERSION_MINOR>0 | TLM_VERSION_PATCH>0)  
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::reset(){
  if (m_nb_peq) m_nb_peq->reset();
}
#endif

//Ctor to be used for modules that are not interested in configuration callbacks
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::ocp_master_socket
  (const char* name, ocp_layer_ids layer, allocation_scheme_type scheme)
  : base_type(name, scheme)
  , ocp_config_support_type(base_type::name(), BUSWIDTH, NUM_BINDS, this, ocp_config_support_type::OCP_MST, layer)
  , m_use_sp(false)
  , m_nb_peq(NULL)
  , m_reset_peq(NULL)
  , m_if_wrapper(this, m_nb_peq, layer)
{
}

//Ctor that allows for registering a configuration listener callback
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
template <typename MODULE>
OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::ocp_master_socket(
    const char* name
  , ocp_layer_ids layer
  , MODULE* owner
  , void (MODULE::*timing_cb)(ocp_tl1_slave_timing)
  , allocation_scheme_type scheme
)
  : base_type(name, scheme)
  , ocp_config_support_type(base_type::name(), BUSWIDTH, NUM_BINDS, this, ocp_config_support_type::OCP_MST, layer)
  , m_use_sp(false)
  , m_nb_peq(NULL)
  , m_reset_peq(NULL)
  , m_if_wrapper(this, m_nb_peq, layer)
{
  m_time_cb.set_function(owner, timing_cb);
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::~ocp_master_socket(){
  if (m_nb_peq)    delete m_nb_peq;
  if (m_reset_peq) delete m_reset_peq;
}

//assign non default timing to this socket
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::set_master_timing(const ocp_tl1_master_timing& my_timing){
  m_one_for_all_timing=my_timing; //set local default timing  
  for (unsigned int i=0; i<m_timing.size(); i++){ //if there are already some bound targets, inform them
    if (my_timing!=m_timing[i]){
      m_timing[i]=my_timing;
      if (m_bound[i]) transmit_timing(i);
    }
  }
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::set_master_timing(const ocp_tl1_master_timing& my_timing, unsigned int index){
  if (index>=m_timing.size()) m_timing.resize(index+1, m_one_for_all_timing);
  if (index>=m_bound.size())  m_bound.resize(index+1, false);
  if (my_timing!=m_timing[index]){
    //std::cout<<"    diff in timing of "<<index<<std::endl;
    m_timing[index]=my_timing;
    if (m_bound[index]) transmit_timing(index);
    //else std::cout<<"    not bound yet in timing of "<<index<<std::endl;
  }
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::nb_transport_bw_peq(tlm::tlm_generic_payload& trans,
                               const tlm::tlm_phase& phase){
  tlm::tlm_phase ph=phase;
  m_null_time=sc_core::SC_ZERO_TIME;
  tlm::tlm_sync_enum retVal=m_nb_bw_functor(trans, ph, m_null_time);

  if (!ocp_config_support_type::m_resolved_params[0].respaccept && phase==tlm::BEGIN_RESP)
    //in case we did an automatic accept make sure the user would have accepted as well
    assert(retVal==tlm::TLM_UPDATED && ph==tlm::END_RESP && m_null_time==sc_core::SC_ZERO_TIME);
  else
    //this must be an updated return we did not cover with an autoaccept
    if (retVal==tlm::TLM_UPDATED) {
      retVal=(*this)->nb_transport_fw(trans, ph, m_null_time);
      //given the OCP TLM rules, whatever we got by TLM_UPDATED, can only be accepted by the other end
      assert(retVal==tlm::TLM_ACCEPTED);
    }
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::nb_transport_bw_peq(unsigned int index, tlm::tlm_generic_payload& trans,
                               const tlm::tlm_phase& phase){
  tlm::tlm_phase ph=phase;
  m_null_time=sc_core::SC_ZERO_TIME;
  tlm::tlm_sync_enum retVal=m_nb_bw_functor(index, trans, ph, m_null_time);

  if (!ocp_config_support_type::m_resolved_params[index].respaccept && phase==tlm::BEGIN_RESP)
    //in case we did an automatic accept make sure the user would have accepted as well
    assert(retVal==tlm::TLM_UPDATED && ph==tlm::END_RESP && m_null_time==sc_core::SC_ZERO_TIME);
  else
    //this must be an updated return we did not cover with an autoaccept
    if (retVal==tlm::TLM_UPDATED) {
      retVal=(*this)[index]->nb_transport_fw(trans, ph, m_null_time);
      //given the OCP TLM rules, whatever we got by TLM_UPDATED, can only be accepted by the other end
      assert(retVal==tlm::TLM_ACCEPTED);
    }
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::activate_delta_cycle_protection(){
  std::cerr<<"Warning: "<<base_type::name()<<" use of deprecated function activate_delta_cycle_protection() use activate_synchronization_protection() instead."<<std::endl;
  activate_synchronization_protection();
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::activate_synchronization_protection(){
  m_use_sp=true;
  if (m_nb_peq) {
    assert(m_reset_peq);
    m_nb_peq->activate_sp();
    m_reset_peq->activate_sp();
  }
  else 
  if (!base_type::m_nb_bw_functor.empty()){
    std::cerr<<"Warning: "<<base_type::name()<<" synchronization protection (SP) activated, but no peq is used. SP will not work."<<std::endl;
  }
}


//register an nb_transport like function
// It is called after a 'real' nb_transport arrived and the time
//  argument of this nb_transport has expired.
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
template <typename MODULE>
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::register_nb_transport_bw(
    MODULE* mod
  , typename nb_cb_type_selector<MODULE, NUM_BINDS!=1>::nb_cb_type cb
  , bool use_peq
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
    m_nb_bw_functor.set_function(mod, cb);
    base_type::register_nb_transport_bw(this, &ocp_master_socket::nb_transport_bw);
    m_nb_peq=new peq_type(this, &ocp_master_socket::nb_transport_bw_peq, this);
    m_reset_peq= new peq_type(this, &ocp_master_socket::nb_transport_bw_peq, this);
    if (m_use_sp) {
      m_nb_peq->activate_sp();
      m_reset_peq->activate_sp();
    }
    //m_if_wrapper.set_has_peq(true);
  }
  else {
    if (m_use_sp){
      std::cerr<<"Warning: "<<base_type::name()<<" synchronization protection (SP) activated, but no peq is used. SP will not work."<<std::endl;
    }
    if (m_nb_peq) {
      delete m_nb_peq; m_nb_peq=NULL;
      assert(m_reset_peq);
      delete m_reset_peq; m_reset_peq=NULL;
    }
    base_type::register_nb_transport_bw(mod, cb);
    //m_if_wrapper.set_has_peq(false);
  }
}


template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
template <typename MODULE>
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::register_nb_transport_bw(
    MODULE* mod
  , typename nb_cb_type_selector<MODULE, NUM_BINDS!=1>::nb_cb_type cb
){
  //if no peq mode is provided we default to peq usage if we can do TL1
  if (ocp_config_support_type::m_layer & ocp_tl1)
    register_nb_transport_bw(mod, cb, true);
  else
    register_nb_transport_bw(mod, cb, false);
}

/*
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
template<typename MODULE>
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::register_nb_transport_bw( MODULE* mod
                             , tlm::tlm_sync_enum (MODULE::*cb)(unsigned int, tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&)
                             , bool use_peq)
{
  if (use_peq){
    std::cerr<<"Error: "<<base_type::name()<<" built-in PEQ cannot be used with multi sockets."<<std::endl;
    exit(1);
  }
  base_type::register_nb_transport_bw(mod, cb);
}                            
*/

//nb_transport_nw is overridden to either feed it into the PEQ
// when the peq nb_transport was registered or to feed it directly
//  into the slave (through the ocp base socket)
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
tlm::tlm_sync_enum OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::nb_transport_bw(tlm::tlm_generic_payload& trans,
                               tlm::tlm_phase& phase,
                               sc_core::sc_time& t){
  assert(m_nb_peq);
  assert(m_reset_peq);
  
  if (t>sc_core::SC_ZERO_TIME || m_use_sp){
    //if we have a timing annotation or use delta cycle protection we push the transport into the PEQ
    if (phase==BEGIN_RESET || phase==END_RESET) m_reset_peq->notify(trans, phase, t);
    else m_nb_peq->notify(trans, phase, t);
    if (!ocp_config_support_type::m_resolved_params[0].respaccept && phase==tlm::BEGIN_RESP){
      //if there is no RespAccept control flow and we get a BEGIN_RESP, we accept it on behalf of the user code
      phase=tlm::END_RESP;
      return tlm::TLM_UPDATED;
    }
    else
      return tlm::TLM_ACCEPTED;
  }
  else
    return m_nb_bw_functor(trans, phase, t);
  
  //unreachable. Just to staisfy the compiler
  return tlm::TLM_ACCEPTED;
}

template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
tlm::tlm_sync_enum OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::nb_transport_bw(unsigned int index, tlm::tlm_generic_payload& trans,
                               tlm::tlm_phase& phase,
                               sc_core::sc_time& t){
  assert(m_nb_peq);
  assert(m_reset_peq);
  
  if (t>sc_core::SC_ZERO_TIME || m_use_sp){
    //if we have a timing annotation or use delta cycle protection we push the transport into the PEQ
    if (phase==BEGIN_RESET || phase==END_RESET) m_reset_peq->notify(index, trans, phase, t);
    else m_nb_peq->notify(index, trans, phase, t);
    if (!ocp_config_support_type::m_resolved_params[index].respaccept && phase==tlm::BEGIN_RESP){
      //if there is no RespAccept control flow and we get a BEGIN_RESP, we accept it on behalf of the user code
      phase=tlm::END_RESP;
      return tlm::TLM_UPDATED;
    }
    else
      return tlm::TLM_ACCEPTED;
  }
  else
    return m_nb_bw_functor(index, trans, phase, t);
  
  //unreachable. Just to staisfy the compiler
  return tlm::TLM_ACCEPTED;
}

//called when the slave socket has changed its timing
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::set_slave_timing(const ocp_tl1_slave_timing& other_timing){
  if (!m_time_cb.empty()) m_time_cb(other_timing);
}

//helper to transmit the timing information to the connected master
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::transmit_timing(unsigned int index){
  assert((ocp_config_support_type::other_ocp.size()>index && ocp_config_support_type::other_ocp[index]!=NULL) 
          && "Socket used without default timing, but connected to a non OCP socket.");
  ocp_config_support_type::other_ocp[index]->set_master_timing(m_timing[index]);
}

//called by the ocp base socket as soon as it was successfully bound to another socket
// at this time extension/phase based bindability has already been checked by the ocp base socket
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::bound_to(const std::string& other_type, bindability_base_type* other, unsigned int index){
  if (index>=m_bound.size())  m_bound.resize(index+1, false);
  if (index>=m_timing.size()) m_timing.resize(index+1, m_one_for_all_timing);
  m_bound[index]=true;
  ocp_config_support_type::check_config(other_type, other, index);
  if (m_timing[index]!=ocp_tl1_master_timing::get_default_timing()) transmit_timing(index);
}

//at start of simulation the ocp socket makes sure that it is not generic anymore
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::start_of_simulation(){
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
void OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::before_end_of_elaboration(){
  base_type::before_end_of_elaboration();
  //ocp_config_support_type::auto_set_config();
}

//get the type string of this socket. This function is required by ocp base socket
template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
std::string& OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::get_type_string(){return s_type;}


template <unsigned int BUSWIDTH, unsigned int NUM_BINDS, typename BASE_SOCKET>
std::string OCPIP_VERSION::ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>::s_type("ocp_master_socket");


