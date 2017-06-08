// 
//  (c) Copyright OCP-IP 2005
//  OCP-IP Confidential and Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : Tim Kogel, CoWare, Inc.
//
//          $Id:
//
//  Description : OCP TL2 Monitor Observer Interface
//
// ============================================================================

#ifndef __OCP_TL2_MONITOR_OBSERVER_IF_LEGACY_H__
#define __OCP_TL2_MONITOR_OBSERVER_IF_LEGACY_H__

template <class, class> class OCP_TL2_MonitorPeekIF;


template <class Tdata, class Taddr>
class OCP_TL2_Monitor_ObserverIF
{
public:
  typedef OCP_TL2_MonitorPeekIF<Tdata,Taddr> tl2_peek_type;

  virtual ~OCP_TL2_Monitor_ObserverIF() {};

  virtual void registerChannel(tl2_peek_type *, 
			       bool master_is_node=false,
			       bool slave_is_node=false){ std::cerr << "OCP_TL2_Monitor_ObserverIF::registerChannel, not implemented in this monitor\n"; }
  virtual void start_of_simulation()			{ std::cerr << "OCP_TL2_Monitor_ObserverIF::start_of_simulation, not implemented in this monitor\n"; }

  virtual void NotifyRequestStart(tl2_peek_type *)	{ std::cerr << "OCP_TL2_Monitor_ObserverIF::NotifyRequestStart, not implemented in this monitor\n"; }
  virtual void NotifyRequestEnd(tl2_peek_type *)	{ std::cerr << "OCP_TL2_Monitor_ObserverIF::NotifyRequestEnd, not implemented in this monitor\n"; }
  virtual void NotifyResponseStart(tl2_peek_type *)	{ std::cerr << "OCP_TL2_Monitor_ObserverIF::NotifyResponseStart, not implemented in this monitor\n"; }
  virtual void NotifyResponseEnd(tl2_peek_type *)	{ std::cerr << "OCP_TL2_Monitor_ObserverIF::NotifyResponseEnd, not implemented in this monitor\n"; }

  virtual void NotifyMThreadBusy(tl2_peek_type *)	{ std::cerr << "OCP_TL2_Monitor_ObserverIF::NotifyMThreadBusy, not implemented in this monitor\n"; }
  virtual void NotifySThreadBusy(tl2_peek_type *)	{ std::cerr << "OCP_TL2_Monitor_ObserverIF::NotifySThreadBusy, not implemented in this monitor\n"; }

  // timing
  virtual void NotifyMasterTiming(tl2_peek_type *)	{ std::cerr << "OCP_TL2_Monitor_ObserverIF::NotifyMasterTiming, not implemented in this monitor\n"; }
  virtual void NotifySlaveTiming(tl2_peek_type *)	{ std::cerr << "OCP_TL2_Monitor_ObserverIF::NotifySlaveTiming, not implemented in this monitor\n"; }

  // reset
  virtual void NotifyResetStart(tl2_peek_type *)	{ std::cerr << "OCP_TL2_Monitor_ObserverIF::NotifyResetStart, not implemented in this monitor\n"; }
  virtual void NotifyResetEnd(tl2_peek_type *)		{ std::cerr << "OCP_TL2_Monitor_ObserverIF::NotifyResetEnd, not implemented in this monitor\n"; }

  // sideband signals
  virtual void NotifySidebandMaster(tl2_peek_type *)	{ std::cerr << "OCP_TL2_Monitor_ObserverIF::NotifySidebandMaster, not implemented in this monitor\n"; }
  virtual void NotifySidebandSlave(tl2_peek_type *)	{ std::cerr << "OCP_TL2_Monitor_ObserverIF::NotifySidebandSlave, not implemented in this monitor\n"; }
  virtual void NotifySidebandCore(tl2_peek_type *)	{ std::cerr << "OCP_TL2_Monitor_ObserverIF::NotifySidebandCore, not implemented in this monitor\n"; }
  virtual void NotifySidebandSystem(tl2_peek_type *)	{ std::cerr << "OCP_TL2_Monitor_ObserverIF::NotifySidebandSystem, not implemented in this monitor\n"; }
};
#endif
