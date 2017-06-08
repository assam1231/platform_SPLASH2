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
//  Description :  Configuration manager that enables master ande slave
//                 modules to configure the channels and that performs
//                 configuration compliance checks
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef __TL_CONFIG_MANAGER_LEGACY_H__
#define __TL_CONFIG_MANAGER_LEGACY_H__

class Config_manager {

public:

  inline Config_manager(const char* name, OCPParameters& _params);
  inline ~Config_manager();
  
  inline void setExternalConfiguration(MapStringType& _map);
  inline void setMasterConfiguration(MapStringType& _map);
  inline void setSlaveConfiguration(MapStringType& _map);
  inline void addConfigListener(OCP_TL_Config_Listener& _Listener);

private:

  const char* m_name;  //name of connected channel
  OCPParameters* m_params;  //pointer to the parameter class of the connected channel
  OCP_TL_Config_Listener* m_pListener1;  //pointer to first module that registers at the channel
  OCP_TL_Config_Listener* m_pListener2;  //pointer to second module that registers at the channel
  bool m_masterConfig, m_slaveConfig, m_externalConfig;  //bools indicating whether master, slave or topLevel configured the channel
  
  bool m_mismatch;  //used inside map compliance check, may vanish when more sophisticated checks are implemented

  inline void doCallBacks();  //performs callbacks to the registered listeners
  inline void checkMapCompliance(MapStringType& _map, bool _mstNsl);  //checks whether the two temporary maps match or not
  inline void setChannelConfig(MapStringType& _map); //sets the channels config accoding to the given map

  inline bool compareParams(int master_param, int slave_param, int& final);  //helper methods to do paramClass member checks
  inline bool compareParams(bool master_param, bool slave_param, bool& final);
  inline bool compareParams(float master_param, float slave_param, float& final);

public:
  inline static void rebuildMap(OCPParameters paramClass, MapStringType& targetmap); //builds a MapStringType map out of a given paramClass  
  //let the compiler do the type checking for basic types
  inline static char getTypeAbbreviation(int value);
  inline static char getTypeAbbreviation(bool value);
  inline static char getTypeAbbreviation(float value);
  inline static char getTypeAbbreviation(std::string value);

};

#define COMPARE(value) \
  compareParams(pMasterParam->value, pSlaveParam->value, finalMap.value)
  
#define FATAL(value) \
  std::cout<<"ERROR: FATAL mismatch in parameter: "<<#value<<std::endl<<"       master's configuration specifies: "<<pMasterParam->value<<std::endl \
  <<"       slave's configuration specifies: "<<pSlaveParam->value<<std::endl<<std::flush; \
  m_mismatch=true;

#define CRITICAL(value) \
  std::cout<<"WARNING: CRITICAL mismatch in parameter: "<<#value<<std::endl<<"         master's configuration specifies: "<<pMasterParam->value<<std::endl \
  <<"         slave's configuration specifies: "<<pSlaveParam->value<<std::endl<<std::flush;

#define NONCRITICAL(value) \
  std::cout<<"WARNING: NONCRITICAL mismatch in parameter: "<<#value<<std::endl<<"         master's configuration specifies: "<<pMasterParam->value<<std::endl \
  <<"         slave's configuration specifies: "<<pSlaveParam->value<<std::endl<<std::flush;

//we do only check if its a float value, otherwise we assume it being bool or int (both have prefix i)
//in case there will be strings in the future (like ocp_version), we'll have to add that
#define ADD2TARGETMAP(value) \
{ \
  std::ostringstream tmp;					     \
  tmp<<getTypeAbbreviation(paramClass.value)<<":"<<paramClass.value; \
  targetmap[#value]=tmp.str().c_str(); \
}

Config_manager::Config_manager(const char* name, OCPParameters& _params) : 
  m_name(name),
  m_params(&_params),
  m_pListener1(NULL),
  m_pListener2(NULL),
  m_masterConfig(false),
  m_slaveConfig(false),
  m_externalConfig(false),
  m_mismatch(false)
{
}

Config_manager::~Config_manager(){}

void Config_manager::setChannelConfig(MapStringType& _map){
  //we have to build a complete map before passing it to the channel, to make sure we overwrite values which might not be explicity contained
  //in the given map
  OCPParameters tmp;
  tmp.setOCPConfiguration(m_name, _map);
  MapStringType fullMap;
  rebuildMap(tmp, fullMap);
  m_params->setOCPConfiguration(m_name, fullMap);
  m_params->configured=true;
}

void Config_manager::setExternalConfiguration(MapStringType& _map){
  if (m_masterConfig | m_slaveConfig){
    std::cout<<"WARNING: external configuration passed to channel "<<m_name
      <<" after at least one of the connected modules configured the channel."<<std::endl
      <<"         Therefore the EXTERNAL configuration will be IGNORED."<<std::endl<<std::flush;
    return;
  }
  if (m_externalConfig) {
    std::cout<<"WARNING: external configuration passed to channel "<<m_name
      <<" after a previous external configuration."<<std::endl
      <<"         Therefore the preceding EXTERNAL configuration will be OVERRIDDEN."<<std::endl<<std::flush;
  }
  setChannelConfig(_map); 
  m_externalConfig=true;
  doCallBacks();
}

void Config_manager::setMasterConfiguration(MapStringType& _map){
  if(m_masterConfig){
    std::cout<<"WARNING: the master passed a configuration to channel "<<m_name
      <<" after a previous master configuration."<<std::endl
      <<"         Therefore the preceding MASTER configuration will be OVERRIDDEN."<<std::endl<<std::flush;    
  }
  if(m_externalConfig){
    std::cout<<"WARNING: the master passed a configuration to channel "<<m_name
      <<" after a previous external configuration."<<std::endl
      <<"         Therefore the EXTERNAL configuration will be OVERRIDDEN."<<std::endl<<std::flush;  
  }
  m_externalConfig=false;
  m_masterConfig=true;
  if (!m_slaveConfig) { //no slave config in place so the master config is directly used as config
    setChannelConfig(_map);
  }
  else { //if there is already a slave config in place, we have to compare
    checkMapCompliance(_map, true);
  }
  doCallBacks();
}

void Config_manager::setSlaveConfiguration(MapStringType& _map){
  if(m_slaveConfig){
    std::cout<<"WARNING: the slave passed a configuration to channel "<<m_name
      <<" after a previous slave configuration."<<std::endl
      <<"         Therefore the preceding SLAVE configuration will be OVERRIDDEN."<<std::endl<<std::flush;    
  }
  if(m_externalConfig){
    std::cout<<"WARNING: the slave passed a configuration to channel "<<m_name
      <<" after a previous external configuration."<<std::endl
      <<"         Therefore the EXTERNAL configuration will be OVERRIDDEN."<<std::endl<<std::flush;    
  }
  m_slaveConfig=true;
  m_externalConfig=false;
  if (!m_masterConfig) {
    setChannelConfig(_map);
  }
  else {
    checkMapCompliance(_map, false);
  }
  doCallBacks();
}
  
void Config_manager::addConfigListener(OCP_TL_Config_Listener& _Listener){
  if(&_Listener==m_pListener1 | &_Listener==m_pListener2){
    std::cout<<"WARNING: A configuration listener registered itself at channel "<<m_name
      <<" but registered itself before."<<std::endl<<std::flush;
    return;
  }
  if(m_pListener1!=NULL && m_pListener2!=NULL) {
    std::cout<<"ERROR: It seems that a THIRD module registered itself as a configuration listener at channel "<<m_name<<std::endl
      <<"       This is absolutely not expected and will be treated as FATAL."<<std::endl<<std::flush;
    abort();
  }
  if(m_pListener1==NULL){
    m_pListener1=&_Listener;
  }
  else{
    m_pListener2=&_Listener;
  }
  doCallBacks();
}

void Config_manager::doCallBacks(){
    if(m_pListener1!=NULL)
      m_pListener1->set_configuration(*m_params, std::string(m_name));
    if(m_pListener2!=NULL)
      m_pListener2->set_configuration(*m_params, std::string(m_name));
}


bool Config_manager::compareParams(int master_param, int slave_param, int& final){
  if(master_param!=slave_param){
    return false;
  }
  final=master_param;
  return true;
}

bool Config_manager::compareParams(bool master_param, bool slave_param, bool& final){
  if(master_param!=slave_param){
    return false;
  }
  final=master_param;
  return true;
}

bool Config_manager::compareParams(float master_param, float slave_param, float& final){
  if(master_param!=slave_param){
    return false;
  }
  final=master_param;
  return true;
}

void Config_manager::rebuildMap(OCPParameters paramClass, MapStringType& targetmap){

  ADD2TARGETMAP(ocp_version);
  ADD2TARGETMAP(broadcast_enable);
  ADD2TARGETMAP(burst_aligned);
  ADD2TARGETMAP(burstseq_dflt1_enable);
  ADD2TARGETMAP(burstseq_dflt2_enable);
  ADD2TARGETMAP(burstseq_incr_enable);
  ADD2TARGETMAP(burstseq_strm_enable);
  ADD2TARGETMAP(burstseq_unkn_enable);
  ADD2TARGETMAP(burstseq_wrap_enable);
  ADD2TARGETMAP(burstseq_xor_enable);
  ADD2TARGETMAP(burstseq_blck_enable);
  ADD2TARGETMAP(endian);
  ADD2TARGETMAP(force_aligned);
  ADD2TARGETMAP(mthreadbusy_exact);
  ADD2TARGETMAP(rdlwrc_enable);
  ADD2TARGETMAP(read_enable);
  ADD2TARGETMAP(readex_enable);
  ADD2TARGETMAP(sdatathreadbusy_exact);
  ADD2TARGETMAP(sthreadbusy_exact);
  ADD2TARGETMAP(write_enable);
  ADD2TARGETMAP(writenonpost_enable);
  ADD2TARGETMAP(datahandshake);
  ADD2TARGETMAP(reqdata_together);
  ADD2TARGETMAP(writeresp_enable);
  ADD2TARGETMAP(addr);
  ADD2TARGETMAP(addr_wdth);
  ADD2TARGETMAP(addrspace);
  ADD2TARGETMAP(addrspace_wdth);
  ADD2TARGETMAP(atomiclength);
  ADD2TARGETMAP(atomiclength_wdth);
  ADD2TARGETMAP(burstlength);
  ADD2TARGETMAP(burstlength_wdth);
  ADD2TARGETMAP(blockheight);
  ADD2TARGETMAP(blockheight_wdth);
  ADD2TARGETMAP(blockstride);
  ADD2TARGETMAP(blockstride_wdth);
  ADD2TARGETMAP(burstprecise);
  ADD2TARGETMAP(burstseq);
  ADD2TARGETMAP(burstsinglereq);
  ADD2TARGETMAP(byteen);
  ADD2TARGETMAP(cmdaccept);
  ADD2TARGETMAP(connid);
  ADD2TARGETMAP(connid_wdth);
  ADD2TARGETMAP(dataaccept);
  ADD2TARGETMAP(datalast);
  ADD2TARGETMAP(datarowlast);  
  ADD2TARGETMAP(data_wdth);
  ADD2TARGETMAP(mdata);
  ADD2TARGETMAP(mdatabyteen);
  ADD2TARGETMAP(mdatainfo);
  ADD2TARGETMAP(mdatainfo_wdth);
  ADD2TARGETMAP(mdatainfobyte_wdth);
  ADD2TARGETMAP(sdatathreadbusy);
  ADD2TARGETMAP(mthreadbusy);
  ADD2TARGETMAP(reqinfo);
  ADD2TARGETMAP(reqinfo_wdth);
  ADD2TARGETMAP(reqlast);
  ADD2TARGETMAP(reqrowlast);
  ADD2TARGETMAP(resp);
  ADD2TARGETMAP(respaccept);
  ADD2TARGETMAP(respinfo);
  ADD2TARGETMAP(respinfo_wdth);
  ADD2TARGETMAP(resplast);
  ADD2TARGETMAP(resprowlast);  
  ADD2TARGETMAP(sdata);
  ADD2TARGETMAP(sdatainfo);
  ADD2TARGETMAP(sdatainfo_wdth);
  ADD2TARGETMAP(sdatainfobyte_wdth);
  ADD2TARGETMAP(sthreadbusy);
  ADD2TARGETMAP(threads);
  ADD2TARGETMAP(tags);
  ADD2TARGETMAP(taginorder);
  ADD2TARGETMAP(control);
  ADD2TARGETMAP(controlbusy);
  ADD2TARGETMAP(control_wdth);
  ADD2TARGETMAP(controlwr);
  ADD2TARGETMAP(interrupt);
  ADD2TARGETMAP(merror);
  ADD2TARGETMAP(mflag);
  ADD2TARGETMAP(mflag_wdth);
  ADD2TARGETMAP(mreset);
  ADD2TARGETMAP(serror);
  ADD2TARGETMAP(sflag);
  ADD2TARGETMAP(sflag_wdth);
  ADD2TARGETMAP(sreset);
  ADD2TARGETMAP(status);
  ADD2TARGETMAP(statusbusy);
  ADD2TARGETMAP(statusrd);
  ADD2TARGETMAP(status_wdth);
  ADD2TARGETMAP(sthreadbusy_pipelined);
  ADD2TARGETMAP(sdatathreadbusy_pipelined);
  ADD2TARGETMAP(mthreadbusy_pipelined);
  ADD2TARGETMAP(enableclk);
  ADD2TARGETMAP(tag_interleave_size);
  
}

//just a makro to save some space and time..
#define MASTERMUSTNOT(message) \
std::cout<<"         The Master must not (under no circumstances) "<<message<<", because the slave cannot handle that."<<std::endl<<std::flush;

#define MASTERHASTO(message)\
std::cout<<"         The Master has to (under all circumstances) " <<message<<", because the slave requires that."<<std::endl<<std::flush;

#define SLAVEWILL(message)\
std::cout<<"         The Slave will " <<message<<", keep that in mind."<<std::endl<<std::flush;

#define MASTERWILL(message)\
std::cout<<"         The Master will " <<message<<", keep that in mind."<<std::endl<<std::flush;

void Config_manager::checkMapCompliance(MapStringType& _map, bool _mstNsl){
  //the idea: we create a ParamCl from each Map and compare those, because the maps may differ but still describe the same
  //config because one map may set a value to its default and the other doesn't
  OCPParameters* pMasterParam;
  OCPParameters* pSlaveParam; 
  OCPParameters finalMap;
  if (_mstNsl) {
    pMasterParam=new OCPParameters();
    pMasterParam->setOCPConfiguration(m_name, _map);
    pSlaveParam=new OCPParameters(*m_params);
  }
  else {
    pMasterParam=new OCPParameters(*m_params);
    pSlaveParam=new OCPParameters();
    pSlaveParam->setOCPConfiguration(m_name, _map);
  }
  //now that we converted the two maps into comparable structures, it gets dirty because we have to check EACH (!) single element of
  //the ParamCl to exactly identify mismatches (and we do need this information to perform tie-offs)

  if (!COMPARE(ocp_version)) {
    FATAL(ocp_version);
  }
  if (!COMPARE(broadcast_enable)) {
    if (pMasterParam->broadcast_enable){
      CRITICAL(broadcast_enable);
      MASTERMUSTNOT("issue a broadcast transfer");
    }
    finalMap.broadcast_enable=false;  //in case of mismatch no broadcasts will take place
  }
  if (!COMPARE(burst_aligned)){
    if (pSlaveParam->burst_aligned){
      CRITICAL(burst_aligned);
      MASTERHASTO("keep INCR bursts aligned");
    }
    finalMap.burst_aligned=true;  //in case of mismatch all INCR burst will be aligned
  }
  if (!COMPARE(burstseq_dflt1_enable)){
    if (pMasterParam->burstseq_dflt1_enable) {
      CRITICAL(burstseq_dflt1_enable);
      MASTERMUSTNOT("use the DFLT1 burst sequence");
    }
    finalMap.burstseq_dflt1_enable=false;  //in case of mismatch no DFLT1 bursts will take place
  }
  if (!COMPARE(burstseq_dflt2_enable)){
    if (pMasterParam->burstseq_dflt2_enable) {
      CRITICAL(burstseq_dflt2_enable);
      MASTERMUSTNOT("use the DFLT2 burst sequence");
    }
    finalMap.burstseq_dflt2_enable=false;  //in case of mismatch no DFLT2 bursts will take place
  }
  if (!COMPARE(burstseq_incr_enable)){
    if (pMasterParam->burstseq_incr_enable) {
      CRITICAL(burstseq_incr_enable);
      MASTERMUSTNOT("use the INCR burst sequence");
    }
    finalMap.burstseq_incr_enable=false;  //in case of mismatch no INCR bursts will take place
  }
  if (!COMPARE(burstseq_strm_enable)){
    if (pMasterParam->burstseq_strm_enable) {
      CRITICAL(burstseq_strm_enable);
      MASTERMUSTNOT("use the STRM burst sequence");
    }
    finalMap.burstseq_strm_enable=false;  //in case of mismatch no STRM bursts will take place
  }
  if (!COMPARE(burstseq_unkn_enable)){
    if (pMasterParam->burstseq_unkn_enable) {
      CRITICAL(burstseq_unkn_enable);
      MASTERMUSTNOT("use the UNKN burst sequence");
    }
    finalMap.burstseq_unkn_enable=false;  //in case of mismatch no UKNK bursts will take place
  }
  if (!COMPARE(burstseq_wrap_enable)){
    if (pMasterParam->burstseq_wrap_enable) {
      CRITICAL(burstseq_wrap_enable);
      MASTERMUSTNOT("use the WRAP burst sequence");
    }
    finalMap.burstseq_wrap_enable=false;  //in case of mismatch no WRAP bursts will take place
  }
  if (!COMPARE(burstseq_xor_enable)){
    if (pMasterParam->burstseq_xor_enable) {
      CRITICAL(burstseq_xor_enable);
      MASTERMUSTNOT("use the XOR burst sequence");
    }
    finalMap.burstseq_xor_enable=false;  //in case of mismatch no XOR bursts will take place
  }
  if (!COMPARE(burstseq_blck_enable)){
    if (pMasterParam->burstseq_blck_enable) {
      CRITICAL(burstseq_blck_enable);
      MASTERMUSTNOT("use the BLCK burst sequence");
    }
    finalMap.burstseq_blck_enable=false;  //in case of mismatch no BLCK bursts will take place
  }
  if (!COMPARE(endian)){
    NONCRITICAL(endian);
    std::cout<<"        Just be careful there..."<<std::endl<<std::flush;
    finalMap.endian=pMasterParam->endian;  //we have to decide
  }
  if (!COMPARE(force_aligned)){
    if (pSlaveParam->force_aligned){
      CRITICAL(force_aligned);
      MASTERHASTO("keep byte enable patterns aligned");
    }
    finalMap.force_aligned=true;  //in case of mismatch all byte enables must be aligned
  }
  if (!COMPARE(mthreadbusy_exact)){
    if(pMasterParam->mthreadbusy | pSlaveParam->mthreadbusy) { //if any module is using mthreadbusy the exact values must match
      FATAL(mthreadbusy_exact);
    }
    else
      finalMap.mthreadbusy_exact=false; //if no module is using mthreadbusy the exact parameter may be false as well.
  }
  if (!COMPARE(rdlwrc_enable)){
    if(pMasterParam->rdlwrc_enable){
      CRITICAL(rdlwrc_enable);
      MASTERMUSTNOT("use a ReadLinked or a WriteCondintional command");
    }
    finalMap.rdlwrc_enable=false; //in case of mismatch no RDLWRC commands may take place
  }
  if (!COMPARE(read_enable)){
    if(pMasterParam->read_enable){
      CRITICAL(read_enable);
      MASTERMUSTNOT("use a read command");
    }
    finalMap.read_enable=false; //in case of mismatch no READ transfers are allowed
  }
  if (!COMPARE(readex_enable)){
    if(pMasterParam->readex_enable){
      CRITICAL(readex_enable);
      MASTERMUSTNOT("use a readEx command");
    }
    finalMap.readex_enable=false; //in case if mismatch no READEX transfers are allowed
  }
  if (!COMPARE(sdatathreadbusy_exact)){
    if(pMasterParam->sdatathreadbusy | pSlaveParam->sdatathreadbusy){
      FATAL(sdatathreadbusy_exact);
    }
    else
      finalMap.sdatathreadbusy_exact=false;
  }
  if (!COMPARE(sthreadbusy_exact)){
    if(pMasterParam->sthreadbusy | pSlaveParam->sthreadbusy){
      FATAL(sthreadbusy_exact);
    }
    else
      finalMap.sthreadbusy_exact=false;
  }
  if (!COMPARE(write_enable)){
    if(pMasterParam->write_enable){
      CRITICAL(write_enable);
      MASTERMUSTNOT("use a write command");
    }
    finalMap.write_enable=false;
  }
  if (!COMPARE(writenonpost_enable)){
    if(pMasterParam->writenonpost_enable){
      CRITICAL(writenonpost_enable);
      MASTERMUSTNOT("use a writeNonPost command");
    }
    finalMap.writenonpost_enable=false;
  }
  if (!COMPARE(datahandshake)){
    FATAL(datahandshake);
  }
  if (!COMPARE(reqdata_together)){
    FATAL(reqdata_together);
  }
  if (!COMPARE(writeresp_enable)){
    FATAL(writeresp_enable);
  }
  if (!COMPARE(addr)){
    if (pMasterParam->addr){
      CRITICAL(addr);
      SLAVEWILL("ignore the address field inside the requests from the master");
    }
    else {
      NONCRITICAL(addr);
      MASTERWILL("not set any address, so that all transfer will go to address 0");
    }
    finalMap.addr=false;
  }
  if(!COMPARE(addr_wdth) && finalMap.addr){
    if (pMasterParam->addr_wdth>pSlaveParam->addr_wdth){
      CRITICAL(addr_wdth);
      SLAVEWILL("ignore the top most "<< (pMasterParam->addr_wdth-pSlaveParam->addr_wdth) << "bits of the address field");
      finalMap.addr_wdth=pSlaveParam->addr_wdth;
    }
    else {
      NONCRITICAL(addr_wdth);
      MASTERWILL("not set the top most "<< (pSlaveParam->addr_wdth-pMasterParam->addr_wdth) << "bits of the address field");
      finalMap.addr_wdth=pMasterParam->addr_wdth;
    }
  }
  if (!COMPARE(addrspace)){
    if (pMasterParam->addrspace){
      CRITICAL(addrspace);
      SLAVEWILL("ignore the address space field inside the requests from the master");
    }
    else {
      NONCRITICAL(addrspace);
      MASTERWILL("not set any address space field, so that all transfer will go to address space 0");
    }
    finalMap.addrspace=false;
  }
  if(!COMPARE(addrspace_wdth) && finalMap.addrspace){
    if (pMasterParam->addrspace_wdth>pSlaveParam->addrspace_wdth){
      CRITICAL(addrspace_wdth);
      SLAVEWILL("ignore the top most "<< (pMasterParam->addrspace_wdth-pSlaveParam->addrspace_wdth) << "bits of the address space field");
      finalMap.addrspace_wdth=pSlaveParam->addrspace_wdth;
    }
    else {
      NONCRITICAL(addrspace_wdth);
      MASTERWILL("not set the top most "<< (pSlaveParam->addrspace_wdth-pMasterParam->addrspace_wdth) << "bits of the address space field");
      finalMap.addrspace_wdth=pMasterParam->addrspace_wdth;      
    }
  }  
  if (!COMPARE(atomiclength)){
    if (pMasterParam->atomiclength){
      CRITICAL(atomiclength);
      SLAVEWILL("ignore the atomic length field inside the requests from the master");
    }
    else {
      NONCRITICAL(atomiclength);
      MASTERWILL("not set atomic length, so that all transfer will have an atomic length of 1");
    }
    finalMap.atomiclength=false;
  }
  if(!COMPARE(atomiclength_wdth) && finalMap.atomiclength){
    if (pMasterParam->atomiclength_wdth>pSlaveParam->atomiclength_wdth){
      CRITICAL(atomiclength_wdth);
      SLAVEWILL("ignore the top most "<< (pMasterParam->atomiclength_wdth-pSlaveParam->atomiclength_wdth) << "bits of the atomic length field");
      finalMap.atomiclength_wdth=pSlaveParam->atomiclength_wdth;
    }
    else {
      //MASTERWILL("not set the top most "<< (pSlaveParam->atomiclength_wdth-pMasterParam->atomiclength_wdth) << "bits of the atomic length field");
      finalMap.atomiclength_wdth=pMasterParam->atomiclength_wdth;      
    }
  }   
  if (!COMPARE(burstlength)){
    if (pMasterParam->burstlength){
      CRITICAL(burstlength);
      SLAVEWILL("ignore the burst length field inside the requests from the master");
    }
    else {
      //MASTERWILL("not set burst length, so that all transfer will have a burst length of 1");
    }
    finalMap.burstlength=false;
  }
  if(!COMPARE(burstlength_wdth) && finalMap.burstlength){
    if (pMasterParam->burstlength_wdth>pSlaveParam->burstlength_wdth){
      CRITICAL(burstlength_wdth);
      SLAVEWILL("ignore the top most "<< (pMasterParam->burstlength_wdth-pSlaveParam->burstlength_wdth) << "bits of the burst length field");
      finalMap.burstlength_wdth=pSlaveParam->burstlength_wdth;
    }
    else {
      //MASTERWILL("not set the top most "<< (pSlaveParam->burstlength_wdth-pMasterParam->burstlength_wdth) << "bits of the burst length field");
      finalMap.burstlength_wdth=pMasterParam->burstlength_wdth;      
    }
  }
  if (!COMPARE(blockheight)){
    if (pMasterParam->blockheight){
      CRITICAL(blockheight);
      SLAVEWILL("ignore the block height field inside the requests from the master");
    }
    finalMap.blockheight=false;
  }
  if(!COMPARE(blockheight_wdth) && finalMap.blockheight){
    if (pMasterParam->blockheight_wdth>pSlaveParam->blockheight_wdth){
      CRITICAL(blockheight_wdth);
      SLAVEWILL("ignore the top most "<< (pMasterParam->blockheight_wdth-pSlaveParam->blockheight_wdth) << "bits of the block height field");
      finalMap.blockheight_wdth=pSlaveParam->blockheight_wdth;
    }
    else {
      finalMap.blockheight_wdth=pMasterParam->blockheight_wdth;      
    }
  }
  if (!COMPARE(blockstride)){
    if (pMasterParam->blockstride){
      CRITICAL(blockstride);
      SLAVEWILL("ignore the block stride field inside the requests from the master");
    }
    finalMap.blockstride=false;
  }
  if(!COMPARE(blockstride_wdth) && finalMap.blockstride){
    if (pMasterParam->blockstride_wdth>pSlaveParam->blockstride_wdth){
      CRITICAL(blockstride_wdth);
      SLAVEWILL("ignore the top most "<< (pMasterParam->blockstride_wdth-pSlaveParam->blockstride_wdth) << "bits of the block stride field");
      finalMap.blockstride_wdth=pSlaveParam->blockstride_wdth;
    }
    else {
      finalMap.blockstride_wdth=pMasterParam->blockstride_wdth;      
    }
  }
  if (!COMPARE(burstprecise)){
    if (pMasterParam->burstprecise){
      CRITICAL(burstprecise);    
      SLAVEWILL("ignore the burst precise field inside the requests from the master");
    }
    else {
      //MASTERWILL("not set burst precise, so that all transfer will be concidered precise");   
    }
    finalMap.burstprecise=true;       
  }  
  if (!COMPARE(burstseq)){
    if (pMasterParam->burstseq){
      CRITICAL(burstseq);
      SLAVEWILL("ignore the burst precise field inside the requests from the master (and concider them INCR)");
      finalMap.burstseq=false;
    }
    else {
      //MASTERWILL("not set burst precise, so that all transfer will be concidered INCR bursts");
      finalMap.burstseq=false;      
    }
  }
  if (!COMPARE(burstsinglereq)){
    if (pMasterParam->burstsinglereq){
      CRITICAL(burstsinglereq);
      SLAVEWILL("ignore the burst single req field inside the requests from the master (and concider it being 0)");
      finalMap.burstsinglereq=false;
    }
    else {
      //MASTERWILL("not set burst single req, so that it will be concidered 0");
      finalMap.burstsinglereq=false;      
    }
  }
  if (!COMPARE(byteen)){
    if (pMasterParam->byteen){
      CRITICAL(byteen);
      SLAVEWILL("ignore the byteen field inside the requests from the master (and concider it being all 1s)");
      finalMap.byteen=false;
    }
    else {
      //MASTERWILL("not set byteen req, so that it will be concidered all 1s");
      finalMap.byteen=false;      
    }    
  }
  if (!COMPARE(cmdaccept)){
    if (pMasterParam->cmdaccept){
      //SLAVEWILL("not set the SCmdAccept signal.");
      finalMap.cmdaccept=false;
    }
    else {
      FATAL(cmdaccept);
      MASTERWILL("ignore the SCmdAccept signal and concider it being 1");
      //finalMap.cmdaccept=false;      
    }     
  }
  if (!COMPARE(connid)){
    if (pMasterParam->connid){
      CRITICAL(connid);
      SLAVEWILL("ignore the conn id field inside the requests from the master");
    }
    else {
      //MASTERWILL("not set conn id, so that all transfer will be concidered using conn id 0");
    }
    finalMap.connid=false;
  }
  if(!COMPARE(connid_wdth) && finalMap.connid){
    if (pMasterParam->connid_wdth>pSlaveParam->connid_wdth){
      CRITICAL(connid_wdth);
      SLAVEWILL("ignore the top most "<< (pMasterParam->connid_wdth-pSlaveParam->connid_wdth) << "bits of the conn id field");
      finalMap.connid_wdth=pSlaveParam->connid_wdth;
    }
    else {
      //MASTERWILL("not set the top most "<< (pSlaveParam->connid_wdth-pMasterParam->connid_wdth) << "bits of the conn id field");
      finalMap.connid_wdth=pMasterParam->connid_wdth;      
    }
  }
  if (finalMap.datahandshake){
    if (!COMPARE(dataaccept)){
      if(pMasterParam->dataaccept){
        //SLAVEWILL("not set dataaccept signal and so it's concidered being tied to 1");
      }
      else {
        FATAL(dataaccept);
        MASTERWILL("ignore the dataaccept signal and concider it being tied to 1");
      }
      finalMap.dataaccept=false;
    }
    if (!COMPARE(datalast)){
      if(pMasterParam->datalast){
        //SLAVEWILL("ignore datalast signal and count for itself.");
      }
      else {
        FATAL(datalast);
        MASTERWILL("will not set a datalast signal so slave has to count for itself");
      }
      finalMap.datalast=false;
    }
    if (!COMPARE(datarowlast)){
      if(pMasterParam->datarowlast){
      }
      else {
        FATAL(datarowlast);
        MASTERWILL("will not set a datarowlast signal so slave has to count for itself");
      }
      finalMap.datarowlast=false;
    }
  }
  if (!COMPARE(data_wdth)){
    int mst_msb=0;
    int sl_msb=0;
    bool mst_moreThanOne=false;
    bool sl_moreThanOne=false;
    int mask=0x1;
    for (int i=1;i<32;i++){
      if ( (pMasterParam->data_wdth)&mask ) {
        if (mst_msb) mst_moreThanOne=true;
        mst_msb=i;
      }
      if ( (pSlaveParam->data_wdth)&mask ) {
        if (sl_msb) sl_moreThanOne=true;
        sl_msb=i;
      }
      mask=mask<<1;
    }
    if (mst_moreThanOne) mst_msb++;
    if (sl_moreThanOne) sl_msb++;
    if (mst_msb != sl_msb) {FATAL(data_wdth);}
    else {
      if (pMasterParam->data_wdth>pSlaveParam->data_wdth)
        finalMap.data_wdth=pMasterParam->data_wdth;
      else
        finalMap.data_wdth=pSlaveParam->data_wdth;
    }
  }
  if (!COMPARE(mdata) && finalMap.write_enable) {
    if (pMasterParam->mdata){
      CRITICAL(mdata);
      SLAVEWILL("ignore the master data");
    }
    else {
      //MASTERWILL("not provide write data when doing a write (whatever sense this will make...)");
    }
    finalMap.mdata=false;
  }
  if (!COMPARE(mdatabyteen) && finalMap.mdata){
    if (pMasterParam->mdatabyteen){
      CRITICAL(mdatabyteen);
      SLAVEWILL("ignore the master data byteen");
    }
    else {
      //MASTERWILL("not provide write data byteen when doing a write");
    }
    finalMap.mdatabyteen=false;
  }
  if (!COMPARE(mdatainfo)){
    NONCRITICAL(mdatainfo);
    if (pMasterParam->mdatainfo){
      SLAVEWILL("ignore the master data info");
    }
    else {
      MASTERWILL("not provide write data info when doing a write");
    }
    finalMap.mdatainfo=false;
  }
  if (!COMPARE(mdatainfo_wdth) && finalMap.mdatainfo) {
    NONCRITICAL(mdatainfo_wdth);
    if (pMasterParam->mdatainfo_wdth>pSlaveParam->mdatainfo_wdth){
      SLAVEWILL("ignore the top most "<< (pMasterParam->mdatainfo_wdth-pSlaveParam->mdatainfo_wdth) << "bits of the mdata info field");
      finalMap.mdatainfo_wdth=pSlaveParam->mdatainfo_wdth;
    }
    else {
      MASTERWILL("not set the top most "<< (pSlaveParam->mdatainfo_wdth-pMasterParam->mdatainfo_wdth) << "bits of the mdata info field");
      finalMap.mdatainfo_wdth=pMasterParam->mdatainfo_wdth;      
    }    
  }
  if (!COMPARE(mdatainfobyte_wdth) && finalMap.mdatainfo){
    NONCRITICAL(mdatainfobyte_wdth);
    finalMap.mdatainfobyte_wdth=(pMasterParam->mdatainfobyte_wdth>pSlaveParam->mdatainfobyte_wdth)? pSlaveParam->mdatainfobyte_wdth : pMasterParam->mdatainfobyte_wdth;
  }
  if (!COMPARE(sdatathreadbusy)){
    NONCRITICAL(sdatathreadbusy);
    if (pMasterParam->sdatathreadbusy) {
      SLAVEWILL("ignore sdatathreadbusy and concider it being 0");
    }
    else {
      MASTERWILL("not set sdatathreadbusy.");
    }
    finalMap.sdatathreadbusy=false;
  }
  if(!COMPARE(mthreadbusy)){
    NONCRITICAL(mthreadbusy);
    if (pMasterParam->mthreadbusy) {
      SLAVEWILL("ignore mthreadbusy and concider it being 0");
    }
    else {
      MASTERWILL("not set mthreadbusy.");
    }
    finalMap.mthreadbusy=false;
  }
  if(!COMPARE(reqinfo)){
    NONCRITICAL(reqinfo);
    if (pMasterParam->reqinfo) {
      SLAVEWILL("ignore reqinfo");
    }
    else {
      MASTERWILL("not set reqinfo.");
    }
    finalMap.reqinfo=false;
  }
  if (!COMPARE(reqinfo_wdth) && finalMap.reqinfo){
    NONCRITICAL(reqinfo_wdth);
    if (pMasterParam->reqinfo_wdth>pSlaveParam->reqinfo_wdth){
      SLAVEWILL("ignore the top most "<< (pMasterParam->reqinfo_wdth-pSlaveParam->reqinfo_wdth) << "bits of the req info field");
      finalMap.reqinfo_wdth=pSlaveParam->reqinfo_wdth;
    }
    else {
      MASTERWILL("not set the top most "<< (pSlaveParam->reqinfo_wdth-pMasterParam->reqinfo_wdth) << "bits of the req info field");
      finalMap.reqinfo_wdth=pMasterParam->reqinfo_wdth;      
    }      
  }
  if (!COMPARE(reqlast)){
    if (pMasterParam->reqlast) {
      //SLAVEWILL("ignore reqlast and count for itself");
    }
    else {
      FATAL(reqlast);
      MASTERWILL("not set reqlast and so slave must count for itself.");
    }
    finalMap.reqlast=false;    
  }
  if (!COMPARE(reqrowlast)){
    if (pMasterParam->reqrowlast) {
    }
    else {
      FATAL(reqrowlast);
      MASTERWILL("not set reqrowlast and so slave must count for itself.");
    }
    finalMap.reqrowlast=false;    
  }
  if (!COMPARE(resp) && (finalMap.read_enable | finalMap.writeresp_enable)){
    FATAL(resp);
    /*
    if (pMasterParam->resp) {
      SLAVEWILL("not provide a resp during read transfers or write responses(god knows how that will work...)");
    }
    else {
      MASTERWILL("ignore the resp during a read transfer.");
    }
    finalMap.resp=false;
    */
  }
  if (!COMPARE(respaccept) && finalMap.resp){
    if (pMasterParam->respaccept) {
      FATAL(respaccept);
      SLAVEWILL("ignore MRespAccept signal during read transfers");
    }
    else {
      //MASTERWILL("not provide MRespAccept signal during read transfers.");
    }
    finalMap.respaccept=false;   
  }
  if (!COMPARE(respinfo) && finalMap.resp){
    NONCRITICAL(respinfo);
    if (pMasterParam->respinfo) {
      SLAVEWILL("not provide respinfo during response phase");
    }
    else {
      MASTERWILL("ignore respinfo during response phase.");
    }
    finalMap.respinfo=false;   
  }
  if (!COMPARE(respinfo_wdth) && finalMap.respinfo){
    NONCRITICAL(respinfo_wdth);
    if (pMasterParam->respinfo_wdth>pSlaveParam->respinfo_wdth){
      SLAVEWILL("not set the top most "<< (pMasterParam->respinfo_wdth-pSlaveParam->respinfo_wdth) << "bits of the resp info field");
      finalMap.respinfo_wdth=pSlaveParam->respinfo_wdth;
    }
    else {
      MASTERWILL("ignore the top most "<< (pSlaveParam->respinfo_wdth-pMasterParam->respinfo_wdth) << "bits of the resp info field");
      finalMap.respinfo_wdth=pMasterParam->respinfo_wdth;      
    }      
  }
  if (!COMPARE(resplast) && finalMap.resp){
    if (pMasterParam->resplast) {
      FATAL(resplast);
      SLAVEWILL("not provide resplast during response phase, so master has to count for itself");
    }
    else {
      //MASTERWILL("ignore resplast during response phase and count for itself.");
    }
    finalMap.resplast=false;     
  }
  if (!COMPARE(resprowlast) && finalMap.resp){
    if (pMasterParam->resprowlast) {
      FATAL(resprowlast);
      SLAVEWILL("not provide resprowlast during response phase, so master has to count for itself");
    }
    finalMap.resprowlast=false;     
  }
  if (!COMPARE(sdata) && finalMap.resp) {
    if (pMasterParam->sdata){
      //SLAVEWILL("not provide data when giving a response");
    }
    else {
      CRITICAL(sdata);
      MASTERWILL("ignore the slave data");
    }
    finalMap.sdata=false;
  }
  if (!COMPARE(sdatainfo) && finalMap.sdata){
    NONCRITICAL(sdatainfo);
    if (pMasterParam->sdatainfo){
      SLAVEWILL("not provide sdata info when giving a response");
    }
    else {
      MASTERWILL("ignore the sdata info");
    }
    finalMap.sdatainfo=false;
  }
  if (!COMPARE(sdatainfo_wdth) && finalMap.sdatainfo) {
    NONCRITICAL(sdatainfo_wdth);
    if (pMasterParam->sdatainfo_wdth>pSlaveParam->sdatainfo_wdth){
      SLAVEWILL("not set the top most "<< (pMasterParam->sdatainfo_wdth-pSlaveParam->sdatainfo_wdth) << "bits of the sdata info field");
      finalMap.sdatainfo_wdth=pSlaveParam->sdatainfo_wdth;
    }
    else {
      MASTERWILL("ignore the top most "<< (pSlaveParam->sdatainfo_wdth-pMasterParam->sdatainfo_wdth) << "bits of the sdata info field");
      finalMap.sdatainfo_wdth=pMasterParam->sdatainfo_wdth;      
    }    
  }
  if (!COMPARE(sdatainfobyte_wdth) && finalMap.sdatainfo){
    NONCRITICAL(sdatainfobyte_wdth);
    finalMap.sdatainfobyte_wdth=(pMasterParam->sdatainfobyte_wdth>pSlaveParam->sdatainfobyte_wdth)? pSlaveParam->sdatainfobyte_wdth : pMasterParam->sdatainfobyte_wdth;
  }
  if(!COMPARE(sthreadbusy)){
    NONCRITICAL(sthreadbusy);
    if (pMasterParam->sthreadbusy) {
      SLAVEWILL("not set sthreadbusy");
    }
    else {
      MASTERWILL("ignore sthreadbusy and concider it being 0");
    }
    finalMap.sthreadbusy=false;
  }
  if (!COMPARE(threads)){
    if (pMasterParam->threads>pSlaveParam->threads){
      CRITICAL(threads);
      std::cout<<"         The master supports more threads than the slave, so the channel will support only the number the slave supports."<<std::endl<<std::flush;
      finalMap.threads=pSlaveParam->threads;
    }
    else{
      //std::cout<<"         The slave supports more threads than the master, so the channel will support only the number the master supports."<<std::endl<<std::flush;
      finalMap.threads=pMasterParam->threads;
    }
  }
  if (!COMPARE(tags)){
    if (pMasterParam->tags>pSlaveParam->tags){
      CRITICAL(tags);
      std::cout<<"         The master supports more tags than the slave, so the channel will support only the number the slave supports."<<std::endl<<std::flush;
      finalMap.tags=pSlaveParam->tags;
    }
    else{
      //std::cout<<"         The slave supports more tags than the master, so the channel will support only the number the master supports."<<std::endl<<std::flush;
      finalMap.tags=pMasterParam->tags;
    }
  }
  if (!COMPARE(taginorder)){
    CRITICAL(taginorder);
    if(pSlaveParam->taginorder){
      MASTERHASTO("keep tags in order");
    }
    else{
      std::cout<<"         The Slave has to (under all circumstances) keep tags in order, because the master requires that."<<std::endl<<std::flush;    
    }
    finalMap.taginorder=true;
  }
  /*
  bool slControl=false;
  bool msControl=false;
  if (!COMPARE(control)){
    NONCRITICAL(control);
    std::cout<<"        One of the modules (see above) will ignore the control signals."<<std::endl<<std::flush;
    finalMap.control=true;
    slControl=pSlaveParam->control;
    msControl=pMasterParam->control;
  }
  
  finalMap.controlbusy=false;
  if (slControl && !pSlaveParam->controlbusy)
    std::cout<<"WARNING: The slave will ignore controlbusy although using the control signals."<<std::endl<<std::flush;
  else
    finalMap.controlbusy=true;    
  if (msControl && !pMasterParam->controlbusy)
    std::cout<<"WARNING: The master will ignore controlbusy although using the control signals."<<std::endl<<std::flush;
  else
    finalMap.controlbusy=true;
    
  if (!COMPARE(control_wdth) && finalMap.control){
    NONCRITICAL(control_wdth);
    std::cout<<"        Using the largest active control width."<<std::endl<<std::flush;
    if(msControl && slControl){
      if(pMasterParam->control_wdth>pSlaveParam->control_wdth)
        finalMap.control_wdth=pMasterParam->control_wdth;
      else
        finalMap.control_wdth=pSlaveParam->control_wdth;
    }
    else if (pMasterParam->control)
      finalMap.control_wdth=pMasterParam->control_wdth;
    else
      finalMap.control_wdth=pSlaveParam->control_wdth;
  }

  finalMap.controlwr=false;
  if (slControl && !pSlaveParam->controlwr)
    std::cout<<"WARNING: The slave will ignore controlwr although using the control signals."<<std::endl<<std::flush;
  else
    finalMap.controlwr=true;    
  if (msControl && !pMasterParam->controlwr)
    std::cout<<"WARNING: The master will ignore controlwr although using the control signals."<<std::endl<<std::flush;
  else
    finalMap.controlwr=true;
  */
  if(!COMPARE(interrupt)){
    if(pMasterParam->interrupt){
      //SLAVEWILL("not drive the interrupt signal");
    }
    else{
      CRITICAL(interrupt);
      MASTERWILL("ignore the interrupt signal");
    }
    finalMap.interrupt=false;
  }
  if(!COMPARE(merror)){
    if(pMasterParam->merror){
      CRITICAL(merror);
      SLAVEWILL("ignore the merror signal");
    }
    else{
      //MASTERWILL("not drive the merror signal");
    }
    finalMap.merror=false;
  }
  if (!COMPARE(mflag)){
    NONCRITICAL(mflag);  
    if(pMasterParam->mflag){
      SLAVEWILL("ignore the mflag signal");
    }
    else{
      MASTERWILL("not drive the mflag signal");
    }
    finalMap.mflag=false;
  }
  if(!COMPARE(mflag_wdth) && finalMap.mflag){
    NONCRITICAL(mflag_wdth);
    if (pMasterParam->mflag_wdth>pSlaveParam->mflag_wdth){
      SLAVEWILL("ignore the top most "<< (pMasterParam->mflag_wdth-pSlaveParam->mflag_wdth) << "bits of the mflag field");
      finalMap.mflag_wdth=pSlaveParam->mflag_wdth;
    }
    else {
      MASTERWILL("not set the top most "<< (pSlaveParam->mflag_wdth-pMasterParam->mflag_wdth) << "bits of the mflag field");
      finalMap.mflag_wdth=pMasterParam->mflag_wdth;      
    }     
  }
  if (!COMPARE(mreset)){
    FATAL(mreset);
  }
  if(!COMPARE(serror)){
    if(pMasterParam->serror){
      //SLAVEWILL("not drive the serror signal");
    }
    else{
      CRITICAL(serror);
      MASTERWILL("ignore the serror signal");
    }
    finalMap.serror=false;
  }
  if (!COMPARE(sflag)){
    NONCRITICAL(sflag);  
    if(pMasterParam->sflag){
      SLAVEWILL("not set the sflag signal");
    }
    else{
      MASTERWILL("ignore the sflag signal");
    }
    finalMap.sflag=false;
  }
  if(!COMPARE(sflag_wdth) && finalMap.sflag){
    NONCRITICAL(sflag_wdth);
    if (pMasterParam->sflag_wdth>pSlaveParam->sflag_wdth){
      SLAVEWILL("not set the top most "<< (pMasterParam->sflag_wdth-pSlaveParam->sflag_wdth) << "bits of the sflag field");
      finalMap.sflag_wdth=pSlaveParam->sflag_wdth;
    }
    else {
      MASTERWILL("ignore the top most "<< (pSlaveParam->sflag_wdth-pMasterParam->sflag_wdth) << "bits of the sflag field");
      finalMap.sflag_wdth=pMasterParam->sflag_wdth;      
    }     
  }
  if (!COMPARE(sreset)){
    FATAL(sreset);
  }
  /*
  bool slStatus=false;
  bool msStatus=false;
  if (!COMPARE(status)){
    NONCRITICAL(status);
    std::cout<<"        One of the modules (see above) will ignore the status signals."<<std::endl<<std::flush;
    finalMap.status=true;
    slControl=pSlaveParam->status;
    msControl=pMasterParam->status;
  }
  finalMap.statusbusy=false;
  if (slStatus && !pSlaveParam->statusbusy)
    std::cout<<"WARNING: The slave will ignore statusbusy although using the status signals."<<std::endl<<std::flush;
  else
    finalMap.statusbusy=true;    
  if (msStatus && !pMasterParam->statusbusy)
    std::cout<<"WARNING: The master will ignore statusbusy although using the status signals."<<std::endl<<std::flush;
  else
    finalMap.statusbusy=true;
  if (!COMPARE(status_wdth) && finalMap.status){
    NONCRITICAL(status_wdth);
    std::cout<<"        Using the largest active status width."<<std::endl<<std::flush;
    if(msStatus && slStatus){
      if(pMasterParam->status_wdth>pSlaveParam->status_wdth)
        finalMap.status_wdth=pMasterParam->status_wdth;
      else
        finalMap.status_wdth=pSlaveParam->status_wdth;
    }
    else if (pMasterParam->status)
      finalMap.status_wdth=pMasterParam->status_wdth;
    else
      finalMap.status_wdth=pSlaveParam->status_wdth;
  }
  finalMap.statusrd=false;
  if (slStatus && !pSlaveParam->statusrd)
    std::cout<<"WARNING: The slave will ignore statusrd although using the status signals."<<std::endl<<std::flush;
  else
    finalMap.statusrd=true;    
  if (msStatus && !pMasterParam->statusrd)
    std::cout<<"WARNING: The master will ignore statusrd although using the status signals."<<std::endl<<std::flush;
  else
    finalMap.statusrd=true;
  */
  if (!COMPARE(sdatathreadbusy_pipelined) && finalMap.sdatathreadbusy_exact){
    FATAL(sdatathreadbusy_exact);
  }
  if (!COMPARE(sthreadbusy_pipelined) && finalMap.sthreadbusy_exact){
    FATAL(sthreadbusy_exact);
  }
  if (!COMPARE(mthreadbusy_pipelined) && finalMap.mthreadbusy_exact){
    FATAL(mthreadbusy_exact);
  }
  
  if(m_mismatch){
    std::cout<<"ERROR: Master and Slave configurations that have been applied to channel "<<m_name<<" are NOT compliante (see FATAL Errors above)."<<std::endl<<std::flush;
    abort();
    }
  else {
    MapStringType THE_MAP;
    rebuildMap(finalMap, THE_MAP);
    m_params->setOCPConfiguration( m_name, THE_MAP );
    m_params->configured=true;
  }
}

inline char Config_manager::getTypeAbbreviation(int value){return 'i';}
inline char Config_manager::getTypeAbbreviation(bool value){return 'i';}
inline char Config_manager::getTypeAbbreviation(float value){return 'f';}
inline char Config_manager::getTypeAbbreviation(std::string value){return 's';}

#undef COMPARE
#undef FATAL
#undef CRITICAL
#undef NONCRITICAL
#undef MASTERMUSTNOT
#undef MASTERHASTO
#undef SLAVEWILL
#undef MASTERWILL

#endif
