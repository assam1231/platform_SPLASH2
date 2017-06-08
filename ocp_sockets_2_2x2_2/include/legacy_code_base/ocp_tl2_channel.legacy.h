///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Copyright 2004 OCP-IP
// OCP-IP Confidential & Proprietary
//
//
//============================================================================
//      Project : OCP SLD WG
//       Author : Alan Kamas for Sonics, Inc.
//              : This channel is modeled on the original OCP TL2 channel by 
//              : Yann Bajot, Prosilog
//              : Joe Chou, Sonics, Inc.
//              : Anssi Haverinen, Nokia
//              : Norman Weyrich, Synopsys
//          $Id:
//
//  Description :  OCP TL 2 SystemC Channel Model
//        This performance version of the channel is event driven with new
//        timing variables for increased accuracy.
//        This is channel is backward compatible with the original OCP TL2 API.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __OCP_TL2_CHANNEL_LEGACY_H__
#define __OCP_TL2_CHANNEL_LEGACY_H__

// -----------------------------------------------------------------

#define MAX_THREADS 32
#define NOTIFY_OBSERVER(vector,method)\
    void notify_##vector##_observer()\
    {\
      for( observer_iterator_type\
	     it  = m_##vector##_observer.begin(),\
	     end = m_##vector##_observer.end();\
	   it != end;\
	   ++it) {\
	(*it)->Notify##method( this);\
      }\
    }\

template <class Tdata, class Taddr>
class OCP_TL2_Channel :
    public sc_core::sc_module,
    public OCP_TL2_MonitorIF<Tdata,Taddr>,
    public OCP_TL2_MasterIF<Tdata,Taddr>,
    public OCP_TL2_SlaveIF<Tdata,Taddr>
{
    typedef typename OCP_TL2_MonitorIF<Tdata,Taddr>::observer_type observer_type;

  public:
	typedef Tdata Ta;
	typedef Taddr Td;

    friend class OCP_TL2_TMon<Tdata,Taddr>;
    
    // SystemC macro to support the Req & Resp Accept methods
    SC_HAS_PROCESS(OCP_TL2_Channel);

    //---------------------------------------------------------------
    // constructor
    //---------------------------------------------------------------
    OCP_TL2_Channel(sc_core::sc_module_name name, ostream *traceStreamPtr=NULL) :
	sc_core::sc_module(name),
	m_confMan(name, ocpParams),
        m_requestInProgress(false),
        m_requestUnread(false),
        m_responseInProgress(false),
        m_responseUnread(false),
        m_mThreadBusy(0),
        m_sThreadBusy(0),
        m_clkPeriod(1,sc_core::SC_NS),
	m_tMonPtr(NULL)
    { 
        // setup the SystemC methods 
        // for delayed request and response accept
        SC_METHOD(release_request_method);
        sensitive << m_ReleaseRequestEvent;
        dont_initialize();

        SC_METHOD(release_response_method);
        sensitive << m_ReleaseResponseEvent;
        dont_initialize();

        // Set up the monitor (if any)
        if (traceStreamPtr) {
            m_tMonPtr = new OCP_TL2_TMon<Tdata, Taddr> (traceStreamPtr, this);
        }
    }

    //---------------------------------------------------------------
    // destructor
    //---------------------------------------------------------------
    virtual ~OCP_TL2_Channel()
    { 
        // Clean up the transaction monitor (if any)
        if (m_tMonPtr) {
            delete m_tMonPtr;
        }
    }

  public:

    //---------------------------------------------------------------
    // Request Commands
    //---------------------------------------------------------------

    bool sendOCPRequest(const OCPTL2RequestGrp<Tdata,Taddr>& req) 
    {
        // NOTE: for performance reasons, no check for threadbusy here

        // Check to see if there is already a request in progress
        if (! m_requestInProgress)
        {
            // Put the request on the channel
            m_requestInProgress = true;
            m_requestUnread = true;
            m_currentRequest = req;
            // Trigger the event
            if (m_tMonPtr) {
                m_tMonPtr -> tLogNewReq();
            }
            notify_request_start_observer();
            m_RequestStartEvent.notify();
            return true;
        }
        // Already a request in progress. Failed.
        return false;
    }

    bool sendOCPRequestBlocking(const OCPTL2RequestGrp<Tdata,Taddr>& req) 
    {
        // The outer while loop makes sure the channel did get busy while 
        // waiting for threadbusy to clear
        while (m_requestInProgress || 
               (ocpParams.sthreadbusy && getSThreadBusyBit(req.MThreadID))) {
            // We cannot send request as channel is busy or thread is busy
            // Wait for channel to free up
            while (m_requestInProgress) {
                wait(m_RequestEndEvent);
            }
            // Now wait for threadbusy to free up
            while(ocpParams.sthreadbusy && getSThreadBusyBit(req.MThreadID)) {
                wait(m_SThreadBusyEvent);
            }
        } 
        // We can now send our request
        if (!sendOCPRequest(req)) {
            return false;
        }
        // This blocking command ends when the request is accepted
        wait(m_RequestEndEvent);
        return true;
    }

    bool getOCPRequest(OCPTL2RequestGrp<Tdata,Taddr>& req) 
    {
        // Check to see if there is already a request in progress
        // that we haven't read yet
        if (m_requestInProgress && m_requestUnread)
        {
            // There is a request to get
            req = m_currentRequest;
            // Mark it as read
            m_requestUnread = false;
            return true;
        }
        return false;
    }

    bool getOCPRequestBlocking(OCPTL2RequestGrp<Tdata,Taddr>& req) 
    {
        if (!getOCPRequest(req)) {
            wait(m_RequestStartEvent);
            return getOCPRequest(req);
        }
        return true;
    }

    bool acceptRequest(void)
    {
        if (m_requestInProgress) {
            m_requestInProgress = false;
#ifndef NDEBUG
            if (m_requestUnread) {
                cerr << "WARNING: TL2 Channel \"" << name() 
                    << "\" - request was accepted but never read at time: " 
                    << sc_core::sc_time_stamp() << endl;
            }
#endif
            m_requestUnread = false;
            notify_request_end_observer();
            m_RequestEndEvent.notify();
            return true;
        }
        return false;
    }
    
    bool acceptRequest(const sc_core::sc_time& accept_time)
    {
        if (m_requestInProgress) {
            m_ReleaseRequestEvent.notify(accept_time);
            return true;
        }
        return false;
    }

    bool acceptRequest(int cycles)
    {
        if (m_requestInProgress) {
            if (cycles < 0) {
                // Do automatic acceptance based on the
                // estimated number of cycles this request
                // should take
                int estimatedCycles = getTL2ReqDuration();
                m_ReleaseRequestEvent.notify(
                        estimatedCycles * m_clkPeriod);
                return true;
            } else if (cycles == 0) {
                // Accept Right now
                return acceptRequest();
            } else {
                // wait for "cycles" clock periods and accept then
                m_ReleaseRequestEvent.notify(cycles * m_clkPeriod);
                return true;
            }
        }
        return false;
    }

    bool requestInProgress(void) const 
    {
        return m_requestInProgress;
    }

    //---------------------------------------------------------------
    // SystemC Methods to support delayed accepts
    //---------------------------------------------------------------
    void release_request_method(void)
    {
        acceptRequest();
    }


    //---------------------------------------------------------------
    // Old API Request Commands
    // For backward compatibility
    //---------------------------------------------------------------
    bool sendOCPRequest(const OCPRequestGrp<Tdata,Taddr>& req, 
            unsigned int ReqChunkLen = 1, 
            bool last_chunk_of_a_burst = true) 
    {
        // Convert to TL2 request group
        temp_Request.copyFrom(req,ReqChunkLen,last_chunk_of_a_burst);
        
        return sendOCPRequest(temp_Request);
    }

    bool startOCPRequest(const OCPRequestGrp<Tdata,Taddr>& req, 
            unsigned int ReqChunkLen = 1, 
            bool last_chunk_of_a_burst = true) 
    {
        // Convert to TL2 request group
        temp_Request.copyFrom(req,ReqChunkLen,last_chunk_of_a_burst);
        
        return sendOCPRequest(temp_Request);
    }

    bool sendOCPRequestBlocking(const OCPRequestGrp<Tdata,Taddr>& req, 
            unsigned int ReqChunkLen = 1, 
            bool last_chunk_of_a_burst = true) 
    {
        // Convert to TL2 request group
        temp_sendBlkRequest.copyFrom(req,ReqChunkLen,last_chunk_of_a_burst);
        
        return sendOCPRequestBlocking(temp_sendBlkRequest);
    }

    bool startOCPRequestBlocking(const OCPRequestGrp<Tdata,Taddr>& req, 
            unsigned int ReqChunkLen = 1, 
            bool last_chunk_of_a_burst = true) 
    {
        // Convert to TL2 request group
        temp_sendBlkRequest.copyFrom(req,ReqChunkLen,last_chunk_of_a_burst);
        
        if (!sendOCPRequest(temp_sendBlkRequest)) {
            wait(m_RequestEndEvent);
            if (!sendOCPRequest(temp_sendBlkRequest))
            {
                return false;
            }
        }
        return true;
    }
    
    bool getOCPRequest(OCPRequestGrp<Tdata,Taddr>& req, 
            bool accept, 
            unsigned int& ReqChunkLen, 
            bool& last_chunk_of_a_burst)
    {
        if (!getOCPRequest(temp_Request))
        {
            return false;
        } 

        // convert result to original style Request Group
        req.copyFrom(temp_Request);
        ReqChunkLen = temp_Request.DataLength;
        last_chunk_of_a_burst = temp_Request.LastOfBurst;

        // Auto accept if requested.
        if (accept) {
            return acceptRequest();
        }
        return true;
    }

    bool getOCPRequestBlocking(OCPRequestGrp<Tdata,Taddr>& req, 
            bool accept, 
            unsigned int& ReqChunkLen, 
            bool& last_chunk_of_a_burst)
    {
        if (!getOCPRequestBlocking(temp_getBlkRequest))
        {
            return false;
        } 

        // convert result to original style Request Group
        req.copyFrom(temp_getBlkRequest);
        ReqChunkLen = temp_getBlkRequest.DataLength;
        last_chunk_of_a_burst = temp_getBlkRequest.LastOfBurst;

        // Auto accept if requested.
        if (accept) {
            return acceptRequest();
        }
        return true;
    }

    bool getSBusy(void) const
    {
        return m_requestInProgress;
    }
    
    bool putSCmdAccept(void)
    {
        return acceptRequest();
    }

    // NOTE: different than original TL1/TL2 implementation
    bool putSCmdAccept(const sc_core::sc_time& after)
    {
        return acceptRequest( after );
    }

    bool getSCmdAccept(void) const
    {
        return !m_requestInProgress;
    }

    void waitSCmdAccept(void)
    {
        if (!m_requestInProgress) {
            return;
        } else {
            wait(m_RequestEndEvent);
        }
    }

    //---------------------------------------------------------------
    // Response Commands
    //---------------------------------------------------------------

    bool sendOCPResponse(const OCPTL2ResponseGrp<Tdata>& resp) 
    {
        // NOTE: for performance reasons, no check for threadbusy here

        // Check to see if there is already a response in progress
        if (! m_responseInProgress)
        {
            // Put the response on the channel
            m_responseInProgress = true;
            m_responseUnread = true;
            m_currentResponse = resp;
            // Trigger the event
            if (m_tMonPtr) {
                m_tMonPtr->tLogNewResp();
            }
            notify_response_start_observer();
            m_ResponseStartEvent.notify();
            return true;
        }
        // Already a resposne in progress. Failed.
        return false;
    }

    bool sendOCPResponseBlocking(const OCPTL2ResponseGrp<Tdata>& resp) 
    {
        // The outer while loop makes sure the channel did get busy while 
        // waiting for threadbusy to clear
        while (m_responseInProgress || 
               (ocpParams.mthreadbusy && getMThreadBusyBit(resp.SThreadID))) {
            // We cannot send response as channel is busy or thread is busy
            // Wait for channel to free up
            while (m_responseInProgress) {
                wait(m_ResponseEndEvent);
            }
            // Now wait for threadbusy to free up
            while(ocpParams.mthreadbusy && getMThreadBusyBit(resp.SThreadID)) {
                wait(m_MThreadBusyEvent);
            }
        } 
        // We can now send our response
        if (!sendOCPResponse(resp)) {
            return false;
        }
        // This blocking command ends when the response is accepted
        wait(m_ResponseEndEvent);
        return true;
    }
    
    bool getOCPResponse(OCPTL2ResponseGrp<Tdata>& resp) 
    {
        // Check to see if there is already a response in progress
        // that we haven't read yet
        if (m_responseInProgress && m_responseUnread)
        {
            // There is a response to get
            resp = m_currentResponse;
            // Mark it as read
            m_responseUnread = false;
            return true;
        }
        return false;
    }

    bool getOCPResponseBlocking(OCPTL2ResponseGrp<Tdata>& resp)
    {
        if (!getOCPResponse(resp)) {
            wait(m_ResponseStartEvent);
            return getOCPResponse(resp);
        }
        return true;
    }

    bool acceptResponse(void)
    {
        if (m_responseInProgress) {
            m_responseInProgress = false;
#ifndef NDEBUG
            if (m_responseUnread) {
                cerr << "WARNING: TL2 Channel \"" << name() 
                    << "\" - response was accepted but never read at time: " 
                    << sc_core::sc_time_stamp() << endl;
            }
#endif
            m_responseUnread = false;
            notify_response_end_observer();
            m_ResponseEndEvent.notify();
            return true;
        }
        return false;
    }

    bool acceptResponse(const sc_core::sc_time& accept_time)
    {
        if (m_responseInProgress) {
            m_ReleaseResponseEvent.notify(accept_time);
            return true;
        }
        return false;
    }

    bool acceptResponse(int cycles)
    {
        if (m_responseInProgress) {
            if (cycles < 0) {
                // Do automatic acceptance based on the
                // estimated number of cycles this response
                // should take
                int estimatedCycles = getTL2RespDuration();
                m_ReleaseResponseEvent.notify(
                        estimatedCycles * m_clkPeriod);
                return true;
            } else if (cycles == 0) {
                // Accept Right now
                return acceptResponse();
            } else {
                // wait for "cycles" clock periods and accept then
                m_ReleaseResponseEvent.notify(cycles * m_clkPeriod);
                return true;
            }
        }
        return false;
    }

    bool responseInProgress(void) const 
    {
        return m_responseInProgress;
    }
    
    //---------------------------------------------------------------
    // SystemC Methods to support delayed accepts
    //---------------------------------------------------------------
    void release_response_method(void)
    {
        acceptResponse();
    }

    //---------------------------------------------------------------
    // Old API Response Commands
    // For backward compatibility
    //---------------------------------------------------------------

    bool sendOCPResponse(const OCPResponseGrp<Tdata>& resp, 
            unsigned int RespChunkLen = 1, 
            bool last_chunk_of_a_burst = true) 
    {
        // Convert to TL2 response group
        temp_Response.copyFrom(resp,RespChunkLen,last_chunk_of_a_burst);
        
        return sendOCPResponse(temp_Response);
    }

    bool startOCPResponse(const OCPResponseGrp<Tdata>& resp, 
            unsigned int RespChunkLen = 1, 
            bool last_chunk_of_a_burst = true) 
    {
        // Convert to TL2 response group
        temp_Response.copyFrom(resp,RespChunkLen,last_chunk_of_a_burst);
        
        return sendOCPResponse(temp_Response);
    }

    bool sendOCPResponseBlocking(const OCPResponseGrp<Tdata>& resp, 
            unsigned int ReqChunkLen = 1, 
            bool last_chunk_of_a_burst = true) 
    {
        // Convert to TL2 request group
        temp_sendBlkResponse.copyFrom(resp,ReqChunkLen,last_chunk_of_a_burst);
        
        return sendOCPResponseBlocking(temp_sendBlkResponse);
    }

    bool startOCPResponseBlocking(const OCPResponseGrp<Tdata>& resp, 
            unsigned int RespChunkLen = 1, 
            bool last_chunk_of_a_burst = true) 
    {
        // Convert to TL2 request group
        temp_sendBlkResponse.copyFrom(resp,RespChunkLen,last_chunk_of_a_burst);
        
        if (!sendOCPResponse(temp_sendBlkResponse)) {
            wait(m_ResponseEndEvent);
            if (!sendOCPResponse(temp_sendBlkResponse))
            {
                return false;
            }
        }
        return true;
    }

    bool getOCPResponse(OCPResponseGrp<Tdata>& resp, 
            bool accept, 
            unsigned int& RespChunkLen, 
            bool& last_chunk_of_a_burst)
    {
        if (!getOCPResponse(temp_Response))
        {
            return false;
        } 

        // convert result to original style Request Group
        resp.copyFrom(temp_Response);
        RespChunkLen = temp_Response.DataLength;
        last_chunk_of_a_burst = temp_Response.LastOfBurst;

        // Auto accept if requested.
        if (accept) {
            return acceptResponse();
        }
        return true;
    }

    bool getOCPResponseBlocking(OCPResponseGrp<Tdata>& resp, 
            bool accept, 
            unsigned int& RespChunkLen, 
            bool& last_chunk_of_a_burst)
    {
        if (!getOCPResponseBlocking(temp_getBlkResponse))
        {
            return false;
        } 

        // convert result to original style Response Group
        resp.copyFrom(temp_getBlkResponse);
        RespChunkLen = temp_getBlkResponse.DataLength;
        last_chunk_of_a_burst = temp_getBlkResponse.LastOfBurst;

        // Auto accept if requested.
        if (accept) {
            return acceptResponse();
        }
        return true;
    }

    bool getMBusy(void) const
    {
        return m_responseInProgress;
    }

    bool putMRespAccept(void)
    {
        return acceptResponse();
    }

    bool putMRespAccept(const sc_core::sc_time& after)
    {
        return acceptResponse(after);
    }

    bool getMRespAccept(void) const
    {
        return !m_responseInProgress;
    }

    void waitMRespAccept(void)
    {
        if (!m_responseInProgress) {
            return;
        } else {
            wait(m_ResponseEndEvent);
        }
    }

    //---------------------------------------------------------------
    // Old API Serialized Commands
    // For backward compatibility
    //---------------------------------------------------------------
    bool OCPReadTransfer(const OCPRequestGrp<Tdata,Taddr>& req,
            OCPResponseGrp<Tdata>& resp,
            unsigned int TransferLen =1)
    {
        if( req.MCmd != OCP_MCMD_RD ) {
            return false;
        }
        if (getSThreadBusyBit(req.MThreadID)) {
            return false;
        }
        if (!sendOCPRequestBlocking(req,TransferLen,true)) {
            return false; 
        }
        bool lastChunk;
        return getOCPResponseBlocking(resp, true, TransferLen, lastChunk);
    }

    bool OCPWriteTransfer(const OCPRequestGrp<Tdata,Taddr>& req,
            unsigned int TransferLen =1)
    {
        if( req.MCmd != OCP_MCMD_WR ) {
            return false;
        }
        if (getSThreadBusyBit(req.MThreadID)) {
            return false;
        }
        return sendOCPRequestBlocking(req,TransferLen,true);
    } 
        
    //---------------------------------------------------------------
    // ThreadBusy Commands
    //---------------------------------------------------------------
    
    bool getSThreadBusyBit(unsigned int myThreadID = 0) const 
    { 
        // NOTE: could check to see if threadbusy is part of the channel here.
        //       (if so, then do inside #ifndef NDEBUG )
        // NOTE: could check that myThreadID is in range.
        //       (if so, then do inside #ifndef NDEBUG )

        return m_sThreadBusy[myThreadID];
    }

    void putSThreadBusyBit(bool nextBitValue, unsigned int myThreadID = 0) 
    {
        // NOTE: could check that myThreadID is in range.
        //       (if so, then do inside #ifndef NDEBUG )
        // NOTE: could check to see if threadbusy is part of the channel here.
        //       (if so, then do inside #ifndef NDEBUG )
        m_sThreadBusy[myThreadID] = nextBitValue;
        if (sc_core::sc_get_curr_simcontext()->is_running()) {
            // only fire the event when the simulation is running.
            // Firing during end_of_elaboration or start_of_simulation
            // could cause problems.
	    notify_sthredbusy_observer();
            m_SThreadBusyEvent.notify();
        }
    }

    bool getMThreadBusyBit(unsigned int myThreadID = 0) const 
    { 
        // NOTE: could check to see if threadbusy is part of the channel here.
        //       (if so, then do inside #ifndef NDEBUG )
        // NOTE: could check that myThreadID is in range.
        //       (if so, then do inside #ifndef NDEBUG )

        return m_mThreadBusy[myThreadID];
    }

    void putMThreadBusyBit(bool nextBitValue, unsigned int myThreadID = 0) 
    {
        // NOTE: could check that myThreadID is in range.
        //       (if so, then do inside #ifndef NDEBUG )
        // NOTE: could check to see if threadbusy is part of the channel here.
        //       (if so, then do inside #ifndef NDEBUG )
        m_mThreadBusy[myThreadID] = nextBitValue;
        if (sc_core::sc_get_curr_simcontext()->is_running()) {
            // only fire the event when the simulation is running.
            // Firing during end_of_elaboration or start_of_simulation
            // could cause problems.
	    notify_mthredbusy_observer();
            m_MThreadBusyEvent.notify();
        }
    }

    //---------------------------------------------------------------
    // Old API ThreadBusy Commands
    // For backward compatibility
    //---------------------------------------------------------------
    void putMThreadBusy(unsigned int nextMThreadBusy)
    {
        m_mThreadBusy = bitset<MAX_THREADS>(nextMThreadBusy);
        if (sc_core::sc_get_curr_simcontext()->is_running()) {
            // only fire the event when the simulation is running.
            // Firing during end_of_elaboration or start_of_simulation
            // could cause problems.
	    notify_mthredbusy_observer();
            m_MThreadBusyEvent.notify();
        }
    }

    void putSThreadBusy(unsigned int nextSThreadBusy)
    {
        m_sThreadBusy = bitset<MAX_THREADS>(nextSThreadBusy);
        if (sc_core::sc_get_curr_simcontext()->is_running()) {
            // only fire the event when the simulation is running.
            // Firing during end_of_elaboration or start_of_simulation
            // could cause problems.
	    notify_sthredbusy_observer();
            m_SThreadBusyEvent.notify();
        }
    }

    unsigned int getMThreadBusy(void) const
    {
        return m_mThreadBusy.to_ulong();
    }

    unsigned int getSThreadBusy(void) const
    {
        return m_sThreadBusy.to_ulong();
    }

    //---------------------------------------------------------------
    // Timing Value Functions
    //---------------------------------------------------------------

    void putMasterTiming(const MTimingGrp& mTimes)
    {
        m_mTiming = mTimes;
        if (sc_core::sc_get_curr_simcontext()->is_running()) {
            // only fire the event when the simulation is running.
            // Firing during end_of_elaboration or start_of_simulation
            // could cause problems.
	    notify_master_timing_observer();
            m_MasterTimingEvent.notify();
        }
    }

    void getMasterTiming(MTimingGrp& mTimes) const
    {
        mTimes = m_mTiming;
    }

    void putSlaveTiming(const STimingGrp& sTimes)
    {
        m_sTiming = sTimes;
        if (sc_core::sc_get_curr_simcontext()->is_running()) {
            // only fire the event when the simulation is running.
            // Firing during end_of_elaboration or start_of_simulation
            // could cause problems.
	    notify_slave_timing_observer();
            m_SlaveTimingEvent.notify();
        }
    }

    void getSlaveTiming(STimingGrp& sTimes) const
    {
        sTimes = m_sTiming;
    }

    void setPeriod( const sc_core::sc_time& newPeriod )
    {
        m_clkPeriod = newPeriod;
    }

    const sc_core::sc_time& getPeriod(void) const 
    {
        return m_clkPeriod;
    }

    //---------------------------------------------------------------
    // Timing Helper Functions
    //---------------------------------------------------------------

    // Write Data Interval
    int getWDI(void) const   
    {
        return max(m_mTiming.DSndI, m_sTiming.DAL);
    }
    
    // Read Request Interval
    int getRqI(void) const   
    {
        return max(m_mTiming.RqSndI, m_sTiming.RqAL);
    }

    int getTL2ReqDuration(void) const    
    {
        // Add Request Data Latency only if this is a write request of some sort
        if ( (m_currentRequest.MCmd==OCP_MCMD_WR) ||
                (m_currentRequest.MCmd==OCP_MCMD_WRNP) ||
                (m_currentRequest.MCmd==OCP_MCMD_WRC) ||
                (m_currentRequest.MCmd==OCP_MCMD_BCST) ) {
            return(m_mTiming.RqDL + (getWDI())*(m_currentRequest.DataLength));
        }
        // A read request
        int numReqs = m_currentRequest.DataLength;
        if ( ocpParams.burstsinglereq && m_currentRequest.MBurstSingleReq ) {
            // SRMD: Single Request, Multiple Data
            numReqs = 1;
        }
        // Duration is that time for one request times number of requests
        return((getRqI())*(numReqs) );
    }

    // Response Data Interval
    int getRDI(void) const
    {
        return max(m_sTiming.RpSndI, m_mTiming.RpAL);
    }
        
    // NOTE: Assumes here that if channel supports SRMD, then the write request
    // was sent SRMD. If this is not the case, then this helper funciton should
    // not be used and the timing should instead be computed on a case by
    // case basis.
    int getTL2RespDuration(void) const
    {
        // Check for SRMD write response
        int numResp = m_currentResponse.DataLength;
        if ( ocpParams.burstsinglereq && (m_currentResponse.SDataPtr==NULL)){
            // SRMR write response
            numResp = 1;
        }
        return( (getRDI())*(numResp) );
    }

    // -------------------------------------------------------------
    // RESET
    // Reset functions for backward compatibility
    // -------------------------------------------------------------
    void MResetAssert(void)
    {
        // Set the reset signal
        m_sideband.MReset_n = false;
        // Call the reset event
        if (sc_core::sc_get_curr_simcontext()->is_running()) {
            // only fire the event when the simulation is running.
            // Firing during end_of_elaboration or start_of_simulation
            // could cause problems.
	    notify_reset_start_observer();
            m_ResetStartEvent.notify();
        }
    }
    void MResetDeassert(void)
    {
        // Now pull out of reset mode
        m_sideband.MReset_n = true;
        // Call the end of reset event
        if (sc_core::sc_get_curr_simcontext()->is_running()) {
            // only fire the event when the simulation is running.
            // Firing during end_of_elaboration or start_of_simulation
            // could cause problems.
	    notify_reset_end_observer();
            m_ResetEndEvent.notify();
        }
    }
    void SResetAssert(void)
    {
        // Set the reset signal
        m_sideband.SReset_n = false;
        // Call the reset event
        if (sc_core::sc_get_curr_simcontext()->is_running()) {
            // only fire the event when the simulation is running.
            // Firing during end_of_elaboration or start_of_simulation
            // could cause problems.
	    notify_reset_start_observer();
            m_ResetStartEvent.notify();
        }
    }
    void SResetDeassert(void)
    {
        // Now pull out of reset mode
        m_sideband.SReset_n = true;
        // Call the end of reset event
        if (sc_core::sc_get_curr_simcontext()->is_running()) {
            // only fire the event when the simulation is running.
            // Firing during end_of_elaboration or start_of_simulation
            // could cause problems.
	    notify_reset_end_observer();
            m_ResetEndEvent.notify();
        }
    }
    bool getReset(void) 
    { 
        return ((m_sideband.MReset_n==false)||(m_sideband.SReset_n==false));
    }

    bool getSReset(void) 
    { return (m_sideband.SReset_n==false); }

    bool getMReset(void) 
    { return (m_sideband.MReset_n==false); }

//     // -------------------------------------------------------------
//     // RESET
//     // Old TL2 API & Generic Methods for Reset
//     // -------------------------------------------------------------
//     void reset(void)
//     {
//         // Set both reset signals
//         m_sideband.MReset_n = false;
//         m_sideband.SReset_n = false;
//         // Call the reset event
//         m_ResetStartEvent.notify();
//     }
//     void remove_reset(void)
//     {
//         // Clear both resets
//         // Set both reset signals
//         m_sideband.MReset_n = true;
//         m_sideband.SReset_n = true;
//         // Call the reset end event
//         m_ResetEndEvent.notify();
//     }
//     bool get_reset(void)
//     {
//         return getReset();
//     }
//     // NOTE: Slightly different implementation then original TL2 channel.
//     // NOTE: Can only be called from an SC_THREAD due to the wait.
//     void Reset(void) {
//         reset();
//         wait(SC_ZERO_TIME);
//         remove_reset();
//     }
//     // Sideband reset replaced with reset assert/deassert calls
//     void SputSReset_n(bool nextValue)
//     {
//         noSupport("SputSReset_n","SResetAssert() and SResetDeassert()");
//     }
//     bool SgetMReset_n(void) const
//     {
//         noSupport("SgetMReset_n","getReset");
//         return false;
//     }
//     // Sideband reset replaced with reset assert/deassert calls
//     void MputMReset_n(bool nextValue)
//     {
//         noSupport("SputMReset_n","MResetAssert() and MResetDeassert()");
//     }
//     bool MgetSReset_n(void) const
//     {
//         noSupport("MgetSReset_n","getReset");
//         return false;
//     }
    
    // TODO:(Alan Kamas) Document sideband commands.

    //---------------------------------------------------------------------
    // SIDEBAND methods
    //---------------------------------------------------------------------

    // Master SideBand Group Access methods
    void MputMError(bool nextValue)
    {
        // only trigger event if signal has actually changed
        if (m_sideband.MError!=nextValue) {
            m_sideband.MError=nextValue;
            if (sc_core::sc_get_curr_simcontext()->is_running()) {
                // only fire the event when the simulation is running.
                // Firing during end_of_elaboration or start_of_simulation
                // could cause problems.
	        notify_sideband_master_observer();	    
                m_SidebandMasterEvent.notify();
            }
        }
    }
    void MputMFlag(sc_dt::uint64 nextValue)
    {
        // only trigger event if signal has actually changed
        if (m_sideband.MFlag!=nextValue) {
            m_sideband.MFlag=nextValue;
            if (sc_core::sc_get_curr_simcontext()->is_running()) {
                // only fire the event when the simulation is running.
                // Firing during end_of_elaboration or start_of_simulation
                // could cause problems.
	        notify_sideband_master_observer();	    
                m_SidebandMasterEvent.notify();
            }
        }
    }
    bool MgetSError(void) const
    {
        return m_sideband.SError;
    }
    sc_dt::uint64 MgetSFlag(void) const
    {
        return m_sideband.SFlag;
    }
    bool MgetSInterrupt(void) const
    {
        return m_sideband.SInterrupt;
    }
    // Slave SideBand Group Access methods
    bool SgetMError(void) const
    {
        return m_sideband.MError;
    }
    sc_dt::uint64 SgetMFlag(void) const
    {
        return m_sideband.MFlag;
    }
    void SputSError(bool nextValue)
    {
        // only trigger event if signal has actually changed
        if (m_sideband.SError!=nextValue) {
            m_sideband.SError=nextValue;
            if (sc_core::sc_get_curr_simcontext()->is_running()) {
                // only fire the event when the simulation is running.
                // Firing during end_of_elaboration or start_of_simulation
                // could cause problems.
	        notify_sideband_slave_observer();	    
                m_SidebandSlaveEvent.notify();
            }
        }
    }
    void SputSFlag(sc_dt::uint64 nextValue)
    {
        // only trigger event if signal has actually changed
        if (m_sideband.SFlag!=nextValue) {
            m_sideband.SFlag=nextValue;
            if (sc_core::sc_get_curr_simcontext()->is_running()) {
                // only fire the event when the simulation is running.
                // Firing during end_of_elaboration or start_of_simulation
                // could cause problems.
	        notify_sideband_slave_observer();	    
                m_SidebandSlaveEvent.notify();
            }
        }
    }
    void SputSInterrupt(bool nextValue)
    {
        // only trigger event if signal has actually changed
        if (m_sideband.SInterrupt!=nextValue) {
            m_sideband.SInterrupt=nextValue;
            if (sc_core::sc_get_curr_simcontext()->is_running()) {
                // only fire the event when the simulation is running.
                // Firing during end_of_elaboration or start_of_simulation
                // could cause problems.
	        notify_sideband_slave_observer();	    
                m_SidebandSlaveEvent.notify();
            }
        }
    }
    //---------------------------------------------------------------------
    // System/Core Sideband signals 
    //---------------------------------------------------------------------
    void SysputControl(unsigned int nextValue)
    {
        // only trigger event if signal has actually changed
        if (m_sideband.Control!=nextValue) {
            m_sideband.Control=nextValue;
            if (sc_core::sc_get_curr_simcontext()->is_running()) {
                // only fire the event when the simulation is running.
                // Firing during end_of_elaboration or start_of_simulation
                // could cause problems.
	        notify_sideband_system_observer();	    
                m_SidebandSystemEvent.notify();
            }
        }
    }
    bool SysgetControlBusy(void) const
    {
        return m_sideband.ControlBusy;
    }
    void SysputControlWr(bool nextValue)
    {
        // only trigger event if signal has actually changed
        if (m_sideband.ControlWr!=nextValue) {
            m_sideband.ControlWr=nextValue;
            if (sc_core::sc_get_curr_simcontext()->is_running()) {
                // only fire the event when the simulation is running.
                // Firing during end_of_elaboration or start_of_simulation
                // could cause problems.
	        notify_sideband_system_observer();	    
                m_SidebandSystemEvent.notify();
            }
        }
    }
    unsigned int SysgetStatus(void) const
    {
        return m_sideband.Status;
    }
    bool SysgetStatusBusy(void) const
    {
        return m_sideband.StatusBusy;
    }
    void SysputStatusRd(bool nextValue)
    {
        // only trigger event if signal has actually changed
        if (m_sideband.StatusRd!=nextValue) {
            m_sideband.StatusRd=nextValue;
            if (sc_core::sc_get_curr_simcontext()->is_running()) {
                // only fire the event when the simulation is running.
                // Firing during end_of_elaboration or start_of_simulation
                // could cause problems.
	        notify_sideband_system_observer();	    
                m_SidebandSystemEvent.notify();
            }
        }

    }
    unsigned int CgetControl(void) const
    {
        return m_sideband.Control;
    }
    void CputControlBusy(bool nextValue)
    {
        // only trigger event if signal has actually changed
        if (m_sideband.ControlBusy!=nextValue) {
            m_sideband.ControlBusy=nextValue;
            if (sc_core::sc_get_curr_simcontext()->is_running()) {
                // only fire the event when the simulation is running.
                // Firing during end_of_elaboration or start_of_simulation
                // could cause problems.
	        notify_sideband_core_observer();	    
                m_SidebandCoreEvent.notify();
            }
        }
    }
    bool CgetControlWr(void) const
    {
        return m_sideband.ControlWr;
    }
    void CputStatus(unsigned int nextValue)
    {
        // only trigger event if signal has actually changed
        if (m_sideband.Status!=nextValue) {
            m_sideband.Status=nextValue;
            if (sc_core::sc_get_curr_simcontext()->is_running()) {
                // only fire the event when the simulation is running.
                // Firing during end_of_elaboration or start_of_simulation
                // could cause problems.
	        notify_sideband_core_observer();	    
                m_SidebandCoreEvent.notify();
            }
        }
    }
    void CputStatusBusy(bool nextValue)
    {
        // only trigger event if signal has actually changed
        if (m_sideband.StatusBusy!=nextValue) {
            m_sideband.StatusBusy=nextValue;
            if (sc_core::sc_get_curr_simcontext()->is_running()) {
                // only fire the event when the simulation is running.
                // Firing during end_of_elaboration or start_of_simulation
                // could cause problems.
	        notify_sideband_core_observer();	    
                m_SidebandCoreEvent.notify();
            }
        }
    }
    bool CgetStatusRd(void) const
    {
        return m_sideband.StatusRd;
    }

    //---------------------------------------------------------------------
    // Configuration Functions
    //---------------------------------------------------------------------

    void setConfiguration( MapStringType& passedMap )
    {
        // Use the passed data base to set up the configuration class
        //ocpParams.setOCPConfiguration( name(), passedMap );
		m_confMan.setExternalConfiguration(passedMap);

        if (m_tMonPtr) {
            m_tMonPtr->tLogSetUp();
            m_tMonPtr->tLogHeader();
        }
    } 

    void setConfiguration( const OCPParameters& params )
    {
        // Use the passed data base to set up the configuration class
        ocpParams = params;

        if (m_tMonPtr) {
            m_tMonPtr->tLogSetUp();
            m_tMonPtr->tLogHeader();
        }
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

    //---------------------------------------------------------------
    // Event Access
    //---------------------------------------------------------------

    const sc_core::sc_event& RequestStartEvent(void) const
    {
        return m_RequestStartEvent;
    }
    const sc_core::sc_event& RequestEndEvent(void) const
    {
        return m_RequestEndEvent;
    }
    const sc_core::sc_event& ResponseStartEvent(void) const
    {
        return m_ResponseStartEvent;
    }
    const sc_core::sc_event& ResponseEndEvent(void) const
    {
        return m_ResponseEndEvent;
    }
    const sc_core::sc_event& SThreadBusyEvent(void) const
    {
        return m_SThreadBusyEvent;
    }
    const sc_core::sc_event& MThreadBusyEvent(void) const
    {
        return m_MThreadBusyEvent;
    }
    const sc_core::sc_event& MasterTimingEvent(void) const
    {
        return m_MasterTimingEvent;
    }
    const sc_core::sc_event& SlaveTimingEvent(void) const
    {
        return m_SlaveTimingEvent;
    }
    const sc_core::sc_event& ResetStartEvent(void) const
    {
        return m_ResetStartEvent;
    }
    const sc_core::sc_event& ResetEndEvent(void) const
    {
        return m_ResetEndEvent;
    }
    const sc_core::sc_event& SidebandMasterEvent(void) const
    {
        return m_SidebandMasterEvent;
    }
    const sc_core::sc_event& SidebandSlaveEvent(void) const{
        return m_SidebandSlaveEvent;
    }
    const sc_core::sc_event& SidebandSystemEvent(void) const
    {
        return m_SidebandSystemEvent;
    }
    const sc_core::sc_event& SidebandCoreEvent(void) const
    {
        return m_SidebandCoreEvent;
    }
    const sc_core::sc_event& SidebandControlEvent(void) const
    {
        noSupport("SidebandControlEvent()",
                "SidebandSystemEvent() and SidebandCoreEvent()");
        return m_SidebandCoreEvent;
    }
    const sc_core::sc_event& SidebandStatusEvent(void) const
    {
        noSupport("SidebandStatusEvent()",
                "SidebandSystemEvent() and SidebandCoreEvent()");
        return m_SidebandSystemEvent;
    }

    //---------------------------------------------------------------
    // Old API Data Class Commands
    // For backward compatibility
    //---------------------------------------------------------------
    
    // "No Support" error messages
    void noSupport(string oldCommandName, string newCommandName = "noNewCommand") const
    {
        cerr << "ERROR: TL2 Channel \"" << name() 
            << "\" no longer supports the command: \"" << oldCommandName 
            << "\"." << endl;
        if (newCommandName != "noNewCommand") {
            cerr << "       Please use new command \"" 
                << newCommandName << "\" instead." << endl;
        }
        assert(0);
    }
    void noSignal(string oldSignalName, string newSignalName = "noNewSignal") const
    {
        cerr << "WARNING: The TL2 Channel \"" << name() 
            << "\" no longer supports the TL1 OCP signal: \"" << oldSignalName 
            << "\"." << endl;
        if (newSignalName != "noNewSignal") {
            cerr << "         Please use new signal \"" 
                << newSignalName << "\" instead." << endl;
        }
    }


    virtual void register_port(sc_core::sc_port_base& port, const char* if_typename)
    {
      std::string nm( if_typename ); 
      if( nm == typeid( OCP_TL2_MasterIF<Tdata,Taddr> ).name() ) {
	m_master_name = port.name();
      }
      if( nm == typeid( OCP_TL2_SlaveIF<Tdata,Taddr> ).name() ) {
	m_slave_name = port.name();
      }
    }

    // New alternative function
    OCPParameters* GetParamCl(void)
    {
        return &(ocpParams);
    }

    //----------------------------------------------------------------------
    // Monitor methods
    //----------------------------------------------------------------------
    const string			peekChannelName()	const
    {
      return name();
    }
    const string			peekMasterPortName()	const
    {
      return m_master_name;
    }
    const string			peekSlavePortName()	const
    {
      return m_slave_name;
    }
    const OCPTL2RequestGrp<Tdata,Taddr>& peekOCPRequest()	const
    {
      return m_currentRequest;
    }
    const OCPTL2ResponseGrp<Tdata>&	peekOCPResponse()	const
    {
      return m_currentResponse;
    }
    const unsigned int			peekMThreadBusy()	const
    {
      return m_mThreadBusy.to_ulong();
    }
    const unsigned int			peekSThreadBusy()	const
    {
      return  m_sThreadBusy.to_ulong();
    }
    const MTimingGrp&			peekMasterTiming()	const
    {
      return m_mTiming;
    }
    const STimingGrp&			peekSlaveTiming()	const
    {
      return m_sTiming;
    }
    const OCPSidebandGrp&		peekSideband()		const
    {
      return m_sideband;
    }

    void RegisterRequestStart(observer_type* monitor)
    { m_request_start_observer.push_back(monitor); }
    void RegisterRequestEnd(observer_type* monitor)
    { m_request_end_observer.push_back(monitor); }
    void RegisterResponseStart(observer_type* monitor)
    { m_response_start_observer.push_back(monitor); }
    void RegisterResponseEnd(observer_type* monitor)
    { m_response_end_observer.push_back(monitor); }

    void RegisterMThreadBusy(observer_type* monitor)
    { m_mthredbusy_observer.push_back(monitor); }
    void RegisterSThreadBusy(observer_type* monitor)
    { m_sthredbusy_observer.push_back(monitor); }
    void RegisterMasterTiming(observer_type* monitor)
    { m_master_timing_observer.push_back(monitor); }
    void RegisterSlaveTiming(observer_type* monitor)
    { m_slave_timing_observer.push_back(monitor); }
    void RegisterResetStart(observer_type* monitor)
    { m_reset_start_observer.push_back(monitor); }
    void RegisterResetEnd(observer_type* monitor)
    { m_reset_end_observer.push_back(monitor); }
    void RegisterSidebandMaster(observer_type* monitor)
    { m_sideband_master_observer.push_back(monitor); }
    void RegisterSidebandSlave(observer_type* monitor)
    { m_sideband_slave_observer.push_back(monitor); }
    void RegisterSidebandCore(observer_type* monitor)
    { m_sideband_core_observer.push_back(monitor); }
    void RegisterSidebandSystem(observer_type* monitor)
    { m_sideband_system_observer.push_back(monitor); }

    NOTIFY_OBSERVER(request_start,RequestStart)
    NOTIFY_OBSERVER(request_end,RequestEnd)
    NOTIFY_OBSERVER(response_start,ResponseStart)
    NOTIFY_OBSERVER(response_end,ResponseEnd)

    NOTIFY_OBSERVER(mthredbusy,MThreadBusy)
    NOTIFY_OBSERVER(sthredbusy,SThreadBusy)
    NOTIFY_OBSERVER(master_timing,MasterTiming)
    NOTIFY_OBSERVER(slave_timing,SlaveTiming)
    NOTIFY_OBSERVER(reset_start,ResetStart)
    NOTIFY_OBSERVER(reset_end,ResetEnd)
    NOTIFY_OBSERVER(sideband_master,SidebandMaster)
    NOTIFY_OBSERVER(sideband_slave,SidebandSlave)
    NOTIFY_OBSERVER(sideband_core,SidebandCore)
    NOTIFY_OBSERVER(sideband_system,SidebandSystem)

    //----------------------------------------------------------------------
    // member data
    //----------------------------------------------------------------------
  public:
    // Channel Parameters
    OCPParameters ocpParams;

  protected:
	Config_manager m_confMan;
  
    // Channel State
    bool m_requestInProgress;
    bool m_requestUnread;
    bool m_responseInProgress;
    bool m_responseUnread;

    // Channel Data
    OCPTL2RequestGrp<Tdata,Taddr> m_currentRequest; 
    OCPTL2ResponseGrp<Tdata> m_currentResponse; 
    
    // ThreadBusy Data
    // TODO (Alan Kamas): the "MAX_THREADS" should be based on the number of threads.
    bitset<MAX_THREADS> m_mThreadBusy;
    bitset<MAX_THREADS> m_sThreadBusy;

    // Timing Data
    MTimingGrp m_mTiming;
    STimingGrp m_sTiming;
    sc_core::sc_time m_clkPeriod;

    // Sideband Data
    OCPSidebandGrp m_sideband;

    // Channel Events
    sc_core::sc_event m_RequestStartEvent;
    sc_core::sc_event m_RequestEndEvent;
    sc_core::sc_event m_ResponseStartEvent;
    sc_core::sc_event m_ResponseEndEvent;
    sc_core::sc_event m_MThreadBusyEvent;
    sc_core::sc_event m_SThreadBusyEvent;
    sc_core::sc_event m_MasterTimingEvent;
    sc_core::sc_event m_SlaveTimingEvent;
    sc_core::sc_event m_ResetStartEvent;
    sc_core::sc_event m_ResetEndEvent;

    // Sideband Events
    sc_core::sc_event m_SidebandMasterEvent;
    sc_core::sc_event m_SidebandSlaveEvent;
    sc_core::sc_event m_SidebandSystemEvent;
    sc_core::sc_event m_SidebandCoreEvent;
    
    // Delayed Acceptance Events
    sc_core::sc_event m_ReleaseRequestEvent;
    sc_core::sc_event m_ReleaseResponseEvent;
    
    // Transaction Logging
    OCP_TL2_TMon<Tdata,Taddr> *m_tMonPtr;

    // Temp Variables
    // members for efficiency only (to avoid new/deletes)
    OCPTL2RequestGrp<Tdata,Taddr> temp_Request;
    OCPTL2RequestGrp<Tdata,Taddr> temp_sendBlkRequest;
    OCPTL2RequestGrp<Tdata,Taddr> temp_getBlkRequest;
    OCPTL2ResponseGrp<Tdata> temp_Response;
    OCPTL2ResponseGrp<Tdata> temp_sendBlkResponse;
    OCPTL2ResponseGrp<Tdata> temp_getBlkResponse;
//     WaitReq<Tdata,Taddr> temp_waitReq;

    std::string m_master_name;
    std::string m_slave_name;

    typedef ::std::vector< observer_type* > observer_container_type;
    typedef typename observer_container_type::iterator observer_iterator_type;
    observer_container_type m_request_start_observer;
    observer_container_type m_request_end_observer;
    observer_container_type m_response_start_observer;
    observer_container_type m_response_end_observer;
    observer_container_type m_mthredbusy_observer;
    observer_container_type m_sthredbusy_observer;
    observer_container_type m_master_timing_observer;
    observer_container_type m_slave_timing_observer;
    observer_container_type m_reset_start_observer;
    observer_container_type m_reset_end_observer;
    observer_container_type m_sideband_master_observer;
    observer_container_type m_sideband_slave_observer;
    observer_container_type m_sideband_core_observer;
    observer_container_type m_sideband_system_observer;
};

#endif
