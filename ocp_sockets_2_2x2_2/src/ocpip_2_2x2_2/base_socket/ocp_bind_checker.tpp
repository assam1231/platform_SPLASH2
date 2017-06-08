/*
Copyright (c) 2009 GreenSocs Ltd

Permission is hereby granted, free of charge, to any person  
obtaining a copy of this software and associated documentation files  
(the "Software"), to deal in the Software without restriction,  
including without limitation the rights to use, copy, modify, merge,  
publish, distribute, sublicense, and/or sell copies of the Software,  
and to permit persons to whom the Software is furnished to do so,  
subject to the following conditions:

The above copyright notice and this permission notice shall be  
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,  
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF  
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND  
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS  
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN  
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN  
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  
SOFTWARE.
*/


template <typename TRAITS>
template<typename SOCKET_CALLBACK_BASE>
sc_core::sc_interface* OCPIP_VERSION::infr::bind_checker<TRAITS>::get_bottom_of_hierarchy(void* mod)
{
  SOCKET_CALLBACK_BASE* me_as_socket=static_cast<SOCKET_CALLBACK_BASE*>(mod);
  return me_as_socket->get_bottom_of_hierarchy();
}

template <typename TRAITS>
template<typename SOCKET_CALLBACK_BASE>
unsigned int OCPIP_VERSION::infr::bind_checker<TRAITS>::get_num_bindings(void* mod)
{
  SOCKET_CALLBACK_BASE* me_as_socket=static_cast<SOCKET_CALLBACK_BASE*>(mod);
  return me_as_socket->size();
}

template <typename TRAITS>
template<typename SOCKET_CALLBACK_BASE>
 sc_core::sc_interface* OCPIP_VERSION::infr::bind_checker<TRAITS>::get_interface(void* mod, unsigned int index)
{
  SOCKET_CALLBACK_BASE* me_as_socket=static_cast<SOCKET_CALLBACK_BASE*>(mod);
  me_as_socket->SOCKET_CALLBACK_BASE::end_of_elaboration();
  return me_as_socket->operator[](index);
}

template <typename TRAITS>
sc_core::sc_interface* OCPIP_VERSION::infr::bind_checker<TRAITS>::get_bottom_of_hierarchy()
{
  return get_bottom_of_hierarchy_ptr(m_socket);
}


template <typename TRAITS>
unsigned int OCPIP_VERSION::infr::bind_checker<TRAITS>::get_num_bindings()
{
  return get_num_bindinocp_base_ptr(m_socket);
}


template <typename TRAITS>
OCPIP_VERSION::infr::bindability_base<TRAITS>* OCPIP_VERSION::infr::bind_checker<TRAITS>::
 get_other_side(unsigned int index, unsigned int& other_index)
{
  sc_core::sc_interface* other=get_interface_ptr(m_socket, index);
  if (other){
    bindability_base<traits_type>* retVal=NULL;
    OCPIP_VERSION::infr::ocp_base_callback_binder_base* binder=dynamic_cast<OCPIP_VERSION::infr::ocp_base_callback_binder_base*>(other);
    if (binder) {
      bool tmp_bool=static_cast<bindability_base<traits_type>*>(binder->get_owner())->get_t_piece_end(retVal, other_index);
      if (tmp_bool){ 
        return retVal;
      }
      other_index=binder->get_index();
      return static_cast<bindability_base<traits_type>*>(binder->get_owner());
    }
    return retVal;
  }
  else
    assert(0);
  return NULL;
}


template <typename TRAITS>
template<typename SOCKET_CALLBACK_BASE>
OCPIP_VERSION::infr::bind_checker<TRAITS>::bind_checker(const char* name, SOCKET_CALLBACK_BASE* socket)
  : m_name(name)
  , bind_check_state(ocp_base_bind_check_not_allowed)
  , m_socket(static_cast<void*>(socket))
  , m_bus_width(socket->get_bus_width())
{
  m_configs.resize(1);
  cb_done.resize(1);
  get_interface_ptr=&get_interface<SOCKET_CALLBACK_BASE>;
  get_num_bindinocp_base_ptr=&get_num_bindings<SOCKET_CALLBACK_BASE>;
  get_bottom_of_hierarchy_ptr=&get_bottom_of_hierarchy<SOCKET_CALLBACK_BASE>;
}

template <typename TRAITS>
template<typename SOCKET_CALLBACK_BASE>
OCPIP_VERSION::infr::bind_checker<TRAITS>::bind_checker(const char* name, SOCKET_CALLBACK_BASE* socket, unsigned int bus_width)
  : m_name(name)
  , bind_check_state(ocp_base_bind_check_not_allowed)
  , m_socket(static_cast<void*>(socket))
  , m_bus_width(bus_width)
{
  m_configs.resize(1);
  cb_done.resize(1);
  get_interface_ptr=&get_interface<SOCKET_CALLBACK_BASE>;
  get_num_bindinocp_base_ptr=&get_num_bindings<SOCKET_CALLBACK_BASE>;
  get_bottom_of_hierarchy_ptr=&get_bottom_of_hierarchy<SOCKET_CALLBACK_BASE>;
}

template <typename TRAITS>
OCPIP_VERSION::infr::bind_checker<TRAITS>::~bind_checker()
{
}

template <typename TRAITS>
sc_core::sc_object* OCPIP_VERSION::infr::bind_checker<TRAITS>::get_parent(){
  sc_core::sc_object* me_as_object=dynamic_cast<sc_core::sc_object*>(this);
  if (!me_as_object){
    std::cerr<<"Error: Unexpected class hierarchy for a GreenSocket."<<std::endl;
    assert(0);
    exit(1);
  }
  return me_as_object->get_parent();
}

template <typename TRAITS>
const char* OCPIP_VERSION::infr::bind_checker<TRAITS>::get_name()const{
  return m_name;
}

template <typename TRAITS>
bool OCPIP_VERSION::infr::bind_checker<TRAITS>::get_t_piece_end(bindability_base<traits_type>*&, unsigned int&){
  return false;
}

template <typename TRAITS>
void OCPIP_VERSION::infr::bind_checker<TRAITS>::set_config(const config<traits_type>& config){
  for (unsigned int i=0; i<m_configs.size(); i++){
    set_config(config, i);
  }
}

template <typename TRAITS>
void OCPIP_VERSION::infr::bind_checker<TRAITS>::set_config(const config<traits_type>& config, unsigned int index){
  //assert(m_config.invalid); //change to error message
  if (index>=m_configs.size()){
    std::cerr<<"Number of connected sockets on "<<get_name()<<" has not yet been resolved."<<std::endl
             <<" Use set_config(config, index) only during end_of_elaboration or later"<<std::endl;
    exit(1);
  }
  m_configs[index]=config;
  m_configs[index].m_type_string=&get_type_string();
  m_configs[index].invalid=false;
  if (bind_check_state==ocp_base_bind_check_allowed) check_binding(index);
}

template <typename TRAITS>
void OCPIP_VERSION::infr::bind_checker<TRAITS>::check_binding(unsigned int index){
  resize_configs();
  assert(m_configs.size()>index);
  if (m_configs[index].invalid) {
    //std::cerr<<"Socket "<<get_name()<<" has no configuration at end of elaboration."<<std::endl;
    bind_check_state=ocp_base_bind_check_allowed; //we allow rechecks
    return; //we don't test anything. We assume we get a config later. exit(1);
  }
  unsigned int other_index=0;
  bindability_base<traits_type>* other=get_other_side(index,other_index);
  bind_check_state=ocp_base_bind_check_running;
  config<traits_type>& other_conf=(!other)? osci_config::get_cfg() : other->get_config(other_index);
  const char* other_name=(!other)? "osci_socket" : other->get_name();
  if (other_conf.invalid) {
    //std::cerr<<"Error "<<other->get_name()<<" has no configuration at end of elaboration."<<std::endl;
    //exit(1);
    bind_check_state=ocp_base_bind_check_allowed; //we allow rechecks
    return; //we don't test anything. We assume we get a config later.
  }
  
  bool do_cb=!cb_done[index] | other_conf.force_reeval | m_configs[index].force_own_cb; //do a callback if we checking this binding for the first time or because the other side enforces it
  other_conf.force_reeval=false; //reset the enforcement of the callback (because we just used that info)
  m_configs[index].force_own_cb=false;

  {
    config<traits_type> tmp=m_configs[index]; //remember the current config
    m_configs[index].merge_with(get_name(), other_name, other_conf); //merge current config with other side's config
    do_cb|=tmp.diff(m_configs[index]); //if the resolved config differs from the initial one, we'll do a callback
  }
  if (do_cb){ 
    config<traits_type> tmp=m_configs[index]; //remember the resolved config
    bound_to(*other_conf.m_type_string, other, index); //do the callback
    cb_done[index]=true; //remember that we have done a callback
    if (m_configs[index].force_reeval)  //if the changed config now forces us to re-evaluate
      check_binding(index); //we are forced to re-evaluate
    else
    if (tmp.diff(m_configs[index])){ //the callback changed the config
      //so let's check if that change will also effect the resolved config
      m_configs[index].merge_with(get_name(), other_name, other_conf);
      if (tmp.diff(m_configs[index])) //the new resolved config differs from the old
        check_binding(index); //so we redo the bind check
    }
  }
  bind_check_state=ocp_base_bind_check_allowed;
}

template <typename TRAITS>
void OCPIP_VERSION::infr::bind_checker<TRAITS>::resize_configs(){
  if (m_configs.size()==1 && get_num_bindings()>1){
    m_configs.resize(get_num_bindings());
    cb_done.resize(get_num_bindings(), false);
    for (unsigned int i=1; i<m_configs.size(); i++)
      m_configs[i]=m_configs[0]; //assign initial config to all bound sockets
  }
}


template <typename TRAITS>
OCPIP_VERSION::infr::config<TRAITS>& OCPIP_VERSION::infr::bind_checker<TRAITS>::get_config(unsigned int index){
  resize_configs();
  assert(m_configs.size()>index);
  if (bind_check_state!=ocp_base_bind_check_running) check_binding(index); //if our bind check aborted because the other side was not configured we retry now
  return m_configs[index];
}

//called by owner
template <typename TRAITS>
OCPIP_VERSION::infr::config<TRAITS>& OCPIP_VERSION::infr::bind_checker<TRAITS>::get_recent_config(unsigned int index){
  return m_configs[index];
}

template <typename TRAITS>
std::string OCPIP_VERSION::infr::bind_checker<TRAITS>::to_string(){
  std::stringstream s;
  s<<"Configuration of GreenSocket "<<m_name<<std::endl;
  for (int i=0; i<m_configs.size(); i++)
    s<<"Index "<<i<<std::endl<<m_configs[i].to_string()<<std::endl;
  return s.str();
}
