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
//  Description : OCP TL1 Master port declaration.
//                These methods are implemented in the Channel
//                (see ocp_tl1_channel.h)
//
// ============================================================================

#ifndef __OCP_TL1_MASTER_PORT_LEGACY_H__
#define __OCP_TL1_MASTER_PORT_LEGACY_H__

template <class TdataCl> class OCP_TL1_MasterPort
: public sc_core::sc_port< OCP_TL1_MasterIF<TdataCl> >
{
    public:

        // Constructor
        OCP_TL1_MasterPort(const char* name)
            : sc_core::sc_port< OCP_TL1_MasterIF<TdataCl> >(name)
        {
            // do nothing
        }

        // Access to the "in-band" events of the OCP TL1 Channel using event finders
	sc_core::sc_event_finder& RequestStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::RequestStartEvent ); }
	sc_core::sc_event_finder& RequestEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::RequestEndEvent ); }
	sc_core::sc_event_finder& RequestReleaseEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::RequestReleaseEvent ); }
	sc_core::sc_event_finder& DataHSStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::DataHSStartEvent ); }
	sc_core::sc_event_finder& DataHSEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::DataHSEndEvent ); }
	sc_core::sc_event_finder& DataHSReleaseEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::DataHSReleaseEvent ); }
	sc_core::sc_event_finder& ResponseStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::ResponseStartEvent ); }
	sc_core::sc_event_finder& ResponseEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::ResponseEndEvent ); }
	sc_core::sc_event_finder& ResponseReleaseEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::ResponseReleaseEvent ); }

        // Access to the ThreadBusy Events
	sc_core::sc_event_finder& CurrentSThreadBusyEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::CurrentSThreadBusyEvent ); }
	sc_core::sc_event_finder& CurrentSDataThreadBusyEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::CurrentSDataThreadBusyEvent ); }
    
	sc_core::sc_event_finder& SThreadBusyEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SThreadBusyEvent ); }
	sc_core::sc_event_finder& SDataThreadBusyEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SDataThreadBusyEvent ); }

        // Access to the Reset Events
	sc_core::sc_event_finder& ResetStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::ResetStartEvent ); }
	sc_core::sc_event_finder& ResetEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::ResetEndEvent ); }
    sc_core::sc_event_finder& getFakeResetEndEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::getFakeResetEndEvent ); }    
    sc_core::sc_event_finder& getFakeResetStartEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::getFakeResetStartEvent ); }    

        // Access to the Master Sideband events of the OCP TL1 Channel using event finders
	sc_core::sc_event_finder& SidebandSErrorEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandSErrorEvent ); }
	sc_core::sc_event_finder& SidebandSFlagEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandSFlagEvent ); }
	sc_core::sc_event_finder& SidebandSInterruptEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandSInterruptEvent ); }

        // Access to OCP System Sideband Events of the OCP TL1 Channel using event finders
	sc_core::sc_event_finder& SidebandControlBusyEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandControlBusyEvent ); }
	sc_core::sc_event_finder& SidebandStatusEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandStatusEvent ); }
	sc_core::sc_event_finder& SidebandStatusBusyEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandStatusBusyEvent ); }

        // Access to OCP Core Sideband Events of the OCP TL1 Channel using event finders
	sc_core::sc_event_finder& SidebandControlEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandControlEvent ); }
	sc_core::sc_event_finder& SidebandControlWrEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandControlWrEvent ); }
	sc_core::sc_event_finder& SidebandStatusRdEvent() const { return *new sc_core::sc_event_finder_t<OCP_TL1_MasterIF<TdataCl> >( *this, &OCP_TL1_MasterIF<TdataCl>::SidebandStatusRdEvent ); }

};


#endif
