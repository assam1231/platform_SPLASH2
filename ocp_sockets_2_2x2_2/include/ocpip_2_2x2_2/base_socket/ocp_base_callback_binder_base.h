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
  #error ocp_base_callback_binder_base.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{
namespace infr{

class ocp_base_callback_binder_base
{
public:
    int get_index(){return m_id;}
  void* get_owner(){
    return m_owner;
  }
  
  template<typename T>
  void  set_owner(T* owner) {
    m_owner=static_cast<void*>(owner);
  }
  
  void set_offset(unsigned int offset){
    if (offset){
      std::cerr<<"OCP sockets do not support ID offsets"<<std::endl;
      abort();
    }
    m_offset=offset;
  }
  
  template<typename T>
  ocp_base_callback_binder_base(int id, T* owner, unsigned int offset)
    : m_owner(static_cast<void*>(owner))
    , m_id(id)
    , m_offset(offset)
  {
    if (offset){
      std::cerr<<"OCP sockets do not support ID offsets"<<std::endl;
      abort();
    }  
  }

  template<typename T>
  ocp_base_callback_binder_base(int id, T* owner)
    : m_owner(static_cast<void*>(owner))
    , m_id(id)
    , m_offset(0)
  {
  }

protected:
  void* m_owner;
  unsigned int m_id;
  unsigned int m_offset;  //the tag offset
};

template <typename TRAITS>
class ocp_base_multi_to_multi_bind_base{
public:
  virtual ~ocp_base_multi_to_multi_bind_base(){}
  virtual tlm::tlm_fw_transport_if<TRAITS>* get_last_binder(tlm::tlm_bw_transport_if<TRAITS>*)
  {std::cerr<<"Did you forget to overrid get_last_binder in your implementation?"<<std::endl; abort();}
};

}
}
