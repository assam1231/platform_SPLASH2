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
  #error ocp_base_socket_observer_base.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{
namespace infr{

class ocp_observer_base{
  typedef transfer_triplet<tlm::tlm_base_protocol_types> triplet;

public:
#ifdef USE_GREEN_CONFIG
  GC_HAS_CALLBACKS();
  boost::shared_ptr<gs::cnf::ParamCallbAdapt_b> cbAdapt;  
#endif

  template <unsigned int BUSWIDTH>
  struct call_converter{
    static bindability_base<tlm::tlm_base_protocol_types>* get_connected_target(void* mon_){
      monitor<BUSWIDTH, tlm::tlm_base_protocol_types>* mon=static_cast<monitor<BUSWIDTH, tlm::tlm_base_protocol_types>*>(mon_);
      return mon->get_connected_target();
    }
    static bindability_base<tlm::tlm_base_protocol_types>* get_connected_initiator(void* mon_){
      monitor<BUSWIDTH, tlm::tlm_base_protocol_types>* mon=static_cast<monitor<BUSWIDTH, tlm::tlm_base_protocol_types>*>(mon_);
      return mon->get_connected_initiator();
    }
    static triplet& get_param(void* mon_){
      monitor<BUSWIDTH, tlm::tlm_base_protocol_types>* mon=static_cast<monitor<BUSWIDTH, tlm::tlm_base_protocol_types>*>(mon_);
      return mon->get_param();
    }
  };

  typedef bindability_base<tlm::tlm_base_protocol_types>* (*get_connected_cb)(void*);
  typedef triplet& (*get_param_cb)(void*);
  
  get_connected_cb m_get_conn_init, m_get_conn_tgt;
  get_param_cb m_get_param;
  void* m_mon;
  std::string m_name;

  template<unsigned int BUSWIDTH>
  ocp_observer_base(monitor<BUSWIDTH, tlm::tlm_base_protocol_types>* mon) 
  #ifdef USE_GREEN_CONFIG
    : m_name(mon->get_param().getName())
  #else
    : m_name(mon->get_name())
  #endif
  {
  #ifdef USE_GREEN_CONFIG
    cbAdapt = GC_REGISTER_PARAM_CALLBACK(&mon->get_param(), ocp_observer_base, decode);
  #else
    mon->register_observer(this);
  #endif
    m_get_conn_init = &call_converter<BUSWIDTH>::get_connected_initiator;
    m_get_conn_tgt  = &call_converter<BUSWIDTH>::get_connected_target;
    m_get_param     = &call_converter<BUSWIDTH>::get_param;
    m_mon=static_cast<void*>(mon);
  }

  const bindability_base<tlm::tlm_base_protocol_types>* get_connected_target()const{
    return m_get_conn_tgt(m_mon);
  }
  const bindability_base<tlm::tlm_base_protocol_types>* get_connected_initiator()const{
    return m_get_conn_init(m_mon);
  }
  
  virtual ~ocp_observer_base(){
  #ifdef USE_GREEN_CONFIG
    GC_UNREGISTER_CALLBACK(cbAdapt);
  #endif
  }
  
  const std::string& get_name() const{return m_name;}
  
  #ifdef USE_GREEN_CONFIG
  void decode(gs::gs_param_base& par){
    if (par.is_destructing()) return;
    gs_param<triplet>* tmp_p=static_cast<gs_param<triplet>*>(&par);
    triplet tmp_i=tmp_p->getValue();
  #else
  void decode(){
    triplet tmp_i=m_get_param(m_mon);
  #endif
    triplet* tmp=&tmp_i;
    if (tmp->phase==B_TRANSPORT_ID_PHASE){
      switch (tmp->call_or_return){
        case TLM_CALLID_FW:
          b_call_callback(*tmp->txn, tmp->time);
        case TLM_B_TRANSPORT_RETURN:
          b_return_callback(*tmp->txn, tmp->time);
        default:
          std::cerr<<"Warning: error decoding "<<m_name<<" during observation."<<std::endl;
      }
    }
    else{
      switch (tmp->call_or_return){
        case tlm::TLM_ACCEPTED:
        case tlm::TLM_UPDATED:
        case tlm::TLM_COMPLETED:
          nb_return_callback(true, *tmp->txn, tmp->phase, tmp->time, (tlm::tlm_sync_enum)tmp->call_or_return);
          break;
        case TLM_CALLID_FW:
          nb_call_callback(true, *tmp->txn, tmp->phase, tmp->time);
          break;
        case TLM_CALLID_BW:
          nb_call_callback(false, *tmp->txn, tmp->phase, tmp->time);
          break;
        case TLM_ACCEPTED_BW:
          nb_return_callback(false, *tmp->txn, tmp->phase, tmp->time, tlm::TLM_ACCEPTED);
          break;
        case TLM_UPDATED_BW:
          nb_return_callback(false, *tmp->txn, tmp->phase, tmp->time, tlm::TLM_UPDATED);
          break;
        case TLM_COMPLETED_BW:
          nb_return_callback(false, *tmp->txn, tmp->phase, tmp->time, tlm::TLM_COMPLETED);
          break;
        default:
          std::cerr<<"Warning: error decoding "<<m_name<<" during observation."<<std::endl;
      }
    }
  }
  
  virtual void nb_call_callback(bool fwNbw, tlm::tlm_generic_payload& txn, const tlm::tlm_phase& phase, const sc_core::sc_time& time)=0;
  virtual void nb_return_callback(bool fwNbw, tlm::tlm_generic_payload& txn, const tlm::tlm_phase& phase, const sc_core::sc_time& time, tlm::tlm_sync_enum retVal)=0;

  virtual void b_call_callback(tlm::tlm_generic_payload& txn, const sc_core::sc_time& time)=0;
  virtual void b_return_callback(tlm::tlm_generic_payload& txn, const sc_core::sc_time& time)=0;

};

}
}

