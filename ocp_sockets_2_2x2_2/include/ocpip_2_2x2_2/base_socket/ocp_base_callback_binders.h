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
  #error ocp_base_callback_binders.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{
namespace infr{


/*
This class implements the fw interface.
It allows to register a callback for each of the fw interface methods.
The callbacks simply forward the fw interface call, but add the id (an int)
of the callback binder to the signature of the call.
*/
template <typename TRAITS, typename CB_BINDER_BASE>
class ocp_base_callback_binder_fw_base 
  : public tlm::tlm_fw_transport_if<TRAITS>
  , public CB_BINDER_BASE
{
public:
    template<typename T>
    ocp_base_callback_binder_fw_base(int id, T* owner)
      : CB_BINDER_BASE(id, owner)
    {
    }

    //getter method to get the port that is bound to that callback binder
    // NOTE: this will only return a valid value at end of elaboration
    //  (but not before end of elaboration!)
    sc_core::sc_port_base* get_other_side(){return m_caller_port;}
    //the SystemC standard callback register_port:
    // - called when a port if bound to the interface
    // - allowd to find out who is bound to that callback binder
    void register_port(sc_core::sc_port_base& b, const char* name){
      m_caller_port=&b;
    }

protected:
    //the port bound to that callback binder
    sc_core::sc_port_base* m_caller_port;   
};

template <typename TRAITS, bool TAGGED, typename CB_BINDER_BASE>
class ocp_base_callback_binder_fw{};


template <typename TRAITS, typename CB_BINDER_BASE>
class ocp_base_callback_binder_fw<TRAITS, true, CB_BINDER_BASE> : public ocp_base_callback_binder_fw_base<TRAITS, CB_BINDER_BASE>{
#define OCP_I_KNOW_VERY_WELL_WHAT_I_AM_DOING_WHEN_INCLUDING_THIS_FILE
#define OCP_USE_WITH_TAG
#include __MACRO_STRINGYFY__(../include/OCPIP_VERSION/base_socket/ocp_base_callback_fw_binders_body.h)
#undef OCP_USE_WITH_TAG
#undef OCP_I_KNOW_VERY_WELL_WHAT_I_AM_DOING_WHEN_INCLUDING_THIS_FILE
  private:
    
    //the callbacks
    nb_transport_tagged_functor<TRAITS>* m_nb_fw_functor;
    b_transport_tagged_functor<TRAITS>*  m_b_functor;
    get_direct_mem_ptr_tagged_functor<TRAITS>* m_get_dmi_functor;
    transport_dbg_tagged_functor<TRAITS>* m_dbg_functor;
    
};

template <typename TRAITS, typename CB_BINDER_BASE>
class ocp_base_callback_binder_fw<TRAITS, false, CB_BINDER_BASE> : public ocp_base_callback_binder_fw_base<TRAITS, CB_BINDER_BASE>{
#define OCP_I_KNOW_VERY_WELL_WHAT_I_AM_DOING_WHEN_INCLUDING_THIS_FILE
#include __MACRO_STRINGYFY__(../include/OCPIP_VERSION/base_socket/ocp_base_callback_fw_binders_body.h)
#undef OCP_I_KNOW_VERY_WELL_WHAT_I_AM_DOING_WHEN_INCLUDING_THIS_FILE
  private:
    //the callbacks
    nb_transport_functor<TRAITS>* m_nb_fw_functor;
    b_transport_functor<TRAITS>*  m_b_functor;
    get_direct_mem_ptr_functor<TRAITS>* m_get_dmi_functor;
    transport_dbg_functor<TRAITS>* m_dbg_functor;
};

/*
This class implements the bw interface.
It allows to register a callback for each of the bw interface methods.
The callbacks simply forward the bw interface call, but add the id (an int)
of the callback binder to the signature of the call.
*/

template <typename TRAITS, typename CB_BINDER_BASE>
class ocp_base_callback_binder_bw_base
  : public tlm::tlm_bw_transport_if<TRAITS>
  , public CB_BINDER_BASE
{
public:
    template <typename T>
    ocp_base_callback_binder_bw_base(int id, T* owner)
      : CB_BINDER_BASE(id, owner)
    {
    }
};


template <typename TRAITS, bool TAGGED, typename CB_BINDER_BASE>
class ocp_base_callback_binder_bw{};


template <typename TRAITS, typename CB_BINDER_BASE>
class ocp_base_callback_binder_bw<TRAITS, true, CB_BINDER_BASE>: public ocp_base_callback_binder_bw_base<TRAITS, CB_BINDER_BASE>{
#define OCP_I_KNOW_VERY_WELL_WHAT_I_AM_DOING_WHEN_INCLUDING_THIS_FILE
#define OCP_USE_WITH_TAG
#include __MACRO_STRINGYFY__(../include/OCPIP_VERSION/base_socket/ocp_base_callback_bw_binders_body.h)
#undef OCP_USE_WITH_TAG
#undef OCP_I_KNOW_VERY_WELL_WHAT_I_AM_DOING_WHEN_INCLUDING_THIS_FILE
  private:
    //the callbacks
    nb_transport_tagged_functor<TRAITS>* m_nb_bw_functor;
    invalidate_direct_mem_ptr_tagged_functor<TRAITS>* m_inval_dmi_functor;
};

template <typename TRAITS, typename CB_BINDER_BASE>
class ocp_base_callback_binder_bw<TRAITS, false, CB_BINDER_BASE>: public ocp_base_callback_binder_bw_base<TRAITS, CB_BINDER_BASE>{
#define OCP_I_KNOW_VERY_WELL_WHAT_I_AM_DOING_WHEN_INCLUDING_THIS_FILE
#include __MACRO_STRINGYFY__(../include/OCPIP_VERSION/base_socket/ocp_base_callback_bw_binders_body.h)
#undef OCP_I_KNOW_VERY_WELL_WHAT_I_AM_DOING_WHEN_INCLUDING_THIS_FILE
  private:
    //the callbacks
    nb_transport_functor<TRAITS>* m_nb_bw_functor;
    invalidate_direct_mem_ptr_functor<TRAITS>* m_inval_dmi_functor;
};


/*
This class forms the base for multi initiator sockets.
It enforces a multi initiator socket to implement all functions
needed to do hierarchical bindings.
*/
template <unsigned int BUSWIDTH = 32,
          typename TRAITS = tlm::tlm_base_protocol_types,
          unsigned int N=0,
          typename CB_BINDER_BASE = ocp_base_callback_binder_base
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
          ,sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND
#endif
          >
class multi_init_base: public tlm::tlm_initiator_socket<BUSWIDTH,
                                                  TRAITS,
                                                  N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
                                                  ,POL
#endif
                                                  >{
public:
  //typedef for the base type: the standard tlm initiator socket
  typedef tlm::tlm_initiator_socket<BUSWIDTH,
                              TRAITS,
                              N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
                              ,POL
#endif
                              > base_type;
  
  //this method shall disable the code that does the callback binding
  // that registers callbacks to binders
  virtual void disable_cb_bind()=0;
  
  //this method shall return the multi_init_base to which the
  // multi_init_base is bound hierarchically
  //  If the base is not bound hierarchically it shall return a pointer to itself
  virtual multi_init_base* get_hierarch_bind()=0;
  
  //this method shall return a vector of the callback binders of multi initiator socket
  virtual std::vector<ocp_base_callback_binder_bw<TRAITS, N!=1, CB_BINDER_BASE>* >& get_binders()=0;
  
  //this method shall return a vector of all target interfaces bound to this multi init socket
  virtual std::vector<tlm::tlm_fw_transport_if<TRAITS>*>&  get_sockets()=0;
  
  //ctor and dtor
  virtual ~multi_init_base(){}
  multi_init_base(const char* name):base_type(name){}
};

/*
This class forms the base for multi target sockets.
It enforces a multi target socket to implement all functions
needed to do hierarchical bindings.
*/
template <unsigned int BUSWIDTH = 32,
          typename TRAITS = tlm::tlm_base_protocol_types,
          unsigned int N=0,
          typename CB_BINDER_BASE = ocp_base_callback_binder_base
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
          ,sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND
#endif
          >
class multi_target_base: public tlm::tlm_target_socket<BUSWIDTH, 
                                                TRAITS,
                                                N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)                                                
                                                ,POL
#endif
                                                >{
public:
  //typedef for the base type: the standard tlm target socket
  typedef tlm::tlm_target_socket<BUSWIDTH, 
                              TRAITS,
                              N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
                              ,POL
#endif
                              > base_type;
  
  //this method shall return the multi_init_base to which the
  // multi_init_base is bound hierarchically
  //  If the base is not bound hierarchically it shall return a pointer to itself                                                
  virtual multi_target_base* get_hierarch_bind()=0;
  
  //this method shall inform the multi target socket that it is bound
  // hierarchically and to which other multi target socket it is bound hierarchically
  virtual void set_hierarch_bind(multi_target_base*)=0;
  
  //this method shall return a vector of the callback binders of multi initiator socket
  virtual std::vector<ocp_base_callback_binder_fw<TRAITS,N!=1,CB_BINDER_BASE>* >& get_binders()=0;
  
  //this method shall return a map of all multi initiator sockets that are bound to this multi target
  // the key of the map is the index at which the multi initiator i bound, while the value
  //  is the interface of the multi initiator socket that is bound at that index
  virtual std::map<unsigned int, tlm::tlm_bw_transport_if<TRAITS>*>&  get_multi_binds()=0;
  
  //ctor and dtor
  virtual ~multi_target_base(){}
  multi_target_base(const char* name):base_type(name){}
};

template <typename TRAITS, bool TAGGED>
struct multi_socket_functor_types{
};

template <typename TRAITS>
struct multi_socket_functor_types<TRAITS, true>{
  typedef nb_transport_tagged_functor<TRAITS> nb_functor_type;
  typedef b_transport_tagged_functor<TRAITS>  b_functor_type;
  typedef get_direct_mem_ptr_tagged_functor<TRAITS> get_dmi_functor_type;
  typedef transport_dbg_tagged_functor<TRAITS> dbg_functor_type;
  typedef invalidate_direct_mem_ptr_tagged_functor<TRAITS> inval_dmi_functor_type;
};

template <typename TRAITS>
struct multi_socket_functor_types<TRAITS, false>{
  typedef nb_transport_functor<TRAITS> nb_functor_type;
  typedef b_transport_functor<TRAITS>  b_functor_type;
  typedef get_direct_mem_ptr_functor<TRAITS> get_dmi_functor_type;
  typedef transport_dbg_functor<TRAITS> dbg_functor_type;
  typedef invalidate_direct_mem_ptr_functor<TRAITS> inval_dmi_functor_type;
};

template <typename MODULE, typename TRAITS, bool TAGGED>
struct multi_socket_function_signatures{};

template <typename MODULE, typename TRAITS>
struct multi_socket_function_signatures<MODULE, TRAITS, true>{
  typedef typename TRAITS::tlm_payload_type              transaction_type;
  typedef typename TRAITS::tlm_phase_type                phase_type;
  typedef tlm::tlm_sync_enum                            sync_enum_type;

  typedef sync_enum_type (MODULE::*nb_cb)(unsigned int, transaction_type&, phase_type&, sc_core::sc_time&);
  typedef void (MODULE::*b_cb)(unsigned int, transaction_type&, sc_core::sc_time&);
  typedef unsigned int (MODULE::*dbg_cb)(unsigned int, transaction_type& txn);
  typedef bool (MODULE::*get_dmi_cb)(unsigned int, transaction_type& txn, tlm::tlm_dmi& dmi);
  typedef void (MODULE::*inval_dmi_cb)(unsigned int, sc_dt::uint64, sc_dt::uint64);
};

template <typename MODULE, typename TRAITS>
struct multi_socket_function_signatures<MODULE, TRAITS, false>{
  typedef typename TRAITS::tlm_payload_type              transaction_type;
  typedef typename TRAITS::tlm_phase_type                phase_type;
  typedef tlm::tlm_sync_enum                            sync_enum_type;

  typedef sync_enum_type (MODULE::*nb_cb)(transaction_type&, phase_type&, sc_core::sc_time&);
  typedef void (MODULE::*b_cb)(transaction_type&, sc_core::sc_time&);
  typedef unsigned int (MODULE::*dbg_cb)(transaction_type& txn);
  typedef bool (MODULE::*get_dmi_cb)(transaction_type& txn, tlm::tlm_dmi& dmi);
  typedef void (MODULE::*inval_dmi_cb)(sc_dt::uint64, sc_dt::uint64);
};


}
}

