// 
//  (c) Copyright OCP-IP 2003
//  OCP-IP Confidential and Proprietary
//
// ============================================================================
//      Project : OCP SLD WG
//       Author : Norman Weyrich, Synopsys Inc.
//                Anssi Haverinen, Nokia Inc.
//                Joe Chou, Sonics Inc.
//                Alan Kamas, for Sonics Inc.
//           $Id:
//
//  Description : Transaction Level - parameter class.
//                The parameter class is a means to report
//                parameters like Master priorities or Slave addresses
//                to other modules (e.g. the Bus). 
//                Since this class is built upon the OCP parameter class,
//                all of the OCP parameters are available as well.
//
// ============================================================================

// This file contains the definition of the classes:
// template <class TdataCl> class ParamCl
// class ParamCl

#ifndef __OCP_TL_PARAM_CL_LEGACY_H__
#define __OCP_TL_PARAM_CL_LEGACY_H__


typedef std::map<std::string, std::string> MapStringType;

template <class TdataCl> 
class ParamCl : public OCPParameters
{
public:

  typedef typename TdataCl::AddrType Ta;

  // Constructor
  ParamCl()
      : OCPParameters()
      , MasterID(-1)
      , Priority(-1)
      , SlaveID(-1)
      , StartAddress(0)
      , EndAddress(0)
  { }

  //  Master parameter
  int MasterID; // Non-negative integer number < TL2_MAX_MASTERS
  int Priority; // Non-negative integer number (higher value = higher priority)

  // Slave parameter
  int SlaveID;     // Non-negative integer number < TL2_MAX_SLAVES
  Ta StartAddress; // Start address of the memory
  Ta EndAddress;   // End address of the memory

};


#endif
