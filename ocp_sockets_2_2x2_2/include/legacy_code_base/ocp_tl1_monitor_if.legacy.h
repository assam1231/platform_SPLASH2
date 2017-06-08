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
//  Description : OCP TL1 Monitor Interface
//
// ============================================================================

#ifndef __OCP_TL1_MONITOR_IF_LEGACY_H__
#define __OCP_TL1_MONITOR_IF_LEGACY_H__

template <typename TdataCl>
class OCP_TL1_MonitorIF :  virtual public sc_core::sc_interface
{
public:

  typedef typename TdataCl::DataType	Td;
  typedef typename TdataCl::AddrType	Ta;
  typedef OCPRequestGrp<Td,Ta>		request_type;
  typedef OCPDataHSGrp<Td>		datahs_type;
  typedef OCPResponseGrp<Td>		response_type;
  typedef ParamCl<TdataCl>		paramcl_type;

  // Monitor access
  virtual  const OCPMCmdType getMCmdTrace ()   const = 0;
  virtual  const bool getMDataValidTrace ()    const = 0;
  virtual  const OCPSRespType getSRespTrace () const = 0;

  // port names
  virtual const std::string		peekChannelName()	const = 0;
  virtual const std::string		peekMasterPortName()	const = 0;
  virtual const std::string		peekSlavePortName()	const = 0;

  // transactions
  virtual const request_type&	peekOCPRequest()	const = 0;
  virtual const datahs_type&	peekDataHS()		const = 0;
  virtual const response_type&	peekOCPResponse()	const = 0;

  virtual const bool		peekRequestEnd()	const = 0;
  virtual const bool		peekRequestStart()	const = 0;
  virtual const bool		peekRequestEarlyEnd()	const = 0;
  
  virtual const bool		peekResponseEnd()	const = 0;
  virtual const bool		peekResponseStart()	const = 0;
  virtual const bool		peekResponseEarlyEnd()	const = 0;

  virtual const bool		peekDataRequestEnd()	const = 0;
  virtual const bool		peekDataRequestStart()	const = 0;
  virtual const bool		peekDataRequestEarlyEnd() const = 0;

  // thread busy 
  virtual const unsigned int	peekSThreadBusy()	const = 0;
  virtual const unsigned int	peekSDataThreadBusy()	const = 0;
  virtual const unsigned int	peekMThreadBusy()	const = 0;

  // reset
  virtual const bool		peekMReset_n()		const = 0;
  virtual const bool		peekSReset_n()		const = 0;

  // sideband signals
  virtual const bool		peekMError()		const = 0;
  virtual const unsigned int	peekMFlag()		const = 0;
  virtual const bool		peekSError()		const = 0;
  virtual const unsigned int	peekSFlag()		const = 0;
  virtual const bool		peekSInterrupt()	const = 0;
  virtual const unsigned int	peekControl()		const = 0;
  virtual const bool		peekControlWr()		const = 0;
  virtual const bool		peekControlBusy()	const = 0;
  virtual const unsigned int	peekStatus()		const = 0;
  virtual const bool		peekStatusRd()		const = 0;
  virtual const bool		peekStatusBusy()	const = 0;
  virtual const bool		peekExitAfterOCPMon()	const = 0;
    

  // OCP paramertes
  virtual paramcl_type*		GetParamCl()		      = 0;
  virtual OCPParameters*        GetParameters()               = 0;
};

#endif
