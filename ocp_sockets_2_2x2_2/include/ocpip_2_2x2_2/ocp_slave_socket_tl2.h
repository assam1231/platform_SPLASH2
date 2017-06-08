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
//  Description :
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_slave_socket_tl2.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{

template <unsigned int BUSWIDTH=32, unsigned int NUM_BINDS=1
         , typename BASE_SOCKET=OCPIP_VERSION::infr::target_socket_base<BUSWIDTH, tlm::tlm_base_protocol_types, NUM_BINDS> >
class ocp_slave_socket_tl2 : public ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>{
public:
  typedef ocp_slave_socket<BUSWIDTH, NUM_BINDS,BASE_SOCKET> base_type;
  ocp_slave_socket_tl2(const char* name)
    : base_type(name, ocp_tl2){}
  
};

}
