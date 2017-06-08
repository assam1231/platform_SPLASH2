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
//  Description :  This file contains the adapter for legacy TL1 slaves.
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_tl1_slave_legacy_adapter.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{

template <typename DataCl, unsigned int BUSWIDTH=DataCl::SizeCalc::bit_size>
class ocp_tl1_slave_legacy_adapter
  : public sc_core::sc_module
  , public ocpip_legacy::OCP_TL1_Slave_TimingIF
  , public ocpip_legacy::OCP_TL_Config_Listener
{
public:
  typedef ocp_tl1_legacy_convertion_helper<DataCl
                                          ,ocp_tl1_slave_legacy_adapter<DataCl, BUSWIDTH>
                                          ,typename ocp_slave_socket_tl1<BUSWIDTH>::ext_support_type
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

  ocpip_legacy::OCP_TL1_MasterPort<DataCl>      master_port;
  ocp_slave_socket_tl1<BUSWIDTH> slave_socket;

  SC_HAS_PROCESS(ocp_tl1_slave_legacy_adapter);

  ocp_tl1_slave_legacy_adapter(sc_core::sc_module_name name)
    : m_master_name(std::string(name)+"master_port")
    , m_slave_name(std::string(name)+"slave_socket")
    , master_port(m_master_name.c_str())
    , slave_socket(  m_slave_name.c_str()
                    , this
                    , &ocp_tl1_slave_legacy_adapter::set_master_timing)
    , m_converter(std::string(name).c_str(), this, &ocp_tl1_slave_legacy_adapter::get_params)
    , m_get_param_called(false)
    , m_own_reset(false)
    , m_reset_given(false)
    , m_reset_end_given(false)
    , m_tb_txn(NULL)
    , m_req_txn(NULL)
    , m_dt_txn(NULL)
  {
    SC_METHOD(resp_handler);
    sensitive<<master_port.ResponseStartEvent();
    dont_initialize();

    SC_METHOD(data_end_handler);
    sensitive<<master_port.DataHSEndEvent();
    dont_initialize();

    SC_METHOD(req_end_handler);
    sensitive<<master_port.RequestEndEvent();
    dont_initialize();

    SC_METHOD(sthreadbusy_handler);
    sensitive<<master_port.SThreadBusyEvent();
    dont_initialize();

    SC_METHOD(sdatathreadbusy_handler);
    sensitive<<master_port.SDataThreadBusyEvent();
    dont_initialize();

#if (TLM_VERSION_MAJOR>2 | TLM_VERSION_MINOR>0 | TLM_VERSION_PATCH>0)
    SC_METHOD(reset_start_handler);
    sensitive<<master_port.ResetStartEvent()<<master_port.getFakeResetStartEvent();
    dont_initialize();

    SC_METHOD(reset_end_handler);
    sensitive<<master_port.ResetEndEvent()<<master_port.getFakeResetEndEvent();
    dont_initialize();
#endif

    slave_socket.register_nb_transport_fw(this, &ocp_tl1_slave_legacy_adapter::nb_transport);
    slave_socket.register_configuration_listener_callback(this, &ocp_tl1_slave_legacy_adapter::socket_config_callback);
    slave_socket.activate_synchronization_protection();
    slave_socket.make_generic();

    m_tb_txn=slave_socket.get_tb_transaction();
    m_ctb_ph=CMD_THREAD_BUSY_CHANGE;
    m_dtb_ph=DATA_THREAD_BUSY_CHANGE;
    m_ctb=slave_socket.template get_extension<cmd_thread_busy>(*m_tb_txn);
    m_dtb=slave_socket.template get_extension<data_thread_busy>(*m_tb_txn);
  }

  ~ocp_tl1_slave_legacy_adapter(){

  }

  void before_end_of_elaboration(){
    ocpip_legacy::OCP_TL1_Master_TimingCl legacy_m_timing;
    legacy_m_timing.RequestGrpStartTime=sc_core::sc_get_time_resolution();
    legacy_m_timing.DataHSGrpStartTime=sc_core::sc_get_time_resolution();
    legacy_m_timing.MThreadBusyStartTime=sc_core::sc_get_time_resolution();
    legacy_m_timing.MRespAcceptStartTime=sc_core::sc_get_time_resolution();
    master_port->setOCPTL1MasterTiming(legacy_m_timing);

    master_port->addOCPConfigurationListener(*this);
    master_port->registerTimingSensitiveOCPTL1Master(this);
  }

  void start_of_simulation(){
    //allocate a Q of responses per tag per thread
    m_rsp_txns.resize(get_params().threads);
#ifndef NDEBUG
    m_lock_objects.resize(get_params().threads);
#endif
    for (unsigned int i=0; i<m_rsp_txns.size(); i++){
      m_rsp_txns[i].resize(get_params().tags);
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
  void set_master_timing(ocp_tl1_master_timing m_timing){
//    std::cout<<"Timing callback from TLM2 side"<<std::endl;
    ocpip_legacy::OCP_TL1_Master_TimingCl legacy_m_timing;
    legacy_m_timing.RequestGrpStartTime=m_timing.RequestGrpStartTime+sc_core::sc_get_time_resolution();
    legacy_m_timing.DataHSGrpStartTime=m_timing.DataHSGrpStartTime+sc_core::sc_get_time_resolution();
    legacy_m_timing.MThreadBusyStartTime=m_timing.MThreadBusyStartTime+sc_core::sc_get_time_resolution();
    legacy_m_timing.MRespAcceptStartTime=m_timing.MRespAcceptStartTime+sc_core::sc_get_time_resolution();
    master_port->setOCPTL1MasterTiming(legacy_m_timing);
  }

  //old kit callbacks
  void setOCPTL1SlaveTiming(ocpip_legacy::OCP_TL1_Slave_TimingCl legacy_s_timing){
//    std::cout<<"Timing callback from legacy side"<<std::endl;
    ocp_tl1_slave_timing s_timing;
    s_timing.ResponseGrpStartTime=legacy_s_timing.ResponseGrpStartTime;
    s_timing.SThreadBusyStartTime=legacy_s_timing.SThreadBusyStartTime;
    s_timing.SDataThreadBusyStartTime=legacy_s_timing.SDataThreadBusyStartTime;
    s_timing.SCmdAcceptStartTime=legacy_s_timing.SCmdAcceptStartTime;
    s_timing.SDataAcceptStartTime=legacy_s_timing.SDataAcceptStartTime;
    slave_socket.set_slave_timing(s_timing);
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

    if (params.diff(slave_socket.get_ocp_config()))
      slave_socket.set_ocp_config(params);
  }

  void resp_handler(){
    if (master_port->getReset() || m_own_reset){
      return;
    }
    typename converter_type::response_type rsp;
    master_port->getOCPResponse(rsp);
    assert(m_rsp_txns[rsp.SThreadID][rsp.STagID].size());
    tlm::tlm_generic_payload* rsp_txn=m_rsp_txns[rsp.SThreadID][rsp.STagID].front();
    m_curr_threadID=rsp.SThreadID;
    m_curr_tagID=rsp.STagID;
    assert(rsp_txn);
    m_converter.update_txn_from_legacy_rsp(rsp, *rsp_txn);
    m_txn_time=sc_core::SC_ZERO_TIME;
    m_txn_ph=tlm::BEGIN_RESP;
    switch (slave_socket->nb_transport_bw(*rsp_txn, m_txn_ph, m_txn_time)){
      case tlm::TLM_ACCEPTED : break;
      case tlm::TLM_UPDATED  :
        assert(m_txn_ph==tlm::END_RESP);
        finish_resp();
        break;
      case tlm::TLM_COMPLETED: assert(0);
    }
  }

  void finish_resp(){
    if (get_params().respaccept) master_port->putMRespAccept();
    typename converter_type::have_seen_it* have_seen;
    m_converter.acc(*m_rsp_txns[m_curr_threadID][m_curr_tagID].front()).get_extension(have_seen);
    assert(have_seen);
    bool is_locked_txn=have_seen->unlocking_wr;
    if (m_converter.check_end_of_monitoring(*m_rsp_txns[m_curr_threadID][m_curr_tagID].front(), 2)){
      if (is_locked_txn){
        lock* lck;
        slave_socket.template get_extension<lock>(lck, *m_rsp_txns[m_curr_threadID][m_curr_tagID].front());
        lck->value->atomic_txn_completed(); //we finished the unlocking write
      }
      m_running_txns.erase(m_rsp_txns[m_curr_threadID][m_curr_tagID].front());
      m_rsp_txns[m_curr_threadID][m_curr_tagID].pop_front();

    }
  }

  void data_end_handler(){
    if (master_port->getReset() || m_own_reset){
      return;
    }

    if (get_params().dataaccept){
      m_txn_time=sc_core::SC_ZERO_TIME;
      assert(m_dt_txn);
      m_txn_ph=END_DATA;
      finish_data();
      slave_socket->nb_transport_bw(*m_dt_txn, m_txn_ph, m_txn_time);
      m_dt_txn=NULL;
    }
  }

  void req_end_handler(){
    if (master_port->getReset() || m_own_reset){
      return;
    }

    if (get_params().cmdaccept){
      m_txn_time=sc_core::SC_ZERO_TIME;
      assert(m_req_txn);
      //if (m_is_srmd){
      //  m_txn_ph=END_SRMD_REQ;
      //}
      //else{
        m_txn_ph=tlm::END_REQ;
        finish_req();
      //}
      slave_socket->nb_transport_bw(*m_req_txn, m_txn_ph, m_txn_time);
      m_req_txn=NULL;
    }
  }

  void finish_req(){
    //TODO: go on here
    if (!get_params().datahandshake && m_req_txn->is_write() && !get_params().writeresp_enable){
      typename converter_type::have_seen_it* have_seen;
      m_converter.acc(*m_req_txn).get_extension(have_seen);
      assert(have_seen);
      bool is_unlocking_write=have_seen->unlocking_wr;
      if(m_converter.check_end_of_monitoring(*m_req_txn, 0)){
        if (is_unlocking_write){
          lock* lck;
          slave_socket.template get_extension<lock>(lck, *m_req_txn);
          lck->value->atomic_txn_completed(); //we finished the unlocking write
        }
        m_running_txns.erase(m_req_txn);
      }
    }
  }

  void sthreadbusy_handler(){
    if (master_port->getReset() || m_own_reset){
      return;
    }

    m_ctb->value=master_port->getSThreadBusy();
    m_txn_time=sc_core::SC_ZERO_TIME;
    slave_socket->nb_transport_bw(*m_tb_txn, m_ctb_ph, m_txn_time);
  }

  void sdatathreadbusy_handler(){
    if (master_port->getReset() || m_own_reset){
      return;
    }

    m_dtb->value=master_port->getSDataThreadBusy();
    m_txn_time=sc_core::SC_ZERO_TIME;
    slave_socket->nb_transport_bw(*m_tb_txn, m_dtb_ph, m_txn_time);
  }

#if (TLM_VERSION_MAJOR>2 | TLM_VERSION_MINOR>0 | TLM_VERSION_PATCH>0)
  void reset_start_handler(){
    if (get_params().sreset && !master_port->peekSReset_n() && !m_reset_given){ //slave reset
      tlm::tlm_phase ph(BEGIN_RESET);
      m_txn_time=sc_core::SC_ZERO_TIME;
      slave_socket->nb_transport_bw(*slave_socket.get_reset_transaction(), ph, m_txn_time);
      m_reset_given=true;
    }
    if (!m_own_reset){
      m_own_reset=true;
      slave_socket.reset();
      for (std::set<tlm::tlm_generic_payload*>::iterator i=m_running_txns.begin(); i!=m_running_txns.end(); i++)
        (*i)->acquire();
    }
  }

  void reset_end_handler(){
    //we either get a fake end (still in reset) or we get a real end (not in reset any more)

    //still in reset but our own reset is over, so we do nothing
    if (master_port->getReset() && master_port->peekMReset_n()) return;//our own fake end

    //still in reset but other side is not, so it's the fake end of the other side
    //OR
    //reset is over but we did not yet give the end to our slave
    if ((master_port->getReset() && master_port->peekSReset_n())|| (get_params().sreset && !master_port->getReset() && !m_reset_end_given)){
      assert(get_params().sreset && m_own_reset);
      tlm::tlm_phase ph(END_RESET);
      m_txn_time=sc_core::SC_ZERO_TIME;
      slave_socket->nb_transport_bw(*slave_socket.get_reset_transaction(), ph, m_txn_time);
      m_reset_end_given=true;
    }
    if (!master_port->getReset()){ //must be the real end
      assert(m_own_reset);
      m_req_txn=NULL;
      m_dt_txn=NULL;
      for (unsigned int i=0; i<m_rsp_txns.size(); i++)
        for (unsigned int j=0; j<m_rsp_txns[i].size(); j++)
          m_rsp_txns[i][j].clear();
      for (std::set<tlm::tlm_generic_payload*>::iterator i=m_running_txns.begin(); i!=m_running_txns.end(); i++){
        m_converter.check_end_of_monitoring(**i, 3);
        (*i)->release();
      }
      m_running_txns.clear();
      m_own_reset=false;
      m_reset_end_given=false;
      m_reset_given=false;
#ifndef NDEBUG
      for (unsigned int i=0; i<m_lock_objects.size(); i++) m_lock_objects[i]=NULL; //forget about all locks
#endif
      if (!get_params().sreset) //only the master has reset, so we will not inform him about the end of reset
        return;
    }
  }
#endif

  void finish_data(){
    if (!get_params().writeresp_enable){
      typename converter_type::have_seen_it* have_seen;
      m_converter.acc(*m_dt_txn).get_extension(have_seen);
      assert(have_seen);
      bool is_unlocking_write=have_seen->unlocking_wr;
      if(m_converter.check_end_of_monitoring(*m_dt_txn, 1)){
        if (is_unlocking_write){
          lock* lck;
          slave_socket.template get_extension<lock>(lck, *m_dt_txn);
          lck->value->atomic_txn_completed(); //we finished the unlocking write
        }
        m_running_txns.erase(m_dt_txn);
      }
    }
  }


  tlm::tlm_sync_enum nb_transport(tlm::tlm_generic_payload& txn, tlm::tlm_phase& ph, sc_core::sc_time& time){
    switch(ph){
      case tlm::BEGIN_REQ:
        assert(m_req_txn==NULL);
        if (m_converter.acc(txn).get_extension(converter_type::have_seen_it::priv_id)){ //if so, just update the things that have valid values
          master_port->startOCPRequest(m_converter.update_req(txn));
        }
        else{ //otherwise take everything out of the txn because now it is stable
          typename converter_type::request_type& req=m_converter.fill_req(txn);
          m_running_txns.insert(&txn);
#ifndef NDEBUG
          if (m_lock_objects[req.MThreadID] && (req.MCmd==ocpip_legacy::OCP_MCMD_WR || req.MCmd==ocpip_legacy::OCP_MCMD_WRNP)){
            lock* lck;
            slave_socket.template get_extension<lock>(lck, txn);
            if (lck->value!=m_lock_objects[req.MThreadID]) {
              std::cerr<<name()<<" ReadEx and Write(Np) are not properly locked together"<<std::endl;
              exit(1);
            }
            m_lock_objects[req.MThreadID]=NULL; //check was okay, so we can reset
          }
          if (req.MCmd==ocpip_legacy::OCP_MCMD_RDEX){
            lock* lck;
            slave_socket.template get_extension<lock>(lck, txn);
            assert(lck->value->number_of_txns==2);
            m_lock_objects[req.MThreadID]=lck->value;
          }
#endif
          master_port->startOCPRequest(req);
          if ((txn.is_write() && get_params().writeresp_enable) || txn.is_read()){
            m_rsp_txns[req.MThreadID][req.MTagID].push_back(&txn);
          }
        }

        if (!get_params().cmdaccept){
          ph=tlm::END_REQ;
          m_req_txn=&txn;
          finish_req();
          m_req_txn=NULL;
          return tlm::TLM_UPDATED;
        }
//        m_is_srmd=false;
        m_req_txn=&txn;
        break;
      case tlm::END_RESP:
        finish_resp();
        break;
      default:
        if (ph==BEGIN_DATA){
          assert(m_dt_txn==NULL);
          master_port->startOCPDataHS(m_converter.update_dhs(txn));
          if (!get_params().dataaccept){
            ph=END_DATA;
            m_dt_txn=&txn;
            finish_data();
            m_dt_txn=NULL;
            return tlm::TLM_UPDATED;
          }
          m_dt_txn=&txn;
        }
        else
        if (ph==RESP_THREAD_BUSY_CHANGE){
          resp_thread_busy* tb=slave_socket.template get_extension<resp_thread_busy>(txn);
          master_port->putMThreadBusy(tb->value);
        }
#if (TLM_VERSION_MAJOR>2 | TLM_VERSION_MINOR>0 | TLM_VERSION_PATCH>0)
        else
        if (ph==BEGIN_RESET){
          if (!m_own_reset){
            m_own_reset=true;
            for (std::set<tlm::tlm_generic_payload*>::iterator i=m_running_txns.begin(); i!=m_running_txns.end(); i++)
              (*i)->acquire();
            slave_socket.reset();
          }
          master_port->MResetAssert();
        }
        else
        if (ph==END_RESET){
          master_port->MResetDeassert();
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
  bool m_get_param_called, m_own_reset, m_reset_given, m_reset_end_given;
  tlm::tlm_generic_payload *m_tb_txn, *m_req_txn, *m_dt_txn;
  sc_core::sc_time m_txn_time;
  tlm::tlm_phase  m_txn_ph, m_ctb_ph, m_dtb_ph;
  std::vector<std::vector<std::deque<tlm::tlm_generic_payload*> > > m_rsp_txns;
#ifndef NDEBUG
  std::vector<lock_object_base*> m_lock_objects;
#endif
  cmd_thread_busy *m_ctb;
  data_thread_busy *m_dtb;
  unsigned int m_curr_threadID, m_curr_tagID;
  std::set<tlm::tlm_generic_payload*> m_running_txns;
};

}
