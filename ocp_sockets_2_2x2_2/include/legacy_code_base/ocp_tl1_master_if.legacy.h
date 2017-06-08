//
//  (c) Copyright OCP-IP 2003, 2004, 2005
//  OCP-IP Confidential and Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : Alan Kamas, for Sonics Inc.
//                Yann Bajot, Prosilog
//                Anssi Haverinen, Nokia Inc.
//
//          $Id:
//
//  Description : Master Interface for the OCP TL1 Channel
//
//
//  Old Change History before moving to CVS
//             12/14/2003 Original revision for OCP 2.0
//             03/29/2004 Added putMRespAccept(bool)
//             09/27/2004 Adds public access methods for 'RequestReleaseEvent',
//             'ResponseReleaseEvent' and 'DataHSReleaseEvent' (needed by
//             TL0-TL1 adapters)
// ============================================================================


#ifndef __OCP_TL1_MASTER_IF_LEGACY_H__
#define __OCP_TL1_MASTER_IF_LEGACY_H__

template <class TdataCl>
class OCP_TL1_MasterIF :
  virtual public sc_core::sc_interface,
  virtual public OCP_TL1_Master_TimingIF
{
  public:

  typedef typename TdataCl::DataType Td;
  typedef typename TdataCl::AddrType Ta;

  //////////////////////////////////////////////////////////////
  // OCP TL1 specific methods
  //////////////////////////////////////////////////////////////

  //---------------------------------------------------------------------
  // public OCP Master methods for channel configuration
  //---------------------------------------------------------------------
  virtual void setOCPMasterConfiguration(MapStringType& passedMap) = 0;
  virtual void addOCPConfigurationListener(OCP_TL_Config_Listener& listener) = 0;
  virtual const std::string	peekChannelName()	const = 0;
  
  //---------------------------------------------------------------------
  // public OCP Master methods for the request phase
  //---------------------------------------------------------------------
  virtual bool getSBusy(void) const = 0;
  virtual unsigned int getSThreadBusy(void) const = 0;
  virtual const sc_core::sc_event& SThreadBusyEvent(void) const = 0; 
  virtual unsigned int getCurrentSThreadBusy(void) const = 0;
  virtual const sc_core::sc_event& CurrentSThreadBusyEvent(void) const = 0;
  virtual bool startOCPRequest(OCPRequestGrp<Td,Ta>& req) = 0;
  virtual bool startOCPRequestBlocking(OCPRequestGrp<Td,Ta>& req) = 0;
  virtual bool getSCmdAccept(void) const = 0;
  virtual void waitSCmdAccept(void) = 0;
  virtual const sc_core::sc_event& RequestStartEvent(void) const = 0;
  virtual const sc_core::sc_event& RequestEndEvent(void) const = 0;

  //---------------------------------------------------------------------
  // public OCP Master methods for the response phase
  //---------------------------------------------------------------------
  virtual bool getOCPResponse(OCPResponseGrp<Td>& resp, bool accept) = 0;
  virtual bool getOCPResponse(OCPResponseGrp<Td>& resp) = 0;
  virtual bool getOCPResponseBlocking(OCPResponseGrp<Td>& resp, bool accept) = 0;
  virtual bool getOCPResponseBlocking(OCPResponseGrp<Td>& resp) = 0;
  virtual bool putMRespAccept() = 0;
  virtual void putMRespAccept(bool respaccept) = 0;
  virtual void putMThreadBusy(unsigned int mthreadbusy) = 0;
  virtual const sc_core::sc_event& ResponseStartEvent(void) const = 0;
  virtual const sc_core::sc_event& ResponseEndEvent(void) const = 0;

  //---------------------------------------------------------------------
  // public OCP Master methods for the data handshake phase
  //---------------------------------------------------------------------
  virtual bool getSBusyDataHS(void) const = 0;
  virtual unsigned int getSDataThreadBusy(void) const = 0;
  virtual const sc_core::sc_event& SDataThreadBusyEvent(void) const = 0; 
  virtual unsigned int getCurrentSDataThreadBusy(void) const = 0;
  virtual const sc_core::sc_event& CurrentSDataThreadBusyEvent(void) const = 0;
  virtual bool startOCPDataHS(OCPDataHSGrp<Td>& datahs) = 0;
  virtual bool startOCPDataHSBlocking(OCPDataHSGrp<Td>& datahs) = 0;
  virtual bool getSDataAccept(void) const = 0;
  virtual void waitSDataAccept(void) = 0;
  virtual const sc_core::sc_event& DataHSStartEvent(void) const = 0;
  virtual const sc_core::sc_event& DataHSEndEvent(void) const = 0;

  //---------------------------------------------------------------------
  // public OCP Timing Methods
  //---------------------------------------------------------------------
  virtual void ocpWait(double count = 1) = 0;
  virtual bool ocpSafeWait(double count = 1) = 0;

  //---------------------------------------------------------------------
  // public OCP Reset Methods
  //---------------------------------------------------------------------
  virtual bool getReset(void) = 0;
  virtual void MResetAssert(void) = 0;
  virtual void MResetDeassert(void) = 0;
  virtual const sc_core::sc_event& ResetStartEvent(void) const = 0;
  virtual const sc_core::sc_event& ResetEndEvent(void) const = 0;
  virtual const sc_core::sc_event& getFakeResetEndEvent(void) const =0;
  virtual const sc_core::sc_event& getFakeResetStartEvent(void) const =0;
  virtual const bool		peekMReset_n()		const = 0;
  virtual const bool		peekSReset_n()		const = 0;  

  //---------------------------------------------------------------------
  // public OCP Master Sideband Commands
  //---------------------------------------------------------------------
  virtual void MputMError(bool nextValue) = 0;
  virtual void MputMFlag(sc_dt::uint64 nextValue) = 0;
  virtual void MputMFlag(sc_dt::uint64 nextValue,
          sc_dt::uint64 mask) = 0;
  virtual bool MgetSError(void) const = 0;
  virtual sc_dt::uint64 MgetSFlag(void) const = 0;
  virtual bool MgetSInterrupt(void) const = 0;

  //---------------------------------------------------------------------
  // public OCP Master Sideband Events
  //---------------------------------------------------------------------
  virtual const sc_core::sc_event& SidebandSErrorEvent(void) const = 0;
  virtual const sc_core::sc_event& SidebandSFlagEvent(void) const = 0;
  virtual const sc_core::sc_event& SidebandSInterruptEvent(void) const = 0;

  //---------------------------------------------------------------------
  // public OCP System Sideband Commands
  //---------------------------------------------------------------------
  virtual bool SysputControl(int nextValue) = 0;
  virtual bool SysgetControlBusy(void) const = 0;
  virtual void SysputControlWr(bool nextValue) = 0;
  virtual int SysgetStatus(void) const = 0;
  virtual bool readStatus(int& currentValue) const = 0;
  virtual bool SysgetStatusBusy(void) const = 0;
  virtual void SysputStatusRd(bool nextValue) = 0;

  //---------------------------------------------------------------------
  // public OCP System Sideband Events
  //---------------------------------------------------------------------
  virtual const sc_core::sc_event& SidebandControlBusyEvent(void) const = 0;
  virtual const sc_core::sc_event& SidebandStatusEvent(void) const = 0;
  virtual const sc_core::sc_event& SidebandStatusBusyEvent(void) const = 0;

  //---------------------------------------------------------------------
  // public OCP Core Sideband Commands
  //---------------------------------------------------------------------
  virtual int CgetControl(void) const = 0;
  virtual void CputControlBusy(bool nextValue) = 0;
  virtual bool CgetControlWr(void) const = 0;
  virtual void CputStatus(int nextValue) = 0;
  virtual void CputStatusBusy(bool nextValue) = 0;
  virtual bool CgetStatusRd(void) const = 0;

  //---------------------------------------------------------------------
  // public OCP Core Sideband Events
  //---------------------------------------------------------------------
  virtual const sc_core::sc_event& SidebandControlEvent(void) const = 0;
  virtual const sc_core::sc_event& SidebandControlWrEvent(void) const = 0;
  virtual const sc_core::sc_event& SidebandStatusRdEvent(void) const = 0;


  //---------------------------------------------------------------------
  // Public access methods for 'Release' events.
  // Note: these events are only needed in a very special case (the TL0-TL1
  // adapters), and should not be used to code 'normal' TL1 models.
  //---------------------------------------------------------------------
  virtual const sc_core::sc_event& RequestReleaseEvent(void) const = 0;
  virtual const sc_core::sc_event& DataHSReleaseEvent(void) const = 0;
  virtual const sc_core::sc_event& ResponseReleaseEvent(void) const = 0;

  //---------------------------------------------------------------------
  // old OCP Master methods deprecated
  //---------------------------------------------------------------------

  // Generic Default Event
  // This event is triggered if an Mput or an Sput method is called
  // in the Channel connected to the Master.
  virtual const sc_core::sc_event& default_event() const = 0;

  // Generic Access methods to the user defined data structures of the Channel.
  virtual TdataCl          *GetDataCl()    = 0;
  virtual ParamCl<TdataCl> *GetParamCl()   = 0;
  virtual CommCl           *GetCommCl()    = 0;
  virtual OCPParameters    *GetParameters()= 0;

  // Method for the master to register itself as timing-sensitive
  virtual void registerTimingSensitiveOCPTL1Master(OCP_TL1_Slave_TimingIF *master) = 0;

};

#endif
