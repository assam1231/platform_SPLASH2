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

#ifndef OCPIP_VERSION
  #error ocp_extension_base.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{
namespace infr{

//struct bindability_proxy;

//This enum defines the bindability state of a phase or extension
// It is evaluated when two sockets are bound. 
//  ocp_base_mandatory means: 
//    If the connected socket does not know or reject this extension/phase
//    the binding cannot be done.
//  ocp_base_optional means:
//    If the connected socket rejects this extension/phase it will not be used.
//    If the connected socket requires use of this extension/phase it will be used.
//    If the connected socket does also treat this extension/phase as ignorable the user
//     may or may not use it
//  ocp_base_reject means:
//    The connected socket may not use this extension/phase.
enum ocp_base_extension_bindability_enum{
  ocp_base_optional=0, ocp_base_mandatory=1, ocp_base_reject=2, ocp_base_unknown=3
};

enum ocp_base_extension_type_enum{
  ocp_base_array_guard=0, ocp_base_data=1, ocp_base_guarded_data=2//, ocp_base_tlm_array_overflow_protector_guard=3
};


//This is the base class for all ocp_base_extensions
//  It carries 
//  the name of the extension and some virtual functions.
//  It's only purpose to allow for introspecting extension through a template-free base class
//
// The user shall never directly use this class, it is for GreenSocket internal use only
struct ocp_base_extension_base{
  ocp_base_extension_base(tlm::tlm_extension_base* _this, const char* _name)
    : 
#ifdef NO_DEMANGLE
      name(_name)
#else
      name(abi::__cxa_demangle(_name,0,0,&dummy_status_int))
#endif
  {
  }
  
  virtual ~ocp_base_extension_base(){}
  virtual std::string dump() const=0;
  virtual ocp_base_extension_type_enum get_type(unsigned int&) const=0;
  virtual bool is_valid(){return false;}

  const std::string& get_name()const{return name;}
protected:
  int dummy_status_int;
  std::string name;
}; 

//the following three structs allow for compile time differentiation
// between the three different types of ocp_base_extensions
// That saves a lot of runtime if-else statements
struct ocp_base_array_guard_id{ocp_base_array_guard_id(){}};
struct ocp_base_data_id{ocp_base_data_id() {}};
struct ocp_base_guarded_data_id{ocp_base_guarded_data_id() {}};
struct ocp_base_real_guard_id{ocp_base_real_guard_id(){}};

template<typename T>
struct dummy_guard{static T ID;};
template<typename T>
T dummy_guard<T>::ID=0xdeadbeef;



template<typename T, typename GUARD, typename ID_TYPE, ocp_base_extension_type_enum ENUM_VAL>
struct ocp_base_extension_t : public tlm::tlm_extension<T>, public ocp_base_extension_base
{
  typedef GUARD guard;
  ocp_base_extension_t():ocp_base_extension_base(this, typeid(T).name()), my_pool(NULL){}
  ~ocp_base_extension_t(){}
  virtual void free()=0;
  virtual ocp_base_extension_type_enum get_type(unsigned int& g_id)const {g_id=guard::ID; return ENUM_VAL;}
  virtual void copy_from(tlm::tlm_extension_base const &)=0;
  virtual tlm::tlm_extension_base* clone() const=0;
  virtual std::string dump() const=0;
  virtual bool is_valid(){return m_valid;}
  static const ID_TYPE _ID;
  ocp_extension_pool<T>* my_pool;
  bool m_valid;
};

template<typename T, typename GUARD, typename ID_TYPE, ocp_base_extension_type_enum ENUM_VAL>
const ID_TYPE ocp_base_extension_t<T, GUARD, ID_TYPE, ENUM_VAL>::_ID;

template <typename T>
struct unique_type_generator{};


template<typename T>
struct ocp_array_guard_only_extension
  : public OCPIP_VERSION::infr::ocp_base_extension_t<T
                                                    , dummy_guard<unsigned int>
                                                    , ocp_base_array_guard_id
                                                    , ocp_base_array_guard>
{
  virtual std::string dump() const{return std::string("</extension name=\"")+(this->get_name())+std::string("\" type=\"guard\" value=\"set\">");}
  virtual tlm::tlm_extension_base* clone() const {
    return const_cast<tlm::tlm_extension_base*>((const tlm::tlm_extension_base*)this);
  } 
  virtual void copy_from(tlm::tlm_extension_base const &){}
  virtual void free(){}
};

template <typename T>
struct ocp_guard_only_extension 
  : public ocp_base_extension_t<T
                                              , ocp_array_guard_only_extension<unique_type_generator<T> >
                                              , ocp_base_real_guard_id
                                              , ocp_base_guarded_data>
{
  virtual std::string dump() const{ 
    std::stringstream s; 
    s<<"</extension name=\""<<this->get_name()<<"\" type=\"guard\" value=\""<<this->m_valid<<"\">";
    return s.str();
  }
  void copy_from(tlm::tlm_extension_base const & ext){ 
    const ocp_guard_only_extension<T>* tmp=static_cast<const ocp_guard_only_extension<T>*>(&ext); 
    this->m_valid=tmp->m_valid; 
  } 
  tlm::tlm_extension_base* clone() const { 
    ocp_guard_only_extension<T>* tmp; 
    if (this->my_pool){ 
      tmp=this->my_pool->create(); 
      tmp->my_pool=this->my_pool; 
    }
    else 
      tmp=new ocp_guard_only_extension<T>(); 
    tmp->m_valid=this->m_valid; return tmp;
  }
  virtual void free(){
    if(this->my_pool) this->my_pool->recycle(static_cast<T*>(this)); else delete this;
  }
};

template <typename T>
struct ocp_data_only_extension 
  : public ocp_base_extension_t<T
                              , dummy_guard<unsigned int>
                              , ocp_base_data_id
                              , ocp_base_data>
{
  virtual void copy_from(tlm::tlm_extension_base const &)=0;
  virtual tlm::tlm_extension_base* clone() const=0;
  virtual std::string dump() const=0;
  virtual void free(){
    if(this->my_pool) this->my_pool->recycle(static_cast<T*>(this)); else delete this;
  }

};


template <typename T>
struct ocp_guarded_data_extension 
  : public ocp_base_extension_t<T
                              , ocp_array_guard_only_extension<unique_type_generator<T> >
                              , ocp_base_guarded_data_id
                              , ocp_base_guarded_data>
{
  virtual void copy_from(tlm::tlm_extension_base const &)=0;
  virtual tlm::tlm_extension_base* clone() const=0;
  virtual std::string dump() const=0;
  virtual void free(){
    if(this->my_pool) this->my_pool->recycle(static_cast<T*>(this)); else delete this;
  }

};

template <typename T, typename VAL>
struct ocp_single_member_guarded_data
  : public ocp_guarded_data_extension<T>
{
  void copy_from(tlm::tlm_extension_base const & ext){
    const ocp_single_member_guarded_data<T,VAL>* tmp=static_cast<const ocp_single_member_guarded_data<T,VAL>*>(&ext);
    this->m_valid=tmp->m_valid;
    this->value=tmp->value; 
  } 
  tlm::tlm_extension_base* clone() const { 
    ocp_single_member_guarded_data<T,VAL>* tmp; 
    if (this->my_pool){ 
      tmp=this->my_pool->create(); 
      tmp->my_pool=this->my_pool; 
    }
    else 
      tmp=new ocp_single_member_guarded_data<T,VAL>(); 
    tmp->m_valid=this->m_valid;
    tmp->value=this->value; return tmp;
  } 
  std::string dump() const{ std::stringstream s; s<<"</extension name=\""<<this->get_name()<<"\" type=\"guarded_data\" value=\""<<value<<"\">"; return s.str();} 
  
  VAL value;
};

template <typename T, typename VAL>
struct ocp_single_member_data
  : public ocp_data_only_extension<T>
{
  void copy_from(tlm::tlm_extension_base const & ext){
    const ocp_single_member_data<T,VAL>* tmp=static_cast<const ocp_single_member_data<T,VAL>*>(&ext);
    this->value=tmp->value; 
  } 
  tlm::tlm_extension_base* clone() const { 
    ocp_single_member_data<T,VAL>* tmp; 
    if (this->my_pool){ 
      tmp=this->my_pool->create(); 
      tmp->my_pool=this->my_pool; 
    }
    else 
      tmp=new ocp_single_member_data<T,VAL>(); 
    tmp->m_valid=this->m_valid;
    tmp->value=this->value; return tmp;
  } 
  std::string dump() const{ std::stringstream s; s<<"</extension name=\""<<this->get_name()<<"\" type=\"data\" value=\""<<value<<"\">"; return s.str();} 
  
  VAL value;
};

template<typename T>
struct vector_container : public std::vector<T>{
};

template <typename T>
inline std::ostream& operator<< (std::ostream & os, const vector_container<T> & vec){
  os<<"supressed";
  return os;
}

template <typename T>
inline T* create_extension(const OCPIP_VERSION::infr::ocp_base_array_guard_id&){
  static T* retVal=NULL;
  if (!retVal) retVal=new T();
  return retVal;
}

template <typename T>
inline T* create_extension(const OCPIP_VERSION::infr::ocp_base_data_id&){
  static ocp_extension_pool<T>* p_pool=NULL;
  if (!p_pool) p_pool=new ocp_extension_pool<T>(10);
  T* retVal=p_pool->create();//new T();
  retVal->my_pool=p_pool;
  return retVal;
}

template <typename T>
inline T* create_extension(const OCPIP_VERSION::infr::ocp_base_guarded_data_id&){
  static ocp_extension_pool<T>* p_pool=NULL;
  if (!p_pool) p_pool=new ocp_extension_pool<T>(10);
  T* retVal=p_pool->create();//new T();
  retVal->my_pool=p_pool;
  return retVal;
}

template <typename T>
inline T* create_extension(const OCPIP_VERSION::infr::ocp_base_real_guard_id&){
  static ocp_extension_pool<T>* p_pool=NULL;
  if (!p_pool) p_pool=new ocp_extension_pool<T>(10);
  T* retVal=p_pool->create();//new T();
  retVal->my_pool=p_pool;
  return retVal;
}

template <typename T>
inline T* create_extension(){
  return create_extension<T>(T::_ID);
}


} //end ns infr
} //end ns OCPIP_VERSION

