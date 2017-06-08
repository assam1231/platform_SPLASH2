///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Copyright 2005 OCP-IP
// OCP-IP Confidential & Proprietary
//
//
//============================================================================
//      Project : OCP SLD WG
//       Author : Herve Alexanian, Sonics, Inc.
//          $Id :
//
//  Description :  OCP TL2 SystemC Channel Transaction Monitor
//        For use with the performance OCP TL2 channel.
//        Outputs a view of each transaction moving through the OCP channel.
//        ( matches ocpdis2 -w transfer output )
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _OCP_TL2_TMON_LEGACY_H
#define _OCP_TL2_TMON_LEGACY_H

// -----------------------------------------------------------------
// Utilities for printing and formatting data
// -----------------------------------------------------------------
struct DisplayableBase
{
    virtual ostream& streamIt( ostream& ) const = 0;
    virtual ~DisplayableBase(){}
};
inline ostream& operator<<(ostream& os, const DisplayableBase& d ) {
    return ( d.streamIt( os ) );
}
template<typename T>
struct Displayable: public DisplayableBase
{
    Displayable& operator=( T data ) {
        m_data = data;
        return *this;
    }
    virtual ~Displayable() {}
    virtual ostream& streamIt( ostream& os ) const {
        return( os << m_data );
    }
    T m_data;
};

enum DisplayFormatMode { FORMAT_DEC=1, FORMAT_HEX=2, FORMAT_STR=4 };
struct DisplayableFormat: public DisplayableBase
{
    virtual ~DisplayableFormat(){}
    virtual ostream& streamIt( ostream& os ) const {
        char fill    = ' ';
        if ( m_mode & FORMAT_HEX )
            fill = '0';
        if ( m_mode & FORMAT_DEC )
            os << dec;
        else if ( m_mode & FORMAT_HEX )
            os << hex;
        return ( os << resetiosflags(ios::left) << setiosflags(ios::right)
                 << setfill( fill ) << setw( m_width ) );
    }
    int               m_mode;
    unsigned int      m_width;
};

template <typename objT>
struct DisplayEntryBase
{
    DisplayEntryBase( const char* name, const char* reference,
                      const char* legend ) :
        m_name( name ), m_reference( reference ), m_legend( legend )
        {
            m_format.m_mode = FORMAT_STR;
        }
    virtual const DisplayableBase& display( const objT& ) const = 0;
    virtual const DisplayableBase& format () const {
        return m_format;
    }
    void setFormat( int mode, int width ) {
        m_format.m_mode = mode;
        m_format.m_width = width;
        if ( strlen( m_name ) > m_format.m_width )
            m_format.m_width = strlen( m_name );
    }
    virtual ~DisplayEntryBase() {}

    const   char*     m_name;
    const   char*     m_reference;
    const   char*     m_legend;
    DisplayableFormat m_format;
};

template <typename objT, typename ownerT, typename dataT>
class DisplayEntry: public DisplayEntryBase<objT>
{
  public:
    typedef dataT ( ownerT::*AccessFunc )( const objT& ) const;
    DisplayEntry( const ownerT* pOwner, const char* name, const char* reference,
                  AccessFunc f    , const char* legend ) :
        DisplayEntryBase<objT>( name, reference, legend ),
        m_pOwner( pOwner ),
        m_func( f )
        {}
    virtual const DisplayableBase& display( const objT& o ) const {        
        m_display = ( m_pOwner->*m_func )( o );
        return m_display;
    }
  private:
    const ownerT*              m_pOwner;
    AccessFunc                 m_func;
    mutable Displayable<dataT> m_display;
};

// There is an instance of this in the channel; Blow this away ASAP
template <class Td, class Ta> struct WaitReq {};


template <class Tdata, class Taddr> class OCP_TL2_Channel;

// The Monitor class
template <class Tdata, class Taddr>
class OCP_TL2_TMon
{    
    typedef OCPTL2RequestGrp<Tdata, Taddr> Request;
    typedef OCPTL2ResponseGrp<Tdata >      Response;
 
    // Internal representation of transfers (Request+Response+Timing)
    struct Transfer
    {
        Transfer() {
            *m_burstRepr = *m_respLatRepr = *m_respAccRepr = '\0';
        }
        OCPTL2RequestGrp<Tdata, Taddr> m_request;
        OCPTL2ResponseGrp<Tdata>       m_response;
        // Store the data and byteen. We cannot rely on request and response data
        // pointers as we are buffering transfers to output in correct order
        Tdata                          m_data;
        unsigned int                   m_byteen;
        sc_core::sc_time               m_startTime;
        int                            m_burstCount;
        char                           m_burstRepr[10];
        char                           m_respLatRepr[10];
        char                           m_respAccRepr[10];
        bool                           m_complete;
        bool                           m_needsResponse;
        int                            m_RqDL;
    };

  public:
    OCP_TL2_TMon( ostream *pStream, OCP_TL2_Channel<Tdata,Taddr> *pChannel ) :
        m_os( *pStream ),
        m_channel( *pChannel ),
        m_ocp( m_channel.ocpParams )
        { 
        }

    ~OCP_TL2_TMon()
    {
        bool force=true;
        flushQueue( force );
        typename vector<DisplayEntryBase<Transfer>*>::iterator it;
        for( it = m_entries.begin(); it != m_entries.end(); ++it ) {
            delete *it;
        }
        m_entries.clear();
    }    

    // Called by channel when OCP configuration is known (typically end of elab)
    void tLogSetUp( void )
    {
        m_burstCounter.resize ( m_ocp.threads );
        m_burstTimer.resize   ( m_ocp.threads );
        m_burstSequence.resize( m_ocp.threads, OcpIp::BurstSequence<Taddr>(
                                    m_ocp.data_wdth, m_ocp.addr_wdth, 0, 0 ) );
        m_pendingTransfers.resize( m_ocp.threads );
        initFields();
    }

    void tLogHeader( void )
    {
        m_os << resetiosflags( ios::left ) << setiosflags( ios::right )
             << "##\n"
             << "# Legend for \"transfer\" write style (sorted by request cycle)\n" 
             << "##\n";

        typename vector<DisplayEntryBase<Transfer>*>::iterator it;
        for( it = m_entries.begin(); it != m_entries.end(); ++it ) {
            const DisplayEntryBase<Transfer>& entry = **it;
            if ( strlen( entry.m_name ) == 0 )
                continue;
            m_os << "# " << resetiosflags( ios::right ) << setiosflags( ios::left )
                 << setfill( ' ' ) << setw( 9 ) << entry.m_name
                 << setw( 1 )  <<  ":"
                 << resetiosflags( ios::left ) << setiosflags( ios::right )
                 << setw( 39 ) << entry.m_legend
                 << setw( 1 )  <<  " : "
                 << resetiosflags( ios::right ) << setiosflags( ios::left )
                 << setw( 28 ) << entry.m_reference << setw( 1 ) << '\n';
        }
        m_os << '\n';
        for( it = m_entries.begin(); it != m_entries.end(); ++it ) {
            const DisplayEntryBase<Transfer>& entry = **it;
            m_os << entry.format() << setfill( ' ' ) << entry.m_name << " ";
        }
        m_os << '\n';
        for( it = m_entries.begin(); it != m_entries.end(); ++it ) {
            const DisplayEntryBase<Transfer>& entry = **it;
            m_os << entry.format() << setfill( '-' ) << "" << setw( 1 ) << " ";
        }
        m_os << '\n';
    }
        
    // Called by channel when a request arrives
    void tLogNewReq(void)
    {
        Request request = m_channel.m_currentRequest;
        int thread = request.MThreadID;
        bool isRead = OcpIp::isRead( request.MCmd );
        sc_core::sc_time now = sc_core::sc_time_stamp();
        sc_core::sc_time reqInterval( 0, sc_core::SC_NS );
        int interval = isRead ? m_channel.getRqI() : m_channel.getWDI();
        reqInterval = interval * m_channel.getPeriod();
        bool needsResponse = isRead || m_ocp.writeresp_enable;

        // Expand TL2 request into individual transfers
        vector<Transfer> transfers( request.DataLength );
        for ( unsigned int i=0; i < request.DataLength; ++i ) {
            Transfer& transfer            = transfers[i];
            transfer.m_RqDL = m_channel.m_mTiming.RqDL;
            transfer.m_request            = request;
            transfer.m_request.DataLength = 1;
            bool burstLengthSure = 
                ( request.MBurstPrecise || ( i == request.DataLength - 1 ) );
            if ( !burstLengthSure )
                transfer.m_request.MBurstLength = 0xbad1dea; // not displayed
            else if ( !request.MBurstPrecise )
                transfer.m_request.MBurstLength = 1;
            // deadbeef will stay for reads that don't complete response
            transfer.m_data   = 0xdeadbeef;
            transfer.m_byteen = request.MByteEn;
            if ( !isRead && request.MDataPtr != NULL )
                transfer.m_data = request.MDataPtr[i];
            if ( request.MByteEnPtr != NULL )
                transfer.m_byteen = request.MByteEnPtr[i];
            OcpIp::trimWidth( transfer.m_byteen, int( m_ocp.data_wdth / 8 ) );
            transfer.m_needsResponse      =  needsResponse;
            m_burstCounter[thread].next( transfer.m_request );
            // Keep track of when burst is started. This will be the start time
            // of all transfers within a SRMD burst
            if ( m_burstCounter[thread].first() ) {
                m_burstTimer[thread] = now;
            } else {
                // only the first transfer of a SRMD write burst has a response
                if (needsResponse && !isRead && request.MBurstSingleReq) {
                    transfer.m_needsResponse = false;
                }
            }
            if ( request.MBurstSingleReq && m_ocp.burstsinglereq) {
                transfer.m_startTime      = m_burstTimer[thread];
                if ( !isRead ) {
                    transfer.m_RqDL = (int) (now.to_default_time_units() -
                        transfer.m_startTime.to_default_time_units());
                    transfer.m_RqDL += i * interval;
                }
            } else {
                transfer.m_startTime      = now + ( i * reqInterval );
            }
            transfer.m_burstCount         = m_burstCounter[thread].count();
            assert( transfer.m_burstCount > 0 );
            // Build burst string (count/length)
            if ( !burstLengthSure )
                sprintf( transfer.m_burstRepr, "%x/.", transfer.m_burstCount );
            else
                sprintf( transfer.m_burstRepr, "%x/%x", transfer.m_burstCount,
                         transfer.m_request.MBurstLength );

            // Compute address according to burst sequence
            if ( transfer.m_burstCount == 1 ) {
                m_burstSequence[thread].init(
                    request.MAddr, request.MBurstSeq,
                    request.MBurstLength, request.MBurstPrecise,
                    request.MBlockHeight, request.MBlockStride );
            }
            if ( request.DataLength > 1 ) {
                transfer.m_request.MAddr = m_burstSequence[thread].next();
            } else if ( request.MBurstSeq != OCP_MBURSTSEQ_UNKN ) {
                m_burstSequence[thread].next(); // advance sequence but ignore
            }

            //
            transfer.m_complete = !isRead &&
                (!m_ocp.writeresp_enable ||
                 (request.MBurstSingleReq && (transfer.m_burstCount > 1)));
            //transfer.m_complete = !needsResponse;

            // Queue it by time, and if incomplete in pending transfer queue
            m_timedTransfers.push_back( transfer );
            if ( !transfer.m_complete ) {
                m_pendingTransfers[thread].push_back(
                    &( m_timedTransfers.back() ) );
            }
        }
        flushQueue();
    }

    // Called by channel when a response arrives
    void tLogNewResp( void )
    {
        // A response has come in. 
        Response response   = m_channel.m_currentResponse;
        unsigned int thread = response.SThreadID;
        sc_core::sc_time now= sc_core::sc_time_stamp();
        int respInterval    = m_channel.getRDI();

        // Expand TL2 response to fill pending transfers
        for ( unsigned int i=0; i < response.DataLength; ++i ) {
            assert( !m_pendingTransfers[thread].empty() );
            Transfer& transfer = *( m_pendingTransfers[thread].front() );
            assert( !transfer.m_complete );
            transfer.m_response             = response;
            transfer.m_response.DataLength  = 1;
            if ( OcpIp::isRead( transfer.m_request.MCmd ) ) {
                assert ( response.SDataPtr != NULL );
                transfer.m_data = response.SDataPtr[i];
            }
            transfer.m_complete             = true;
            sc_core::sc_time delay = now - transfer.m_startTime;
            sprintf( transfer.m_respLatRepr, "%d",
                     int( delay/m_channel.getPeriod() ) + i * respInterval );
            sprintf( transfer.m_respAccRepr, "%d", m_channel.m_mTiming.RpAL );
            m_pendingTransfers[thread].pop_front();
        }
        flushQueue();
    }

    // Access functions to the transfer information
  private:
    inline double      _StartTime( const Transfer& t ) const
        { return t.m_startTime.to_default_time_units(); }
    inline unsigned long long _StartCycle( const Transfer& t ) const
        { return static_cast<unsigned long long>(
            t.m_startTime / m_channel.getPeriod() ); }
    inline unsigned int _MThreadID( const Transfer& t ) const
        { return t.m_request.MThreadID; }
    inline unsigned int _MConnID( const Transfer& t ) const
        { return t.m_request.MConnID; }
    inline const char* _MCmd( const Transfer& t ) const
        { return OcpIp::Literals::MCmd( t.m_request.MCmd  ); }
    inline const char* _MBurst( const Transfer& t ) const
        { return t.m_burstRepr; }
    inline const char* _MBurstSeq( const Transfer& t ) const
        { return OcpIp::Literals::MBurstSeq( t.m_request.MBurstSeq ); }
    inline char _LeftParen( const Transfer& t ) const
        { return ( t.m_request.MBurstSingleReq && t.m_burstCount > 1 ) ? '(' : ' '; }
    inline char _RightParen( const Transfer& t ) const
        { return ( t.m_request.MBurstSingleReq && t.m_burstCount > 1 ) ? ')' : ' '; }
    inline unsigned int _MAddrSpace( const Transfer& t ) const
        { return t.m_request.MAddrSpace; }
    inline Taddr _MAddr( const Transfer& t ) const
        { return t.m_request.MAddr; }
    inline unsigned int _MByteEn( const Transfer& t ) const
        { return t.m_byteen; }
    inline unsigned long long _MReqInfo( const Transfer& t ) const
        { return t.m_request.MReqInfo; }
    inline unsigned int _RqDL( const Transfer& t ) const
        { return t.m_RqDL; }
    inline unsigned int _RqAL( const Transfer& t ) const
        { return m_channel.m_sTiming.RqAL; }
    inline unsigned int _DAL( const Transfer& t ) const
        { return m_channel.m_sTiming.DAL; }
    inline Tdata _Data( const Transfer& t ) const {
        return t.m_data;
    }   
    inline const char* _SResp( const Transfer& t ) const
        { return ( t.m_needsResponse ?
                   OcpIp::Literals::SResp( t.m_response.SResp ) : "." ); }
    inline unsigned long long _SRespInfo( const Transfer& t ) const
        { return t.m_response.SRespInfo; }
    inline const char* _RqRpL( const Transfer& t ) const
        { return t.m_needsResponse ? t.m_respLatRepr : "."; }
    inline const char* _RpAL( const Transfer& t ) const
        { return t.m_needsResponse ? t.m_respAccRepr : "."; }
    inline unsigned long long _MFlag( const Transfer& t ) const
        { return m_channel.m_sideband.MFlag; }
    inline unsigned long long _SFlag( const Transfer& t ) const
        { return m_channel.m_sideband.SFlag; }
    inline bool _SInterrupt( const Transfer& t ) const
        { return m_channel.m_sideband.SInterrupt; }

    // The output has the following format:
    // Field(full name) / display width / base / [condition (if any)]
    // SimTime / 9 / dec
    // Cycle / 7 / dec
    // ( (left parenthesis) / 1 / char / burstsinglereq == true
    // T(Thread) / (ocpParams.threads-1)/10 +1 / dec / ocpParams.threads > 1
    // C(ConnID) / (connid_wdth-1)/4 +1 / hex / connid == true
    // Cmd (MCmd) / 4 / char
    // BuS (MBurstSeq) / 4 / char 
    // Burst (transfer count/MBurstLength) / 
    //          5 (tcount=2 right, "/"=1, dLen=2 left) / dec
    // AS (MAddressSpace) / (addrspace_wdth-1)/4 +1 / hex / addrspace
    // Addr (MAddr) / (addr_wdth-1)/4 +1 / hex
    // ) (right parenthesis) / 1 / char / burstsinglereq == true
    // BE (byteen) / (data_wdth-1)/32 +1 / hex / byteen
    // Ri(MReqInfo) / (reqinfo_wdth-1)/4 +1 / hex / reqinfo
    // RqDL (Request Data Latency) / 4 / dec
    // RqAL (Request Accept Latency) / 4 / dec
    // DAL (Data Accept Latency) / 3 / dec
    // Data (MData or SData) / (data_wdth-1)/4 +1 
    // ( (left parenthesis) / 1 / char / burstsinglereq == true
    // Resp(SResp) / 4 / char
    // RI (SRespInfo) / (respinfo_wdth-1)/4 +1 / hex / respinfo
    // ) (right parenthesis) / 1 / char / burstsinglereq == true
    // RqRpL (Request Response Latency / 5 / dec
    // RpAL (Response Accept Latency) / 4 / dec
    // MF (MFlag) / (mflag_wdth-1)/4 +1 / hex / mflag
    // SF (SFlag) / (sflag_wdth-1)/4 +1 / hex / sflag
    // I (SInterrupt) / 1 / dec / interrupt

    void initFields() {
// This macro allows to condense this verbose text quite a bit
#define ADDENTRY( type, access, name, reference, format, width, legend ) \
        { pEntry = new DisplayEntry<Transfer, OCP_TL2_TMon, type>(       \
            this, name, reference, &OCP_TL2_TMon:: access, legend ); \
            pEntry->setFormat( format, width );                         \
            m_entries.push_back( pEntry );                              \
        }            
        DisplayEntryBase<Transfer>* pEntry;
        ADDENTRY( double, _StartTime, "SimTime", "Time", FORMAT_DEC, 9,
                  "Simulation Time" );
        ADDENTRY( unsigned long long, _StartCycle, "Cycle", "Cycle", FORMAT_DEC, 7,
                  "Cycle Valid Time" );
        if ( m_ocp.burstsinglereq)
            ADDENTRY( char, _LeftParen, "", "", FORMAT_STR, 1, "");
        if ( m_ocp.threads > 1)
            ADDENTRY( unsigned int, _MThreadID, "T", "ThreadID", FORMAT_DEC,
                      int( ( ( m_ocp.threads - 1 ) / 10 ) + 1 ),
                      "Master and Slave Thread ID" );
        if ( m_ocp.connid )
            ADDENTRY( unsigned int, _MConnID, "C", "MConnID", FORMAT_DEC,
                      int( ( ( m_ocp.connid_wdth - 1 ) / 4) + 1 ),
                      "Connection Identifier" );
        ADDENTRY( const char*, _MCmd, "Cmd", "MCmd", FORMAT_STR, 4,
                  "Master Command" );
        if ( m_ocp.burstseq )
            ADDENTRY( const char*, _MBurstSeq, "BuS", "MBurstSeq", FORMAT_STR, 4,
                                   "Burst Sequence Type" );
        if ( m_ocp.burstlength )
            ADDENTRY( const char*, _MBurst, "Burst", "transfer count/MBurstLength",
                      FORMAT_STR, 4, "req burst transfer count/burst length" );
        if ( m_ocp.addrspace )
            ADDENTRY( unsigned int, _MAddrSpace, "AS", "MAddrSpace",
                      FORMAT_DEC, int( ( ( m_ocp.addrspace_wdth - 1 ) / 4 ) + 1 ),
                      "Master Address Space" );
        if ( m_ocp.addr )
            ADDENTRY( Taddr, _MAddr, "Addr", "MAddr",
                      FORMAT_HEX, int( ( ( m_ocp.addr_wdth - 1 ) / 4 ) + 1 ),
                      "Master Address" );
        if ( m_ocp.burstsinglereq)
            ADDENTRY( char, _RightParen, "", "", FORMAT_STR, 1, "" );
        if ( m_ocp.byteen || m_ocp.mdatabyteen )
            ADDENTRY( unsigned int, _MByteEn, "BE", "MByteEn",
                      FORMAT_HEX, int( ( ( m_ocp.data_wdth - 1 ) / 32 ) + 1 ),
                      "Byte Enables" );
        if ( m_ocp.reqinfo )
            ADDENTRY( unsigned long long, _MReqInfo, "Ri", "MReqInfo",
                      FORMAT_HEX, int( ( ( m_ocp.reqinfo_wdth - 1 ) / 4 ) + 1 ),
                      "Request Info" );
        if ( m_ocp.datahandshake )
            ADDENTRY( unsigned int, _RqDL, "RqDL", "RqDL", FORMAT_DEC, 4,
                      "Request Data Latency" );
        ADDENTRY( unsigned int, _RqAL, "RqAL", "RqAL", FORMAT_DEC, 4,
                  "Request Accept Latency" );
        if ( m_ocp.datahandshake )
            ADDENTRY( unsigned int, _DAL, "DAL", "DAL", FORMAT_DEC, 3,
                      "Data Accept Latency" );
        if ( m_ocp.mdata || m_ocp.sdata )
            ADDENTRY( Tdata, _Data, "Data", "Data",
                      FORMAT_HEX, int( ( ( m_ocp.data_wdth - 1 ) / 4 ) + 1 ),
                      "MData (if write) or SData (if read)" );
        if ( m_ocp.resp )
            ADDENTRY( const char*, _SResp, "Resp", "SResp", FORMAT_STR, 4,
                      "Slave Response" );
        if ( m_ocp.respinfo )
            ADDENTRY( unsigned long long, _SRespInfo, "RI", "SRespInfo",
                      FORMAT_HEX, int( ( ( m_ocp.respinfo_wdth - 1 ) / 4 ) + 1 ),
                      "Response Info" );
        if ( m_ocp.resp ) {
            ADDENTRY( const char*, _RqRpL, "RqRpL", "RqRpL", FORMAT_DEC, 5,
                      "Request Response Latency" );
            ADDENTRY( const char*, _RpAL, "RpAL", "RpAL", FORMAT_DEC, 4,
                      "Response Accept Latency" );
        }
        if ( m_ocp.mflag )
            ADDENTRY( unsigned long long, _MFlag, "MF", "MFlag",
                      FORMAT_HEX, int( ( ( m_ocp.mflag_wdth - 1 ) / 4 ) + 1 ),
                      "Master Flags" );
        if ( m_ocp.sflag )
            ADDENTRY( unsigned long long, _SFlag, "SF", "SFlag",
                      FORMAT_HEX, int( ( ( m_ocp.sflag_wdth - 1 ) / 4 ) + 1 ),
                      "Slave Flags" );
        if ( m_ocp.interrupt )
            ADDENTRY( bool, _SInterrupt, "I", "SInterrrupt", FORMAT_DEC, 1,
                      "Slave Interrupt" );
#undef ADDENTRY
    }

    void printTransfer( const Transfer& t ) {
        for( typename vector<DisplayEntryBase<Transfer>*>::iterator it =
                 m_entries.begin(); it != m_entries.end(); ++it ) {
            const DisplayEntryBase<Transfer>& entry = **it;
            m_os << entry.format()
                         << entry.display( t ) << " ";
        }
        m_os << '\n';
        m_os << std::flush;
    }

    void flushQueue( bool force = false ) {
        while ( !m_timedTransfers.empty() ) {
            const Transfer& transfer = m_timedTransfers.front();
            if ( !transfer.m_complete && !force ) break;
            printTransfer( transfer );
            m_timedTransfers.pop_front();
        }
    }

    ostream&                             m_os;
    const OCP_TL2_Channel<Tdata, Taddr>& m_channel;
    const OCPParameters&                 m_ocp;
    vector<DisplayEntryBase<Transfer>* > m_entries;
    deque<Transfer>                      m_timedTransfers;
    vector<deque<Transfer*> >            m_pendingTransfers;
    vector<OcpIp::BurstCounter>          m_burstCounter;
    vector<sc_core::sc_time>             m_burstTimer;
    vector<OcpIp::BurstSequence<Taddr> > m_burstSequence;
};

#endif
