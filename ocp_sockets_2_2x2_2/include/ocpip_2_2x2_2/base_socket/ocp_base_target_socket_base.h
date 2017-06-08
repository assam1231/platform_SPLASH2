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
  #error ocp_base_target_socket_base.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{
namespace infr{
//This is the green_target_socket from which other target_sockets
// shall be derived
template <unsigned int BUSWIDTH=32,
          typename TRAITS=tlm::tlm_base_protocol_types,
          unsigned int N=0,
          typename BIND_BASE=bind_checker<TRAITS>
          >
class target_socket_base: 
                             public target_socket_callback_base<BUSWIDTH,TRAITS,N, typename BIND_BASE::root_type, typename BIND_BASE::multi_root_type>
                            ,public BIND_BASE
                            ,public BIND_BASE::ext_support_type
{
public:
//typedef section
  typedef TRAITS                                                       traits_type;
  typedef target_socket_callback_base<BUSWIDTH,TRAITS,N, typename BIND_BASE::root_type, typename BIND_BASE::multi_root_type>                 base_type;
  typedef typename base_type::base_initiator_socket_type          base_initiator_socket_type;
  typedef typename TRAITS::tlm_payload_type                       payload_type;
  typedef typename TRAITS::tlm_phase_type                         phase_type;
  typedef tlm::tlm_sync_enum                                      sync_enum_type;
  //typedef bind_checker<base_type>                       bind_checker_type;
  typedef BIND_BASE bind_checker_type;
  typedef typename BIND_BASE::ext_support_type             ext_support_type;

public:

  //the constructor requires a name
  target_socket_base(const char* name); 
  
protected:  

  //ATTENTION: derived socket should not implement end_of_elaboration
  // if they do, they have to call this end_of_elaboration before anything else
  void end_of_elaboration();
  
  //this function is called when we were successfully bound to someone
  // override it to react to the resolved config
  virtual void bound_to(const std::string&, typename BIND_BASE::bind_base_type*, unsigned int);
  
  //override this function when deriving from the greensocket
  virtual const std::string& get_type_string();

  //virtual unsigned int get_num_bindings();

  const static std::string s_kind;
  bool base_eoe_done;
};

} //end ns infr
} //end ns OCPIP_VERSION

#include __MACRO_STRINGYFY__(../src/OCPIP_VERSION/base_socket/ocp_base_target_socket_base.tpp)

