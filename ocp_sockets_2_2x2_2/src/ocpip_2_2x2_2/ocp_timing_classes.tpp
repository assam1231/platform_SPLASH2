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
//  Description :  This file contains the timing classes to be used by master
//                 and slave sockets. They have been taken out of the 
//                 OCP TL1 channel kit, but their names have been changed
//                 to under_score style instead of CamelCaps.
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

bool OCPIP_VERSION::ocp_tl1_master_timing::operator == (const ocp_tl1_master_timing& rhs) const {
  return ((RequestGrpStartTime  == rhs.RequestGrpStartTime)
       && (DataHSGrpStartTime   == rhs.DataHSGrpStartTime)
       && (MThreadBusyStartTime == rhs.MThreadBusyStartTime)
       && (MRespAcceptStartTime == rhs.MRespAcceptStartTime) );
}

bool OCPIP_VERSION::ocp_tl1_master_timing::operator != (const ocp_tl1_master_timing& rhs) const {
  return !(operator==(rhs));
}

const OCPIP_VERSION::ocp_tl1_master_timing& OCPIP_VERSION::ocp_tl1_master_timing::get_default_timing(){
  static ocp_tl1_master_timing def_timing;
  return def_timing;
}

bool OCPIP_VERSION::ocp_tl1_slave_timing::operator == (const ocp_tl1_slave_timing& rhs) const {
  return ((ResponseGrpStartTime      == rhs.ResponseGrpStartTime)
       && (SThreadBusyStartTime      == rhs.SThreadBusyStartTime)
       && (SDataThreadBusyStartTime  == rhs.SDataThreadBusyStartTime)
       && (SCmdAcceptStartTime       == rhs.SCmdAcceptStartTime)
       && (SDataAcceptStartTime      == rhs.SDataAcceptStartTime) );
}

bool OCPIP_VERSION::ocp_tl1_slave_timing::operator != (const ocp_tl1_slave_timing& rhs) const {
  return !(operator==(rhs));
}

const OCPIP_VERSION::ocp_tl1_slave_timing& OCPIP_VERSION::ocp_tl1_slave_timing::get_default_timing(){
  static ocp_tl1_slave_timing def_timing;
  return def_timing;
}
    

