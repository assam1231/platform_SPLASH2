// (c) Copyright OCP-IP 2005
// OCP-IP Confidential and Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : James Aldis, Texas Instruments France
//         Date : 23/06/2005
//
//  Description : Structure for storing all timing information for a TL1 OCP slave
//
//  Change History:
//                Created 6.7.05
// ============================================================================


#ifndef __OCP_TL1_SLAVE_TIMING_CL_LEGACY_H__
#define __OCP_TL1_SLAVE_TIMING_CL_LEGACY_H__

class OCP_TL1_Slave_TimingCl {
  public:
    sc_core::sc_time ResponseGrpStartTime;
    sc_core::sc_time SThreadBusyStartTime;
    sc_core::sc_time SDataThreadBusyStartTime;
    sc_core::sc_time SCmdAcceptStartTime;    // only for preemptive release.  used by channel, not by master
    sc_core::sc_time SDataAcceptStartTime;   // only for preemptive release.  used by channel, not by master
    // default constructor for sc_core::sc_time makes SC_ZERO_TIME - this is "default timing"

    // test for equality
    inline bool operator == (const OCP_TL1_Slave_TimingCl& rhs) const {
      return ((ResponseGrpStartTime      == rhs.ResponseGrpStartTime)
           && (SThreadBusyStartTime      == rhs.SThreadBusyStartTime)
           && (SDataThreadBusyStartTime  == rhs.SDataThreadBusyStartTime)
           && (SCmdAcceptStartTime       == rhs.SCmdAcceptStartTime)
           && (SDataAcceptStartTime      == rhs.SDataAcceptStartTime) );
    }
};

#endif

