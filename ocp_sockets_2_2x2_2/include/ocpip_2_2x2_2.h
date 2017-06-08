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
//  Description :  This is the main include header for ocp release ocpip_2_2x2_2
//                 
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef __OCP_IP_2_2x2_2_H__
#define __OCP_IP_2_2x2_2_H__

#define OCPIP_VERSION ocpip_2_2x2_2

#include "instance_specific_extensions.h"
#include "peq_with_cb_and_phase.h"

#ifndef NO_DEMANGLE
#include <cxxabi.h>
#endif

#include "generic/ocp_macros.h"

#include <math.h>
#include <stdio.h>

#include <fstream>

#include <string>
#include <map>
#include <vector>
#include <set>

#include <assert.h>
#include <deque>
#include <list>

#include <iostream>
#include <sstream>

#include <iomanip>
#include <bitset>

#include <stdint.h>
#include <stdarg.h>

#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_data_class.h)

//just a pool
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_extension_pool.h)

//this is the part of the OCPIP infrastructure that enables bind checking
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_base_callback_binder_base.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_extension_base.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_base_socket_config.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_base_socket_osci_config.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_base_socket_extension_support_base.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_bind_checker.h)

//this is the (replaceable) socket code
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_base_callbacks.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_base_callback_binders.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_base_initiator_socket_callback_base.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_base_socket_mm_base.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_base_socket_default_pool.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_base_initiator_socket_base.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_base_target_socket_callback_base.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_base_target_socket_base.h)

//'move' legacy code into namespace
namespace ocpip_legacy{
#if defined (SYSTEMC20) || defined (SYSTEMC21V1)
    #warning "Please change the define in the build script to replace SYSTEMC20\
		      with OSCI20 -OR- SYSTEMC21V1 with OSCI21V1" 
    //If the above is changed to error then remove the next two defines. 
    #ifdef SYSTEMC20
        #define OSCI20
    #endif

    #ifdef SYSTEMC21V1
        #define OSCI21V1
    #endif
#endif //End of define checks for SYSTEMC20 || SYSTEM21V1

#include "legacy_code_base/ocp_globals.legacy.h"
#include "legacy_code_base/ocp_param.legacy.h"
#include "legacy_code_base/ocp_tl_param_cl.legacy.h"
#include "legacy_code_base/ocp_utils.legacy.h"
#include "legacy_code_base/ocp_tl1_monitor_if.legacy.h"
#include "legacy_code_base/ocp_tl2_monitor_observer_if.legacy.h"
#include "legacy_code_base/ocp_tl2_monitor_if.legacy.h"

#ifdef OCP_USE_LEGACY

#include "legacy_code_base/multiport_event_finder.legacy.h"
#include "legacy_code_base/ocp_tl_config_listener.legacy.h"
#include "legacy_code_base/ocp_tl1_data_cl.legacy.h"
#include "legacy_code_base/tl_comm_cl.legacy.h"
#include "legacy_code_base/tl_channel.legacy.h"
#include "legacy_code_base/ocp_tl1_master_timing_cl.legacy.h"
#include "legacy_code_base/ocp_tl1_slave_timing_cl.legacy.h"
#include "legacy_code_base/ocp_tl1_master_timing_if.legacy.h"
#include "legacy_code_base/ocp_tl1_slave_timing_if.legacy.h"
#include "legacy_code_base/ocp_tl1_master_if.legacy.h"
#include "legacy_code_base/ocp_tl1_slave_if.legacy.h"
#include "legacy_code_base/ocp_tl1_thread_pipelined_support.legacy.h"
#include "legacy_code_base/tl_config_manager.legacy.h"
#include "legacy_code_base/ocp_tl1_master_port.legacy.h"
#include "legacy_code_base/ocp_tl1_slave_port.legacy.h"
#include "legacy_code_base/ocp_tl1_master_multiport.legacy.h"
#include "legacy_code_base/ocp_tl1_slave_multiport.legacy.h"
#include "legacy_code_base/ocp_tl1_channel.legacy.h"
#include "legacy_code_base/ocp_tl1_channel_clocked.legacy.h"
#include "legacy_code_base/ocp_tl1_channel_untimed.legacy.h"
#include "legacy_code_base/ocp_tl2_master_if.legacy.h"
#include "legacy_code_base/ocp_tl2_slave_if.legacy.h"
#include "legacy_code_base/ocp_tl2_master_port.legacy.h"
#include "legacy_code_base/ocp_tl2_slave_port.legacy.h"
#include "legacy_code_base/ocp_tl2_tmon.legacy.h"
#include "legacy_code_base/ocp_tl2_channel.legacy.h"

#endif
}

// || defined(USE_OCP_MONITOR_ocpip_2_2x2_2)
#if defined(OCP_USE_LEGACY_SCV)
# include "scv.h"
// this legacy file defines scv template specializations and thus does not belong
// in the ocpip_legacy namespace
#ifdef USE_OCP_MONITOR_ocpip_2_2x2_2
#  include "../mon/include/legacy_code_base/ocp_perf_monitor_extensions.legacy.h"
#endif
#endif

//and this is the part of the code that defines the behavior of the OCP kit
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_tlm_extensions.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_parameters.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_timing_classes.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_callbacks.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_peq.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_config_support.h)

#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_interface_wrapper.h)

#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_master_socket.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_master_socket_tl1.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_master_socket_tl2.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_master_socket_tl3.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_slave_socket.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_slave_socket_tl1.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_slave_socket_tl2.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_slave_socket_tl3.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_tl1_legacy_convertion_helper.h)

//"convenience" layer
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_txn_inst_extensions.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_txn_utils.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_tl1_master_timing_guard.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_tl1_slave_timing_guard.h)

#ifdef OCP_USE_LEGACY
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_tl1_master_legacy_adapter.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_tl1_slave_legacy_adapter.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_tl2_master_legacy_adapter.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/ocp_tl2_slave_legacy_adapter.h)
#endif

#ifdef USE_OCP_MONITOR_ocpip_2_2x2_2
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_base_socket_monitor.h)
#include __MACRO_STRINGYFY__(OCPIP_VERSION/base_socket/ocp_base_socket_observer_base.h)
//the following paths are relative to ocpip/include
#include __MACRO_STRINGYFY__(../mon/include/OCPIP_VERSION/ocp_monitor_helpers.h)
#include __MACRO_STRINGYFY__(../mon/include/OCPIP_VERSION/ocp_tl1_monitor_signal.h)
#include __MACRO_STRINGYFY__(../mon/include/OCPIP_VERSION/ocp_tl1_monitor_adapter.h)
#include __MACRO_STRINGYFY__(../mon/include/OCPIP_VERSION/ocp_tl2_txn_monitor.h)
#include __MACRO_STRINGYFY__(../mon/include/OCPIP_VERSION/ocp_tl3_imc_logger.h)

namespace ocpip_legacy{
#include "../mon/include/legacy_code_base/ocp_tl1_monitor_port.legacy.h"
#include "../mon/include/legacy_code_base/ocp_tl1_ocpmongen_cl.legacy.h"
//perf monitors need SCV
#if defined(OCP_USE_LEGACY_SCV)
#  include "../mon/include/legacy_code_base/ocp_perf_monitor_channel_scv.legacy.h"
#  include "../mon/include/legacy_code_base/ocp_perf_monitor_system_scv.legacy.h"
#  include "../mon/include/legacy_code_base/ocp_perf_monitor_system_registry.legacy.h"
#  include "../mon/include/legacy_code_base/ocp_tl1_perf_monitor_wrapper.legacy.h"
#  include "../mon/include/legacy_code_base/ocp_tl1_perf_monitor.legacy.h"
#endif
#include "../mon/include/legacy_code_base/ocp_tl1_trace_monitor_clocked.legacy.h"
}
#endif

#undef OCPIP_VERSION

#endif
