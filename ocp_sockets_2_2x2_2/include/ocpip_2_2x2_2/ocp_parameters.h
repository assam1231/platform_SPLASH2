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
//  Description :  A class that contains all the ocp parameters and shall
//                 be used to configure an ocp socket
//
//                 Basically there are two classes. One for socket internal
//                 use only (ocp_parameters_configurable) that enables to use
//                 external configuration tools to configure sockets.
//
//                 The other one (ocp_parameters) shall be used by users.
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#ifndef OCPIP_VERSION
  #error ocp_parameters.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif


#include <string>
#include <sstream>
#include <iostream>
#include <map>

namespace OCPIP_VERSION{

enum ocp_layer_ids{
  ocp_tl1=0x1,
  ocp_tl2=0x2,
  ocp_tl3=0x4,
  ocp_tl4=0x8
};

enum ocp_config_state_enum {ocp_unconfigured=0, ocp_generic=1, ocp_configured=2};

class ocp_parameters
{
public:

  // Constructor
  inline ocp_parameters();

  inline std::string as_rtl_conf(bool show_defaults = false) const;

  template<typename DataCl>
  ocpip_legacy::ParamCl<DataCl> legacy_conversion() const;
  
  inline void init_from_legacy(const ocpip_legacy::OCPParameters&);
  

  //this functions extracts the used TLM2 extensions and phases
  // out of the ocp paramters set
  template<typename BIND_BASE>
  typename BIND_BASE::config_type convert_to_base_socket_config(bool mNs, ocp_layer_ids layer);
    
  //only used, if the an ocp socket is connected to a non ocp socket
  //TODO: make it fit to the new has_extension and has_phase signatures
  template<typename BIND_BASE>
  void init_from_base_socket_config(const typename BIND_BASE::config_type& conf, unsigned int data_width);
  
  //this function initializes an ocp parameters set so that it is OSCI compliant
  //the argument data_width will be used to configure the data_wdth parameter of
  //ocp
  inline void init_as_osci_config(unsigned int data_width);

  //this function compares ocp parameter set provided as the function argument to
  // the ocp parameters set on which the function is called
  // It returns true if there is a difference
  inline bool diff(const ocp_parameters& other);
  
  inline ocp_parameters& operator=(const ocp_parameters&);
  
  // OCP parameters
  float ocp_version;
  std::string name; 
  bool broadcast_enable;
  bool burst_aligned;
  bool burstseq_dflt1_enable;
  bool burstseq_dflt2_enable;
  bool burstseq_incr_enable;
  bool burstseq_strm_enable;
  bool burstseq_unkn_enable;
  bool burstseq_wrap_enable;
  bool burstseq_xor_enable;
  bool burstseq_blck_enable;
  int endian;
  bool force_aligned;
  bool mthreadbusy_exact;
  bool rdlwrc_enable;
  bool read_enable;
  bool readex_enable;
  bool sdatathreadbusy_exact;
  bool sthreadbusy_exact;
  bool write_enable;
  bool writenonpost_enable;
  bool datahandshake;
  bool reqdata_together;
  bool writeresp_enable;
  bool addr;
  int addr_wdth;
  bool addrspace;
  int addrspace_wdth;
  bool atomiclength;
  int atomiclength_wdth;
  bool burstlength;
  int burstlength_wdth;
  bool blockheight;
  int blockheight_wdth;
  bool blockstride;
  int blockstride_wdth;
  bool burstprecise;
  bool burstseq;
  bool burstsinglereq;
  bool byteen;
  bool cmdaccept;
  bool connid;
  int connid_wdth;
  bool dataaccept;
  bool datalast;
  bool datarowlast;
  int data_wdth;
  bool mdata;
  bool mdatabyteen;
  bool mdatainfo;
  int mdatainfo_wdth;
  int mdatainfobyte_wdth;
  bool sdatathreadbusy;
  bool mthreadbusy;
  bool reqinfo;
  int reqinfo_wdth;
  bool reqlast;
  bool reqrowlast;
  bool resp;
  bool respaccept;
  bool respinfo;
  int respinfo_wdth;
  bool resplast;
  bool resprowlast;
  bool sdata;
  bool sdatainfo;
  int sdatainfo_wdth;
  int sdatainfobyte_wdth;
  bool sthreadbusy;
  int threads;
  int tags;
  bool taginorder;
  bool control;
  bool controlbusy;
  int control_wdth;
  bool controlwr;
  bool interrupt;
  bool merror;
  bool mflag;
  int mflag_wdth;
  bool mreset;
  bool serror;
  bool sflag;
  int sflag_wdth;
  bool sreset;
  bool status;
  bool statusbusy;
  bool statusrd;
  int status_wdth;
  bool sthreadbusy_pipelined;
  bool mthreadbusy_pipelined;
  bool sdatathreadbusy_pipelined;
  int tag_interleave_size;
  int enableclk;  
  int config_state;
};

} //end ns OCPIP_VERSION


#include __MACRO_STRINGYFY__(../src/OCPIP_VERSION/ocp_parameters.tpp)
/*

//Copy and paste the code in this comment section to
// set up a configuration that covers all possible
// parameters

map_string_type config_map";

config_map["broadcast_enable"]="i:0";
config_map["burst_aligned"]="i:0";
config_map["burstseq_dflt1_enable"]="i:0";
config_map["burstseq_dflt2_enable"]="i:0";
config_map["burstseq_incr_enable"]="i:0";
config_map["burstseq_strm_enable"]="i:0";
config_map["burstseq_unkn_enable"]="i:0";
config_map["burstseq_wrap_enable"]="i:0";
config_map["burstseq_xor_enable"]="i:0";
config_map["burstseq_blck_enable"]="i:0";
config_map["endian"]="i:1";
config_map["force_aligned"]="i:0";
config_map["mthreadbusy_exact"]="i:0";
config_map["rdlwrc_enable"]="i:0";            
config_map["read_enable"]="i:1";
config_map["readex_enable"]="i:0";
config_map["sdatathreadbusy_exact"]="i:0";
config_map["sthreadbusy_exact"]="i:0";
config_map["write_enable"]="i:1";
config_map["writenonpost_enable"]="i:0";
config_map["datahandshake"]="i:0";
config_map["reqdata_together"]="i:1";
config_map["writeresp_enable"]="i:1";
config_map["addr"]="i:1";
config_map["addr_wdth"]="i:64";
config_map["addrspace"]="i:0";
config_map["addrspace_wdth"]="i:1";
config_map["atomiclength"]="i:0";
config_map["atomiclength_wdth"]="i:1";
config_map["burstlength"]="i:1";
config_map["burstlength_wdth"]="i:0";
config_map["blockheight"]="i:0";
config_map["blockheight_wdth"]="i:1";
config_map["blockstride"]="i:0";
config_map["blockstride_wdth"]="i:1";
config_map["burstprecise"]="i:0";
config_map["burstseq"]="i:0";
config_map["burstsinglereq"]="i:0";
config_map["byteen"]="i:1";
config_map["cmdaccept"]="i:1";
config_map["connid"]="i:0";
config_map["connid_wdth"]="i:1";
config_map["dataaccept"]="i:0";
config_map["datalast"]="i:0";
config_map["datarowlast"]="i:0";
config_map["data_wdth"]="i:0";
config_map["mdata"]="i:1";
config_map["mdatabyteen"]="i:1";
config_map["mdatainfo"]="i:0";
config_map["mdatainfo_wdth"]="i:1";
config_map["mdatainfobyte_wdth"]="i:1";
config_map["sdatathreadbusy"]="i:0";
config_map["mthreadbusy"]="i:0";
config_map["reqinfo"]="i:0";
config_map["reqinfo_wdth"]="i:1";
config_map["reqlast"]="i:0";
config_map["reqrowlast"]="i:0";
config_map["resp"]="i:1";
config_map["respaccept"]="i:1";
config_map["respinfo"]="i:0";
config_map["respinfo_wdth"]="i:1";
config_map["resplast"]="i:0";
config_map["resprowlast"]="i:0";
config_map["sdata"]="i:1";
config_map["sdatainfo"]="i:0";
config_map["sdatainfo_wdth"]="i:1";
config_map["sdatainfobyte_wdth"]="i:1";
config_map["sthreadbusy"]="i:0";
config_map["threads"]="i:1";
config_map["tags"]="i:1";
config_map["taginorder"]="i:0";
config_map["control"]="i:0";
config_map["controlbusy"]="i:0";
config_map["control_wdth"]="i:1";
config_map["controlwr"]="i:0";
config_map["interrupt"]="i:0";
config_map["merror"]="i:0";
config_map["mflag"]="i:0";
config_map["mflag_wdth"]="i:1";
config_map["mreset"]="i:0";
config_map["serror"]="i:0";
config_map["sflag"]="i:0";
config_map["sflag_wdth"]="i:1";
config_map["sreset"]="i:0";
config_map["status"]="i:0";
config_map["statusbusy"]="i:0";
config_map["statusrd"]="i:0";
config_map["status_wdth"]="i:1";
config_map["sthreadbusy_pipelined"]="i:0";
config_map["mthreadbusy_pipelined"]="i:0";
config_map["sdatathreadbusy_pipelined"]="i:0";

*/
