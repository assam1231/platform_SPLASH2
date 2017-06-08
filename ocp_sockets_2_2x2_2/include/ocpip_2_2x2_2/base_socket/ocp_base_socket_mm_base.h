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
  #error ocp_base_socket_mm_base.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{
namespace infr{

enum allocation_scheme{OCP_TXN_ONLY, OCP_TXN_WITH_DATA, OCP_TXN_WITH_BE, OCP_TXN_WITH_BE_AND_DATA};

struct per_txn_data_and_be_struct{
  std::vector<unsigned char> data;
  std::vector<unsigned char> be;
};

//this is not a functional extension, but a mere MM extension, hence we will not create it using a MACRO
struct per_txn_data_and_be
  : public tlm::tlm_extension<per_txn_data_and_be>
{
  void copy_from(tlm::tlm_extension_base const & ext){} //copy from, because we do not clone
  tlm::tlm_extension_base* clone() const {return NULL;} //no clone of per txn data and be
  per_txn_data_and_be_struct value;
};

//This function is a base class for the memory management features of the
// ocp_base_initiator socket. It is for internal use only
template <typename TRAITS>
class mm_base:
    public tlm::tlm_mm_interface
{
public:
  typedef typename TRAITS::tlm_payload_type  payload_type;
  typedef typename TRAITS::tlm_phase_type  phase_type;
  inline payload_type* fill_txn(payload_type*);  
  virtual void free(payload_type*)=0;
  mm_base(allocation_scheme);
  void reserve_data_size(payload_type&, unsigned int);
  void reserve_be_size(payload_type&, unsigned int);
  unsigned int get_reserved_data_size(payload_type&);
  unsigned int get_reserved_be_size(payload_type&);
  
protected:
  allocation_scheme m_scheme;
};

} //end ns infr
} //end ns OCPIP_VERSION

#include __MACRO_STRINGYFY__(../src/OCPIP_VERSION/base_socket/ocp_base_socket_mm_base.tpp)


