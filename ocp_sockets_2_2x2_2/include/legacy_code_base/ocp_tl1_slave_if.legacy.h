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
//  Description : Slave Interface for the OCP TL1 Channel
//
//
//  Old Change History before moving to CVS
//             12/14/2003 Original revision for OCP 2.0
//             03/30/2004 Added putSCmdAccept(bool), putSDataAccept(bool)
//             09/27/2004 Adds public access methods for 'RequestReleaseEvent',
//             'ResponseReleaseEvent' and 'DataHSReleaseEvent' (needed by
//             TL0-TL1 adapters)
// ============================================================================//


#ifndef __OCP_TL1_SLAVE_IF_LEGACY_H__
#define __OCP_TL1_SLAVE_IF_LEGACY_H__

template <class TdataCl>
class OCP_TL1_SlaveIF :
  virtual public sc_core::sc_interface,
  virtual public OCP_TL1_Slave_TimingIF
{
  public:

  typedef typename TdataCl::DataType Td;
  typedef typename TdataCl::AddrType Ta;

  //////////////////////////////////////////////////////////////
  // OCP TL1 specific methods
  //////////////////////////////////////////////////////////////

  //---------------------------------------------------------------------
  // public OCP Slave methods for channel configuration
  //---------------------------------------------------------------------
  virtual void setOCPSlaveConfiguration(MapStringType& passedMap) = 0;
  virtual void addOCPConfigurationListener(OCP_TL_Config_Listener& listener) = 0;
  virtual const std::string	peekChannelName()	const = 0;

  //---------------------------------------------------------------------
  // public OCP methods for the request phase
  //---------------------------------------------------------------------
  virtual bool getOCPRequest(OCPRequestGrp<Td,Ta>& req, bool accept) = 0;
  virtual bool getOCPRequest(OCPRequestGrp<Td,Ta>& req) = 0;
  virtual bool getOCPRequestBlocking(OCPRequestGrp<Td,Ta>& req, bool accept) = 0;
  virtual bool getOCPRequestBlocking(OCPRequestGrp<Td,Ta>& req) = 0;
  virtual bool putSCmdAccept() = 0;
  virtual void putSCmdAccept(bool scmdccept) = 0;
  virtual void putSThreadBusy(unsigned int sthreadbusy) = 0;
  virtual const sc_core::sc_event& RequestStartEvent(void) const = 0;
  virtual const sc_core::sc_event& RequestEndEvent(void) const = 0;

  //---------------------------------------------------------------------
  // public OCP methods for the response phase
  //---------------------------------------------------------------------
  virtual bool getMBusy(void) const = 0;
  virtual unsigned int getMThreadBusy(void) const = 0;
  virtual const sc_core::sc_event& MThreadBusyEvent(void) const = 0; 
  virtual unsigned int getCurrentMThreadBusy(void) const = 0;
  virtual const sc_core::sc_event& CurrentMThreadBusyEvent(void) const = 0;  
  virtual bool startOCPResponse(OCPResponseGrp<Td>& resp) = 0;
  virtual bool startOCPResponseBlocking(OCPResponseGrp<Td>& resp) = 0;
  virtual bool getMRespAccept(void) const = 0;
  virtual void waitMRespAccept(void) = 0;
  virtual const sc_core::sc_event& ResponseStartEvent(void) const = 0;
  virtual const sc_core::sc_event& ResponseEndEvent(void) const = 0;

  //---------------------------------------------------------------------
  // public OCP methods for the data phase
  //---------------------------------------------------------------------
  virtual bool getOCPDataHS(OCPDataHSGrp<Td>& datahs, bool accept) = 0;
  virtual bool getOCPDataHS(OCPDataHSGrp<Td>& datahs) = 0;
  virtual bool getOCPDataHSBlocking(OCPDataHSGrp<Td>& datahs, bool accept) = 0;
  virtual bool getOCPDataHSBlocking(OCPDataHSGrp<Td>& datahs) = 0;
  virtual bool putSDataAccept(void) = 0;
  virtual void putSDataAccept(bool dataaccept) = 0;
  virtual void putSDataThreadBusy(unsigned int sdatathreadbusy) = 0;
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
  virtual void SResetAssert(void) = 0;
  virtual void SResetDeassert(void) = 0;
  virtual const sc_core::sc_event& ResetStartEvent(void) const = 0;
  virtual const sc_core::sc_event& ResetEndEvent(void) const = 0;
  virtual const sc_core::sc_event& getFakeResetEndEvent(void) const =0;
  virtual const sc_core::sc_event& getFakeResetStartEvent(void) const =0;
  virtual const bool		peekMReset_n()		const = 0;
  virtual const bool		peekSReset_n()		const = 0;
  

  //---------------------------------------------------------------------
  // public OCP Slave Sideband Commands
  //---------------------------------------------------------------------
  virtual bool SgetMError(void) const = 0;
  virtual sc_dt::uint64 SgetMFlag(void) const = 0;
  virtual void SputSError(bool nextValue) = 0;
  virtual void SputSFlag(sc_dt::uint64 nextValue) = 0;
  virtual void SputSFlag(sc_dt::uint64 nextValue,
          sc_dt::uint64 mask) = 0;
  virtual void SputSInterrupt(bool nextValue) = 0;

  //---------------------------------------------------------------------
  // public OCP Slave Sideband Events
  //---------------------------------------------------------------------
  virtual const sc_core::sc_event& SidebandMErrorEvent(void) const = 0;
  virtual const sc_core::sc_event& SidebandMFlagEvent(void) const = 0;

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
  // old OCP methods deprecated
  //---------------------------------------------------------------------

  // Generic default event.
  // This event is triggered if an Mput or an Sput method is called
  // in the Channel connected to the Slave.
  virtual const sc_core::sc_event& default_event() const = 0;

  // Generic access methods to the user defined data structures of the Channel.
  virtual TdataCl          *GetDataCl()    = 0;
  virtual ParamCl<TdataCl> *GetParamCl()   = 0;
  virtual CommCl           *GetCommCl()    = 0;
  virtual OCPParameters    *GetParameters()= 0;

  // Method for the slave to register itself as timing-sensitive
  virtual void registerTimingSensitiveOCPTL1Slave(OCP_TL1_Master_TimingIF *slave) = 0;
};
#endif
