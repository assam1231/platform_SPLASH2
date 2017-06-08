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
OCPIP_VERSION::infr::mm_base<TRAITS>::mm_base(allocation_scheme scheme)
  : m_scheme(scheme)
  {}

template <typename TRAITS>
typename OCPIP_VERSION::infr::mm_base<TRAITS>::payload_type* OCPIP_VERSION::infr::mm_base<TRAITS>::
fill_txn(payload_type* txn){
  txn->set_mm(this);
  if (m_scheme!=OCP_TXN_ONLY) {
    per_txn_data_and_be* tmp=new per_txn_data_and_be();
    txn->set_extension(tmp);
  }
  return txn;
}

//OCP_TXN_ONLY, OCP_TXN_WITH_DATA, OCP_TXN_WITH_BE, OCP_TXN_WITH_BE_AND_DATA
template <typename TRAITS>
unsigned int OCPIP_VERSION::infr::mm_base<TRAITS>::get_reserved_data_size(payload_type& txn){
  assert(m_scheme==OCP_TXN_WITH_DATA | m_scheme==OCP_TXN_WITH_BE_AND_DATA);
  per_txn_data_and_be* tmp;
  txn.get_extension(tmp);
  assert(tmp);
  return tmp->value.data.size();
}

template <typename TRAITS>
unsigned int OCPIP_VERSION::infr::mm_base<TRAITS>::get_reserved_be_size(payload_type& txn ){
  assert(m_scheme==OCP_TXN_WITH_BE | m_scheme==OCP_TXN_WITH_BE_AND_DATA);
  per_txn_data_and_be* tmp;
  txn.get_extension(tmp);
  assert(tmp);
  return tmp->value.be.size();
}

template <typename TRAITS>
void OCPIP_VERSION::infr::mm_base<TRAITS>::reserve_data_size(payload_type& txn, unsigned int size){
  assert(m_scheme==OCP_TXN_WITH_DATA | m_scheme==OCP_TXN_WITH_BE_AND_DATA);
  per_txn_data_and_be* tmp;
  txn.get_extension(tmp);
  assert(tmp);
  if (tmp->value.data.size()<size) {
    tmp->value.data.resize(size);
    
  }
  txn.set_data_ptr(&(tmp->value.data[0]));
  txn.set_data_length(size);
}

template <typename TRAITS>
void OCPIP_VERSION::infr::mm_base<TRAITS>::reserve_be_size(payload_type& txn, unsigned int size){
  assert(m_scheme==OCP_TXN_WITH_BE | m_scheme==OCP_TXN_WITH_BE_AND_DATA);
  per_txn_data_and_be* tmp;
  txn.get_extension(tmp);
  assert(tmp);
  if (tmp->value.be.size()<size) {
    tmp->value.be.resize(size);
  }  
  txn.set_byte_enable_ptr(&(tmp->value.be[0]));
  txn.set_byte_enable_length(size);
}

