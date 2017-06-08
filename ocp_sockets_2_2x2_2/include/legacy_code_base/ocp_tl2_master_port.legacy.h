// 
// Copyright 2003 OCP-IP
// OCP-IP Confidential & Proprietary
//
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : Alan Kamas for Sonics, Inc.
//              : Yann Bajot, Prosilog
//          $Id:
//
//  Description : OCP TL2 Master port declaration.
//                These methods are implemented in the Channel
//
// ============================================================================

#ifndef __OCP_TL2_MASTER_PORT_LEGACY_H__
#define __OCP_TL2_MASTER_PORT_LEGACY_H__

template <class Tdata, class Taddr> 
class OCP_TL2_MasterPort
    : public sc_core::sc_port< OCP_TL2_MasterIF<Tdata, Taddr> >
{
    public:

        // Constructor
        OCP_TL2_MasterPort(const char* name)
            : sc_core::sc_port< OCP_TL2_MasterIF<Tdata, Taddr> >(name)
        {
            // do nothing
        }

        // Access to the "in-band" events of the OCP TL2 Channel using event finders
	sc_core::sc_event_finder& RequestStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_MasterIF<Tdata, Taddr> >( *this, &OCP_TL2_MasterIF<Tdata, Taddr>::RequestStartEvent ); }
	sc_core::sc_event_finder& RequestEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_MasterIF<Tdata, Taddr> >( *this, &OCP_TL2_MasterIF<Tdata, Taddr>::RequestEndEvent ); }
	sc_core::sc_event_finder& ResponseStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_MasterIF<Tdata, Taddr> >( *this, &OCP_TL2_MasterIF<Tdata, Taddr>::ResponseStartEvent ); }
	sc_core::sc_event_finder& ResponseEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_MasterIF<Tdata, Taddr> >( *this, &OCP_TL2_MasterIF<Tdata, Taddr>::ResponseEndEvent ); }

        // Access to the SThreadBusyEvent event of the OCP TL2 Channel using event finders
	sc_core::sc_event_finder& SThreadBusyEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_MasterIF<Tdata, Taddr> >( *this, &OCP_TL2_MasterIF<Tdata, Taddr>::SThreadBusyEvent ); }

        // Access to timing variable update event
	sc_core::sc_event_finder& SlaveTimingEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_MasterIF<Tdata, Taddr> >( *this, &OCP_TL2_MasterIF<Tdata, Taddr>::SlaveTimingEvent ); }

        // Reset Events
	sc_core::sc_event_finder& ResetStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_MasterIF<Tdata, Taddr> >( *this, &OCP_TL2_MasterIF<Tdata, Taddr>::ResetStartEvent ); }
	sc_core::sc_event_finder& ResetEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_MasterIF<Tdata, Taddr> >( *this, &OCP_TL2_MasterIF<Tdata, Taddr>::ResetEndEvent ); }

        // Sideband Events
	sc_core::sc_event_finder& SidebandSlaveEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_MasterIF<Tdata, Taddr> >( *this, &OCP_TL2_MasterIF<Tdata, Taddr>::SidebandSlaveEvent ); }
	sc_core::sc_event_finder& SidebandCoreEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_MasterIF<Tdata, Taddr> >( *this, &OCP_TL2_MasterIF<Tdata, Taddr>::SidebandCoreEvent ); }
	sc_core::sc_event_finder& SidebandSystemEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_MasterIF<Tdata, Taddr> >( *this, &OCP_TL2_MasterIF<Tdata, Taddr>::SidebandSystemEvent ); }
};
#endif
