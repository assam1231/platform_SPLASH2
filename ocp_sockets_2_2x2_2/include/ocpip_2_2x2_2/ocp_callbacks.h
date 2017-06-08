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
//  Description :  This file contains (generates) the callback functors 
//                 used within the ocp kit.
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_callbacks.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

#define OCP_FUNC_WRAPPER(func) \
template <typename MODULE, typename TRAITS> \
class func##_wrapper{ \
public: \
\
  typedef OCP_FUNC_RETURN (MODULE::*cb)(OCP_FUNC_ARGS); \
\
  struct func_container{ \
    cb cb_; \
  }; \
\
  static inline OCP_FUNC_RETURN call_function(void* mod, void* fn, OCP_FUNC_ARGS){ \
    MODULE* tmp_mod=static_cast<MODULE*>(mod); \
    func_container* tmp_cb =static_cast<func_container*> (fn); \
    return (tmp_mod->*(tmp_cb->cb_))(OCP_FUNC_ARGS_WITHOUT_TYPES); \
  } \
\
  static inline void delete_func_container(void* fn){ \
    func_container* tmp_cb =static_cast<func_container*> (fn); \
    if (tmp_cb) delete tmp_cb; \
  }\
\
  static inline void* copy_func_container(void* fn){ \
    func_container* tmp_cb =static_cast<func_container*> (fn); \
    func_container* new_cb = new func_container();\
    new_cb->cb_=tmp_cb->cb_;\
    return new_cb;\
  }\
\
}


#define OCP_FUNC_FUNCTOR(func) \
template <typename TRAITS> \
class func##_functor{ \
public: \
  typedef OCP_FUNC_RETURN (*call_fn)(void*,void*, OCP_FUNC_ARGS); \
  typedef void (*del_fn)(void*); \
  typedef void* (*cpy_fn)(void*); \
\
  func##_functor(): m_fn(0), m_del_fn(0), m_cpy_fn(0), m_mod(0), m_mem_fn(0){} \
  ~func##_functor(){if (m_del_fn) (*m_del_fn)(m_mem_fn);}  \
\
  template <typename MODULE> \
  void set_function(MODULE* mod, OCP_FUNC_RETURN (MODULE::*cb)(OCP_FUNC_ARGS)){ \
    m_fn=&func##_wrapper<MODULE,TRAITS>::call_function; \
    m_del_fn=&func##_wrapper<MODULE,TRAITS>::delete_func_container; \
    m_cpy_fn=&func##_wrapper<MODULE,TRAITS>::copy_func_container; \
    m_del_fn(m_mem_fn); \
    typename func##_wrapper<MODULE,TRAITS>::func_container* tmp= new typename func##_wrapper<MODULE,TRAITS>::func_container(); \
    tmp->cb_=cb; \
    m_mod=static_cast<void*>(mod); \
    m_mem_fn=static_cast<void*>(tmp); \
  } \
\
  OCP_FUNC_RETURN operator()(OCP_FUNC_ARGS){ \
    return m_fn(m_mod,m_mem_fn, OCP_FUNC_ARGS_WITHOUT_TYPES); \
  } \
\
  OCP_FUNC_RETURN operator()(unsigned int index, OCP_FUNC_ARGS){ \
    return m_fn(m_mod,m_mem_fn, OCP_FUNC_ARGS_WITHOUT_TYPES); \
  } \
\
  bool empty(){return (m_mod==0 || m_mem_fn==0 || m_fn==0);}\
\
protected: \
  call_fn m_fn;\
  del_fn m_del_fn; \
  cpy_fn m_cpy_fn;\
  void* m_mod; \
  void* m_mem_fn; \
public: \
  func##_functor& operator=(const func##_functor& other){\
    m_fn=other.m_fn; \
    m_del_fn=other.m_del_fn;\
    if (m_del_fn) m_del_fn(m_mem_fn); \
    m_cpy_fn=other.m_cpy_fn;\
    m_mod=other.m_mod; \
    if (m_cpy_fn) m_mem_fn=m_cpy_fn(other.m_mem_fn);\
    else m_mem_fn=NULL;\
    return *this; \
  }\
}

//void (MODULE::*set_config_cb)(const ocp_parameters&, const std::string&);
//void (MODULE::*timing_cb)(ocp_tl1_master_timing);
//void (MODULE::*timing_cb)(ocp_tl1_slave_timing);
//void (MODULE::*peq_nb_transport)(tlm::tlm_generic_payload&, const tlm::tlm_phase&);

namespace OCPIP_VERSION{
#define OCP_FUNC_RETURN void
#define OCP_FUNC_ARGS const ocp_parameters& params, const std::string& name_
#define OCP_FUNC_ARGS_WITHOUT_TYPES params,name_
OCP_FUNC_WRAPPER(config_callback);
OCP_FUNC_FUNCTOR(config_callback);
#undef OCP_FUNC_RETURN
#undef OCP_FUNC_ARGS
#undef OCP_FUNC_ARGS_WITHOUT_TYPES  


#define OCP_FUNC_RETURN void
#define OCP_FUNC_ARGS const ocp_parameters& params, const std::string& name_, unsigned int index_
#define OCP_FUNC_ARGS_WITHOUT_TYPES params,name_,index_
OCP_FUNC_WRAPPER(config_callback_with_index);
OCP_FUNC_FUNCTOR(config_callback_with_index);
#undef OCP_FUNC_RETURN
#undef OCP_FUNC_ARGS
#undef OCP_FUNC_ARGS_WITHOUT_TYPES  


#define OCP_FUNC_RETURN void
#define OCP_FUNC_ARGS ocp_tl1_master_timing mst_timing
#define OCP_FUNC_ARGS_WITHOUT_TYPES mst_timing
OCP_FUNC_WRAPPER(set_master_timing_callback);
OCP_FUNC_FUNCTOR(set_master_timing_callback);
#undef OCP_FUNC_RETURN
#undef OCP_FUNC_ARGS
#undef OCP_FUNC_ARGS_WITHOUT_TYPES  

#define OCP_FUNC_RETURN void
#define OCP_FUNC_ARGS ocp_tl1_slave_timing slv_timing
#define OCP_FUNC_ARGS_WITHOUT_TYPES slv_timing
OCP_FUNC_WRAPPER(set_slave_timing_callback);
OCP_FUNC_FUNCTOR(set_slave_timing_callback);
#undef OCP_FUNC_RETURN
#undef OCP_FUNC_ARGS
#undef OCP_FUNC_ARGS_WITHOUT_TYPES  

#define OCP_FUNC_RETURN void
#define OCP_FUNC_ARGS typename TRAITS::tlm_payload_type& txn, const typename TRAITS::tlm_phase_type& ph
#define OCP_FUNC_ARGS_WITHOUT_TYPES txn, ph
OCP_FUNC_WRAPPER(nb_peq_callback);
OCP_FUNC_FUNCTOR(nb_peq_callback);
#undef OCP_FUNC_RETURN
#undef OCP_FUNC_ARGS
#undef OCP_FUNC_ARGS_WITHOUT_TYPES  

#define OCP_FUNC_RETURN void
#define OCP_FUNC_ARGS unsigned int i, typename TRAITS::tlm_payload_type& txn, const typename TRAITS::tlm_phase_type& ph
#define OCP_FUNC_ARGS_WITHOUT_TYPES i,txn, ph
OCP_FUNC_WRAPPER(nb_peq_callback_tagged);
OCP_FUNC_FUNCTOR(nb_peq_callback_tagged);
#undef OCP_FUNC_RETURN
#undef OCP_FUNC_ARGS
#undef OCP_FUNC_ARGS_WITHOUT_TYPES  

#define OCP_FUNC_RETURN tlm::tlm_sync_enum
#define OCP_FUNC_ARGS unsigned int i, typename TRAITS::tlm_payload_type& txn, typename TRAITS::tlm_phase_type& ph, sc_core::sc_time& time
#define OCP_FUNC_ARGS_WITHOUT_TYPES i,txn,ph,time
OCP_FUNC_WRAPPER(nb_transport_tagged);
OCP_FUNC_FUNCTOR(nb_transport_tagged);
#undef OCP_FUNC_RETURN
#undef OCP_FUNC_ARGS
#undef OCP_FUNC_ARGS_WITHOUT_TYPES

#define OCP_FUNC_RETURN tlm::tlm_sync_enum
#define OCP_FUNC_ARGS typename TRAITS::tlm_payload_type& txn, typename TRAITS::tlm_phase_type& ph, sc_core::sc_time& time
#define OCP_FUNC_ARGS_WITHOUT_TYPES txn,ph,time
OCP_FUNC_WRAPPER(nb_transport);
OCP_FUNC_FUNCTOR(nb_transport);
#undef OCP_FUNC_RETURN
#undef OCP_FUNC_ARGS
#undef OCP_FUNC_ARGS_WITHOUT_TYPES

template <bool TAGGED>
struct nb_functor_type_selector{};

template <>
struct nb_functor_type_selector<true>
{
  typedef nb_peq_callback_tagged_functor<tlm::tlm_base_protocol_types> peq_functor_type;
  typedef nb_transport_tagged_functor<tlm::tlm_base_protocol_types> nb_functor_type;
};

template <>
struct nb_functor_type_selector<false>
{
  typedef nb_peq_callback_functor<tlm::tlm_base_protocol_types> peq_functor_type;
  typedef nb_transport_functor<tlm::tlm_base_protocol_types> nb_functor_type;
};

template <typename MOD, bool TAGGED>
struct nb_cb_type_selector{};

template <typename MOD>
struct nb_cb_type_selector<MOD,true>
{
  typedef void (MOD::*peq_cb_type)(unsigned int, tlm::tlm_generic_payload&,const  tlm::tlm_phase&);
  typedef tlm::tlm_sync_enum (MOD::*nb_cb_type)(unsigned int, tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&);
};

template <typename MOD>
struct nb_cb_type_selector<MOD,false>
{
  typedef void (MOD::*peq_cb_type)(tlm::tlm_generic_payload&, const  tlm::tlm_phase&);
  typedef tlm::tlm_sync_enum (MOD::*nb_cb_type)(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&);
};

#undef OCP_FUNC_WRAPPER
#undef OCP_FUNC_FUNCTOR

}
