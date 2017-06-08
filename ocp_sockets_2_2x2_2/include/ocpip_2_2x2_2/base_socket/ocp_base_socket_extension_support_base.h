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
  #error ocp_base_socket_extension_support_base.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{
namespace infr{


//This class handles the extension access
// it is used by both the target and the initiator socket
template <typename TRAITS>
class extension_support_base
{
public:

  typedef typename TRAITS::tlm_payload_type  payload_type;
  typedef typename TRAITS::tlm_phase_type  phase_type;

  extension_support_base();
  virtual ~extension_support_base();
  
  //use this function to get an extension from a transaction
  // It can be used for all types of extensions (guard, data, guarded data)
  // The return value shows if the extension was present or not
  // For guards the ptr argument is just used to determine the type
  //  but it shall not be used after the call
  // For data extensions the return value can be ignored, because if
  //  it was not present the call will add the extension to the txn
  //  and return a valid pointer
  // For guarded data extensions, the return value shows if the guard
  //  was set, and the returned pointer always points to a valid
  //  data part of the extension
  //
  //Example guard extension:
  //  my_guard_extension* tmp;  //just used to determine the type
  //  if (socket.get_extension(tmp, txn) guard_was_set_action();
  //  else guard_was_not_set_action();
  //Example data extension:
  //  my_data_extension* tmp;  //just used to determine the type
  //  socket.get_extension(tmp, txn);  //return value ignored
  //  tmp->value=33; //set the data of the extension
  //Example data extension:
  //  my_guarded_data_extension* tmp;  //just used to determine the type
  //  if (socket.get_extension(tmp, txn)){
  //    //extension was already set
  //    tmp->value=42; just change the value
  //  }
  //  else {
  //    //extension was not set, so we set it now
  //    socket.validate_extension<my_guarded_data_extension>(txn);
  //    tmp->value=33; //set the data of the extension
  //  }
  template <typename T>
  static inline bool get_extension(T*& ptr, payload_type& txn);  

  //This function can only be used for data and guard extensions
  //It provides simplified access, where the former call would introduce
  //  avoidable overhead
  //Example guard extension:
  //  if (socket.get_extension<my_guard_extension>(txn)) guard_was_set_action();
  //  else guard_was_not_set_action();
  //Example data extension:
  //  socket.get_extension<my_data_extension>(txn)->value=42; //just set the data
  template <typename T>
  static inline T* get_extension(payload_type& txn);  

  //This function immediately validates a guard or a guard of a guarded data extension
  // it returns true if there was memory management and false if there wasn't
  //  The return value is only of value within b_transport (because within nb_transport there is always memory management)
  //  if there was memory management, the memory manager will clean off the extension when the reference count hits zero
  //  if not, it has to be removed after a b_transport call returned
  //Example guard extension nb_transport:
  //  socket.validate_extension<my_guard_extension>(txn); //set the guard
  //Example guarded data extension nb_transport:
  //  socket.validate_extension<my_guarded_data_extension>(txn); //set the guard of the guarded data
  //Example b_transport:
  //  void b_transport(txn, ...){
  //    bool needs_invalidate=!socket.validate<my_guard_extension>(txn);
  //    out_socket->b_transport(txn, ...);
  //    if (needs_invalidate) socket.invalidate<my_guard_extension>(txn);
  //  }
  template <typename T>
  static inline bool validate_extension(payload_type& txn, unsigned int index=0);  

  //This function invalidates a guard or a guard of a guarded data extension immediately
  //Example guard extension:
  //  socket.validate_extension<my_data_extension>(txn); //set the guard
  //Example guarded data extension:
  //  socket.validate_extension<my_guarded_data_extension>(txn); //set the guard of the guarded data  
  template <typename T>
  static inline void invalidate_extension(payload_type& txn, unsigned int index=0);  

protected:

  template <typename T>
  static inline bool get_extension(const OCPIP_VERSION::infr::ocp_base_guarded_data_id&, T*& ptr, payload_type& txn);
  template <typename T>
  static inline bool get_extension(const OCPIP_VERSION::infr::ocp_base_data_id&, T*& ptr, payload_type& txn);

  template <typename T>
  static inline T* get_extension(const OCPIP_VERSION::infr::ocp_base_real_guard_id&, payload_type& txn);
  template <typename T>
  static inline T* get_extension(const OCPIP_VERSION::infr::ocp_base_data_id&, payload_type& txn);

  template <typename T>
  static inline bool validate_extension(const OCPIP_VERSION::infr::ocp_base_real_guard_id&, payload_type& txn, unsigned int index);
  template <typename T>
  static inline bool validate_extension(const OCPIP_VERSION::infr::ocp_base_guarded_data_id&, payload_type& txn, unsigned int index);
  
  template <typename T>
  static inline void invalidate_extension(const OCPIP_VERSION::infr::ocp_base_real_guard_id&, payload_type& txn, unsigned int index);
  template <typename T>
  static inline void invalidate_extension(const OCPIP_VERSION::infr::ocp_base_guarded_data_id&, payload_type& txn, unsigned int index);
  
};

} //end ns infr

typedef infr::extension_support_base<tlm::tlm_base_protocol_types> extension_api;

} //end ns OCPIP_VERSION

#include __MACRO_STRINGYFY__(../src/OCPIP_VERSION/base_socket/ocp_base_socket_extension_support_base.tpp)


