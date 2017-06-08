// 
//  Copyright 2003 OCP-IP
//
// ============================================================================
//      Project : OCP SLD WG, OCP Transaction Level 
//       Author : Alan Kamas, for Sonics, Inc.
//         $Id: multiport_event_finder.h,v 1.3 2005/11/25 17:27:15 jaldis Exp $
//
//  Description : Extends the Event Finder from SystemC to find events on a 
//                particular port of a multi-port channel.
//                Note that since the SystemC sc_core::sc_event_finder_t class contains
//                private members that this class would need access to,
//                this class copies and extends the class rather than
//                inheriting from it.
//
// ============================================================================

#ifndef __OCP_MULTIPORT_EVENT_FINDER_LEGACY_H__
#define __OCP_MULTIPORT_EVENT_FINDER_LEGACY_H__

template <class IF>
class multiport_event_finder_t
: public sc_core::sc_event_finder
{
public:

    // constructor
    multiport_event_finder_t( const sc_core::sc_port<IF,0>& port_,
            const sc_core::sc_event& (IF::*event_method_) () const,
            int portNum_)
        : sc_core::sc_event_finder( port_ ), m_sc_port(port_), m_event_method( event_method_ ), m_portNum(portNum_)
    {
        // do nothing
    }

    // destructor (does nothing)
    virtual ~multiport_event_finder_t()
    {
        // do nothing 
    }

    virtual const sc_core::sc_event& find_event() const;
    virtual const sc_core::sc_event& find_event(sc_core::sc_interface *if_p = 0) const;

protected:
    const sc_core::sc_port<IF,0>& m_sc_port;
    const sc_core::sc_event& (IF::*m_event_method) () const;
    int m_portNum;

};


// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII

template <class IF>
inline
const sc_core::sc_event&
multiport_event_finder_t<IF>::find_event() const
{
    const IF* iface = DCAST<const IF*>( m_sc_port[m_portNum] );
    if( iface == 0 ) {
        report_error( sc_core::SC_ID_FIND_EVENT_, "passed port number is not bound" );
    }
    return (CCAST<IF*>( iface )->*m_event_method) ();
}

template <class IF>
inline
const sc_core::sc_event&
multiport_event_finder_t<IF>::find_event( sc_core::sc_interface* if_p ) const
{
    const IF* iface = ( if_p ) ? DCAST<const IF*>( if_p ) : DCAST<const IF*>( m_sc_port[m_portNum] );
    if( iface == 0 ) {
        report_error( sc_core::SC_ID_FIND_EVENT_, "port is not bound" );
    }
    return (CCAST<IF*>( iface )->*m_event_method) ();
}


#endif
