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
  #error ocp_base_socket_osci_config.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{
namespace infr{

//this class provides global access to a OSCI compliant
// GreenSocket config.
//It is used to check bindability when connected to a non
// GreenSocket TLM2 socket
class osci_config{
public:
  static config<tlm::tlm_base_protocol_types>& get_cfg(){
    static OCPIP_VERSION::infr::osci_config tmp;
    return tmp.m_cfg;
  }

  
private:
  osci_config(){
    m_cfg.use_mandatory_phase(tlm::BEGIN_REQ);
    m_cfg.use_mandatory_phase(tlm::END_REQ);
    m_cfg.use_mandatory_phase(tlm::BEGIN_RESP);
    m_cfg.use_mandatory_phase(tlm::END_RESP);
    m_cfg.set_string_ptr(new std::string("plain_osci_socket"));
    m_cfg.treat_unknown_as_rejected();
    m_cfg.set_invalid(false);
  }
  config<tlm::tlm_base_protocol_types> m_cfg;
};


} //end ns infr
} //ens ns OCPIP_VERSION


