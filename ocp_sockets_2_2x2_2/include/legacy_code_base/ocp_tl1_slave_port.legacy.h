// 
//  (c) Copyright OCP-IP 2004, 2005
//  OCP-IP Confidential and Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : Yann Bajot, Prosilog
//              : Alan Kamas, for Sonics, Inc.
//           $Id:
//
//  Description : OCP TL1 Slave port declaration.
//                These methods are implemented in the Channel
//                (see ocp_tl1_channel.h)
//
// ============================================================================

#ifndef __OCP_TL1_SLAVE_PORT_LEGACY_H__
#define __OCP_TL1_SLAVE_PORT_LEGACY_H__

template <class TdataCl> class OCP_TL1_SlavePort
: public sc_core::sc_port< OCP_TL1_SlaveIF<TdataCl> >
{
    public:

        // Constructor
        OCP_TL1_SlavePort(const char* name)
            : sc_core::sc_port< OCP_TL1_SlaveIF<TdataCl> >(name)
        {
            // do nothing
        }

        // Access to the "in-band" events of the OCP TL1 Channel using event finders
	sc_core::sc_event_finder& RequestStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::RequestStartEvent ); }
	sc_core::sc_event_finder& RequestEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::RequestEndEvent ); }
	sc_core::sc_event_finder& DataHSStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::DataHSStartEvent ); }
	sc_core::sc_event_finder& DataHSEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::DataHSEndEvent ); }
	sc_core::sc_event_finder& ResponseStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::ResponseStartEvent ); }
	sc_core::sc_event_finder& ResponseEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::ResponseEndEvent ); }

        // Access to the ThreadBusy Events
        sc_core::sc_event_finder& CurrentMThreadBusyEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::CurrentMThreadBusyEvent ); }
        
        sc_core::sc_event_finder& MThreadBusyEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::MThreadBusyEvent ); }
        
        // Access to the Reset Events
        sc_core::sc_event_finder& ResetStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::ResetStartEvent ); }
        sc_core::sc_event_finder& ResetEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::ResetEndEvent ); }
        sc_core::sc_event_finder& getFakeResetEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::getFakeResetEndEvent ); }
        sc_core::sc_event_finder& getFakeResetStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::getFakeResetStartEvent ); }
        
        // Access to the Slave Sideband events of the OCP TL1 Channel using event finders
	sc_core::sc_event_finder& SidebandMErrorEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::SidebandMErrorEvent ); }
	sc_core::sc_event_finder& SidebandMFlagEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::SidebandMFlagEvent ); }

        // Access to OCP System Sideband Events of the OCP TL1 Channel using event finders
	sc_core::sc_event_finder& SidebandControlBusyEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::SidebandControlBusyEvent ); }
	sc_core::sc_event_finder& SidebandStatusEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::SidebandStatusEvent ); }
	sc_core::sc_event_finder& SidebandStatusBusyEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::SidebandStatusBusyEvent ); }

        // Access to OCP Core Sideband Events of the OCP TL1 Channel using event finders
	sc_core::sc_event_finder& SidebandControlEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::SidebandControlEvent ); }
	sc_core::sc_event_finder& SidebandControlWrEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::SidebandControlWrEvent ); }
	sc_core::sc_event_finder& SidebandStatusRdEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::SidebandStatusRdEvent ); }

        //---------------------------------------------------------------------
        // Public access methods for 'Release' events.
        // Note: these events are only needed in a very special case (the TL0-TL1
        // adapters), and should not be used to code 'normal' TL1 models.
        //---------------------------------------------------------------------

	sc_core::sc_event_finder& RequestReleaseEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::RequestReleaseEvent ); }
	sc_core::sc_event_finder& DataHSReleaseEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::DataHSReleaseEvent ); }
	sc_core::sc_event_finder& ResponseReleaseEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_SlaveIF<TdataCl> >( *this, &OCP_TL1_SlaveIF<TdataCl>::ResponseReleaseEvent ); }


};

#endif
