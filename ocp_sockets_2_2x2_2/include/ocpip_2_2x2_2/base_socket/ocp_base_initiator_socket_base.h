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
  #error ocp_base_initiator_socket_base.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION {
namespace infr{


//This is the green_initiator_socket from which other initiator_sockets
// shall be derived
template <unsigned int BUSWIDTH=32,
          typename TRAITS=tlm::tlm_base_protocol_types,
          unsigned int N=0,
          typename BIND_BASE=bind_checker<TRAITS>,
          typename EXT_FILLER=mm_base<TRAITS>,
          typename POOL=default_pool<TRAITS, EXT_FILLER>
          >
class initiator_socket_base: 
                              public initiator_socket_callback_base<BUSWIDTH,TRAITS, N, typename BIND_BASE::root_type, typename BIND_BASE::multi_root_type>
                            , public BIND_BASE
                            , public BIND_BASE::ext_support_type
                            , public mm_base<TRAITS>
{
public:
//typedef section
  typedef TRAITS                                                       traits_type;
  typedef initiator_socket_callback_base<BUSWIDTH,TRAITS,N, typename BIND_BASE::root_type, typename BIND_BASE::multi_root_type>     base_type;
  typedef typename base_type::base_target_socket_type                  base_target_socket_type;
  typedef typename TRAITS::tlm_payload_type                            payload_type;
  typedef typename TRAITS::tlm_phase_type                              phase_type;
  typedef tlm::tlm_sync_enum                                           sync_enum_type;
//  typedef bind_checker<base_type>                            bind_checker_type;
  
  typedef BIND_BASE bind_checker_type;
  
  typedef typename BIND_BASE::ext_support_type                  ext_support_type;
  typedef  allocation_scheme                            allocation_scheme_type;
  
  typedef mm_base<TRAITS>              mm_base_type;
  
  //enum allocation_scheme{OCP_TXN_ONLY, OCP_TXN_WITH_DATA, OCP_TXN_WITH_BE, OCP_TXN_WITH_BE_AND_DATA};
  
  static allocation_scheme_type mm_txn_only(){return OCP_TXN_ONLY;}
  static allocation_scheme_type mm_txn_with_data(){return OCP_TXN_WITH_DATA;}
  static allocation_scheme_type mm_txn_with_be(){return OCP_TXN_WITH_BE;}
  static allocation_scheme_type mm_txn_with_be_and_data(){return OCP_TXN_WITH_BE_AND_DATA;}
  
  
public:
  //constructor requires an name. The last argument is for future use and shall not be used.
  initiator_socket_base(const char* name, allocation_scheme alloc_scheme=mm_txn_only(), EXT_FILLER* ext_filler=NULL);
  ~initiator_socket_base();
  
  //get a memory managed transaction from the pool.
  // Its ref count is already set to 1
  payload_type* get_transaction();

  //Return a memory managed transaction to the pool.
  // It shall be called when an initiator has no longer use for a transaction
  void release_transaction(payload_type* txn);
/*   
  //Register a callback for the nb_transport_bw TLM2 interface method
  template <typename MODULE>
  void register_nb_transport_bw(MODULE* mod, sync_enum_type (MODULE::*cb)(payload_type& trans, phase_type& phase, sc_core::sc_time& t));

  //Register a callback for the invalidate_dmi TLM2 interface method
  template <typename MODULE>
  void register_invalidate_dmi(MODULE* mod, void (MODULE::*cb)(sc_dt::uint64 start_range, sc_dt::uint64 end_range));
*/
  //test if a transaction is from the pool of this socket (for debug only. This call is SLOW!!)
  bool is_from(payload_type* gp){return m_pool->is_from(gp);}
  
protected:

  //ATTENTION: derived socket should not implement end_of_elaboration
  // if they do, they have to call this end_of_elaboration before anything else
  void end_of_elaboration();
  
  virtual void free(payload_type* txn);
  
  //this function is called when we were successfully bound to someone
  // override it to react to the resolved config
  virtual void bound_to(const std::string&, typename BIND_BASE::bind_base_type*, unsigned int);
  
  //override this function when deriving from the greensocket
  virtual const std::string& get_type_string();

  
  //virtual unsigned int get_num_bindings();

  const static std::string s_kind;
  POOL* m_pool;
  EXT_FILLER * m_filler;
  
  //nb_transport_functor<TRAITS> m_nb_bw_functor;
  //invalidate_direct_mem_ptr_functor<TRAITS> m_inval_dmi_functor;
};

} //end ns infr
} //end ns OCPIP_VERSION

#include __MACRO_STRINGYFY__(../src/OCPIP_VERSION/base_socket/ocp_base_initiator_socket_base.tpp)

