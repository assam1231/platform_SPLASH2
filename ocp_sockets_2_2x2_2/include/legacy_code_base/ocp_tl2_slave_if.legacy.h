// 
//  (c) Copyright OCP-IP 2003
//  OCP-IP Confidential and Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : Alan Kamas for Sonics, Inc.
//                based on previous work by Yann Bajot, Norman Weyrich, 
//                Anssi Haverinen, and Joe Chou.
//          $Id:
//
//  Description : OCP TL2 Slave Interface
//
// ============================================================================

#ifndef __OCP_TL2_SLAVE_IF_LEGACY_H__
#define __OCP_TL2_SLAVE_IF_LEGACY_H__

template <class Tdata, class Taddr>
class OCP_TL2_SlaveIF :  virtual public sc_core::sc_interface
{
  public:

    //---------------------------------------------------------------------
    // public OCP Slave methods for channel configuration
    //---------------------------------------------------------------------
    virtual void setOCPSlaveConfiguration(MapStringType& passedMap) = 0;
    virtual void addOCPConfigurationListener(OCP_TL_Config_Listener& listener) = 0;
    virtual const string peekChannelName()	const = 0;

    //////////////////////////////////////////////////////////////
    // OCP TL2 Specific Slave Methods
    //////////////////////////////////////////////////////////////

    virtual bool getOCPRequest(OCPTL2RequestGrp<Tdata,Taddr>& req) =0;
    virtual bool getOCPRequestBlocking(OCPTL2RequestGrp<Tdata,Taddr>& req) =0;

    virtual bool sendOCPResponse(const OCPTL2ResponseGrp<Tdata>& resp) =0;
    virtual bool sendOCPResponseBlocking(const OCPTL2ResponseGrp<Tdata>& resp) =0;

    virtual bool acceptRequest(void) = 0;
    virtual bool acceptRequest(const sc_core::sc_time& time) = 0;
    virtual bool acceptRequest(int cycles) = 0;

    virtual bool responseInProgress(void) const = 0;
    virtual bool requestInProgress(void) const = 0;

    // Get OCP Channel Parameter Pointer 
    virtual OCPParameters* GetParamCl(void) =0;

    // TL2 Timing Methods.
    virtual void getMasterTiming(MTimingGrp& mTimes) const = 0;
    virtual void getSlaveTiming(STimingGrp& sTimes) const = 0;
    virtual void putSlaveTiming(const STimingGrp& sTimes) = 0;
    virtual const sc_core::sc_time& getPeriod(void) const = 0;

    // TL2 Tming Helper Functions
    virtual int getWDI(void) const = 0;
    virtual int getRqI(void) const = 0;
    virtual int getTL2ReqDuration(void) const = 0;
    virtual int getRDI(void) const = 0;
    virtual int getTL2RespDuration(void) const = 0;

    // ThreadBusy Commands
    virtual bool getMThreadBusyBit(unsigned int ThreadID = 0) const = 0;
    virtual bool getSThreadBusyBit(unsigned int ThreadID = 0) const = 0;
    virtual void putSThreadBusyBit(bool nextBitValue, unsigned int ThreadID = 0) = 0;
    
    // Reset Commands
    virtual void SResetAssert(void) =0;
    virtual void SResetDeassert(void) =0;
    virtual bool getReset(void) = 0;

    // SideBand Group Access methods
    virtual bool SgetMError(void) const = 0;
    virtual sc_dt::uint64 SgetMFlag(void) const = 0;
    virtual void SputSError(bool nextValue) = 0;
    virtual void SputSFlag(sc_dt::uint64 nextValue) = 0;
    virtual void SputSInterrupt(bool nextValue) = 0;
    virtual void SysputControl(unsigned int nextValue) = 0;
    virtual bool SysgetControlBusy(void) const = 0;
    virtual void SysputControlWr(bool nextValue) = 0;
    virtual unsigned int SysgetStatus(void) const = 0;
    virtual bool SysgetStatusBusy(void) const = 0;
    virtual void SysputStatusRd(bool nextValue) = 0;
    virtual unsigned int CgetControl(void) const = 0;
    virtual void CputControlBusy(bool nextValue) = 0;
    virtual bool CgetControlWr(void) const = 0;
    virtual void CputStatus(unsigned int nextValue) = 0;
    virtual void CputStatusBusy(bool nextValue) = 0;
    virtual bool CgetStatusRd(void) const = 0;

    // Event access
    virtual const sc_core::sc_event& RequestStartEvent(void) const = 0; 
    virtual const sc_core::sc_event& RequestEndEvent(void) const = 0; 
    virtual const sc_core::sc_event& ResponseStartEvent(void) const = 0; 
    virtual const sc_core::sc_event& ResponseEndEvent(void) const = 0; 
    virtual const sc_core::sc_event& MThreadBusyEvent(void) const = 0; 
    virtual const sc_core::sc_event& MasterTimingEvent(void) const = 0; 
    virtual const sc_core::sc_event& ResetStartEvent(void) const = 0; 
    virtual const sc_core::sc_event& ResetEndEvent(void) const = 0; 
    virtual const sc_core::sc_event& SidebandMasterEvent(void) const = 0; 
    virtual const sc_core::sc_event& SidebandSystemEvent(void) const = 0;
    virtual const sc_core::sc_event& SidebandCoreEvent(void) const = 0;
    
    //////////////////////////////////////////////////////////////
    // OCP Original TL2 Slave Methods for backward compatibility
    //////////////////////////////////////////////////////////////

    virtual bool getOCPRequest(OCPRequestGrp<Tdata,Taddr>& req, 
            bool accept, 
            unsigned int& ReqChunkLen, 
            bool& last_chunk_of_a_burst) = 0;

    virtual bool getOCPRequestBlocking(
            OCPRequestGrp<Tdata,Taddr>& req, 
            bool accept, 
            unsigned int& ReqChunkLen, 
            bool& last_chunk_of_a_burst) = 0;

    virtual bool sendOCPResponse(const OCPResponseGrp<Tdata>& resp, 
            unsigned int RespChunkLen = 1, 
            bool last_chunk_of_a_burst = true) = 0;

    virtual bool startOCPResponse(const OCPResponseGrp<Tdata>& resp, 
            unsigned int RespChunkLen = 1, 
            bool last_chunk_of_a_burst = true) = 0;

    virtual bool sendOCPResponseBlocking(
            const OCPResponseGrp<Tdata>& resp, 
            unsigned int RespChunkLen = 1, 
            bool last_chunk_of_a_burst = true) = 0;

    virtual bool startOCPResponseBlocking(
            const OCPResponseGrp<Tdata>& resp, 
            unsigned int RespChunkLen = 1, 
            bool last_chunk_of_a_burst = true) = 0;

    virtual bool getMBusy(void) const = 0;

    virtual bool putSCmdAccept() = 0;

    virtual bool putSCmdAccept(const sc_core::sc_time&) = 0;

    virtual bool getMRespAccept(void) const = 0;

    virtual void waitMRespAccept(void) = 0;

    // Old Style ThreadBusy Commands 
    virtual void putSThreadBusy(unsigned int nextMThreadBusy) = 0;
    virtual unsigned int getMThreadBusy(void) const = 0;
    virtual unsigned int getSThreadBusy(void) const = 0;

//     // Original TL2 Reset commands for backward compatibility
//     virtual void SputSReset_n(bool nextValue) = 0;
//     virtual bool SgetMReset_n(void) const = 0;
//     // Generic Reset commands
//     // Generic reset, cycles through within one delta cycle
//     virtual void Reset(void) = 0;
//     virtual void reset(void) = 0;
//     virtual void remove_reset(void) =0;
//     virtual bool get_reset(void) = 0;

    // Original TL2 Sideband Events - no longer supported
    virtual const sc_core::sc_event& SidebandControlEvent(void) const = 0;
    virtual const sc_core::sc_event& SidebandStatusEvent(void) const = 0;
};

#endif

