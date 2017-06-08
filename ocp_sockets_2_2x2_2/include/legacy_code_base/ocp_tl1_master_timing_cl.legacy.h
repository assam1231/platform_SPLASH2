// (c) Copyright OCP-IP 2005
// OCP-IP Confidential and Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : James Aldis, Texas Instruments France
//         Date : 23/06/2005
//
//  Description : Structure for storing all timing information for a TL1 OCP master
//
//  Change History:
//                Created 6.7.05
// ============================================================================


#ifndef __OCP_TL1_MASTER_TIMING_CL_LEGACY_H__
#define __OCP_TL1_MASTER_TIMING_CL_LEGACY_H__

class OCP_TL1_Master_TimingCl {
  public:
    sc_core::sc_time RequestGrpStartTime;
    sc_core::sc_time DataHSGrpStartTime;
    sc_core::sc_time MThreadBusyStartTime;
    sc_core::sc_time MRespAcceptStartTime;   // only for preemptive release.  used by channel, not by master
    // default constructor for sc_core::sc_time makes SC_ZERO_TIME - this is "default timing"

    // test for equality
    inline bool operator == (const OCP_TL1_Master_TimingCl& rhs) const {
      return ((RequestGrpStartTime  == rhs.RequestGrpStartTime)
           && (DataHSGrpStartTime   == rhs.DataHSGrpStartTime)
           && (MThreadBusyStartTime == rhs.MThreadBusyStartTime)
           && (MRespAcceptStartTime == rhs.MRespAcceptStartTime) );
    }
};

#endif
