///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// (c) Copyright OCP-IP 2008
// OCP-IP Confidential and Proprietary
//
//
//============================================================================
//      Project : OCP SLD WG
//       Author : Robert Guenzel (from TU of Braunschweig) for Greensocs Ltd.
//
//          $Id:
//
//  Description :  This file contains provides a centralized type definition proxy.
//                 Using this proxy every module will use the same data types,
//                 when using the same BUSWIDTH template argument.
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_extension_pool.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{

template <class T>
class ocp_extension_pool{
  struct ocp_extension_pool_entry{
    public:
    ocp_extension_pool_entry(T* content){
      value=content;
      next=NULL;
    }
    T* value;
    ocp_extension_pool_entry* next;
  };
  
public:
  ocp_extension_pool(int size): out_of_pool(NULL){
    in_pool=new ocp_extension_pool_entry(new T());  //create first one
    mine.push_back(in_pool->value);
    for (int i=0; i<size-1; i++){
      ocp_extension_pool_entry* e=new ocp_extension_pool_entry(new T());
      e->next=in_pool;
      in_pool=e;
      mine.push_back(in_pool->value);
    }
  }
  
  ~ocp_extension_pool(){
    for (unsigned int i=0; i<mine.size(); i++){
      delete mine[i];
    }
    
    while (in_pool){
      ocp_extension_pool_entry* e=in_pool;
      in_pool=in_pool->next;
      delete e;
    }

    while (out_of_pool){
      ocp_extension_pool_entry* e=out_of_pool;
      out_of_pool=out_of_pool->next;
      delete e;
    }
  }
  
  bool is_from(T* cont){
    for (int i=0; i<mine.size(); i++){
      if (mine[i]==cont) return true;
    }
    return false;
  }
  
  T* create(){
    ocp_extension_pool_entry* e;
    if (in_pool==NULL){
      e=new ocp_extension_pool_entry(new T());
      mine.push_back(e->value);
    }
    else{
      e=in_pool;
      in_pool=in_pool->next;
    }
    e->next=out_of_pool;
    out_of_pool=e;
    return out_of_pool->value; 
  }

  void recycle(T* cont){
    assert(out_of_pool);
    ocp_extension_pool_entry* e=out_of_pool;
    out_of_pool=e->next;
    e->value=cont;
    e->next=in_pool;
    in_pool=e;
  }
  
private:
  ocp_extension_pool_entry* in_pool;
  ocp_extension_pool_entry* out_of_pool;
  std::vector<T*> mine; //just for clean up and is_from
};

}


