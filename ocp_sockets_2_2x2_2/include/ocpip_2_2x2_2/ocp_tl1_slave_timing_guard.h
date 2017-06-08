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
  #error ocp_tl1_slave_timing_guard.h may not be included directly. Use #include "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION {
class ocp_tl1_slave_timing_guard : public sc_core::sc_module
{
public:
    SC_HAS_PROCESS( ocp_tl1_slave_timing_guard );
    // define all methods in place, this class is not templated
    ocp_tl1_slave_timing_guard( sc_core::sc_module_name nm ) :
	sc_core::sc_module( nm )
	, m_clk( "clk" )
	, m_posedge_time( sc_core::SC_ZERO_TIME )
    {
	SC_METHOD( on_pos_edge );
	sensitive << m_clk.pos();
	dont_initialize();
    }

    sc_core::sc_in_clk    m_clk;
    void receive_master_timing( ocp_tl1_master_timing timing ) { m_master_timing = timing; }

    #define ELAPSED_TIME( varname ) sc_core::sc_time varname = sc_core::sc_time_stamp() - m_posedge_time
    bool is_request_stable() const {
	ELAPSED_TIME( time_since_posedge );
	return time_since_posedge >= m_master_timing.RequestGrpStartTime;
    }
    sc_core::sc_time time_to_request_stable() const {
	ELAPSED_TIME( time_since_posedge );
	if ( time_since_posedge >= m_master_timing.RequestGrpStartTime )
	    return sc_core::SC_ZERO_TIME; // stable
	else
	    return m_master_timing.RequestGrpStartTime - time_since_posedge;
    }
    bool is_datahs_stable() const {
	ELAPSED_TIME( time_since_posedge );
	return time_since_posedge >= m_master_timing.DataHSGrpStartTime;
    }
    sc_core::sc_time time_to_datahs_stable() const {
	ELAPSED_TIME( time_since_posedge );
	if ( time_since_posedge >= m_master_timing.DataHSGrpStartTime )
	    return sc_core::SC_ZERO_TIME; // stable
	else
	    return m_master_timing.DataHSGrpStartTime - time_since_posedge;
    }

    bool is_respaccept_stable() const {
	ELAPSED_TIME( time_since_posedge );
	return time_since_posedge >= m_master_timing.MRespAcceptStartTime;
    }
    sc_core::sc_time time_to_respaccept_stable() const {
	ELAPSED_TIME( time_since_posedge );
	if ( time_since_posedge >= m_master_timing.MRespAcceptStartTime )
	    return sc_core::SC_ZERO_TIME; // stable
	else
	    return m_master_timing.MRespAcceptStartTime - time_since_posedge;
    }

    bool is_mthreadbusy_stable() const {
	ELAPSED_TIME( time_since_posedge );
	return time_since_posedge >= m_master_timing.MThreadBusyStartTime;
    }
    sc_core::sc_time time_to_mthreadbusy_stable() const {
	ELAPSED_TIME( time_since_posedge );
	if ( time_since_posedge >= m_master_timing.MThreadBusyStartTime )
	    return sc_core::SC_ZERO_TIME; // stable
	else
	    return m_master_timing.MThreadBusyStartTime - time_since_posedge;
    }
#undef ELAPSED_TIME

    ocp_tl1_master_timing m_master_timing;

private:
    sc_core::sc_time      m_posedge_time;
    void                  on_pos_edge() {
	m_posedge_time = sc_core::sc_time_stamp();
    }
};
}
