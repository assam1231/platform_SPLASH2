/*
Copyright (c) 2009 GreenSocs Ltd

Permission is hereby granted, free of charge, to any person  
obtaining a copy of this software and associated documentation files  
(the "Software"), to deal in the Software without restriction,  
including without limitation the rights to use, copy, modify, merge,  
publish, distribute, sublicense, and/or sell copies of the Software,  
and to permit persons to whom the Software is furnished to do so,  
subject to the following conditions:

The above copyright notice and this permission notice shall be  
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,  
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF  
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND  
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS  
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN  
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN  
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  
SOFTWARE.
*/

template <typename TRAITS>
std::vector<std::string>& OCPIP_VERSION::infr::config<TRAITS>::get_ext_name_vect(){
  static std::vector<std::string>* p_name_vec=NULL;
  if (!p_name_vec) p_name_vec=new std::vector<std::string>(tlm::max_num_extensions(), "not known yet.");
  return *p_name_vec;
}

template <typename TRAITS>
std::string& OCPIP_VERSION::infr::config<TRAITS>::get_ext_name(unsigned int index){
  static std::string out_of_bounds("Extension not existing.");
  if (index>=tlm::max_num_extensions()) return out_of_bounds;
  return get_ext_name_vect()[index];
}


template <typename TRAITS>
bool OCPIP_VERSION::infr::config<TRAITS>::diff(const OCPIP_VERSION::infr::config<TRAITS>& other){
  if(m_treat_unknown_as_rejected!=other.m_treat_unknown_as_rejected) {
    //std::cout<<"diff reason treat_unkn"<<std::endl; 
    return true;
  }

  if(m_used_phases.size()!=other.m_used_phases.size()) {
    //std::cout<<"diff reason ph_size"<<std::endl; 
    return true;
  } //there is a change if phase maps are unequal
  
  if(m_used_extensions.size()!=other.m_used_extensions.size()) {
    //std::cout<<"diff reason ext_size"<<std::endl; 
    return true;
  }
  
  for (unsigned int i=0;i<m_used_extensions.size(); i++){
    if (other.m_used_extensions.at(i)!=m_used_extensions[i]) {
      //std::cout<<"diff reason ext bindable"<<std::endl; 
      return true;
    }

  }
  
  for (std::map<std::string, OCPIP_VERSION::infr::ocp_base_extension_bindability_enum>::iterator i=m_used_phases.begin(); i!=m_used_phases.end(); i++){
    std::map<std::string, OCPIP_VERSION::infr::ocp_base_extension_bindability_enum>::const_iterator finder=other.m_used_phases.find(i->first);
    if (finder==other.m_used_phases.end()) {
      //std::cout<<"diff reason ph have/dont have"<<std::endl; 
      return true;
    }
    else 
    if (finder->second!= i->second) {
      //std::cout<<"diff reason ph bindable"<<std::endl; 
      return true;
    }
  }

  for (std::map<std::string, OCPIP_VERSION::infr::ocp_base_extension_bindability_enum>::const_iterator i=other.m_used_phases.begin(); i!=other.m_used_phases.end(); i++){
    std::map<std::string, OCPIP_VERSION::infr::ocp_base_extension_bindability_enum>::iterator finder=m_used_phases.find(i->first);
    if (finder==m_used_phases.end()) {
      //std::cout<<"diff reason ph dont have/have"<<std::endl; 
      return true;
    }
    else 
    if (finder->second!= i->second) {
      //std::cout<<"diff reason ph bindable"<<std::endl; 
      return true;
    }
  }
  return false;  
}

template <typename TRAITS>
OCPIP_VERSION::infr::config<TRAITS>::config(const config& other){
  *this=other;
}

template <typename TRAITS>
OCPIP_VERSION::infr::config<TRAITS>& OCPIP_VERSION::infr::config<TRAITS>::operator=(const OCPIP_VERSION::infr::config<TRAITS>& other){
  //m_used_extension_ids=other.m_used_extension_ids;
  m_used_phases=other.m_used_phases;
  m_type_string=other.m_type_string;
  m_treat_unknown_as_rejected=other.m_treat_unknown_as_rejected;
  invalid=other.invalid;
  force_reeval=other.force_reeval;
  force_own_cb=other.force_own_cb;
  m_used_extensions=other.m_used_extensions;
  return *this;
}

template <typename TRAITS>
OCPIP_VERSION::infr::config<TRAITS>::~config(){
}

template <typename TRAITS>
void OCPIP_VERSION::infr::config<TRAITS>::set_force_reeval(bool val){force_reeval|=val; force_own_cb|=val;}

template <typename TRAITS>
void OCPIP_VERSION::infr::config<TRAITS>::set_string_ptr(const std::string* name_){m_type_string=name_;}

template <typename TRAITS>
bool OCPIP_VERSION::infr::config<TRAITS>::get_invalid()const{return invalid;}

template <typename TRAITS>
void OCPIP_VERSION::infr::config<TRAITS>::set_invalid(bool value){invalid=value;}

template <typename TRAITS>
template <typename T>
OCPIP_VERSION::infr::ocp_base_extension_bindability_enum OCPIP_VERSION::infr::config<TRAITS>::has_extension()const{
  if (m_used_extensions.size()<=T::ID) {
    if (m_treat_unknown_as_rejected)
      return OCPIP_VERSION::infr::ocp_base_reject;
    else
      return OCPIP_VERSION::infr::ocp_base_optional;
  }
  if (m_used_extensions.at(T::ID)!=OCPIP_VERSION::infr::ocp_base_unknown){
    return m_used_extensions.at(T::ID);
  }
  else
  if (m_treat_unknown_as_rejected)
    return OCPIP_VERSION::infr::ocp_base_reject;
  else
    return OCPIP_VERSION::infr::ocp_base_optional;
  
  //return has_extension<T>(T::_ID);
}

template <typename TRAITS>
OCPIP_VERSION::infr::ocp_base_extension_bindability_enum OCPIP_VERSION::infr::config<TRAITS>::has_phase(unsigned int ph)const{
  phase_type tmp=ph; 
  std::stringstream s; 
  s<<tmp;
  if (m_used_phases.find(s.str())!=m_used_phases.end()){
    return m_used_phases.find(s.str())->second;
  }
  else
  if (m_treat_unknown_as_rejected)
    return OCPIP_VERSION::infr::ocp_base_reject;
  else
    return OCPIP_VERSION::infr::ocp_base_optional;
}

template <typename TRAITS>
OCPIP_VERSION::infr::config<TRAITS>::config(): invalid(true), force_reeval(false),force_own_cb(false){}

template <typename TRAITS>
bool OCPIP_VERSION::infr::config<TRAITS>::merge_with(const char* my_name, const char* other_name, config<TRAITS>& other_conf, bool abort_at_mismatch){
  m_used_extensions.resize(tlm::max_num_extensions(), OCPIP_VERSION::infr::ocp_base_unknown);
  other_conf.m_used_extensions.resize(tlm::max_num_extensions(), OCPIP_VERSION::infr::ocp_base_unknown);
  bool do_abort=false;
  
  for (unsigned int i=0; i<m_used_extensions.size(); i++){
    OCPIP_VERSION::infr::ocp_base_extension_bindability_enum& ext1=m_used_extensions[i];
    OCPIP_VERSION::infr::ocp_base_extension_bindability_enum  ext2=other_conf.m_used_extensions[i];
    switch(ext1){
      case OCPIP_VERSION::infr::ocp_base_mandatory:
        if (ext2==OCPIP_VERSION::infr::ocp_base_unknown){
          if(other_conf.m_treat_unknown_as_rejected){
            std::cerr<<"Error: "<<my_name<<" requires use of extension "<<get_ext_name(i)<<" but "<<other_name
                     <<" doesn't know that extension and reject unknown extensions."<<std::endl;
            do_abort=true;
          }
        }
        else{
          if (ext2==OCPIP_VERSION::infr::ocp_base_reject){
            std::cerr<<"Error: "<<my_name<<" requires use of extension "<<get_ext_name(i)<<" but "<<other_name<<" rejects that extension."<<std::endl;
            do_abort=true;
          }
          //else nothing. The extension stays mandatory
        }
        break;
      case OCPIP_VERSION::infr::ocp_base_optional:
        switch(ext2){
          case OCPIP_VERSION::infr::ocp_base_unknown:
            if (other_conf.m_treat_unknown_as_rejected) ext1=OCPIP_VERSION::infr::ocp_base_reject;
            break;            
          case OCPIP_VERSION::infr::ocp_base_reject:
            ext1=OCPIP_VERSION::infr::ocp_base_reject;
            break;
          case OCPIP_VERSION::infr::ocp_base_mandatory:
            ext1=OCPIP_VERSION::infr::ocp_base_mandatory; //change to mandatory
            break;
          case OCPIP_VERSION::infr::ocp_base_optional:
            break;
        }
        break;
      case OCPIP_VERSION::infr::ocp_base_reject:
        //the other side will check this
        break;
      case OCPIP_VERSION::infr::ocp_base_unknown:
        if (ext2!=OCPIP_VERSION::infr::ocp_base_unknown){
          if(m_treat_unknown_as_rejected){
            if (ext2==OCPIP_VERSION::infr::ocp_base_mandatory){
              std::cerr<<"Error: "<<my_name<<" treats unknown extensions as rejected but "<<other_name<<" uses the extension "
                       <<get_ext_name(i)<<" which is unknown to "<<my_name<<"."<<std::endl;
              do_abort=true;
            }
          }
          else{ //adopt the extension 
            ext1=ext2;
          }
        }
    }    
  }
  
  std::vector<std::string> to_erase;
  
  for (std::map<std::string, OCPIP_VERSION::infr::ocp_base_extension_bindability_enum>::iterator i=m_used_phases.begin(); i!=m_used_phases.end(); i++){
    std::map<std::string, OCPIP_VERSION::infr::ocp_base_extension_bindability_enum>::iterator finder=other_conf.m_used_phases.find(i->first);
    switch (i->second){
      case OCPIP_VERSION::infr::ocp_base_optional:
        if (finder!=other_conf.m_used_phases.end())
          switch(finder->second){
            case OCPIP_VERSION::infr::ocp_base_mandatory: i->second=OCPIP_VERSION::infr::ocp_base_mandatory; break;
            case OCPIP_VERSION::infr::ocp_base_reject: i->second=OCPIP_VERSION::infr::ocp_base_reject; break;//to_erase.push_back(i->first); break; //m_used_phases.erase(i); break;
            case OCPIP_VERSION::infr::ocp_base_optional: break;
            case OCPIP_VERSION::infr::ocp_base_unknown: assert(0 && "This should NEVER happen!"); exit(666);
          }
        else
          if (other_conf.m_treat_unknown_as_rejected) i->second=OCPIP_VERSION::infr::ocp_base_reject;//to_erase.push_back(i->first);//m_used_phases.erase(i);
        break;
      case OCPIP_VERSION::infr::ocp_base_mandatory:
        if (finder==other_conf.m_used_phases.end()){
          if(other_conf.m_treat_unknown_as_rejected){
            std::cerr<<"Error: "<<my_name<<" requires use of phase "<<i->first<<" but "<<other_name
                     <<" doesn't know that phase and rejects unknown phases."<<std::endl;
            do_abort=true;
          }
        }
        else
        if (finder->second==OCPIP_VERSION::infr::ocp_base_reject){
          std::cerr<<"Error: "<<my_name<<" requires use of phase "<<i->first<<" but "<<other_name<<" rejects that phase."<<std::endl;
          do_abort=true;
        }
        break;
      case OCPIP_VERSION::infr::ocp_base_reject:
        break;
      case OCPIP_VERSION::infr::ocp_base_unknown: assert(0 && "This should NEVER happen!"); exit(666);
    }
  }
  
  for (unsigned int i=0; i<to_erase.size(); i++) m_used_phases.erase(to_erase[i]);
  
  for (std::map<std::string, OCPIP_VERSION::infr::ocp_base_extension_bindability_enum>::iterator i=other_conf.m_used_phases.begin(); i!=other_conf.m_used_phases.end(); i++){
    std::map<std::string, OCPIP_VERSION::infr::ocp_base_extension_bindability_enum>::iterator finder=m_used_phases.find(i->first);
    if (finder==m_used_phases.end()){// && i->second!=OCPIP_VERSION::infr::ocp_base_reject){
      if (m_treat_unknown_as_rejected){
        if (i->second==OCPIP_VERSION::infr::ocp_base_mandatory){
          std::cerr<<"Error: "<<my_name<<" treats unknown phases as rejected but "<<other_name<<" uses the phase "
                   <<i->first<<" which is unknown to "<<my_name<<"."<<std::endl;
          do_abort=true;
        }
      }
      else{
        m_used_phases[i->first]=i->second;
      }
    }
  }
  m_treat_unknown_as_rejected=m_treat_unknown_as_rejected | other_conf.m_treat_unknown_as_rejected; //reject is stronger than ignore
  if (abort_at_mismatch && do_abort) exit(1);
  
  return do_abort;
}

template <typename TRAITS>
void OCPIP_VERSION::infr::config<TRAITS>::treat_unknown_as_optional(){
  m_treat_unknown_as_rejected=false;
}

template <typename TRAITS>
void OCPIP_VERSION::infr::config<TRAITS>::treat_unknown_as_rejected(){
  m_treat_unknown_as_rejected=true;
}

template <typename TRAITS>
bool OCPIP_VERSION::infr::config<TRAITS>::unknowns_are_optional(){
  return !m_treat_unknown_as_rejected;
}

template <typename TRAITS>
void OCPIP_VERSION::infr::config<TRAITS>::use_mandatory_phase(unsigned int ph1){
  std::vector<unsigned int> phs;
  phs.push_back(ph1);
  add_to_phase_map(phs, OCPIP_VERSION::infr::ocp_base_mandatory);
}

template <typename TRAITS>
void OCPIP_VERSION::infr::config<TRAITS>::use_optional_phase(unsigned int ph1){
  std::vector<unsigned int> phs;
  phs.push_back(ph1);
  add_to_phase_map(phs, OCPIP_VERSION::infr::ocp_base_optional);
}

template <typename TRAITS>
void OCPIP_VERSION::infr::config<TRAITS>::reject_phase(unsigned int ph1){
  std::vector<unsigned int> phs;
  phs.push_back(ph1);
  add_to_phase_map(phs, OCPIP_VERSION::infr::ocp_base_reject);
}

template <typename TRAITS>
void OCPIP_VERSION::infr::config<TRAITS>::remove_phase(unsigned int ph1){
  phase_type tmp=ph1;
  std::stringstream s;
  s<<tmp;
  m_used_phases.erase(s.str());
}

template <typename TRAITS>
template <typename T>
void OCPIP_VERSION::infr::config<TRAITS>::remove_extension(){
  T tmp;
  get_ext_name(T::ID)=tmp.get_name();
  m_used_extensions.resize(tlm::max_num_extensions(), OCPIP_VERSION::infr::ocp_base_unknown);
  m_used_extensions[T::ID]=OCPIP_VERSION::infr::ocp_base_unknown;
}

template <typename TRAITS>
template <typename T>
void OCPIP_VERSION::infr::config<TRAITS>::use_mandatory_extension(){
  T tmp;
  get_ext_name(T::ID)=tmp.get_name();
  m_used_extensions.resize(tlm::max_num_extensions(), OCPIP_VERSION::infr::ocp_base_unknown);
  m_used_extensions[T::ID]=OCPIP_VERSION::infr::ocp_base_mandatory;
}

template <typename TRAITS>
template <typename T>
void OCPIP_VERSION::infr::config<TRAITS>::reject_extension(){
  T tmp;
  get_ext_name(T::ID)=tmp.get_name();
  m_used_extensions.resize(tlm::max_num_extensions(), OCPIP_VERSION::infr::ocp_base_unknown);
  m_used_extensions[T::ID]=OCPIP_VERSION::infr::ocp_base_reject;
}

template <typename TRAITS>
template <typename T>
void OCPIP_VERSION::infr::config<TRAITS>::use_optional_extension(){
  T tmp;
  get_ext_name(T::ID)=tmp.get_name();
  m_used_extensions.resize(tlm::max_num_extensions(), OCPIP_VERSION::infr::ocp_base_unknown);
  m_used_extensions[T::ID]=OCPIP_VERSION::infr::ocp_base_optional;
}

template <typename TRAITS>
void OCPIP_VERSION::infr::config<TRAITS>::add_to_phase_map(std::vector<unsigned int>& phs, OCPIP_VERSION::infr::ocp_base_extension_bindability_enum mandatory){
  for (unsigned int i=0; i<phs.size(); i++){
    phase_type tmp=phs[i];
    std::stringstream s;
    s<<tmp;
    //if phase is already there just the bindability state is changed
    m_used_phases[s.str()]=mandatory;
  }
}

template <typename TRAITS>
std::string OCPIP_VERSION::infr::config<TRAITS>::to_string() const{
  std::stringstream s;
  if (invalid) {s<<" Note: Unassigned configuration."<<std::endl;}
  s<<"  Treats unknown as "<<((m_treat_unknown_as_rejected)?"rejected.":"optional.")<<std::endl;
  s<<"  Used extensions:"<<std::endl;
  for (unsigned int i=0; i<m_used_extensions.size(); i++){
    if (m_used_extensions.at(i)!=OCPIP_VERSION::infr::ocp_base_unknown){
      s<<"    "<<get_ext_name(i)<<" used as ";
      switch(m_used_extensions.at(i)){
        case OCPIP_VERSION::infr::ocp_base_mandatory: s<<"mandatory"; break;
        case OCPIP_VERSION::infr::ocp_base_optional: s<<"optional"; break;
        case OCPIP_VERSION::infr::ocp_base_reject: s<<"rejected"; break;
        case OCPIP_VERSION::infr::ocp_base_unknown: ;
      }
      s<<"."<<std::endl;
    }
  }
  s<<"  Used phases:"<<std::endl;
  for (std::map<std::string,OCPIP_VERSION::infr::ocp_base_extension_bindability_enum>::const_iterator i=m_used_phases.begin(); i!=m_used_phases.end(); i++){
    s<<"    "<<i->first<<" used as ";
    switch(i->second){
      case OCPIP_VERSION::infr::ocp_base_mandatory: s<<"mandatory"; break;
      case OCPIP_VERSION::infr::ocp_base_optional: s<<"optional"; break;
      case OCPIP_VERSION::infr::ocp_base_reject: s<<"rejected"; break;
      case OCPIP_VERSION::infr::ocp_base_unknown: ;
    }
    s<<"."<<std::endl;

  }
  return s.str();
}
