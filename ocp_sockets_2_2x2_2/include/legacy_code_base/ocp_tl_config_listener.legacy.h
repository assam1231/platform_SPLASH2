///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// (c) Copyright OCP-IP 2006
// OCP-IP Confidential and Proprietary
//
//
//============================================================================
//      Project : OCP SLD WG
//       Author : Robert Guenzel (from Technical Univerity of Braunschweig) for Greensocs Ltd.
//
//          $Id:
//
//  Description :  Base class from which all modules must inherit that want to
//                 get the channels configuration at end_of_elaboration after
//                 having registered themselves as configuration listeners at
//                 a TL1 or TL2 channel
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef __OCP_TL_CONFIG_LISTENER_LEGACY_H__
#define __OCP_TL_CONFIG_LISTENER_LEGACY_H__

class OCP_TL_Config_Listener {
public:
	virtual void set_configuration(OCPParameters& passedConfig, std::string channelName)=0;
    virtual ~OCP_TL_Config_Listener(){}

};
#endif
