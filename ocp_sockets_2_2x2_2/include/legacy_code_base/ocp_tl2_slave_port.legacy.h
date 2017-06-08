// 
// Copyright 2004 OCP-IP
// OCP-IP Confidential & Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : Alan Kamas, for Sonics, Inc.
//              : Yann Bajot, Prosilog
//          $Id:
//
//  Description : OCP TL2 Slave port declaration.
//                These methods are implemented in the Channel
//
// ============================================================================

#ifndef __OCP_TL2_SLAVE_PORT_LEGACY_H__
#define __OCP_TL2_SLAVE_PORT_LEGACY_H__

template <class Tdata, class Taddr> class OCP_TL2_SlavePort
    : public sc_core::sc_port< OCP_TL2_SlaveIF<Tdata, Taddr> >
{
    public:

        // Constructor
        OCP_TL2_SlavePort(const char* name)
            : sc_core::sc_port< OCP_TL2_SlaveIF<Tdata, Taddr> >(name)
        {
            // do nothing
        }

        // Access to the "in-band" events of the OCP TL2 Channel using event finders
	sc_core::sc_event_finder& RequestStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_SlaveIF<Tdata, Taddr> >( *this, &OCP_TL2_SlaveIF<Tdata, Taddr>::RequestStartEvent ); }
	sc_core::sc_event_finder& RequestEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_SlaveIF<Tdata, Taddr> >( *this, &OCP_TL2_SlaveIF<Tdata, Taddr>::RequestEndEvent ); }
	sc_core::sc_event_finder& ResponseStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_SlaveIF<Tdata, Taddr> >( *this, &OCP_TL2_SlaveIF<Tdata, Taddr>::ResponseStartEvent ); }
	sc_core::sc_event_finder& ResponseEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_SlaveIF<Tdata, Taddr> >( *this, &OCP_TL2_SlaveIF<Tdata, Taddr>::ResponseEndEvent ); }

        // Access to the MThreadBusyEvent event of the OCP TL2 Channel using event finders
	sc_core::sc_event_finder& MThreadBusyEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_SlaveIF<Tdata, Taddr> >( *this, &OCP_TL2_SlaveIF<Tdata, Taddr>::MThreadBusyEvent ); }

        // Access to timing variables update event of OCP TL2 Channel using event finders
	sc_core::sc_event_finder& MasterTimingEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_SlaveIF<Tdata, Taddr> >( *this, &OCP_TL2_SlaveIF<Tdata, Taddr>::MasterTimingEvent ); }

        // Reset Events
        sc_core::sc_event_finder& ResetStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_SlaveIF<Tdata, Taddr> >( *this, &OCP_TL2_SlaveIF<Tdata, Taddr>::ResetStartEvent ); }
        sc_core::sc_event_finder& ResetEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_SlaveIF<Tdata, Taddr> >( *this, &OCP_TL2_SlaveIF<Tdata, Taddr>::ResetEndEvent ); }

        // Sideband Events
        sc_core::sc_event_finder& SidebandMasterEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_SlaveIF<Tdata, Taddr> >( *this, &OCP_TL2_SlaveIF<Tdata, Taddr>::SidebandMasterEvent ); }
        sc_core::sc_event_finder& SidebandCoreEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_SlaveIF<Tdata, Taddr> >( *this, &OCP_TL2_SlaveIF<Tdata, Taddr>::SidebandCoreEvent ); }
        sc_core::sc_event_finder& SidebandSystemEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL2_SlaveIF<Tdata, Taddr> >( *this, &OCP_TL2_SlaveIF<Tdata, Taddr>::SidebandSystemEvent ); }

};
#endif
