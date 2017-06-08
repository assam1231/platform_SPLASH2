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
  #error ocp_base_socket_default_pool.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{
namespace infr{


//This is the pool of transactions used within
// ocp_base_initiator_sockets.
// It's for internal use only
template<typename TRAITS,
        typename EXT_FILLER,
        unsigned int N=20
        >
class default_pool{
public:
  typedef typename TRAITS::tlm_payload_type payload_type;

protected:
  struct entry{
    public:
    entry(payload_type* content);
    
    payload_type* that;
    entry* next;
  };
    
public:
  default_pool(EXT_FILLER* filler);
  
  ~default_pool();
  
  bool is_from(payload_type* cont);
  
  inline payload_type* construct();

  inline void free (payload_type* cont);
  
protected:
  entry* unused;
  entry* used;
  std::vector<payload_type*> mine; //just for clean up and is_from
  EXT_FILLER* m_filler;
};

} //end ns infr
} //end ns OCPIP_VERSION

#include __MACRO_STRINGYFY__(../src/OCPIP_VERSION/base_socket/ocp_base_socket_default_pool.tpp)


