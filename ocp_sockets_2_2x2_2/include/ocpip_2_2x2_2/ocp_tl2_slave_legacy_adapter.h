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
//  Description :  This file contains the adapter for legacy TL2 slaves.
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_tl2_slave_legacy_adapter.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{

template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH>
class ocp_tl2_slave_legacy_adapter
  : public sc_core::sc_module
  , public ocpip_legacy::OCP_TL_Config_Listener
{
    typedef ocp_data_class_unsigned<BUSWIDTH, ADDRWIDTH> DataCl;
    typedef typename DataCl::DataType Td;
    typedef typename DataCl::AddrType Ta;
    typedef ocpip_legacy::OCPTL2RequestGrp<Td, Ta>  ocp_legacy_req;
    typedef ocpip_legacy::OCPTL2ResponseGrp<Td>     ocp_legacy_resp;

public:

  std::string m_master_name, m_slave_name;

  ocpip_legacy::OCP_TL2_MasterPort<Td, Ta>      master_port;
  ocp_slave_socket_tl2<BUSWIDTH>                slave_socket;

  SC_HAS_PROCESS(ocp_tl2_slave_legacy_adapter);

  ocp_tl2_slave_legacy_adapter(sc_core::sc_module_name name)
    : m_master_name(std::string(name)+"_master_port")
    , m_slave_name(std::string(name)+"_slave_socket")
    , master_port(m_master_name.c_str())
    , slave_socket(  m_slave_name.c_str() )
    , m_invariant_ext_pool(5)
    , m_position_ext_pool(5)
    , m_legacy_req_ext_pool(5)
    , m_get_param_called(false)
    , m_tb_txn(NULL)
    , m_req_txn(NULL)
  {
    SC_METHOD(resp_handler);
    sensitive<<master_port.ResponseStartEvent();
    dont_initialize();

    SC_METHOD(req_end_handler);
    sensitive<<master_port.RequestEndEvent();
    dont_initialize();

    SC_METHOD(sthreadbusy_handler);
    sensitive<<master_port.SThreadBusyEvent();
    dont_initialize();

    SC_METHOD(stiming_handler);
    sensitive<<master_port.SlaveTimingEvent();

    slave_socket.register_nb_transport_fw(this, &ocp_tl2_slave_legacy_adapter::nb_transport);
    slave_socket.register_configuration_listener_callback(this, &ocp_tl2_slave_legacy_adapter::socket_config_callback);
    slave_socket.make_generic();

    m_tb_txn=slave_socket.get_tb_transaction();
    m_ctb_ph=CMD_THREAD_BUSY_CHANGE;
    m_timing_ph=TL2_TIMING_CHANGE;
    m_ctb=slave_socket.template get_extension<cmd_thread_busy>(*m_tb_txn);
  }

  ~ocp_tl2_slave_legacy_adapter(){

  }

  void start_of_simulation(){
    m_params = slave_socket.get_ocp_config();
    int threads = m_params.threads;
    int tags    = m_params.tags;

    //allocate a Q of responses per tag per thread
    m_rsp_txns.resize(threads);
#ifndef NDEBUG
    m_lock_objects.resize(threads);
#endif
    for (unsigned int i=0; i<m_rsp_txns.size(); i++){
      m_rsp_txns[i].resize(tags);
#ifndef NDEBUG
      m_lock_objects[i].resize(tags);
#endif
    }

    m_burst_tracker.resize( threads );
    for ( int t=0; t<threads; ++t ) {
	m_burst_tracker[t].resize( tags, ocp_txn_track( &m_params, false ) );
    }
  }

  //callback shall merge conigs of socket and port
  ocpip_legacy::ParamCl<DataCl>& get_params(){
    return m_legacy_config;
  }

  //new kit callbacks
  void socket_config_callback(const ocp_parameters& params, const std::string&){
      if (params.config_state==ocp_generic){
	  return;
      }
      ocpip_legacy::MapStringType config_map;
      ocpip_legacy::Config_manager::rebuildMap(params.template legacy_conversion<DataCl>(), config_map);

      ocp_parameters tmp;
      tmp.init_from_legacy(m_legacy_config);

      if (tmp.diff(params)){
	  master_port->setOCPMasterConfiguration(config_map);
      }
  }

  void set_configuration(ocpip_legacy::OCPParameters& passedConfig, std::string channelName){
    if (!passedConfig.configured){
      return;
    }
    ocpip_legacy::MapStringType config_map;
    ocpip_legacy::Config_manager::rebuildMap(passedConfig, config_map);
    ocp_parameters params;
    params.init_from_legacy(passedConfig);
    //remember how the legacy channel is configured
    m_legacy_config.setOCPConfiguration(slave_socket.name(), config_map);

    slave_socket.set_ocp_config(params);
  }

  void resp_handler(){
    ocp_legacy_resp rsp;
    master_port->getOCPResponse(rsp);
    assert(m_rsp_txns[rsp.SThreadID][rsp.STagID].size());
    tlm::tlm_generic_payload* rsp_txn=m_rsp_txns[rsp.SThreadID][rsp.STagID].front();
    resp_update( *rsp_txn, rsp );

    m_txn_ph=tlm::BEGIN_RESP;
    m_txn_time=sc_core::SC_ZERO_TIME;
    switch (slave_socket->nb_transport_bw(*rsp_txn, m_txn_ph, m_txn_time)) {
      case tlm::TLM_ACCEPTED : break;
      case tlm::TLM_UPDATED  :
	  assert(m_txn_ph==tlm::END_RESP);
	  finish_resp();
	  break;
      case tlm::TLM_COMPLETED: assert(0);
    }
  }

  void finish_resp() {
      master_port->acceptResponse();
  }

  void req_end_handler() {
      if (get_params().cmdaccept){
	  m_txn_time=sc_core::SC_ZERO_TIME;
	  assert(m_req_txn);
	  m_txn_ph=tlm::END_REQ;
	  slave_socket->nb_transport_bw(*m_req_txn, m_txn_ph, m_txn_time);
	  if ( !m_req_txn->is_write() || !m_params.datahandshake ) {
	      finish_req();
	      m_req_txn=NULL;
	  }
      }
      if (get_params().dataaccept && m_req_txn && m_req_txn->is_write()){
	  assert( m_params.datahandshake );
	  m_txn_time=sc_core::SC_ZERO_TIME;
	  m_txn_ph=END_DATA;
	  slave_socket->nb_transport_bw(*m_req_txn, m_txn_ph, m_txn_time);
	  finish_req();
	  m_req_txn=NULL;
      }
  }

  void finish_req(){
    //TODO: go on here
    if (!get_params().datahandshake && m_req_txn->is_write() && !get_params().writeresp_enable){
//       typename converter_type::have_seen_it* have_seen;
//       m_converter.acc(*m_req_txn).get_extension(have_seen);
//       assert(have_seen);
//       bool is_unlocking_write=have_seen->unlocking_wr;
//       if(m_converter.check_end_of_monitoring(*m_req_txn, 0)){
//         if (is_unlocking_write){
//           lock* lck;
//           slave_socket.template get_extension<lock>(lck, *m_req_txn);
//           lck->value->atomic_txn_completed(); //we finished the unlocking write
//         }
//       }
    }
  }

  void sthreadbusy_handler(){
    m_ctb->value=master_port->getSThreadBusy();
    m_txn_time=sc_core::SC_ZERO_TIME;
    slave_socket->nb_transport_bw(*m_tb_txn, m_ctb_ph, m_txn_time);
  }

  void stiming_handler() {
      ocpip_legacy::STimingGrp s_timing;
      master_port->getSlaveTiming(s_timing);
      tlm::tlm_generic_payload* p_timing_txn = slave_socket.get_tl2_timing_transaction();
      tl2_timing*        p_timing_ext = slave_socket.template get_extension<tl2_timing>(*p_timing_txn);
      p_timing_ext->value.type = SLAVE_TIMING;
      p_timing_ext->value.slave_timing.RqAL   = s_timing.RqAL;
      p_timing_ext->value.slave_timing.DAL    = s_timing.DAL;
      p_timing_ext->value.slave_timing.RpSndI = s_timing.RpSndI;
      m_txn_time=sc_core::SC_ZERO_TIME;
      slave_socket->nb_transport_bw(*p_timing_txn, m_timing_ph, m_txn_time);
  }

  tlm::tlm_sync_enum nb_transport(tlm::tlm_generic_payload& txn, tlm::tlm_phase& ph, sc_core::sc_time& time) {
      bool sent_req = false;
      switch(ph){
      case tlm::BEGIN_REQ: {
	  assert(m_req_txn==NULL);
	  ocp_legacy_req& req = req_handler( txn, ph );
	  if ( !txn.is_write() || !m_params.datahandshake ) {
	      sent_req = master_port->sendOCPRequest(req);
	  } else {
	      // we'll send it at BEGIN_DATA
	  }
	  if ((txn.is_write() && m_params.writeresp_enable) || txn.is_read()){
	      m_rsp_txns[req.MThreadID][req.MTagID].push_back(&txn);
	  }

	  m_req_txn=&txn;
	  if (!get_params().cmdaccept){
	      ph=tlm::END_REQ;
	      if ( sent_req )
		  finish_req();
	      m_req_txn=NULL;
	      return tlm::TLM_UPDATED;
	  }
	  break;
      }
      case tlm::END_RESP:
	  finish_resp();
	  break;
      default:
	  if (ph==BEGIN_DATA){
	      ocp_legacy_req& req = req_handler( txn, ph );
	      assert( txn.is_write() && m_params.datahandshake );
	      sent_req = master_port->sendOCPRequest(req);

	      m_req_txn=&txn;
 	      if (!get_params().dataaccept){
 		  ph=END_DATA;
		  if ( sent_req )
		      finish_req();
 		  m_req_txn=NULL;
 		  return tlm::TLM_UPDATED;
 	      }
	  }
	  else if (ph==RESP_THREAD_BUSY_CHANGE){
	      resp_thread_busy* tb=slave_socket.template get_extension<resp_thread_busy>(txn);
	      master_port->putMThreadBusy(tb->value);
	  }
	  else if (ph==TL2_TIMING_CHANGE){
	      tl2_timing* p_timing=slave_socket.template get_extension<tl2_timing>(txn);
	      ocpip_legacy::MTimingGrp m_timing;
	      m_timing.RqSndI = p_timing->value.master_timing.RqSndI;
	      m_timing.DSndI  = p_timing->value.master_timing.DSndI;
	      m_timing.RpAL   = p_timing->value.master_timing.RpAL;
	      master_port->putMasterTiming( m_timing );
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
    void get_thread_and_tag(
	tlm::tlm_generic_payload& txn, uint32_t& thread, uint32_t& tag) {
	thread = tag = 0;
	thread_id* th_id;
	if ( slave_socket.get_extension(th_id, txn) )
	    thread = th_id->value;
	tag_id* tg_id;
	if ( slave_socket.get_extension(tg_id, txn) )
	    tag = tg_id->value;
    }

    ocp_legacy_req& req_handler( tlm::tlm_generic_payload& txn, tlm::tlm_phase& ph ) {
	// extract the invariant and attach to txn + track burst position
	ocp_txn_burst_invariant* p_inv;
	ocp_txn_position * p_pos;
	ocp_legacy_req_ext* p_ocp_req;

	uint32_t thread, tag;
	get_thread_and_tag( txn, thread, tag );
	assert( thread < m_burst_tracker.size() );
	assert( tag    < m_burst_tracker[thread].size() );
	ocp_txn_track& track = m_burst_tracker[thread][tag];
	bool new_txn = false;
	if ( ph==tlm::BEGIN_REQ && !track.has_pending_request() ) {
	    // this is a new transaction
	    new_txn = true;
	    p_inv = m_invariant_ext_pool.create();
	    *p_inv = ocp_txn_burst_invariant::init_from( txn, m_params, slave_socket );
	    acc(txn).set_extension( p_inv );
	    p_pos = m_position_ext_pool.create();
	    acc(txn).set_extension( p_pos );
	    p_pos->req_position .count = 0;
	    p_pos->dh_position  .count = 0;
	    p_pos->resp_position.count = 0;
	    p_ocp_req = m_legacy_req_ext_pool.create();
	    acc(txn).set_extension( p_ocp_req );
	}
	acc(txn).get_extension(p_inv);
	assert(p_inv);
	acc(txn).get_extension(p_pos);
	assert(p_pos);
	acc(txn).get_extension(p_ocp_req);
	assert(p_ocp_req);
	ocp_legacy_req& req = p_ocp_req->req;

	if ( new_txn ) {
	    req.MCmd            = p_inv->cmd;
	    req.MThreadID       = p_inv->threadid;
	    req.MConnID         = p_inv->connid;
	    req.MTagID          = p_inv->tagid;
	    req.MAddrSpace      = p_inv->addr_space;
	    req.MBurstSeq       = static_cast<ocpip_legacy::OCPMBurstSeqType>( p_inv->get_sequence() );
	    req.MBurstLength    = p_inv->burst_length;
	    req.MBurstPrecise   = p_inv->precise;
	    req.MBurstSingleReq = p_inv->srmd;
	}

	uint32_t starting_count = p_pos->req_position.count + 1;
	try {
	    if ( ph == tlm::BEGIN_REQ )
		p_pos->req_position = track.track_phase( txn, tlm::BEGIN_REQ );
	    else
		p_pos->dh_position = track.track_phase( txn, BEGIN_DATA );
	} catch ( const ocp_txn_track_error& err ) {
	    SC_REPORT_ERROR( "TLM transport fw", err.what() );
	}

	word_count* word_count_info = NULL;
	bool has_word_count= slave_socket.get_extension(word_count_info, txn);
	uint32_t req_word_count = has_word_count ?
	    word_count_info->value.request_wc : p_inv->burst_length;
	uint32_t data_word_count = has_word_count ?
	    word_count_info->value.data_wc : p_inv->burst_length;
	if ( ph == BEGIN_DATA ) {
	    req.DataLength = data_word_count;
	    req.LastOfBurst = p_pos->dh_position.remain == 0;
	    req.LastOfRow   = p_pos->dh_position.row_remain == 0;
	} else {
	    req.DataLength = req_word_count;
	    req.LastOfBurst = p_pos->req_position.remain == 0;
	    req.LastOfRow = p_pos->req_position.row_remain == 0;
	}

	req.MAddr           = txn.get_address();

#ifndef NDEBUG
	if (m_lock_objects[req.MThreadID][req.MTagID] &&
	    (req.MCmd==ocpip_legacy::OCP_MCMD_WR || req.MCmd==ocpip_legacy::OCP_MCMD_WRNP)){
	    lock* lck;
	    slave_socket.template get_extension<lock>(lck, txn);
	    if (lck->value!=m_lock_objects[req.MThreadID][req.MTagID]) {
		std::cerr<<name()<<" ReadEx and Write(Np) are not properly locked together"<<std::endl;
		exit(1);
	    }
	    m_lock_objects[req.MThreadID][req.MTagID]=NULL; //check was okay, so we can reset
	}
	if (req.MCmd==ocpip_legacy::OCP_MCMD_RDEX){
	    lock* lck;
	    slave_socket.template get_extension<lock>(lck, txn);
	    assert(lck->value->number_of_txns==2);
	    m_lock_objects[req.MThreadID][req.MTagID]=lck->value;
	}
#endif
	// This is incorrect. The array needs to be copied and shuffled according to burst sequence
	req.MDataPtr   = (Td*)txn.get_data_ptr();
	if ( txn.get_byte_enable_ptr() != NULL ) {
	    req.MByteEnPtr = (unsigned int*)txn.get_byte_enable_ptr();
	} else {
	    req.MByteEn=0xffffffff;
	}
	return req;
    }

    void resp_update( tlm::tlm_generic_payload& txn, ocp_legacy_resp& resp ) {
	// extract the invariant and attach to txn + track burst position
	ocp_txn_burst_invariant* p_inv;
	ocp_txn_position * p_pos;

	acc(txn).get_extension(p_inv);
	assert(p_inv);
	acc(txn).get_extension(p_pos);
	assert(p_pos);

	word_count* word_count_info;
	slave_socket.template get_extension<word_count>(word_count_info, txn);
	word_count_info->value.response_wc = resp.DataLength;
	slave_socket.template validate_extension<word_count>( txn );

	ocp_txn_track& track = m_burst_tracker[p_inv->threadid][p_inv->tagid];
	try {
	    p_pos->resp_position = track.track_phase( txn, tlm::BEGIN_RESP );
	} catch ( const ocp_txn_track_error& err ) {
	    SC_REPORT_ERROR( "Legacy response", err.what() );
	}

	switch(resp.SResp){
	case ocpip_legacy::OCP_SRESP_DVA : txn.set_response_status(tlm::TLM_OK_RESPONSE); break;
	case ocpip_legacy::OCP_SRESP_NULL: txn.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE); break;
	case ocpip_legacy::OCP_SRESP_FAIL:
	{
	    txn.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
	    semaphore* sem;
	    slave_socket.template get_extension<semaphore>(sem, txn);
	    slave_socket.template validate_extension<semaphore>(txn);
	    sem->value=false;
	    break;
	}
	case ocpip_legacy::OCP_SRESP_ERR:  txn.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
	}

	if ( txn.is_read() && resp.SDataPtr != NULL ) {
	    // TODO: this is too simplistic. Depends on the burstseq and host endianness
	    memcpy( txn.get_data_ptr(), resp.SDataPtr, resp.DataLength * m_byte_width );
	}
    }


    struct ocp_legacy_req_ext : public tlm_utils::instance_specific_extension<ocp_legacy_req_ext> {
	ocp_legacy_req req;
    };

    tlm_utils::instance_specific_extension_accessor acc;
    ocp_extension_pool<ocp_txn_burst_invariant> m_invariant_ext_pool;
    ocp_extension_pool<ocp_txn_position>        m_position_ext_pool;
    ocp_extension_pool<ocp_legacy_req_ext>      m_legacy_req_ext_pool;

  protected:

      ocpip_legacy::ParamCl<DataCl> m_legacy_config;
      ocp_parameters                m_params;
      bool m_get_param_called;
    tlm::tlm_generic_payload *m_tb_txn, *m_req_txn;
      sc_core::sc_time m_txn_time;
      tlm::tlm_phase  m_txn_ph, m_ctb_ph, m_timing_ph;
      std::vector<std::vector<std::deque<tlm::tlm_generic_payload*> > > m_rsp_txns;
#ifndef NDEBUG
      std::vector<std::vector<lock_object_base* > >m_lock_objects;
#endif
      // one burst position handler per thread/tag
      std::vector<std::vector<ocp_txn_track> > m_burst_tracker;
      cmd_thread_busy *m_ctb;
      unsigned int m_curr_threadID, m_curr_tagID;
      static const uint32_t m_byte_width = (BUSWIDTH+7)>>3;
  };

}
