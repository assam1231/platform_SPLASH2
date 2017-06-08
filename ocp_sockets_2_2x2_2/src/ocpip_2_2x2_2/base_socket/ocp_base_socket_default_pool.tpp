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

template<typename TRAITS,typename EXT_FILLER,unsigned int N>
OCPIP_VERSION::infr::default_pool<TRAITS,EXT_FILLER,N>::entry::entry(payload_type* content){
  that=content;
  next=NULL;
}
    
template<typename TRAITS,typename EXT_FILLER,unsigned int N>
OCPIP_VERSION::infr::default_pool<TRAITS,EXT_FILLER,N>::default_pool(EXT_FILLER* filler): used(NULL), m_filler(filler){
  unused=new entry(m_filler->fill_txn(new payload_type()));  //create first one
  mine.push_back(unused->that);
  for (unsigned int i=0; i<N-1; i++){
    entry* e=new entry(m_filler->fill_txn(new payload_type()));
    e->next=unused;
    unused=e;
    mine.push_back(unused->that);
  }
}

template<typename TRAITS,typename EXT_FILLER,unsigned int N>
OCPIP_VERSION::infr::default_pool<TRAITS,EXT_FILLER,N>::~default_pool(){
  //delete all payload_type* that belong to this pool
  for (unsigned int i=0; i<mine.size(); i++){
    delete mine[i];
  }
  
  //delete all unused elements
  while (unused){
    entry* e=unused;
    unused=unused->next;
    delete e;
  }

  //delete all used elements
  while (used){
    entry* e=used;
    used=used->next;
    delete e;
  }
}

template<typename TRAITS,typename EXT_FILLER,unsigned int N>
bool OCPIP_VERSION::infr::default_pool<TRAITS,EXT_FILLER,N>::is_from(payload_type* cont){ //slow!!!
  for (unsigned int i=0; i<mine.size(); i++){
    if (mine[i]==cont) return true;
  }
  return false;
}

template<typename TRAITS,typename EXT_FILLER,unsigned int N>
typename TRAITS::tlm_payload_type* OCPIP_VERSION::infr::default_pool<TRAITS,EXT_FILLER,N>::construct(){
  entry* e;
  if (unused==NULL){
    e=new entry(m_filler->fill_txn(new payload_type()));
    mine.push_back(e->that);
  }
  else{
    e=unused;
    unused=unused->next;
  }
  e->next=used;
  used=e;
  return used->that; //if all elements of pool are used, just create a new one and go on      
}

template<typename TRAITS,typename EXT_FILLER,unsigned int N>
void OCPIP_VERSION::infr::default_pool<TRAITS,EXT_FILLER,N>::free(payload_type* cont){
  assert(used);
  entry* e=used;
  used=e->next;
  e->that=cont;
  e->next=unused;
  unused=e;
}
