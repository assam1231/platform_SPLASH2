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
//  Description :  This file contains the ocp_socket_base from which the
//                 ocp_config_support is derived. It enables timing
//                 information and configuration distribution.
//
//                 The file also contains the ocp_config_support class from
//                 which both the ocp master and slave socket will be derived.
//                 It deals with the socket configuration and the ocp bindability
//                 checks.
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_socket_base.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{

//template<typename BIND_BASE>
class ocp_socket_base{
public:
  virtual ocp_parameters get_ocp_config() const=0;
  virtual void set_master_timing(const ocp_tl1_master_timing&)=0;
  virtual void set_slave_timing(const ocp_tl1_slave_timing&)=0;
  virtual ~ocp_socket_base(){}
};



template<typename BIND_BASE>
class ocp_config_support
  : public ocp_socket_base
  , public tlm::tlm_mm_interface
{
public:
  typedef typename BIND_BASE::bind_checker_base_type bind_checker_base_type;
  typedef typename BIND_BASE::bind_base_type         bindability_base_type;
  typedef typename BIND_BASE::config_type            config_type;
  //typedef ocp_parameters<BIND_BASE>                  ocp_parameters_type;

enum master_or_slave_enum{OCP_MST, OCP_SLV};

  ocp_config_support( const char* parent_name
                    , unsigned int parent_bus_width
                    , unsigned int parent_binds
                    , bind_checker_base_type* parent
                    , master_or_slave_enum mNs
                    , ocp_layer_ids layer);

  ~ocp_config_support();

  //This function returns the pointer to the transaction that
  // is supposed to be used to signal thread busy changes
  tlm::tlm_generic_payload* get_tb_transaction();

  // This function returns the pointer to the transaction that
  // is supposed to be used to change timing annotations in TL2
  tlm::tlm_generic_payload* get_tl2_timing_transaction();

#if (TLM_VERSION_MAJOR>2 | TLM_VERSION_MINOR>0 | TLM_VERSION_PATCH>0)
  // This function returns the pointer to the transaction that
  // is supposed to be used to transmit start and end of resets
  tlm::tlm_generic_payload* get_reset_transaction();
#endif

  //This function returns the pointer to the transaction that
  // is supposed to be used to signal interrupt changes
  tlm::tlm_generic_payload* get_interrupt_transaction();

  //This function returns the pointer to the transaction that
  // is supposed to be used to signal m/s-flag changes
  tlm::tlm_generic_payload* get_flag_transaction();

  //This function returns the pointer to the transaction that
  // is supposed to be used to signal m/s-error
  tlm::tlm_generic_payload* get_error_transaction();

  void free(tlm::tlm_generic_payload*);

  //This function is used to assign an ocp parameters set to a
  // socket
  void set_ocp_config(const ocp_parameters& config);

  //call this function at before end of elab
  //it checks if the ocp base socket has been configured
  // if not it configures it now with what we have (maybe it was configured via a configuration environment)
  void auto_set_config();

  //This function registers a configuration listener with the socket
  template <typename MODULE>
  void register_configuration_listener_callback(MODULE* owner, void (MODULE::*set_config_cb)(const ocp_parameters&, const std::string&));

  //This function registers a configuration listener with the socket
  template <typename MODULE>
  void register_configuration_listener_callback(MODULE* owner, void (MODULE::*set_config_cb)(const ocp_parameters&, const std::string&, unsigned int));

  //this function gets the current config from a socket
  ocp_parameters get_resolved_ocp_config(unsigned int index=0) const;
  ocp_parameters get_ocp_config() const;

  //this function sets a socket in the ocp generic mode
  void make_generic();

  //this function can be used to check if a socket is generic or not
  bool is_generic(unsigned int& index);

  bool is_configured();

  //this function checks if an ocp socket is compatible with another socket
  void check_config(const std::string& other_type, bindability_base_type* other, unsigned int index);

  //All functions below are taken from the OCP TL1 channel kit's tl_config_manager (slightly adjusted)
/*
  static bool compareParams(const int& master_param, const int& slave_param, ocp_param<int>& final);
  static bool compareParams(const bool& master_param, const bool& slave_param, ocp_param<bool>& final);
  static bool compareParams(const float& master_param, const float& slave_param, ocp_param<float>& final);
  */
  static bool compareParams(const int& master_param, const int& slave_param, int& final);
  static bool compareParams(const bool& master_param, const bool& slave_param, bool& final);
  static bool compareParams(const float& master_param, const float& slave_param, float& final);

  static void check_against_ocp_conf( master_or_slave_enum mOrS
                                    , const ocp_parameters& original_params
                                    , const ocp_parameters& other_params
                                    , ocp_parameters& resolved_params);

protected:

  const ocp_config_support* get_bottom_of_hierarchy(const char*, const std::string&) const;

  bind_checker_base_type*            m_parent;
  std::string  m_parent_name;
  unsigned int m_parent_bus_width, m_parent_binds;
  mutable ocp_parameters                  m_original_params;
  std::vector<ocp_parameters>     m_resolved_params;
  master_or_slave_enum m_master_or_slave;
  ocp_layer_ids m_layer;
  tlm::tlm_generic_payload m_tb_txn;
  cmd_thread_busy              m_ctb_ext;
  data_thread_busy              m_dtb_ext;
  resp_thread_busy              m_rtb_ext;

  // Timing annotation for TL2
  //tlm::tlm_generic_payload m_tl2_timing_txn;
  tl2_timing               m_tl2_timing_ext;

  std::vector<ocp_socket_base*> other_ocp;
  mutable std::vector<config_callback_functor<BIND_BASE>* > m_conf_cb;
  mutable std::vector<config_callback_with_index_functor<BIND_BASE>* > m_conf_with_index_cb;
  mutable std::string m_identified_as_bottom_of_hierarchy_by;
};

}//end ns OCPIP_VERSION

#include __MACRO_STRINGYFY__(../src/OCPIP_VERSION/ocp_config_support.tpp)
