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

template <unsigned int BUSWIDTH, typename TRAITS, unsigned int N, typename BIND_BASE, typename EXT_FILLER, typename POOL>
OCPIP_VERSION::infr::initiator_socket_base<BUSWIDTH, TRAITS, N, BIND_BASE, EXT_FILLER, POOL>::~initiator_socket_base(){
  if (m_pool) delete m_pool;
}

template <unsigned int BUSWIDTH, typename TRAITS, unsigned int N, typename BIND_BASE, typename EXT_FILLER, typename POOL>
OCPIP_VERSION::infr::initiator_socket_base<BUSWIDTH, TRAITS, N, BIND_BASE, EXT_FILLER, POOL>::initiator_socket_base
  (const char* name, allocation_scheme scheme, EXT_FILLER* ext_filler): base_type(name, static_cast<typename BIND_BASE::bind_base_type*>(this))
                                                 , bind_checker_type(base_type::name(), (base_type*)this)
//                                                 , ext_support_type(BUSWIDTH)
                                                 , mm_base_type(scheme)
                                                 , m_filler(ext_filler)
{
  m_pool=NULL;
  //base_type::bind((tlm::tlm_bw_transport_if<TRAITS>&)*this);
}


//get MMed transaction from pool (use for nb_transport)
template <unsigned int BUSWIDTH, typename TRAITS, unsigned int N, typename BIND_BASE, typename EXT_FILLER, typename POOL>
typename TRAITS::tlm_payload_type* OCPIP_VERSION::infr::initiator_socket_base<BUSWIDTH, TRAITS, N, BIND_BASE, EXT_FILLER, POOL>::get_transaction(){ 
  if (!m_pool) {
    std::cerr<<"create_txn() called before pool was constructed (before simulation started)."<<std::endl
             <<" It may only be called during simulation runtime."<<std::endl;
    exit(1);
  }
  payload_type* tmp=m_pool->construct();
  tmp->acquire();
  return tmp;
}

//return MMed transaction to pool (use for nb_transport)
template <unsigned int BUSWIDTH, typename TRAITS, unsigned int N, typename BIND_BASE, typename EXT_FILLER, typename POOL>
void OCPIP_VERSION::infr::initiator_socket_base<BUSWIDTH, TRAITS, N, BIND_BASE, EXT_FILLER, POOL>::release_transaction(payload_type* txn){
  txn->release();
}

template <unsigned int BUSWIDTH, typename TRAITS, unsigned int N, typename BIND_BASE, typename EXT_FILLER, typename POOL>
void OCPIP_VERSION::infr::initiator_socket_base<BUSWIDTH, TRAITS, N, BIND_BASE, EXT_FILLER, POOL>::end_of_elaboration(){
  if (base_type::m_beoe_disabled) {
    return;
  }
  //bind_checker_type::resize_configs();
  for (unsigned int i=0; i<base_type::size(); i++)
    bind_checker_type::check_binding(i);
  if (m_filler==NULL) m_pool=new POOL(this);
  else m_pool=new POOL(m_filler);    
}

template <unsigned int BUSWIDTH, typename TRAITS, unsigned int N, typename BIND_BASE, typename EXT_FILLER, typename POOL>
void OCPIP_VERSION::infr::initiator_socket_base<BUSWIDTH, TRAITS, N, BIND_BASE, EXT_FILLER, POOL>::free(payload_type* txn){
  txn->reset();
  m_pool->free(txn);
}

//this function is called when we get info from someone else
// it allows the user to adjust the socket config according to what he learned from the string and the void *
template <unsigned int BUSWIDTH, typename TRAITS, unsigned int N, typename BIND_BASE, typename EXT_FILLER, typename POOL>
void OCPIP_VERSION::infr::initiator_socket_base<BUSWIDTH, TRAITS, N, BIND_BASE, EXT_FILLER, POOL>::bound_to(const std::string&, typename BIND_BASE::bind_base_type*, unsigned int){
}

template <unsigned int BUSWIDTH, typename TRAITS, unsigned int N, typename BIND_BASE, typename EXT_FILLER, typename POOL>
const std::string& OCPIP_VERSION::infr::initiator_socket_base<BUSWIDTH, TRAITS, N, BIND_BASE, EXT_FILLER, POOL>::get_type_string(){return s_kind;}

template <unsigned int BUSWIDTH, typename TRAITS, unsigned int N, typename BIND_BASE, typename EXT_FILLER, typename POOL>
const std::string OCPIP_VERSION::infr::initiator_socket_base<BUSWIDTH, TRAITS, N, BIND_BASE, EXT_FILLER, POOL>::s_kind="basic_green_socket";
