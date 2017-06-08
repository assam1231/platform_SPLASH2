// 
//  (c) Copyright OCP-IP 2004, 2005
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
//  Description : Parameters for an OCP Connection
//                This parameter class contains all of the parameters
//                needed for an OCP connection as per the OCP
//                specification.
//                Its basic usage model is to write values to this class 
//                in the elaboration phase and read these values from 
//                the ocp parameter class during the simulation.
//
// ============================================================================

// This file contains the definition of the class:
// class OCPParameters

#ifndef __OCP_PARAM_LEGACY_H__
#define __OCP_PARAM_LEGACY_H__

typedef std::map<std::string, std::string> MapStringType;

class OCPParameters
{
public:

  // Constructor
  OCPParameters()
  {
      // OCP Standard Configuration Parameter Defaults
      ocp_version=2.0;                // NOTE: currently no way to change this.
      name="unnamed_ocp_channel";     // NOTE: set in "setConfiguration()"
      broadcast_enable=false;         // Is the Broadcast command supported?
      burst_aligned=false;            // Are bursts forced to be aligned by power of two?
      burstseq_dflt1_enable=false;    // Is DFLT1 burst mode enabled?
      burstseq_dflt2_enable=false;    // Is DFLT2 burst mode enabled?
      burstseq_incr_enable=true;      // Are Incrementing Bursts allowed?
      burstseq_strm_enable=false;     // Are Streaming Bursts allowed?
      burstseq_unkn_enable=false;     // Is UNKN burst mode enabled?
      burstseq_wrap_enable=false;     // Is WRAP burst mode enabled?
      burstseq_xor_enable=false;      // Is XOR burst mode enabled?
      burstseq_blck_enable=false;
      endian=1;                       // Endian is an int
      force_aligned=false;            // Forces the byte-enable patterns to be powers of two.
      mthreadbusy_exact=false;        // Slave must use MTheadbusy to send Resp. Master must accept immediately on non-busy threads.
      rdlwrc_enable=false;            
      read_enable=true;               // Is the Read command supported?
      readex_enable=false;            // Is the ReadEx command supported?
      sdatathreadbusy_exact=false;    // Master must use SDataThreadBusy to send new Data. Slave must accept new data immediately on non-busy threads.
      sthreadbusy_exact=false;        // Master must use SThreadBusy to send new Request. Slave must accept new request immediately on non-busy threads.
      write_enable=true;              // Is the Write command supported?
      writenonpost_enable=false;      // Is the WriteNonPost command supported?
      datahandshake=false;            // Is there a separate channel for request data?
      reqdata_together=false;         // Master always puts Request and Data in same cycle. Slave always accepts together in same cycle. 
      writeresp_enable=false;         // Should Responses be sent for write commands?
      addr=true;                      // Is MAddr (Requst Address) part of the OCP?
      addr_wdth=2;                    // Address width must be set by user if addr=true. No default value.
      addrspace=false;                // Is the MAddrSpace signal part of the OCP?
      addrspace_wdth=1;               // No default value. Must be user specificied.
      atomiclength=false;             // Is there a minimum number of transfers to hold together during a burst?
      atomiclength_wdth=1;            // The minimum number. No default value.
      burstlength=false;              // Is there a set number of transfers in a burst.
      burstlength_wdth=1;             // The number of transfers in a burst. User defined. No default value.
      blockheight=false;
      blockheight_wdth=1;
      blockstride=false;
      blockstride_wdth=1;
      burstprecise=false;             // Will the length of a burst be known at the start of the burst?
      burstseq=false;                 // Is there a sequence of addresses in a burst?
      burstsinglereq=false;           // Is a single request allowed to generate multiple data transfers in a burst?
      byteen=false;                   // Is MByteEn part of the OCP?
      cmdaccept=true;                 // Does slave accept commands?
      connid=false;                   // Is MConnID connection identifier pard of the Request group?
      connid_wdth=1;                  // Width of MConnID. User specified. No default.
      dataaccept=false;               // Does slave accept data handshakes?
      datalast=false;                 // Is the MDataLast burst signal part of the OCP? 
      datarowlast=false;
      data_wdth=32;                   // width of MData. User specified. No default.
      mdata=true;                     // is MData part of the OCP?
      mdatabyteen=false;              // is the MDataByteEn signal in the OCP? 
      mdatainfo=false;                // is the MDataInfo signal in the OCP?
      mdatainfo_wdth=1;               // Width of MDataInfo. User specified. no default.
      mdatainfobyte_wdth=1;           // Number of bits of MDataInfo that are associated with each data byte of MData.
      sdatathreadbusy=false;          // Is SDataThreadBusy part of the OCP channel?
      mthreadbusy=false;              // Is MThreadBusy part of the OCP channel?
      reqinfo=false;                  // Is the MReqInfo signal part of the OCP channel?
      reqinfo_wdth=1;                 // Width of MReqInfo. User Specified. No default.
      reqlast=false;                  // Is the MReqLast burst signal part of the OCP channel?
      reqrowlast=false;
      resp=true;                      // Is the SResp signal part of the OCP channel?
      respaccept=false;               // Does the Master accept Responses with the MRespAccept signal?
      respinfo=false;                 // Is the SRespInfo signal on the OCP?
      respinfo_wdth=1;                // Width of SRespInfo. User Specified. No Default.
      resplast=false;                 // Is the SRespLast burst signal part of the OCP channel? 
      resprowlast=false;
      sdata=true;                     // Is the SData signal part of the OCP channel?
      sdatainfo=false;                // Is the SDataInfo signal supported?
      sdatainfo_wdth=1;               // Width of SDataInfo. User Specified. No default.
      sdatainfobyte_wdth=1;           // Number of bits in SDataInfo devoted to each byte of SData. User Specified. No default.
      sthreadbusy=false;              // Is the SThreadBusy signal supported?
      threads=1;                      // Number of threads allowed.
      tags=1;                         // Number of tags allowed. 
      taginorder=0;                   // In order required among tags  
      control=false;                  // Is the Sideband Control signal supported?
      controlbusy=false;              // Is the Sideband ControlBusy signal supported?
      control_wdth=1;                 // Width of ControlBusy. User Specified. No default.
      controlwr=false;                // Is the Sideband ControlWr signal supported?
      interrupt=false;                // Is the Sideband SInterrupt signal supported?
      merror=false;                   // Is the Sideband MError signal supported?
      mflag=false;                    // Is the Sideband MFlag signal supported?
      mflag_wdth=1;                   // Width of MFlag. User Specified. No default.
      mreset=false;                   // NOTE: no default. Is the Sideband MReset signal supported?
      serror=false;                   // Is the Sideband SError signal supported?
      sflag=false;                    // Is the Sideband SFlag signal supported?
      sflag_wdth=1;                   // Width of SFlag. User Specified. No default.
      sreset=false;                   // Note: no default. Is the SReset signal part of the OCP channel?
      status=false;                   // Is the Sideband Status signal supported?
      statusbusy=false;               // Is the Sideband StatusBusy signal supported
      statusrd=false;                 // Is the Sideband StatusRd signal supported?
      status_wdth=1;                  // Width of Status. User Specified. No default.
      tag_interleave_size=1;//default interleave size?
      enableclk=0;//clock enable?      
      sthreadbusy_pipelined=false;
      mthreadbusy_pipelined=false;
      sdatathreadbusy_pipelined=false;
      configured=false;
  }

  // The parameter myParamValue is set only if the parameter is found in the 
  // configuration map and if it is of type 'i' for integer.
  static bool 
  getBoolOCPConfigValue(const std::string& myPrefix, const std::string& myParamName, bool &myParamValue, MapStringType& Map, std::string my_name="") 
  {
      MapStringType::iterator pos = Map.find(myPrefix+myParamName);
      if (pos == Map.end())
      {
          // This parameter not found
          return false;
      } 

//#ifndef NDEBUG
//          std::cout << "Found parameter >" << myParamName << "< and it has value of >" << pos->second << "<." << std::endl;
//#endif

      // Set the parameter
      std::string myValue = pos->second;
      std::istringstream valueStream(myValue);
      char type;
      valueStream >> type;
      if (type != 'i')
      {
          // We were expecting an integer type for this command.
#ifndef NDEBUG 
          std::cout << "WARNING: " << my_name << ": Type 'i' expected for boolean parameter "
		    << myPrefix+myParamName
		    << " but got '" << myValue << "' instead." << std::endl;
#endif
          return false;
      }
      // passed all tests. Set the value.
      // first get rid of the colon
      char shouldBeColon;
      valueStream >> shouldBeColon;
      valueStream >> myParamValue;
// #ifndef NDEBUG
//          std::cout << "Processed parameter >" << myParamName << "< with value field >" << pos->second << "< and found it to have type  >" 
//               << type << "< and value >" << myParamValue << "<." << std::endl;
//#endif
      return true;
  }  

  // The parameter myParamValue is set only if the parameter is found in the 
  // configuration map and if it is of type 'i' for integer.
  static bool 
  getIntOCPConfigValue(const std::string& myPrefix, 
          const std::string& myParamName, 
          int &myParamValue, 
          MapStringType& Map, 
          std::string my_name="")
  {
      MapStringType::iterator pos = Map.find(myPrefix+myParamName);
      if (pos == Map.end())
      {
          // This parameter not found
          return false;
      } 

      // Set the parameter
      std::string myValue = pos->second;
      std::istringstream valueStream(myValue);
      char type;
      valueStream >> type;
      if (type != 'i')
      {
          // We were expecting an integer type for this command.
#ifndef NDEBUG
          std::cout << "WARNING: " << my_name << ": Type 'i' expected for parameter "
              << myPrefix+myParamName
              << " but got '" << myValue << "' instead." << std::endl;
#endif
          return false;
      }
      // passed all tests. Set the value.
      // first get rid of the colon
      char shouldBeColon;
      valueStream >> shouldBeColon;
      valueStream >> myParamValue;
      return true;
  }  

  // The parameter myParamValue is set only if the parameter is found in the 
  // configuration map and if it is of type 's' for string.
  static bool 
  getStringOCPConfigValue(const std::string& myPrefix, 
          const std::string& myParamName, std::string& myParamValue, 
          MapStringType& Map, std::string my_name="") 
  {
      MapStringType::iterator pos = Map.find(myPrefix+myParamName);
      if (pos == Map.end())
      {
          // This parameter not found
          return false;
      } 

//#ifndef NDEBUG
//          std::cout << "Found parameter >" << myParamName << "< and it has value of >" << pos->second << "<." << std::endl;
//#endif

      // Set the parameter
      std::string myValue = pos->second;
      std::istringstream valueStream(myValue);
      char type;
      valueStream >> type;
      if (type != 's')
      {
          // We were expecting a string type for this command.
#ifndef NDEBUG
          std::cout << "WARNING: " << my_name << ": Type 's' expected for string parameter "
              << myPrefix+myParamName
              << " but got '" << myValue << "' instead." << std::endl;
#endif
          return false;
      }
      // passed all tests. Set the value.
      // first get rid of the colon
      char shouldBeColon;
      valueStream >> shouldBeColon;
      valueStream >> myParamValue;
      return true;
  }  

  void 
  setOCPConfiguration(std::string ocp_name, MapStringType& passedMap)
  {
      // ocp_name is passed by value as it will be changed below.

      // Set my Map to the passed map.
      Map = passedMap;

      // Set my name to the passed name.
      name = ocp_name;

      // NOTE: keyPrefix is no longer needed, but kept here in case it changes
      //       again in the future.
      // string keyPrefix = ocp_name;
      std::string keyPrefix = "";

//#ifndef NDEBUG
//      std::cout << "OCP Name is: >" << ocp_name << "<" << std::endl;
//#endif
     
      getBoolOCPConfigValue(keyPrefix, "broadcast_enable", broadcast_enable,Map,name);
      getBoolOCPConfigValue(keyPrefix, "burst_aligned", burst_aligned,Map,name);
      getBoolOCPConfigValue(keyPrefix, "burstseq_dflt1_enable", burstseq_dflt1_enable,Map,name);
      getBoolOCPConfigValue(keyPrefix, "burstseq_dflt2_enable", burstseq_dflt2_enable,Map,name);
      getBoolOCPConfigValue(keyPrefix, "burstseq_incr_enable", burstseq_incr_enable,Map,name);
      getBoolOCPConfigValue(keyPrefix, "burstseq_strm_enable", burstseq_strm_enable,Map,name);
      getBoolOCPConfigValue(keyPrefix, "burstseq_unkn_enable", burstseq_unkn_enable,Map,name);
      getBoolOCPConfigValue(keyPrefix, "burstseq_wrap_enable", burstseq_wrap_enable,Map,name);
      getBoolOCPConfigValue(keyPrefix, "burstseq_xor_enable", burstseq_xor_enable,Map,name);
      getBoolOCPConfigValue(keyPrefix, "burstseq_blck_enable", burstseq_blck_enable, Map, name);
      getIntOCPConfigValue(keyPrefix, "endian", endian,Map,name);
      getBoolOCPConfigValue(keyPrefix, "force_aligned", force_aligned,Map,name);
      getBoolOCPConfigValue(keyPrefix, "mthreadbusy_exact", mthreadbusy_exact,Map,name);
      getBoolOCPConfigValue(keyPrefix, "rdlwrc_enable", rdlwrc_enable,Map,name);
      getBoolOCPConfigValue(keyPrefix, "read_enable", read_enable,Map,name);
      getBoolOCPConfigValue(keyPrefix, "readex_enable", readex_enable,Map,name);
      getBoolOCPConfigValue(keyPrefix, "sdatathreadbusy_exact", sdatathreadbusy_exact,Map,name);
      getBoolOCPConfigValue(keyPrefix, "sthreadbusy_exact", sthreadbusy_exact,Map,name);
      getBoolOCPConfigValue(keyPrefix, "write_enable", write_enable,Map,name);
      getBoolOCPConfigValue(keyPrefix, "writenonpost_enable", writenonpost_enable,Map,name);
      getBoolOCPConfigValue(keyPrefix, "datahandshake", datahandshake,Map,name);
      getBoolOCPConfigValue(keyPrefix, "reqdata_together", reqdata_together,Map,name);
      getBoolOCPConfigValue(keyPrefix, "writeresp_enable", writeresp_enable,Map,name);
      getBoolOCPConfigValue(keyPrefix, "addr", addr,Map,name);
      getIntOCPConfigValue(keyPrefix, "addr_wdth", addr_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "addrspace", addrspace,Map,name);
      getIntOCPConfigValue(keyPrefix, "addrspace_wdth", addrspace_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "atomiclength", atomiclength,Map,name);
      getIntOCPConfigValue(keyPrefix, "atomiclength_wdth", atomiclength_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "burstlength", burstlength,Map,name);
      getIntOCPConfigValue(keyPrefix, "burstlength_wdth", burstlength_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "blockheight", blockheight,Map,name);
      getIntOCPConfigValue(keyPrefix, "blockheight_wdth", blockheight_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "blockstride", blockstride,Map,name);
      getIntOCPConfigValue(keyPrefix, "blockstride_wdth", blockstride_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "burstprecise", burstprecise,Map,name);
      getBoolOCPConfigValue(keyPrefix, "burstseq", burstseq,Map,name);
      getBoolOCPConfigValue(keyPrefix, "burstsinglereq", burstsinglereq,Map,name);
      getBoolOCPConfigValue(keyPrefix, "byteen", byteen,Map,name);
      getBoolOCPConfigValue(keyPrefix, "cmdaccept", cmdaccept,Map,name);
      getBoolOCPConfigValue(keyPrefix, "connid", connid,Map,name);
      getIntOCPConfigValue(keyPrefix, "connid_wdth", connid_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "dataaccept", dataaccept,Map,name);
      getBoolOCPConfigValue(keyPrefix, "datalast", datalast,Map,name);
      getBoolOCPConfigValue(keyPrefix, "datarowlast", datarowlast,Map,name);
      getIntOCPConfigValue(keyPrefix, "data_wdth", data_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "mdata", mdata,Map,name);
      getBoolOCPConfigValue(keyPrefix, "mdatabyteen", mdatabyteen,Map,name);
      getBoolOCPConfigValue(keyPrefix, "mdatainfo", mdatainfo,Map,name);
      getIntOCPConfigValue(keyPrefix, "mdatainfo_wdth", mdatainfo_wdth,Map,name);
      getIntOCPConfigValue(keyPrefix, "mdatainfobyte_wdth", mdatainfobyte_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "sdatathreadbusy", sdatathreadbusy,Map,name);
      getBoolOCPConfigValue(keyPrefix, "mthreadbusy", mthreadbusy,Map,name);
      getBoolOCPConfigValue(keyPrefix, "reqinfo", reqinfo,Map,name);
      getIntOCPConfigValue(keyPrefix, "reqinfo_wdth", reqinfo_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "reqlast", reqlast,Map,name);
      getBoolOCPConfigValue(keyPrefix, "reqrowlast", reqrowlast,Map,name);
      getBoolOCPConfigValue(keyPrefix, "resp", resp,Map,name);
      getBoolOCPConfigValue(keyPrefix, "respaccept", respaccept,Map,name);
      getBoolOCPConfigValue(keyPrefix, "respinfo", respinfo,Map,name);
      getIntOCPConfigValue(keyPrefix, "respinfo_wdth", respinfo_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "resplast", resplast,Map,name);
      getBoolOCPConfigValue(keyPrefix, "resprowlast", resprowlast,Map,name);
      getBoolOCPConfigValue(keyPrefix, "sdata", sdata,Map,name);
      getBoolOCPConfigValue(keyPrefix, "sdatainfo", sdatainfo,Map,name);
      getIntOCPConfigValue(keyPrefix, "sdatainfo_wdth", sdatainfo_wdth,Map,name);
      getIntOCPConfigValue(keyPrefix, "sdatainfobyte_wdth", sdatainfobyte_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "sthreadbusy", sthreadbusy,Map,name);
      getIntOCPConfigValue(keyPrefix, "threads", threads,Map,name);
      getIntOCPConfigValue(keyPrefix, "tags", tags,Map,name);
      getBoolOCPConfigValue(keyPrefix, "taginorder", taginorder,Map,name);
      getBoolOCPConfigValue(keyPrefix, "control", control,Map,name);
      getBoolOCPConfigValue(keyPrefix, "controlbusy", controlbusy,Map,name);
      getIntOCPConfigValue(keyPrefix, "control_wdth", control_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "controlwr", controlwr,Map,name);
      getBoolOCPConfigValue(keyPrefix, "interrupt", interrupt,Map,name);
      getBoolOCPConfigValue(keyPrefix, "merror", merror,Map,name);
      getBoolOCPConfigValue(keyPrefix, "mflag", mflag,Map,name);
      getIntOCPConfigValue(keyPrefix, "mflag_wdth", mflag_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "mreset", mreset,Map,name);
      getBoolOCPConfigValue(keyPrefix, "serror", serror,Map,name);
      getBoolOCPConfigValue(keyPrefix, "sflag", sflag,Map,name);
      getIntOCPConfigValue(keyPrefix, "sflag_wdth", sflag_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "sreset", sreset,Map,name);
      getBoolOCPConfigValue(keyPrefix, "status", status,Map,name);
      getBoolOCPConfigValue(keyPrefix, "statusbusy", statusbusy,Map,name);
      getBoolOCPConfigValue(keyPrefix, "statusrd", statusrd,Map,name);
      getIntOCPConfigValue(keyPrefix, "status_wdth", status_wdth,Map,name);
      getBoolOCPConfigValue(keyPrefix, "sthreadbusy_pipelined", sthreadbusy_pipelined, Map, name);
      getBoolOCPConfigValue(keyPrefix, "sdatathreadbusy_pipelined", sdatathreadbusy_pipelined, Map, name);
      getBoolOCPConfigValue(keyPrefix, "mthreadbusy_pipelined", mthreadbusy_pipelined, Map, name);   
      getIntOCPConfigValue(keyPrefix, "tag_interleave_size", tag_interleave_size,Map,name);
      getIntOCPConfigValue(keyPrefix, "enableclk", enableclk,Map,name);
         
  }

  void dumpConfiguration(void) {
      std::cout << std::endl << "Parameter map for OCP channel " << name << ":" << std::endl;
      MapStringType::iterator map_it;
      for (map_it = Map.begin(); map_it != Map.end(); ++map_it) {
          std::cout << "map[" << map_it->first << "] = " << map_it->second
               << std::endl;
      }
  }

  // OCP parameters
  float ocp_version; // NOTE: currently set is code above - not parametized.
  std::string name; 
  bool broadcast_enable;
  bool burst_aligned;
  bool burstseq_dflt1_enable;
  bool burstseq_dflt2_enable;
  bool burstseq_incr_enable;
  bool burstseq_strm_enable;
  bool burstseq_unkn_enable;
  bool burstseq_wrap_enable;
  bool burstseq_xor_enable;
  bool burstseq_blck_enable;
  int endian;
  bool force_aligned;
  bool mthreadbusy_exact;
  bool rdlwrc_enable;
  bool read_enable;
  bool readex_enable;
  bool sdatathreadbusy_exact;
  bool sthreadbusy_exact;
  bool write_enable;
  bool writenonpost_enable;
  bool datahandshake;
  bool reqdata_together;
  bool writeresp_enable;
  bool addr;
  int addr_wdth;
  bool addrspace;
  int addrspace_wdth;
  bool atomiclength;
  int atomiclength_wdth;
  bool burstlength;
  int burstlength_wdth;
  bool blockheight;
  int blockheight_wdth;
  bool blockstride;
  int blockstride_wdth;
  bool burstprecise;
  bool burstseq;
  bool burstsinglereq;
  bool byteen;
  bool cmdaccept;
  bool connid;
  int connid_wdth;
  bool dataaccept;
  bool datalast;
  bool datarowlast;
  int data_wdth;
  bool mdata;
  bool mdatabyteen;
  bool mdatainfo;
  int mdatainfo_wdth;
  int mdatainfobyte_wdth;
  bool sdatathreadbusy;
  bool mthreadbusy;
  bool reqinfo;
  int reqinfo_wdth;
  bool reqlast;
  bool reqrowlast;
  bool resp;
  bool respaccept;
  bool respinfo;
  int respinfo_wdth;
  bool resplast;
  bool resprowlast;
  bool sdata;
  bool sdatainfo;
  int sdatainfo_wdth;
  int sdatainfobyte_wdth;
  bool sthreadbusy;
  int threads;
  int tags;
  bool taginorder;
  bool control;
  bool controlbusy;
  int control_wdth;
  bool controlwr;
  bool interrupt;
  bool merror;
  bool mflag;
  int mflag_wdth;
  bool mreset;
  bool serror;
  bool sflag;
  int sflag_wdth;
  bool sreset;
  bool status;
  bool statusbusy;
  bool statusrd;
  int status_wdth;

  bool sthreadbusy_pipelined;
  bool mthreadbusy_pipelined;
  bool sdatathreadbusy_pipelined;

  int tag_interleave_size;
  int enableclk;
  bool configured;

  MapStringType Map;

};


#endif
