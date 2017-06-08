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
//  Description : OCP TL2 Monitor Interface
//
// ============================================================================

#ifndef __OCP_TL2_MONITOR_IF_LEGACY_H__
#define __OCP_TL2_MONITOR_IF_LEGACY_H__

template <class Tdata, class Taddr>
class OCP_TL2_MonitorPeekIF :  virtual public sc_core::sc_interface
{
public:
  typedef OCPTL2RequestGrp<Tdata,Taddr>			request_type;
  typedef OCPTL2ResponseGrp<Tdata>			response_type;

  // port names
  virtual const std::string     peekChannelName()	const = 0;
  virtual const std::string     peekMasterPortName()	const = 0;
  virtual const std::string     peekSlavePortName()	const = 0;

  // transactions
  virtual const request_type&	peekOCPRequest()	const = 0;
  virtual const response_type&	peekOCPResponse()	const = 0;
  virtual bool			requestInProgress()	const = 0;
  virtual bool			responseInProgress()	const = 0;

  // thread busy 
  virtual const unsigned int	peekMThreadBusy()	const = 0;
  virtual const unsigned int	peekSThreadBusy()	const = 0;

  // timing
  virtual const MTimingGrp&	peekMasterTiming()	const = 0;
  virtual const STimingGrp&	peekSlaveTiming()	const = 0;

  // timing helper
  virtual int			getWDI()		const = 0;
  virtual int			getRqI()		const = 0;
  virtual int			getTL2ReqDuration()	const = 0;
  virtual int			getRDI()		const = 0;
  virtual int			getTL2RespDuration()	const = 0;

  // reset
  virtual bool			getReset()		      = 0;

  // sideband signals
  virtual const OCPSidebandGrp&	peekSideband()		const = 0;

  // OCP paramertes
  virtual OCPParameters*	GetParamCl()		      = 0;
};

template <class Tdata, class Taddr>
class OCP_TL2_MonitorRegisterIF :  virtual public sc_core::sc_interface
{
public:
  typedef OCP_TL2_Monitor_ObserverIF<Tdata,Taddr>	observer_type;

  // transactions
  virtual void			RegisterRequestStart (observer_type *) = 0;
  virtual void			RegisterRequestEnd   (observer_type *) = 0;
  virtual void			RegisterResponseStart(observer_type *) = 0;
  virtual void			RegisterResponseEnd  (observer_type *)  = 0;
  // thread busy 
  virtual void			RegisterMThreadBusy(observer_type *)  = 0;
  virtual void			RegisterSThreadBusy(observer_type *)  = 0;

  // timing
  virtual void			RegisterMasterTiming(observer_type *) = 0;
  virtual void			RegisterSlaveTiming (observer_type *) = 0;

  // reset
  virtual void			RegisterResetStart(observer_type *) = 0;
  virtual void			RegisterResetEnd  (observer_type *) = 0;

  // sideband signals
  virtual void			RegisterSidebandMaster(observer_type *) = 0;
  virtual void			RegisterSidebandSlave (observer_type *) = 0;
  virtual void			RegisterSidebandCore  (observer_type *) = 0;
  virtual void			RegisterSidebandSystem(observer_type *) = 0;

};
template <class Tdata, class Taddr>
class OCP_TL2_MonitorIF :  
  virtual public OCP_TL2_MonitorPeekIF<Tdata,Taddr>,
  virtual public OCP_TL2_MonitorRegisterIF<Tdata,Taddr>
{};

#endif
