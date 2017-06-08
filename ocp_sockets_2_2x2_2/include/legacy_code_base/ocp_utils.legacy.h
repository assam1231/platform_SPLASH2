//////////////////////////////////////////////////////////
//
//  (c) Copyright OCP-IP 2005
//  OCP-IP Confidential and Proprietary
//  $Id:
//
//  OCP protocol utility functions and classes
//
// Herve Alexanian, Sonics Inc.
//
//////////////////////////////////////////////////////////
#ifndef __OCP_UTILS_LEGACY_H__
#define __OCP_UTILS_LEGACY_H__

namespace OcpIp {


namespace Literals {
    template <typename T> const char* MCmd     ( const T& );
    template <typename T> const char* SResp    ( const T& );
    template <typename T> const char* MBurstSeq( const T& );

template <>
inline const char* MCmd<int>( const int& cmd ) {
    static const char* _cmds[8] = { "IDLE", "WR"  , "RD"  , "RDEX",
                                    "RDL" , "WRNP", "WRC" , "BCST" };
    return _cmds[cmd];
}
template<typename T> inline const char* MCmd( const T& cmd ) {
    return MCmd( static_cast<int>( cmd ) );
}

template <>
inline const char* MBurstSeq<int>( const int& seq ) {
    static const char* _seqs[8] = { "INCR", "DFT1", "WRAP", "DFT2",
                                    "XOR" , "STRM", "UNKN", "BLCK" };
    return _seqs[seq];
}
template<typename T> inline const char* MBurstSeq( const T& seq ) {
    return MBurstSeq( static_cast<int>( seq ) );
}

template <>
inline const char* SResp<int>( const int& resp ) {
    static const char* _resps[4] = { "NULL", "DVA", "FAIL", "ERR" };
    return _resps[resp];
}
template<typename T> inline const char* SResp( const T& resp ) {
    return SResp( static_cast<int>( resp ) );
}
}

// Basic Utility functions
inline bool isRead( const OCPMCmdType& cmd )
{
    return ( ( cmd == OCP_MCMD_RD   ) ||
             ( cmd == OCP_MCMD_RDEX ) ||
             ( cmd == OCP_MCMD_RDL  ) );
}

inline bool isWrite( const OCPMCmdType& cmd )
{
    return ( ( cmd == OCP_MCMD_WR   ) ||
             ( cmd == OCP_MCMD_WRNP ) ||
             ( cmd == OCP_MCMD_WRC  ) ||
             ( cmd == OCP_MCMD_BCST ) );
}

inline unsigned int 
ceilLog2( unsigned int x )
{
    int bits = 0;
    while ( x > ( 1U << bits ) ) {
        ++bits;
        if ( bits == 32 )
            break;
    }    
    return bits;
}

inline int
getByteenWidth( int dataWidth )
{
    if ( dataWidth <= 8 ) return 1;
    return ( 1 << ( ceilLog2( dataWidth ) - 3 ) );
}

template < typename T>
inline void
trimWidth( T& num, int width )
{
    if ( (unsigned int)( width ) >= sizeof( T ) * 8 ) return;
    num &= ( ( 1ULL << width ) - 1 );
}

template<typename numT>
void
fillBitRange( numT& num, int msb, int lsb = 0, bool value = true )
{
    int max = sizeof( numT ) * 8;
    assert( msb == -1 || msb < (int)( sizeof( numT ) * 8 ) );
    assert( msb == -1 || lsb <= msb );

    // special case if setting a value initially at 0
    if ( num == 0 && value ) {
        numT allZero = 0;
        num = ~ allZero ;
        num = ( num >> lsb ) << lsb;
        if ( msb >= 0 ) {
            num = ( num << ( max - 1 - msb ) );
            num = num >> ( max - 1 - msb );
        }
    } else {
        numT bitsSet(0);
        // this call will fall into the previous if block
        fillBitRange( bitsSet, msb, lsb, true );
        num &= ~bitsSet;
        if ( value ) {
            num |= bitsSet;
        }
    }
}

//////////////////////////////////////////////////////////////////
// CLASS: BurstCounter
// DESCRIPTION: class used to track the burst count on a thread
// Templated on request type (must have MBurstLength, MBurstPrecise
// MThreadID fields)
// USAGE: After constructor, call next() for every new request
// on a thread (mixing threads causes an assertion)
// Then use queries first(), last() and count()
//////////////////////////////////////////////////////////////////
class BurstCounter
{
  public:
    BurstCounter() :
    m_burstOpen( false ), m_threadId( 0 ), m_count( 0 ) {}

    template<typename reqT> void next( const reqT& );
    inline bool          first() const { return m_count == 1; }
    inline bool          last()  const { return ( m_count > 0 && !m_burstOpen ); }
    inline unsigned int  count() const { return m_count; }
  private:
    template<typename reqT> int getLength( const reqT& ) const;
    template<typename reqT> bool getBurstEnd( const reqT& ) const;
    // would like to do this as a specialization but don't know how. Using
    // overloading instead
    template<typename Td, typename Ta>
        int getLength( const OCPTL2RequestGrp<Td, Ta>& ) const;
    template<typename Td, typename Ta>
        bool getBurstEnd( const OCPTL2RequestGrp<Td, Ta>& ) const;
    bool         m_burstOpen;
    unsigned int m_threadId;
    unsigned int m_count;
};

//////////////////////////////////////////////////////////////////
// CLASS: BurstSequence
// DESCRIPTION: class used to calculate an ocp burst sequence.
// ARGUMENTS: startingAddress, address sequence, optional arguments
// for length and precise flag
// USAGE: After constructor, two ways to get the sequence.
// using a series of next() calls or using calculate(). next()
//////////////////////////////////////////////////////////////////
template<typename T>
class BurstSequence
{
  public:

    typedef typename std::deque<T>::const_iterator iterator;

    BurstSequence( int dataWidth, int addressWidth, T startingAddress,
                   int sequence, int length = -1, bool precise = true,
                   int height = 1, int stride = -1 );

    iterator begin() const { return m_address.begin(); }
    iterator end()   const { return m_address.end();   }    
    T        next();
    T        get_low_addr();
    
    
    // Re-set to calculate a new burst
    void     init( T startingAddress, int sequence, int length = -1,
                   bool precise = true, int height = 1, int stride = -1 );

    // length is required when the burst is imprecise.
    // this function calls next() for the length of the burst.
    // since next() can only be called burstlength times for a precise burst
    // calling next() after calculate() for a precise burst will fail.
    void
    calculate( int length = -1 )
    {
        if ( length < 0 ) length = m_length * m_height;
        assert( length > 0 );
        for ( int i = 0; i < length; ++i ) next();
    }

  private:

    int                         m_sequence;
    int                         m_length;
    bool                        m_precise;
    int                         m_height;
    int                         m_stride;
    std::deque<T>               m_address;
    typename std::deque<T>::iterator m_current;

    // internal variables for sequence calculation
    int                         m_addrWidth;
    int                         m_byteenWidth;
    int                         m_wrapBits;
    T                           m_lowWrapAddress;
    T                           m_firstAddressOffset;
};


// Implementation
template<typename reqT>
void
BurstCounter::next ( const reqT& request )
{
    // check that the thread is the same one we're tracking
    assert( !m_burstOpen || request.MThreadID == m_threadId );
    m_threadId = request.MThreadID;
    if ( !m_burstOpen )
        m_count = 0;
    m_count += getLength( request );
    m_burstOpen = !getBurstEnd( request );
}

template<typename reqT>
int
BurstCounter::getLength( const reqT& request ) const {
    return 1;
}

template<typename Td, typename Ta>
int
BurstCounter::getLength( const OCPTL2RequestGrp<Td, Ta> & request ) const {
    return request.DataLength;
}

template<typename reqT>
bool
BurstCounter::getBurstEnd( const reqT& request ) const {
    if ( request.MBurstPrecise ){
        unsigned int blockHeight = request.MBlockHeight;
        if ( blockHeight == 0 )
            blockHeight = 1;
        return ( m_count == blockHeight * request.MBurstLength );
    }
    else
        return ( request.MBurstLength == 1 );
}

template<typename Td, typename Ta>
bool
BurstCounter::getBurstEnd( const OCPTL2RequestGrp<Td, Ta> & request ) const {
    return request.LastOfBurst;
}


template<typename T>
BurstSequence<T>::BurstSequence( int dataWidth, int addressWidth,
                                 T startingAddress, int sequence,
                                 int length, bool precise,
                                 int height, int stride )
  : m_addrWidth( addressWidth ),
    m_byteenWidth( getByteenWidth( dataWidth ) ),
    m_lowWrapAddress( startingAddress ),
    m_firstAddressOffset( startingAddress )
{
    init( startingAddress, sequence, length, precise, height, stride );
    assert( m_sequence != OCP_MBURSTSEQ_UNKN );
}

template<typename T>
void
BurstSequence<T>::init( T startingAddress, int sequence, int length,
                        bool precise, int height, int stride )
{
    m_sequence = sequence;
    m_precise  = precise;
    m_length   = precise? length : -1;
    m_height   = height;
    m_stride   = stride;//stride > 0 ? stride : m_length * m_byteenWidth;
    m_wrapBits = precise? ceilLog2( length * m_byteenWidth ): 0;
    m_lowWrapAddress = m_firstAddressOffset = startingAddress;
    m_address.clear();
    m_address.push_back( startingAddress );
    m_current = m_address.end();
    
    
    if ( precise && m_wrapBits > 0 ) {
        fillBitRange( m_lowWrapAddress, m_wrapBits-1, 0, 0 );
    }
    if ( m_wrapBits < m_addrWidth ) {
        fillBitRange( m_firstAddressOffset, m_addrWidth - 1, m_wrapBits, 0 );
    }
    assert( ( sequence == OCP_MBURSTSEQ_BLCK || m_height < 2 ) &&
            "Height >1 is only valid for BLCK bursts" );
}


template<typename T>
T
BurstSequence<T>::get_low_addr()
{
  return m_lowWrapAddress;
}

template<typename T>
T
BurstSequence<T>::next()
{
    if ( m_current == m_address.end() ) {
        // this is the first pass
        m_current = m_address.begin();
    } else {
        int position = m_current - m_address.begin() + 1;
        assert( !m_precise ||
                position < ( m_length * ( m_height ? m_height : 1 ) ) );

        T   newAddress( *m_current );

        if ( m_sequence == OCP_MBURSTSEQ_INCR ||
             m_sequence == OCP_MBURSTSEQ_DFLT1 ||
             m_sequence == OCP_MBURSTSEQ_DFLT2 ) {
            newAddress += m_byteenWidth;
        } else if ( m_sequence == OCP_MBURSTSEQ_STRM) {
            // keep same address
        } else if ( m_sequence == OCP_MBURSTSEQ_WRAP) {
            T increment = newAddress;
            increment  = position * m_byteenWidth;
            increment += m_firstAddressOffset;
            if ( m_wrapBits <= m_addrWidth ) {
                fillBitRange( increment, -1, m_wrapBits, 0 );
            }
            newAddress = m_lowWrapAddress | increment;
        } else if ( m_sequence == OCP_MBURSTSEQ_XOR ) {
            T increment = newAddress;
            increment   = position << ceilLog2( m_byteenWidth );
            increment  ^= m_firstAddressOffset;
            newAddress  = m_lowWrapAddress | increment;
        } else if ( m_sequence == OCP_MBURSTSEQ_BLCK ) {
            newAddress += m_byteenWidth;
            if ( position % m_length == 0 ) {
                // new row
                int row = position / m_length;
                newAddress = *(m_address.begin()) + m_stride * row;
            }
        } else {
            assert( false );
        }
        m_current = m_address.insert( m_address.end(), newAddress );
    }
    return *m_current;
}
}

#endif
