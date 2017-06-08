// 
//  (c) Copyright OCP-IP 2003, 2004, 2005
//  OCP-IP Confidential and Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : Norman Weyrich, Synopsys Inc.
//                Anssi Haverinen, Nokia Inc.
//                Joe Chou, Sonics Inc.
//                Alan Kamas for Sonics Inc.
//           $Id:
//
//  Description : OCP Transaction Level communication class.
//                This class gives the modules read access to
//                the events and states used in the Channel.
//                We recommend users not to change this class.
//
// Old change history. Not updated since version control moved to CVS
// 06/13/2003 Original revision for OCP 1.0
// 12/14/2003 Added OCP 2.0 support
// 03/30/2004 Added support for tl_channel *PE() - methods
// ============================================================================

// This file contains the definition of the classes:
// class CommCl


#ifndef __TL_COMM_CL_LEGACY_H__
#define __TL_COMM_CL_LEGACY_H__

// This class contains the states and events used by the
// communication mechanism of the Channel. These states and events
// must not be changed by Masters and Slaves.
// The purpose to export these states and events is to
// give Masters and Slaves read access. Again, these access must be read only.
// For the (intended) normal use of the Channel this class should not be
// changed.

// NOTE: There are some states that may not be needed:
// NOTE: May want to remove: GetRequestPending

class CommCl
{
 public:

  // Constructor
  CommCl()
    : RequestStart(false)
    , RequestEnd(false)
    , RequestAccepted(false)
    , RequestEarlyEnd(false)
    , RequestEndTime(-1.0)
    , ResponseStart(false)
    , ResponseEnd(false) 
    , ResponseAccepted(false) 
    , ResponseEarlyEnd(false)
    , ResponseEndTime(-1.0)
    , DataRequestStart(false)
    , DataRequestEnd(false) 
    , DataRequestAccepted(false) 
    , DataRequestEarlyEnd(false)
    , DataRequestEndTime(-1.0)
    , RequestPending(false)
    , DataRequestPending(false)
    , BlockingRequestPending(false)
    , BlockingDataRequestPending(false)
    , BlockingGetRequestPending(false)
    , BlockingGetDataRequestPending(false)
    , ResponsePending(false)
    , BlockingResponsePending(false)
    , BlockingGetResponsePending(false)
    , RequestUnread(false)
    , DataRequestUnread(false)
    , ResponseUnread(false)
    , RequestRelease(false)
    , ResponseRelease(false)
    , DataRelease(false)
    , RequestEndSent(false)
    , DataRequestEndSent(false)
    , ResponseEndSent(false)
    , Reset(false)
  {
      reset();
  }

  void
  reset(void)
  {
     RequestStart = false;
     RequestEnd = false;
     RequestAccepted = false;
     RequestEarlyEnd = false;
     RequestEndTime = -1.0;
     ResponseStart = false;
     ResponseEnd = false;
     ResponseAccepted = false;
     ResponseEarlyEnd = false;
     ResponseEndTime = -1.0;
     DataRequestStart = false;
     DataRequestEnd = false;
     DataRequestAccepted = false;
     DataRequestEarlyEnd = false;
     DataRequestEndTime = -1.0;
     RequestPending = false;
     DataRequestPending = false;
     BlockingRequestPending = false;
     BlockingDataRequestPending = false;
     BlockingGetRequestPending = false;
     BlockingGetDataRequestPending = false;
     ResponsePending = false;
     BlockingResponsePending = false;
     BlockingGetResponsePending = false;
     RequestUnread = false;
     DataRequestUnread = false;
     ResponseUnread = false;
     RequestRelease = false;
     ResponseRelease = false;
     DataRelease = false;
     RequestEndSent = false;
     DataRequestEndSent = false;
     ResponseEndSent = false;
  }
      
  // States for non-blocking and blocking synchronization
  bool RequestStart;
  bool RequestEnd;
  bool RequestAccepted;
  bool RequestEarlyEnd;
  double RequestEndTime;
  bool ResponseStart;
  bool ResponseEnd;
  bool ResponseAccepted;
  bool ResponseEarlyEnd;
  double ResponseEndTime;
  bool DataRequestStart;
  bool DataRequestEnd;  
  bool DataRequestAccepted;
  bool DataRequestEarlyEnd;
  double DataRequestEndTime;
  bool RequestPending;
  bool DataRequestPending;
  bool BlockingRequestPending;
  bool BlockingDataRequestPending;
  bool GetDataRequestPending;
  bool BlockingGetRequestPending;
  bool BlockingGetDataRequestPending;
  bool ResponsePending;
  bool BlockingResponsePending;
  bool BlockingGetResponsePending;
  bool RequestUnread;   
  bool DataRequestUnread; 
  bool ResponseUnread;  
  
  // For pre-emptive release
  bool RequestRelease; 
  bool ResponseRelease;
  bool DataRelease; 
  bool RequestEndSent;
  bool DataRequestEndSent;
  bool ResponseEndSent;

  // States for Reset
  bool Reset;   // Active High reset flag

  // Events for blocking synchronization
  sc_core::sc_event RequestStartEvent;
  sc_core::sc_event RequestEndEvent;
  sc_core::sc_event RequestReleaseEvent;
  sc_core::sc_event DataRequestStartEvent;
  sc_core::sc_event DataRequestEndEvent;
  sc_core::sc_event DataRequestReleaseEvent;
  sc_core::sc_event ResponseStartEvent;
  sc_core::sc_event ResponseEndEvent;
  sc_core::sc_event ResponseReleaseEvent;
  sc_core::sc_event RequestEndEventN;
  sc_core::sc_event DataRequestEndEventN;
  sc_core::sc_event ResponseEndEventN;

  // default event to indicate port access
  sc_core::sc_event StartEvent;

  // Events for reset
  sc_core::sc_event ResetStartEvent;
  sc_core::sc_event ResetEndEvent;

 private:
};

#endif
