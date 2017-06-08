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
//  Description :  This file contains the ocp_slave_socket that ocp slave 
//                 modules shall use.
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_slave_socket.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{

template < unsigned int BUSWIDTH=32
        , unsigned int NUM_BINDS=1
        , typename BASE_SOCKET=OCPIP_VERSION::infr::target_socket_base<BUSWIDTH, tlm::tlm_base_protocol_types, NUM_BINDS> >
class ocp_slave_socket : public BASE_SOCKET
              , public ocp_config_support<typename BASE_SOCKET::bind_checker_type>
{

friend class ocp_connection_monitor_timing_attachment;
  
public:
  typedef BASE_SOCKET base_type;
  typedef ocp_peq<ocp_slave_socket<BUSWIDTH, NUM_BINDS, BASE_SOCKET>, NUM_BINDS!=1> peq_type;
  typedef ocp_config_support<typename base_type::bind_checker_type> ocp_config_support_type;
  typedef tlm::tlm_base_protocol_types traits_type;

  typedef typename base_type::bind_checker_type::bind_base_type bindability_base_type;
  typedef typename base_type::bind_checker_type::ext_support_type ext_support_type;

  //Ctor to be used for modules that are not interested in timing callbacks
  ocp_slave_socket(const char* name, ocp_layer_ids layer);
  
  //Ctor that allows for registering a timing listener callback
  template<typename MODULE>
  ocp_slave_socket(const char* name, ocp_layer_ids layer, MODULE* owner, void (MODULE::*timing_cb)(ocp_tl1_master_timing));

  //assign a non default timing to that socket
  void set_slave_timing(const ocp_tl1_slave_timing& my_timing);
  void set_slave_timing(const ocp_tl1_slave_timing& my_timing, unsigned int);
  
  //register an nb_transport like function
  // It is called after a 'real' nb_transport arrived and the time
  //  argument of this nb_transport has expired.
  template<typename MODULE>
  void register_nb_transport_fw( MODULE* mod
                               , typename nb_cb_type_selector<MODULE, NUM_BINDS!=1>::nb_cb_type
                               , bool use_peq);

  template<typename MODULE>
  void register_nb_transport_fw( MODULE* mod
                               , typename nb_cb_type_selector<MODULE, NUM_BINDS!=1>::nb_cb_type);

  void activate_delta_cycle_protection();
  void activate_synchronization_protection();
  
  bw_interface_wrapper<base_type, peq_type, NUM_BINDS!=1>* operator->(){return &m_if_wrapper;}
  bw_interface_wrapper<base_type, peq_type, NUM_BINDS!=1>* operator[](unsigned int i){return m_if_wrapper[i];}
  
#if (TLM_VERSION_MAJOR>2 | TLM_VERSION_MINOR>0 | TLM_VERSION_PATCH>0)  
  void reset();
#endif
  
protected:

  //nb_transport_fw is overridden to either feed it into the PEQ
  // when the peq nb_transport was registered or to feed it directly
  //  into the slave (through the ocp base socket)
  
  void nb_transport_fw_peq(tlm::tlm_generic_payload& trans,
                                 const tlm::tlm_phase& phase);

  void nb_transport_fw_peq(unsigned int, tlm::tlm_generic_payload& trans,
                                 const tlm::tlm_phase& phase);
  


  tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans,
                                 tlm::tlm_phase& phase,
                                 sc_core::sc_time& t);

  tlm::tlm_sync_enum nb_transport_fw(unsigned int, tlm::tlm_generic_payload& trans,
                                 tlm::tlm_phase& phase,
                                 sc_core::sc_time& t);

  //called when the master socket has changed its timing
  void set_master_timing(const ocp_tl1_master_timing& other_timing);
  
  //helper to transmit the timing information to the connected master
  void transmit_timing(unsigned int);
  
  //called by the ocp base socket as soon as it was successfully bound to another socket
  // at this time extension/phase based bindability has already been checked by the ocp base socket
  virtual void bound_to(const std::string& other_type, bindability_base_type* other, unsigned int);

  //get the type string of this socket. This function is required by ocp base socket
  virtual std::string& get_type_string();

  //at start of simulation the ocp socket makes sure that it is not generic anymore
  virtual void start_of_simulation();

  virtual void before_end_of_elaboration();

  static std::string s_type;
  set_master_timing_callback_functor<traits_type> m_time_cb;
  std::vector<ocp_tl1_slave_timing> m_timing;
  ocp_tl1_slave_timing m_one_for_all_timing;
  std::vector<bool> m_bound;
  bool m_use_sp;
  peq_type* m_nb_peq, *m_reset_peq;
  typename nb_functor_type_selector<NUM_BINDS!=1>::nb_functor_type m_nb_fw_functor;
  sc_core::sc_time m_null_time;
  bw_interface_wrapper<base_type, peq_type, NUM_BINDS!=1> m_if_wrapper;
};

} //end ns OCPIP_VERSION

#include __MACRO_STRINGYFY__(../src/OCPIP_VERSION/ocp_slave_socket.tpp)

