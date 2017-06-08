///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// (c) Copyright OCP-IP 2008
// OCP-IP Confidential and Proprietary
//
//
//============================================================================
//      Project : OCP SLD WG
//       Author : Robert Guenzel (from TU of Braunschweig) for Greensocs Ltd.
//
//          $Id:
//
//  Description :  This file contains the timing classes to be used by master
//                 and slave sockets. They have been taken out of the 
//                 OCP TL1 channel kit, but their names have been changed
//                 to under_score style instead of CamelCaps.
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_timing_classes.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

//TODO: change members from CamelCaps to under_score

namespace OCPIP_VERSION{

class ocp_tl1_master_timing {
  public:
    sc_core::sc_time RequestGrpStartTime;
    sc_core::sc_time DataHSGrpStartTime;
    sc_core::sc_time MThreadBusyStartTime;
    sc_core::sc_time MRespAcceptStartTime;   // only for preemptive release.  used by channel, not by master
    // default constructor for sc_core::sc_time makes SC_ZERO_TIME - this is "default timing"

    // test for equality
    inline bool operator == (const ocp_tl1_master_timing& rhs) const;
    inline bool operator != (const ocp_tl1_master_timing& rhs) const;

    static inline const ocp_tl1_master_timing& get_default_timing();
};

class ocp_tl1_slave_timing {
  public:
    sc_core::sc_time ResponseGrpStartTime;
    sc_core::sc_time SThreadBusyStartTime;
    sc_core::sc_time SDataThreadBusyStartTime;
    sc_core::sc_time SCmdAcceptStartTime;    // only for preemptive release.  used by channel, not by master
    sc_core::sc_time SDataAcceptStartTime;   // only for preemptive release.  used by channel, not by master
    // default constructor for sc_core::sc_time makes SC_ZERO_TIME - this is "default timing"

    // test for equality
    inline bool operator == (const ocp_tl1_slave_timing& rhs) const;
    inline bool operator != (const ocp_tl1_slave_timing& rhs) const;
    static inline const ocp_tl1_slave_timing& get_default_timing();    
};

} //end ns OCPIP_VERSION

#include __MACRO_STRINGYFY__(../src/OCPIP_VERSION/ocp_timing_classes.tpp)
