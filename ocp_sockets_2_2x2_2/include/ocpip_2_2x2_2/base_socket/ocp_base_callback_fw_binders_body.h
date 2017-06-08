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

#ifndef OCP_I_KNOW_VERY_WELL_WHAT_I_AM_DOING_WHEN_INCLUDING_THIS_FILE
# error This file may never ever be included without special care!
#endif

public:
//typedefs according to the used TRAITS class
typedef typename TRAITS::tlm_payload_type              transaction_type;
typedef typename TRAITS::tlm_phase_type                phase_type;  
typedef tlm::tlm_sync_enum                            sync_enum_type;

//ctor: an ID is needed to create a callback binder
template <typename T>
ocp_base_callback_binder_fw(int id, T* owner)
  : ocp_base_callback_binder_fw_base<TRAITS,CB_BINDER_BASE>(id, owner)
  , m_nb_fw_functor(NULL)
  , m_b_functor(NULL)
  , m_get_dmi_functor(NULL)
  , m_dbg_functor(NULL)
{
}

//the nb_transport method of the fw interface
sync_enum_type nb_transport_fw(transaction_type& txn,
                            phase_type& p,
                            sc_core::sc_time& t){
  //check if a callback is registered
  if (!m_nb_fw_functor || m_nb_fw_functor->empty()){
    std::cerr<<"No function registered"<<std::endl;
    exit(1);
  }
  else
    return (*m_nb_fw_functor)(
#ifdef OCP_USE_WITH_TAG    
      ocp_base_callback_binder_fw_base<TRAITS,CB_BINDER_BASE>::m_id, 
#endif
      txn, p, t); //do the callback
}

//the b_transport method of the fw interface
void b_transport(transaction_type& trans,sc_core::sc_time& t){
  //check if a callback is registered
  if (!m_b_functor || m_b_functor->empty()){
    std::cerr<<"No function registered"<<std::endl;
    exit(1);
  }
  else
    (*m_b_functor)(
#ifdef OCP_USE_WITH_TAG    
      ocp_base_callback_binder_fw_base<TRAITS,CB_BINDER_BASE>::m_id, 
#endif
      trans,t); //do the callback
}

//the DMI method of the fw interface
bool get_direct_mem_ptr(transaction_type& trans, tlm::tlm_dmi&  dmi_data){
  //check if a callback is registered
  if (!m_get_dmi_functor || m_get_dmi_functor->empty()){
    std::cerr<<"No function registered"<<std::endl; 
    exit(1);
  }
  else
    return (*m_get_dmi_functor)(
#ifdef OCP_USE_WITH_TAG    
      ocp_base_callback_binder_fw_base<TRAITS,CB_BINDER_BASE>::m_id, 
#endif
      trans,dmi_data); //do the callback
}

//the debug method of the fw interface
unsigned int transport_dbg(transaction_type& trans){
  //check if a callback is registered
  if (!m_dbg_functor || m_dbg_functor->empty()){
    std::cerr<<"No function registered"<<std::endl; 
    exit(1);
  }
  else
    return (*m_dbg_functor)(
#ifdef OCP_USE_WITH_TAG    
      ocp_base_callback_binder_fw_base<TRAITS,CB_BINDER_BASE>::m_id, 
#endif
    trans); //do the callback
}


//register callbacks for all fw interface methods at once
void set_callbacks(
  nb_transport_tagged_functor<TRAITS>& nb_fw_functor,
  b_transport_tagged_functor<TRAITS>&  b_functor,
  get_direct_mem_ptr_tagged_functor<TRAITS>& get_dmi_functor,
  transport_dbg_tagged_functor<TRAITS>& dbg_functor
){
#ifdef OCP_USE_WITH_TAG
  m_nb_fw_functor=&nb_fw_functor;
  m_b_functor=&b_functor;
  m_get_dmi_functor=&get_dmi_functor;
  m_dbg_functor=&dbg_functor;
#else
  assert(0);
#endif
}

void set_callbacks(
  nb_transport_functor<TRAITS>& nb_fw_functor,
  b_transport_functor<TRAITS>&  b_functor,
  get_direct_mem_ptr_functor<TRAITS>& get_dmi_functor,
  transport_dbg_functor<TRAITS>& dbg_functor
){
#ifdef OCP_USE_WITH_TAG
  assert(0);
#else
  m_nb_fw_functor=&nb_fw_functor;
  m_b_functor=&b_functor;
  m_get_dmi_functor=&get_dmi_functor;
  m_dbg_functor=&dbg_functor;
#endif
}

