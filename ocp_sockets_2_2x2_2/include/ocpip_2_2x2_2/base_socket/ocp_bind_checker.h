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
  #error ocp_bind_checker.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{
namespace infr{

//This is the base class for the GreenSocket.
// It is used to determine whether a socket is greensocket (using a dynamic cast)
// and to exchange the configuration information
template <typename TRAITS>
class bindability_base{
public:
  virtual OCPIP_VERSION::infr::config<TRAITS>&       get_config(unsigned int)=0;
  virtual bool get_t_piece_end(bindability_base*&, unsigned int&)=0;
  virtual const char* get_name()const =0;
  virtual sc_core::sc_object* get_parent() =0;
  virtual ~bindability_base(){}
};


template <typename TRAITS>
class bind_checker_base
{
public:
  virtual void set_config(const OCPIP_VERSION::infr::config<TRAITS>&)=0;
  virtual void set_config(const OCPIP_VERSION::infr::config<TRAITS>&, unsigned int)=0;
  virtual OCPIP_VERSION::infr::config<TRAITS>& get_recent_config(unsigned int index=0)=0;
  virtual ~bind_checker_base(){}
};

//This class handles the extension access
// it is used by both the target and the initiator socket
template <typename TRAITS>
class bind_checker
  : public bindability_base<TRAITS>
  , public bind_checker_base<TRAITS>
{
public:

  typedef TRAITS traits_type;
  typedef typename traits_type::tlm_payload_type     payload_type;
  typedef typename traits_type::tlm_phase_type       phase_type;
  typedef bindability_base<traits_type>       bind_base_type;
  typedef config<traits_type>                 config_type;
  typedef bind_checker_base<traits_type>      bind_checker_base_type;
  typedef extension_support_base<traits_type> ext_support_type;
  typedef ocp_base_callback_binder_base             root_type;
  typedef ocp_base_multi_to_multi_bind_base<TRAITS> multi_root_type; 

  template<typename SOCKET_CALLBACK_BASE>
  bind_checker(const char*, SOCKET_CALLBACK_BASE*);
  template<typename SOCKET_CALLBACK_BASE>
  bind_checker(const char*, SOCKET_CALLBACK_BASE*, unsigned int);
  virtual ~bind_checker();
  
  //convert the configuration of the class to a string
  std::string to_string();

  //this function applies a configuration to a socket derived from this class
  void set_config(const OCPIP_VERSION::infr::config<traits_type>&);
  
  void set_config(const OCPIP_VERSION::infr::config<traits_type>&, unsigned int);
  
  //this function returns the current configuration of a socket derived from this class
  OCPIP_VERSION::infr::config<traits_type>& get_recent_config(unsigned int index=0);
  
  //this function returns the name of a socket derived from this class
  virtual const char* get_name() const;

  virtual sc_core::sc_object* get_parent();

  //This function is called after bindability check
  // it provides the type (as a string) of and a pointer to the connected socket
  // sockets may override this function to react to the result of the binding
  virtual void bound_to(const std::string&, bindability_base<traits_type>*, unsigned int)=0;
  
  //This function must return a reference to a string that identifies the type of the socket
  // sockets shall override this function to return a string that represents their own type
  virtual const std::string& get_type_string()=0;
  
  void check_binding(unsigned int);
  bindability_base<traits_type>* get_other_side(unsigned int,unsigned int&);
  
  sc_core::sc_interface* get_bottom_of_hierarchy();
  
  unsigned int get_bus_width(){return m_bus_width;}
  
protected:

  unsigned int get_num_bindings();

  template<typename SOCKET_CALLBACK_BASE>
  static sc_core::sc_interface* get_bottom_of_hierarchy(void*);

  template<typename SOCKET_CALLBACK_BASE>
  static unsigned int get_num_bindings(void*);

  template<typename SOCKET_CALLBACK_BASE>
  static sc_core::sc_interface* get_interface(void*, unsigned int);
  
  virtual OCPIP_VERSION::infr::config<traits_type>& get_config(unsigned int); 
  virtual bool get_t_piece_end(bindability_base<traits_type>*&, unsigned int&);
  void resize_configs();
    
  std::vector<config<traits_type> > m_configs;
  const char* m_name;
  std::vector<bool> cb_done;
  enum {ocp_base_bind_check_not_allowed=0, ocp_base_bind_check_allowed=1, ocp_base_bind_check_running=2} bind_check_state;
  unsigned int (*get_num_bindinocp_base_ptr)(void*);
  sc_core::sc_interface* (*get_interface_ptr)(void*, unsigned int);
  sc_core::sc_interface* (*get_bottom_of_hierarchy_ptr)(void*);
  void* m_socket;
  const unsigned int m_bus_width;
};


} //end ns infr
} //end ns OCPIP_VERSION

#include __MACRO_STRINGYFY__(../src/OCPIP_VERSION/base_socket/ocp_bind_checker.tpp)

