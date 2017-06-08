///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// (c) Copyright OCP-IP 2008,2009
// OCP-IP Confidential and Proprietary
//
//
//============================================================================
//      Project : OCP SLD WG
//       Author : Herve Alexanian
//
//          $Id:
//
//  Description :  This file contains the adapter for legacy TL2 masters.
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_tl2_master_legacy_adapter.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{

template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH>
class ocp_tl2_master_legacy_adapter
  : public sc_core::sc_module
  , public ocpip_legacy::OCP_TL_Config_Listener
{
    typedef ocp_data_class_unsigned<BUSWIDTH, ADDRWIDTH> DataCl;
    typedef typename DataCl::DataType Td;
    typedef typename DataCl::AddrType Ta;
    typedef ocpip_legacy::OCPTL2RequestGrp<Td, Ta>  legacy_req;
    typedef ocpip_legacy::OCPTL2ResponseGrp<Td>     legacy_resp;

  struct my_lock_object: public lock_object_base
  {
  private:
    friend class ocp_extension_pool<my_lock_object>;

    void atomic_txn_completed(){
      get_pool()->recycle(this);
    }

    my_lock_object(){
      lock_object_base::number_of_txns=2;
    }

  public:
    static ocp_extension_pool<my_lock_object>* get_pool(){
      static ocp_extension_pool<my_lock_object> s_pool(5);
      return &s_pool;
    }
  };

public:
  std::string m_master_name, m_slave_name;

  ocpip_legacy::OCP_TL2_SlavePort<Td, Ta>      slave_port;
  ocp_master_socket_tl2<BUSWIDTH> master_socket;

  SC_HAS_PROCESS(ocp_tl2_master_legacy_adapter);

  ocp_tl2_master_legacy_adapter(sc_core::sc_module_name name, unsigned int max_impr_burst_length=64)
    : m_master_name(std::string(name)+"_master_socket")
    , m_slave_name(std::string(name)+"_slave_port")
    , slave_port(m_slave_name.c_str())
    , master_socket(m_master_name.c_str()
		    , ocp_master_socket_tl2<BUSWIDTH>::mm_txn_with_be_and_data())
    , m_invariant_ext_pool(5)
    , m_position_ext_pool(5)
    , m_get_param_called(false)
    , m_max_impr_burst(max_impr_burst_length)
  {
    SC_METHOD(req_handler);
    sensitive<<slave_port.RequestStartEvent();
    dont_initialize();

    SC_METHOD(resp_end_handler);
    sensitive<<slave_port.ResponseEndEvent();
    dont_initialize();

    SC_METHOD(mthreadbusy_handler);
    sensitive<<slave_port.MThreadBusyEvent();
    dont_initialize();

    SC_METHOD(mtiming_handler);
    sensitive<<slave_port.MasterTimingEvent();

    master_socket.register_nb_transport_bw(this, &ocp_tl2_master_legacy_adapter::nb_transport);
    master_socket.register_configuration_listener_callback(this, &ocp_tl2_master_legacy_adapter::socket_config_callback);
    master_socket.make_generic();

    m_tb_txn=master_socket.get_tb_transaction();
    m_rtb_ph=RESP_THREAD_BUSY_CHANGE;
    m_timing_ph=TL2_TIMING_CHANGE;
    m_rtb=master_socket.template get_extension<resp_thread_busy>(*m_tb_txn);
  }

  ~ocp_tl2_master_legacy_adapter(){

  }

  void start_of_simulation(){
    m_params = master_socket.get_ocp_config();
    int threads = m_params.threads;
    int tags    = m_params.tags;
    m_lock_objects.resize(threads);
    m_burst_tracker.resize( threads );
    m_fw_txn.resize( threads );
    for ( int t=0; t<threads; ++t ) {
	m_burst_tracker[t].resize( tags, ocp_txn_track( &m_params, false ) );
	m_fw_txn[t].resize( tags, static_cast<tlm::tlm_generic_payload*>( NULL ) );
    }
  }

  //callback shall merge configs of socket and port
  ocpip_legacy::ParamCl<DataCl>& get_params(){
    if (m_get_param_called) return m_legacy_config;
    //std::cout<<"Checking configs"<<std::endl;
    ocpip_legacy::MapStringType config_map;
    ocpip_legacy::Config_manager::rebuildMap(m_legacy_config, config_map);
    ocp_parameters params;
    params=create_ocp_configuration_from_map(sc_module::name(), config_map);
    //check if the configs match (should be otherwise someone else would have complained...)
    master_socket.check_against_ocp_conf( ocp_master_socket_tl2<BUSWIDTH>::ocp_config_support_type::OCP_SLV
                                        , master_socket.get_resolved_ocp_config()
                                        , params
                                        , params);
    m_get_param_called=true;
    return m_legacy_config;
  }

  //new kit callbacks
  void socket_config_callback(const ocp_parameters& params, const std::string&){
    //std::cout<<"Config callback from TLM2 side"<<std::endl;
    if (params.config_state==ocp_generic){
//      std::cout<<"  Still generic. Not setting channel config"<<std::endl;
      return;
    }
    ocpip_legacy::MapStringType config_map;
    ocpip_legacy::Config_manager::rebuildMap(params.template legacy_conversion<DataCl>(), config_map);
    slave_port->setOCPSlaveConfiguration(config_map);
  }

  void set_configuration(ocpip_legacy::OCPParameters& passedConfig, std::string channelName){
//    std::cout<<"Config callback from legacy side"<<std::endl;
    if (!passedConfig.configured){
//      std::cout<<"  still generic, doing nothing"<<std::endl;
      return;
    }
    ocpip_legacy::MapStringType config_map;
    ocpip_legacy::Config_manager::rebuildMap(passedConfig, config_map);
    ocp_parameters params;
    params.init_from_legacy(passedConfig);

    //remember how the legacy channel is configured
    m_legacy_config.setOCPConfiguration(master_socket.name(), config_map);

    master_socket.set_ocp_config(params);
  }

  uint32_t get_thread( legacy_req req ) {
    return ( m_params.threads > 1 ) ? req.MThreadID: 0;
  }
  uint32_t get_tag( legacy_req req ) {
    return ( m_params.tags > 1 ) ? req.MTagID: 0;
  }
  uint32_t get_thread( legacy_resp resp ) {
    return ( m_params.threads > 1 ) ? resp.SThreadID: 0;
  }
  uint32_t get_tag( legacy_resp resp ) {
    return ( m_params.tags > 1 ) ? resp.STagID: 0;
  }

  tlm::tlm_generic_payload* new_txn( const legacy_req& req ) {
      tlm::tlm_generic_payload* p_txn = master_socket.get_transaction();
      assert( p_txn!=NULL );
      tlm::tlm_generic_payload& txn = *p_txn;

      // hang burst position extension
      ocp_txn_position* p_pos = m_position_ext_pool.create();
      acc(txn).set_extension( p_pos );
      p_pos->req_position .count  = 0;
      p_pos->dh_position  .count  = 0;
      p_pos->resp_position.count  = 0;
      p_pos->req_position .remain = -1;
      p_pos->dh_position  .remain = -1;
      p_pos->resp_position.remain = -1;

      lock_object_base* lock_object=NULL;
      if (req.MCmd==ocpip_legacy::OCP_MCMD_WR || req.MCmd==ocpip_legacy::OCP_MCMD_WRNP){
	  lock_object=m_lock_objects[req.MThreadID];
	  m_lock_objects[req.MThreadID]=NULL; //rdex wr(np) pair is done for this thread
      }
      if (!req.MBurstPrecise)
	  master_socket.reserve_data_size(txn, ((BUSWIDTH+7)>>3)*m_max_impr_burst*req.MBlockHeight);
      else
	  master_socket.reserve_data_size(txn, ((BUSWIDTH+7)>>3)*req.MBurstLength*req.MBlockHeight);
      //txn.set_data_length(((BUSWIDTH+7)>>3)*req.MBurstLength+m_converter.get_diff());
      if (m_params.byteen | m_params.mdatabyteen){
	  if (req.MBurstSingleReq && !m_params.mdatabyteen) //SRMD and dhs has no byte en: We have just as single BE pattern
	      master_socket.reserve_be_size(txn,   ((BUSWIDTH+7)>>3));
	  else{ //we must expect a different be pattern per word
	      if (!req.MBurstPrecise)
		  master_socket.reserve_be_size(txn,   ((BUSWIDTH+7)>>3)*m_max_impr_burst*req.MBlockHeight);
	      else
		  master_socket.reserve_be_size( txn,   ((BUSWIDTH+7)>>3)*req.MBurstLength*req.MBlockHeight);
	  }
	  //txn.set_byte_enable_length(((BUSWIDTH+7)>>3)*req.MBurstLength+m_converter.get_diff());
      }
      else{
	  txn.set_byte_enable_ptr(NULL);
	  txn.set_byte_enable_length(0);
      }
      if (req.MCmd==ocpip_legacy::OCP_MCMD_RDEX) {
	  assert(m_lock_objects[req.MThreadID]==NULL);
	  uint32_t thread = get_thread( req );
	  m_lock_objects[thread]=my_lock_object::get_pool()->create();
	  lock_object=m_lock_objects[thread];
      }

      switch(req.MCmd) {
      case ocpip_legacy::OCP_MCMD_IDLE: txn.set_command(tlm::TLM_IGNORE_COMMAND); break;
      case ocpip_legacy::OCP_MCMD_WR:
	  txn.set_command(tlm::TLM_WRITE_COMMAND);
      master_socket.template validate_extension<posted>(txn);
	  if (lock_object){
	      lock* lck;
	      master_socket.template get_extension<lock>(lck, txn);
	      lck->value=lock_object;
	      master_socket.template validate_extension<lock>(txn);
	  }
	  break;
      case ocpip_legacy::OCP_MCMD_RD:   txn.set_command(tlm::TLM_READ_COMMAND); break;
      case ocpip_legacy::OCP_MCMD_RDEX:
	  txn.set_command(tlm::TLM_READ_COMMAND);
	  lock* lck;
	  master_socket.template get_extension<lock>(lck, txn);
	  lck->value=lock_object;
	  master_socket.template validate_extension<lock>(txn);
	  break;
      case ocpip_legacy::OCP_MCMD_RDL:
	  txn.set_command(tlm::TLM_READ_COMMAND);
	  master_socket.template validate_extension<semaphore>(txn);
	  break;
      case ocpip_legacy::OCP_MCMD_WRNP:
	  txn.set_command(tlm::TLM_WRITE_COMMAND);
	  //master_socket.template validate_extension<nonposted>(txn);
	  if (lock_object){
	      lock* lck;
	      master_socket.template get_extension<lock>(lck, txn);
	      lck->value=lock_object;
	      master_socket.template validate_extension<lock>(txn);
	  }
	  break;
      case ocpip_legacy::OCP_MCMD_WRC:{
	  txn.set_command(tlm::TLM_WRITE_COMMAND);
	  semaphore* sem;
	  master_socket.template get_extension<semaphore>(sem, txn);
	  sem->value=false;
	  master_socket.template validate_extension<semaphore>(txn);
      }
	  break;
      case ocpip_legacy::OCP_MCMD_BCST:
	  txn.set_command(tlm::TLM_WRITE_COMMAND);
	  master_socket.template validate_extension<broadcast>(txn);
	  break;
      }

      txn.set_streaming_width(txn.get_data_length()); //assume no streaming
      txn.set_address(req.MAddr); //assume a INCR addressing scheme
      if ( txn.is_write() ) {
	  // TODO: this is too simplistic. Depends on the burstseq and host endianness
	  memcpy( txn.get_data_ptr(), req.MDataPtr, req.DataLength * m_byte_width );
      }
      if ( txn.get_byte_enable_ptr() != NULL ) {
	  if ( req.MByteEnPtr != NULL ) {
	      memcpy( txn.get_byte_enable_ptr(), req.MByteEnPtr, req.DataLength );
	  } else {
	      for ( uint32_t i = 0; i<txn.get_byte_enable_length(); ++i ) {
		  *(txn.get_byte_enable_ptr() + i ) = req.MByteEn;
	      }
	  }
      }

      if (m_params.burstseq){
	  burst_sequence* b_seq;
	  master_socket.template get_extension<burst_sequence>(b_seq, txn);
	  b_seq->value.sequence=(burst_seqs)req.MBurstSeq;
	  master_socket.template validate_extension<burst_sequence>(txn);

	  switch (req.MBurstSeq){
	  case ocpip_legacy::OCP_MBURSTSEQ_BLCK:
	      b_seq->value.block_height=req.MBlockHeight;
	      b_seq->value.block_stride=req.MBlockStride;
	      b_seq->value.blck_row_length_in_bytes=m_byte_width*req.MBurstLength;
	      break;
	  case ocpip_legacy::OCP_MBURSTSEQ_DFLT1:
	  case ocpip_legacy::OCP_MBURSTSEQ_DFLT2:
	  case ocpip_legacy::OCP_MBURSTSEQ_UNKN:
	      if (b_seq->value.unkn_dflt_addresses.size()<1) b_seq->value.unkn_dflt_addresses.resize(req.MBurstLength);
	      b_seq->value.unkn_dflt_addresses[0]=req.MAddr;
	      b_seq->value.unkn_dflt_bytes_per_address=m_byte_width;
	      b_seq->value.unkn_dflt_addresses_valid=true;
	      break;
	  case ocpip_legacy::OCP_MBURSTSEQ_WRAP:
	  case ocpip_legacy::OCP_MBURSTSEQ_XOR:
	      b_seq->value.xor_wrap_address=req.MAddr;
	      //TODO: calc sequence txn.set_address(have_seen->data_seq_calc.get_low_addr());
	  case ocpip_legacy::OCP_MBURSTSEQ_INCR:
	      break;
	  case ocpip_legacy::OCP_MBURSTSEQ_STRM:
	      txn.set_streaming_width(m_byte_width);
	      break;
	  case ocpip_legacy::OCP_MBURSTSEQ_RESERVED:
	      assert(0); exit(1);
	  }
      }

      if (m_params.addrspace)  {
	  address_space* addr_sp;
	  master_socket.get_extension(addr_sp, txn);
	  addr_sp->value=req.MAddrSpace;
	  master_socket.template validate_extension<address_space>(txn);
      }

//     if (m_params.byteen){
//       set_BE_into_txn(req.MByteEn, &txn, 0);
//     }
      if (m_params.threads>1){
	  thread_id* th_id;
	  master_socket.template get_extension<thread_id>(th_id, txn);
	  th_id->value=req.MThreadID;
	  master_socket.template validate_extension<thread_id>(txn);
      }
      if (m_params.connid){
	  conn_id* cn_id;
	  master_socket.template get_extension<conn_id>(cn_id, txn);
	  cn_id->value=req.MConnID;
	  master_socket.template validate_extension<conn_id>(txn);
      }
      if (m_params.tags>1){
	  tag_id* tg_id;
	  master_socket.template get_extension<tag_id>(tg_id, txn);
	  tg_id->value=req.MTagID;
	  master_socket.template validate_extension<tag_id>(txn);  //we have a tag, we set it and mark it as valid
      }
      if (m_params.taginorder && req.MTagInOrder)
	  master_socket.template invalidate_extension<tag_id>(txn); //tags are in order? so the tag becomes don't care and we invalidate it
      if (m_params.burstlength){
	  burst_length* b_len;
	  master_socket.template get_extension<burst_length>(b_len, txn);
	  b_len->value=req.MBurstLength;
	  master_socket.template validate_extension<burst_length>(txn);
      }
      if (m_params.burstprecise && !req.MBurstPrecise){
	  master_socket.template validate_extension<imprecise>(txn);
      }

//     if (req.HasMData && req.MCmd==ocpip_legacy::OCP_MCMD_WR){
//       de_serialize<Td,true,false,&my_type::put_word_into_txn>(req.MBurstSeq,txn, have_seen, req.MData, 1); //cannot happen for SRMD
//     }
      if (req.MBurstSingleReq && m_params.burstsinglereq)
	master_socket.template validate_extension<srmd>(txn);

//     //MReqLast is derivable information
//     have_seen->req_cnt=1;
//     have_seen->dhs_cnt=0;
//     have_seen->rsp_cnt=0;

//     //we have set addr, cmd and streaming width
//     // the caller must have set data_ptr, data_length, byte_enable_ptr and byte_enable_length
//     // so we still need resp and dmi
      txn.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
      txn.set_dmi_allowed(false);

      word_count* word_count_info;
      master_socket.template get_extension<word_count>(word_count_info, txn);
      word_count_info->value.request_wc = req.DataLength;
      master_socket.template validate_extension<word_count>( txn );
      if ( txn.is_write() && m_params.datahandshake ) {
	  word_count_info->value.data_wc = req.DataLength;
      }

      ocp_txn_burst_invariant* p_inv = m_invariant_ext_pool.create();
      *p_inv = ocp_txn_burst_invariant::init_from( txn, m_params, master_socket );
      acc(txn).set_extension( p_inv );
      return p_txn;
  }

    void continuing_txn( tlm::tlm_generic_payload& txn, const legacy_req& req ) {
	ocp_txn_burst_invariant *p_inv;
	acc(txn).get_extension( p_inv );
	assert( p_inv != NULL );

	// update data array
//     if (req.HasMData && req.MCmd==ocpip_legacy::OCP_MCMD_WR){
//       de_serialize<Td,true,false,&my_type::put_word_into_txn>(req.MBurstSeq,txn, have_seen, req.MData, 1); //cannot happen for SRMD
//     }

//     if (m_params.byteen){
//       set_BE_into_txn(req.MByteEn, &txn, 0);
//     }

	word_count* word_count_info;
	master_socket.template get_extension<word_count>(word_count_info, txn);
	word_count_info->value.request_wc = req.DataLength;
	master_socket.template validate_extension<word_count>( txn );
	if ( txn.is_write() && m_params.datahandshake ) {
	    word_count_info->value.data_wc = req.DataLength;
	}
    }

  void req_handler() {
      legacy_req req;
      slave_port->getOCPRequest(req);
      uint32_t thread = get_thread( req );
      uint32_t tag    = get_tag   ( req );
      ocp_txn_track& track = m_burst_tracker[thread][tag];
      if ( !track.has_pending_request() ) {
	  // this is a new transaction
	  m_fw_txn[thread][tag] = new_txn( req );
      } else {
	  assert( m_fw_txn[thread][tag] != NULL );
	  continuing_txn( *m_fw_txn[thread][tag], req );
      }
      tlm::tlm_generic_payload& txn =  *m_fw_txn[thread][tag];
      m_req_txn = &txn;

      // Send request and data phases
      ocp_txn_position * p_pos;
      acc(txn).get_extension(p_pos);
      assert(p_pos);

      m_txn_time=sc_core::SC_ZERO_TIME;
      m_txn_ph=tlm::BEGIN_REQ;
      try {
	  p_pos->req_position = track.track_phase(txn, tlm::BEGIN_REQ);
      } catch ( const ocp_txn_track_error& err ) {
	  SC_REPORT_ERROR( "Legacy request", err.what() );
      }
      switch (master_socket->nb_transport_fw(txn, m_txn_ph, m_txn_time)) {
      case tlm::TLM_ACCEPTED : break;
      case tlm::TLM_UPDATED  :
	  assert(m_txn_ph==tlm::END_REQ);
	  if ( !txn.is_write() || !m_params.datahandshake )
	      finish_req();
	  break;
      case tlm::TLM_COMPLETED: assert(0);
      }
      if ( txn.is_write() && m_params.datahandshake ) {
	  m_txn_ph=BEGIN_DATA;
	  try {
	      p_pos->dh_position = track.track_phase(txn, BEGIN_DATA);
	  } catch ( const ocp_txn_track_error& err ) {
	      SC_REPORT_ERROR( "Legacy request", err.what() );
	  }
	  switch (master_socket->nb_transport_fw(txn, m_txn_ph, m_txn_time)) {
	  case tlm::TLM_ACCEPTED : break;
	  case tlm::TLM_UPDATED  :
	      assert(m_txn_ph==END_DATA);
	      finish_req();
	      break;
	  case tlm::TLM_COMPLETED: assert(0);
	  }
      }
  }

  bool is_txn_complete( tlm::tlm_generic_payload& txn ) {
      ocp_txn_position * p_pos;
      acc(txn).get_extension(p_pos);
      assert(p_pos);
      return ( p_pos->req_position .remain == 0 &&
	       p_pos->dh_position  .remain == 0 &&
	       p_pos->resp_position.remain == 0 );
  }

  void finish_req(){
    slave_port->acceptRequest();
    assert( m_req_txn != NULL );
    if ( is_txn_complete( *m_req_txn ) ) {
	master_socket.release_transaction(m_req_txn);
    }
    m_req_txn = NULL;
  }

  legacy_resp resp_handler( tlm::tlm_generic_payload& txn ) {
      ocp_txn_burst_invariant* p_inv;
      acc(txn).get_extension(p_inv);
      assert(p_inv);
      ocp_txn_position * p_pos;
      acc(txn).get_extension(p_pos);
      assert(p_pos);
      uint32_t thread = p_inv->threadid;
      uint32_t tag    = p_inv->tagid;
      assert( thread < m_burst_tracker.size() );
      assert( tag    < m_burst_tracker[thread].size() );
      ocp_txn_track& track = m_burst_tracker[thread][tag];
      try {
	  p_pos->resp_position = track.track_phase(txn, tlm::BEGIN_RESP);
      } catch ( const ocp_txn_track_error& err ) {
	  SC_REPORT_ERROR( "TLM response", err.what() );
      }

      legacy_resp resp;
      resp.SThreadID = thread;
      resp.STagID    = tag;
      resp.SDataInfo = 0;
      resp.SRespInfo = 0;
      switch(txn.get_response_status())
      {
      case tlm::TLM_OK_RESPONSE: resp.SResp=ocpip_legacy::OCP_SRESP_DVA; break;
      case tlm::TLM_INCOMPLETE_RESPONSE: resp.SResp=ocpip_legacy::OCP_SRESP_NULL; break;
      default: resp.SResp=ocpip_legacy::OCP_SRESP_ERR; break;
      }
      if (p_inv->cmd==ocpip_legacy::OCP_MCMD_WRC){
	  semaphore* sem;
	  master_socket.template get_extension<semaphore>(sem, txn);
	  if (sem->value==false) resp.SResp=ocpip_legacy::OCP_SRESP_FAIL;
      }

      // DataLength
      word_count* word_count_info = NULL;
      bool has_word_count= master_socket.get_extension(word_count_info, txn);
      uint32_t resp_word_count = has_word_count ?
	  word_count_info->value.response_wc : p_inv->burst_length;
      if ( txn.is_write() && p_inv->srmd ) {
	  resp_word_count = 1;
      }
      resp.DataLength = resp_word_count;
      resp.LastOfBurst = p_pos->resp_position.remain==0;
      resp.LastOfRow   = p_pos->resp_position.row_remain==0;

      // This is incorrect
      resp.SDataPtr = (Td*)txn.get_data_ptr();

      return resp;
  }

  void resp_end_handler(){
    if (m_params.respaccept){
      m_txn_ph=tlm::END_RESP;
      m_txn_time=sc_core::SC_ZERO_TIME;
      assert(m_rsp_txn);
      master_socket->nb_transport_fw(*m_rsp_txn, m_txn_ph, m_txn_time);
      if ( is_txn_complete( *m_rsp_txn ) ) {
         master_socket.release_transaction(m_rsp_txn);
         m_rsp_txn=NULL;
       }
    }
  }

  void mthreadbusy_handler(){
    m_rtb->value=slave_port->getMThreadBusy();
    m_txn_time=sc_core::SC_ZERO_TIME;
    master_socket->nb_transport_fw(*m_tb_txn, m_rtb_ph, m_txn_time);
  }

  void mtiming_handler() {
      ocpip_legacy::MTimingGrp m_timing;
      slave_port->getMasterTiming(m_timing);
      tlm::tlm_generic_payload* p_timing_txn = master_socket.get_tl2_timing_transaction();
      tl2_timing*        p_timing_ext = master_socket.template get_extension<tl2_timing>(*p_timing_txn);
      p_timing_ext->value.type = MASTER_TIMING;
      p_timing_ext->value.master_timing.RqSndI = m_timing.RqSndI;
      p_timing_ext->value.master_timing.DSndI  = m_timing.DSndI;
      p_timing_ext->value.master_timing.RpAL   = m_timing.RpAL;
      m_txn_time=sc_core::SC_ZERO_TIME;
      master_socket->nb_transport_fw(*p_timing_txn, m_timing_ph, m_txn_time);
  }

  tlm::tlm_sync_enum nb_transport(tlm::tlm_generic_payload& txn, tlm::tlm_phase& ph, sc_core::sc_time& time){
    switch (ph){
      case tlm::END_REQ:
        assert(m_req_txn==&txn);
	if ( !m_req_txn->is_write() || !m_params.datahandshake ) {
	    finish_req();
	} else {
	    // will finish when we get the END_DATA
	}
        break;
      case tlm::BEGIN_RESP:{
	  legacy_resp resp = resp_handler(txn);
	  slave_port->sendOCPResponse(resp);
	  m_rsp_txn=&txn;
	  if (!m_params.respaccept) {
	      ph=tlm::END_RESP;
	      if ( is_txn_complete( *m_rsp_txn ) ) {
		  master_socket.release_transaction(m_rsp_txn);
		  m_rsp_txn=NULL;
	      }
	      return tlm::TLM_UPDATED;
	  }
	  break;
      }
      default:
        if (ph==END_DATA) {
	    assert(m_req_txn==&txn);
	    assert(m_req_txn->is_write() && m_params.datahandshake );
	    finish_req();
        }
        else
        if (ph==CMD_THREAD_BUSY_CHANGE){
          cmd_thread_busy* tb=master_socket.template get_extension<cmd_thread_busy>(txn);
	  slave_port->putSThreadBusy(tb->value);
        }
	else if (ph==TL2_TIMING_CHANGE){
	    tl2_timing* p_timing=master_socket.template get_extension<tl2_timing>(txn);
	    ocpip_legacy::STimingGrp s_timing;
	    s_timing.RqAL   = p_timing->value.slave_timing.RqAL;
	    s_timing.DAL    = p_timing->value.slave_timing.DAL;
	    s_timing.RpSndI = p_timing->value.slave_timing.RpSndI;
	    slave_port->putSlaveTiming( s_timing );
	}
#ifndef NDEBUG
        else{
          std::stringstream s;
          s<<"Ignorable phase detected: "<<ph;
          SC_REPORT_WARNING(name(), s.str().c_str());
        }
#endif

    }
    return tlm::TLM_ACCEPTED;
  }

private:
  tlm_utils::instance_specific_extension_accessor acc;
  ocp_extension_pool<ocp_txn_burst_invariant> m_invariant_ext_pool;
  ocp_extension_pool<ocp_txn_position>        m_position_ext_pool;

protected:

  ocpip_legacy::ParamCl<DataCl> m_legacy_config;
  bool m_get_param_called, m_next_req_expected;
  ocp_parameters                m_params;
  // the transactions being assembled on the forward path. Can be interleaved
  // hence per thread/per tag structure
  std::vector<std::vector<tlm::tlm_generic_payload*> > m_fw_txn;
  // valid during the request phase, one at a time
  tlm::tlm_generic_payload* m_req_txn;
  tlm::tlm_generic_payload* m_rsp_txn, *m_tb_txn;
  sc_core::sc_time m_txn_time;
  tlm::tlm_phase  m_txn_ph, m_rtb_ph, m_timing_ph;
  std::deque<tlm::tlm_generic_payload*> m_dt_txns;
  std::vector<lock_object_base*> m_lock_objects;
  // one burst position handler per thread/tag
  std::vector<std::vector<ocp_txn_track> > m_burst_tracker;
  resp_thread_busy *m_rtb;
  uint32_t  m_max_impr_burst;
  static const uint32_t m_byte_width = (BUSWIDTH+7)>>3;
};

}
