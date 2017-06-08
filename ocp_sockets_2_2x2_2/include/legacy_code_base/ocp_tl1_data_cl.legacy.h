// 
//  (c) Copyright OCP-IP 2003, 2004, 2005
//  OCP-IP Confidential and Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//      Authors : Norman Weyrich, Synopsys Inc.
//                Anssi Haverinen, Nokia Inc.
//                Alan Kamas, for Sonics Inc.
//                Joe Chou, Sonics Inc.
//          $Id:
//
//  Description : Transaction Level - Layer-1 data class for
//                the Open Core Protocol (OCP) 2.0.
//                This is class is used in two ways. It is used in 
//                conjunction with the base generic channel tl_channel.h) 
//                to transport OCP formatted data between Initiator and 
//                Target modules. It is also used in the OCP specific
//                TL1 channel (ocp_tl1_channel.h) to implement an OCP
//                correct cycle-accurate channel model.
//
//                For more information on OCP, please see the
//                Open Core Protocol Specification document available
//                from www.ocpip.org.
//
// Change History:
//
// 12/14/2003 Original revision
// 03/30/2004 Changes MCmd, MDataValid, SResp and
//            OCP trace behavior.
//            
// ============================================================================

// This file contains the definition of the classes:
// template <class Td, class Ta> class OCP_TL1_DataCl
// 
// Template parameters Td: OCP data field type, Ta: OCP address field type 
// See "ocp_globals.h" for details

#ifndef __OCP_TL1_DATA_CL_LEGACY_H__
#define __OCP_TL1_DATA_CL_LEGACY_H__

// A structure for tracking Threadbusy changes
// and for forcing updates precisely at the clock edge.
struct ThreadBusyStruct 
{
    unsigned int currentValue;
    unsigned int nextDeltaValue;
    bool setAtNextDelta;
    unsigned int nextTickValue;
    sc_core::sc_time nextTickTime;
    bool setAtNextTick;
    unsigned int lastValue;
    sc_core::sc_time lastSavedTime;
    sc_core::sc_event event;
    unsigned int lastChangeDelta;
    unsigned int lastCheckDelta;
    void reset(void) 
    {
        currentValue = 0;
        nextDeltaValue = 0;
        setAtNextDelta = false;
        nextTickValue = 0;
        nextTickTime = sc_core::SC_ZERO_TIME;
        setAtNextTick = false;
        lastValue = 0;
        lastSavedTime = sc_core::SC_ZERO_TIME;
        lastChangeDelta = 0;
        lastCheckDelta = 0;
    };
    void update(const sc_core::sc_time& timeNow) {
        if (setAtNextTick && (nextTickTime <= timeNow)) {
            // Current time has caught up to our update time. Do the update.
            setAtNextTick = false;
            lastValue = currentValue;
            lastSavedTime = timeNow;
            currentValue = nextTickValue;
            event.notify();
        }
    };
};

#if defined (OSCI20) || defined (OSCI21V1)
#define sc_process_handle sc_process_b*
#define sc_get_current_process_handle sc_get_curr_process_handle
#else
class procHandleCmp {
  public:
    procHandleCmp(){}
    
    bool operator()(const sc_core::sc_process_handle& x, const sc_core::sc_process_handle& y){
      //frankly speaking this is ugly, but it's the only LRM compliant way to compare process_handles
      return ( (strcmp(x.name(), y.name())>0)? true: false);
    }
};
#endif

template <class Td, class Ta>
class OCP_TL1_DataCl : public sc_core::sc_prim_channel
{
public:
  typedef Td DataType;
  typedef Ta AddrType;
  typedef OCPIP_VERSION::unsigned_type<sizeof(Td)*8>        SizeCalc;
  #if defined  (OSCI20) || defined (OSCI21V1)
  typedef std::map< sc_core::sc_process_handle , bool> monitor_map_type;  
  #else
  typedef std::map< sc_core::sc_process_handle , bool, procHandleCmp> monitor_map_type;
  #endif

  // Constructor
  OCP_TL1_DataCl(void)
  {
    // initialization
    m_MReset_n = true;
    m_MResetStartTime = sc_core::SC_ZERO_TIME;
    m_SReset_n = true;
    m_SResetStartTime = sc_core::SC_ZERO_TIME;
    Reset();
  }

  // Destructor
  virtual ~OCP_TL1_DataCl(void)
  {
  }

  void Reset(void)
  {
    //
    m_ReqGrp[0].reset();
    m_ReqGrp[1].reset();
    m_RespGrp[0].reset();
    m_RespGrp[1].reset();
    m_DataHSGrp[0].reset();
    m_DataHSGrp[1].reset();

    m_SThreadBusy.reset();
    m_SDataThreadBusy.reset();
    m_MThreadBusy.reset();

    // initialize internal members
    m_Synchron = true; // hard coded. OCP TL1 model not build for asynchronous mode.

    m_ReqToggle = 0;

    m_RespToggle = 0;
    
    m_DataHSToggle = 0;

    // Reset Sideband Signals
    m_MError = false;
    m_nextMError = false;
    m_MFlag = 0;
    m_nextMFlag = 0;
    m_SError = false;
    m_nextSError = false;
    m_SFlag = 0;
    m_nextSFlag = 0;
    m_SInterrupt = false;
    m_nextSInterrupt = false;
    m_Control = 0;
    m_nextControl = 0;
    m_ControlWr = false;
    m_nextControlWr = false;
    m_ControlBusy = false;
    m_nextControlBusy = false;
    m_Status = 0;
    m_nextStatus = 0;
    m_StatusRd = false;
    m_nextStatusRd = false;
    m_StatusBusy = false;
    m_nextStatusBusy = false;
    
    m_pPosedgeEvent=NULL;

  }

  // ---------------------------------------------------------
  // START of ACCESS METHODS
  // ---------------------------------------------------------

  // ---------------------------------------------------------
  // Helper Functions
  // ---------------------------------------------------------

  // is the passed thread ID number busy?
  bool 
  isThisThreadBusy( int threadNum, unsigned int threadField )
  {
      return ( 1 & (threadField >> threadNum) );
  }


  // ---------------------------------------------------------
  // Channel Methods
  // These methods are called by the transaction channel, and
  // they are mandatory.
  // ---------------------------------------------------------

  // Test and set access direction (read or write) 
  bool IsWriteRequest(void) const
  {
    return ((m_ReqGrp[1 - m_ReqToggle].MCmd == OCP_MCMD_WR)? true : false);
  }
  void SetWriteRequest(void)
  {
    m_ReqGrp[m_ReqToggle].MCmd = OCP_MCMD_WR;
  }
  void SetReadRequest(void)
  {
    m_ReqGrp[m_ReqToggle].MCmd = OCP_MCMD_RD;
  }

  // ----------------------------------------------------------
  // Request Signal Group 
  // ----------------------------------------------------------

  // Access to the Whole Request Group
  void 
  MputRequestGroup(const OCPRequestGrp<Td,Ta>& newRequest)
  {
      m_ReqGrp[m_ReqToggle] = newRequest;
      saveMCmd();
  }
  OCPRequestGrp<Td,Ta> 
  SgetRequestGroup(void) const
  {
      return m_ReqGrp[1 - m_ReqToggle];
  }

#ifdef PERF_MONITOR
    void MputTrHandle(long long h)
    {
	m_ReqGrp[1 - m_ReqToggle].TrHandle = h;
    }
#endif

  // Access methods for MAddr
  void MputMAddr(Ta a)
  {
    m_ReqGrp[m_ReqToggle].MAddr = a;
  }
  Ta SgetMAddr(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MAddr);
  }

  // Access methods for MAddrSpace
  void MputMAddrSpace(int a)
  {
    m_ReqGrp[m_ReqToggle].MAddrSpace = a;
  }
  int SgetMAddrSpace(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MAddrSpace);
  }

  // old OCP 1.0 signal MBurst is no longer supported
  // void MputMBurst(OCPMBurstType a) 
  // { 
  //   m_ReqGrp[m_ReqToggle].MBurst = a; 
  // } 
  // OCPMBurstType SgetMBurst(void) const 
  // { 
  //   return (m_ReqGrp[1 - m_ReqToggle].MBurst); 
  // } 

  // Access methods for MByteEn
  void MputMByteEn(unsigned int a)
  {
    m_ReqGrp[m_ReqToggle].MByteEn = a;
  }
  unsigned int SgetMByteEn(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MByteEn);
  }

  // Access methods for MCmd
  void MputMCmd(OCPMCmdType a)
  {
    m_ReqGrp[m_ReqToggle].MCmd = a;
  }

  void saveMCmd(void)
  {
    m_ReqGrp[m_ReqToggle].MCmdSaved = m_ReqGrp[m_ReqToggle].MCmd;
    for (monitor_map_type::iterator i=req_monitor_map[m_ReqToggle].begin(); i!=req_monitor_map[m_ReqToggle].end(); i++) 
      i->second=true;
  }

  // Gets and resets MCmd
  OCPMCmdType SgetMCmd(void)
  {
    OCPMCmdType a = m_ReqGrp[1 - m_ReqToggle].MCmd;
    // reset MCmd after testing it
    m_ReqGrp[1 - m_ReqToggle].MCmd = OCP_MCMD_IDLE;
    

    return (a);
  }
  // Gets shadow MCmd
  OCPMCmdType getMCmdTrace(void)
  {
    // reset shadow MCmd after testing it
    bool first_time=true;    
    sc_core::sc_process_handle ph = sc_core::sc_get_current_process_handle();
    monitor_map_type::iterator i=req_monitor_map[1-m_ReqToggle].find(ph);
    if (i!=req_monitor_map[1-m_ReqToggle].end()) first_time=i->second;
    req_monitor_map[1-m_ReqToggle][ph]=false;
    //m_ReqGrp[1 - m_ReqToggle].MCmdSaved = OCP_MCMD_IDLE;
    return (first_time? m_ReqGrp[1 - m_ReqToggle].MCmdSaved : OCP_MCMD_IDLE);
  }

  // Const version of the function leaves MCmd alone
  OCPMCmdType SreadMCmd(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MCmd);
  }

  // Access methods for MReqInfo
  void MputMReqInfo(sc_dt::uint64 d)
  {
    m_ReqGrp[m_ReqToggle].MReqInfo = d;
  }
  void SgetMReqInfo(sc_dt::uint64 &d) const
  {
    d = m_ReqGrp[1 - m_ReqToggle].MReqInfo;
  }
  sc_dt::uint64 SgetMReqInfo(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MReqInfo);
  }

  // Access methods for MConnID
  void MputMConnID(int a)
  {
    m_ReqGrp[m_ReqToggle].MConnID = a;
  }
  int SgetMConnID(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MConnID);
  }

  // Access methods for MData (no Data HS)
  void MputMData(Td d)
  {
    m_ReqGrp[m_ReqToggle].MData = d;
  }
  void SgetMData(Td &d) const
  {
    d = m_ReqGrp[1 - m_ReqToggle].MData;
  }
  Td SgetMData(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MData);
  }

  // Access methods for MDataInfo (no Data HS)
  void MputMDataInfo(sc_dt::uint64 d)
  {
    m_ReqGrp[m_ReqToggle].MDataInfo = d;
  }
  void SgetMDataInfo(sc_dt::uint64 &d) const
  {
    d = m_ReqGrp[1 - m_ReqToggle].MDataInfo;
  }
  sc_dt::uint64 SgetMDataInfo(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MDataInfo);
  }

  // Access methods for MThreadID
  void MputMThreadID(int a)
  {
    m_ReqGrp[m_ReqToggle].MThreadID = a;
  }
  int SgetMThreadID(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MThreadID);
  }

  // Access methods for MTagID
  void MputMTagID(int a)
  {
    m_ReqGrp[m_ReqToggle].MTagID = a;
  }
  int SgetMTagID(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MTagID);
  }

  // Access methods for MTagInOrder
  void MputMTagInorder(bool a)
  {
    m_ReqGrp[m_ReqToggle].MTagInOrder = a;
  }
  bool SgetMTagInOrder(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MTagInOrder);
  }

  // Access methods for Bursts
  void MputMAtomicLength(unsigned int a)
  {
    m_ReqGrp[m_ReqToggle].MAtomicLength = a;
  }
  unsigned int SgetMAtomicLength(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MAtomicLength);
  }
  void MputMBurstPrecise(bool a)
  {
    m_ReqGrp[m_ReqToggle].MBurstPrecise = a;
  }
  bool SgetMBurstPrecise(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MBurstPrecise);
  }
  void MputMBurstSeq(OCPMBurstSeqType a)
  {
    m_ReqGrp[m_ReqToggle].MBurstSeq = a;
  }
  OCPMBurstSeqType SgetMBurstSeq(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MBurstSeq);
  }
  void MputMBurstSingleReq(bool a)
  {
    m_ReqGrp[m_ReqToggle].MBurstSingleReq = a;
  }
  bool SgetMBurstSingleReq(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MBurstSingleReq);
  }
  void MputMReqLast(bool a)
  {
    m_ReqGrp[m_ReqToggle].MReqLast = a;
  }
  bool SgetMReqLast(void) const
  {
    return (m_ReqGrp[1 - m_ReqToggle].MReqLast);
  }

  // ----------------------------------------------------------
  // Response Signal Group 
  // ----------------------------------------------------------

  // Access to the Whole Response Group
  void 
  SputResponseGroup(const OCPResponseGrp<Td>& newResponse)
  {
      m_RespGrp[m_RespToggle] = newResponse;
      saveSResp();
  }
  OCPResponseGrp<Td> 
  MgetResponseGroup(void) const
  {
      return m_RespGrp[1 - m_RespToggle];
  }

#ifdef PERF_MONITOR
   void SputTrHandle(long long h)
   {
      m_RespGrp[1 - m_RespToggle].TrHandle = h;
   }
#endif

  // Access methods for SData
  void SputSData(Td d)
  {
    m_RespGrp[m_RespToggle].SData = d;
  }
  void MgetSData(Td &d) const
  {
    d = m_RespGrp[1 - m_RespToggle].SData;
  }
  Td MgetSData(void) const
  {
    return (m_RespGrp[1 - m_RespToggle].SData);
  }

  // Access methods for SDataInfo
  void SputSDataInfo(sc_dt::uint64 d)
  {
    m_RespGrp[m_RespToggle].SDataInfo = d;
  }
  void MgetSDataInfo(sc_dt::uint64 &d) const
  {
    d = m_RespGrp[1 - m_RespToggle].SDataInfo;
  }
  sc_dt::uint64 MgetSDataInfo(void) const
  {
    return (m_RespGrp[1 - m_RespToggle].SDataInfo);
  }

  // Access methods for SResp
  void SputSResp(OCPSRespType a)
  {
    m_RespGrp[m_RespToggle].SResp = a;
  }
  void saveSResp(void)
  {
    m_RespGrp[m_RespToggle].SRespSaved = m_RespGrp[m_RespToggle].SResp;
    for (monitor_map_type::iterator i=resp_monitor_map[m_RespToggle].begin(); i!=resp_monitor_map[m_RespToggle].end(); i++) 
      i->second=true;    
    
  }

  // This function gets and then resets SResp
  OCPSRespType MgetSResp(void)
  {
    OCPSRespType a = m_RespGrp[1 - m_RespToggle].SResp;
    // reset SResp after testing it
    m_RespGrp[1 - m_RespToggle].SResp = OCP_SRESP_NULL;

    return (a);
  }

  // Gets shadow SResp
  OCPSRespType getSRespTrace(void)
  {
    bool first_time=true;    
    sc_core::sc_process_handle ph = sc_core::sc_get_current_process_handle();
    monitor_map_type::iterator i=resp_monitor_map[1-m_RespToggle].find(ph);
    if (i!=resp_monitor_map[1-m_RespToggle].end()) first_time=i->second;
    resp_monitor_map[1-m_RespToggle][ph]=false;
    //m_ReqGrp[1 - m_ReqToggle].MCmdSaved = OCP_MCMD_IDLE;
    return (first_time? m_RespGrp[1 - m_RespToggle].SRespSaved : OCP_SRESP_NULL);
  }

  // Const Version leaves SResp alone
  OCPSRespType MreadSResp(void) const
  {
    return (m_RespGrp[1 - m_RespToggle].SResp);
  }

  // Access methods for SRespInfo
  void SputSRespInfo(sc_dt::uint64 d)
  {
    m_RespGrp[m_RespToggle].SRespInfo = d;
  }
  void MgetSRespInfo(sc_dt::uint64 &d) const
  {
    d = m_RespGrp[1 - m_RespToggle].SRespInfo;
  }
  sc_dt::uint64 MgetSRespInfo(void) const
  {
    return (m_RespGrp[1 - m_RespToggle].SRespInfo);
  }

  // Access methods for SRespLast
  void SputSRespLast(bool d)
  {
    m_RespGrp[m_RespToggle].SRespLast = d;
  }
  void MgetSRespLast(bool &d) const
  {
    d = m_RespGrp[1 - m_RespToggle].SRespLast;
  }
  bool MgetSRespLast(void) const
  {
    return (m_RespGrp[1 - m_RespToggle].SRespLast);
  }

  // Access methods for SThreadID
  void SputSThreadID(int a)
  {
    m_RespGrp[m_RespToggle].SThreadID = a;
  }
  int MgetSThreadID(void) const
  {
    return (m_RespGrp[1 - m_RespToggle].SThreadID);
  }

  // Access methods for STagID
  void SputSTagID(int a)
  {
    m_RespGrp[m_RespToggle].STagID = a;
  }
  int MgetSTagID(void) const
  {
    return (m_RespGrp[1 - m_RespToggle].STagID);
  }

  // Access methods for STagInOrder
  void SputSTagInOrder(bool a)
  {
    m_RespGrp[m_RespToggle].STagInOrder = a;
  }
  bool MgetSTagInOrder(void) const
  {
    return (m_RespGrp[1 - m_RespToggle].STagInOrder);
  }

  // ----------------------------------------------------------
  // Datahandshake Signal Group 
  // ----------------------------------------------------------

  // Access to the Whole Data Handshake Group
  void 
  MputDataHSGroup(const OCPDataHSGrp<Td>& newDataHS)
  {
      m_DataHSGrp[m_DataHSToggle] = newDataHS;
      saveMDataValid();
  }
  OCPDataHSGrp<Td> 
  SgetDataHSGroup(void) const
  {   
      return m_DataHSGrp[1 - m_DataHSToggle];
  }

  // Access methods for MData with data handshake
  void MputMDataHS(Td d)
  {
    m_DataHSGrp[m_DataHSToggle].MData = d;
  }
  void SgetMDataHS(Td &d) const
  {
    d = m_DataHSGrp[1 - m_DataHSToggle].MData;
  }
  Td SgetMDataHS(void) const
  {
    return (m_DataHSGrp[1 - m_DataHSToggle].MData);
  }

  // Access methods for MDataInfo with data handshake
  void MputMDataInfoHS(sc_dt::uint64 d)
  {
    m_DataHSGrp[m_DataHSToggle].MDataInfo = d;
  }
  void SgetMDataInfoHS(sc_dt::uint64 &d) const
  {
    d = m_DataHSGrp[1 - m_DataHSToggle].MDataInfo;
  }
  sc_dt::uint64 SgetMDataInfoHS(void) const
  {
    return (m_DataHSGrp[1 - m_DataHSToggle].MDataInfo);
  }

  // Access methods for MDataValid
  void MputMDataValid(bool a)
  {
    m_DataHSGrp[m_DataHSToggle].MDataValid = a;
  }

  void saveMDataValid(void)
  {
    m_DataHSGrp[m_DataHSToggle].MDataValidSaved = m_DataHSGrp[m_DataHSToggle].MDataValid;
    for (monitor_map_type::iterator i=data_monitor_map[m_DataHSToggle].begin(); i!=data_monitor_map[m_DataHSToggle].end(); i++) 
      i->second=true;    
  }

  // This function gets and resets the MDataValid signal
  bool SgetMDataValid(void)
  {
    bool a = m_DataHSGrp[1 - m_DataHSToggle].MDataValid;
    // reset
    m_DataHSGrp[1 - m_DataHSToggle].MDataValid = false;

    return (a);
  }
  // Gets shadow MDataValid
  bool getMDataValidTrace(void)
  {
    bool first_time=true;    
    sc_core::sc_process_handle ph = sc_core::sc_get_current_process_handle();
    monitor_map_type::iterator i=data_monitor_map[1-m_DataHSToggle].find(ph);
    if (i!=data_monitor_map[1-m_DataHSToggle].end()) first_time=i->second;
    data_monitor_map[1-m_DataHSToggle][ph]=false;
    //m_ReqGrp[1 - m_ReqToggle].MCmdSaved = OCP_MCMD_IDLE;
    return (first_time? m_DataHSGrp[1 - m_DataHSToggle].MDataValidSaved : false);
  }

  // This const version leaves MDataValid alone
  bool SreadMDataValid(void) const
  {
    return(m_DataHSGrp[1 - m_DataHSToggle].MDataValid);
  }

  // Access methods for MDataThreadID
  void MputMDataThreadID(int a)
  {
    m_DataHSGrp[m_DataHSToggle].MDataThreadID = a;
  }
  int SgetMDataThreadID(void) const
  {
    return (m_DataHSGrp[1 - m_DataHSToggle].MDataThreadID);
  }

  // Access methods for MDataTagID
  void MputMDataTagID(int a)
  {
    m_DataHSGrp[m_DataHSToggle].MDataTagID = a;
  }
  int SgetMDataTagID(void) const
  {
    return (m_DataHSGrp[1 - m_DataHSToggle].MDataTagID);
  }

  // ----------------------------------------------------------
  // Thread busy signaling. 
  // ----------------------------------------------------------

  // Access methods for MThreadBusy
  void MputMThreadBusy(unsigned int newValue, const char* chName, bool nocheck=false)
  {
      m_MThreadBusy.nextDeltaValue = newValue; 
      m_MThreadBusy.setAtNextDelta = true;
      if (m_MThreadBusy.lastCheckDelta == simcontext()->delta_count() && !nocheck)
        std::cout<<"WARNING: Coinstantenous (same delta cycle) read and write of non pipelined MThreadBusy in channel "<<chName<<" detected. Reader gets old value."<<std::endl<<std::flush;
      m_MThreadBusy.lastChangeDelta = simcontext()->delta_count();      
      if (m_Synchron) {
        request_update();
      } else {
          update();
      }
  }
  void MputNextMThreadBusy(unsigned int newValue)
  {
      const sc_core::sc_time oneTick(1,sc_core::SC_NS);
      sc_core::sc_time timeNow(sc_core::sc_time_stamp());
      // Last chance to use the current "next" values (if any) to do any necessary 
      // update before they are erased:
      m_MThreadBusy.update(timeNow);
      m_MThreadBusy.nextTickValue = newValue; 
      m_MThreadBusy.nextTickTime = timeNow + oneTick;
      m_MThreadBusy.setAtNextTick = true;
  }
  unsigned int SgetMThreadBusy(const char* chName, bool nocheck=false) 
  {
      m_MThreadBusy.lastCheckDelta=simcontext()->delta_count();
      m_MThreadBusy.update(sc_core::sc_time_stamp());
      if (m_MThreadBusy.lastChangeDelta == simcontext()->delta_count() && !nocheck)
        std::cout<<"WARNING: Coinstantenous (same delta cycle) read and write of non pipelined MThreadBusy in channel "<<chName<<" detected. Reader gets old value."<<std::endl<<std::flush;
      return m_MThreadBusy.currentValue;
  }
  // Internal function used by the OCP Monitor to the get last value before the clock tick
  unsigned int SgetLastMThreadBusy(void) 
  {
      m_MThreadBusy.update(sc_core::sc_time_stamp());
      if (m_MThreadBusy.lastSavedTime == sc_core::sc_time_stamp()) {
          // Either the update above or one called by a getThreadBusy function
          // changed the current value during this time step.
          // return the previous value instead.
          return m_MThreadBusy.lastValue;
      }
      // Current value hasn't changed this clock cycle so it has the value from the 
      // end of the previous cycle.
      return m_MThreadBusy.currentValue;
  }

  // Access methods for SThreadBusy
  void SputSThreadBusy(unsigned int newValue, const char * chName, bool nocheck=false)
  {
      m_SThreadBusy.nextDeltaValue = newValue; 
      m_SThreadBusy.setAtNextDelta = true;
      if (m_SThreadBusy.lastCheckDelta == simcontext()->delta_count() && !nocheck)
        std::cout<<"WARNING: Coinstantenous (same delta cycle) read and write of non pipelined SThreadBusy in channel "<<chName<<" detected. Reader gets old value."<<std::endl<<std::flush;
      m_SThreadBusy.lastChangeDelta = simcontext()->delta_count();
      if (m_Synchron) {
        request_update();
      } else {
          update();
      }
  }
  
  void SputNextSThreadBusy(unsigned int newValue)
  {
      const sc_core::sc_time oneTick(1,sc_core::SC_NS);
      sc_core::sc_time timeNow(sc_core::sc_time_stamp());
      // Last chance to use the current "next" values (if any) to do any necessary 
      // update before they are erased:
      m_SThreadBusy.update(timeNow);
      m_SThreadBusy.nextTickValue = newValue; 
      m_SThreadBusy.nextTickTime = timeNow + oneTick;
      m_SThreadBusy.setAtNextTick = true;
  }
  unsigned int MgetSThreadBusy(const char* chName, bool nocheck=false) 
  {
      m_SThreadBusy.lastCheckDelta=simcontext()->delta_count();
      if (m_SThreadBusy.lastChangeDelta == simcontext()->delta_count() && !nocheck)
        std::cout<<"WARNING: Coinstantenous (same delta cycle) read and write of non pipelined SThreadBusy in channel "<<chName<<" detected. Reader gets old value."<<std::endl<<std::flush;
      m_SThreadBusy.update(sc_core::sc_time_stamp());
      return m_SThreadBusy.currentValue;
  }
  // Internal function used by the OCP Monitor to the get last value before the clock tick
  unsigned int MgetLastSThreadBusy(void) 
  {
      m_SThreadBusy.update(sc_core::sc_time_stamp());
      if (m_SThreadBusy.lastSavedTime == sc_core::sc_time_stamp()) {
          // Either the update above or one called by a getThreadBusy function
          // changed the current value during this time step.
          // return the previous value instead.
          return m_SThreadBusy.lastValue;
      }
      // Current value hasn't changed this clock cycle so it has the value from the 
      // end of the previous cycle.
      return m_SThreadBusy.currentValue;
  }

  // Access methods for SDataThreadBusy
  void SputSDataThreadBusy(unsigned int newValue, const char* chName, bool nocheck=false)
  {
      m_SDataThreadBusy.nextDeltaValue = newValue; 
      m_SDataThreadBusy.setAtNextDelta = true;
      if (m_SDataThreadBusy.lastCheckDelta == simcontext()->delta_count() && !nocheck)
        std::cout<<"WARNING: Coinstantenous (same delta cycle) read and write of non pipelined SDataThreadBusy in channel "<<chName<<" detected. Reader gets old value."<<std::endl<<std::flush;
      m_SDataThreadBusy.lastChangeDelta = simcontext()->delta_count();      
      if (m_Synchron) {
        request_update();
      } else {
          update();
      }
  }
  void SputNextSDataThreadBusy(unsigned int newValue)
  {
      const sc_core::sc_time oneTick(1,sc_core::SC_NS);
      sc_core::sc_time timeNow(sc_core::sc_time_stamp());
      // Last chance to use the current "next" values (if any) to do any necessary 
      // update before they are erased:
      m_SDataThreadBusy.update(timeNow);
      m_SDataThreadBusy.nextTickValue = newValue; 
      m_SDataThreadBusy.nextTickTime = timeNow + oneTick;
      m_SDataThreadBusy.setAtNextTick = true;
  }
  unsigned int MgetSDataThreadBusy(const char* chName, bool nocheck=false) 
  {
      m_SDataThreadBusy.lastCheckDelta=simcontext()->delta_count();  
      if (m_SDataThreadBusy.lastChangeDelta == simcontext()->delta_count() && !nocheck)
        std::cout<<"WARNING: Coinstantenous (same delta cycle) read and write of non pipelined SDataThreadBusy in channel "<<chName<<" detected. Reader gets old value."<<std::endl<<std::flush;
      m_SDataThreadBusy.update(sc_core::sc_time_stamp());
      return m_SDataThreadBusy.currentValue;
  }
  // Internal function used by the OCP Monitor to the get last value before the clock tick
  unsigned int MgetLastSDataThreadBusy(void) 
  {
      m_SDataThreadBusy.update(sc_core::sc_time_stamp());
      if (m_SDataThreadBusy.lastSavedTime == sc_core::sc_time_stamp()) {
          // Either the update above or one called by a getThreadBusy function
          // changed the current value during this time step.
          // return the previous value instead.
          return m_SDataThreadBusy.lastValue;
      }
      // Current value hasn't changed this clock cycle so it has the value from the 
      // end of the previous cycle.
      return m_SDataThreadBusy.currentValue;
  }

  // ----------------------------------------------------------
  // Sideband Signal Group. 
  // 
  // Each sideband signal generates its own event.
  // NOTE: When a pointer to this data signal class is used,
  //       there is no checking that the proper port is calling
  //       the proper command. ie There is nothing to stop the
  //       the Slave from issuing a MputMError(true) command.
  // ----------------------------------------------------------

  // Update function for sideband signals
  // NOTE: that this simple function could be inlined into the
  //       sideband "put" functions to save the function call
  //       overhead. Instead it is a seperate function because
  //       the synchronization of the OCP channel is still an
  //       open issue. This function provides a single change
  //       point to implement any changes in the synchronization
  //       scheme.
  void requestSidebandUpdate(void)
  {
    if (m_Synchron) {
      request_update();
    } else {
      update();
    }
  }
  
  // Access methods for MError
  void MputMError(bool nextValue)
  {
    m_nextMError = nextValue;
    requestSidebandUpdate();
  }
  bool SgetMError(void) const
  {
    return (m_MError);
  }
  const sc_core::sc_event& SidebandMErrorEvent(void) const { 
    return (m_SidebandMErrorEvent); 
  }

  // Access methods for MFlag
  void MputMFlag(sc_dt::uint64 nextValue)
  {
    m_nextMFlag = nextValue;
    requestSidebandUpdate();
  }
  void MputMFlag(sc_dt::uint64 nextValue, 
          sc_dt::uint64 mask)
  {
    m_nextMFlag = ( m_nextMFlag & ~mask ) | ( nextValue & mask );
    requestSidebandUpdate();
  }
  sc_dt::uint64 SgetMFlag(void) const
  {
    return (m_MFlag);
  }
  const sc_core::sc_event& SidebandMFlagEvent(void) const { 
    return (m_SidebandMFlagEvent); 
  }

  // Access methods for SError
  void SputSError(bool nextValue)
  {
    m_nextSError = nextValue;
    requestSidebandUpdate();
  }
  bool MgetSError(void) const
  {
    return (m_SError);
  }
  const sc_core::sc_event& SidebandSErrorEvent(void) const { 
    return (m_SidebandSErrorEvent); 
  }

  // Access methods for SFlag
  void SputSFlag(sc_dt::uint64 nextValue)
  {
    m_nextSFlag = nextValue;
    requestSidebandUpdate();
  }
  void SputSFlag(sc_dt::uint64 nextValue, 
          sc_dt::uint64 mask)
  {
    m_nextSFlag = ( m_nextSFlag & ~mask ) | ( nextValue & mask );
    requestSidebandUpdate();
  }
  sc_dt::uint64 MgetSFlag(void) const
  {
    return (m_SFlag);
  }
  const sc_core::sc_event& SidebandSFlagEvent(void) const { 
    return (m_SidebandSFlagEvent); 
  }

  // Access methods for SInterrupt
  void SputSInterrupt(bool nextValue)
  {
    m_nextSInterrupt = nextValue;
    requestSidebandUpdate();
  }
  bool MgetSInterrupt(void) const
  {
    return (m_SInterrupt);
  }
  const sc_core::sc_event& SidebandSInterruptEvent(void) const { 
    return (m_SidebandSInterruptEvent); 
  }

  //---------------------------------------------------------------------
  // system/core signal group.
  // 
  // NOTE: there is no control over how these functions are accessed.
  //       ie. The System side of the channel may call any of the 
  //       Core side's functions without restriction
  //       
  //---------------------------------------------------------------------

  // Access methods for Control
  bool SysputControl(int nextValue)
  {
    // cannot change control signal if ControlBusy is true.
    if (m_ControlBusy) {
      return (false);
    }
    m_nextControl = nextValue;
    requestSidebandUpdate();
    return (true);
  }
  int CgetControl(void) const
  {
    return (m_Control);
  }
  const sc_core::sc_event& SidebandControlEvent(void) const { 
    return (m_SidebandControlEvent); 
  }

  // Access methods for ControlBusy
  void CputControlBusy(bool nextValue)
  {
    m_nextControlBusy = nextValue;
    requestSidebandUpdate();
  }
  bool SysgetControlBusy(void) const
  {
    return (m_ControlBusy);
  }
  const sc_core::sc_event& SidebandControlBusyEvent(void) const { 
    return (m_SidebandControlBusyEvent); 
  }

  // Access methods for ControlWr
  void SysputControlWr(bool nextValue)
  {
    m_nextControlWr = nextValue; 
    requestSidebandUpdate();
  }
  bool CgetControlWr(void) const
  {
    return (m_ControlWr);
  }
  const sc_core::sc_event& SidebandControlWrEvent(void) const { 
    return (m_SidebandControlWrEvent); 
  }

  // Access methods for Status
  void CputStatus(int nextValue)
  {
    m_nextStatus = nextValue;
    requestSidebandUpdate();
  }
  int SysgetStatus(void) const
  {
    return (m_Status);
  }
  bool readStatus(int& currentValue) const
  {
    // Cannot read the Status value of StatusBusy is true
    if (m_StatusBusy) {
      return (false);
    }
    currentValue = m_Status;
    return (true);
  }
  const sc_core::sc_event& SidebandStatusEvent(void) const { 
    return (m_SidebandStatusEvent); 
  }

  // Access methods for StatusBusy
  void CputStatusBusy(bool nextValue)
  {
    m_nextStatusBusy = nextValue;
    requestSidebandUpdate();
  }
  bool SysgetStatusBusy(void) const
  {
    return (m_StatusBusy);
  }
  const sc_core::sc_event& SidebandStatusBusyEvent(void) const { 
    return (m_SidebandStatusBusyEvent); 
  }

  // Access methods for StatusRd
  void SysputStatusRd(bool nextValue)
  {
    m_nextStatusRd = nextValue;
    requestSidebandUpdate();
  }
  bool CgetStatusRd(void) const
  {
    return (m_StatusRd);
  }
  const sc_core::sc_event& SidebandStatusRdEvent(void) const { 
    return (m_SidebandStatusRdEvent); 
  }

  //---------------------------------------------------------------------
  // END of ACCESS METHODS
  //---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // UPDATE METHODS
  //---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Request Phase updates. The when the channel updates its request
  // state, these functions are called so that this data class stays
  // synched to the channel.
  //---------------------------------------------------------------------
  void update_Fw(int eventSelect)
  {
      // eventSelect: 1=Start of phase, 2=End of phase
      switch (eventSelect)
      {
          case 1:
              // Put new Request on the channel
              m_ReqToggle = 1 - m_ReqToggle;
              break;

          case 2:
              break;

          default:
              break;
      }
  }

  //---------------------------------------------------------------------
  // Data Handshake Phase updates. The when the channel updates its dataHS
  // state, these functions are called so that this data class stays
  // synched to the channel.
  //---------------------------------------------------------------------

  void update_FwD(int eventSelect)
  {
      // eventSelect: 1=Start of phase, 2=End of phase
      switch (eventSelect)
      {
          case 1:
              // Make sure that the new data will be marked as valid
              MputMDataValid(true);
              // Put new Data on the channel
              m_DataHSToggle = 1 - m_DataHSToggle;
              break;

          case 2:
              break;

          default:
              break;
      }
  }

  //---------------------------------------------------------------------
  // Response Phase updates. The when the channel updates its response
  // state, these functions are called so that this data class stays
  // synched to the channel.
  //---------------------------------------------------------------------

  void update_Bw(int eventSelect)
  {
      // eventSelect: 1=Start of phase, 2=End of phase
      switch (eventSelect)
      {
          case 1:
              // Put new Response on the channel
              m_RespToggle = 1 - m_RespToggle;
              break;

          case 2:
              break;

          default:
              break;
      }
  }

  //---------------------------------------------------------------------
  // This method updates the ungrouped and sideband signals.
  // The method is either called synchronously (via request_update()) 
  // or asynchronously (as direct call), depending on the value of m_Synchron.
  //---------------------------------------------------------------------

  void update(void)
  {
      if (m_MThreadBusy.setAtNextDelta) {
          m_MThreadBusy.update(sc_core::sc_time_stamp());
          m_MThreadBusy.currentValue = m_MThreadBusy.nextDeltaValue;
          m_MThreadBusy.setAtNextDelta = false;
          m_MThreadBusy.event.notify( sc_core::SC_ZERO_TIME );
      }

      if (m_SThreadBusy.setAtNextDelta) {
          m_SThreadBusy.update(sc_core::sc_time_stamp());
          m_SThreadBusy.currentValue = m_SThreadBusy.nextDeltaValue;
          m_SThreadBusy.setAtNextDelta = false;
          m_SThreadBusy.event.notify( sc_core::SC_ZERO_TIME );
      }

      if (m_SDataThreadBusy.setAtNextDelta) {
          m_SDataThreadBusy.update(sc_core::sc_time_stamp());
          m_SDataThreadBusy.currentValue = m_SDataThreadBusy.nextDeltaValue;
          m_SDataThreadBusy.setAtNextDelta = false;
          m_SDataThreadBusy.event.notify( sc_core::SC_ZERO_TIME );
      }

      // Update sideband signals and set events
      // NOTE: if this is an Asynchronous update then notify should be immediate.
      //       if it is a synchronous update, then it sould be sc_core::SC_ZERO_TIME.
      // NOTE: The TL1 channel currently does not support Asynchronous update.
      if (m_MError != m_nextMError) {
          m_MError = m_nextMError;
          m_SidebandMErrorEvent.notify(sc_core::SC_ZERO_TIME);
      }
      if (m_MFlag != m_nextMFlag) {
          m_MFlag = m_nextMFlag;
          m_SidebandMFlagEvent.notify(sc_core::SC_ZERO_TIME);
      }
      if (m_SError != m_nextSError) {
          m_SError = m_nextSError;
          m_SidebandSErrorEvent.notify(sc_core::SC_ZERO_TIME);
      }
      if (m_SFlag != m_nextSFlag) {
          m_SFlag = m_nextSFlag;
          m_SidebandSFlagEvent.notify(sc_core::SC_ZERO_TIME);
      }
      if (m_SInterrupt != m_nextSInterrupt) {
          m_SInterrupt = m_nextSInterrupt;
          m_SidebandSInterruptEvent.notify(sc_core::SC_ZERO_TIME);
      }
      if (m_Control != m_nextControl) {
          m_Control = m_nextControl;
          m_SidebandControlEvent.notify(sc_core::SC_ZERO_TIME);
      }
      if (m_ControlWr != m_nextControlWr) {
          m_ControlWr = m_nextControlWr;
          m_SidebandControlWrEvent.notify(sc_core::SC_ZERO_TIME);
      }
      if (m_ControlBusy != m_nextControlBusy) {
          m_ControlBusy = m_nextControlBusy;
          m_SidebandControlBusyEvent.notify(sc_core::SC_ZERO_TIME);
      }
      if (m_Status != m_nextStatus) {
          m_Status = m_nextStatus;
          m_SidebandStatusEvent.notify(sc_core::SC_ZERO_TIME);
      }
      if (m_StatusRd != m_nextStatusRd) {
          m_StatusRd = m_nextStatusRd;
          m_SidebandStatusRdEvent.notify(sc_core::SC_ZERO_TIME);
      }
      if (m_StatusBusy != m_nextStatusBusy) {
          m_StatusBusy = m_nextStatusBusy;
          m_SidebandStatusBusyEvent.notify(sc_core::SC_ZERO_TIME);
      } 
  }

  // ---------------------------------------------------------
  //
  // OCP Data Members
  //
  // ---------------------------------------------------------

  // Request Signal Group
  OCPRequestGrp<Td,Ta> m_ReqGrp[2];

  // Response Signal Group
  OCPResponseGrp<Td>   m_RespGrp[2];

  // Datahandshake Signal Group
  OCPDataHSGrp<Td> m_DataHSGrp[2];
  
  // MultiMonitorSuuport
  monitor_map_type req_monitor_map[2];
  monitor_map_type data_monitor_map[2];
  monitor_map_type resp_monitor_map[2];    

  // Ungrouped Signals
  ThreadBusyStruct m_MThreadBusy;
  ThreadBusyStruct m_SThreadBusy;
  ThreadBusyStruct m_SDataThreadBusy;

  // ---------------------
  // Sideband Signal Group
  // ---------------------
  bool m_MError;
  bool m_nextMError;
  sc_core::sc_event m_SidebandMErrorEvent;
  sc_dt::uint64 m_MFlag;
  sc_dt::uint64 m_nextMFlag;
  sc_core::sc_event m_SidebandMFlagEvent;
  bool m_SError;
  bool m_nextSError;
  sc_core::sc_event m_SidebandSErrorEvent;
  sc_dt::uint64 m_SFlag;
  sc_dt::uint64 m_nextSFlag;
  sc_core::sc_event m_SidebandSFlagEvent;
  bool m_SInterrupt;
  bool m_nextSInterrupt;
  sc_core::sc_event m_SidebandSInterruptEvent;
  int m_Control;
  int m_nextControl;
  sc_core::sc_event m_SidebandControlEvent;
  bool m_ControlWr;
  bool m_nextControlWr;
  sc_core::sc_event m_SidebandControlWrEvent;
  bool m_ControlBusy;
  bool m_nextControlBusy;
  sc_core::sc_event m_SidebandControlBusyEvent;
  int m_Status;
  int m_nextStatus;
  sc_core::sc_event m_SidebandStatusEvent;
  bool m_StatusRd;
  bool m_nextStatusRd;
  sc_core::sc_event m_SidebandStatusRdEvent;
  bool m_StatusBusy;
  bool m_nextStatusBusy;
  sc_core::sc_event m_SidebandStatusBusyEvent;

  // Reset Signals
  bool m_MReset_n;
  bool m_SReset_n;
  sc_core::sc_time m_MResetStartTime;
  sc_core::sc_time m_SResetStartTime;

  // ---------------------------------------------------------
  // End of OCP Data Members
  // ---------------------------------------------------------

  // synchronous/asynchronous update of the data members
  bool m_Synchron;

  // request channel update
  int  m_ReqToggle;

  // response channel update
  int  m_RespToggle;

  // second request channel (for data handshake) update
  int  m_DataHSToggle;
  
  // pointer to posedge event of channel clock (used for pipelined thread busy support)
  sc_core::sc_event* m_pPosedgeEvent;
};

#endif
