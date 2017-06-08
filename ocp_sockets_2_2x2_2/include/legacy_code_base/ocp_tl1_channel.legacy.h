///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// (c) Copyright OCP-IP 2003, 2004, 2005
// OCP-IP Confidential and Proprietary
//
//
//============================================================================
//      Project : OCP SLD WG
//       Author : Alan Kamas, for Sonics Inc.
//                Anssi Haverinen, Nokia Inc.
//                Joe Chou, Sonics Inc.
//
//          $Id:
//
//  Description :  OCP TL 1 SystemC Channel Model
//        This version of Channel is derived from the original
//        OCP SLD WG, OCP Transaction Level Layer-1 model by
//        Norman Weyrich, Anssi Haverinen, and Joe Chou.
//        Please see that model for details.
//
// This version adds full channel access through the ports;
// single function calls to send and receive requests, data, and responses;
// and clocking to resolve OCP timing.
//
// Old Change History, before moving to CVS:
//
// 12/14/2003 Original revision
// 05/14/2004 Adds clocked blocking calls, moves
//            OCP trace monitor to a separate file, makes OCP
//            checker optional, adds *accept(bool)
//            calls for accepting transaction before it is issued.
//            Removes reset event sensitivity from blocking calls,
//            relying on new tl_channel reset functionality.
//            Data and address mask disabled to support array data types
//            Constructors modified.
// 09/27/2004 Adds public access methods for 'RequestReleaseEvent',
//            'ResponseReleaseEvent' and 'DataHSReleaseEvent'
//            (needed by TL0-TL1 adapters)
// 10/01/2004 Adds monitor object delete in destructor
// 16/12/2005 Remove old monitor, add new monitor interface
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef __OCP_TL1_CHANNEL_LEGACY_H__
#define __OCP_TL1_CHANNEL_LEGACY_H__

// Added for gcc 3.3 Compatibility
using namespace std;

// -----------------------------------------------------------------
// -----------------------------------------------------------------

template<typename TdataCl>
class OCP_TL1_Channel :
  public TL_Channel<TdataCl>,
  public OCP_TL1_MasterIF<TdataCl>,
  public OCP_TL1_SlaveIF<TdataCl>,
  public OCP_TL1_MonitorIF<TdataCl>
{
public:

  // -----------------------------------------------------------------
  // gcc 3.4 Compatibility
  // -----------------------------------------------------------------
  using TL_Channel<TdataCl>::m_ParamCl;
  using TL_Channel<TdataCl>::m_CommCl;
  using TL_Channel<TdataCl>::m_DataCl;
  using TL_Channel<TdataCl>::name;


  // -----------------------------------------------------------------
  // public types
  // -----------------------------------------------------------------

  typedef typename TdataCl::DataType Td;
  typedef typename TdataCl::AddrType Ta;
  typedef TL_Channel<TdataCl> base_type;

  // Needed for when the ocp clock tick process is used
  SC_HAS_PROCESS(OCP_TL1_Channel);


  //---------------------------------------------------------------
  // constructor for clocked and non-clocked channels
  //---------------------------------------------------------------

  OCP_TL1_Channel(std::string name,
                  sc_core::sc_in_clk * clk,
		  std::string monFileName = "" 
                  )
    : TL_Channel<TdataCl>(name.c_str(),true,true,true,false),
	m_confMan(this->name(), *m_ParamCl),
    m_addressMask(0x3),
    m_dataMask(0xFFFFFFFF),
    m_exitAfterOCPMon(false),
    m_clocked(true),
    m_clk(NULL),
    m_in_clk(clk),
    m_sig_clk(NULL),
    master_timing_sensitive(false),
    slave_timing_sensitive(false)
  {

#ifndef NDEBUG
    m_doRunCheck = true;
#else
    m_doRunCheck = false;
#endif
    if (clk==NULL) {
      m_clocked = false;
    } else {
      if (! monFileName.empty() ) {
	cerr << "Integrated Trace Monitor is deprecated, \n" 
	     << "please download monitor package from www.ocpip.org.\n"
	     << "Explicitly instantiate trace monitor and "
	     << "bind it to the channel and the clock\n";
      }
    }
    if (m_clocked) {
        m_sthreadbusy_pipelined=new OCP_TL1_Thread_pipelined_support((name+"-sthreadbusys_pipelined").c_str(), *clk);
        m_sdatathreadbusy_pipelined=new OCP_TL1_Thread_pipelined_support((name+"-sdatathreadbusys_pipelined").c_str(), *clk);
        m_mthreadbusy_pipelined=new OCP_TL1_Thread_pipelined_support((name+"-mthreadbusys_pipelined").c_str(), *clk);
    }
    m_no_pipelined_sanity_checks=false;
    m_getDataCl_deprecated_warning_given=false;
    m_getParamCl_deprecated_warning_given=false;
  }

  //---------------------------------------------------------------
  // constructor for clocked and non-clocked channels
  //---------------------------------------------------------------

  OCP_TL1_Channel(std::string name,
                  sc_core::sc_clock * clk,
		  std::string monFileName = ""
                  )
    : TL_Channel<TdataCl>(name.c_str(),true,true,true,false),
	m_confMan(this->name(), *m_ParamCl),
    m_addressMask(0x3),
    m_dataMask(0xFFFFFFFF),
    m_exitAfterOCPMon(false),
    m_clocked(true),
    m_clk(clk),
    m_in_clk(NULL),
    m_sig_clk(NULL),
    master_timing_sensitive(false),
    slave_timing_sensitive(false)
  {

#ifndef NDEBUG
    m_doRunCheck = true;
#else
    m_doRunCheck = false;
#endif
    if (clk==NULL) {
      m_clocked = false;
    } else {
      if (! monFileName.empty() ) {
	cerr << "Integrated Trace Monitor is deprecated, \n" 
	     << "please download monitor package from www.ocpip.org.\n"
	     << "Explicitly instantiate trace monitor and "
	     << "bind it to the channel and the clock\n";
      }
    }
    if (m_clocked) {
        m_sthreadbusy_pipelined=new OCP_TL1_Thread_pipelined_support((name+"-sthreadbusys_pipelined").c_str(), *clk);
        m_sdatathreadbusy_pipelined=new OCP_TL1_Thread_pipelined_support((name+"-sdatathreadbusys_pipelined").c_str(), *clk);
        m_mthreadbusy_pipelined=new OCP_TL1_Thread_pipelined_support((name+"-mthreadbusys_pipelined").c_str(), *clk);
    }
    m_no_pipelined_sanity_checks=false;
    m_getDataCl_deprecated_warning_given=false;
    m_getParamCl_deprecated_warning_given=false;
  }

  //---------------------------------------------------------------
  // constructor for clocked channels
  //---------------------------------------------------------------

  OCP_TL1_Channel(std::string name,
                  sc_core::sc_signal<bool> * clk,
		  std::string monFileName = ""
                  )
    : TL_Channel<TdataCl>(name.c_str(),true,true,true,false),
	m_confMan(this->name(), *m_ParamCl),
    m_addressMask(0x3),
    m_dataMask(0xFFFFFFFF),
    m_exitAfterOCPMon(false),
    m_clocked(true),
    m_clk(NULL),
    m_in_clk(NULL),
    m_sig_clk(clk),
    master_timing_sensitive(false),
    slave_timing_sensitive(false)
  {

#ifndef NDEBUG
    m_doRunCheck = true;
#else
    m_doRunCheck = false;
#endif
    if (clk==NULL) {
      m_clocked = false;
    } else {
      if (! monFileName.empty() ) {
	cerr << "Integrated Trace Monitor is deprecated, \n" 
	     << "please download monitor package from www.ocpip.org.\n"
	     << "Explicitly instantiate trace monitor and "
	     << "bind it to the channel and the clock\n";
      }
    }
    if (m_clocked) {
        m_sthreadbusy_pipelined=new OCP_TL1_Thread_pipelined_support((name+"-sthreadbusys_pipelined").c_str(), *clk);
        m_sdatathreadbusy_pipelined=new OCP_TL1_Thread_pipelined_support((name+"-sdatathreadbusys_pipelined").c_str(), *clk);
        m_mthreadbusy_pipelined=new OCP_TL1_Thread_pipelined_support((name+"-mthreadbusys_pipelined").c_str(), *clk);
    }
    m_no_pipelined_sanity_checks=false;
    m_getDataCl_deprecated_warning_given=false;
    m_getParamCl_deprecated_warning_given=false;
  }
 protected:
  //---------------------------------------------------------------
  // constructor for clocked channels used by clocked wrapper
  //---------------------------------------------------------------

  OCP_TL1_Channel(sc_core::sc_module_name name)
    : TL_Channel<TdataCl>(name,true,true,true,false),
	m_confMan( this->name(), *m_ParamCl),
    m_addressMask(0x3),
    m_dataMask(0xFFFFFFFF),
    m_exitAfterOCPMon(false),
    m_clocked(true),
    m_clk(NULL),
    m_in_clk(NULL),
    m_sig_clk(NULL),
    master_timing_sensitive(false),
    slave_timing_sensitive(false)
  {

#ifndef NDEBUG
    m_doRunCheck = true;
#else
    m_doRunCheck = false;
#endif
    m_no_pipelined_sanity_checks=false;
    m_getDataCl_deprecated_warning_given=false;
    m_getParamCl_deprecated_warning_given=false;
  }
 public:
  //---------------------------------------------------------------
  // Simple constructor for non-clocked channels
  //---------------------------------------------------------------
  OCP_TL1_Channel(std::string name,
                  bool use_event,
                  bool use_default_event = true
                  )
    : TL_Channel<TdataCl>(name.c_str(),true,use_event,use_default_event, false),
	  m_confMan(this->name(), *m_ParamCl),
      m_addressMask(0x3),
      m_dataMask(0xFFFFFFFF),
      m_exitAfterOCPMon(false),
      m_clocked(false),
      m_clk(NULL),
      m_in_clk(NULL),
      m_sig_clk(NULL),
      master_timing_sensitive(false),
      slave_timing_sensitive(false)
  {
#ifndef NDEBUG
    m_doRunCheck = true;
#else
    m_doRunCheck = false;
#endif
    m_no_pipelined_sanity_checks=false;
    m_getDataCl_deprecated_warning_given=false;
    m_getParamCl_deprecated_warning_given=false;
  }

  //---------------------------------------------------------------
  // destructor
  //---------------------------------------------------------------
  virtual ~OCP_TL1_Channel()
  {
  }

public:

  // -----------------------------------------------------------------
  // Basic Channel Methods
  // -----------------------------------------------------------------
  virtual void register_port(sc_core::sc_port_base& port, const char* if_typename)
  { 
     std::string nm( if_typename ); 
     if( nm == typeid( OCP_TL1_MasterIF<TdataCl> ).name() ) {
       m_master_name = port.name();
     }
     if( nm == typeid( OCP_TL1_SlaveIF<TdataCl> ).name() ) {
       m_slave_name = port.name();
     }
     TL_Channel<TdataCl>::register_port(port,if_typename); 
  }

  /////////////////////////////////////////
  // OCP Specific Commands
  /////////////////////////////////////////

  //---------------------------------------------------------------------
  // Configuration Functions
  //---------------------------------------------------------------------

  void
  setConfiguration( MapStringType& passedMap )
  {
    // Use the passed data base to set up the configuration class
    // NOTE: no "prefix" needed, "name()" passed for debugging
    //m_ParamCl->setOCPConfiguration( name(), passedMap );
	m_confMan.setExternalConfiguration(passedMap);

    // Set up the address mask.
    m_addressMask=0;
/*      for (int i=0; i<(m_ParamCl->addr_wdth); i++) { */
/*        m_addressMask = m_addressMask << 1; */
/*        m_addressMask = m_addressMask +1; */
/*      } */

    // Set up the data mask.
    m_dataMask=0;
/*      for (int i=0; i<(m_ParamCl->data_wdth); i++) { */
/*        m_dataMask = m_dataMask << 1; */
/*        m_dataMask = m_dataMask +1; */
/*      } */

  }
  
  void setOCPMasterConfiguration(MapStringType& passedMap) {
    m_confMan.setMasterConfiguration(passedMap);
  }

  void setOCPSlaveConfiguration(MapStringType& passedMap) {
    m_confMan.setSlaveConfiguration(passedMap);  
  }
  
  void addOCPConfigurationListener(OCP_TL_Config_Listener& listener) {
    m_confMan.addConfigListener(listener);
  }
  
  // Deprecated
  void setPeriod( const sc_core::sc_time& period ) {

  }
  // Deprecated
  const sc_core::sc_time& getPeriod() const {
    return sc_core::SC_ZERO_TIME;
  }

  //---------------------------------------------------------------------
  // Timing Functions
  //---------------------------------------------------------------------
  //
  void ocpWait(double count = 1) {
    if (m_clocked){
      for (int i=0;i<count;i++)
	if (m_clk!=NULL)
	  sc_core::wait(m_clk->posedge_event()); // For clocked channel
	else if (m_in_clk!=NULL)
	  sc_core::wait(m_in_clk->posedge_event()); // For clocked channel
	else if (m_sig_clk!=NULL)
	  sc_core::wait(m_sig_clk->posedge_event()); // For clocked channel
	else {
	  fatalError("ocpWait()", "Only clocked channels can use ocpWait");
	}
    }
    else {
      fatalError("ocpWait()", "Only clocked channels can use ocpWait");
    }
  }

  // Handle the case of reset during a longer sc_core::wait
  // true if successful sc_core::wait. false if interrupted by reset.
  bool ocpSafeWait(double count = 1) {
    if (this->get_reset()) {
      return false;
    }
    if (m_clocked){
      for (int i=0;i<count;i++)
	if (m_clk!=NULL)
	  sc_core::wait(m_clk->posedge_event()|m_CommCl->ResetStartEvent);
	else if (m_in_clk!=NULL)
	  sc_core::wait(m_in_clk->posedge_event()|m_CommCl->ResetStartEvent);
	else if (m_sig_clk!=NULL)
	  sc_core::wait(m_sig_clk->posedge_event()|m_CommCl->ResetStartEvent);
	else {
	  return false;
	}
    }
    else
      return false;
    return (! (this->get_reset()) );
  }

  //////////////////////////////////////////////////////////////
  // OCP TL1 Methods
  //////////////////////////////////////////////////////////////

  // -----------------------------------------------------------------
  // OCP TL1 commands with overloaded parameters
  // -----------------------------------------------------------------
  bool getOCPResponse( OCPResponseGrp<Td>& myResponse )
  { return getOCPResponse(myResponse, false); }
  bool getOCPResponseBlocking( OCPResponseGrp<Td>& myResponse )
  { return getOCPResponseBlocking(myResponse, false); }
  bool getOCPRequest( OCPRequestGrp<Td,Ta>& myRequest )
  { return getOCPRequest(myRequest, false); }
  bool getOCPRequestBlocking( OCPRequestGrp<Td,Ta>& myRequest )
  { return getOCPRequestBlocking(myRequest, false); }
  bool getOCPDataHS( OCPDataHSGrp<Td>& myData )
  { return getOCPDataHS(myData, false); }
  bool getOCPDataHSBlocking( OCPDataHSGrp<Td>& myData )
  { return getOCPDataHSBlocking(myData, false); }


  //---------------------------------------------------------------------
  // public methods for the OCP Request phase
  //---------------------------------------------------------------------

  bool getSBusy(void) const
  {
      return (TL_Channel<TdataCl>::MgetSbusy());
  }

  bool
  startOCPRequest(OCPRequestGrp<Td,Ta>& newRequest)
  {
    // is the channel in reset?
    if (this->get_reset()) {
      // In reset. Give up.
      return false;
    }
    // Is the thread free?
    if (m_ParamCl->sthreadbusy_exact)
    {
      // SThreadBusy is part of the channel and we are required to check it.
      // Now we need to test to see if the thread of the request is busy.
      if ( m_ParamCl->sthreadbusy_pipelined && m_ParamCl->sthreadbusy_exact) {
          if (!m_no_pipelined_sanity_checks){
              if (!m_clocked) {
                 pipelinedThreadButNotClockedWarning();
                 m_no_pipelined_sanity_checks=true;
              }
              else {
                if ( m_sthreadbusy_pipelined->isThisThreadBusy(newRequest.MThreadID)) 
                  return false;
              }
          }
      }
      else
          if ( m_DataCl->isThisThreadBusy(newRequest.MThreadID, getSThreadBusy()) )
          {
        // The thread is busy. Fail.
    // #ifndef NDEBUG
        //               cout << name() << ": startOCPRequest failed. It was called for a busy thread on a channel with sthreadbusy_exact." << endl;
    // #endif
          return false;
          }
    }

    // Mark that a new request is going on the channel
    if ( !(this->MputRequest()) )
    {
      return false;
    }

    // Removed from this release
    // Make sure the data & address fits the channel by masking off extra bits.
    //    newRequest.MData &= m_dataMask;
    //    newRequest.MAddr &= m_addressMask;

    // Load the new request info onto the channel
    m_DataCl->MputRequestGroup(newRequest);

    return true;
  }

  // Don't use with multithreaded interfaces
  bool startOCPRequestBlocking(OCPRequestGrp<Td,Ta>& newRequest)
  {
    bool tmp = true;
    if (m_clocked) // Clocked channel
    {
      tmp = startOCPRequest(newRequest);
      while (!tmp)
      {
	ocpWait();
	if (this->get_reset())
	{
	  // In reset. Give up.
	  return false;
	}
	tmp = startOCPRequest(newRequest);
      }
      return (tmp);
    }
    else
    {
      fatalError("startOCPRequestBlocking()", "Only clocked channels can use blocking calls");
      return false;
    }
  }

  bool getSCmdAccept(void) const
  {
    if (!m_ParamCl->cmdaccept)
      return(true);
    else
      return (!getSBusy());
  }

  void
  waitSCmdAccept(void)
  {
    // is the channel in reset?
    if (!this->get_reset()) {
      if (!getSBusy() || !m_ParamCl->cmdaccept){
	// The command has already been accepted
	return;
      }
      sc_core::wait(m_CommCl->RequestEndEvent);
    }
  }

  void
  waitSDataAccept(void)
  {
    // is the channel in reset?
    if (!this->get_reset()) {
      if (!getSBusyDataHS() || !m_ParamCl->dataaccept) {
        // The data has already been accepted
        return;
      }
      sc_core::wait(m_CommCl->DataRequestEndEvent);
    }
  }

  void
  waitMRespAccept(void)
  {
    // is the channel in reset?
    if (!this->get_reset()) {
      if (!getMBusy() || !m_ParamCl->respaccept) {
        // The command has already been accepted
        return;
      }
      sc_core::wait(m_CommCl->ResponseEndEvent);
    }
  }

  unsigned int
  getSThreadBusy(void) const
  {
    // is the channel in reset?
    if (m_CommCl->Reset) {
      // In reset. Give up.
      return false;
    }
    // The signal sthreadbusy must be part of the channel
    // for this function to make sense.
    assert( m_ParamCl->sthreadbusy );

    if (m_ParamCl->sthreadbusy_exact && m_ParamCl->sthreadbusy_pipelined && !m_no_pipelined_sanity_checks){
      if(!m_clocked) {
        pipelinedThreadButNotClockedWarning();
        return m_DataCl->MgetSThreadBusy(name(), true); //since it's pipelined we disable same-delta-cycle-access-warning 
      }
      return m_sthreadbusy_pipelined->getThisCyclesThreadBusy();
    }
    else
      return m_DataCl->MgetSThreadBusy(name(), !m_ParamCl->sthreadbusy_exact || m_ParamCl->sthreadbusy_pipelined);
  }

  unsigned int
  getCurrentSThreadBusy(void) const
  {
    return m_DataCl->MgetSThreadBusy(name(), true);    
  }

  void
  putSThreadBusy(unsigned int sthreadbusy)
  {
    // is the channel in reset?
    if (!this->get_reset()) {
      // The signal sthreadbusy must be part of the channel
      // for this function to make sense.
      assert( m_ParamCl->sthreadbusy );

      m_DataCl->SputSThreadBusy( sthreadbusy,name(), !m_ParamCl->sthreadbusy_exact | m_ParamCl->sthreadbusy_pipelined);
      if (!m_no_pipelined_sanity_checks && m_ParamCl->sthreadbusy_pipelined && m_ParamCl->sthreadbusy_exact){
          if (!m_clocked) {
             pipelinedThreadButNotClockedWarning();
             m_no_pipelined_sanity_checks=true;
          }
          else
              if(m_ParamCl->sthreadbusy_pipelined){
                m_sthreadbusy_pipelined->setNewValue(sthreadbusy);
              }
      }
    }
  }

  const sc_core::sc_event& SThreadBusyEvent(void) const
    { 
    if(m_ParamCl->sthreadbusy_pipelined && m_ParamCl->sthreadbusy_exact && !m_no_pipelined_sanity_checks){
      if(!m_clocked){
            pipelinedThreadButNotClockedWarning();
            return (m_DataCl->m_SThreadBusy.event);
      }
      return m_sthreadbusy_pipelined->delayedChangeEvent();
    }
    return (m_DataCl->m_SThreadBusy.event);
    }

  const sc_core::sc_event& CurrentSThreadBusyEvent(void) const
  { return (m_DataCl->m_SThreadBusy.event); }


  bool getOCPRequest(OCPRequestGrp<Td,Ta>& myRequest, bool acceptRequest=0)
  {
    // is the channel in reset?
    if (this->get_reset()) {
      // In reset. Give up.
      return false;
    }
    if ( !this->SgetRequest(acceptRequest) ){
      // Could not get a request. Maybe there is not one available.
      return false;
    }
    // We got the request - now get the request info for the user
    myRequest = m_DataCl->SgetRequestGroup();
    m_DataCl->SgetMCmd();
    return true;
  }

  // Don't use with multithreaded interfaces
  bool getOCPRequestBlocking(OCPRequestGrp<Td,Ta>& myRequest, bool acceptRequest=0)
  {
    // is the channel in reset?
    if (this->get_reset()) {
      // In reset. Give up.
      return false;
    }
    // Is there a blocking get request already in progress?
    // If so, that getRequestBlocking command has priority to read first.

    // REMOVED r2.1.2. No multiple callers allowed.
    //if ( m_CommCl->BlockingGetRequestPending ) {
    //  return false;
    //}
    // We are the blocking getRequest. Mark it so that no one else can go.
    // m_CommCl->BlockingGetRequestPending = true;

    // Is there a request for us to read?
    if ( !(m_CommCl->RequestStart) || !(m_CommCl->RequestUnread) ) {
      // No request yet. Must be patient.
      sc_core::wait(m_CommCl->RequestStartEvent);
      if (this->get_reset()) {
	// The channel has been reset. Give up.
	return false;
      }
    }

    // There should be a request now
    assert( m_CommCl->RequestStart == true );
    assert( m_CommCl->RequestUnread == true );

    // Blocking is over - get the request normally
    // m_CommCl->BlockingGetRequestPending = false;
    // get and return the request
    return getOCPRequest(myRequest, acceptRequest);
  }

  bool
  putSCmdAccept()
  {
    // is the channel in reset?
    if (this->get_reset()) {
      // In reset. Give up.
      return false;
    }
    // Was this command called at the correct time?
    if (! m_CommCl->RequestStart ) {
#ifndef NDEBUG
      cout << name() << ": WARNING - putSCmdAccept called when there was no request on the channel." << endl;
#endif
      return false;
      }
    this->Srelease();
    return true;
  }

  void
  putSCmdAccept(bool scmdaccept)
  {
    // is the channel in reset?
    if (!this->get_reset()) {
      if (scmdaccept)
	this->SreleasePE();
      else
	this->SunreleasePE();
    }
  }

  const sc_core::sc_event& RequestStartEvent(void) const
  { return (m_CommCl->RequestStartEvent); }

  const sc_core::sc_event& RequestEndEvent(void) const
  { return (m_CommCl->RequestEndEvent); }

  const sc_core::sc_event& RequestReleaseEvent(void) const
  { return (m_CommCl->RequestReleaseEvent); }

  //---------------------------------------------------------------------
  // public methods for the OCP Response Phase
  //---------------------------------------------------------------------


  bool getMBusy(void) const
  {
    return (TL_Channel<TdataCl>::SgetMbusy());
  }

  bool startOCPResponse(OCPResponseGrp<Td>& newResponse)
  {
    // is the channel in reset?
    if (this->get_reset()) {
      // In reset. Give up.
      return false;
    }
    // Is the thread free?
    if (m_ParamCl->mthreadbusy_exact)  {
      if ( m_ParamCl->mthreadbusy_pipelined && m_ParamCl->mthreadbusy_exact) {
          if (!m_no_pipelined_sanity_checks){
              if (!m_clocked) {
                 pipelinedThreadButNotClockedWarning();
                 m_no_pipelined_sanity_checks=true;
              }
              else
                  if ( m_mthreadbusy_pipelined->isThisThreadBusy(newResponse.SThreadID)) 
                    return false;
          }
      }
      else
          // MThreadBusy is part of the channel and we are required to check it.
          // Now we need to test to see if the thread is busy.
          if ( m_DataCl->isThisThreadBusy(newResponse.SThreadID, getMThreadBusy()) ){
        // The thread is busy. Fail.
    #ifdef DEBUG_C
        cout << name() << ": startOCPResponse failed. It was called for a busy thread on a channel with mthreadbusy_exact." << endl;
    #endif
        return false;
          }
    }

#ifndef NDEBUG
    // Is the thread free?
    if (m_ParamCl->mthreadbusy)
      {
	// MThreadBusy is part of the channel.
	// Now we test to see if the thread of the request is busy.
	if ( m_DataCl->isThisThreadBusy(newResponse.SThreadID, getMThreadBusy()) ) {
	  // The thread is busy. Warn the user.
	  cout << name() << ": WARNING - startOCPResponse called for a busy thread." << endl;
	}
      }
#endif

    // Mark that a new response is going on the channel
    if ( !(this->SputResponse()) ) {
      // Could not start the response
      return false;
    }

    // #ifndef NDEBUG
    //    cout << name() << ": startOCPResponse placed the new response on the channel." << endl;
    // #endif

    // Removed from this release
    // Make sure the data fits on the channel
    //    newResponse.SData &= m_dataMask;

    // The response is ours to send. Put our data unto the channel
    m_DataCl->SputResponseGroup(newResponse);

    // All done
    return true;
  }

  // Don't use with multithreaded interfaces
  bool
  startOCPResponseBlocking(OCPResponseGrp<Td>& newResponse)
  {
    bool tmp = true;
    if (m_clocked) // Clocked channel
    {
      tmp = startOCPResponse(newResponse);
      while (!tmp)
      {
	ocpWait();
	if (this->get_reset())
	{
	  // In reset. Give up.
	  return false;
	}
	tmp = startOCPResponse(newResponse);
      }
      return (tmp);
    }
    else
    {
      fatalError("startOCPResponseBlocking()", "Only clocked channels can use blocking calls");

      return false;
    }
    // we are done
    return true;
  }

  bool
  getMRespAccept(void) const
  {
    if (!m_ParamCl->respaccept) {
      return(true);
    }
    else {
      return(!getMBusy());
    }
  }


  unsigned int getMThreadBusy(void) const
  {
    // is the channel in reset?
    if (m_CommCl->Reset) {
      // In reset. Give up.
      return false;
    }
    // There is trouble here if this function is being called on a channel
    // that does not have mthreadbusy signal.
    assert( m_ParamCl->mthreadbusy == true);

    if (m_ParamCl->mthreadbusy_exact && m_ParamCl->mthreadbusy_pipelined && !m_no_pipelined_sanity_checks){
      if(!m_clocked) {
        pipelinedThreadButNotClockedWarning();
        return m_DataCl->SgetMThreadBusy(name(), true); //since it's pipelined we disable same-delta-cycle-access-warning 
      }
      return m_mthreadbusy_pipelined->getThisCyclesThreadBusy();
    }
    else
      return m_DataCl->SgetMThreadBusy(name(),m_ParamCl->mthreadbusy_pipelined | !m_ParamCl->mthreadbusy_exact);
  }

  unsigned int getCurrentMThreadBusy(void) const
  {
    return m_DataCl->SgetMThreadBusy(name(), true);    
  }

  void
  putMThreadBusy(unsigned int mthreadbusy)
  {
    // is the channel in reset?
    if (!this->get_reset()) {
      // There is trouble here if this function is being called on a channel
      // that does not have mthreadbusy signal.
      assert( m_ParamCl->mthreadbusy == true);

      m_DataCl->MputMThreadBusy( mthreadbusy,name(),!m_ParamCl->mthreadbusy_exact | m_ParamCl->mthreadbusy_pipelined );
      if (!m_no_pipelined_sanity_checks && m_ParamCl->mthreadbusy_pipelined && m_ParamCl->mthreadbusy_exact){
          if (!m_clocked) {
             pipelinedThreadButNotClockedWarning();
             m_no_pipelined_sanity_checks=true;
          }
          else
              if(m_ParamCl->mthreadbusy_pipelined){
                m_mthreadbusy_pipelined->setNewValue(mthreadbusy);
              }
      }
    }
  }

  const sc_core::sc_event& MThreadBusyEvent(void) const
  { 
    if(m_ParamCl->mthreadbusy_pipelined && m_ParamCl->mthreadbusy_exact && !m_no_pipelined_sanity_checks){
      if(!m_clocked){
          pipelinedThreadButNotClockedWarning();
          return (m_DataCl->m_MThreadBusy.event);
      }
      return m_mthreadbusy_pipelined->delayedChangeEvent();
    }
    return (m_DataCl->m_MThreadBusy.event); 
  }

  const sc_core::sc_event& CurrentMThreadBusyEvent(void) const
  { return (m_DataCl->m_MThreadBusy.event);}

  bool
  getOCPResponse(OCPResponseGrp<Td>& newResp, bool acceptResponse=0)
  {
    // is the channel in reset?
    if (this->get_reset()) {
      // In reset. Give up.
      return false;
    }
    if ( !this->MgetResponse(acceptResponse) )  {
      // Could not get a response. Maybe there is not one available.
      return false;
    }

    // We got a response: now get the data out of the channel
    newResp = m_DataCl->MgetResponseGroup();
    m_DataCl->MgetSResp();
    return true;
  }

  // Don't use with multithreaded interfaces
  bool
  getOCPResponseBlocking(OCPResponseGrp<Td>& newResp, bool acceptResponse=0)
  {
    // is the channel in reset?
    if (this->get_reset()) {
      // In reset. Give up.
      return false;
    }
    // Is there a blocking get response already in progress?
    // If so, that getResponseBlocking command has priority to read first.
    // REMOVED r2.2. No multiple callers allowed.
    //if ( m_CommCl->BlockingGetResponsePending ) {
    //  return false;
    //}
    // We are the blocking getResponse. Mark it so that no one else can go.
    //m_CommCl->BlockingGetResponsePending = true;

    // Is there a response for us to read?
    if ( !(m_CommCl->ResponseStart) || !(m_CommCl->ResponseUnread) ) {
      // No response yet. Must be patient.
      sc_core::wait(m_CommCl->ResponseStartEvent);
      // Did the channel get reset while we were waiting?
      if (this->get_reset()) {
	// The channel has been reset. Give up.
	return false;
      }
    }

    // There should be a response now
    assert( m_CommCl->ResponseStart == true );
    assert( m_CommCl->ResponseUnread == true );

    // Blocking is over - get the response normally
    //m_CommCl->BlockingGetResponsePending = false;
    // get and return the response
    return getOCPResponse(newResp, acceptResponse);
  }

  bool putMRespAccept()
  {
    // is the channel in reset?
    if (this->get_reset()) {
      // In reset. Give up.
      return false;
    }
    // Was this command called at the correct time?
    if (! m_CommCl->ResponseStart )
      {
#ifndef NDEBUG
	cout << name() << ": WARNING - putMRespAccept called when there was no response on the channel." << endl;
#endif
	return false;
      }
    this->Mrelease();
    return true;
  }

  void
  putMRespAccept(bool mrespaccept)
  {
    // is the channel in reset?
    if (!this->get_reset()) {
      if (mrespaccept)
	this->MreleasePE();
      else
	this->MunreleasePE();
    }
  }

  const sc_core::sc_event& ResponseStartEvent(void) const
  { return (m_CommCl->ResponseStartEvent); }

  const sc_core::sc_event& ResponseEndEvent(void) const
  { return (m_CommCl->ResponseEndEvent); }

  const sc_core::sc_event& ResponseReleaseEvent(void) const
  { return (m_CommCl->ResponseReleaseEvent); }

  //---------------------------------------------------------------------
  // public methods for the OCP Data Handshake Phase
  //---------------------------------------------------------------------

  bool getSBusyDataHS(void) const
  {
    return (TL_Channel<TdataCl>::MgetSbusyData());
  }

  bool
  startOCPDataHS(OCPDataHSGrp<Td>& datahs)
  {
    // is the channel in reset?
    if (this->get_reset()) {
      // In reset. Give up.
      return false;
    }
    // Is the thread free?
    if (m_ParamCl->sdatathreadbusy_exact) {
      if ( m_ParamCl->sdatathreadbusy_pipelined && m_ParamCl->sdatathreadbusy_exact) {
          if (!m_no_pipelined_sanity_checks){
              if (!m_clocked) {
                 pipelinedThreadButNotClockedWarning();
                 m_no_pipelined_sanity_checks=true;
              }
              else
                  if ( m_sdatathreadbusy_pipelined->isThisThreadBusy(datahs.MDataThreadID)) 
                    return false;
          }
      }
      else    
          // SDataThreadBusy is part of the channel and we are required to check it.
          // Now we need to test to see if the thread of the data handshake is busy.
          if ( m_DataCl->isThisThreadBusy(datahs.MDataThreadID, getSDataThreadBusy()) ) {
        // The thread is busy. Fail.
        // #ifndef NDEBUG
        // cout << name() << ": putData failed. It was called for a busy thread on a channel with sdatathreadbusy_exact." << endl;
        // #endif
        return false;
          }
    }

    // Mark that new data is going on the channel
    if ( !(this->MputDataRequest()) ){
      return false;
    }
    // Removed  from this release
    // Make sure the data fits on the channel
    //    datahs.MData &= m_dataMask;

    // Load the new request info onto the channel
    m_DataCl->MputDataHSGroup(datahs);

    return true;
  }

  // Don't use with multithreaded interfaces
  bool
  startOCPDataHSBlocking(OCPDataHSGrp<Td>& newData)
  {
    bool tmp = true;
    if (m_clocked) // Clocked channel
    {
      tmp = startOCPDataHS(newData);
      while (!tmp)
      {
	ocpWait();
	if (this->get_reset())
	{
	  // In reset. Give up.
	  return false;
	}
	tmp = startOCPDataHS(newData);
      }
      return (tmp);
    }
    else
    {
      fatalError("startOCPDataHSBlocking()", "Only clocked channels can use blocking calls");
      return false;
    }
    // We are done
    return true;
  }

  // NOTE: acceptData should default to false
  bool getOCPDataHS(OCPDataHSGrp<Td>& myDataHS, bool acceptData)
  {
    // is the channel in reset?
    if (this->get_reset()) {
      // In reset. Give up.
      return false;
    }
    if ( !this->SgetDataRequest(acceptData) ){
      // Could not get data. Maybe it is not available.
      return false;
    }

    // We got the data - now get the data hand shake info for the user
    myDataHS = m_DataCl->SgetDataHSGroup();
    m_DataCl->SgetMDataValid();
    return true;
  }

  // Don't use with multithreaded interfaces
  bool getOCPDataHSBlocking(OCPDataHSGrp<Td>& myDataHS, bool acceptData)
  {
    // is the channel in reset?
    if (this->get_reset()) {
      // In reset. Give up.
      return false;
    }
    // Is there a blocking get data already in progress?
    // If so, that getDataRequestBlocking command has priority to read first.
    // REMOVED r2.2. No multiple callers allowed.
    //if ( m_CommCl->BlockingGetDataRequestPending ) {
    //  return false;
    //}
    // We are the blocking getData. Mark it so that no one else can go.
    //m_CommCl->BlockingGetDataRequestPending = true;

    // Is there data for us to read?
    if ( !(m_CommCl->DataRequestStart) || !(m_CommCl->DataRequestUnread) ) {
      // No data yet. Must be patient.
      sc_core::wait(m_CommCl->DataRequestStartEvent);
      // Did the channel enter reset while we were waiting?
      if (this->get_reset()) {
	// The channel has been reset. Give up.
	return false;
      }
    }

    // There should be data now
    assert( m_CommCl->DataRequestStart == true );
    assert( m_CommCl->DataRequestUnread == true );

    // Blocking is over - get the data normally
    //m_CommCl->BlockingGetDataRequestPending = false;
    // get and return the data
    return getOCPDataHS(myDataHS, acceptData);
  }

  bool putSDataAccept()
  {
    // is the channel in reset?
    if (this->get_reset()) {
      // In reset. Give up.
      return false;
    }
    // Was this command called at the correct time?
    if (! m_CommCl->DataRequestStart ) {
#ifndef NDEBUG
      cout << name() << ": WARNING - putSDataAccept called when there was no data on the channel." << endl;
#endif
      return false;
    }
    this->SreleaseData();
    return true;
  }

  void
  putSDataAccept(bool dataaccept)
  {
    // is the channel in reset?
    if (!this->get_reset()) {
      if (dataaccept)
	this->SreleaseDataPE();
      else
	this->SunreleaseDataPE();
    }
  }

  bool getSDataAccept(void) const
  {
    //      return ( !(m_CommCl->DataRequestPending) && (m_CommCl->DataRequestAccepted
    //				    ||m_CommCl->DataRequestEarlyEnd) ) ;
    if (!m_ParamCl->dataaccept) {
      return (true);
    }
    else {
      return (!getSBusyDataHS());
    }
  }

  // Optional command provided to the verify the timing of the Master
  // NOTE: doesn't actually do anything
  // NOTE: Not required.
  void putNullMDataValid(void) const
  {
#ifndef NDEBUG
      if (m_DataCl->m_DataHSGrp[1 - m_DataCl->m_DataHSToggle].MDataValid == false )
      {
          cout << name() << ": MDataValid is set to Null" << endl;
      } else {
          cout << name() << ": WARNING - putNullMDataValid called and MDataValid is not false (Data Request still in progress?)" << endl;
      }
#endif
  }

  const sc_core::sc_event& DataHSStartEvent(void) const
  { return (m_CommCl->DataRequestStartEvent); }

  const sc_core::sc_event& DataHSEndEvent(void) const
  { return (m_CommCl->DataRequestEndEvent); }

  const sc_core::sc_event& DataHSReleaseEvent(void) const
  { return (m_CommCl->DataRequestReleaseEvent); }

  void putSDataThreadBusy(unsigned int sdatathreadbusy)
  {
    // is the channel in reset?
    if (!this->get_reset()) {
      // The sdatathreadbusy signal needs to be part of the
      // channel for this function to work
      assert(m_ParamCl->sdatathreadbusy == true);

      m_DataCl->SputSDataThreadBusy( sdatathreadbusy,name(), !m_ParamCl->sdatathreadbusy_exact | m_ParamCl->sdatathreadbusy_pipelined );
      if (!m_no_pipelined_sanity_checks && m_ParamCl->sdatathreadbusy_pipelined && m_ParamCl->sdatathreadbusy_exact){
          if (!m_clocked) {
             pipelinedThreadButNotClockedWarning();
             m_no_pipelined_sanity_checks=true;
          }
          else
              if(m_ParamCl->sdatathreadbusy_pipelined){
                m_sdatathreadbusy_pipelined->setNewValue(sdatathreadbusy);
              }
      }
    }
  }

  unsigned int getSDataThreadBusy(void) const
  {
    // is the channel in reset?
    if (m_CommCl->Reset) {
      // In reset. Give up.
      return false;
    }
    // The sdatathreadbusy signal must be part of the channel for this command
    // to function correctly
    assert( m_ParamCl->sdatathreadbusy == true );

    if (m_ParamCl->sdatathreadbusy_exact && m_ParamCl->sdatathreadbusy_pipelined && !m_no_pipelined_sanity_checks){
      if(!m_clocked) {
        //ToDo: call one const inline function with a complete warning
        pipelinedThreadButNotClockedWarning();
        return m_DataCl->MgetSDataThreadBusy(name(), true); //since it's pipelined we disable same-delta-cycle-access-warning 
      }
      return m_sdatathreadbusy_pipelined->getThisCyclesThreadBusy();
    }
    else
      return m_DataCl->MgetSDataThreadBusy(name(),m_ParamCl->sdatathreadbusy_pipelined | !m_ParamCl->sdatathreadbusy_exact);
  }

  unsigned int getCurrentSDataThreadBusy(void) const
  {
    return m_DataCl->MgetSDataThreadBusy(name(), true);    
  }

  const sc_core::sc_event& SDataThreadBusyEvent(void) const
  { 
    if(m_ParamCl->sdatathreadbusy_pipelined && m_ParamCl->sdatathreadbusy_exact && !m_no_pipelined_sanity_checks){
      if(!m_clocked){
          pipelinedThreadButNotClockedWarning();
          return (m_DataCl->m_SDataThreadBusy.event);
      }
      return m_sdatathreadbusy_pipelined->delayedChangeEvent();
    }
    return (m_DataCl->m_SDataThreadBusy.event); 
  }

  const sc_core::sc_event& CurrentSDataThreadBusyEvent(void) const
  { return (m_DataCl->m_SDataThreadBusy.event);}

  //---------------------------------------------------------------------
  // public Reset Commands
  //---------------------------------------------------------------------

  bool getReset(void)
  { return (this->get_reset()); }

  const sc_core::sc_event& ResetStartEvent(void) const
  { return (m_CommCl->ResetStartEvent); }

  const sc_core::sc_event& ResetEndEvent(void) const
  { return (m_CommCl->ResetEndEvent); }
  
  const sc_core::sc_event& getFakeResetEndEvent(void) const
  { return (fakeResetEndEvent); }

  const sc_core::sc_event& getFakeResetStartEvent(void) const
  { return (fakeResetStartEvent); }

  void MResetAssert(void)
  {
      if (! m_DataCl->m_MReset_n) {
          // Master has already caused a reset.
          // Do nothing.
          return;
      }

      assert(m_ParamCl->mreset);

      m_DataCl->m_MReset_n = false;
      m_DataCl->m_MResetStartTime= sc_core::sc_time_stamp();

      // TODO: This should be a separate function
      // If the channel is not already in reset, reset it.
      if (! this->get_reset()) {
          // reset the channel
          this->reset();
      }
      else
        fakeResetStartEvent.notify();
  }

  void MResetDeassert(void)
  {
      if (m_DataCl->m_MReset_n) {
          // Master is already out of reset
          // Do nothing.
          return;
      }

      assert(m_ParamCl->mreset);

      m_DataCl->m_MReset_n = true;

      if (m_DataCl->m_SReset_n) {
          // The slave reset has also been deasserted. Reset is over.
          assert(this->get_reset());  // Channel should be in reset mode.

          // reset the channel again to make sure all signals are re-initialized
          this->reset();
          // Now pull out of reset mode
          m_CommCl->Reset = false;
          m_CommCl->ResetEndEvent.notify();
      }
      else
        fakeResetEndEvent.notify();
      
  }

  void SResetAssert(void)
  {
      if (! m_DataCl->m_SReset_n) {
          // Slave has already caused a reset.
          // Do nothing.
          return;
      }

      assert(m_ParamCl->sreset);

      m_DataCl->m_SReset_n = false;
      m_DataCl->m_SResetStartTime= sc_core::sc_time_stamp();

      // TODO: This should be a separate function
      // If the channel is not already in reset, reset it.
      if (! this->get_reset()) {
          // reset the channel
          this->reset();
      }
      else
        fakeResetStartEvent.notify();
  }

  void SResetDeassert(void)
  {
      if (m_DataCl->m_SReset_n) {
          // Slave is already out of reset
          // Do nothing.
          return;
      }

      assert(m_ParamCl->sreset);

      m_DataCl->m_SReset_n = true;

      if (m_DataCl->m_MReset_n) {
          // The master reset has also been deasserted. Reset is over.
          assert(this->get_reset());  // Channel should be in reset mode.

          // reset the channel again to make sure all signals are re-initialized
          this->reset();
          // Now pull out of reset mode
          m_CommCl->Reset = false;
          m_CommCl->ResetEndEvent.notify();
      }
      else
        fakeResetEndEvent.notify();
  }

  //---------------------------------------------------------------------
  // public OCP Master Sideband Commands
  //---------------------------------------------------------------------
  void MputMError(bool nextValue)
  { m_DataCl->MputMError(nextValue); }
  void MputMFlag(sc_dt::uint64 nextValue)
  { m_DataCl->MputMFlag(nextValue); }
  void MputMFlag(sc_dt::uint64 nextValue,
          sc_dt::uint64 mask)
  { m_DataCl->MputMFlag(nextValue, mask); }
  bool MgetSError(void) const
  { return (m_DataCl->MgetSError()); }
  sc_dt::uint64 MgetSFlag(void) const
  { return (m_DataCl->MgetSFlag()); }
  bool MgetSInterrupt(void) const
  { return (m_DataCl->MgetSInterrupt()); }

  //---------------------------------------------------------------------
  // public OCP Master Sideband Events
  //---------------------------------------------------------------------
  const sc_core::sc_event& SidebandSErrorEvent(void) const
  { return (m_DataCl->SidebandSErrorEvent()); }
  const sc_core::sc_event& SidebandSFlagEvent(void) const
  { return (m_DataCl->SidebandSFlagEvent()); }
  const sc_core::sc_event& SidebandSInterruptEvent(void) const
  { return (m_DataCl->SidebandSInterruptEvent()); }

  //---------------------------------------------------------------------
  // public OCP Slave Sideband Commands
  //---------------------------------------------------------------------
  bool SgetMError(void) const
  { return (m_DataCl->SgetMError()); }
  sc_dt::uint64 SgetMFlag(void) const
  { return (m_DataCl->SgetMFlag()); }
  void SputSError(bool nextValue)
  { m_DataCl->SputSError(nextValue); }
  void SputSFlag(sc_dt::uint64 nextValue)
  { m_DataCl->SputSFlag(nextValue); }
  void SputSFlag(sc_dt::uint64 nextValue,
          sc_dt::uint64 mask)
  { m_DataCl->SputSFlag(nextValue, mask); }
  void SputSInterrupt(bool nextValue)
  { m_DataCl->SputSInterrupt(nextValue); }

  //---------------------------------------------------------------------
  // public OCP Slave Sideband Events
  //---------------------------------------------------------------------
  const sc_core::sc_event& SidebandMErrorEvent(void) const
  { return (m_DataCl->SidebandMErrorEvent()); }
  const sc_core::sc_event& SidebandMFlagEvent(void) const
  { return (m_DataCl->SidebandMFlagEvent()); }

  //---------------------------------------------------------------------
  // public OCP System Sideband Commands
  //---------------------------------------------------------------------
  bool SysputControl(int nextValue)
  { return m_DataCl->SysputControl(nextValue); }
  bool SysgetControlBusy(void) const
  { return (m_DataCl->SysgetControlBusy()); }
  void SysputControlWr(bool nextValue)
  { m_DataCl->SysputControlWr(nextValue); }
  int SysgetStatus(void) const
  { return (m_DataCl->SysgetStatus()); }
  bool readStatus(int& currentValue) const
  { return (m_DataCl->readStatus(currentValue)); }
  bool SysgetStatusBusy(void) const
  { return (m_DataCl->SysgetStatusBusy()); }
  void SysputStatusRd(bool nextValue)
  { m_DataCl->SysputStatusRd(nextValue); }

  //---------------------------------------------------------------------
  // public OCP System Sideband Events
  //---------------------------------------------------------------------
  const sc_core::sc_event& SidebandControlBusyEvent(void) const
  { return (m_DataCl->SidebandControlBusyEvent()); }
  const sc_core::sc_event& SidebandStatusEvent(void) const
  { return (m_DataCl->SidebandStatusEvent()); }
  const sc_core::sc_event& SidebandStatusBusyEvent(void) const
  { return (m_DataCl->SidebandStatusBusyEvent()); }

  //---------------------------------------------------------------------
  // public OCP Core Sideband Commands
  //---------------------------------------------------------------------
  int CgetControl(void) const
  { return (m_DataCl->CgetControl()); }
  void CputControlBusy(bool nextValue)
  { m_DataCl->CputControlBusy(nextValue); }
  bool CgetControlWr(void) const
  { return (m_DataCl->CgetControlWr()); }
  void CputStatus(int nextValue)
  { m_DataCl->CputStatus(nextValue); }
  void CputStatusBusy(bool nextValue)
  { m_DataCl->CputStatusBusy(nextValue); }
  bool CgetStatusRd(void) const
  { return (m_DataCl->CgetStatusRd()); }

  //---------------------------------------------------------------------
  // public OCP Core Sideband Events
  //---------------------------------------------------------------------
  const sc_core::sc_event& SidebandControlEvent(void) const
  { return (m_DataCl->SidebandControlEvent()); }
  const sc_core::sc_event& SidebandControlWrEvent(void) const
  { return (m_DataCl->SidebandControlWrEvent()); }
  const sc_core::sc_event& SidebandStatusRdEvent(void) const
  { return (m_DataCl->SidebandStatusRdEvent()); }


  // -----------------------------------------------------------------
  // OCP_TL1_MonitorIF
  // -----------------------------------------------------------------
  const std::string peekChannelName() const
  { return name(); }
  const std::string peekMasterPortName() const
  { return m_master_name; }
  const std::string peekSlavePortName()	const
  { return m_slave_name; }
  const OCPRequestGrp<Td,Ta>& peekOCPRequest() const 
  { 
    m_tmp_req = m_DataCl->SgetRequestGroup();
    return m_tmp_req; 
  }
  const OCPDataHSGrp<Td>& peekDataHS() const
  { 
    m_tmp_hs = m_DataCl->SgetDataHSGroup();
    return m_tmp_hs; 
  }
  const OCPResponseGrp<Td>& peekOCPResponse() const
  { 
    m_tmp_resp = m_DataCl->MgetResponseGroup(); 
    return m_tmp_resp;
  }

  const OCPMCmdType getMCmdTrace () const
  { 
    OCPMCmdType a = m_DataCl->getMCmdTrace();
    return (a);
  }

  const bool getMDataValidTrace () const
  { 
    bool a = m_DataCl->getMDataValidTrace();
    return (a);
  }

  const OCPSRespType getSRespTrace () const
  { 
    OCPSRespType a = m_DataCl->getSRespTrace();
    return (a);
  }

  const bool		peekRequestEnd()	const 
  { return m_CommCl->RequestEnd; }
  const bool		peekRequestStart()	const 
  { return m_CommCl->RequestStart; }
  const bool		peekRequestEarlyEnd()	const 
  { return m_CommCl->RequestEarlyEnd; }
 
  const bool		peekResponseEnd()	const 
  { return m_CommCl->ResponseEnd; }
  const bool		peekResponseStart()	const 
  { return m_CommCl->ResponseStart; }
  const bool		peekResponseEarlyEnd()	const 
  { return m_CommCl->ResponseEarlyEnd; }

  const bool		peekDataRequestEnd()	const 
  { return m_CommCl->DataRequestEnd; }
  const bool		peekDataRequestStart()	const 
  { return m_CommCl->DataRequestStart; }
  const bool		peekDataRequestEarlyEnd() const
  { return m_CommCl->DataRequestEarlyEnd; }

  const unsigned int	peekSThreadBusy()	const 
  { 
    return m_DataCl->MgetLastSThreadBusy();
  }
  const unsigned int	peekSDataThreadBusy()	const 
  { 
    return m_DataCl->MgetLastSDataThreadBusy();
  }
  const unsigned int	peekMThreadBusy()	const 
  { 
    return m_DataCl->SgetLastMThreadBusy();
  }

 
  const bool		peekMReset_n()		const 
  { return m_DataCl->m_MReset_n; }
  const bool		peekSReset_n()		const 
  { return m_DataCl->m_SReset_n; }

  const bool		peekMError()		const 
  { return m_DataCl->m_MError; }
  const unsigned int	peekMFlag()		const 
  { return m_DataCl->m_MFlag; }
  const bool		peekSError()		const 
  { return m_DataCl->m_SError; }
  const unsigned int	peekSFlag()		const 
  { return m_DataCl->m_SFlag; }
  const bool		peekSInterrupt()	const 
  { return m_DataCl->m_SInterrupt; }
  const unsigned int	peekControl()		const 
  { return m_DataCl->m_Control; }
  const bool		peekControlWr()		const 
  { return m_DataCl->m_ControlWr; }
  const bool		peekControlBusy()	const 
  { return m_DataCl->m_ControlBusy; }
  const unsigned int	peekStatus()		const 
  { return m_DataCl->m_Status; }
  const bool		peekStatusRd()		const 
  { return m_DataCl->m_StatusRd; }
  const bool		peekStatusBusy()	const
  { return m_DataCl->m_StatusBusy; }
  const bool		peekExitAfterOCPMon()	const 
  { return m_exitAfterOCPMon; }

  ///////////////////////////////////
  // Port Access to the Generic commands in the underlying Generic Channel
  // Generic commands kept for backward compatibility
  ///////////////////////////////////

  // -----------------------------------------------------------------
  // Generic Direct Channel Access Methods, deprecated
  // -----------------------------------------------------------------

  // -----------------------------------------------------------------
  // Direct Access to the internals of the channel.
  // Necessary for the use of the Generic Commands.
  // Note: use with caution.
  // -----------------------------------------------------------------
  TdataCl* GetDataCl(void)
  { 
    if (!m_getDataCl_deprecated_warning_given){
      cout<<"WARNING: Detected use of GetDataCl in channel "<<TL_Channel<TdataCl>::name()<<", which is deprecated."<<endl
          <<"         GetDataCl will be removed from the channel interface in upcoming releases."<<endl<<flush;
      m_getDataCl_deprecated_warning_given=true;
    }
    return (TL_Channel<TdataCl>::GetDataCl()); 
  }
  ParamCl<TdataCl>* GetParamCl(void)
  { 
    if (!m_getParamCl_deprecated_warning_given){
      cout<<"WARNING: Detected use of GetParamCl in channel "<<TL_Channel<TdataCl>::name()<<", which is deprecated."<<endl
          <<"         GetParamCl will be removed from the channel interface in upcoming releases."<<endl
          <<"         Use the configuration from cores option or getParameters instead."<<endl<<flush;
      m_getParamCl_deprecated_warning_given=true;
    }  
    return (TL_Channel<TdataCl>::GetParamCl()); 
  }

  OCPParameters* GetParameters(void)
  { 
    return (static_cast<OCPParameters*> (TL_Channel<TdataCl>::GetParamCl())); 
  }
  
  CommCl* GetCommCl(void)
  { return (TL_Channel<TdataCl>::GetCommCl()); }

  // -----------------------------------------------------------------
  // Generic Channel Methods Deprecated
  // -----------------------------------------------------------------

  // -----------------------------------------------------------------
  // Generic Channel Events
  // -----------------------------------------------------------------
  const sc_core::sc_event& default_event() const
  { return (TL_Channel<TdataCl>::default_event()); }


protected:
  //---------------------------------------------------------------
  // protected methods
  //---------------------------------------------------------------

  // display a fatal error message and quick the program
  void fatalError(char* sub_title, char* msg)
  {
    cout << endl << name() << ": ERROR: " << sub_title << ":" << msg << endl;
    assert(0);
  }
  
  void pipelinedThreadButNotClockedWarning() const {
    cout<<"WARNING: Pipelined thread busy is used but the channel is not clocked. The effects are:"<<endl
        <<"         - getXThreadBusy will behave as if it was not pipelined."<<endl
        <<"         - getCurrentXThreadBusy will behave as if it was not pipelined."<<endl
        <<"         - XThreadBusyEvent will behave as if it was not pipelined."<<endl
        <<"         - CurrentXThreadBusyEvent will behave as if it was not pipelined."<<endl
        <<"         - Sanity checks concerning busy signals can not be performed."<<endl
        <<flush;
  }

  //----------------------------------------------------------------------
  // member data
  //----------------------------------------------------------------------

  bool m_doRunCheck;

  Config_manager m_confMan;

  // Address Mask - used to make sure user address is not to big for the channel.
  Ta m_addressMask;

  // Data Mask - used to make sure user data is not to big for the channel.
  Td m_dataMask;

  // Flag - set if a fatal error has occured and the simulation should be stopped
  // once the last line of the OCP Trace File has been output (for debugging).
  bool m_exitAfterOCPMon;

  // Are blocking calls clocked?
  bool m_clocked;
  sc_core::sc_clock * m_clk;
  sc_core::sc_in_clk * m_in_clk;
  sc_core::sc_signal<bool> * m_sig_clk;

  std::string m_master_name;
  std::string m_slave_name;
  
  OCP_TL1_Thread_pipelined_support* m_sthreadbusy_pipelined;  
  OCP_TL1_Thread_pipelined_support* m_sdatathreadbusy_pipelined;  
  OCP_TL1_Thread_pipelined_support* m_mthreadbusy_pipelined;
  bool m_no_pipelined_sanity_checks;
  
  bool m_getDataCl_deprecated_warning_given;
  bool m_getParamCl_deprecated_warning_given;
  
  sc_core::sc_event fakeResetEndEvent, fakeResetStartEvent;

  // helper members for monitor interface
  mutable OCPRequestGrp<Td,Ta> m_tmp_req;
  mutable OCPResponseGrp<Td> m_tmp_resp;
  mutable OCPDataHSGrp<Td> m_tmp_hs;

  // Timing distribution stuff
  bool master_timing_sensitive, slave_timing_sensitive;
  OCP_TL1_Master_TimingCl master_timing;
  OCP_TL1_Slave_TimingCl slave_timing;
  OCP_TL1_Slave_TimingCl default_timing;
  OCP_TL1_Slave_TimingIF *sensitive_master;
  OCP_TL1_Master_TimingIF *sensitive_slave;

public:
  // Timing distribution methods
  void setOCPTL1MasterTiming(OCP_TL1_Master_TimingCl master_timing_p) {
    if(!(master_timing == master_timing_p)) {
      master_timing = master_timing_p;
      if(slave_timing_sensitive) {
        sensitive_slave->setOCPTL1MasterTiming(master_timing);
      }
    }
  }

  void setOCPTL1SlaveTiming(OCP_TL1_Slave_TimingCl slave_timing_p) {
    if(!(slave_timing == slave_timing_p)) {
      slave_timing = slave_timing_p;
      if(master_timing_sensitive) {
        sensitive_master->setOCPTL1SlaveTiming(slave_timing);
      }
    }
  }

  void registerTimingSensitiveOCPTL1Master(OCP_TL1_Slave_TimingIF *master_p) {
    master_timing_sensitive = true;
    sensitive_master = master_p;
    OCP_TL1_Slave_TimingCl default_timing;
    if(!(slave_timing == default_timing)) {
      sensitive_master->setOCPTL1SlaveTiming(slave_timing);
    }
  }

  void registerTimingSensitiveOCPTL1Slave(OCP_TL1_Master_TimingIF *slave_p) {
    slave_timing_sensitive = true;
    sensitive_slave = slave_p;
    OCP_TL1_Master_TimingCl default_timing;
    if(!(master_timing == default_timing)) {
      sensitive_slave->setOCPTL1MasterTiming(master_timing);
    }
  }

};

#endif
