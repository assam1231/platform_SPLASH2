///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// (c) Copyright OCP-IP 2008-2009
// OCP-IP Confidential and Proprietary
//
//
//============================================================================
//      Project : OCP SLD WG
//       Author : Herve Alexanian - Sonics, inc.
//
//          $Id:
//
//  Description :  This file defines a utility class to manage wait times for
//                 TL1 phases with combinatorial dependencies.
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_tl1_master_timing_guard.h may not be included directly. Use #include "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION {
class ocp_tl1_master_timing_guard : public sc_core::sc_module
{
public:
    SC_HAS_PROCESS( ocp_tl1_master_timing_guard );
    // define all methods in place, this class is not templated
    ocp_tl1_master_timing_guard( sc_core::sc_module_name nm ) :
	sc_core::sc_module( nm )
	, m_clk( "clk" )
	, m_posedge_time( sc_core::SC_ZERO_TIME )
    {
	SC_METHOD( on_pos_edge );
	sensitive << m_clk.pos();
	dont_initialize();
    }

    sc_core::sc_in_clk            m_clk;
    void receive_slave_timing( ocp_tl1_slave_timing timing ) { m_slave_timing = timing; }

    #define ELAPSED_TIME( varname ) sc_core::sc_time varname = sc_core::sc_time_stamp() - m_posedge_time
    bool is_response_stable() const {
	ELAPSED_TIME( time_since_posedge );
	return time_since_posedge >= m_slave_timing.ResponseGrpStartTime;
    }
    sc_core::sc_time time_to_response_stable() const {
	ELAPSED_TIME( time_since_posedge );
	if ( time_since_posedge >= m_slave_timing.ResponseGrpStartTime )
	    return sc_core::SC_ZERO_TIME; // stable
	else
	    return m_slave_timing.ResponseGrpStartTime - time_since_posedge;
    }

    bool is_cmdaccept_stable() const {
	ELAPSED_TIME( time_since_posedge );
	return time_since_posedge >= m_slave_timing.SCmdAcceptStartTime;
    }
    sc_core::sc_time time_to_cmdaccept_stable() const {
	ELAPSED_TIME( time_since_posedge );
	if ( time_since_posedge >= m_slave_timing.SCmdAcceptStartTime )
	    return sc_core::SC_ZERO_TIME; // stable
	else
	    return m_slave_timing.SCmdAcceptStartTime - time_since_posedge;
    }

    bool is_dataaccept_stable() const {
	ELAPSED_TIME( time_since_posedge );
	return time_since_posedge >= m_slave_timing.SDataAcceptStartTime;
    }
    sc_core::sc_time time_to_dataaccept_stable() const {
	ELAPSED_TIME( time_since_posedge );
	if ( time_since_posedge >= m_slave_timing.SDataAcceptStartTime )
	    return sc_core::SC_ZERO_TIME; // stable
	else
	    return m_slave_timing.SDataAcceptStartTime - time_since_posedge;
    }

    bool is_sthreadbusy_stable() const {
	ELAPSED_TIME( time_since_posedge );
	return time_since_posedge >= m_slave_timing.SThreadBusyStartTime;
    }
    sc_core::sc_time time_to_sthreadbusy_stable() const {
	ELAPSED_TIME( time_since_posedge );
	if ( time_since_posedge >= m_slave_timing.SThreadBusyStartTime )
	    return sc_core::SC_ZERO_TIME; // stable
	else
	    return m_slave_timing.SThreadBusyStartTime - time_since_posedge;
    }

    bool is_sdatathreadbusy_stable() const {
	ELAPSED_TIME( time_since_posedge );
	return time_since_posedge >= m_slave_timing.SDataThreadBusyStartTime;
    }
    sc_core::sc_time time_to_sdatathreadbusy_stable() const {
	ELAPSED_TIME( time_since_posedge );
	if ( time_since_posedge >= m_slave_timing.SDataThreadBusyStartTime )
	    return sc_core::SC_ZERO_TIME; // stable
	else
	    return m_slave_timing.SDataThreadBusyStartTime - time_since_posedge;
    }
#undef ELAPSED_TIME

    ocp_tl1_slave_timing m_slave_timing;

private:
    sc_core::sc_time     m_posedge_time;
    void                 on_pos_edge() {
	m_posedge_time = sc_core::sc_time_stamp();
    }
};
}
