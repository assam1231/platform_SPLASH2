// (c) Copyright OCP-IP 2005
// OCP-IP Confidential and Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : James Aldis, Texas Instruments France
//         Date : 23/06/2005
//
//  Description : Interface for transferring OCP TL1 master timing information
//
//  Change History:
//                Created 6.7.05
// ============================================================================


#ifndef __OCP_TL1_MASTER_TIMING_IF_LEGACY_H__
#define __OCP_TL1_MASTER_TIMING_IF_LEGACY_H__

class OCP_TL1_Master_TimingIF
{
  public:
    virtual void setOCPTL1MasterTiming(OCP_TL1_Master_TimingCl master_timing) = 0;
    virtual ~OCP_TL1_Master_TimingIF(){}
};


#endif
