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

#define PARAM_CL_MEMBER_COPY(name) \
tmp_param_cl.name=name
template <typename DataCl>
ocpip_legacy::ParamCl<DataCl> OCPIP_VERSION::ocp_parameters::legacy_conversion() const{
  ocpip_legacy::ParamCl<DataCl> tmp_param_cl;
  PARAM_CL_MEMBER_COPY(ocp_version);
  PARAM_CL_MEMBER_COPY(broadcast_enable);
  PARAM_CL_MEMBER_COPY(burst_aligned);
  PARAM_CL_MEMBER_COPY(burstseq_dflt1_enable);
  PARAM_CL_MEMBER_COPY(burstseq_dflt2_enable);
  PARAM_CL_MEMBER_COPY(burstseq_incr_enable);
  PARAM_CL_MEMBER_COPY(burstseq_strm_enable);
  PARAM_CL_MEMBER_COPY(burstseq_unkn_enable);
  PARAM_CL_MEMBER_COPY(burstseq_wrap_enable);
  PARAM_CL_MEMBER_COPY(burstseq_xor_enable);
  PARAM_CL_MEMBER_COPY(burstseq_blck_enable);
  PARAM_CL_MEMBER_COPY(endian);
  PARAM_CL_MEMBER_COPY(force_aligned);
  PARAM_CL_MEMBER_COPY(mthreadbusy_exact);
  PARAM_CL_MEMBER_COPY(rdlwrc_enable);
  PARAM_CL_MEMBER_COPY(read_enable);
  PARAM_CL_MEMBER_COPY(readex_enable);
  PARAM_CL_MEMBER_COPY(sdatathreadbusy_exact);
  PARAM_CL_MEMBER_COPY(sthreadbusy_exact);
  PARAM_CL_MEMBER_COPY(write_enable);
  PARAM_CL_MEMBER_COPY(writenonpost_enable);
  PARAM_CL_MEMBER_COPY(datahandshake);
  PARAM_CL_MEMBER_COPY(reqdata_together);
  PARAM_CL_MEMBER_COPY(writeresp_enable);
  PARAM_CL_MEMBER_COPY(addr);
  PARAM_CL_MEMBER_COPY(addr_wdth);
  PARAM_CL_MEMBER_COPY(addrspace);
  PARAM_CL_MEMBER_COPY(addrspace_wdth);
  PARAM_CL_MEMBER_COPY(atomiclength);
  PARAM_CL_MEMBER_COPY(atomiclength_wdth);
  PARAM_CL_MEMBER_COPY(burstlength);
  PARAM_CL_MEMBER_COPY(burstlength_wdth);
  PARAM_CL_MEMBER_COPY(blockheight);
  PARAM_CL_MEMBER_COPY(blockheight_wdth);
  PARAM_CL_MEMBER_COPY(blockstride);
  PARAM_CL_MEMBER_COPY(blockstride_wdth);
  PARAM_CL_MEMBER_COPY(burstprecise);
  PARAM_CL_MEMBER_COPY(burstseq);
  PARAM_CL_MEMBER_COPY(burstsinglereq);
  PARAM_CL_MEMBER_COPY(byteen);
  PARAM_CL_MEMBER_COPY(cmdaccept);
  PARAM_CL_MEMBER_COPY(connid);
  PARAM_CL_MEMBER_COPY(connid_wdth);
  PARAM_CL_MEMBER_COPY(dataaccept);
  PARAM_CL_MEMBER_COPY(datalast);
  PARAM_CL_MEMBER_COPY(datarowlast);
  PARAM_CL_MEMBER_COPY(data_wdth);
  PARAM_CL_MEMBER_COPY(mdata);
  PARAM_CL_MEMBER_COPY(mdatabyteen);
  PARAM_CL_MEMBER_COPY(mdatainfo);
  PARAM_CL_MEMBER_COPY(mdatainfo_wdth);
  PARAM_CL_MEMBER_COPY(mdatainfobyte_wdth);
  PARAM_CL_MEMBER_COPY(sdatathreadbusy);
  PARAM_CL_MEMBER_COPY(mthreadbusy);
  PARAM_CL_MEMBER_COPY(reqinfo);
  PARAM_CL_MEMBER_COPY(reqinfo_wdth);
  PARAM_CL_MEMBER_COPY(reqlast);
  PARAM_CL_MEMBER_COPY(reqrowlast);
  PARAM_CL_MEMBER_COPY(resp);
  PARAM_CL_MEMBER_COPY(respaccept);
  PARAM_CL_MEMBER_COPY(respinfo);
  PARAM_CL_MEMBER_COPY(respinfo_wdth);
  PARAM_CL_MEMBER_COPY(resplast);
  PARAM_CL_MEMBER_COPY(resprowlast);
  PARAM_CL_MEMBER_COPY(sdata);
  PARAM_CL_MEMBER_COPY(sdatainfo);
  PARAM_CL_MEMBER_COPY(sdatainfo_wdth);
  PARAM_CL_MEMBER_COPY(sdatainfobyte_wdth);
  PARAM_CL_MEMBER_COPY(sthreadbusy);
  PARAM_CL_MEMBER_COPY(threads);
  PARAM_CL_MEMBER_COPY(tags);
  PARAM_CL_MEMBER_COPY(taginorder);
  PARAM_CL_MEMBER_COPY(control);
  PARAM_CL_MEMBER_COPY(controlbusy);
  PARAM_CL_MEMBER_COPY(control_wdth);
  PARAM_CL_MEMBER_COPY(controlwr);
  PARAM_CL_MEMBER_COPY(interrupt);
  PARAM_CL_MEMBER_COPY(merror);
  PARAM_CL_MEMBER_COPY(mflag);
  PARAM_CL_MEMBER_COPY(mflag_wdth);
  PARAM_CL_MEMBER_COPY(mreset);
  PARAM_CL_MEMBER_COPY(serror);
  PARAM_CL_MEMBER_COPY(sflag);
  PARAM_CL_MEMBER_COPY(sflag_wdth);
  PARAM_CL_MEMBER_COPY(sreset);
  PARAM_CL_MEMBER_COPY(status);
  PARAM_CL_MEMBER_COPY(statusbusy);
  PARAM_CL_MEMBER_COPY(statusrd);
  PARAM_CL_MEMBER_COPY(status_wdth);
  PARAM_CL_MEMBER_COPY(sthreadbusy_pipelined);
  PARAM_CL_MEMBER_COPY(mthreadbusy_pipelined);
  PARAM_CL_MEMBER_COPY(sdatathreadbusy_pipelined);
  PARAM_CL_MEMBER_COPY(tag_interleave_size);
  PARAM_CL_MEMBER_COPY(enableclk);
  return tmp_param_cl;
}
#undef PARAM_CL_MEMBER_COPY

#define PARAM_CL_MEMBER_COPY(name) \
name=legacy_params.name

void OCPIP_VERSION::ocp_parameters::init_from_legacy(const ocpip_legacy::OCPParameters& legacy_params){
  PARAM_CL_MEMBER_COPY(ocp_version);
  PARAM_CL_MEMBER_COPY(broadcast_enable);
  PARAM_CL_MEMBER_COPY(burst_aligned);
  PARAM_CL_MEMBER_COPY(burstseq_dflt1_enable);
  PARAM_CL_MEMBER_COPY(burstseq_dflt2_enable);
  PARAM_CL_MEMBER_COPY(burstseq_incr_enable);
  PARAM_CL_MEMBER_COPY(burstseq_strm_enable);
  PARAM_CL_MEMBER_COPY(burstseq_unkn_enable);
  PARAM_CL_MEMBER_COPY(burstseq_wrap_enable);
  PARAM_CL_MEMBER_COPY(burstseq_xor_enable);
  PARAM_CL_MEMBER_COPY(burstseq_blck_enable);
  PARAM_CL_MEMBER_COPY(endian);
  PARAM_CL_MEMBER_COPY(force_aligned);
  PARAM_CL_MEMBER_COPY(mthreadbusy_exact);
  PARAM_CL_MEMBER_COPY(rdlwrc_enable);
  PARAM_CL_MEMBER_COPY(read_enable);
  PARAM_CL_MEMBER_COPY(readex_enable);
  PARAM_CL_MEMBER_COPY(sdatathreadbusy_exact);
  PARAM_CL_MEMBER_COPY(sthreadbusy_exact);
  PARAM_CL_MEMBER_COPY(write_enable);
  PARAM_CL_MEMBER_COPY(writenonpost_enable);
  PARAM_CL_MEMBER_COPY(datahandshake);
  PARAM_CL_MEMBER_COPY(reqdata_together);
  PARAM_CL_MEMBER_COPY(writeresp_enable);
  PARAM_CL_MEMBER_COPY(addr);
  PARAM_CL_MEMBER_COPY(addr_wdth);
  PARAM_CL_MEMBER_COPY(addrspace);
  PARAM_CL_MEMBER_COPY(addrspace_wdth);
  PARAM_CL_MEMBER_COPY(atomiclength);
  PARAM_CL_MEMBER_COPY(atomiclength_wdth);
  PARAM_CL_MEMBER_COPY(burstlength);
  PARAM_CL_MEMBER_COPY(burstlength_wdth);
  PARAM_CL_MEMBER_COPY(blockheight);
  PARAM_CL_MEMBER_COPY(blockheight_wdth);
  PARAM_CL_MEMBER_COPY(blockstride);
  PARAM_CL_MEMBER_COPY(blockstride_wdth);
  PARAM_CL_MEMBER_COPY(burstprecise);
  PARAM_CL_MEMBER_COPY(burstseq);
  PARAM_CL_MEMBER_COPY(burstsinglereq);
  PARAM_CL_MEMBER_COPY(byteen);
  PARAM_CL_MEMBER_COPY(cmdaccept);
  PARAM_CL_MEMBER_COPY(connid);
  PARAM_CL_MEMBER_COPY(connid_wdth);
  PARAM_CL_MEMBER_COPY(dataaccept);
  PARAM_CL_MEMBER_COPY(datalast);
  PARAM_CL_MEMBER_COPY(datarowlast);
  PARAM_CL_MEMBER_COPY(data_wdth);
  PARAM_CL_MEMBER_COPY(mdata);
  PARAM_CL_MEMBER_COPY(mdatabyteen);
  PARAM_CL_MEMBER_COPY(mdatainfo);
  PARAM_CL_MEMBER_COPY(mdatainfo_wdth);
  PARAM_CL_MEMBER_COPY(mdatainfobyte_wdth);
  PARAM_CL_MEMBER_COPY(sdatathreadbusy);
  PARAM_CL_MEMBER_COPY(mthreadbusy);
  PARAM_CL_MEMBER_COPY(reqinfo);
  PARAM_CL_MEMBER_COPY(reqinfo_wdth);
  PARAM_CL_MEMBER_COPY(reqlast);
  PARAM_CL_MEMBER_COPY(reqrowlast);
  PARAM_CL_MEMBER_COPY(resp);
  PARAM_CL_MEMBER_COPY(respaccept);
  PARAM_CL_MEMBER_COPY(respinfo);
  PARAM_CL_MEMBER_COPY(respinfo_wdth);
  PARAM_CL_MEMBER_COPY(resplast);
  PARAM_CL_MEMBER_COPY(resprowlast);
  PARAM_CL_MEMBER_COPY(sdata);
  PARAM_CL_MEMBER_COPY(sdatainfo);
  PARAM_CL_MEMBER_COPY(sdatainfo_wdth);
  PARAM_CL_MEMBER_COPY(sdatainfobyte_wdth);
  PARAM_CL_MEMBER_COPY(sthreadbusy);
  PARAM_CL_MEMBER_COPY(threads);
  PARAM_CL_MEMBER_COPY(tags);
  PARAM_CL_MEMBER_COPY(taginorder);
  PARAM_CL_MEMBER_COPY(control);
  PARAM_CL_MEMBER_COPY(controlbusy);
  PARAM_CL_MEMBER_COPY(control_wdth);
  PARAM_CL_MEMBER_COPY(controlwr);
  PARAM_CL_MEMBER_COPY(interrupt);
  PARAM_CL_MEMBER_COPY(merror);
  PARAM_CL_MEMBER_COPY(mflag);
  PARAM_CL_MEMBER_COPY(mflag_wdth);
  PARAM_CL_MEMBER_COPY(mreset);
  PARAM_CL_MEMBER_COPY(serror);
  PARAM_CL_MEMBER_COPY(sflag);
  PARAM_CL_MEMBER_COPY(sflag_wdth);
  PARAM_CL_MEMBER_COPY(sreset);
  PARAM_CL_MEMBER_COPY(status);
  PARAM_CL_MEMBER_COPY(statusbusy);
  PARAM_CL_MEMBER_COPY(statusrd);
  PARAM_CL_MEMBER_COPY(status_wdth);
  PARAM_CL_MEMBER_COPY(sthreadbusy_pipelined);
  PARAM_CL_MEMBER_COPY(mthreadbusy_pipelined);
  PARAM_CL_MEMBER_COPY(sdatathreadbusy_pipelined);
  PARAM_CL_MEMBER_COPY(tag_interleave_size);
  PARAM_CL_MEMBER_COPY(enableclk);
}
#undef PARAM_CL_MEMBER_COPY


//only used, if the an ocp socket is connected to a non ocp ocp base socket
//TODO: make it fit to the new has_extension and has_phase signatures
template<typename BIND_BASE>
void OCPIP_VERSION::ocp_parameters::init_from_base_socket_config(
  const typename BIND_BASE::config_type& conf, unsigned int data_width)
{
  std::cerr<<"init_from_base_socket_config is not implemented yet."<<std::endl;
  abort();
}
//this functions extracts the used TLM2 extensions and phases
// out of the ocp paramters set

template<typename BIND_BASE>
typename BIND_BASE::config_type
OCPIP_VERSION::ocp_parameters::convert_to_base_socket_config(bool mNs, ocp_layer_ids layer){
  typename BIND_BASE::config_type conf;
  //each OCP socket will reject unknown extensions by default
  conf.treat_unknown_as_rejected();
  //each socket mandates the use of the BP phases
  conf.use_mandatory_phase(tlm::BEGIN_REQ);
  conf.use_mandatory_phase(tlm::END_REQ);
  conf.use_mandatory_phase(tlm::BEGIN_RESP);
  conf.use_mandatory_phase(tlm::END_RESP);

  //if the socket can do ONLY TL1, we enforce the connected socket to understand TL1
  if (layer == ocp_tl1){
    conf.template use_mandatory_extension<tl1_id>();
  }
  else
  //if the socket can do TL1 and others, we allow to bind to TL1
  // if that fails (i.e. after the binding the tl1_id extension becomes rejected
  // the owner of the sockets knows not to use TL1
  if (layer & ocp_tl1){
    conf.template use_optional_extension<tl1_id>();
  }
  //the socket does not understand TL1 so we reject the extension
  else
  {
    conf.template reject_extension<tl1_id>();
  }

  //if the socket can do ONLY TL2, we enforce the connected socket to understand TL2
  if (layer == ocp_tl2){
                                  conf.template use_mandatory_extension<word_count>();
                                  conf.template use_mandatory_extension<tl2_timing>();
  }
  else
  //if the socket can do TL2 and others, we allow to bind to TL2
  // if that fails (i.e. after the binding the word_count and tl2_timing extensions become rejected
  // the owner of the sockets knows not to use TL2
  if (layer & ocp_tl2){
                                  conf.template use_optional_extension<word_count>();
                                  conf.template use_optional_extension<tl2_timing>();
  }
  //the socket does not understand TL2 so we reject the extensions
  else{
                                  conf.template reject_extension<word_count>();
                                  conf.template reject_extension<tl2_timing>();
  }

  //There is no extension that enforces understanding of TL3, because TL3 without mandatory extensions==BP.
  // and the understanding of BP is ensured via the traits class.


  //if a configuration is generic ocp, it treats all ocp extensions
  // as optional
  if (config_state==ocp_generic){
    conf.treat_unknown_as_rejected();
    conf.template use_optional_extension<broadcast>();
    conf.template use_optional_extension<burst_sequence>();
    conf.template use_optional_extension<semaphore>();
    conf.template use_optional_extension<lock>();
    conf.template use_optional_extension<posted>();
    //socket is TL3 only, so we do not allow data phases
    if (layer == ocp_tl3){
      conf.reject_phase(BEGIN_DATA);
      conf.reject_phase(END_DATA);
    }
    else
    //socket can do other TLs than 3, so we treat data phases optionally
    {
      conf.use_optional_phase(BEGIN_DATA);
      conf.use_optional_phase(END_DATA);
    }
    conf.template use_optional_extension<address_space>();
    conf.template use_optional_extension<atomic_length>();
    conf.template use_optional_extension<imprecise>();
    conf.template use_optional_extension<srmd>();
    conf.template use_optional_extension<conn_id>();
    conf.template use_optional_extension<thread_id>();
    conf.template use_optional_extension<tag_id>();
    conf.template use_optional_extension<cmd_thread_busy>();
    conf.template use_optional_extension<data_thread_busy>();
    conf.template use_optional_extension<resp_thread_busy>();
    conf.use_optional_phase(CMD_THREAD_BUSY_CHANGE);
    conf.use_optional_phase(DATA_THREAD_BUSY_CHANGE);
    conf.use_optional_phase(RESP_THREAD_BUSY_CHANGE);
    conf.template use_optional_extension<burst_length>();
    return conf;
  }

  //only use tb if layer is unequal to TL3 only
  if (sthreadbusy_exact && (layer!=ocp_tl3)) {
                                 conf.template use_mandatory_extension<cmd_thread_busy>();
                                 conf.use_mandatory_phase(CMD_THREAD_BUSY_CHANGE);
                                 }
  else
  if (sthreadbusy && (layer!=ocp_tl3)) {
                                 conf.template use_optional_extension<cmd_thread_busy>();
                                 conf.use_optional_phase(CMD_THREAD_BUSY_CHANGE);
  }
  else{
                                 conf.template reject_extension<cmd_thread_busy>();
                                 conf.reject_phase(CMD_THREAD_BUSY_CHANGE);
  }

  if (sdatathreadbusy_exact && (layer!=ocp_tl3)) {
                                 conf.template use_mandatory_extension<data_thread_busy>();
                                 conf.use_mandatory_phase(DATA_THREAD_BUSY_CHANGE);
                                 }
  else
  if (sdatathreadbusy && (layer!=ocp_tl3)) {
                                 conf.template use_optional_extension<data_thread_busy>();
                                 conf.use_optional_phase(DATA_THREAD_BUSY_CHANGE);
  }
  else{
                                 conf.template reject_extension<data_thread_busy>();
                                 conf.reject_phase(DATA_THREAD_BUSY_CHANGE);
  }

  if (mthreadbusy_exact && (layer!=ocp_tl3)) {
                                 conf.template use_mandatory_extension<resp_thread_busy>();
                                 conf.use_mandatory_phase(RESP_THREAD_BUSY_CHANGE);
                                 }
  else
  if (mthreadbusy && (layer!=ocp_tl3)) {
                                 conf.template use_optional_extension<resp_thread_busy>();
                                 conf.use_optional_phase(RESP_THREAD_BUSY_CHANGE);
  }
  else{
                                 conf.template reject_extension<resp_thread_busy>();
                                 conf.reject_phase(RESP_THREAD_BUSY_CHANGE);
  }

  if (datahandshake && (layer!=ocp_tl3)){ //only use dhs if we use a layer (combination) unequal to TL3 only
                                 conf.use_mandatory_phase(BEGIN_DATA);
                                 conf.use_mandatory_phase(END_DATA);
  }
  else{
                                 conf.reject_phase(BEGIN_DATA);
                                 conf.reject_phase(END_DATA);
  }

  if (blockheight | blockstride) assert((burstseq) && "blockheight or blockstride used without burstseq enabled.");

  //master and slave config treat some extensions differently wrt ignorability
  if (mNs){ //master config

    if (addrspace)                 conf.template use_mandatory_extension<address_space>();
    else                           conf.template use_optional_extension<address_space>();

    if (atomiclength)
          if (layer & ocp_tl3)     conf.template use_optional_extension<atomic_length>();
          else                     conf.template use_mandatory_extension<atomic_length>();
    else                           conf.template use_optional_extension<atomic_length>();

    if (broadcast_enable)          conf.template use_mandatory_extension<broadcast>();
    else                           conf.template use_optional_extension<broadcast>();

    if (burstlength)               conf.template use_optional_extension<burst_length>();
    else                           conf.template use_optional_extension<burst_length>();

    if (burstseq){
      if (burstseq_dflt1_enable | burstseq_dflt2_enable | burstseq_unkn_enable | burstseq_blck_enable)
                                   conf.template use_mandatory_extension<burst_sequence>();
      else
      if (burstseq_wrap_enable | burstseq_xor_enable)
        if (layer & ocp_tl3)       conf.template use_optional_extension<burst_sequence>();
        else                       conf.template use_mandatory_extension<burst_sequence>();
      else
                                   conf.template use_optional_extension<burst_sequence>();
    }
    else                           conf.template use_optional_extension<burst_sequence>();

    if (connid)                    conf.template use_mandatory_extension<conn_id>();
    else                           conf.template use_optional_extension<conn_id>();

    if (burstprecise)
      if (layer & ocp_tl3)
                                   conf.template use_optional_extension<imprecise>();
      else
                                   conf.template use_mandatory_extension<imprecise>();
    else                           conf.template use_optional_extension<imprecise>();

    if (readex_enable)
      if (layer & ocp_tl1)         conf.template use_mandatory_extension<lock>();
      else                         conf.template use_optional_extension<lock>();
    else                           conf.template use_optional_extension<lock>();

    //if (writenonpost_enable)       conf.template use_mandatory_extension<posted>();
    //else                           conf.template use_optional_extension<posted>();

    if (write_enable)              conf.template use_mandatory_extension<posted>();
    else                           conf.template use_optional_extension<posted>();

    if (rdlwrc_enable)             conf.template use_mandatory_extension<semaphore>();
    else                           conf.template use_optional_extension<semaphore>();

    if (burstsinglereq)
      if (layer & ocp_tl3)
                                   conf.template use_optional_extension<srmd>();
      else                         conf.template use_mandatory_extension<srmd>();
    else                           conf.template use_optional_extension<srmd>();

    if (tags>1)                    conf.template use_optional_extension<tag_id>();
    else                           conf.template use_optional_extension<tag_id>();

    if (threads>1)                 conf.template use_mandatory_extension<thread_id>();
    else                           conf.template use_optional_extension<thread_id>();

  }
  else{ //slave config
    if (addrspace)                 conf.template use_optional_extension<address_space>();
    else                           conf.template reject_extension<address_space>();

    if (atomiclength)              conf.template use_optional_extension<atomic_length>();
    else
      if (layer & ocp_tl3)         conf.template use_optional_extension<atomic_length>();
      else                         conf.template reject_extension<atomic_length>();

    if (broadcast_enable)          conf.template use_optional_extension<broadcast>();
    else                           conf.template reject_extension<broadcast>();

    if (burstlength)               conf.template use_optional_extension<burst_length>();
    else                           conf.template use_optional_extension<burst_length>();

    if (burstseq)                  conf.template use_optional_extension<burst_sequence>();
    else                           conf.template reject_extension<burst_sequence>();

    if (connid)                    conf.template use_optional_extension<conn_id>();
    else                           conf.template reject_extension<conn_id>();

    if (burstprecise)              conf.template use_optional_extension<imprecise>();
    else
      if (layer & ocp_tl3)         conf.template use_optional_extension<imprecise>();
      else                         conf.template reject_extension<imprecise>();

    if (readex_enable)             conf.template use_optional_extension<lock>();
    else
      if (layer & ocp_tl1)         conf.template reject_extension<lock>();
      else                         conf.template use_optional_extension<lock>();

    //if (writenonpost_enable)       conf.template use_optional_extension<posted>();
    //else                           conf.template reject_extension<posted>();

    if (write_enable)              conf.template use_optional_extension<posted>();
    else                           conf.template reject_extension<posted>();

    if (rdlwrc_enable)             conf.template use_optional_extension<semaphore>();
    else                           conf.template reject_extension<semaphore>();

    if (burstsinglereq)            conf.template use_optional_extension<srmd>();
    else
      if (layer & ocp_tl3)         conf.template use_optional_extension<srmd>();
      else                         conf.template reject_extension<srmd>();

    if (tags>1)                    conf.template use_optional_extension<tag_id>();
    else                           conf.template reject_extension<tag_id>();

    if (threads>1)                 conf.template use_optional_extension<thread_id>();
    else                           conf.template reject_extension<thread_id>();

  }
  return conf;
}

#define show_param(param) \
  if(show_defaults || (param != defaults.param)) \
    retval << "  param " << #param << " " << param << std::endl;

#define show_wdth_param(param, selector) \
  if(show_defaults || selector) \
    retval << "  param " << #param << " " << param << std::endl;

#define show_nodef_param(param) \
  retval << "  param " << #param << " " << param << std::endl;

OCPIP_VERSION::ocp_parameters::ocp_parameters()
  : ocp_version(2.0)                // NOTE: currently no way to change this.
  , name("unnamed_ocp_channel")     // NOTE: set in "setConfiguration()"
  , broadcast_enable(false)         // Is the Broadcast command supported?
  , burst_aligned(false)            // Are bursts forced to be aligned by power of two?
  , burstseq_dflt1_enable(false)    // Is DFLT1 burst mode enabled?
  , burstseq_dflt2_enable(false)    // Is DFLT2 burst mode enabled?
  , burstseq_incr_enable(true)      // Are Incrementing Bursts allowed?
  , burstseq_strm_enable(false)     // Are Streaming Bursts allowed?
  , burstseq_unkn_enable(false)     // Is UNKN burst mode enabled?
  , burstseq_wrap_enable(false)     // Is WRAP burst mode enabled?
  , burstseq_xor_enable(false)      // Is XOR burst mode enabled?
  , burstseq_blck_enable(false)
  , endian(1)                       // Endian is an int
  , force_aligned(false)            // Forces the byte-enable patterns to be powers of two.
  , mthreadbusy_exact(false)        // Slave must use MTheadbusy to send Resp. Master must accept immediately on non-busy threads.
  , rdlwrc_enable(false)
  , read_enable(true)               // Is the Read command supported?
  , readex_enable(false)            // Is the ReadEx command supported?
  , sdatathreadbusy_exact(false)    // Master must use SDataThreadBusy to send new Data. Slave must accept new data immediately on non-busy threads.
  , sthreadbusy_exact(false)        // Master must use SThreadBusy to send new Request. Slave must accept new request immediately on non-busy threads.
  , write_enable(true)              // Is the Write command supported?
  , writenonpost_enable(false)      // Is the WriteNonPost command supported?
  , datahandshake(false)            // Is there a separate channel for request data?
  , reqdata_together(false)         // Master always puts Request and Data in same cycle. Slave always accepts together in same cycle.
  , writeresp_enable(false)         // Should Responses be sent for write commands?
  , addr(true)                      // Is MAddr (Requst Address) part of the OCP?
  , addr_wdth(2)                    // Address width must be set by user if addr("",true. No default value.
  , addrspace(false)                // Is the MAddrSpace signal part of the OCP?
  , addrspace_wdth(1)               // No default value. Must be user specificied.
  , atomiclength(false)             // Is there a minimum number of transfers to hold together during a burst?
  , atomiclength_wdth(1)            // The minimum number. No default value.
  , burstlength(false)              // Is there a set number of transfers in a burst.
  , burstlength_wdth(1)             // The number of transfers in a burst. User defined. No default value.
  , blockheight(false)
  , blockheight_wdth(1)
  , blockstride(false)
  , blockstride_wdth(1)
  , burstprecise(false)             // Will the length of a burst be known at the start of the burst?
  , burstseq(false)                 // Is there a sequence of addresses in a burst?
  , burstsinglereq(false)           // Is a single request allowed to generate multiple data transfers in a burst?
  , byteen(false)                   // Is MByteEn part of the OCP?
  , cmdaccept(true)                 // Does slave accept commands?
  , connid(false)                   // Is MConnID connection identifier pard of the Request group?
  , connid_wdth(1)                  // Width of MConnID. User specified. No default.
  , dataaccept(false)               // Does slave accept data handshakes?
  , datalast(false)                 // Is the MDataLast burst signal part of the OCP?
  , datarowlast(false)
  , data_wdth(32)                   // width of MData. User specified. No default.
  , mdata(true)                     // is MData part of the OCP?
  , mdatabyteen(false)              // is the MDataByteEn signal in the OCP?
  , mdatainfo(false)                // is the MDataInfo signal in the OCP?
  , mdatainfo_wdth(1)               // Width of MDataInfo. User specified. no default.
  , mdatainfobyte_wdth(1)           // Number of bits of MDataInfo that are associated with each data byte of MData.
  , sdatathreadbusy(false)          // Is SDataThreadBusy part of the OCP channel?
  , mthreadbusy(false)              // Is MThreadBusy part of the OCP channel?
  , reqinfo(false)                  // Is the MReqInfo signal part of the OCP channel?
  , reqinfo_wdth(1)                 // Width of MReqInfo. User Specified. No default.
  , reqlast(false)                  // Is the MReqLast burst signal part of the OCP channel?
  , reqrowlast(false)
  , resp(true)                      // Is the SResp signal part of the OCP channel?
  , respaccept(false)               // Does the Master accept Responses with the MRespAccept signal?
  , respinfo(false)                 // Is the SRespInfo signal on the OCP?
  , respinfo_wdth(1)                // Width of SRespInfo. User Specified. No Default.
  , resplast(false)                 // Is the SRespLast burst signal part of the OCP channel?
  , resprowlast(false)
  , sdata(true)                     // Is the SData signal part of the OCP channel?
  , sdatainfo(false)                // Is the SDataInfo signal supported?
  , sdatainfo_wdth(1)               // Width of SDataInfo. User Specified. No default.
  , sdatainfobyte_wdth(1)           // Number of bits in SDataInfo devoted to each byte of SData. User Specified. No default.
  , sthreadbusy(false)              // Is the SThreadBusy signal supported?
  , threads(1)                      // Number of threads allowed.
  , tags(1)                         // Number of tags allowed.
  , taginorder(false)                   // In order required among tags
  , control(false)                  // Is the Sideband Control signal supported?
  , controlbusy(false)              // Is the Sideband ControlBusy signal supported?
  , control_wdth(1)                 // Width of ControlBusy. User Specified. No default.
  , controlwr(false)                // Is the Sideband ControlWr signal supported?
  , interrupt(false)                // Is the Sideband SInterrupt signal supported?
  , merror(false)                   // Is the Sideband MError signal supported?
  , mflag(false)                    // Is the Sideband MFlag signal supported?
  , mflag_wdth(1)                   // Width of MFlag. User Specified. No default.
  , mreset(false)                   // NOTE: no default. Is the Sideband MReset signal supported?
  , serror(false)                   // Is the Sideband SError signal supported?
  , sflag(false)                    // Is the Sideband SFlag signal supported?
  , sflag_wdth(1)                   // Width of SFlag. User Specified. No default.
  , sreset(false)                   // Note: no default. Is the SReset signal part of the OCP channel?
  , status(false)                   // Is the Sideband Status signal supported?
  , statusbusy(false)               // Is the Sideband StatusBusy signal supported
  , statusrd(false)                 // Is the Sideband StatusRd signal supported?
  , status_wdth(1)                  // Width of Status. User Specified. No default.
  , sthreadbusy_pipelined(false)
  , mthreadbusy_pipelined(false)
  , sdatathreadbusy_pipelined(false)
  , tag_interleave_size(1)
  , enableclk(0)
  , config_state(ocp_unconfigured)
{
}

  // generate a string version in rtl.conf format
std::string OCPIP_VERSION::ocp_parameters::as_rtl_conf(bool show_defaults) const {
  std::ostringstream retval;
  ocp_parameters defaults;



  retval << "interface " << name << " bundle ocp2 revision ";
  retval << int(10*(ocp_version-2)) << " {\n";
  retval << "  interface_type unknown\n";

  if (config_state==ocp_generic) {retval<<"  # The interface is generic, and will adjust itself to what it is connected to."<<std::endl
                                        <<"  # The following parameters cannot be considered valid."<<std::endl;}

  show_param(broadcast_enable)
  show_param(burst_aligned)
  show_param(burstseq_dflt1_enable)
  show_param(burstseq_dflt2_enable)
  show_param(burstseq_incr_enable)
  show_param(burstseq_strm_enable)
  show_param(burstseq_unkn_enable)
  show_param(burstseq_wrap_enable)
  show_param(burstseq_xor_enable)
  show_param(burstseq_blck_enable)
  show_param(endian)
  show_param(force_aligned)
  show_param(mthreadbusy_exact)
  show_param(rdlwrc_enable)
  show_param(read_enable)
  show_param(readex_enable)
  show_param(sdatathreadbusy_exact)
  show_param(sthreadbusy_exact)
  show_param(write_enable)
  show_param(writenonpost_enable)
  show_param(datahandshake)
  show_param(reqdata_together)
  show_param(writeresp_enable)
  show_param(addr)
  show_wdth_param(addr_wdth, addr)
  show_param(addrspace)
  show_wdth_param(addrspace_wdth, addrspace)
  show_param(atomiclength)
  show_wdth_param(atomiclength_wdth, atomiclength)
  show_param(burstlength)
  show_wdth_param(burstlength_wdth, burstlength)
  show_param(blockheight)
  show_wdth_param(blockheight_wdth, blockheight)
  show_param(blockstride)
  show_wdth_param(blockstride_wdth, blockstride)
  show_param(burstprecise)
  show_param(burstseq)
  show_param(burstsinglereq)
  show_param(byteen)
  show_param(cmdaccept)
  show_param(connid)
  show_wdth_param(connid_wdth, connid)
  show_param(dataaccept)
  show_param(datalast)
  show_param(datarowlast)
  show_wdth_param(data_wdth, mdata || sdata)
  show_param(mdata)
  show_param(mdatabyteen)
  show_param(mdatainfo)
  show_wdth_param(mdatainfo_wdth, mdatainfo)
  show_wdth_param(mdatainfobyte_wdth, mdatainfo)
  show_param(sdatathreadbusy)
  show_param(mthreadbusy)
  show_param(reqinfo)
  show_param(reqinfo)
  show_param(reqlast)
  show_param(reqrowlast)
  show_param(resp)
  show_param(respaccept)
  show_param(respinfo)
  show_wdth_param(respinfo_wdth, respinfo)
  show_param(resplast)
  show_param(resprowlast)
  show_param(sdata)
  show_param(sdatainfo)
  show_wdth_param(sdatainfo_wdth, sdatainfo)
  show_wdth_param(sdatainfobyte_wdth, sdatainfo)
  show_param(sthreadbusy)
  show_param(threads)
  show_param(tags)
  show_param(taginorder)
  show_param(control)
  show_param(controlbusy)
  show_param(control)
  show_param(controlwr)
  show_param(interrupt)
  show_param(merror)
  show_param(mflag)
  show_wdth_param(mflag_wdth, mflag)
  show_nodef_param(mreset)
  show_param(serror)
  show_param(sflag)
  show_wdth_param(sflag_wdth, sflag)
  show_nodef_param(sreset)
  show_param(status)
  show_param(statusbusy)
  show_param(statusrd)
  show_wdth_param(status_wdth, status)
  show_param(sthreadbusy_pipelined)
  show_param(mthreadbusy_pipelined)
  show_param(sdatathreadbusy_pipelined)

  retval << "}"<<std::endl;
  return retval.str();
}

#undef show_param
#undef show_wdth_param
#undef show_nodef_param

//this function initializes an ocp parameters set so that it is OSCI compliant
//the argument data_width will be used to configure the data_wdth parameter of
//ocp
void OCPIP_VERSION::ocp_parameters::init_as_osci_config(unsigned int data_width){
  ocp_version=2.0;                // NOTE: currently no way to change this.
  name="osci_connection";     // NOTE: set in "setConfiguration()"
  broadcast_enable=false;         // Is the Broadcast command supported?
  burst_aligned=false;            // Are bursts forced to be aligned by power of two?
  burstseq_dflt1_enable=false;    // Is DFLT1 burst mode enabled?
  burstseq_dflt2_enable=false;    // Is DFLT2 burst mode enabled?
  burstseq_incr_enable=true;      // Are Incrementing Bursts allowed?
  burstseq_strm_enable=true;     // Are Streaming Bursts allowed?
  burstseq_unkn_enable=false;     // Is UNKN burst mode enabled?
  burstseq_wrap_enable=false;     // Is WRAP burst mode enabled?
  burstseq_xor_enable=false;      // Is XOR burst mode enabled?
  burstseq_blck_enable=false;
  endian=1;                       // Endian is an int
  force_aligned=false;            // Forces the byte-enable patterns to be powers of two.
  mthreadbusy_exact=false;        // Slave must use MTheadbusy to send Resp. Master must accept immediately on non-busy threads.
  rdlwrc_enable=false;
  read_enable=true;               // Is the Read command supported?
  readex_enable=false;            // Is the ReadEx command supported?
  sdatathreadbusy_exact=false;    // Master must use SDataThreadBusy to send new Data. Slave must accept new data immediately on non-busy threads.
  sthreadbusy_exact=false;        // Master must use SThreadBusy to send new Request. Slave must accept new request immediately on non-busy threads.
  write_enable=true;              // Is the Write command supported?
  writenonpost_enable=false;      // Is the WriteNonPost command supported?
  datahandshake=false;            // Is there a separate channel for request data?
  reqdata_together=true;         // Master always puts Request and Data in same cycle. Slave always accepts together in same cycle.
  writeresp_enable=true;         // Should Responses be sent for write commands?
  addr=true;                      // Is MAddr (Requst Address) part of the OCP?
  addr_wdth=64;                    // Address width must be set by user if addr=true. No default value.
  addrspace=false;                // Is the MAddrSpace signal part of the OCP?
  addrspace_wdth=1;               // No default value. Must be user specificied.
  atomiclength=false;             // Is there a minimum number of transfers to hold together during a burst?
  atomiclength_wdth=1;            // The minimum number. No default value.
  burstlength=true;              // Is there a set number of transfers in a burst.
  burstlength_wdth=sizeof(unsigned int)*8;             // The number of transfers in a burst. User defined. No default value.
  blockheight=false;
  blockheight_wdth=1;
  blockstride=false;
  blockstride_wdth=1;
  burstprecise=false;             // Will the length of a burst be known at the start of the burst?
  burstseq=false;                 // Is there a sequence of addresses in a burst?
  burstsinglereq=false;           // Is a single request allowed to generate multiple data transfers in a burst?
  byteen=true;                   // Is MByteEn part of the OCP?
  cmdaccept=true;                 // Does slave accept commands?
  connid=false;                   // Is MConnID connection identifier pard of the Request group?
  connid_wdth=1;                  // Width of MConnID. User specified. No default.
  dataaccept=false;               // Does slave accept data handshakes?
  datalast=false;                 // Is the MDataLast burst signal part of the OCP?
  datarowlast=false;
  data_wdth=data_width;
  mdata=true;                     // is MData part of the OCP?
  mdatabyteen=true;              // is the MDataByteEn signal in the OCP?
  mdatainfo=false;                // is the MDataInfo signal in the OCP?
  mdatainfo_wdth=1;               // Width of MDataInfo. User specified. no default.
  mdatainfobyte_wdth=1;           // Number of bits of MDataInfo that are associated with each data byte of MData.
  sdatathreadbusy=false;          // Is SDataThreadBusy part of the OCP channel?
  mthreadbusy=false;              // Is MThreadBusy part of the OCP channel?
  reqinfo=false;                  // Is the MReqInfo signal part of the OCP channel?
  reqinfo_wdth=1;                 // Width of MReqInfo. User Specified. No default.
  reqlast=false;                  // Is the MReqLast burst signal part of the OCP channel?
  reqrowlast=false;
  resp=true;                      // Is the SResp signal part of the OCP channel?
  respaccept=true;               // Does the Master accept Responses with the MRespAccept signal?
  respinfo=false;                 // Is the SRespInfo signal on the OCP?
  respinfo_wdth=1;                // Width of SRespInfo. User Specified. No Default.
  resplast=false;                 // Is the SRespLast burst signal part of the OCP channel?
  resprowlast=false;
  sdata=true;                     // Is the SData signal part of the OCP channel?
  sdatainfo=false;                // Is the SDataInfo signal supported?
  sdatainfo_wdth=1;               // Width of SDataInfo. User Specified. No default.
  sdatainfobyte_wdth=1;           // Number of bits in SDataInfo devoted to each byte of SData. User Specified. No default.
  sthreadbusy=false;              // Is the SThreadBusy signal supported?
  threads=1;                      // Number of threads allowed.
  tags=1;                         // Number of tags allowed.
  taginorder=0;                   // In order required among tags
  control=false;                  // Is the Sideband Control signal supported?
  controlbusy=false;              // Is the Sideband ControlBusy signal supported?
  control_wdth=1;                 // Width of ControlBusy. User Specified. No default.
  controlwr=false;                // Is the Sideband ControlWr signal supported?
  interrupt=false;                // Is the Sideband SInterrupt signal supported?
  merror=false;                   // Is the Sideband MError signal supported?
  mflag=false;                    // Is the Sideband MFlag signal supported?
  mflag_wdth=1;                   // Width of MFlag. User Specified. No default.
  mreset=false;                   // NOTE: no default. Is the Sideband MReset signal supported?
  serror=false;                   // Is the Sideband SError signal supported?
  sflag=false;                    // Is the Sideband SFlag signal supported?
  sflag_wdth=1;                   // Width of SFlag. User Specified. No default.
  sreset=false;                   // Note: no default. Is the SReset signal part of the OCP channel?
  status=false;                   // Is the Sideband Status signal supported?
  statusbusy=false;               // Is the Sideband StatusBusy signal supported
  statusrd=false;                 // Is the Sideband StatusRd signal supported?
  status_wdth=1;                  // Width of Status. User Specified. No default.
  sthreadbusy_pipelined=false;
  mthreadbusy_pipelined=false;
  sdatathreadbusy_pipelined=false;
  tag_interleave_size=1;
  enableclk=0;
  config_state=ocp_configured;
}

#define DIFF_CHECK(name) if ((name)!=(other.name)) {return true;}
//this function compares ocp parameter set provided as the function argument to
// the ocp parameters set on which the function is called
// It returns true if there is a difference
bool OCPIP_VERSION::ocp_parameters::diff(const ocp_parameters& other){
  DIFF_CHECK(ocp_version);
  DIFF_CHECK(broadcast_enable);
  DIFF_CHECK(burst_aligned);
  DIFF_CHECK(burstseq_dflt1_enable);
  DIFF_CHECK(burstseq_dflt2_enable);
  DIFF_CHECK(burstseq_incr_enable);
  DIFF_CHECK(burstseq_strm_enable);
  DIFF_CHECK(burstseq_unkn_enable);
  DIFF_CHECK(burstseq_wrap_enable);
  DIFF_CHECK(burstseq_xor_enable);
  DIFF_CHECK(burstseq_blck_enable);
  DIFF_CHECK(endian);
  DIFF_CHECK(force_aligned);
  DIFF_CHECK(mthreadbusy_exact);
  DIFF_CHECK(rdlwrc_enable);
  DIFF_CHECK(read_enable);
  DIFF_CHECK(readex_enable);
  DIFF_CHECK(sdatathreadbusy_exact);
  DIFF_CHECK(sthreadbusy_exact);
  DIFF_CHECK(write_enable);
  DIFF_CHECK(writenonpost_enable);
  DIFF_CHECK(datahandshake);
  DIFF_CHECK(reqdata_together);
  DIFF_CHECK(writeresp_enable);
  DIFF_CHECK(addr);
  DIFF_CHECK(addr_wdth);
  DIFF_CHECK(addrspace);
  DIFF_CHECK(addrspace_wdth);
  DIFF_CHECK(atomiclength);
  DIFF_CHECK(atomiclength_wdth);
  DIFF_CHECK(burstlength);
  DIFF_CHECK(burstlength_wdth);
  DIFF_CHECK(blockheight);
  DIFF_CHECK(blockheight_wdth);
  DIFF_CHECK(blockstride);
  DIFF_CHECK(blockstride_wdth);
  DIFF_CHECK(burstprecise);
  DIFF_CHECK(burstseq);
  DIFF_CHECK(burstsinglereq);
  DIFF_CHECK(byteen);
  DIFF_CHECK(cmdaccept);
  DIFF_CHECK(connid);
  DIFF_CHECK(connid_wdth);
  DIFF_CHECK(dataaccept);
  DIFF_CHECK(datalast);
  DIFF_CHECK(datarowlast);
  DIFF_CHECK(data_wdth);
  DIFF_CHECK(mdata);
  DIFF_CHECK(mdatabyteen);
  DIFF_CHECK(mdatainfo);
  DIFF_CHECK(mdatainfo_wdth);
  DIFF_CHECK(mdatainfobyte_wdth);
  DIFF_CHECK(sdatathreadbusy);
  DIFF_CHECK(mthreadbusy);
  DIFF_CHECK(reqinfo);
  DIFF_CHECK(reqinfo_wdth);
  DIFF_CHECK(reqlast);
  DIFF_CHECK(reqrowlast);
  DIFF_CHECK(resp);
  DIFF_CHECK(respaccept);
  DIFF_CHECK(respinfo);
  DIFF_CHECK(respinfo_wdth);
  DIFF_CHECK(resplast);
  DIFF_CHECK(resprowlast);
  DIFF_CHECK(sdata);
  DIFF_CHECK(sdatainfo);
  DIFF_CHECK(sdatainfo_wdth);
  DIFF_CHECK(sdatainfobyte_wdth);
  DIFF_CHECK(sthreadbusy);
  DIFF_CHECK(threads);
  DIFF_CHECK(tags);
  DIFF_CHECK(taginorder);
  DIFF_CHECK(control);
  DIFF_CHECK(controlbusy);
  DIFF_CHECK(control_wdth);
  DIFF_CHECK(controlwr);
  DIFF_CHECK(interrupt);
  DIFF_CHECK(merror);
  DIFF_CHECK(mflag);
  DIFF_CHECK(mflag_wdth);
  DIFF_CHECK(mreset);
  DIFF_CHECK(serror);
  DIFF_CHECK(sflag);
  DIFF_CHECK(sflag_wdth);
  DIFF_CHECK(sreset);
  DIFF_CHECK(status);
  DIFF_CHECK(statusbusy);
  DIFF_CHECK(statusrd);
  DIFF_CHECK(status_wdth);
  DIFF_CHECK(sthreadbusy_pipelined);
  DIFF_CHECK(mthreadbusy_pipelined);
  DIFF_CHECK(sdatathreadbusy_pipelined);
  DIFF_CHECK(tag_interleave_size);
  DIFF_CHECK(enableclk);
  //DIFF_CHECK(config_state);
  return false;
}
#undef DIFF_CHECK

#define COPY_FROM_OTHER(name) name=other.name
OCPIP_VERSION::ocp_parameters&
  OCPIP_VERSION::ocp_parameters::operator=(const ocp_parameters& other)
{
  COPY_FROM_OTHER(ocp_version);
  COPY_FROM_OTHER(name);
  COPY_FROM_OTHER(broadcast_enable);
  COPY_FROM_OTHER(burst_aligned);
  COPY_FROM_OTHER(burstseq_dflt1_enable);
  COPY_FROM_OTHER(burstseq_dflt2_enable);
  COPY_FROM_OTHER(burstseq_incr_enable);
  COPY_FROM_OTHER(burstseq_strm_enable);
  COPY_FROM_OTHER(burstseq_unkn_enable);
  COPY_FROM_OTHER(burstseq_wrap_enable);
  COPY_FROM_OTHER(burstseq_xor_enable);
  COPY_FROM_OTHER(burstseq_blck_enable);
  COPY_FROM_OTHER(endian);
  COPY_FROM_OTHER(force_aligned);
  COPY_FROM_OTHER(mthreadbusy_exact);
  COPY_FROM_OTHER(rdlwrc_enable);
  COPY_FROM_OTHER(read_enable);
  COPY_FROM_OTHER(readex_enable);
  COPY_FROM_OTHER(sdatathreadbusy_exact);
  COPY_FROM_OTHER(sthreadbusy_exact);
  COPY_FROM_OTHER(write_enable);
  COPY_FROM_OTHER(writenonpost_enable);
  COPY_FROM_OTHER(datahandshake);
  COPY_FROM_OTHER(reqdata_together);
  COPY_FROM_OTHER(writeresp_enable);
  COPY_FROM_OTHER(addr);
  COPY_FROM_OTHER(addr_wdth);
  COPY_FROM_OTHER(addrspace);
  COPY_FROM_OTHER(addrspace_wdth);
  COPY_FROM_OTHER(atomiclength);
  COPY_FROM_OTHER(atomiclength_wdth);
  COPY_FROM_OTHER(burstlength);
  COPY_FROM_OTHER(burstlength_wdth);
  COPY_FROM_OTHER(blockheight);
  COPY_FROM_OTHER(blockheight_wdth);
  COPY_FROM_OTHER(blockstride);
  COPY_FROM_OTHER(blockstride_wdth);
  COPY_FROM_OTHER(burstprecise);
  COPY_FROM_OTHER(burstseq);
  COPY_FROM_OTHER(burstsinglereq);
  COPY_FROM_OTHER(byteen);
  COPY_FROM_OTHER(cmdaccept);
  COPY_FROM_OTHER(connid);
  COPY_FROM_OTHER(connid_wdth);
  COPY_FROM_OTHER(dataaccept);
  COPY_FROM_OTHER(datalast);
  COPY_FROM_OTHER(datarowlast);
  COPY_FROM_OTHER(data_wdth);
  COPY_FROM_OTHER(mdata);
  COPY_FROM_OTHER(mdatabyteen);
  COPY_FROM_OTHER(mdatainfo);
  COPY_FROM_OTHER(mdatainfo_wdth);
  COPY_FROM_OTHER(mdatainfobyte_wdth);
  COPY_FROM_OTHER(sdatathreadbusy);
  COPY_FROM_OTHER(mthreadbusy);
  COPY_FROM_OTHER(reqinfo);
  COPY_FROM_OTHER(reqinfo_wdth);
  COPY_FROM_OTHER(reqlast);
  COPY_FROM_OTHER(reqrowlast);
  COPY_FROM_OTHER(resp);
  COPY_FROM_OTHER(respaccept);
  COPY_FROM_OTHER(respinfo);
  COPY_FROM_OTHER(respinfo_wdth);
  COPY_FROM_OTHER(resplast);
  COPY_FROM_OTHER(resprowlast);
  COPY_FROM_OTHER(sdata);
  COPY_FROM_OTHER(sdatainfo);
  COPY_FROM_OTHER(sdatainfo_wdth);
  COPY_FROM_OTHER(sdatainfobyte_wdth);
  COPY_FROM_OTHER(sthreadbusy);
  COPY_FROM_OTHER(threads);
  COPY_FROM_OTHER(tags);
  COPY_FROM_OTHER(taginorder);
  COPY_FROM_OTHER(control);
  COPY_FROM_OTHER(controlbusy);
  COPY_FROM_OTHER(control_wdth);
  COPY_FROM_OTHER(controlwr);
  COPY_FROM_OTHER(interrupt);
  COPY_FROM_OTHER(merror);
  COPY_FROM_OTHER(mflag);
  COPY_FROM_OTHER(mflag_wdth);
  COPY_FROM_OTHER(mreset);
  COPY_FROM_OTHER(serror);
  COPY_FROM_OTHER(sflag);
  COPY_FROM_OTHER(sflag_wdth);
  COPY_FROM_OTHER(sreset);
  COPY_FROM_OTHER(status);
  COPY_FROM_OTHER(statusbusy);
  COPY_FROM_OTHER(statusrd);
  COPY_FROM_OTHER(status_wdth);
  COPY_FROM_OTHER(sthreadbusy_pipelined);
  COPY_FROM_OTHER(mthreadbusy_pipelined);
  COPY_FROM_OTHER(sdatathreadbusy_pipelined);
  COPY_FROM_OTHER(tag_interleave_size);
  COPY_FROM_OTHER(enableclk);
  COPY_FROM_OTHER(config_state);
  return *this;
}
#undef COPY_FROM_OTHER

