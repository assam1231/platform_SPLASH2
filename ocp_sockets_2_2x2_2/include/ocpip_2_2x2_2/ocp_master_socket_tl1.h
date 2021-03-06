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
  #error ocp_master_socket_tl1.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{

template <unsigned int BUSWIDTH=32, unsigned int NUM_BINDS=1
         , typename BASE_SOCKET=OCPIP_VERSION::infr::initiator_socket_base<BUSWIDTH, tlm::tlm_base_protocol_types, NUM_BINDS> >
class ocp_master_socket_tl1 
  : public ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>{
public:
  typedef ocp_master_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET> base_type;

  //Ctor to be used for modules that are not interested in timing callbacks
  ocp_master_socket_tl1( const char* name
                       , typename base_type::allocation_scheme_type scheme=base_type::base_type::mm_txn_only())
    : base_type(name, ocp_tl1, scheme){}
    
  //Ctor that allows for registering a timing listener callback
  template<typename MODULE>
  ocp_master_socket_tl1(const char* name, 
                    MODULE* owner, 
                    void (MODULE::*timing_cb)(ocp_tl1_slave_timing), 
                    typename base_type::allocation_scheme_type scheme=base_type::base_type::mm_txn_only())
    : base_type(name, ocp_tl1, owner, timing_cb, scheme){}

};

}
