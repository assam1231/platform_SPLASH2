// (c) Copyright OCP-IP 2005
// OCP-IP Confidential and Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : James Aldis, Texas Instruments France
//         Date : 23/06/2005
//
//  Description : Interface for transferring OCP TL1 slave timing information
//
//  Change History:
//                Created 6.7.05
// ============================================================================


#ifndef __OCP_TL1_SLAVE_TIMING_IF_LEGACY_H__
#define __OCP_TL1_SLAVE_TIMING_IF_LEGACY_H__


class OCP_TL1_Slave_TimingIF
{
  public:
    virtual void setOCPTL1SlaveTiming(OCP_TL1_Slave_TimingCl slave_timing) = 0;
    virtual ~OCP_TL1_Slave_TimingIF(){}
};


#endif
