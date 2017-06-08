// 
//  (c) Copyright OCP-IP 2004
//  OCP-IP Confidential and Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : Alan Kamas for Sonics, Inc.
//                based on previous work by Yann Bajot, Norman Weyrich, 
//                Anssi Haverinen, and Joe Chou.
//          $Id:
//
//  Description : OCP TL2 Master Interface
//
// ============================================================================


#ifndef __OCP_TL2_MASTER_IF_LEGACY_H__
#define __OCP_TL2_MASTER_IF_LEGACY_H__

// forward definition of OCP parameter class.
class OCPParmeters;

template <class Tdata, class Taddr>
class OCP_TL2_MasterIF :  virtual public sc_core::sc_interface
{
  public:
  
    //---------------------------------------------------------------------
    // public OCP Master methods for channel configuration
    //---------------------------------------------------------------------
    virtual void setOCPMasterConfiguration(MapStringType& passedMap) = 0;
    virtual void addOCPConfigurationListener(OCP_TL_Config_Listener& listener) = 0;
    virtual const string peekChannelName()	const = 0;

    //////////////////////////////////////////////////////////////
    // OCP TL2 Specific Master methods
    //////////////////////////////////////////////////////////////

    virtual bool sendOCPRequest(const OCPTL2RequestGrp<Tdata,Taddr>& req) =0;
    virtual bool sendOCPRequestBlocking(const OCPTL2RequestGrp<Tdata,Taddr>& req) =0;

    virtual bool getOCPResponse(OCPTL2ResponseGrp<Tdata>& resp) =0;
    virtual bool getOCPResponseBlocking(OCPTL2ResponseGrp<Tdata>& resp) =0;
    
    virtual bool acceptResponse(void) = 0;
    virtual bool acceptResponse(const sc_core::sc_time& time) = 0;
    virtual bool acceptResponse(int cycles) = 0;

    virtual bool responseInProgress(void) const = 0;
    virtual bool requestInProgress(void) const = 0;

    // Get Channel Parameter Pointer 
    virtual OCPParameters* GetParamCl(void) =0;

    // TL2 Timing Methods.
    virtual void putMasterTiming(const MTimingGrp& mTimes) = 0;
    virtual void getMasterTiming(MTimingGrp& mTimes) const = 0;
    virtual void getSlaveTiming(STimingGrp& sTimes) const = 0;
    virtual const sc_core::sc_time& getPeriod(void) const = 0; 

    // TL2 Tming Helper Functions
    virtual int getWDI(void) const = 0;
    virtual int getRqI(void) const = 0;
    virtual int getTL2ReqDuration(void) const = 0;
    virtual int getRDI(void) const= 0;
    virtual int getTL2RespDuration(void) const = 0;

    // ThreadBusy Commands
    virtual bool getSThreadBusyBit(unsigned int ThreadID = 0) const = 0;
    virtual bool getMThreadBusyBit(unsigned int ThreadID = 0) const = 0;
    virtual void putMThreadBusyBit(bool nextBitValue, unsigned int ThreadID = 0) = 0;

    // Reset Commands
    virtual void MResetAssert(void) =0;
    virtual void MResetDeassert(void) =0;
    virtual bool getReset(void) = 0;

    // Sideband methods
    virtual void MputMError(bool nextValue) = 0;
    virtual void MputMFlag(sc_dt::uint64 nextValue) = 0;
    virtual bool MgetSError(void) const = 0;
    virtual sc_dt::uint64 MgetSFlag(void) const = 0;
    virtual bool MgetSInterrupt(void) const = 0;
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
    virtual const sc_core::sc_event& SThreadBusyEvent(void) const = 0;
    virtual const sc_core::sc_event& SlaveTimingEvent(void) const = 0;
    virtual const sc_core::sc_event& ResetStartEvent(void) const = 0;
    virtual const sc_core::sc_event& ResetEndEvent(void) const = 0;
    virtual const sc_core::sc_event& SidebandSlaveEvent(void) const = 0; 
    virtual const sc_core::sc_event& SidebandCoreEvent(void) const = 0;
    virtual const sc_core::sc_event& SidebandSystemEvent(void) const = 0;

    //////////////////////////////////////////////////////////////
    // OCP Orginal TL2 Master methods for backward compatibility
    //////////////////////////////////////////////////////////////
    virtual bool sendOCPRequest(const OCPRequestGrp<Tdata,Taddr>& req, 
            unsigned int ReqChunkLen = 1, 
            bool last_chunk_of_a_burst = true) = 0;

    virtual bool startOCPRequest(const OCPRequestGrp<Tdata,Taddr>& req, 
            unsigned int ReqChunkLen = 1, 
            bool last_chunk_of_a_burst = true) = 0;

    virtual bool sendOCPRequestBlocking(
            const OCPRequestGrp<Tdata,Taddr>& req, 
            unsigned int ReqChunkLen = 1, 
            bool last_chunk_of_a_burst = true) = 0;

    virtual bool startOCPRequestBlocking(
            const OCPRequestGrp<Tdata,Taddr>& req, 
            unsigned int ReqChunkLen = 1, 
            bool last_chunk_of_a_burst = true) = 0;

    virtual bool getOCPResponse(OCPResponseGrp<Tdata>& resp, 
            bool accept, 
            unsigned int& RespChunkLen, 
            bool& last_chunk_of_a_burst) = 0;

    virtual bool getOCPResponseBlocking(OCPResponseGrp<Tdata>& resp, 
            bool accept, 
            unsigned int& RespChunkLen, 
            bool& last_chunk_of_a_burst) = 0;

    virtual bool getSBusy(void) const = 0;

    virtual bool getSCmdAccept(void) const = 0;

    virtual bool putMRespAccept() = 0;

    virtual bool putMRespAccept(const sc_core::sc_time&) = 0;

    virtual void waitSCmdAccept(void) = 0;

    // Original Macro (Serialized) Methods (useful for TL2 testbenches)
    virtual bool OCPReadTransfer(
            const OCPRequestGrp<Tdata,Taddr>& req, 
            OCPResponseGrp<Tdata>& resp, 
            unsigned int TransferLen = 1) = 0;
    
    virtual bool OCPWriteTransfer(
            const OCPRequestGrp<Tdata,Taddr>& req, 
            unsigned int TransferLen = 1) = 0;

    // Old Style ThreadBusy Commands
    virtual void putMThreadBusy(unsigned int nextMThreadBusy) = 0;
    virtual unsigned int getSThreadBusy(void) const = 0;
    virtual unsigned int getMThreadBusy(void) const = 0;

//     // Reset commands for backward compatibility
//     virtual void MputMReset_n(bool nextValue) = 0;
//     virtual bool MgetSReset_n(void) const = 0;
//     // Generic Reset commands
//     // Generic reset, cycles through within one delta cycle
//     virtual void Reset(void) = 0;
//     virtual void reset(void) = 0;
//     virtual void remove_reset(void) =0;
//     virtual bool get_reset(void) = 0;

    // Old Sideband Events
    virtual const sc_core::sc_event& SidebandStatusEvent(void) const = 0;
    virtual const sc_core::sc_event& SidebandControlEvent(void) const = 0;
};

#endif


