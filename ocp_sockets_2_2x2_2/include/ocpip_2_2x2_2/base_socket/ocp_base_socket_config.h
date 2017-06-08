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
  #error ocp_base_socket_config.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{
namespace infr{

//This structure represents the configuration for a GreenSocket
// it should be set up during construction or before_end_of_elaboration
// and be assigned to a GreenSocket
template <typename TRAITS>
struct config //: protected OCPIP_VERSION::infr::bindability_proxy
{
  friend struct osci_config;

  typedef typename TRAITS::tlm_payload_type  payload_type;
  typedef typename TRAITS::tlm_phase_type  phase_type;
  //typedef OCPIP_VERSION::infr::bindability_proxy bind_proxy_type;

  //empty constructor
  config();

  //copy constructor
  config(const config&);

  //destructor
  ~config();

  //add a mandatory extension to the configuration
  //Example:
  //  OCPIP_VERSION::socket::config conf;
  //  conf.use_manadatory_extension<cacheable>();
  //
  // This will add the cacheable extension as mandatory to the configuration
  template <typename T>
  void use_mandatory_extension();

  //add a ignorable extension to the configuration
  //Example:
  //  OCPIP_VERSION::socket::config conf;
  //  conf.use_optional_extension<cacheable>();
  //
  // This will add the cacheable extension as ignorable to the configuration
  template <typename T>
  void use_optional_extension();

  //add a rejected extension to the configuration
  //Example:
  //  OCPIP_VERSION::socket::config conf;
  //  conf.reject_extension<cacheable>();
  //
  // This will add the cacheable extension as rejected to the configuration
  template <typename T>
  void reject_extension();

  //remove an extension from the configuration
  //Example:
  //  OCPIP_VERSION::socket::config conf;
  //  conf.remove_extension<cacheable>();
  //
  // This will remove the cacheable extension from the configuration
  template <typename T>
  void remove_extension();
  
  //add a phase to the configuration as mandatory
  //Example:
  //  OCPIP_VERSION::socket::config conf;
  //  conf.use_mandatory_phase(tlm::BEGIN_REQ);
  //
  // This will add the osci phases BEGIN_REQ as mandatory to the configuration
  void use_mandatory_phase(unsigned int ph1);

  //add a phase to the configuration as ignorable
  //Example:
  //  OCPIP_VERSION::socket::config conf;
  //  conf.use_optional_phase(tlm::BEGIN_REQ);
  //
  // This will add the osci phases BEGIN_REQ as ignorable to the configuration
  void use_optional_phase(unsigned int ph1);
  
  //add a phase to the configuration as rejected
  //Example:
  //  OCPIP_VERSION::socket::config conf;
  //  conf.use_optional_phase(tlm::BEGIN_RESP);
  //
  // This will add the osci phases BEGIN_RESP as rejected to the configuration  
  void reject_phase(unsigned int ph1);

  //remove an phase from the configuration
  //Example:
  //  OCPIP_VERSION::socket::config conf;
  //  conf.remove_phase(tlm::BEGIN_REQ);
  //
  // This will remove the BEGIN_REQ phase from the configuration
  void remove_phase(unsigned int ph1);

  //set how the socket will handle unknown extensions/phases (i.e. extensions/phases
  // that are not part of its configuration)
  //
  // they can be either treated as ignoreable (so after binding they will be part of the configuration)
  // or as rejected
  void treat_unknown_as_optional();
  void treat_unknown_as_rejected();
  
  //find out how unknown extensions are handled
  bool unknowns_are_optional();

  //merge a configuration with another
  // the first const char* shall identify the owner of the configuration
  //  on which the function is called, the second const char* shall
  //  identify the owner of the configuration that is passed as the 
  //  third argument (these const chars* will be used in the error
  //  messages)
  //
  //The merge A.merge_with("A","B", B) will only be successful if:
  //  * if all mandatory extension and phases of A are mandatory or ignorable in B
  //  * if no rejected extension or phase of A is mandatory in B
  //  * if A treats unknowns as rejected but there are no unknowns in B
  //
  //After the merge configuration A changes:
  //  * mandatory extensions stay mandatory (no change)
  //  * rejected extensions stay rejected (no change)
  //  * ignorable extensions that were rejected in B are removed from A
  //  * ignorable extension that were mandatory in B are now mandatory in A
  //  * ignorable extension that are ignorable in B stay ignorable
  //  * if B treats unknowns as rejected, ignorables unknown to B are removed from A
  //  * if B treats unknowns as ignorable, ignorables unknown to B stay ignorable in A
  //  * if A treats unknowns as ignorable, unknowns in B are added as ignorable to A
  //  * all the above points for phases
  //  * if B treats unknowns as rejected A will do so now
  bool merge_with(const char*, const char*, config&, bool abort_at_mismatch=true);
  
//  void combine_with(config&);
  
  //this functions converts the configuration into a string to be dumped to files or std::out
  // it is for debug only
  std::string to_string() const;
  
  //test if a certain extension is part of the configuration
  // if a pointer to a bindability_enum is provided, it will point to the
  //  binability state of the extension
  template <typename T>
  OCPIP_VERSION::infr::ocp_base_extension_bindability_enum has_extension() const;

  //test if a certain phase is part of the configuration
  // if a pointer to a bindability_enum is provided, it will point to the
  //  binability state of the extension  
  OCPIP_VERSION::infr::ocp_base_extension_bindability_enum has_phase(unsigned int) const;
  
  //When a configuration is changed during elaboration, the socket will reevaluate the binding only if the new configuration differs from the old one. 
  //However, customized sockets may use the callbacks that are generated due to a binding check to transmit information on top of the actual GreenSocket 
  //configuration. In this case you can use this function to enforce GreenSocket to reevaluate the binding and do all callbacks again, 
  //even if the GreenSocket configuration did not change.  
  void set_force_reeval(bool val);

  //this function can be used to check if a configuration is invalid
  // it shall only be used on socket configurations after construction time
  bool get_invalid()const;  
  
  //assignment operator
  config& operator=(const config&);
protected:

  void set_string_ptr(const std::string* name_);

  void set_invalid(bool value);

  void add_to_phase_map(std::vector<unsigned int>& phs, OCPIP_VERSION::infr::ocp_base_extension_bindability_enum mandatory);

  std::vector<OCPIP_VERSION::infr::ocp_base_extension_bindability_enum> m_used_extensions;
  std::map<std::string, OCPIP_VERSION::infr::ocp_base_extension_bindability_enum> m_used_phases;
public:
  bool diff(const config&);
  const std::string* m_type_string;
protected:
  bool m_treat_unknown_as_rejected;
public:
  bool invalid;
  bool force_reeval;
  bool force_own_cb;
  
  static std::vector<std::string>& get_ext_name_vect();
  static std::string& get_ext_name(unsigned int index);
};

} //end ns infr
} //ens ns OCPIP_VERSION

#include __MACRO_STRINGYFY__(../src/OCPIP_VERSION/base_socket/ocp_base_socket_config.tpp)


