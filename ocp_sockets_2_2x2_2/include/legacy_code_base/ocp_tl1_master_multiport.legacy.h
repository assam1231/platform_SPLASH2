// 
//  (c) Copyright OCP-IP 2003, 2004, 2005
//  OCP-IP Confidential and Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : Alan Kamas, for Sonics, Inc.
//           $Id:
//
//  Description : OCP TL1 Master multiport declaration.
//                These methods are implemented in the Channel
//                (see ocp_tl1_channel.h)
//
// ============================================================================

#ifndef __OCP_TL1_MASTER_MULTIPORT_LEGACY_H__
#define __OCP_TL1_MASTER_MULTIPORT_LEGACY_H__

template <class TdataCl> class OCP_TL1_MasterMultiPort
: public sc_core::sc_port<OCP_TL1_MasterIF<TdataCl>, 0>
{
    public:

        // Constructor
        OCP_TL1_MasterMultiPort(const char* name)
            : sc_core::sc_port<OCP_TL1_MasterIF<TdataCl>, 0>(name)
        {
            // do nothing
        }

        // Access to the "in-band" events of the OCP TL1 Channel using event finders
	sc_core::sc_event_finder& RequestStartEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::RequestStartEvent, myPort ); }
	sc_core::sc_event_finder& RequestEndEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::RequestEndEvent, myPort ); }
	sc_core::sc_event_finder& DataRequestStartEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::DataRequestStartEvent, myPort ); }
	sc_core::sc_event_finder& DataRequestEndEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::DataRequestEndEvent, myPort ); }
	sc_core::sc_event_finder& ResponseStartEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::ResponseStartEvent, myPort ); }
	sc_core::sc_event_finder& ResponseEndEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::ResponseEndEvent, myPort ); }

        // Access to the Master Sideband events of the OCP TL1 Channel using event finders
	sc_core::sc_event_finder& SidebandSErrorEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandSErrorEvent, myPort ); }
	sc_core::sc_event_finder& SidebandSFlagEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandSFlagEvent, myPort ); }
	sc_core::sc_event_finder& SidebandSInterruptEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandSInterruptEvent, myPort ); }

        // Access to OCP System Sideband Events of the OCP TL1 Channel using event finders
	sc_core::sc_event_finder& SidebandControlBusyEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandControlBusyEvent, myPort ); }
	sc_core::sc_event_finder& SidebandStatusEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandStatusEvent, myPort ); }
	sc_core::sc_event_finder& SidebandStatusBusyEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandStatusBusyEvent, myPort ); }

        // Access to OCP Core Sideband Events of the OCP TL1 Channel using event finders
	sc_core::sc_event_finder& SidebandControlEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandControlEvent, myPort ); }
	sc_core::sc_event_finder& SidebandControlWrEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandControlWrEvent, myPort ); }
	sc_core::sc_event_finder& SidebandStatusRdEvent(int myPort = 0) const { return *new multiport_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandStatusRdEvent, myPort ); }

};

#endif

