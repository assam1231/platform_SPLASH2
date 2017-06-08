///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// (c) Copyright OCP-IP 2008,2009
// OCP-IP Confidential and Proprietary
//
//
//============================================================================
//      Project : OCP SLD WG
//       Author : Robert Guenzel (from TU of Braunschweig) for Greensocs Ltd.
//
//          $Id:
//
//  Description :  This file contains the adapter for legacy TL1 masters.
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_tl1_master_legacy_adapter.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{

template <typename DataCl, unsigned int BUSWIDTH=DataCl::SizeCalc::bit_size>
class ocp_tl1_master_legacy_adapter
  : public sc_core::sc_module
  , public ocpip_legacy::OCP_TL1_Master_TimingIF
  , public ocpip_legacy::OCP_TL_Config_Listener
{
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
  typedef ocp_tl1_legacy_convertion_helper<DataCl
                                          ,ocp_tl1_master_legacy_adapter<DataCl, BUSWIDTH>
                                          ,typename ocp_master_socket_tl1<BUSWIDTH>::ext_support_type
                                          ,BUSWIDTH
                                          > converter_type;


#define GEN_EXT2BIT_CB_REG_FNS(type)\
  void assign_##type##_info_extension_to_bit_mask_cb(typename converter_type::ext_to_bit_cb cb){\
    m_converter.assign_##type##_info_extension_to_bit_mask_cb(cb);\
  } \
  void assign_##type##_info_bit_mask_to_extension_cb(typename converter_type::bit_to_ext_cb cb){\
    m_converter.assign_##type##_info_bit_mask_to_extension_cb(cb);\
  }

  GEN_EXT2BIT_CB_REG_FNS(req)
  GEN_EXT2BIT_CB_REG_FNS(mdata)
  GEN_EXT2BIT_CB_REG_FNS(sdata)
  GEN_EXT2BIT_CB_REG_FNS(resp)
#undef GEN_EXT2BIT_CB_REG_FNS

  std::string m_master_name, m_slave_name;

  ocpip_legacy::OCP_TL1_SlavePort<DataCl>      slave_port;
  ocp_master_socket_tl1<BUSWIDTH> master_socket;

  SC_HAS_PROCESS(ocp_tl1_master_legacy_adapter);

  ocp_tl1_master_legacy_adapter(sc_core::sc_module_name name, unsigned int max_impr_burst_length=64)
    : m_master_name(std::string(name)+"master_socket")
    , m_slave_name(std::string(name)+"slave_port")
    , slave_port(m_slave_name.c_str())
    , master_socket(  m_master_name.c_str()
                    , this
                    , &ocp_tl1_master_legacy_adapter::set_slave_timing
                    , ocp_master_socket_tl1<BUSWIDTH>::mm_txn_with_be_and_data())
    , m_converter(std::string(name).c_str(), this, &ocp_tl1_master_legacy_adapter::get_params)
    , m_get_param_called(false)
    , m_own_reset(false)
    , m_reset_given(false)
    , m_reset_end_given(false)
    //, m_next_req_expected(false)
    //, m_req_txn(NULL)
    , m_rsp_txn(NULL)
    , m_max_impr_burst(max_impr_burst_length)
  {
    SC_METHOD(req_handler);
    sensitive<<slave_port.RequestStartEvent();
    dont_initialize();

    SC_METHOD(data_handler);
    sensitive<<slave_port.DataHSStartEvent();
    dont_initialize();

    SC_METHOD(resp_end_handler);
    sensitive<<slave_port.ResponseEndEvent();
    dont_initialize();

    SC_METHOD(mthreadbusy_handler);
    sensitive<<slave_port.MThreadBusyEvent();
    dont_initialize();

#if (TLM_VERSION_MAJOR>2 | TLM_VERSION_MINOR>0 | TLM_VERSION_PATCH>0)
    SC_METHOD(reset_start_handler);
    sensitive<<slave_port.ResetStartEvent()<<slave_port.getFakeResetStartEvent();
    dont_initialize();

    SC_METHOD(reset_end_handler);
    sensitive<<slave_port.ResetEndEvent()<<slave_port.getFakeResetEndEvent();
    dont_initialize();
#endif

    master_socket.register_nb_transport_bw(this, &ocp_tl1_master_legacy_adapter::nb_transport);
    master_socket.register_configuration_listener_callback(this, &ocp_tl1_master_legacy_adapter::socket_config_callback);
    master_socket.activate_synchronization_protection(); //this will ensure become non-default timing
    master_socket.make_generic();

    m_tb_txn=master_socket.get_tb_transaction();
    m_tb_ph=RESP_THREAD_BUSY_CHANGE;
    m_tb=master_socket.template get_extension<resp_thread_busy>(*m_tb_txn);
  }

  ~ocp_tl1_master_legacy_adapter(){

  }

  void before_end_of_elaboration(){
    //since we use a dp PEQ, everything will start at clock_time+time_resolution_unit
    // more precisely, everything will start in the very first delta of this time
    //  hence the old kit master will be on the safe side (he waits at least our time +1 delta)
    ocpip_legacy::OCP_TL1_Slave_TimingCl legacy_s_timing;
    legacy_s_timing.ResponseGrpStartTime=sc_core::sc_get_time_resolution();
    legacy_s_timing.SThreadBusyStartTime=sc_core::sc_get_time_resolution();
    legacy_s_timing.SDataThreadBusyStartTime=sc_core::sc_get_time_resolution();
    legacy_s_timing.SCmdAcceptStartTime=sc_core::sc_get_time_resolution();
    legacy_s_timing.SDataAcceptStartTime=sc_core::sc_get_time_resolution();
    slave_port->setOCPTL1SlaveTiming(legacy_s_timing);

    slave_port->addOCPConfigurationListener(*this);
    slave_port->registerTimingSensitiveOCPTL1Slave(this);
  }

  void start_of_simulation(){
    m_req_txns.resize(get_params().threads);
    m_next_reqs_expected.resize(get_params().threads);
    for (unsigned int i=0; i<m_req_txns.size(); i++){
      m_req_txns[i].resize(get_params().tags, NULL);
      m_next_reqs_expected[i].resize(get_params().tags, false);
    }
    m_lock_objects.resize(get_params().threads);
  }

  //callback shall merge configs of socket and port
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
      slave_port->setOCPSlaveConfiguration(config_map);
    }
  }

  void set_slave_timing(ocp_tl1_slave_timing s_timing){
//    std::cout<<"Timing callback from TLM2 side"<<std::endl;
    ocpip_legacy::OCP_TL1_Slave_TimingCl legacy_s_timing;
    //due to our dp PEQ we have to add one time res unit
    legacy_s_timing.ResponseGrpStartTime=s_timing.ResponseGrpStartTime+sc_core::sc_get_time_resolution();
    legacy_s_timing.SThreadBusyStartTime=s_timing.SThreadBusyStartTime+sc_core::sc_get_time_resolution();
    legacy_s_timing.SDataThreadBusyStartTime=s_timing.SDataThreadBusyStartTime+sc_core::sc_get_time_resolution();
    legacy_s_timing.SCmdAcceptStartTime=s_timing.SCmdAcceptStartTime+sc_core::sc_get_time_resolution();
    legacy_s_timing.SDataAcceptStartTime=s_timing.SDataAcceptStartTime+sc_core::sc_get_time_resolution();
    slave_port->setOCPTL1SlaveTiming(legacy_s_timing);
  }

  //old kit callbacks
  void setOCPTL1MasterTiming(ocpip_legacy::OCP_TL1_Master_TimingCl legacy_m_timing){
//    std::cout<<"Timing callback from legacy side"<<std::endl;
    ocp_tl1_master_timing m_timing;
    m_timing.RequestGrpStartTime=legacy_m_timing.RequestGrpStartTime;
    m_timing.DataHSGrpStartTime=legacy_m_timing.DataHSGrpStartTime;
    m_timing.MThreadBusyStartTime=legacy_m_timing.MThreadBusyStartTime;
    m_timing.MRespAcceptStartTime=legacy_m_timing.MRespAcceptStartTime;
    master_socket.set_master_timing(m_timing);
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
    m_legacy_config.setOCPConfiguration(master_socket.name(), config_map);
    if (params.diff(master_socket.get_ocp_config()))
      master_socket.set_ocp_config(params);
  }


  void req_handler(){
    if (slave_port->getReset() || m_own_reset){
      return;
    }
    typename converter_type::request_type req;
    slave_port->getOCPRequest(req);
    m_curr_threadID=req.MThreadID;
    m_curr_tagID=req.MTagID;
    if (m_next_reqs_expected[m_curr_threadID][m_curr_tagID]){
      if (!req.MBurstPrecise){
        typename converter_type::have_seen_it* have_seen;
        m_converter.acc(*m_req_txns[m_curr_threadID][m_curr_tagID]).get_extension(have_seen);
        assert(have_seen);
        if (have_seen->req_cnt==m_max_impr_burst){
          SC_REPORT_ERROR(name(), "Imprecise burst length exceeds maximum imprecise burst length.");
        }
      }
      m_next_reqs_expected[m_curr_threadID][m_curr_tagID]=!(m_converter.update_txn_from_legacy_req(req, *m_req_txns[m_curr_threadID][m_curr_tagID]));
      m_txn_ph=tlm::BEGIN_REQ;
    }
    else{ //new txn
      m_req_txns[m_curr_threadID][m_curr_tagID]=master_socket.get_transaction();
      m_running_txns.insert(m_req_txns[m_curr_threadID][m_curr_tagID]);
      lock_object_base* lock_object=NULL;
      if (req.MCmd==ocpip_legacy::OCP_MCMD_WR || req.MCmd==ocpip_legacy::OCP_MCMD_WRNP){
        lock_object=m_lock_objects[req.MThreadID];
        m_lock_objects[req.MThreadID]=NULL; //rdex wr(np) pair is done for this thread
      }
      if (!req.MBurstPrecise)
        master_socket.reserve_data_size(*m_req_txns[m_curr_threadID][m_curr_tagID], ((BUSWIDTH+7)>>3)*m_max_impr_burst*req.MBlockHeight);
      else
        master_socket.reserve_data_size(*m_req_txns[m_curr_threadID][m_curr_tagID], ((BUSWIDTH+7)>>3)*req.MBurstLength*req.MBlockHeight);
      //m_req_txn->set_data_length(((BUSWIDTH+7)>>3)*req.MBurstLength+m_converter.get_diff());
      if (get_params().byteen | get_params().mdatabyteen){
        if (req.MBurstSingleReq && !get_params().mdatabyteen) //SRMD and dhs has no byte en: We have just as single BE pattern
          master_socket.reserve_be_size(*m_req_txns[m_curr_threadID][m_curr_tagID],   ((BUSWIDTH+7)>>3));
        else{ //we must expect a different be pattern per word
          if (!req.MBurstPrecise){
            master_socket.reserve_be_size(*m_req_txns[m_curr_threadID][m_curr_tagID],   ((BUSWIDTH+7)>>3)*m_max_impr_burst*req.MBlockHeight);
          }
          else{
            master_socket.reserve_be_size(*m_req_txns[m_curr_threadID][m_curr_tagID],   ((BUSWIDTH+7)>>3)*req.MBurstLength*req.MBlockHeight);
          }
        }
        //m_req_txn->set_byte_enable_length(((BUSWIDTH+7)>>3)*req.MBurstLength+m_converter.get_diff());
      }
      else{
        m_req_txns[m_curr_threadID][m_curr_tagID]->set_byte_enable_ptr(NULL);
        m_req_txns[m_curr_threadID][m_curr_tagID]->set_byte_enable_length(0);
      }
      if (req.MCmd==ocpip_legacy::OCP_MCMD_RDEX) {
        assert(m_lock_objects[req.MThreadID]==NULL);
        m_lock_objects[req.MThreadID]=my_lock_object::get_pool()->create();//master_socket.get_transaction(); //prepare the unlocking txn
        lock_object=m_lock_objects[req.MThreadID];
      }
      m_converter.init_txn_from_legacy_req(req, *m_req_txns[m_curr_threadID][m_curr_tagID], lock_object);
      //if (m_is_srmd)
        //m_txn_ph=BEGIN_SRMD_REQ;
      //else
      m_txn_ph=tlm::BEGIN_REQ;
      m_next_reqs_expected[m_curr_threadID][m_curr_tagID]=((req.MBurstLength>1 || (req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_BLCK && req.MBlockHeight>1)) && !req.MBurstSingleReq)? true:false;

      if (m_req_txns[m_curr_threadID][m_curr_tagID]->is_write()) {
        m_dt_txns.push_back(m_req_txns[m_curr_threadID][m_curr_tagID]);
      }
    }
    m_txn_time=sc_core::SC_ZERO_TIME;
    switch (master_socket->nb_transport_fw(*m_req_txns[m_curr_threadID][m_curr_tagID], m_txn_ph, m_txn_time)){
      case tlm::TLM_ACCEPTED : break;
      case tlm::TLM_UPDATED  :
        assert(m_txn_ph==tlm::END_REQ);
        finish_req();
        break;
      case tlm::TLM_COMPLETED: assert(0);
    }


  }

  void data_handler(){
    if (slave_port->getReset() || m_own_reset){
      return;
    }
    typename converter_type::datahs_type dt;
    slave_port->getOCPDataHS(dt);
    assert(m_dt_txns.size());
    tlm::tlm_generic_payload* dt_txn=m_dt_txns.front();
    assert(dt_txn);
    m_converter.update_txn_from_legacy_datahs(dt, *dt_txn);
    m_txn_time=sc_core::SC_ZERO_TIME;
    m_txn_ph=BEGIN_DATA;
    switch (master_socket->nb_transport_fw(*dt_txn, m_txn_ph, m_txn_time)){
      case tlm::TLM_ACCEPTED : break;
      case tlm::TLM_UPDATED  :
        assert(m_txn_ph==END_DATA);
        finish_data();
        break;
      case tlm::TLM_COMPLETED: assert(0);
    }
  }


  void finish_data(){
    if (get_params().dataaccept) slave_port->putSDataAccept();
    if (!get_params().writeresp_enable){
      if (m_converter.check_end_of_monitoring(*m_dt_txns.front(), 1)){
        master_socket.release_transaction(m_dt_txns.front());
        m_running_txns.erase(m_dt_txns.front());
        m_dt_txns.pop_front();
      }
    }
    else{
      typename converter_type::have_seen_it* have_seen;
      m_converter.acc(*m_dt_txns.front()).get_extension(have_seen);
      assert(have_seen);
      if (
          (!have_seen->req.MBurstPrecise && have_seen->req.MBurstLength==1 && have_seen->req_cnt==have_seen->dhs_cnt)
          |
          (have_seen->req.MBurstPrecise && (have_seen->req.MBurstLength*have_seen->req.MBlockHeight)==have_seen->dhs_cnt)
         )
      {
        m_dt_txns.pop_front();
      }

    }
  }

  void finish_req(){
    if (get_params().cmdaccept) slave_port->putSCmdAccept();
    if (!get_params().datahandshake && m_req_txns[m_curr_threadID][m_curr_tagID]->is_write() && !get_params().writeresp_enable)
      if (m_converter.check_end_of_monitoring(*m_req_txns[m_curr_threadID][m_curr_tagID], 0)){
        master_socket.release_transaction(m_req_txns[m_curr_threadID][m_curr_tagID]);
        m_running_txns.erase(m_req_txns[m_curr_threadID][m_curr_tagID]);
      }
  }

  void resp_end_handler(){
    if (slave_port->getReset() || m_own_reset){
      return;
    }
    if (get_params().respaccept){
      m_txn_ph=tlm::END_RESP;
      m_txn_time=sc_core::SC_ZERO_TIME;
      assert(m_rsp_txn);
      master_socket->nb_transport_fw(*m_rsp_txn, m_txn_ph, m_txn_time);
      if (m_converter.check_end_of_monitoring(*m_rsp_txn,2)){
        master_socket.release_transaction(m_rsp_txn);
        m_running_txns.erase(m_rsp_txn);
        m_rsp_txn=NULL;
      }
    }
  }


  void mthreadbusy_handler(){
    if (slave_port->getReset() || m_own_reset){
      return;
    }
    m_tb->value=slave_port->getMThreadBusy();
    m_txn_time=sc_core::SC_ZERO_TIME;
    master_socket->nb_transport_fw(*m_tb_txn, m_tb_ph, m_txn_time);
  }

#if (TLM_VERSION_MAJOR>2 | TLM_VERSION_MINOR>0 | TLM_VERSION_PATCH>0)
  void reset_start_handler(){
    if (get_params().mreset && !slave_port->peekMReset_n() && !m_reset_given){ //master reset
      tlm::tlm_phase ph(BEGIN_RESET);
      m_txn_time=sc_core::SC_ZERO_TIME;
      master_socket->nb_transport_fw(*master_socket.get_reset_transaction(), ph, m_txn_time);
      m_reset_given=true;
    }
    if (!m_own_reset){
      m_own_reset=true;
      master_socket.reset();
    }
  }

  void reset_end_handler(){
    //we either get a fake end (still in reset) or we get a real end (not in reset any more)

    //still in reset but our own reset is over, so we do nothing
    if (slave_port->getReset() && slave_port->peekSReset_n()) return;//our own fake end

    //still in reset but other side is not, so it's the fake end of the other side
    //OR
    //reset is over but we did not yet give the end to our slave
    if ((slave_port->getReset() && slave_port->peekMReset_n())|| (get_params().mreset && !slave_port->getReset() && !m_reset_end_given)){
      assert(get_params().mreset && m_own_reset);
      tlm::tlm_phase ph(END_RESET);
      m_txn_time=sc_core::SC_ZERO_TIME;
      master_socket->nb_transport_fw(*master_socket.get_reset_transaction(), ph, m_txn_time);
      m_reset_end_given=true;
    }
    if (!slave_port->getReset()){ //must be the real end
      assert(m_own_reset);
      for (unsigned int i=0; i<m_next_reqs_expected.size(); i++)
        for (unsigned int j=0; j<m_next_reqs_expected[i].size(); j++)
          m_next_reqs_expected[i][j]=false;
      for (std::set<tlm::tlm_generic_payload*>::iterator i=m_running_txns.begin(); i!=m_running_txns.end(); i++){
        m_converter.check_end_of_monitoring(**i, 3); //terminate state tracking
        (*i)->release(); //release transaction
      }
      m_running_txns.clear();
      m_dt_txns.clear();
      for (unsigned int i=0; i<m_lock_objects.size(); i++) m_lock_objects[i]=NULL; //forget about all locks
      m_own_reset=false;
      m_reset_end_given=false;
      m_reset_given=false;
      if (!get_params().mreset) //only the slave has reset, so we will not inform him about the end of reset
        return;
    }
  }
#endif

  tlm::tlm_sync_enum nb_transport(tlm::tlm_generic_payload& txn, tlm::tlm_phase& ph, sc_core::sc_time& time){
    switch (ph){
      case tlm::END_REQ:
        assert(m_req_txns[m_curr_threadID][m_curr_tagID]==&txn);
//        assert(!m_is_srmd);
        finish_req();
        break;
      case tlm::BEGIN_RESP:{
        typename converter_type::response_type& resp=m_converter.update_rsp(txn);
        slave_port->startOCPResponse(resp);
        if (!get_params().respaccept) {
          ph=tlm::END_RESP;
          if (m_converter.check_end_of_monitoring(txn,2)){
            master_socket.release_transaction(&txn);
            m_running_txns.erase(&txn);
          }
          return tlm::TLM_UPDATED;
        }
        m_rsp_txn=&txn;
        break;
      }
      default:
/*
        if (ph==END_SRMD_REQ){
          assert(m_req_txn==&txn);
          assert(m_is_srmd);
          finish_req();
        }
        else
*/
        if (ph==END_DATA){
          assert(m_dt_txns.front()==&txn);
          finish_data();
        }
        else
        if (ph==CMD_THREAD_BUSY_CHANGE){
          cmd_thread_busy* tb=master_socket.template get_extension<cmd_thread_busy>(txn);
          slave_port->putSThreadBusy(tb->value);
        }
        else
        if (ph==DATA_THREAD_BUSY_CHANGE){
          data_thread_busy* tb=master_socket.template get_extension<data_thread_busy>(txn);
          slave_port->putSDataThreadBusy(tb->value);
        }
#if (TLM_VERSION_MAJOR>2 | TLM_VERSION_MINOR>0 | TLM_VERSION_PATCH>0)
        else
        if (ph==BEGIN_RESET){
          m_own_reset=true;
          master_socket.reset();
          slave_port->SResetAssert();
        }
        else
        if (ph==END_RESET){
          slave_port->SResetDeassert();
        }
#endif
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

protected:

  converter_type m_converter;
  ocpip_legacy::ParamCl<DataCl> m_legacy_config;
  bool m_get_param_called, m_own_reset, m_reset_given, m_reset_end_given; //, m_next_req_expected;
  std::vector<std::vector<tlm::tlm_generic_payload*> >m_req_txns;
  std::vector<std::vector<bool> >m_next_reqs_expected;
  tlm::tlm_generic_payload* m_rsp_txn, *m_tb_txn;
  sc_core::sc_time m_txn_time;
  tlm::tlm_phase  m_txn_ph, m_tb_ph;
  std::deque<tlm::tlm_generic_payload*> m_dt_txns;
  std::vector<lock_object_base*> m_lock_objects;
  resp_thread_busy *m_tb;
  unsigned int m_max_impr_burst;
  unsigned int m_curr_tagID, m_curr_threadID;
  std::set<tlm::tlm_generic_payload*> m_running_txns;
  sc_core::sc_event m_clear_peq_event;
};

}
