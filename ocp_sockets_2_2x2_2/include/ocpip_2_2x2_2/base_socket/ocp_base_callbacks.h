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
  #error ocp_base_callbacks.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

#define OCP_BASE_FUNC_WRAPPER(func) \
template <typename MODULE, typename TRAITS> \
class func##_wrapper{ \
public: \
  typedef typename TRAITS::tlm_payload_type payload_type; \
  typedef typename TRAITS::tlm_phase_type   phase_type; \
\
  typedef OCP_BASE_FUNC_RETURN (MODULE::*cb)(OCP_BASE_FUNC_ARGS); \
\
  struct func_container{ \
    cb cb_; \
  }; \
\
  struct func_container_tagged{ \
    cb cb_; \
  }; \
\
  static inline OCP_BASE_FUNC_RETURN call_function(void* mod, void* fn, OCP_BASE_FUNC_ARGS){ \
    MODULE* tmp_mod=static_cast<MODULE*>(mod); \
    func_container* tmp_cb =static_cast<func_container*> (fn); \
    return (tmp_mod->*(tmp_cb->cb_))(OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES); \
  } \
\
  static inline void delete_func_container(void* fn){ \
    func_container* tmp_cb =static_cast<func_container*> (fn); \
    if (tmp_cb) delete tmp_cb; \
  }\
\
}


#define OCP_BASE_FUNC_FUNCTOR(func) \
template <typename TRAITS> \
class func##_functor{ \
public: \
  typedef typename TRAITS::tlm_payload_type payload_type; \
  typedef typename TRAITS::tlm_phase_type   phase_type; \
  typedef OCP_BASE_FUNC_RETURN (*call_fn)(void*,void*, OCP_BASE_FUNC_ARGS); \
  typedef void (*del_fn)(void*); \
\
  func##_functor(): m_fn(0), m_del_fn(0), m_mod(0), m_mem_fn(0){} \
  ~func##_functor(){if (m_del_fn) (*m_del_fn)(m_mem_fn);}  \
\
  template <typename MODULE> \
  void set_function(MODULE* mod, OCP_BASE_FUNC_RETURN (MODULE::*cb)(OCP_BASE_FUNC_ARGS)){ \
    m_fn=&func##_wrapper<MODULE,TRAITS>::call_function; \
    m_del_fn=&func##_wrapper<MODULE,TRAITS>::delete_func_container; \
    m_del_fn(m_mem_fn); \
    typename func##_wrapper<MODULE,TRAITS>::func_container* tmp= new typename func##_wrapper<MODULE,TRAITS>::func_container(); \
    tmp->cb_=cb; \
    m_mod=static_cast<void*>(mod); \
    m_mem_fn=static_cast<void*>(tmp); \
  } \
\
  OCP_BASE_FUNC_RETURN operator()(OCP_BASE_FUNC_ARGS){ \
    return m_fn(m_mod,m_mem_fn, OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES); \
  } \
\
  OCP_BASE_FUNC_RETURN operator()(unsigned int index, OCP_BASE_FUNC_ARGS){ \
    return m_fn(m_mod,m_mem_fn, OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES); \
  } \
\
  bool empty(){return (m_mod==0 || m_mem_fn==0 || m_fn==0);}\
\
protected: \
  call_fn m_fn;\
  del_fn m_del_fn; \
  void* m_mod; \
  void* m_mem_fn; \
private: \
  func##_functor& operator=(const func##_functor&); \
}

namespace OCPIP_VERSION{
namespace infr{
#define OCP_BASE_FUNC_RETURN tlm::tlm_sync_enum
#define OCP_BASE_FUNC_ARGS payload_type& txn, phase_type& ph, sc_core::sc_time& time
#define OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES txn,ph,time
OCP_BASE_FUNC_WRAPPER(nb_transport);
OCP_BASE_FUNC_FUNCTOR(nb_transport);
#undef OCP_BASE_FUNC_RETURN
#undef OCP_BASE_FUNC_ARGS
#undef OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES

#define OCP_BASE_FUNC_RETURN void
#define OCP_BASE_FUNC_ARGS payload_type& txn, sc_core::sc_time& time
#define OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES txn,time
OCP_BASE_FUNC_WRAPPER(b_transport);
OCP_BASE_FUNC_FUNCTOR(b_transport);
#undef OCP_BASE_FUNC_RETURN
#undef OCP_BASE_FUNC_ARGS
#undef OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES

#define OCP_BASE_FUNC_RETURN bool
#define OCP_BASE_FUNC_ARGS payload_type& txn, tlm::tlm_dmi& dmi
#define OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES txn,dmi
OCP_BASE_FUNC_WRAPPER(get_direct_mem_ptr);
OCP_BASE_FUNC_FUNCTOR(get_direct_mem_ptr);
#undef OCP_BASE_FUNC_RETURN
#undef OCP_BASE_FUNC_ARGS
#undef OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES

#define OCP_BASE_FUNC_RETURN void
#define OCP_BASE_FUNC_ARGS sc_dt::uint64 start, sc_dt::uint64 end
#define OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES start,end
OCP_BASE_FUNC_WRAPPER(invalidate_direct_mem_ptr);
OCP_BASE_FUNC_FUNCTOR(invalidate_direct_mem_ptr);
#undef OCP_BASE_FUNC_RETURN
#undef OCP_BASE_FUNC_ARGS
#undef OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES

#define OCP_BASE_FUNC_RETURN unsigned int
#define OCP_BASE_FUNC_ARGS payload_type& txn
#define OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES txn
OCP_BASE_FUNC_WRAPPER(transport_dbg);
OCP_BASE_FUNC_FUNCTOR(transport_dbg);
#undef OCP_BASE_FUNC_RETURN
#undef OCP_BASE_FUNC_ARGS
#undef OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES


#define OCP_BASE_FUNC_RETURN tlm::tlm_sync_enum
#define OCP_BASE_FUNC_ARGS unsigned int i, payload_type& txn, phase_type& ph, sc_core::sc_time& time
#define OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES i,txn,ph,time
OCP_BASE_FUNC_WRAPPER(nb_transport_tagged);
OCP_BASE_FUNC_FUNCTOR(nb_transport_tagged);
#undef OCP_BASE_FUNC_RETURN
#undef OCP_BASE_FUNC_ARGS
#undef OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES

#define OCP_BASE_FUNC_RETURN void
#define OCP_BASE_FUNC_ARGS unsigned int i, payload_type& txn, sc_core::sc_time& time
#define OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES i,txn,time
OCP_BASE_FUNC_WRAPPER(b_transport_tagged);
OCP_BASE_FUNC_FUNCTOR(b_transport_tagged);
#undef OCP_BASE_FUNC_RETURN
#undef OCP_BASE_FUNC_ARGS
#undef OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES

#define OCP_BASE_FUNC_RETURN bool
#define OCP_BASE_FUNC_ARGS unsigned int i, payload_type& txn, tlm::tlm_dmi& dmi
#define OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES i,txn,dmi
OCP_BASE_FUNC_WRAPPER(get_direct_mem_ptr_tagged);
OCP_BASE_FUNC_FUNCTOR(get_direct_mem_ptr_tagged);
#undef OCP_BASE_FUNC_RETURN
#undef OCP_BASE_FUNC_ARGS
#undef OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES

#define OCP_BASE_FUNC_RETURN void
#define OCP_BASE_FUNC_ARGS unsigned int i, sc_dt::uint64 start, sc_dt::uint64 end
#define OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES i,start,end
OCP_BASE_FUNC_WRAPPER(invalidate_direct_mem_ptr_tagged);
OCP_BASE_FUNC_FUNCTOR(invalidate_direct_mem_ptr_tagged);
#undef OCP_BASE_FUNC_RETURN
#undef OCP_BASE_FUNC_ARGS
#undef OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES

#define OCP_BASE_FUNC_RETURN unsigned int
#define OCP_BASE_FUNC_ARGS unsigned int i, payload_type& txn
#define OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES i,txn
OCP_BASE_FUNC_WRAPPER(transport_dbg_tagged);
OCP_BASE_FUNC_FUNCTOR(transport_dbg_tagged);
#undef OCP_BASE_FUNC_RETURN
#undef OCP_BASE_FUNC_ARGS
#undef OCP_BASE_FUNC_AROCP_BASE_WITHOUT_TYPES


#undef OCP_BASE_FUNC_WRAPPER
#undef OCP_BASE_FUNC_FUNCTOR
}
}

