// 
//  (c) Copyright OCP-IP 2003, 2004, 2005
//  OCP-IP Confidential and Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : Norman Weyrich, Synopsys Inc.
//                Anssi Haverinen, Nokia Inc.
//                Joe Chou, Sonics Inc.
//                Alan Kamas, for Sonics Inc.
//           $Id: 
//
//  Description : OCP Transaction Level Generic Channel
//    The Generic Channel can be used at all three Transaction Levels.
//    It is based on the generic TL channel released to OSCI
//    (www.systemc.org).
//
//    The Channel is quite generic still, and can be used to implement
//    other interface protocols with different data classes.
//
//    The Generic TL Channel implements the communication between user-written
//    modules. In particular the Channel implements a 
//    Master and a Slave interface. The main features are:
//    - Two way communication: The Generic Channel consists of a
//      . Request Channel, modeling the Master to Slave transfer,
//      . Data Request Channel, modeling the Master to Slave data transfer and a
//      . Response Channel, modeling the Slave to Master transfer
//    - Blocking and non-blocking methods: Read and write methods
//      implemented in the Channel can be blocking or non-blocking.
//    - User configurable data class: The Generic Channel gives Master
//      and Slave access to the user configurable data class.
//      This data class can be used to pass on data and
//      control information between Master and Slave.
//      The data class is a template argument allowing the usage
//      of different data classes in a system.
//    - User configurable parameter class: The parameter class
//      is intented to pass on parameters or setup values like
//      Slave addresses or Master priorities to other modules e.g. the Bus.
//    - Synchronization states and events used in the Channel
//      can be exported and hence made visible to the attached
//      modules by means of the provided communication class.
//
//   The flow of events in the Generic Channel:
//   a) Request Channel
//   - Master submits request for Slave to the Channel.
//     . Channel is blocked until Slave frees the Channel.
//     . If Master used non-blocking method the Master can probe
//       if Channel has been freed by the Slave.
//     . If Master used blocking method, the Master is blocked until the
//       Slave frees the Channel.
//   - The Generic Channel triggers Slave that there is a pending request.
//     . Slave can be made sensitive to a Channel event.
//     . Slave can probe the Channel for a pending request.
//   - Slave receives the pending request and frees the Channel
//   - After the Request Channel has been freed by the Slave the Master
//     is allowed to submit another request.
//   b) Data Request Channel
//   The Data Request Channel works the same as the Request Channel;
//   it is meant for implementing interfaces which use separate data handshake
//   c) Response-Channel
//   The Slave-Response Channel works the same as the Request Channel,
//   except that the roles of Master and Slaves are inverted. That is, the
//   Slave submits the response to the Master and the Master must free the
//   Response Channel for the next Slave response.
//     
// Parameters    :
//   Template arguments.
//     - TdataCl: Data class containing data members and access methods for
//                the data exchanged between Masters and Slaves
//     - Td: Data type
//     - Ta: Address type
//   Constructor arguments.
//     - sc_core::sc_module_name: Name of the module (channel) instance
//     - Synchron: Specifies if the Channel should do a synchronous
//         update (=true) or and asynchronous update (=false)
//         of the Channel events and states.
//         Use the default value unless you know exactly
//         what you are doing.
//     - SyncEvents: Specifies if the Channel should use synchronization
//         events for the communication scheme. The Channel is faster if
//         no synchronization events are used. This could be used e.g.
//         in a Layer-1 application where only non-blocking methods
//         are employed. If blocking methods are used SyncEvents must
//         be true.
//         Use the default value (= true) unless you know exactly
//         what you are doing.
//     - DefaultEvents: Specifies if the Channel should trigger the
//         default event. The Channel is faster if no default event is
//         triggered. DefaultEvent should be set to false if the
//         attached modules are not sensitve to their ports, with other words,
//         if at least one attached module is sensitive to the port,
//         DefaultEvent should be set to true.
//         Use the default value (= true) unless you know exactly
//         what you are doing.
//     - EndTimes: Specifies if transaction end times are recorded.
//         Normally only used by certain OCP channels, default value 
//         should be used otherwise.
//         
//
// Change History: (not updated since version control moved to CVS)
//
// 06/13/2003 Original revision for OCP 1.0
// 12/14/2003 Added OCP 2.0 support
// 05/17/2004 Adds pre-emptive release calls.
//            Adds SemiBlocking methods to support OCP TL2 channel.
//            Some speed optimizations.
//            
// =========================================================================

#ifndef __TL_CHANNEL_LEGACY_H__
#define __TL_CHANNEL_LEGACY_H__

// Added for gcc 3.3:
//using namespace std;

template<class TdataCl> class TL_Channel
: public sc_core::sc_module
{

    //---------------------------------------------------------------
    //
    // Synchronizer class. This class is used to perform a synchronous update
    // of the states and events which control the communication between
    // Master and Slave. In the asynchronouse case the update() method
    // is executed directly from the module TL_Channel, while in the synchronous
    // case this sc_prim_channel is used to call the request_update() method.
    //
    //---------------------------------------------------------------
    class Synchronizer : public sc_core::sc_prim_channel
    {
        public:
            Synchronizer(TL_Channel* Channel) : m_Channel (Channel){}
            void Update() {request_update();}
        protected:
            virtual void update() {m_Channel->update();}
            TL_Channel* m_Channel;
    };


    public:

    typedef typename TdataCl::DataType Td;
    typedef typename TdataCl::AddrType Ta;

    SC_HAS_PROCESS(TL_Channel);

    //---------------------------------------------------------------
    //
    // Constructor
    //
    //---------------------------------------------------------------
    TL_Channel(sc_core::sc_module_name name
            , bool Synchron     = true
            , bool SyncEvent    = true
            , bool DefaultEvent = true
	    , bool EndTimes     = false
            )
        :sc_module        (name)
        , m_Master         (0)
        , m_Slave          (0)
        , m_Synchron       (Synchron)
        , m_SyncEvent      (SyncEvent)
        , m_DefaultEvent   (DefaultEvent)
        , m_EventSelectFw  (0)
        , m_EventSelectFwD (0)
        , m_EventSelectBw  (0)
        , m_EventSelectReset  (0)
      //, m_MdirectIF      (0)
      //, m_SdirectIF      (0)
        , m_Synchronizer   (this)
        , m_EndTimes       (EndTimes)
        {
#ifdef TRACE_C
            outFile = new ofstream(name);
#endif

            if (!m_Synchron) {
                const char * my_name = name;
                std::cout << "Channel " << my_name << ": ERROR: OCP channel model currently does not support Asynchronuos mode" << std::endl;
                // Give up - user is expect channel to behave in a way that is not supported.
                exit(1000);
            }
            m_DataCl  = new TdataCl();
            m_CommCl  = new CommCl();
            m_ParamCl = new ParamCl<TdataCl>();

            SC_METHOD(ReqEnd);
	    sensitive << m_CommCl->RequestEndEventN;
            dont_initialize();

            SC_METHOD(ResEnd);
            sensitive << m_CommCl->ResponseEndEventN;
            dont_initialize();

            SC_METHOD(DataReqEnd);
            sensitive << m_CommCl->DataRequestEndEventN;
            dont_initialize();

	    SC_METHOD(AutoRequestAccept);
            sensitive << m_CommCl->RequestStartEvent;
	    //            sensitive << m_CommCl->RequestReleaseEvent;
            dont_initialize();

	    SC_METHOD(AutoDataRequestAccept);
            sensitive << m_CommCl->DataRequestStartEvent;
	    //            sensitive << m_CommCl->DataRequestReleaseEvent;
            dont_initialize();

            SC_METHOD(AutoResponseAccept);
            sensitive << m_CommCl->ResponseStartEvent;
	    //            sensitive << m_CommCl->ResponseReleaseEvent;
            dont_initialize();

	    SC_METHOD(PERequestAccept);
            sensitive << m_CommCl->RequestReleaseEvent;
            dont_initialize();

	    SC_METHOD(PEDataRequestAccept);
            sensitive << m_CommCl->DataRequestReleaseEvent;
            dont_initialize();

            SC_METHOD(PEResponseAccept);
            sensitive << m_CommCl->ResponseReleaseEvent;
            dont_initialize();

        }

    //---------------------------------------------------------------
    //
    // Destructor
    //
    //---------------------------------------------------------------
    virtual ~TL_Channel()
    {
#ifdef TRACE_C
        delete outFile;
#endif

        delete m_DataCl;
        delete m_CommCl;
        delete m_ParamCl;
    }

    //---------------------------------------------------------------
    //
    // Reset Method
    // Resets Channel
    //
    //---------------------------------------------------------------

    void reset()
    {
        // reset the channel state
        m_EventSelectFw = 0;
        m_EventSelectFwD = 0;
        m_EventSelectBw = 0;
        m_EventSelectReset = 0;

        // reset objects owned by the channel
        m_DataCl->Reset();
        m_CommCl->reset();

        // Sets the special attribute (m_CommCl->Reset) during
        // the current delta-cycle.
        m_CommCl->Reset = true;

        // Notify all the channel events immediatly => generic blocking calls
        // waiting for these calls should exit immediatly and return 'false',
        // thanks to the 'm_CommCl->Reset' test.
        m_CommCl->ResetStartEvent.notify(); 
        m_CommCl->RequestStartEvent.notify();
        m_CommCl->RequestEndEvent.notify();
        m_CommCl->DataRequestStartEvent.notify();
        m_CommCl->DataRequestEndEvent.notify();
        m_CommCl->ResponseStartEvent.notify();
        m_CommCl->ResponseEndEvent.notify();

    }

    //---------------------------------------------------------------
    //
    // Remove Reset Method
    // Unresets Channel
    //
    //---------------------------------------------------------------

    void remove_reset()
    {
      RequestUpdateReset();
    }

    //---------------------------------------------------------------
    //
    // Get the reset state 
    //
    //---------------------------------------------------------------

    bool get_reset()
    {
      return m_CommCl->Reset;
    }


    //---------------------------------------------------------------
    //
    // SC_METHOD/SC_THREAD methods.
    // Processes implementing a delayed Channel release.
    //
    //---------------------------------------------------------------
    void ReqEnd()   { RequestUpdateFw(2); }
    void ResEnd()   { RequestUpdateBw(2);}
    void DataReqEnd()   { RequestUpdateFwD(2); }

    //---------------------------------------------------------------
    //
    // Register ports and perform static check. Deprecated
    //
    //---------------------------------------------------------------
    virtual void register_port(sc_core::sc_port_base& port, const char* if_typename)
    {
/*          sc_string nm(if_typename); */
/*          if (nm == typeid(TLmasterIF<TdataCl> ).name()) */
/*          { */
/*              // only one master can be connected */
/*              assert(m_Master == 0); */
/*              m_Master = &port; */
/*          } */
/*          else if (nm == typeid(TLslaveIF<TdataCl> ).name()) */
/*          { */
/*              // only one slave can be connected */
/*              assert(m_Slave == 0); */
/*              m_Slave = &port; */
/*          } */
    }

    //---------------------------------------------------------------
    //
    // Externally visible events.
    // Event is triggered at Sput and Mput.
    // It allows making Master/Slave sensitive to the Master/Slave port(s).
    //
    //---------------------------------------------------------------
    const sc_core::sc_event& default_event() const { 
        return (m_CommCl->StartEvent); 
    }

    //---------------------------------------------------------------
    //
    // Data interface access methods.
    // These methods return a pointer to the corresponding class.
    // This gives Master/Slaves access to the data structures.
    // The data structures must be provided by the user.
    //
    //---------------------------------------------------------------
    TdataCl          *GetDataCl()  { return m_DataCl; }
    ParamCl<TdataCl> *GetParamCl() { return m_ParamCl; }

    //---------------------------------------------------------------
    // Returns pointer to communication class, which
    // contains state variables and channel events.
    // These maybe used to create richer transactions by the user
    //---------------------------------------------------------------
    CommCl           *GetCommCl()  { return m_CommCl; }


    //---------------------------------------------------------------
    //
    // Explicit Channel release methods.
    // These methods unlock the Channel.
    // Srelease() unlocks the first request channel.
    // Mrelease() unlocks the first response channel.
    // SreleaseData() unlocks the second request channel (for data handshake)
    //
    //---------------------------------------------------------------
    void Mrelease(sc_core::sc_time time) {
      if (!m_CommCl->Reset)
	m_CommCl->ResponseEndEventN.notify(time);
    }
    void Srelease(sc_core::sc_time time) {
      if (!m_CommCl->Reset)
	m_CommCl->RequestEndEventN.notify(time);
    }
    void Mrelease(void) {
      if (!m_CommCl->Reset)
	RequestUpdateBw(2);
    }
    void Srelease(void) {
      if (!m_CommCl->Reset)
	RequestUpdateFw(2); 
    }
    void SreleaseData(sc_core::sc_time time) {
      if (!m_CommCl->Reset)
	m_CommCl->DataRequestEndEventN.notify(time);
    }
    void SreleaseData(void) {
      if (!m_CommCl->Reset)
	RequestUpdateFwD(2); 
    }

    //---------------------------------------------------------------
    //
    // Preemptive Channel release methods.
    // These methods unlock the channel immediately after the
    // request/response has been posted
    // SreleasePE() unlocks the first request channel.
    // MreleasePE() unlocks the first response channel.
    // SreleaseDataPE() unlocks the second request channel.
    // 
    // Unrelease calls remove the unlock.
    // These are meant to be used in TL1 with
    // masters and slaves, which want to call release before they
    // receive request.
    //---------------------------------------------------------------

    void MreleasePE(void) {
      if (!m_CommCl->Reset) {
	m_CommCl->ResponseRelease=true; 
	m_CommCl->ResponseReleaseEvent.notify(sc_core::SC_ZERO_TIME);
      }
    } 
    void MunreleasePE(void) {
      if (!m_CommCl->Reset) {
	m_CommCl->ResponseRelease=false;
	m_CommCl->ResponseReleaseEvent.notify(sc_core::SC_ZERO_TIME);
      }
    } 
    void SreleaseDataPE(void) {
      if (!m_CommCl->Reset) {
	m_CommCl->DataRelease=true; 
	m_CommCl->DataRequestReleaseEvent.notify(sc_core::SC_ZERO_TIME);
      }
    } 
    void SunreleaseDataPE(void) {
      if (!m_CommCl->Reset) {
	m_CommCl->DataRelease=false; 
	m_CommCl->DataRequestReleaseEvent.notify(sc_core::SC_ZERO_TIME);
      }
    } 
    void SreleasePE(void) {
      if (!m_CommCl->Reset) {
	m_CommCl->RequestRelease=true; 
	m_CommCl->RequestReleaseEvent.notify(sc_core::SC_ZERO_TIME);
      }
    } 
    void SunreleasePE(void) {
      if (!m_CommCl->Reset) {
	m_CommCl->RequestRelease=false; 
	m_CommCl->RequestReleaseEvent.notify(sc_core::SC_ZERO_TIME);
      }
    } 


    //---------------------------------------------------------------
    //
    // Methods to check if the channel is busy.
    //
    //---------------------------------------------------------------
    bool MgetSbusy() const
    {
        // Is the channel occupied with a request?
        // The channel is busy if there is a request in progress, or a request pending
	return (m_CommCl->RequestPending || (!m_CommCl->RequestEarlyEnd && (m_CommCl->RequestStart ||  m_CommCl->BlockingRequestPending)));
      
    }

    bool MgetSbusyData() const
    {
        // Is the channel occupied with a data handshake?
        // The channel is busy if there is a data handshake in progress or pending.
        return ( m_CommCl->DataRequestPending || (!m_CommCl->DataRequestEarlyEnd && (m_CommCl->DataRequestStart ||m_CommCl->BlockingDataRequestPending)));

    }

    bool SgetMbusy() const
    {
        // Is the channel occupied with a response?
        // The channel is busy if there is a response in progress or pending.
        return (m_CommCl->ResponsePending || (!m_CommCl->ResponseEarlyEnd && (m_CommCl->ResponseStart || m_CommCl->BlockingResponsePending))); 
    }

    //---------------------------------------------------------------
    //
    // Method for Slave to check if the request is a read or write
    // request. Note that the user must implement the underlying 
    // m_DataCl->IsWriteRequest() method in the data interface class.
    //
    //---------------------------------------------------------------
    bool IsWrite() { return m_DataCl->IsWriteRequest(); }


    //---------------------------------------------------------------
    //
    // Direct interface methods (for debugging). Deprecated
    //---------------------------------------------------------------

    //---------------------------------------------------------------
    //
    // Methods for the Master to initiate a write or read transfer.
    // There exists non-blocking and blocking versions.
    //
    //---------------------------------------------------------------
    bool MputWriteRequest() 
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      bool tmp;
      if ((tmp = !MgetSbusy())) {
	m_DataCl->SetWriteRequest();
	tmp = MputRequest();
      }
      return tmp;
    }
    bool MputReadRequest() 
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      bool tmp;
      if ((tmp = !MgetSbusy())) {
	m_DataCl->SetReadRequest();
	tmp = MputRequest();
      }
      return tmp;
    }
    bool MputWriteRequestBlocking() 
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      m_DataCl->SetWriteRequest(); 
      return MputRequestBlocking();
    }

    bool MputReadRequestBlocking() 
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      m_DataCl->SetReadRequest(); 
      return MputRequestBlocking();
    } 


    //---------------------------------------------------------------
    //
    // The following 4 methods implement the complete communication scheme.
    // For each of the 4 methods there exist a blocking and non-blocking version.
    //
    // The above Mput methods are wrappers around the MputRequest() methods.
    //
    // It may be useful to call the MputRequest() method directly in order
    // to avoid the calling of SetWriteRequest() or SetReadRequest().
    //
    // MputRequest():  Master method to initiate a request (read or write)
    // SgetRequest():  Slave method to read a request
    // SputResponse(): Slave method to initiate a reponse
    // MgetResponse(): Master method to read the response
    //
    //---------------------------------------------------------------
    // Master method to initiate a request (read or write)
    bool MputRequest()
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      // First, is there already a request in progress or about to start?
      if (MgetSbusy()) {
	// A request is in progress, about to be in progress, or waiting to
	// be about to be in progress. Cannot send request now.
	return false;
      } 
      
      // The channel is free.
      // NOTE: it is left to the user to have checked for thread busy
      
      // Lock the channel from any other new requests
      m_CommCl->RequestPending = true;
      
      // the new request will be put on the channel at the end of this delta-cycle
      RequestUpdateFw(1); // _RequestStartEvent
      
      return(true);
    }

    bool MputRequestBlocking()
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      if (m_CommCl->BlockingRequestPending) 
      {
	// Blocking request already started - abort.
	return false;
      }

      // Mark that there is a Blocking Request in progress (us)
      m_CommCl->BlockingRequestPending = true;


      // Is there a request in progress right now?
      if (m_CommCl->RequestPending || m_CommCl->RequestStart) 
      {
	// There is a request in progress
	// We must wait until it has finished, or reset has happened
	wait(m_CommCl->RequestEndEvent);
	// Was the channel reset?
	if (m_CommCl->Reset) {
	  // In reset. Give up.
	  m_CommCl->BlockingRequestPending = false;
	  return false;
	}
	// At this point the previous command has been accepted.
	// Note that the generic channel has no notion of time,
	// Thus we can issue multiple commands in the same cycle.
      }
      
      // Blocking is over. We can make our request now.
      m_CommCl->BlockingRequestPending = false;
      bool myResult = MputRequest();

      if (!myResult) {
	// Give up - could not start request
	return false;
      }

      // Now that the command has been placed on the channel, wait around until it is accepted as well.
      wait(m_CommCl->RequestEndEvent);

      // Did the channel enter reset while we were waiting?
      if (m_CommCl->Reset) {
	// The channel has been reset. Give up.
	return false;
      }
      
      return myResult;
    }

    //  This method acts as MputRequestBlocking(), but does not wait for
    //  request acknowledgment from the slave. Used by the TL2 API only
    bool MputRequestSemiBlocking()
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      if (m_CommCl->BlockingRequestPending) 
      {
	// Blocking request already started - abort.
	return false;
      }

      // Mark that there is a Blocking Request in progress (us)
      m_CommCl->BlockingRequestPending = true;


      // Is there a request in progress right now?
      if (m_CommCl->RequestPending || m_CommCl->RequestStart) 
      {
	// There is a request in progress
	// We must wait until it has finished, or reset has happened
	wait(m_CommCl->RequestEndEvent);
	// Was the channel reset?
	if (m_CommCl->Reset) {
	  // In reset. Give up.
	  m_CommCl->BlockingRequestPending = false;
	  return false;
	}
	// At this point the previous command has been accepted.
	// Note that the generic channel has no notion of time,
	// Thus we can issue multiple commands in the same cycle.
      }
      
      // Blocking is over. We can make our request now.
      m_CommCl->BlockingRequestPending = false;
      bool myResult = MputRequest();

      return myResult;
    }


    //---------------------------------------------------------------
    //
    // Methods for the Slave to get a Master request.
    // There exist non-blocking and blocking versions.
    //
    //---------------------------------------------------------------
    bool SgetRequest(bool Release)
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      // Is there a current request that can be read?
      if ( !(m_CommCl->RequestStart)) {
	// There is no request to be read
	return false;
      }

      // Is there a blocking get request in progress? 
      // If so, that getRequest command has priority.
      if ( m_CommCl->BlockingGetRequestPending ) {
	return false;
      }
      
      // Are we the first to read this request?
      if ( !(m_CommCl->RequestUnread) ) {
	// This request has been previously read
	// return false to show that there was a problem.
	return false;
      }
      
      // All tests passed. This request belongs to us.
      
      // We have read this request. Reset the helper flag.
      m_CommCl->RequestUnread = false;
      
      // If the user asked for it, accept the request we have just read.
      if (Release || (1-m_ParamCl->cmdaccept) || m_CommCl->RequestRelease || m_CommCl->RequestEarlyEnd) { 
	Srelease();
      }
      
      return true;
    }
    
    bool SgetRequestBlocking(bool Release)
    {
      // SgetRequestBlocking rewritten to use new channel state
      // variables and to reuse SgetRequest function.

      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      // Is there a blocking get request already in progress? 
      // If so, that getRequestBlocking command has priority to read first.
      if ( m_CommCl->BlockingGetRequestPending ) {
	return false;
      }

      // We are the blocking getRequest. Mark it so that no one else can go.
      m_CommCl->BlockingGetRequestPending = true;

      // Is there a request for us to read?
      if ( !(m_CommCl->RequestStart) || !(m_CommCl->RequestUnread) ) {
	// Either there is not request for us (requestStart = false)
	// or someone has already read it (RequestUnread = false)
	// We will have to wait for the next request. Must be patient.
	wait(m_CommCl->RequestStartEvent);
	if (m_CommCl->Reset) {
	  // The channel has been reset. Give up.
	  m_CommCl->BlockingGetRequestPending = false;
	  return false;
	}
      }

      // There should be a request now
      assert( m_CommCl->RequestStart == true );
      assert( m_CommCl->RequestUnread == true );
      
      // Blocking is over - get the request normally
      m_CommCl->BlockingGetRequestPending = false;
      return SgetRequest(Release);
    }

    // Deprecated
    bool SgetRequestPE() 
    {
      return SgetRequest(false);
    }

    //---------------------------------------------------------------
    //
    // Methods for the Slave to answer to a Master request.
    // There exist non-blocking and blocking versions.
    //
    //---------------------------------------------------------------

    // Slave method to initiate a reponse
    bool SputResponse()
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      // First, is there already a response in progress or about to start?
      if (SgetMbusy()) {
	// A response is in progress, about to be in progress, or waiting to
	// be about to be in progress. Cannot send response now.
	return false;
      } 
      
      // The channel is free.
      // NOTE: it is left to the user to have checked for thread busy
      // NOTE: May need to add check here in case of threadbusy_exact
      
      // Lock the channel from any other new responses
      m_CommCl->ResponsePending = true;
      
      // the new response will be put on the channel at the end of this delta-cycle
      RequestUpdateBw(1); // _ResponseStartEvent
	
      return(true);
    }

    bool SputResponseBlocking()
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      if (m_CommCl->BlockingResponsePending) {
	// Blocking response already started - abort.
            return false;
      }

      // Mark that there is a Blocking Response in progress (us)
      m_CommCl->BlockingResponsePending = true;


      // Is there a response in progress right now?
      if (m_CommCl->ResponsePending || m_CommCl->ResponseStart) {
	// There is a response in progress
	// We must wait until it has finished
	//#ifndef NDEBUG
	//            std::cout << name() << ": Master waits put response" << std::endl;
	//#endif
	wait(m_CommCl->ResponseEndEvent);
	if (m_CommCl->Reset) {
	  // Channel was reset while we were waiting. Give up.
	  m_CommCl->BlockingResponsePending = false;
	  return false;
	}
	
      }

      // Blocking is over. We can send our response now.
      m_CommCl->BlockingResponsePending = false;
      bool myResult = SputResponse();
      
      if (myResult == false) {
	// give up.
	return false;
      }

      // Now that the response has been placed on the channel, wait around until it is accepted as well.
      wait(m_CommCl->ResponseEndEvent);
      
      // Did the channel enter reset while we were waiting?
      if (m_CommCl->Reset) {
	// The channel has been reset. Give up.
	return false;
      }

      return myResult;
    }


    //  This method acts as SputResponseBlocking(), but does not wait for
    //  response acknowledgment from the master. Used by the TL2 API only
    bool SputResponseSemiBlocking()
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      if (m_CommCl->BlockingResponsePending) {
	// Blocking response already started - abort.
            return false;
      }

      // Mark that there is a Blocking Response in progress (us)
      m_CommCl->BlockingResponsePending = true;


      // Is there a response in progress right now?
      if (m_CommCl->ResponsePending || m_CommCl->ResponseStart) {
	// There is a response in progress
	// We must wait until it has finished
	//#ifndef NDEBUG
	//            std::cout << name() << ": Master waits put response" << std::endl;
	//#endif
	wait(m_CommCl->ResponseEndEvent);
	if (m_CommCl->Reset) {
	  // Channel was reset while we were waiting. Give up.
	  m_CommCl->BlockingResponsePending = false;
	  return false;
	}
	
      }

      // Blocking is over. We can send our response now.
      m_CommCl->BlockingResponsePending = false;
      bool myResult = SputResponse();
      
      return myResult;
    }

    //---------------------------------------------------------------
    //
    // Methods for the Master to request the response from the Slave.
    // There exist non-blocking and blocking versions.
    //
    //---------------------------------------------------------------
    bool MgetResponse(bool Release)
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      // Is there a current response that can be read?
      if ( !(m_CommCl->ResponseStart)) {
	// There is no response to be read
	return false;
      }
      
      // Is there a blocking get response in progress? 
      // If so, that getResponse command has priority.
      if ( m_CommCl->BlockingGetResponsePending ) {
	return false;
      }
      
      // Are we the first to read this response?
      if ( !(m_CommCl->ResponseUnread) ) {
	// This response has been previously read
	// return false to show that there was a problem.
	return false;
      }
      
      // All tests passed. This response belongs to us.
      
      // We have read this response. Reset the helper flag.
      m_CommCl->ResponseUnread = false;
      
      // If the user asked for it, accept the response we have just read.
      if (Release || (1-m_ParamCl->respaccept)|| m_CommCl->ResponseRelease || m_CommCl->ResponseEarlyEnd){ 
	
	Mrelease();
      }
      
      return true;
    }
    
    bool MgetResponseBlocking(bool Release)
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      // SgetResponeBlocking rewritten to use new channel state
      // variables and to reuse MgetResponse function.
      
      // Is there a blocking get response already in progress? 
      // If so, that getResponseBlocking command has priority to read first.
      if ( m_CommCl->BlockingGetResponsePending ) {
	return false;
      }
      
      // We are the blocking getResponse. Mark it so that no one else can go.
      m_CommCl->BlockingGetResponsePending = true;
      
      // Is there a response for us to read?
      if ( !(m_CommCl->ResponseStart) || !(m_CommCl->ResponseUnread) ) {
	// Either there is not resposne for us (ResponseStart = false)
	// or someone has already read it (ResponseUnread = false)
	// We will have to wait for the next response. Must be patient.
	wait(m_CommCl->ResponseStartEvent);
	if (m_CommCl->Reset) {
	  // The channel has been reset. Give up.
	  m_CommCl->BlockingGetResponsePending = false;
	  return false;
	}
      }
      
      // There should be a response now
      assert( m_CommCl->ResponseStart == true );
      assert( m_CommCl->ResponseUnread == true );
      
      // Blocking is over - get the response normally
      m_CommCl->BlockingGetResponsePending = false;
      return MgetResponse(Release);
    }
    
    // Deprecated
    bool MgetResponsePE()
    {
      return MgetResponse(false);
    }

    //---------------------------------------------------------------
    //
    // Methods for the master to post a request on
    // second channel, used for data handshake
    // There exist non-blocking and blocking versions.
    //
    //---------------------------------------------------------------
    bool MputDataRequest()
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      // First, is there already a data handshake in progress or about to start?
      if (MgetSbusyData()) {
	// A data handshake is in progress, about to be in progress, 
	// or waiting to be about to be in progress. 
	// Cannot send request now.
	return false;
      } 
      
      // The data channel is free.
      // NOTE: it is left to the user to have checked for data thread busy
      
      // Lock the channel from any other new data requests
      m_CommCl->DataRequestPending = true;
      
      RequestUpdateFwD(1); // DataStartEvent
      return(true);
    }

    bool MputDataRequestBlocking()
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      if (m_CommCl->BlockingDataRequestPending)  {
	// Blocking data already started - abort.
	return false;
      }
      
      // Mark that there is a Blocking Request in progress (us)
      m_CommCl->BlockingDataRequestPending = true;
      
      
      // Is there a data request in progress right now?
      if (m_CommCl->DataRequestPending || m_CommCl->DataRequestStart) {
	// There is a data request in progress
	// We must wait until it has finished

	wait(m_CommCl->DataRequestEndEvent);
	// Did the channel enter reset while we were waiting?
	if (m_CommCl->Reset) {
	  // The channel has been reset. Give up.
	  m_CommCl->BlockingDataRequestPending = false;
	  return false;
	}
	// At this point the previous data has been accepted.
      }
      
      // NOTE: no check for thread busy here. This may not be the expected
      //       behavior.
      
      // Blocking is over. We can make our request now.
      m_CommCl->BlockingDataRequestPending = false;
      bool myResult = MputDataRequest();
      
      if (myResult == false) {
	// give up.
	return false;
      }
      
      // Now that the data has been placed on the channel, wait around until it is accepted as well.
      wait(m_CommCl->DataRequestEndEvent);
      if (m_CommCl->Reset) {
	// The channel has been reset. Give up.
	return false;
      }
      
      return myResult;
    }

    //---------------------------------------------------------------
    //
    // Methods for the slave to get a master request,
    // second channel, used for data handshake
    // There exist non-blocking and blocking versions.
    //
    //---------------------------------------------------------------
    bool SgetDataRequest(bool Release)
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      // Is there current data that can be read?
      if ( !(m_CommCl->DataRequestStart)) {
	// There is no data to be read
	return false;
      }
      
      // Is there a blocking get data in progress? 
      // If so, that command has priority.
      if ( m_CommCl->BlockingGetDataRequestPending ) {
	return false;
      }
      
      // Are we the first to read this data?
      if ( !(m_CommCl->DataRequestUnread) ) {
	// This data has been previously read
	// return false to show that there was a problem.
	return false;
      }
      
      // All tests passed. This data is ours - now get it.
      
      // We have read this data. Reset the helper flag.
      m_CommCl->DataRequestUnread = false;
      
      // If the user asked for it, accept the data we have just read.
      if (Release || (1-m_ParamCl->dataaccept)|| m_CommCl->DataRelease || m_CommCl->DataRequestEarlyEnd){         
	SreleaseData();
      }
      
      return true;
    }
    
    bool SgetDataRequestBlocking(bool Release)
    {
      // is the channel in reset?
      if (m_CommCl->Reset) {
	// In reset. Give up.
	return false;
      }
      // SgetDataRequestBlocking rewritten to use new channel state
      // variables and to reuse SgetDataRequest function.
      
      // Is there a blocking get data already in progress? 
      // If so, that getDataRequestBlocking command has priority to read first.
      if ( m_CommCl->BlockingGetDataRequestPending ) {
	return false;
      }
      
      // We are the blocking getDataRequest. Mark it so that no one else can go.
      m_CommCl->BlockingGetDataRequestPending = true;
      
      // Is there already data for us to read?
      if ( !(m_CommCl->DataRequestStart) || !(m_CommCl->DataRequestUnread) ) {
	// Either there is not data for us (DataRequestStart = false)
	// or someone has already read it (DataRequestUnread = false)
	// We will have to wait for the next data. Must be patient.
	wait(m_CommCl->DataRequestStartEvent);
	
	// Did the channel enter reset while we were waiting?
	if (m_CommCl->Reset) {
	  // The channel has been reset. Give up.
	  m_CommCl->BlockingGetDataRequestPending = false;
	  return false;
	}   
      }
      
      // There should be data now
      assert( m_CommCl->DataRequestStart == true );
      assert( m_CommCl->DataRequestUnread == true );
      
      // Blocking is over - get the data normally
      m_CommCl->BlockingGetDataRequestPending = false;
      return SgetDataRequest(Release);
    }

    // Deprecated
    bool SgetDataRequestPE() 
    {
      return SgetDataRequest(false);
    }


    //---------------------------------------------------------------
    // Update method. This method is used to update the states and events
    // which control the communication between Master and Slave.
    // This method is not part of the API. However it is public because
    // it is used by the Synchronizer class.
    // update() use delayed notify 
    //---------------------------------------------------------------

    // ---------------------------------------------------------
    // update()
    // Performs an end of delta cycle update for each of the channel phases
    // Request = m_EventSelectFw
    // Data handshake = m_EventSelectFwD
    // Response = m_EventSelectBw
    // ---------------------------------------------------------
    void update()
    {
#ifdef TRACE_C
        double ttime;
#endif

        switch(m_EventSelectFw)
        {
            case 1:
	      // Start a request to the channel
	        if (m_SyncEvent) m_CommCl->RequestStartEvent.notify(sc_core::SC_ZERO_TIME);
	        m_CommCl->RequestPending = false;
                m_CommCl->RequestStart = true;
                m_CommCl->RequestUnread = true;
                // Make sure that RequestEnd is false - we are starting a new Request phase.
                m_CommCl->RequestEnd = false;
                m_CommCl->RequestAccepted = false;
                if (m_DefaultEvent) m_CommCl->StartEvent.notify(sc_core::SC_ZERO_TIME);

                // now update the data class to put the new request on the channel
                m_DataCl->update_Fw(1);

#ifdef TRACE_C
                ttime = sc_core::sc_time_stamp().to_seconds();
                *outFile<<" "<<m_DataCl->SgetMThreadID()<<" Req start at "<<ttime<<std::endl;
#endif
                break;
            case 2:
	      // Accept the request and release the channel
	      m_CommCl->RequestStart = false;
	      m_CommCl->RequestEnd = true;
	      m_CommCl->RequestAccepted = true;

	      // Call the data channel to make any changes needed to release
	      // the request.
	      m_DataCl->update_Fw(2);
	      // Check for request that was never read
	      if (m_CommCl->RequestUnread) {
#ifndef NDEBUG
		std::cout << name() << ": WARNING - request accepted but never read." << std::endl;
#endif
		m_CommCl->RequestUnread = false;
	      }	
	      if (m_EndTimes)
		m_CommCl->RequestEndTime = sc_core::sc_simulation_time();
	      if (m_SyncEvent && ! m_CommCl->RequestEndSent)
		m_CommCl->RequestEndEvent.notify(sc_core::SC_ZERO_TIME);
#ifdef TRACE_C
	      ttime = sc_core::sc_time_stamp().to_seconds();
	      *outFile<<" "<<m_DataCl->SgetMThreadID()<<" Req end at "<<ttime<<std::endl;
#endif
	      break;

	  case 3: // Request end and start at the same clock edge in TL1

	      // End previous request
	      m_DataCl->update_Fw(2);
	      m_CommCl->RequestEnd = false;
	      m_CommCl->RequestAccepted = true;
	      // Check for request that was never read
	      if (m_CommCl->RequestUnread) {
#ifndef NDEBUG
		std::cout << name() << ": WARNING - request accepted but never read." << std::endl;
#endif
		m_CommCl->RequestUnread = false;
	      }	
	      if (m_EndTimes)
		m_CommCl->RequestEndTime = sc_core::sc_simulation_time();
	      if (m_SyncEvent && ! m_CommCl->RequestEndSent)
		m_CommCl->RequestEndEvent.notify(sc_core::SC_ZERO_TIME);
#ifdef TRACE_C
	      ttime = sc_core::sc_time_stamp().to_seconds();
	      *outFile<<" "<<m_DataCl->SgetMThreadID()<<" Req end at "<<ttime<<std::endl;
#endif
	      // Start new request
	      if (m_SyncEvent) m_CommCl->RequestStartEvent.notify(sc_core::SC_ZERO_TIME);
	      m_CommCl->RequestPending = false;
	      m_CommCl->RequestStart = true;
	      m_CommCl->RequestUnread = true;

	      if (m_DefaultEvent) m_CommCl->StartEvent.notify(sc_core::SC_ZERO_TIME);

	      // now update the data class to put the new request on the channel
	      m_DataCl->update_Fw(1);
	      
#ifdef TRACE_C
	      ttime = sc_core::sc_time_stamp().to_seconds();
	      *outFile<<" "<<m_DataCl->SgetMThreadID()<<" Req start at "<<ttime<<std::endl;
#endif
	      break;
            default: 
                break;
        }
        // updates for the datahandshake phase
        switch(m_EventSelectFwD)
        {
            case 1: 
                // Start data on the channel
  	        if (m_SyncEvent) m_CommCl->DataRequestStartEvent.notify(sc_core::SC_ZERO_TIME);
		m_CommCl->DataRequestPending = false;
                m_CommCl->DataRequestStart = true;
                m_CommCl->DataRequestUnread = true;
                // Reset the DataRequestEnd flag as we are starting a new data phase.
                m_CommCl->DataRequestEnd = false;
                m_CommCl->DataRequestAccepted = false;
                if (m_DefaultEvent) m_CommCl->StartEvent.notify(sc_core::SC_ZERO_TIME);

                // Update the data class to put the new data on the channel
                m_DataCl->update_FwD(1);

#ifdef TRACE_C
                ttime = sc_core::sc_time_stamp().to_seconds();
                *outFile<<" Data Handshake started at "<<ttime<<std::endl;
#endif

                break;

            case 2:
	      // Accept the data and release the data handshake channel
	      m_CommCl->DataRequestStart = false;
	      m_CommCl->DataRequestEnd = true;
	      m_CommCl->DataRequestAccepted = true;
	      // Check for data that was never read
	      if (m_CommCl->DataRequestUnread) {
#ifndef NDEBUG
		std::cout << name() << ": WARNING - data accepted but never read." << std::endl;
#endif
		m_CommCl->DataRequestUnread = false;
	      }	
	      if (m_EndTimes)
		m_CommCl->DataRequestEndTime = sc_core::sc_simulation_time();
	      if (m_SyncEvent && ! m_CommCl->DataRequestEndSent)
		m_CommCl->DataRequestEndEvent.notify(sc_core::SC_ZERO_TIME);
	      
#ifdef TRACE_C
	      ttime = sc_core::sc_time_stamp().to_seconds();
	      *outFile<<" Data Handshake end at "<<ttime<<std::endl;
#endif
	      
	      // Update the data class now that the DataHS has been accepted
	      m_DataCl->update_FwD(2);
	      break;

	    case 3: // Data request and end at the same cycle in TL1
	      // End old data request
	      m_DataCl->update_FwD(2);
	      m_CommCl->DataRequestEnd = false;
	      m_CommCl->DataRequestAccepted = true;
	      // Check for data that was never read
	      if (m_CommCl->DataRequestUnread) {
#ifndef NDEBUG
		std::cout << name() << ": WARNING - data accepted but never read." << std::endl;
#endif
		m_CommCl->DataRequestUnread = false;
	      }	
	      if (m_EndTimes)
		m_CommCl->DataRequestEndTime = sc_core::sc_simulation_time();
	      if (m_SyncEvent && ! m_CommCl->DataRequestEndSent) 
		m_CommCl->DataRequestEndEvent.notify(sc_core::SC_ZERO_TIME);
	      
#ifdef TRACE_C
	      ttime = sc_core::sc_time_stamp().to_seconds();
	      *outFile<<" Data Handshake end at "<<ttime<<std::endl;
#endif

	      // Start data on the channel
	      if (m_SyncEvent) m_CommCl->DataRequestStartEvent.notify(sc_core::SC_ZERO_TIME);
	      m_CommCl->DataRequestPending = false;
	      m_CommCl->DataRequestStart = true;
	      m_CommCl->DataRequestUnread = true;

	      if (m_DefaultEvent) m_CommCl->StartEvent.notify(sc_core::SC_ZERO_TIME);
	      
	      // Update the data class to put the new data on the channel
	      m_DataCl->update_FwD(1);
	      
#ifdef TRACE_C
	      ttime = sc_core::sc_time_stamp().to_seconds();
	      *outFile<<" Data Handshake started at "<<ttime<<std::endl;
#endif

	      break;

            
            default: 
	      break;
        }
        // updates for the Response phase
        switch(m_EventSelectBw)
        {
            case 1:
                // start a response on the channel
                if (m_SyncEvent) {
                    m_CommCl->ResponseStartEvent.notify(sc_core::SC_ZERO_TIME);
                }
                m_CommCl->ResponsePending = false;
                m_CommCl->ResponseStart = true;
                m_CommCl->ResponseUnread = true;
                // Reset the ResponseEnd signal as we are now starting a new Response Phase.
                m_CommCl->ResponseEnd = false;
                m_CommCl->ResponseAccepted = false;
                if (m_DefaultEvent) m_CommCl->StartEvent.notify(sc_core::SC_ZERO_TIME);

                // update the data class to tell it to add the new response to channel.
                m_DataCl->update_Bw(1);

#ifdef TRACE_C
                ttime = sc_core::sc_time_stamp().to_seconds();
                *outFile<<"                                         "
                    <<m_DataCl->MgetSThreadID()<<" Response Start at "<<ttime<<std::endl;
#endif
                break;

            case 2:	      
                // Accept the response and release the channel
		  m_CommCl->ResponseStart = false;
		  m_CommCl->ResponseEnd = true;
		  m_CommCl->ResponseAccepted = true;
		  if (m_EndTimes)
		    m_CommCl->ResponseEndTime = sc_core::sc_simulation_time();
		  // Check for responses that were never read
		  if (m_CommCl->ResponseUnread) {
#ifndef NDEBUG
                    std::cout << name() << ": WARNING - response accepted but never read." << std::endl;
#endif
                    m_CommCl->ResponseUnread = false;
		  }

		  if (m_SyncEvent && ! m_CommCl->ResponseEndSent)
		    m_CommCl->ResponseEndEvent.notify(sc_core::SC_ZERO_TIME);

#ifdef TRACE_C
		  ttime = sc_core::sc_time_stamp().to_seconds();
		  *outFile<<"                                         "
			  <<m_DataCl->MgetSThreadID()<<" Response end at "<<ttime<<std::endl;
#endif

		  // Update the data class now that the reponse has been accepted 
		  m_DataCl->update_Bw(2);
		  break;

            case 3:
		// End previus response cycle
		m_DataCl->update_Bw(2);
                m_CommCl->ResponseEnd = false;
                m_CommCl->ResponseAccepted = true;
		if (m_EndTimes)
		  m_CommCl->ResponseEndTime = sc_core::sc_simulation_time();
		// Check for responses that were never read
		if (m_CommCl->ResponseUnread) {
#ifndef NDEBUG
		  std::cout << name() << ": WARNING - response accepted but never read." << std::endl;
#endif
		  m_CommCl->ResponseUnread = false;
		  }
		
		if (m_SyncEvent && ! m_CommCl->ResponseEndSent)
		  m_CommCl->ResponseEndEvent.notify(sc_core::SC_ZERO_TIME);
		
#ifdef TRACE_C
		ttime = sc_core::sc_time_stamp().to_seconds();
		*outFile<<"                                         "
			<<m_DataCl->MgetSThreadID()<<" Response end at "<<ttime<<std::endl;
#endif

                // start a response on the channel
                if (m_SyncEvent) {
                    m_CommCl->ResponseStartEvent.notify(sc_core::SC_ZERO_TIME);
                }
                m_CommCl->ResponsePending = false;
                m_CommCl->ResponseStart = true;
                m_CommCl->ResponseUnread = true;

                if (m_DefaultEvent) m_CommCl->StartEvent.notify(sc_core::SC_ZERO_TIME);
                // update the data class to tell it to add the new response to channel.
                m_DataCl->update_Bw(1);

#ifdef TRACE_C
                ttime = sc_core::sc_time_stamp().to_seconds();
                *outFile<<"                                         "
                    <<m_DataCl->MgetSThreadID()<<" Response Start at "<<ttime<<std::endl;
#endif
		  break;

            default: 
                break;
        }


        // updates for the Reset special event
        if(m_EventSelectReset) {
            m_CommCl->Reset = false;
	    m_CommCl->ResetEndEvent.notify(sc_core::SC_ZERO_TIME);
	}        
	// Reset dataflow update state and set up any follow-up events
	// (such as automatic accepts) for the next delta cycle.
	m_EventSelectFwD = 0;
	m_EventSelectFw = 0;
	m_EventSelectBw = 0;
	m_EventSelectReset = 0;
	
    }

    //------------------ End of public methods ----------------------


 protected:

    void RequestUpdateFw(int i)
    {
      if (m_EventSelectFw ==0) {
	m_EventSelectFw = i;
      }
      else if ( m_EventSelectFw != i) {
	// Request and release in same delta cycle
	m_EventSelectFw = 3;
      } 
      else {
	// We are asking for the same thing twice.
	if (i==1 ) {
	  // Request twice - this is bad
	  // print an error
#ifndef NDEBUG
	  std::cout << std::endl << name() << ": ERROR: " 
	       << "Request :" 
	       << "Request made before previous released " << std::endl;
#endif
	} 
	else if (i == 2) {
	  // Release twice - odd, but not an error
#ifndef NDEBUG
	  std::cout << std::endl << name() << ": NOTE: " 
	       << "Request Release :" 
	       << "Release called more than once " << std::endl;
#endif
	}
      }
      m_Synchronizer.Update();
    }

    void RequestUpdateFwD(int i)
    {
      if (m_EventSelectFwD ==0) {
	m_EventSelectFwD = i;
      }
      else if ( m_EventSelectFwD != i) {
	// Request and release in same delta cycle
	m_EventSelectFwD = 3;
      } 
      else {
	// We are asking for the same thing twice.
	if (i==1 ) {
	  // Request twice - this is bad
	  // print an error
#ifndef NDEBUG
	  std::cout << std::endl << name() << ": ERROR: " 
	       << "Data Request :" 
	       << "Request made before previous released " << std::endl;
#endif
	} 
	else if (i == 2) {
	  // Release twice - odd, but not an error
#ifndef NDEBUG
	  std::cout << std::endl << name() << ": NOTE: " 
	       << "Data Release :" 
	       << "Release called more than once " << std::endl;
#endif
	}
      }
      m_Synchronizer.Update();

    }


    void RequestUpdateBw(int i)
    {
      if (m_EventSelectBw ==0) {
	m_EventSelectBw = i;
      }
      else if ( m_EventSelectBw != i) {
	// Request and release in same delta cycle
	m_EventSelectBw = 3;
      } 
      else {
	// We are asking for the same thing twice.
	if (i==1 ) {
	  // Respond twice - this is bad
	  // print an error
#ifndef NDEBUG
	  std::cout << std::endl << name() << ": ERROR: " 
	       << "Response :" 
	       << "Response made before previous released " << std::endl;
#endif
	} 
	else if (i == 2) {
	  // Release twice - odd, but not an error
#ifndef NDEBUG
	  std::cout << std::endl << name() << ": NOTE: " 
	       << "Response Release :" 
	       << "Release called more than once " << std::endl;
#endif
	}
      }
      m_Synchronizer.Update();

    }

    void RequestUpdateReset()
    {
      m_EventSelectReset = 1;
      m_Synchronizer.Update();
    }


    // These are called at start of transaction

    void AutoRequestAccept()
    {
      if (! m_ParamCl->cmdaccept || m_CommCl->RequestRelease) {
	m_CommCl->RequestEarlyEnd = true;
	if (m_SyncEvent) {
	  m_CommCl->RequestEndEvent.notify(sc_core::SC_ZERO_TIME);
	  m_CommCl->RequestEndSent = true; // for update() task
	}
      }
      else
	m_CommCl->RequestEarlyEnd = false;
    }

    void AutoDataRequestAccept()
    {
      if (! m_ParamCl->dataaccept || m_CommCl->DataRelease ){
	m_CommCl->DataRequestEarlyEnd = true;
	if (m_SyncEvent) {
	  m_CommCl->DataRequestEndEvent.notify(sc_core::SC_ZERO_TIME);
	  m_CommCl->DataRequestEndSent = true;
	}
      }
      else
	m_CommCl->DataRequestEarlyEnd = false;
    }

    void AutoResponseAccept()
    {
      if (! m_ParamCl->respaccept  || m_CommCl->ResponseRelease){
	m_CommCl->ResponseEarlyEnd = true;
	if (m_SyncEvent){
	  m_CommCl->ResponseEndEvent.notify(sc_core::SC_ZERO_TIME);
	  m_CommCl->ResponseEndSent = true;
	}
      }
      else
	m_CommCl->ResponseEarlyEnd = false;
    }


    // These are called with PE release / unrelease

    void PERequestAccept()
    {
      if (m_CommCl->RequestRelease) {
	m_CommCl->RequestEarlyEnd = true;
	if (m_SyncEvent) {
	  m_CommCl->RequestEndEvent.notify(sc_core::SC_ZERO_TIME);
	  m_CommCl->RequestEndSent = true; // for update() task
	}
      }
      else
	m_CommCl->RequestEarlyEnd = false;
    }

    void PEDataRequestAccept()
    {
      if (m_CommCl->DataRelease ){
	m_CommCl->DataRequestEarlyEnd = true;
	if (m_SyncEvent) {
	  m_CommCl->DataRequestEndEvent.notify(sc_core::SC_ZERO_TIME);
	  m_CommCl->DataRequestEndSent = true;
	}
      }
      else
	m_CommCl->DataRequestEarlyEnd = false;
    }

    void PEResponseAccept()
    {
      if ( m_CommCl->ResponseRelease){
	m_CommCl->ResponseEarlyEnd = true;
	if (m_SyncEvent){
	  m_CommCl->ResponseEndEvent.notify(sc_core::SC_ZERO_TIME);
	  m_CommCl->ResponseEndSent = true;
	}
      }
      else
	m_CommCl->ResponseEarlyEnd = false;
    }

    //---------------- End of Methods --------------------------------------


    //---------------- Protected Data ----------------------------------------

    // master and slave ports to be bounded
    sc_core::sc_port_base* m_Master;
    sc_core::sc_port_base* m_Slave;

    // constructor parameters
    bool m_Synchron;
    bool m_SyncEvent;
    bool m_DefaultEvent;

    // Switches for controlling the update() method
    int m_EventSelectFw;   // Request
    int m_EventSelectFwD;  // Request Data
    int m_EventSelectBw;   // Response
    int m_EventSelectReset;   // Reset

    // Direct interface pointer, deprecated

    // Channel to track request_update synchronization.
    Synchronizer m_Synchronizer;

    // Are transaction end-times recorded for OCP TL1 channel
    bool m_EndTimes;

    // Pointers to the data structure classes provided by the user
    TdataCl          *m_DataCl;
    ParamCl<TdataCl> *m_ParamCl;

    // Pointer to the communication class
    CommCl           *m_CommCl;

#ifdef TRACE_C
    // Output file to trace the communication
    ofstream *outFile;
#endif
}; // end module TL_Channel


#endif
