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
//  Description :  This file contains the ocp_socket_base from which the
//                 ocp_config_support is derived. It enables timing
//                 information and configuration distribution.
//
//                 The file also contains the ocp_config_support class from
//                 which both the ocp master and slave socket will be derived.
//                 It deals with the socket configuration and the ocp bindability
//                 checks.
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//This function registers a configuration listener with the socket
template <typename BIND_BASE>
template <typename MODULE>
void OCPIP_VERSION::ocp_config_support<BIND_BASE>::register_configuration_listener_callback(
    MODULE* owner
  , void (MODULE::*cb)(const ocp_parameters&, const std::string&))
{
  if (m_parent_binds!=1){
    std::cerr<<"Warning: Configuration callback without multi socket rank id registered with mutli socket "<<m_parent_name<<"."<<std::endl;
  }
  const ocp_config_support<BIND_BASE>* tmp=get_bottom_of_hierarchy("register_configuration_listener_callback", m_parent_name);

  tmp->m_conf_cb.push_back(new config_callback_functor<BIND_BASE>());
  tmp->m_conf_cb.back()->set_function(owner,cb);
}

//This function registers a configuration listener with the socket
template <typename BIND_BASE>
template <typename MODULE>
void OCPIP_VERSION::ocp_config_support<BIND_BASE>::register_configuration_listener_callback(
    MODULE* owner
  , void (MODULE::*cb)(const ocp_parameters&, const std::string&, unsigned int))
{
  if (m_parent_binds==1){
    std::cerr<<"Warning: Configuration callback with multi socket rank id registered with single socket "<<m_parent_name<<"."<<std::endl;
  }

  const ocp_config_support<BIND_BASE>* tmp=get_bottom_of_hierarchy("register_configuration_listener_callback", m_parent_name);
  tmp->m_conf_with_index_cb.push_back(new config_callback_with_index_functor<BIND_BASE>());
  tmp->m_conf_with_index_cb.back()->set_function(owner,cb);
}

template <typename BIND_BASE>
OCPIP_VERSION::ocp_config_support<BIND_BASE>::ocp_config_support(
    const char* parent_name
  , unsigned int parent_bus_width
  , unsigned int parent_binds
  , bind_checker_base_type* parent
  , master_or_slave_enum mNs
  , ocp_layer_ids layer
) :
  m_parent(parent),
  m_parent_name(parent_name),
  m_parent_bus_width(parent_bus_width),
  m_parent_binds(parent_binds),
  m_master_or_slave(mNs),
  m_layer(layer),
  m_identified_as_bottom_of_hierarchy_by("")
{
  //m_params.resize(1);
  m_tb_txn.set_extension(&m_ctb_ext);
  m_tb_txn.set_extension(&m_dtb_ext);
  m_tb_txn.set_extension(&m_rtb_ext);
  m_tb_txn.set_extension(&m_tl2_timing_ext);
  m_tb_txn.set_mm(this);
  m_tb_txn.acquire(); //need to do this to satisfy TLM-2.0 rules
}

template <typename BIND_BASE>
OCPIP_VERSION::ocp_config_support<BIND_BASE>::~ocp_config_support(){
  m_tb_txn.clear_extension(&m_ctb_ext);
  m_tb_txn.clear_extension(&m_dtb_ext);
  m_tb_txn.clear_extension(&m_rtb_ext);
  m_tb_txn.clear_extension(&m_tl2_timing_ext);
  for (unsigned int i=0; i<m_conf_cb.size(); i++)
    delete m_conf_cb[i];
  for (unsigned int i=0; i<m_conf_with_index_cb.size(); i++)
    delete m_conf_with_index_cb[i];

}

//This function returns the pointer to the transaction that
// is supposed to be used to signal thread busy changes
template <typename BIND_BASE>
void OCPIP_VERSION::ocp_config_support<BIND_BASE>::free(tlm::tlm_generic_payload*){
  assert(0 && "This should never be called");
}

#if (TLM_VERSION_MAJOR>2 | TLM_VERSION_MINOR>0 | TLM_VERSION_PATCH>0)
//This function returns the pointer to the transaction that
// is supposed to be used to signal thread busy changes
template <typename BIND_BASE>
tlm::tlm_generic_payload* OCPIP_VERSION::ocp_config_support<BIND_BASE>::get_reset_transaction(){
  return &m_tb_txn;
}
#endif

//This function returns the pointer to the transaction that
// is supposed to be used to signal thread busy changes
template <typename BIND_BASE>
tlm::tlm_generic_payload* OCPIP_VERSION::ocp_config_support<BIND_BASE>::get_tb_transaction(){
  return &m_tb_txn;
}

// This function returns the pointer to the transaction that
// is supposed to be used to change timing annotations in TL2
template <typename BIND_BASE>
tlm::tlm_generic_payload* OCPIP_VERSION::ocp_config_support<BIND_BASE>::get_tl2_timing_transaction()
{
  return &m_tb_txn;
}

// This function returns the pointer to the transaction that
// is supposed to be used to signal interrupt changes
template <typename BIND_BASE>
tlm::tlm_generic_payload* OCPIP_VERSION::ocp_config_support<BIND_BASE>::get_interrupt_transaction()
{
  return &m_tb_txn;
}

// This function returns the pointer to the transaction that
// is supposed to be used to signal m/s-error changes
template <typename BIND_BASE>
tlm::tlm_generic_payload*
OCPIP_VERSION::ocp_config_support<BIND_BASE>::get_error_transaction()
{
  return &m_tb_txn;
}

// This function returns the pointer to the transaction that
// is supposed to be used to signal m/s-flag changes
template <typename BIND_BASE>
tlm::tlm_generic_payload*
OCPIP_VERSION::ocp_config_support<BIND_BASE>::get_flag_transaction()
{
  return &m_tb_txn;
}

//This function is used to assign an ocp parameters set to a
// socket
template <typename BIND_BASE>
void OCPIP_VERSION::ocp_config_support<BIND_BASE>::set_ocp_config(const ocp_parameters& config){
  const ocp_config_support<BIND_BASE>* bottom=get_bottom_of_hierarchy("set_ocp_config", m_parent_name);
  ocp_parameters& original_params=bottom->m_original_params; //get the params from the bottom (could be ourselve)

  bool force_reeval=false;
  if (original_params.config_state!=ocp_unconfigured){ //we already have received a config
    //okay we allow to override a generic config with a precise one.
    // we also allow overriding a precise one as long as it has not been involved in checks yet
    // but we do not allow to override a precise config with another one after there is at least one resolved config
    if ((original_params.config_state!=ocp_generic) && m_resolved_params.size()){
      std::cerr<<bottom->m_parent_name<<" Error: Double configuration."<<std::endl;
      if (m_parent_name==original_params.name){
        std::cerr<<"  Two different configuration directly assigned to the same socket."<<std::endl;
      }
      else{
        std::cerr<<"  Two different configuration assigned to the same socket via hierarchical configuration."<<std::endl
                 <<"   First was assigned to \""<<original_params.name<<"\" the other was assigned to \""<<m_parent_name<<"\"."<<std::endl;
      }
      exit(1);
    }
    force_reeval=original_params.diff(config); //force reeval at other side if something changed
    // this is important, because we might have just changed a parameter that does not effect
    // the used extension (so the ocp base socket would do nothing)
  }
  original_params=config;
  original_params.name=m_parent_name;
  if (config.config_state!=ocp_generic) {
    original_params.config_state=ocp_configured;
  }
  ocp_parameters tmp=original_params;//.get_params();
  config_type gs_cfg=tmp.template convert_to_base_socket_config<BIND_BASE>(
    (m_master_or_slave==OCP_MST)?true:false, m_layer
  );
  gs_cfg.set_force_reeval(force_reeval);
  m_parent->set_config(gs_cfg);
}

//call this function at before end of elab
//it checks if the ocp base socket has been configured
// if not it configures it now with what we have (maybe it was configured via a configuration environment)
template <typename BIND_BASE>
void OCPIP_VERSION::ocp_config_support<BIND_BASE>::auto_set_config(){
  if (m_resolved_params.size()>0){
    std::cerr<<m_parent_name<<" This should never have happened. Please contact OCPIP SLDWG."<<std::endl;
    exit(1);
  }

  const ocp_config_support<BIND_BASE>* bottom=get_bottom_of_hierarchy("auto_set_config", m_parent_name); //will always deliver real bottom
  if (m_identified_as_bottom_of_hierarchy_by!=std::string("")){ //we were identified as the bottom of hierarchy
    if (bottom!=this){ //so we were identified as bottom, but are not bottom now!
      std::cerr<<"OCP Socket "<<m_parent_name<<" was identified as bottom of hierarchy by a call to "<<m_identified_as_bottom_of_hierarchy_by<<std::endl
               <<"  but is not the bottom of the hierarchy any more. That means the call happened before"<<std::endl
               <<"  the socket binding was complete. Check if you bound the socket before registering callbacks. "<<std::endl
               <<"  Make sure that after calling the functions no further bindings take place in lower hierarchy layers."<<std::endl;
      abort();
    }
  }

  if (bottom!=this){ //this is a hierachical socket. No need to check stuff
    return;
  }
  ocp_parameters tmp=m_original_params;//.get_params();
  if (tmp.config_state==ocp_unconfigured){
    std::cerr<<"OCP Socket "<<m_parent_name<<" has no configuration (it is not even generic) after construction time."<<std::endl;
    abort();
  }
  if (m_parent->get_recent_config().get_invalid()) {//base socket has not been configured yet
    config_type gs_cfg=tmp.template convert_to_base_socket_config<BIND_BASE>(
      (m_master_or_slave==OCP_MST)?true:false, m_layer
    );
    m_parent->set_config(gs_cfg);
  }
}

template <typename BIND_BASE>
const OCPIP_VERSION::ocp_config_support<BIND_BASE>*
OCPIP_VERSION::ocp_config_support<BIND_BASE>::get_bottom_of_hierarchy(const char* cause, const std::string& caller) const{
  BIND_BASE* parent_as_bind_checker=dynamic_cast<BIND_BASE*>(m_parent);
  if (!parent_as_bind_checker){
    std::cerr<<m_parent_name<<" unexpected class hierarchy for this socket (1)!"<<std::endl;
    abort();
  }
  sc_core::sc_interface* bottom=parent_as_bind_checker->get_bottom_of_hierarchy();
  if (!bottom){
  /*
#ifndef OCP_PERMISSIVE
    std::cerr<<"WARNING: "<<m_parent_name<<" is assumed to be the bottom of the hierarchy altough that cannot be verified at the current "
               "stage of elaboration. Cause is a call to "<<cause<<
               " during construction. Move that call to before_end_of_elaboration or compile with the macro OCP_PERMISSIVE "
               "defined to avoid this warning"<<std::endl;
#endif
*/
    m_identified_as_bottom_of_hierarchy_by=caller+std::string(".")+std::string(cause);
    return this;
  }
  typename BIND_BASE::root_type* bottom_as_binder=dynamic_cast<typename BIND_BASE::root_type*>(bottom);
  if (!bottom_as_binder) {
    std::cerr<<m_parent_name<<" unexpected class hierarchy for this socket (2)!"<<std::endl;
    abort();
  }
  else{
    typename BIND_BASE::bind_base_type* bottom_as_bind_base=static_cast<typename BIND_BASE::bind_base_type*>(bottom_as_binder->get_owner());
    if (!bottom_as_bind_base){
      std::cerr<<m_parent_name<<" unexpected class hierarchy for this socket (3)!"<<std::endl;
      abort();
    }
    else{
      ocp_config_support<BIND_BASE>* bottom_as_ocp_config_support=dynamic_cast<ocp_config_support<BIND_BASE>*>(bottom_as_bind_base);
      if (!bottom_as_ocp_config_support){
        std::cerr<<m_parent_name<<" unexpected class hierarchy for this socket (4)!"<<std::endl;
        abort();
      }
      else{
        return bottom_as_ocp_config_support;
      }
    }
  }
  abort();
  return this;
}

//this function gets the current config from a socket
template <typename BIND_BASE>
OCPIP_VERSION::ocp_parameters OCPIP_VERSION::ocp_config_support<BIND_BASE>::get_resolved_ocp_config(unsigned int index) const{
  //test if we are the bottom of the hierarchy
  const ocp_config_support<BIND_BASE>* bottom=get_bottom_of_hierarchy("get_resolved_ocp_config", m_parent_name);
  if(index >= bottom->m_resolved_params.size()) {
    std::cerr<<m_parent_name<<": resolved OCP config requested before resolution!"<<std::endl;
    std::cerr<<m_parent_name<<":     consider moving get_resolved_ocp_config() to start_of_simulation()"<<std::endl;
    std::cerr<<m_parent_name<<":     or registering a callback function to process the OCP configuration"<<std::endl;
    abort();
  }
  return bottom->m_resolved_params[index];
}

template <typename BIND_BASE>
OCPIP_VERSION::ocp_parameters OCPIP_VERSION::ocp_config_support<BIND_BASE>::get_ocp_config() const{
  return get_bottom_of_hierarchy("get_ocp_config", m_parent_name)->m_original_params;//.get_params();
}

//this function sets a socket in the ocp generic mode
template <typename BIND_BASE>
void OCPIP_VERSION::ocp_config_support<BIND_BASE>::make_generic(){
  const ocp_config_support<BIND_BASE>* bottom=get_bottom_of_hierarchy("make_generic", m_parent_name);
  if (bottom!=this){
    std::cerr<<m_parent_name<<" Switch a hierarchical socket into generic mode hs no effect."<<std::endl;
    return;
  }
  if (m_resolved_params.size()>0){
    std::cerr<<m_parent_name<<" Attempt to switch a socket into generic mode after binding was already done."<<std::endl;
    exit(1);
  }
  m_original_params.config_state=ocp_generic;
  ocp_parameters tmp=m_original_params;//.get_params();
  config_type gs_cfg=tmp.template convert_to_base_socket_config<BIND_BASE>((m_master_or_slave==OCP_MST)?true:false, m_layer);
  m_parent->set_config(gs_cfg);
}

//this function can be used to check if a socket is generic or not
template <typename BIND_BASE>
bool OCPIP_VERSION::ocp_config_support<BIND_BASE>::is_generic(unsigned int& index)
{
  const ocp_config_support<BIND_BASE>* bottom=get_bottom_of_hierarchy("is_generic", m_parent_name);
  if (bottom->m_resolved_params.size()){
    for (index=0; index<bottom->m_resolved_params.size(); index++){
      if (((const ocp_config_state_enum&)bottom->m_resolved_params[index].config_state)==ocp_generic) return true;
    }
  }
  else
    return ((const ocp_config_state_enum&)bottom->m_original_params.config_state)==ocp_generic;
  return false;
}

//this function can be used to check if a socket is validly configured
template <typename BIND_BASE>
bool OCPIP_VERSION::ocp_config_support<BIND_BASE>::is_configured()
{
  const ocp_config_support<BIND_BASE>* bottom=get_bottom_of_hierarchy("is_configured", m_parent_name); //will always deliver real bottom
  if (bottom!=this){ //this is a hierachical socket. No need to check stuff
    return true;
  }
  ocp_parameters tmp=m_original_params;//.get_params();
  if (tmp.config_state==ocp_unconfigured) return false;
  else return true;
}

/*
template <typename BIND_BASE>
void check_against_ocp_conf( master_or_slave_enum mOrS
                           , ocp_parameters& original_params
                           , const ocp_parameters& other_params
                           , ocp_parameters& resolved_params)
*/
//this function checks if an ocp socket is compatible with another socket
template <typename BIND_BASE>
void OCPIP_VERSION::ocp_config_support<BIND_BASE>::check_config(
    const std::string& other_type
  , bindability_base_type* other
  , unsigned int index)
{
  if (index>=other_ocp.size()) other_ocp.resize(index+1, NULL);
  if (index>=m_resolved_params.size()) {
    m_resolved_params.resize(index+1, m_original_params);
  }
  if (other){  //other is a ocp base socket
    other_ocp[index]=dynamic_cast<ocp_socket_base* >(other);
    if (other_ocp[index]){ //it even is an ocp socket
      check_against_ocp_conf(m_master_or_slave, m_original_params, other_ocp[index]->get_ocp_config(), m_resolved_params[index]);
    }
    else{ //is is a non-ocp ocp base socket
      ocp_parameters tmp;
      tmp.template init_from_base_socket_config<BIND_BASE>(m_parent->get_recent_config(),m_parent_bus_width);
      check_against_ocp_conf(m_master_or_slave, m_original_params, tmp, m_resolved_params[index]);
    }
  }
  else {//it is not a ocp base socket, so we assume plain OSCI (compatibility on extension level is there, so we just set up an ocp conf for that)
    ocp_parameters tmp;
    tmp.init_as_osci_config(m_parent_bus_width);
    check_against_ocp_conf(m_master_or_slave, m_original_params, tmp, m_resolved_params[index]);
  }
  if (m_conf_cb.size()){
    for (unsigned int i=0; i<m_conf_cb.size(); i++)
      (*m_conf_cb[i])(m_resolved_params[index], m_parent_name);
  }
  if (m_conf_with_index_cb.size()){
    for (unsigned int i=0; i<m_conf_with_index_cb.size(); i++)
      (*m_conf_with_index_cb[i])(m_resolved_params[index], m_parent_name, index);
  }
}

//All functions below are taken from the OCP TL1 channel kit's tl_config_manager (slightly adjusted)
template <typename BIND_BASE>
bool OCPIP_VERSION::ocp_config_support<BIND_BASE>::compareParams(const int& master_param, const int& slave_param, int& final){
  if((const int&)master_param!=(const int&)slave_param){
    return false;
  }
  final=master_param;
  return true;
}

template <typename BIND_BASE>
bool OCPIP_VERSION::ocp_config_support<BIND_BASE>::compareParams(const bool& master_param, const bool& slave_param, bool& final){
  if((const bool&)master_param!=(const bool&)slave_param){
    return false;
  }
  final=master_param;
  return true;
}

template <typename BIND_BASE>
bool OCPIP_VERSION::ocp_config_support<BIND_BASE>::compareParams(const float& master_param, const float& slave_param, float& final){
  if((const float&)master_param!=(const float&)slave_param){
    return false;
  }
  final=master_param;
  return true;
}

#define COMPARE(value) \
compareParams(master_params.value, slave_params.value, resolved_params.value)

#define FATAL(value) \
std::cerr<<"ERROR: FATAL mismatch between master socket "<<master_params.name<<" and slave socket "<<slave_params.name<<" in parameter: "<<#value<<std::endl<<"       master's configuration specifies: "<<(master_params.value)<<std::endl \
<<"       slave's configuration specifies: "<<(slave_params.value)<<std::endl; \
mismatch=true;

#define CRITICAL(value) \
std::cerr<<"WARNING: CRITICAL mismatch between master socket "<<master_params.name<<" and slave socket "<<slave_params.name<<" in parameter: "<<#value<<std::endl<<"         master's configuration specifies: "<<(master_params.value)<<std::endl \
<<"         slave's configuration specifies: "<<(slave_params.value)<<std::endl;

#define NONCRITICAL(value) \
std::cerr<<"WARNING: NONCRITICAL mismatch between master socket "<<master_params.name<<" and slave socket "<<slave_params.name<<" in parameter: "<<#value<<std::endl<<"         master's configuration specifies: "<<(master_params.value)<<std::endl \
<<"         slave's configuration specifies: "<<(slave_params.value)<<std::endl;

#define MASTERMUSTNOT(message) \
std::cerr<<"         The Master must not (under no circumstances) "<<message<<", because the slave cannot handle that."<<std::endl;

#define MASTERHASTO(message)\
std::cerr<<"         The Master has to (under all circumstances) " <<message<<", because the slave requires that."<<std::endl;

#define SLAVEWILL(message)\
std::cerr<<"         The Slave will " <<message<<", keep that in mind."<<std::endl;

#define MASTERWILL(message)\
std::cerr<<"         The Master will " <<message<<", keep that in mind."<<std::endl;

template <typename BIND_BASE>
void OCPIP_VERSION::ocp_config_support<BIND_BASE>::check_against_ocp_conf( master_or_slave_enum mOrS
                           , const ocp_parameters& original_params
                           , const ocp_parameters& other_params
                           , ocp_parameters& resolved_params)

//void OCPIP_VERSION::ocp_config_support<BIND_BASE>::check_against_ocp_conf(const ocp_parameters& other_params, unsigned int index)
{
  //ocp_parameters tmp=m_original_params.get_params();
  const ocp_parameters& master_params=(mOrS==OCP_MST)? original_params : other_params;
  const ocp_parameters& slave_params =(mOrS==OCP_SLV)? original_params : other_params;

  if (original_params.config_state==ocp_generic) { resolved_params=other_params; return;}
  if (other_params.config_state==ocp_generic) {
    resolved_params=original_params;
    return;
  }

  bool mismatch=false;
  if (!COMPARE(ocp_version)) {
    FATAL(ocp_version);
  }
  if (!COMPARE(broadcast_enable)) {
    if (master_params.broadcast_enable){
      CRITICAL(broadcast_enable);
      MASTERMUSTNOT("issue a broadcast transfer");
    }
    resolved_params.broadcast_enable=false;  //in case of mismatch no broadcasts will take place
  }
  if (!COMPARE(burst_aligned)){
    if (slave_params.burst_aligned){
      CRITICAL(burst_aligned);
      MASTERHASTO("keep INCR bursts aligned");
    }
    resolved_params.burst_aligned=true;  //in case of mismatch all INCR burst will be aligned
  }
  if (!COMPARE(burstseq_dflt1_enable)){
    if (master_params.burstseq_dflt1_enable) {
      CRITICAL(burstseq_dflt1_enable);
      MASTERMUSTNOT("use the DFLT1 burst sequence");
    }
    resolved_params.burstseq_dflt1_enable=false;  //in case of mismatch no DFLT1 bursts will take place
  }
  if (!COMPARE(burstseq_dflt2_enable)){
    if (master_params.burstseq_dflt2_enable) {
      CRITICAL(burstseq_dflt2_enable);
      MASTERMUSTNOT("use the DFLT2 burst sequence");
    }
    resolved_params.burstseq_dflt2_enable=false;  //in case of mismatch no DFLT2 bursts will take place
  }
  if (!COMPARE(burstseq_incr_enable)){
    if (master_params.burstseq_incr_enable) {
      CRITICAL(burstseq_incr_enable);
      MASTERMUSTNOT("use the INCR burst sequence");
    }
    resolved_params.burstseq_incr_enable=false;  //in case of mismatch no INCR bursts will take place
  }
  if (!COMPARE(burstseq_strm_enable)){
    if (master_params.burstseq_strm_enable) {
      CRITICAL(burstseq_strm_enable);
      MASTERMUSTNOT("use the STRM burst sequence");
    }
    resolved_params.burstseq_strm_enable=false;  //in case of mismatch no STRM bursts will take place
  }
  if (!COMPARE(burstseq_unkn_enable)){
    if (master_params.burstseq_unkn_enable) {
      CRITICAL(burstseq_unkn_enable);
      MASTERMUSTNOT("use the UNKN burst sequence");
    }
    resolved_params.burstseq_unkn_enable=false;  //in case of mismatch no UKNK bursts will take place
  }
  if (!COMPARE(burstseq_wrap_enable)){
    if (master_params.burstseq_wrap_enable) {
      CRITICAL(burstseq_wrap_enable);
      MASTERMUSTNOT("use the WRAP burst sequence");
    }
    resolved_params.burstseq_wrap_enable=false;  //in case of mismatch no WRAP bursts will take place
  }
  if (!COMPARE(burstseq_xor_enable)){
    if (master_params.burstseq_xor_enable) {
      CRITICAL(burstseq_xor_enable);
      MASTERMUSTNOT("use the XOR burst sequence");
    }
    resolved_params.burstseq_xor_enable=false;  //in case of mismatch no XOR bursts will take place
  }
  if (!COMPARE(burstseq_blck_enable)){
    if (master_params.burstseq_blck_enable) {
      CRITICAL(burstseq_blck_enable);
      MASTERMUSTNOT("use the BLCK burst sequence");
    }
    resolved_params.burstseq_blck_enable=false;  //in case of mismatch no BLCK bursts will take place
  }
  if (!COMPARE(endian)){
    NONCRITICAL(endian);
    std::cerr<<"        Just be careful there..."<<std::endl;
    resolved_params.endian=master_params.endian;  //we have to decide
  }
  if (!COMPARE(force_aligned)){
    if (slave_params.force_aligned){
      CRITICAL(force_aligned);
      MASTERHASTO("keep byte enable patterns aligned");
    }
    resolved_params.force_aligned=true;  //in case of mismatch all byte enables must be aligned
  }
  if (!COMPARE(mthreadbusy_exact)){
    if(master_params.mthreadbusy | slave_params.mthreadbusy) { //if any module is using mthreadbusy the exact values must match
      FATAL(mthreadbusy_exact);
    }
    else
      resolved_params.mthreadbusy_exact=false; //if no module is using mthreadbusy the exact parameter may be false as well.
  }
  if (!COMPARE(rdlwrc_enable)){
    if(master_params.rdlwrc_enable){
      CRITICAL(rdlwrc_enable);
      MASTERMUSTNOT("use a ReadLinked or a WriteCondintional command");
    }
    resolved_params.rdlwrc_enable=false; //in case of mismatch no RDLWRC commands may take place
  }
  if (!COMPARE(read_enable)){
    if(master_params.read_enable){
      CRITICAL(read_enable);
      MASTERMUSTNOT("use a read command");
    }
    resolved_params.read_enable=false; //in case of mismatch no READ transfers are allowed
  }
  if (!COMPARE(readex_enable)){
    if(master_params.readex_enable){
      CRITICAL(readex_enable);
      MASTERMUSTNOT("use a readEx command");
    }
    resolved_params.readex_enable=false; //in case if mismatch no READEX transfers are allowed
  }
  if (!COMPARE(sdatathreadbusy_exact)){
    if(master_params.sdatathreadbusy | slave_params.sdatathreadbusy){
      FATAL(sdatathreadbusy_exact);
    }
    else
      resolved_params.sdatathreadbusy_exact=false;
  }
  if (!COMPARE(sthreadbusy_exact)){
    if(master_params.sthreadbusy | slave_params.sthreadbusy){
      FATAL(sthreadbusy_exact);
    }
    else
      resolved_params.sthreadbusy_exact=false;
  }
  if (!COMPARE(write_enable)){
    if(master_params.write_enable){
      CRITICAL(write_enable);
      MASTERMUSTNOT("use a write command");
    }
    resolved_params.write_enable=false;
  }
  if (!COMPARE(writenonpost_enable)){
    if(master_params.writenonpost_enable){
      CRITICAL(writenonpost_enable);
      MASTERMUSTNOT("use a writeNonPost command");
    }
    resolved_params.writenonpost_enable=false;
  }
  if (!COMPARE(datahandshake)){
    FATAL(datahandshake);
  }
  if (!COMPARE(reqdata_together)){
    FATAL(reqdata_together);
  }
  if (!COMPARE(writeresp_enable)){
    FATAL(writeresp_enable);
  }
  if (!COMPARE(addr)){
    if (master_params.addr){
      CRITICAL(addr);
      SLAVEWILL("ignore the address field inside the requests from the master");
    }
    else {
      NONCRITICAL(addr);
      MASTERWILL("not set any address, so that all transfer will go to address 0");
    }
    resolved_params.addr=false;
  }
  if(!COMPARE(addr_wdth) && resolved_params.addr){
    if (master_params.addr_wdth>slave_params.addr_wdth){
      CRITICAL(addr_wdth);
      SLAVEWILL("ignore the top most "<< (master_params.addr_wdth-slave_params.addr_wdth) << "bits of the address field");
      resolved_params.addr_wdth=slave_params.addr_wdth;
    }
    else {
      NONCRITICAL(addr_wdth);
      MASTERWILL("not set the top most "<< (slave_params.addr_wdth-master_params.addr_wdth) << "bits of the address field");
      resolved_params.addr_wdth=master_params.addr_wdth;
    }
  }
  if (!COMPARE(addrspace)){
    if (master_params.addrspace){
      CRITICAL(addrspace);
      SLAVEWILL("ignore the address space field inside the requests from the master");
    }
    else {
      NONCRITICAL(addrspace);
      MASTERWILL("not set any address space field, so that all transfer will go to address space 0");
    }
    resolved_params.addrspace=false;
  }
  if(!COMPARE(addrspace_wdth) && resolved_params.addrspace){
    if (master_params.addrspace_wdth>slave_params.addrspace_wdth){
      CRITICAL(addrspace_wdth);
      SLAVEWILL("ignore the top most "<< (master_params.addrspace_wdth-slave_params.addrspace_wdth) << "bits of the address space field");
      resolved_params.addrspace_wdth=slave_params.addrspace_wdth;
    }
    else {
      NONCRITICAL(addrspace_wdth);
      MASTERWILL("not set the top most "<< (slave_params.addrspace_wdth-master_params.addrspace_wdth) << "bits of the address space field");
      resolved_params.addrspace_wdth=master_params.addrspace_wdth;
    }
  }
  if (!COMPARE(atomiclength)){
    if (master_params.atomiclength){
      CRITICAL(atomiclength);
      SLAVEWILL("ignore the atomic length field inside the requests from the master");
    }
    else {
      NONCRITICAL(atomiclength);
      MASTERWILL("not set atomic length, so that all transfer will have an atomic length of 1");
    }
    resolved_params.atomiclength=false;
  }
  if(!COMPARE(atomiclength_wdth) && resolved_params.atomiclength){
    if (master_params.atomiclength_wdth>slave_params.atomiclength_wdth){
      CRITICAL(atomiclength_wdth);
      SLAVEWILL("ignore the top most "<< (master_params.atomiclength_wdth-slave_params.atomiclength_wdth) << "bits of the atomic length field");
      resolved_params.atomiclength_wdth=slave_params.atomiclength_wdth;
    }
    else {
      //MASTERWILL("not set the top most "<< (slave_params.atomiclength_wdth-master_params.atomiclength_wdth) << "bits of the atomic length field");
      resolved_params.atomiclength_wdth=master_params.atomiclength_wdth;
    }
  }
  if (!COMPARE(burstlength)){
    if (master_params.burstlength){
      CRITICAL(burstlength);
      SLAVEWILL("ignore the burst length field inside the requests from the master");
    }
    else {
      //MASTERWILL("not set burst length, so that all transfer will have a burst length of 1");
    }
    resolved_params.burstlength=false;
  }
  if(!COMPARE(burstlength_wdth) && resolved_params.burstlength){
    if (master_params.burstlength_wdth>slave_params.burstlength_wdth){
      CRITICAL(burstlength_wdth);
      SLAVEWILL("ignore the top most "<< (master_params.burstlength_wdth-slave_params.burstlength_wdth) << "bits of the burst length field");
      resolved_params.burstlength_wdth=slave_params.burstlength_wdth;
    }
    else {
      //MASTERWILL("not set the top most "<< (slave_params.burstlength_wdth-master_params.burstlength_wdth) << "bits of the burst length field");
      resolved_params.burstlength_wdth=master_params.burstlength_wdth;
    }
  }
  if (!COMPARE(blockheight)){
    if (master_params.blockheight){
      CRITICAL(blockheight);
      SLAVEWILL("ignore the block height field inside the requests from the master");
    }
    resolved_params.blockheight=false;
  }
  if(!COMPARE(blockheight_wdth) && resolved_params.blockheight){
    if (master_params.blockheight_wdth>slave_params.blockheight_wdth){
      CRITICAL(blockheight_wdth);
      SLAVEWILL("ignore the top most "<< (master_params.blockheight_wdth-slave_params.blockheight_wdth) << "bits of the block height field");
      resolved_params.blockheight_wdth=slave_params.blockheight_wdth;
    }
    else {
      resolved_params.blockheight_wdth=master_params.blockheight_wdth;
    }
  }
  if (!COMPARE(blockstride)){
    if (master_params.blockstride){
      CRITICAL(blockstride);
      SLAVEWILL("ignore the block stride field inside the requests from the master");
    }
    resolved_params.blockstride=false;
  }
  if(!COMPARE(blockstride_wdth) && resolved_params.blockstride){
    if (master_params.blockstride_wdth>slave_params.blockstride_wdth){
      CRITICAL(blockstride_wdth);
      SLAVEWILL("ignore the top most "<< (master_params.blockstride_wdth-slave_params.blockstride_wdth) << "bits of the block stride field");
      resolved_params.blockstride_wdth=slave_params.blockstride_wdth;
    }
    else {
      resolved_params.blockstride_wdth=master_params.blockstride_wdth;
    }
  }
  if (!COMPARE(burstprecise)){
    if (master_params.burstprecise){
      CRITICAL(burstprecise);
      SLAVEWILL("ignore the burst precise field inside the requests from the master");
    }
    else {
      //MASTERWILL("not set burst precise, so that all transfer will be concidered precise");
    }
    resolved_params.burstprecise=true;
  }
  if (!COMPARE(burstseq)){
    if (master_params.burstseq){
      CRITICAL(burstseq);
      SLAVEWILL("ignore the burst seq field inside the requests from the master (and concider them INCR)");
      resolved_params.burstseq=false;
    }
    else {
      //MASTERWILL("not set burst burst seq, so that all transfer will be concidered INCR bursts");
      resolved_params.burstseq=false;
    }
  }
  if (!COMPARE(burstsinglereq)){
    if (master_params.burstsinglereq){
      CRITICAL(burstsinglereq);
      SLAVEWILL("ignore the burst single req field inside the requests from the master (and concider it being 0)");
      resolved_params.burstsinglereq=false;
    }
    else {
      //MASTERWILL("not set burst single req, so that it will be concidered 0");
      resolved_params.burstsinglereq=false;
    }
  }
  if (!COMPARE(byteen)){
    if (master_params.byteen){
      CRITICAL(byteen);
      SLAVEWILL("ignore the byteen field inside the requests from the master (and concider it being all 1s)");
      resolved_params.byteen=false;
    }
    else {
      //MASTERWILL("not set byteen req, so that it will be concidered all 1s");
      resolved_params.byteen=false;
    }
  }
  if (!COMPARE(cmdaccept)){
    if (master_params.cmdaccept){
      //SLAVEWILL("not set the SCmdAccept signal.");
      resolved_params.cmdaccept=false;
    }
    else {
      FATAL(cmdaccept);
      MASTERWILL("ignore the SCmdAccept signal and concider it being 1");
      //resolved_params.cmdaccept=false;
    }
  }
  if (!COMPARE(connid)){
    if (master_params.connid){
      CRITICAL(connid);
      SLAVEWILL("ignore the conn id field inside the requests from the master");
    }
    else {
      //MASTERWILL("not set conn id, so that all transfer will be concidered using conn id 0");
    }
    resolved_params.connid=false;
  }
  if(!COMPARE(connid_wdth) && resolved_params.connid){
    if (master_params.connid_wdth>slave_params.connid_wdth){
      CRITICAL(connid_wdth);
      SLAVEWILL("ignore the top most "<< (master_params.connid_wdth-slave_params.connid_wdth) << "bits of the conn id field");
      resolved_params.connid_wdth=slave_params.connid_wdth;
    }
    else {
      //MASTERWILL("not set the top most "<< (slave_params.connid_wdth-master_params.connid_wdth) << "bits of the conn id field");
      resolved_params.connid_wdth=master_params.connid_wdth;
    }
  }
  if (resolved_params.datahandshake){
    if (!COMPARE(dataaccept)){
      if(master_params.dataaccept){
        //SLAVEWILL("not set dataaccept signal and so it's concidered being tied to 1");
      }
      else {
        FATAL(dataaccept);
        MASTERWILL("ignore the dataaccept signal and concider it being tied to 1");
      }
      resolved_params.dataaccept=false;
    }
    if (!COMPARE(datalast)){
      if(master_params.datalast){
        //SLAVEWILL("ignore datalast signal and count for itself.");
      }
      else {
        FATAL(datalast);
        MASTERWILL("will not set a datalast signal so slave has to count for itself");
      }
      resolved_params.datalast=false;
    }
    if (!COMPARE(datarowlast)){
      if(master_params.datarowlast){
      }
      else {
        FATAL(datarowlast);
        MASTERWILL("will not set a datarowlast signal so slave has to count for itself");
      }
      resolved_params.datarowlast=false;
    }
  }
  if (!COMPARE(data_wdth)){
    int mst_msb=0;
    int sl_msb=0;
    bool mst_moreThanOne=false;
    bool sl_moreThanOne=false;
    int mask=0x1;
    for (int i=1;i<32;i++){
      if ( (master_params.data_wdth)&mask ) {
        if (mst_msb) mst_moreThanOne=true;
        mst_msb=i;
      }
      if ( (slave_params.data_wdth)&mask ) {
        if (sl_msb) sl_moreThanOne=true;
        sl_msb=i;
      }
      mask=mask<<1;
    }
    if (mst_moreThanOne) mst_msb++;
    if (sl_moreThanOne) sl_msb++;
    if (mst_msb != sl_msb) {FATAL(data_wdth);}
    else {
      if (master_params.data_wdth>slave_params.data_wdth)
        resolved_params.data_wdth=master_params.data_wdth;
      else
        resolved_params.data_wdth=slave_params.data_wdth;
    }
  }
  if (!COMPARE(mdata) && resolved_params.write_enable) {
    if (master_params.mdata){
      CRITICAL(mdata);
      SLAVEWILL("ignore the master data");
    }
    else {
      //MASTERWILL("not provide write data when doing a write (whatever sense this will make...)");
    }
    resolved_params.mdata=false;
  }
  if (!COMPARE(mdatabyteen) && resolved_params.mdata){
    if (master_params.mdatabyteen){
      CRITICAL(mdatabyteen);
      SLAVEWILL("ignore the master data byteen");
    }
    else {
      //MASTERWILL("not provide write data byteen when doing a write");
    }
    resolved_params.mdatabyteen=false;
  }
  if (!COMPARE(mdatainfo)){
    NONCRITICAL(mdatainfo);
    if (master_params.mdatainfo){
      SLAVEWILL("ignore the master data info");
    }
    else {
      MASTERWILL("not provide write data info when doing a write");
    }
    resolved_params.mdatainfo=false;
  }
  if (!COMPARE(mdatainfo_wdth) && resolved_params.mdatainfo) {
    NONCRITICAL(mdatainfo_wdth);
    if (master_params.mdatainfo_wdth>slave_params.mdatainfo_wdth){
      SLAVEWILL("ignore the top most "<< (master_params.mdatainfo_wdth-slave_params.mdatainfo_wdth) << "bits of the mdata info field");
      resolved_params.mdatainfo_wdth=slave_params.mdatainfo_wdth;
    }
    else {
      MASTERWILL("not set the top most "<< (slave_params.mdatainfo_wdth-master_params.mdatainfo_wdth) << "bits of the mdata info field");
      resolved_params.mdatainfo_wdth=master_params.mdatainfo_wdth;
    }
  }
  if (!COMPARE(mdatainfobyte_wdth) && resolved_params.mdatainfo){
    NONCRITICAL(mdatainfobyte_wdth);
    resolved_params.mdatainfobyte_wdth=(master_params.mdatainfobyte_wdth>slave_params.mdatainfobyte_wdth)? slave_params.mdatainfobyte_wdth : master_params.mdatainfobyte_wdth;
  }
  if (!COMPARE(sdatathreadbusy)){
    NONCRITICAL(sdatathreadbusy);
    if (master_params.sdatathreadbusy) {
      SLAVEWILL("ignore sdatathreadbusy and concider it being 0");
    }
    else {
      MASTERWILL("not set sdatathreadbusy.");
    }
    resolved_params.sdatathreadbusy=false;
  }
  if(!COMPARE(mthreadbusy)){
    NONCRITICAL(mthreadbusy);
    if (master_params.mthreadbusy) {
      SLAVEWILL("ignore mthreadbusy and concider it being 0");
    }
    else {
      MASTERWILL("not set mthreadbusy.");
    }
    resolved_params.mthreadbusy=false;
  }
  if(!COMPARE(reqinfo)){
    NONCRITICAL(reqinfo);
    if (master_params.reqinfo) {
      SLAVEWILL("ignore reqinfo");
    }
    else {
      MASTERWILL("not set reqinfo.");
    }
    resolved_params.reqinfo=false;
  }
  if (!COMPARE(reqinfo_wdth) && resolved_params.reqinfo){
    NONCRITICAL(reqinfo_wdth);
    if (master_params.reqinfo_wdth>slave_params.reqinfo_wdth){
      SLAVEWILL("ignore the top most "<< (master_params.reqinfo_wdth-slave_params.reqinfo_wdth) << "bits of the req info field");
      resolved_params.reqinfo_wdth=slave_params.reqinfo_wdth;
    }
    else {
      MASTERWILL("not set the top most "<< (slave_params.reqinfo_wdth-master_params.reqinfo_wdth) << "bits of the req info field");
      resolved_params.reqinfo_wdth=master_params.reqinfo_wdth;
    }
  }
  if (!COMPARE(reqlast)){
    if (master_params.reqlast) {
      //SLAVEWILL("ignore reqlast and count for itself");
    }
    else {
      FATAL(reqlast);
      MASTERWILL("not set reqlast and so slave must count for itself.");
    }
    resolved_params.reqlast=false;
  }
  if (!COMPARE(reqrowlast)){
    if (master_params.reqrowlast) {
    }
    else {
      FATAL(reqrowlast);
      MASTERWILL("not set reqrowlast and so slave must count for itself.");
    }
    resolved_params.reqrowlast=false;
  }
  if (!COMPARE(resp) && (resolved_params.read_enable | resolved_params.writeresp_enable)){
    FATAL(resp);
    /*
    if (master_params.resp) {
      SLAVEWILL("not provide a resp during read transfers or write responses(god knows how that will work...)");
    }
    else {
      MASTERWILL("ignore the resp during a read transfer.");
    }
    resolved_params.resp=false;
    */
  }
  if (!COMPARE(respaccept) && resolved_params.resp){
    if (master_params.respaccept) {
      FATAL(respaccept);
      SLAVEWILL("ignore MRespAccept signal during read transfers");
    }
    else {
      //MASTERWILL("not provide MRespAccept signal during read transfers.");
    }
    resolved_params.respaccept=false;
  }
  if (!COMPARE(respinfo) && resolved_params.resp){
    NONCRITICAL(respinfo);
    if (master_params.respinfo) {
      SLAVEWILL("not provide respinfo during response phase");
    }
    else {
      MASTERWILL("ignore respinfo during response phase.");
    }
    resolved_params.respinfo=false;
  }
  if (!COMPARE(respinfo_wdth) && resolved_params.respinfo){
    NONCRITICAL(respinfo_wdth);
    if (master_params.respinfo_wdth>slave_params.respinfo_wdth){
      SLAVEWILL("not set the top most "<< (master_params.respinfo_wdth-slave_params.respinfo_wdth) << "bits of the resp info field");
      resolved_params.respinfo_wdth=slave_params.respinfo_wdth;
    }
    else {
      MASTERWILL("ignore the top most "<< (slave_params.respinfo_wdth-master_params.respinfo_wdth) << "bits of the resp info field");
      resolved_params.respinfo_wdth=master_params.respinfo_wdth;
    }
  }
  if (!COMPARE(resplast) && resolved_params.resp){
    if (master_params.resplast) {
      FATAL(resplast);
      SLAVEWILL("not provide resplast during response phase, so master has to count for itself");
    }
    else {
      //MASTERWILL("ignore resplast during response phase and count for itself.");
    }
    resolved_params.resplast=false;
  }
  if (!COMPARE(resprowlast) && resolved_params.resp){
    if (master_params.resprowlast) {
      FATAL(resprowlast);
      SLAVEWILL("not provide resprowlast during response phase, so master has to count for itself");
    }
    resolved_params.resprowlast=false;
  }
  if (!COMPARE(sdata) && resolved_params.resp) {
    if (master_params.sdata){
      //SLAVEWILL("not provide data when giving a response");
    }
    else {
      CRITICAL(sdata);
      MASTERWILL("ignore the slave data");
    }
    resolved_params.sdata=false;
  }
  if (!COMPARE(sdatainfo) && resolved_params.sdata){
    NONCRITICAL(sdatainfo);
    if (master_params.sdatainfo){
      SLAVEWILL("not provide sdata info when giving a response");
    }
    else {
      MASTERWILL("ignore the sdata info");
    }
    resolved_params.sdatainfo=false;
  }
  if (!COMPARE(sdatainfo_wdth) && resolved_params.sdatainfo) {
    NONCRITICAL(sdatainfo_wdth);
    if (master_params.sdatainfo_wdth>slave_params.sdatainfo_wdth){
      SLAVEWILL("not set the top most "<< (master_params.sdatainfo_wdth-slave_params.sdatainfo_wdth) << "bits of the sdata info field");
      resolved_params.sdatainfo_wdth=slave_params.sdatainfo_wdth;
    }
    else {
      MASTERWILL("ignore the top most "<< (slave_params.sdatainfo_wdth-master_params.sdatainfo_wdth) << "bits of the sdata info field");
      resolved_params.sdatainfo_wdth=master_params.sdatainfo_wdth;
    }
  }
  if (!COMPARE(sdatainfobyte_wdth) && resolved_params.sdatainfo){
    NONCRITICAL(sdatainfobyte_wdth);
    resolved_params.sdatainfobyte_wdth=(master_params.sdatainfobyte_wdth>slave_params.sdatainfobyte_wdth)? slave_params.sdatainfobyte_wdth : master_params.sdatainfobyte_wdth;
  }
  if(!COMPARE(sthreadbusy)){
    NONCRITICAL(sthreadbusy);
    if (master_params.sthreadbusy) {
      SLAVEWILL("not set sthreadbusy");
    }
    else {
      MASTERWILL("ignore sthreadbusy and concider it being 0");
    }
    resolved_params.sthreadbusy=false;
  }
  if (!COMPARE(threads)){
    if (master_params.threads>slave_params.threads){
      CRITICAL(threads);
      std::cerr<<"         The master supports more threads than the slave, so the connection will support only the number the slave supports."<<std::endl;
      resolved_params.threads=slave_params.threads;
    }
    else{
      //std::cerr<<"         The slave supports more threads than the master, so the channel will support only the number the master supports."<<std::endl;
      resolved_params.threads=master_params.threads;
    }
  }
  if (!COMPARE(tags)){
    if (master_params.tags>slave_params.tags){
      CRITICAL(tags);
      std::cerr<<"         The master supports more tags than the slave, so the connection will support only the number the slave supports."<<std::endl;
      resolved_params.tags=slave_params.tags;
    }
    else{
      //std::cerr<<"         The slave supports more tags than the master, so the channel will support only the number the master supports."<<std::endl;
      resolved_params.tags=master_params.tags;
    }
  }
  if (!COMPARE(taginorder)){
    CRITICAL(taginorder);
    if(slave_params.taginorder){
      MASTERHASTO("keep tags in order");
    }
    else{
      std::cerr<<"         The Slave has to (under all circumstances) keep tags in order, because the master requires that."<<std::endl;
    }
    resolved_params.taginorder=true;
  }
  if(!COMPARE(interrupt)){
    if(master_params.interrupt){
      //SLAVEWILL("not drive the interrupt signal");
    }
    else{
      CRITICAL(interrupt);
      MASTERWILL("ignore the interrupt signal");
    }
    resolved_params.interrupt=false;
  }
  if(!COMPARE(merror)){
    if(master_params.merror){
      CRITICAL(merror);
      SLAVEWILL("ignore the merror signal");
    }
    else{
      //MASTERWILL("not drive the merror signal");
    }
    resolved_params.merror=false;
  }
  if (!COMPARE(mflag)){
    NONCRITICAL(mflag);
    if(master_params.mflag){
      SLAVEWILL("ignore the mflag signal");
    }
    else{
      MASTERWILL("not drive the mflag signal");
    }
    resolved_params.mflag=false;
  }
  if(!COMPARE(mflag_wdth) && resolved_params.mflag){
    NONCRITICAL(mflag_wdth);
    if (master_params.mflag_wdth>slave_params.mflag_wdth){
      SLAVEWILL("ignore the top most "<< (master_params.mflag_wdth-slave_params.mflag_wdth) << "bits of the mflag field");
      resolved_params.mflag_wdth=slave_params.mflag_wdth;
    }
    else {
      MASTERWILL("not set the top most "<< (slave_params.mflag_wdth-master_params.mflag_wdth) << "bits of the mflag field");
      resolved_params.mflag_wdth=master_params.mflag_wdth;
    }
  }
  if(!COMPARE(serror)){
    if(master_params.serror){
      //SLAVEWILL("not drive the serror signal");
    }
    else{
      CRITICAL(serror);
      MASTERWILL("ignore the serror signal");
    }
    resolved_params.serror=false;
  }
  if (!COMPARE(sflag)){
    NONCRITICAL(sflag);
    if(master_params.sflag){
      SLAVEWILL("not set the sflag signal");
    }
    else{
      MASTERWILL("ignore the sflag signal");
    }
    resolved_params.sflag=false;
  }
  if(!COMPARE(sflag_wdth) && resolved_params.sflag){
    NONCRITICAL(sflag_wdth);
    if (master_params.sflag_wdth>slave_params.sflag_wdth){
      SLAVEWILL("not set the top most "<< (master_params.sflag_wdth-slave_params.sflag_wdth) << "bits of the sflag field");
      resolved_params.sflag_wdth=slave_params.sflag_wdth;
    }
    else {
      MASTERWILL("ignore the top most "<< (slave_params.sflag_wdth-master_params.sflag_wdth) << "bits of the sflag field");
      resolved_params.sflag_wdth=master_params.sflag_wdth;
    }
  }

  if (master_params.mreset && !slave_params.mreset){ //master wants to send reset, slave does not receive
    FATAL(mreset);
  }

  if (!master_params.sreset && slave_params.sreset){ //slave wants to send reset, master does not receive
    FATAL(sreset);
  }


  if (!COMPARE(sdatathreadbusy_pipelined) && resolved_params.sdatathreadbusy_exact){
    FATAL(sdatathreadbusy_exact);
  }
  if (!COMPARE(sthreadbusy_pipelined) && resolved_params.sthreadbusy_exact){
    FATAL(sthreadbusy_exact);
  }
  if (!COMPARE(mthreadbusy_pipelined) && resolved_params.mthreadbusy_exact){
    FATAL(mthreadbusy_exact);
  }

  if(mismatch){
    std::cerr<<"ERROR: Incompatible socket bound (see FATAL Errors above)."<<std::endl;
    abort();
  }
}

#undef COMPARE
#undef FATAL
#undef CRITICAL
#undef NONCRITICAL
#undef MASTERMUSTNOT
#undef MASTERHASTO
#undef SLAVEWILL
#undef MASTERWILL

