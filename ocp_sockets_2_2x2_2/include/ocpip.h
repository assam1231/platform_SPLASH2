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
//  Description :  This header always includes the ocpip release with the
//                 highest release number. This is set up automatically
//                 with every installation
//                 
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
#ifndef __OCPIP_H__
#define __OCPIP_H__

#ifdef USE_OCP_MONITOR
#define USE_OCP_MONITOR_ocpip_2_2x2_2
#endif

#include "ocpip_2_2x2_2.h"
namespace ocpip=ocpip_2_2x2_2;
#endif
