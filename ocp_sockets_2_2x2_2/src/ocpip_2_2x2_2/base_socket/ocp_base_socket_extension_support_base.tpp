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
OCPIP_VERSION::infr::extension_support_base<TRAITS>::extension_support_base()
{
}

template <typename TRAITS>
OCPIP_VERSION::infr::extension_support_base<TRAITS>::~extension_support_base(){
}


template <typename TRAITS>
template <typename T>
bool OCPIP_VERSION::infr::extension_support_base<TRAITS>::get_extension(T*& ptr, payload_type& txn){
  return get_extension<T>(T::_ID, ptr, txn);
}

template <typename TRAITS>
template <typename T>
T* OCPIP_VERSION::infr::extension_support_base<TRAITS>::get_extension(payload_type& txn){
  return get_extension<T>(T::_ID, txn);
}

template <typename TRAITS>
template <typename T>
bool OCPIP_VERSION::infr::extension_support_base<TRAITS>::validate_extension(payload_type& txn, unsigned int index){
  return validate_extension<T>(T::_ID, txn, index);
} 

template <typename TRAITS>
template <typename T>
void OCPIP_VERSION::infr::extension_support_base<TRAITS>::invalidate_extension(payload_type& txn, unsigned int index){
  invalidate_extension<T>(T::_ID, txn, index);
}

template <typename TRAITS>
template <typename T>
bool OCPIP_VERSION::infr::extension_support_base<TRAITS>::get_extension(const OCPIP_VERSION::infr::ocp_base_guarded_data_id&, T*& ptr, payload_type& txn){
  txn.get_extension(ptr);
  if (!ptr){
    ptr=OCPIP_VERSION::infr::create_extension<T>();//new T();
    txn.set_extension(ptr);
    ptr->m_valid=false;
    return false;
  }
  typename T::guard* g_ptr;
  txn.get_extension(g_ptr);
  if (g_ptr && ptr->m_valid) return true;
  return false;
}

template <typename TRAITS>
template <typename T>
bool OCPIP_VERSION::infr::extension_support_base<TRAITS>::get_extension(const OCPIP_VERSION::infr::ocp_base_data_id&, T*& ptr, payload_type& txn){
  txn.get_extension(ptr);
  if (!ptr){
    ptr=OCPIP_VERSION::infr::create_extension<T>();//new T();
    txn.set_extension(ptr);
  }
  return true;
}

template <typename TRAITS>
template <typename T>
T* OCPIP_VERSION::infr::extension_support_base<TRAITS>::get_extension(const OCPIP_VERSION::infr::ocp_base_real_guard_id&, payload_type& txn){
  T* ptr;
  txn.get_extension(ptr);
  if (!ptr) return NULL;
  typename T::guard* g_ptr;
  txn.get_extension(g_ptr);
  if (g_ptr && ptr->m_valid) return ptr; 
  return NULL;
}

template <typename TRAITS>
template <typename T>
T* OCPIP_VERSION::infr::extension_support_base<TRAITS>::get_extension(const OCPIP_VERSION::infr::ocp_base_data_id&, payload_type& txn){
  T* tmp=txn.get_extension<T>();
  if (!tmp) {tmp=OCPIP_VERSION::infr::create_extension<T>(); txn.set_extension(tmp);} //new T(); 
  return tmp;
}

template <typename TRAITS>
template <typename T>
bool OCPIP_VERSION::infr::extension_support_base<TRAITS>::validate_extension(const OCPIP_VERSION::infr::ocp_base_real_guard_id&, payload_type& txn, unsigned int index){
  T* ptr;
  txn.get_extension(ptr);
  if (!ptr){
    ptr=OCPIP_VERSION::infr::create_extension<T>();//new T();
    txn.set_extension(ptr);
  }
  ptr->m_valid=true;
  typename T::guard* g_ptr;
  txn.get_extension(g_ptr);
  if (txn.has_mm()){
    if (!g_ptr) txn.set_auto_extension(T::guard::ID, OCPIP_VERSION::infr::create_extension<typename T::guard>());//->pseudo_cast_to_tlm_extension_base());
    return true;
  }
  txn.set_extension(T::guard::ID, OCPIP_VERSION::infr::create_extension<typename T::guard>());//->pseudo_cast_to_tlm_extension_base());
  return false;  
}

template <typename TRAITS>
template <typename T>
bool OCPIP_VERSION::infr::extension_support_base<TRAITS>::validate_extension(const OCPIP_VERSION::infr::ocp_base_guarded_data_id&, payload_type& txn, unsigned int index){
  T* ptr;
  txn.get_extension(ptr);
  if (!ptr){
    ptr=OCPIP_VERSION::infr::create_extension<T>();//new T();
    txn.set_extension(ptr);
  }
  ptr->m_valid=true;
  typename T::guard* g_ptr;
  txn.get_extension(g_ptr);
  if (txn.has_mm()){
    if (!g_ptr) txn.set_auto_extension(T::guard::ID, OCPIP_VERSION::infr::create_extension<typename T::guard>());//->pseudo_cast_to_tlm_extension_base());
    return true;
  }
  else{
    txn.set_extension(T::guard::ID, OCPIP_VERSION::infr::create_extension<typename T::guard>());//->pseudo_cast_to_tlm_extension_base());
    return false;  
  }
  return false;
}

template <typename TRAITS>
template <typename T>
void OCPIP_VERSION::infr::extension_support_base<TRAITS>::invalidate_extension(const OCPIP_VERSION::infr::ocp_base_real_guard_id&, payload_type& txn, unsigned int index){
  if (!txn.has_mm()) txn.template clear_extension<typename T::guard>();
  else{
    T* ext=txn.template get_extension<T>();
    if (ext) ext->m_valid=false;
  }
}


template <typename TRAITS>
template <typename T>
void OCPIP_VERSION::infr::extension_support_base<TRAITS>::invalidate_extension(const OCPIP_VERSION::infr::ocp_base_guarded_data_id&, payload_type& txn, unsigned int index){
  if (!txn.has_mm()) txn.template clear_extension<typename T::guard>();
  else{
    T* ext=txn.template get_extension<T>();
    if (ext) ext->m_valid=false;
  }
}
