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

#ifndef OCPIP_VERSION
  #error ocp_base_socket_monitor.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif


namespace OCPIP_VERSION{

namespace infr{

DECLARE_EXTENDED_PHASE(B_TRANSPORT_ID_PHASE);
enum {TLM_CALLID_FW=tlm::TLM_ACCEPTED+tlm::TLM_UPDATED+tlm::TLM_COMPLETED,
      TLM_CALLID_BW=TLM_CALLID_FW+1,
      TLM_ACCEPTED_BW=TLM_CALLID_BW+1,
      TLM_UPDATED_BW=TLM_ACCEPTED_BW+1,
      TLM_COMPLETED_BW=TLM_UPDATED_BW+1,
      TLM_B_TRANSPORT_RETURN=TLM_COMPLETED_BW+1};

template <typename TRAITS>
struct transfer_triplet{

  typename TRAITS::tlm_payload_type* txn;
  typename TRAITS::tlm_phase_type    phase;
  sc_core::sc_time                   time;
  unsigned int                       call_or_return;
};

}

#ifdef USE_GREEN_CONFIG
namespace cnf{

template<typename TRAITS>
class gs_param< socket::transfer_triplet<TRAITS> >
: public gs_param_t< socket::transfer_triplet<TRAITS> >
{
  /// Typedef for the value.
  typedef socket::transfer_triplet<TRAITS> val_type;
  
public:
  GS_PARAM_HEAD;
  
  // ///////////////////////
  //  operators
  
  //NO operators!  
  
  /// Overloads gs_param_base::getTypeString
  const std::string getTypeString() const {
#ifdef NO_DEMANGLE
    return string(typeid(val_type).name());
#else
    int tmp;
    return string(abi::__cxa_demangle(typeid(val_type).name(),0,0,&tmp));
#endif  
    
  }
  
  /// Overloads gs_param_base::getType
  const Param_type getType() const {
    return PARTYPE_NOT_AVAILABLE;
  }
  
  /// Overloads gs_param_t<T>::convertValueToString
  std::string convertValueToString(const val_type &val) const {
    if (val.txn==0) return "<TLM2_Transaction type=invalid></TLM2_Transaction>";
    std::stringstream s;
    if (val.phase==socket::B_TRANSPORT_ID_PHASE){
      switch (val.call_or_return){
        case socket::TLM_CALLID_FW:
          s<<"<TLM2_Transaction type=b_transport>"; break;
        case socket::TLM_B_TRANSPORT_RETURN:
          s<<"<TLM2_Transaction type=b_transport_return>"; break;
        default:
          s<<"<TLM2_Transaction type=b_transport_decode_error>";
      }
    }
    else{
      switch (val.call_or_return){
        case tlm::TLM_ACCEPTED:
          s<<"<TLM2_Transaction type=nb_transport_fw_return_ACCEPTED>"; break;
        case tlm::TLM_UPDATED:
          s<<"<TLM2_Transaction type=nb_transport_fw_return_UPDATED>"; break;
        case tlm::TLM_COMPLETED:
          s<<"<TLM2_Transaction type=nb_transport_fw_return_COMPLETED>"; break;
        case socket::TLM_CALLID_FW:
          s<<"<TLM2_Transaction type=nb_transport_fw>"; break;
        case socket::TLM_CALLID_BW:
          s<<"<TLM2_Transaction type=nb_transport_bw>"; break;
        case socket::TLM_ACCEPTED_BW:
          s<<"<TLM2_Transaction type=nb_transport_bw_return_ACCEPTED>"; break;        
        case socket::TLM_UPDATED_BW:
          s<<"<TLM2_Transaction type=nb_transport_bw_return_UPDATED>"; break;
        case socket::TLM_COMPLETED_BW:
          s<<"<TLM2_Transaction type=nb_transport_bw_return_COMPLETED>"; break;
        default:
          s<<"<TLM2_Transaction type=nb_transport_decode_error>";
      }
    }
    s<<"</Command value="<<(val.txn->is_read()? "TLM_READ_COMMAND" : val.txn->is_write()? "TLM_WRITE_COMMAND" : "TLM_IGNORE_COMMAND")<<">"
     <<"</Address value="<<val.txn->get_address()<<">"
     <<"</DataLength value="<<val.txn->get_data_length()<<">"
     //for the time being we do not dump the data
     <<"</StreamingWidth value="<<val.txn->get_streaming_width()<<">"
     <<"</ByteEnableLength value="<<val.txn->get_byte_enable_length()<<">"
     //for the time being we do not dump the byte enables
     <<"</ResponseState value="<<val.txn->get_response_string()<<">"
     <<"</DMIHint value="<<(val.txn->is_dmi_allowed()? "Allowed" : "Not allowed")<<">";
      if (val.phase!=socket::B_TRANSPORT_ID_PHASE) s<<"</TLM2Phase value="<<val.phase<<">";
    s<<"</TimeArgument value="<<val.time<<">"
     <<"<Extensions>";
     
    for (unsigned int i=0; i<tlm::max_num_extensions(); i++){
      tlm::tlm_extension_base* tmp=val.txn->get_extension(i);
      if (tmp){
        ocp_base_extension_base*  ocp_ext=dynamic_cast<ocp_base_extension_base*>(tmp);
        if (ocp_ext) {
          unsigned int potential_guard_id;
          switch(ocp_ext->get_type(potential_guard_id)){
            case ocp_base_data:
              s<<ocp_ext->dump();
              break;
            case ocp_base_guarded_data:
              if (val.txn->get_extension(potential_guard_id) && ocp_ext->is_valid()) //only dump if guard is there
                s<<ocp_ext->dump();
              break;
            case ocp_base_array_guard:;
          }
        }
        else s<<"</extension name=\"unknown\" type=\"non-ocp\" value=\"unknown\">";
      }
    }
    s<<"</Extensions>"<<"</TLM2_Transaction>";

    return s.str();
  }
   
  // Static convertion function called by virtual deserialize and others (e.g. GCnf_API)
  inline static bool static_deserialize(val_type &target_val, const std::string& str) {
    if (str=="") {
      target_val.txn=NULL;
      target_val.phase=tlm::UNINITIALIZED_PHASE;
      target_val.time=sc_core::SC_ZERO_TIME;
      target_val.call_or_return=socket::TLM_CALLID_FW;
      return true;
    }
    std::cout<<"STRANGE!!!"<<std::endl;
    return false;
  }
};
}
#endif

namespace infr{

#ifndef USE_GREEN_CONFIG
template <typename OBSERVER>
struct decode_function_wrapper{
  
  static void decode(void* obs){
    OBSERVER*       callee=static_cast<OBSERVER*>(obs);
    callee->decode();
  }
  
};

struct decode_functor{
  typedef void (*cb)(void*);
  cb m_cb;
  void* mod;
  
  decode_functor(): m_cb(0), mod(NULL){}
  
  template <typename OBSERVER>
  void set(OBSERVER* obs){
    mod=static_cast<void*>(obs);
    m_cb=decode_function_wrapper<OBSERVER>::decode;
  }

  void decode(){
    assert(mod);
    m_cb(mod);
  }
};
#endif


template <unsigned int BUSWIDTH=32,
          typename TRAITS=tlm::tlm_base_protocol_types>
class monitor : public sc_core::sc_module

{
public:
  typedef transfer_triplet<TRAITS> triplet;
private:

  typedef typename TRAITS::tlm_payload_type payload_type;
  typedef typename TRAITS::tlm_phase_type phase_type;

  class monitor_target_socket : public target_socket_base<BUSWIDTH, TRAITS, 1>
  {
  public:
    typedef target_socket_base<BUSWIDTH, TRAITS, 1> base_type;
    using base_type::get_other_side;
    
    monitor_target_socket(const char* name, monitor* mon) 
      : base_type(name), m_mod(mon)
    {
      base_type::register_nb_transport_fw(mon, &monitor::nb_transport_fw);
      base_type::register_b_transport(mon, &monitor::b_transport);
      base_type::register_get_direct_mem_ptr(mon, &monitor::get_dmi);
      base_type::register_transport_dbg(mon, &monitor::dbg_transport);
      config<TRAITS> cfg;
      cfg.treat_unknown_as_optional();
      base_type::set_config(cfg);
    }
    
  private:
    config<TRAITS>&  get_config(unsigned int index){
      return m_mod->t_get_config(index);
    }
    virtual const char* get_name(){
      return m_mod->t_get_name();
    }
    void end_of_elaboration(){
      if (!base_type::base_eoe_done) {base_type::base_type::end_of_elaboration(); base_type::base_eoe_done=true;}
    } //deactivate bind check of underlying socket
    
    bool get_t_piece_end(bindability_base<TRAITS>*& t_piece_end, unsigned int& index){
      t_piece_end=m_mod->t_get_fwd(index);
      return true;
    }
    
    monitor* m_mod;
  };

  class monitor_initiator_socket : public initiator_socket_base<BUSWIDTH, TRAITS, 1>
  {
  public:
    typedef initiator_socket_base<BUSWIDTH, TRAITS, 1> base_type;
    using base_type::get_other_side;
    
    monitor_initiator_socket(const char* name, monitor* mon) 
      : base_type(name), m_mod(mon)
    {
      base_type::register_nb_transport_bw(mon, &monitor::nb_transport_bw);
      base_type::register_invalidate_direct_mem_ptr(mon, &monitor::invalidate_dmi);
      config<TRAITS> cfg;
      cfg.treat_unknown_as_optional();
      base_type::set_config(cfg);
    }
    
  protected:
    config<TRAITS>&  get_config(unsigned int index){
      return m_mod->i_get_config(index);
    }
    virtual const char* get_name(){
      return m_mod->i_get_name();
    }
    void end_of_elaboration(){} //deactivate bind check of underlying socket   

    bool get_t_piece_end(bindability_base<TRAITS>*& t_piece_end, unsigned int& index){
      t_piece_end=m_mod->i_get_fwd(index);
      return true;
    }

    monitor* m_mod;
     
  };

  tlm::tlm_sync_enum nb_transport_fw(payload_type& trans,
                                        phase_type& phase,
                                        sc_core::sc_time& t)
  {
    if (activated){
      triplet tmp={&trans, phase, t, TLM_CALLID_FW};
      m_param=tmp;
#ifndef USE_GREEN_CONFIG
      for (unsigned int i=0; i<m_observers.size(); i++) m_observers[i].decode();
#endif
      tmp.call_or_return=i_socket->nb_transport_fw(trans,phase,t);
      tmp.phase=phase;
      tmp.time=t;
      m_param=tmp;
#ifndef USE_GREEN_CONFIG
      for (unsigned int i=0; i<m_observers.size(); i++) m_observers[i].decode();
#endif
      return (tlm::tlm_sync_enum) tmp.call_or_return;
    }
    return i_socket->nb_transport_fw(trans,phase,t);
  }

    
  void b_transport(payload_type& trans,sc_core::sc_time& t){
    if(activated){
      triplet tmp={&trans, B_TRANSPORT_ID_PHASE, t, TLM_CALLID_FW};
      m_param=tmp;
#ifndef USE_GREEN_CONFIG
      for (unsigned int i=0; i<m_observers.size(); i++) m_observers[i].decode();
#endif
      i_socket->b_transport(trans,t);
      tmp.time=t;
      tmp.call_or_return=TLM_B_TRANSPORT_RETURN;
      m_param=tmp;
#ifndef USE_GREEN_CONFIG
      for (unsigned int i=0; i<m_observers.size(); i++) m_observers[i].decode();
#endif
      return;
    }
    i_socket->b_transport(trans,t);
  }
  
  bool get_dmi(payload_type& trans, tlm::tlm_dmi&  dmi_data){
    return i_socket->get_direct_mem_ptr(trans, dmi_data);
  }

  unsigned int dbg_transport(payload_type& trans){
    return i_socket->transport_dbg(trans);
  }
    
  void invalidate_dmi(sc_dt::uint64 start_range,sc_dt::uint64 end_range){
    t_socket->invalidate_direct_mem_ptr(start_range, end_range);
  }

  tlm::tlm_sync_enum nb_transport_bw(payload_type& trans,
                                        phase_type& phase,
                                        sc_core::sc_time& t)
  {
    if (activated){
      triplet tmp={&trans, phase, t, TLM_CALLID_BW};
      m_param=tmp;
#ifndef USE_GREEN_CONFIG
      for (unsigned int i=0; i<m_observers.size(); i++) m_observers[i].decode();
#endif
      tlm::tlm_sync_enum retVal=t_socket->nb_transport_bw(trans,phase,t);
      tmp.call_or_return=(retVal==tlm::TLM_ACCEPTED)? TLM_ACCEPTED_BW :
                         (retVal==tlm::TLM_UPDATED)? TLM_UPDATED_BW:
                         TLM_COMPLETED_BW;
      tmp.phase=phase;
      tmp.time=t;
      m_param=tmp;
#ifndef USE_GREEN_CONFIG
      for (unsigned int i=0; i<m_observers.size(); i++) m_observers[i].decode();
#endif
      return retVal;
    }
    return t_socket->nb_transport_bw(trans,phase,t);
  }
  
  void t_bound(){
    i_socket->set_config(t_socket.get_recent_config());
  }

  void i_bound(){
    t_socket->set_config(i_socket.get_recent_config());
  }
  config<TRAITS>&  t_get_config(unsigned int index){
    unsigned int tmp;
    if (i_socket.get_other_side(0, tmp))
      return i_socket.get_other_side(0, tmp)->get_config(index);
    return m_osci_conf;
  }
public:
  const char* t_get_name(){
    unsigned int tmp;
    if (i_socket.get_other_side(0, tmp))
      return i_socket.get_other_side(0, tmp)->get_name();
    return "osci_socket";
  }    
protected:
  config<TRAITS>&  i_get_config(unsigned int index){
    unsigned int tmp;
    if (t_socket.get_other_side(0, tmp))
      return t_socket.get_other_side(0, tmp)->get_config(index);
    return m_osci_conf;
  }
public:
  const char* i_get_name(){
    unsigned tmp;
    if (t_socket.get_other_side(0,tmp))
      return t_socket.get_other_side(0,tmp)->get_name();
    return "osci_socket";
  }    
protected:
  bindability_base<TRAITS>* i_get_fwd(unsigned int& index){
    return t_socket.get_other_side(0,index);
  }

  bindability_base<TRAITS>* t_get_fwd(unsigned int& index){
    return i_socket.get_other_side(0, index);
  }
  
  config<TRAITS> m_osci_conf;
  
  #ifdef USE_GREEN_CONFIG
  gs_param<triplet> m_param;
  #else
           triplet  m_param;
  std::string       m_name;
  #endif
  

  
public:
  monitor_target_socket    t_socket;
  monitor_initiator_socket i_socket;

  monitor(sc_core::sc_module_name name, const char* sock1, const char* sock2)
    : sc_core::sc_module(name)
    , m_osci_conf(osci_config::get_cfg())
#ifdef USE_GREEN_CONFIG
    , m_param(std::string(sock1)+std::string("_to_")+std::string(sock2)+std::string("_connection_monitor"), "", true)
#else
    , m_param()
    , m_name(std::string(sock1)+std::string("_to_")+std::string(sock2)+std::string("_connection_monitor"))
#endif
    , t_socket("t_socket", this)
    , i_socket("i_socket", this)
#ifdef USE_GREEN_CONFIG
    , activated("activated", true)
#else
    , activated(true)
#endif
    
  {
  }
  
  bindability_base<TRAITS>* get_connected_initiator(){
    unsigned int tmp;
    return t_socket.get_other_side(0, tmp);
  }

  bindability_base<TRAITS>* get_connected_target(){
    unsigned int tmp;
    return i_socket.get_other_side(0, tmp);
  }

  #ifdef USE_GREEN_CONFIG
  gs_param<triplet>& get_param(){return m_param;}
  gs_param<bool> activated;
  #else
  triplet& get_param(){return m_param;}
  const char* get_name(){return m_name.c_str();}
  std::vector<decode_functor> m_observers;
  
  template<typename OBSERVER>
  void register_observer(OBSERVER* obs){
    m_observers.resize(m_observers.size()+1);
    m_observers.back().set(obs);
  }
  bool activated;
  
  #endif
};

template <unsigned int BUSWIDTH_, typename TRAITS_, typename U, typename V>
static monitor<BUSWIDTH_, TRAITS_>* connect_with_monitor(U& init,V& target)
{
  monitor<BUSWIDTH_, TRAITS_>* tmp=new monitor<BUSWIDTH_, TRAITS_>(sc_core::sc_gen_unique_name("socket_connection_monitor"), init.name(), target.name());
  init(tmp->t_socket);
  tmp->i_socket(target);
  return tmp;
}

}
}

