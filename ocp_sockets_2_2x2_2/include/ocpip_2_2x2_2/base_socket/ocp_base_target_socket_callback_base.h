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
  #error ocp_base_target_socket_callback_base.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION {
namespace infr{

/*
This class implements a trivial multi target socket.
The triviality refers to the fact that the socket does not
do blocking to non-blocking or non-blocking to blocking conversions.

It allows to connect multiple initiators to this socket.
The user has to register callbacks for the fw interface methods
he likes to use. The callbacks are basically equal to the fw interface
methods but carry an additional integer that indicates to which
index of this socket the calling initiator is connected.
*/
template <unsigned int BUSWIDTH = 32,
          typename TRAITS = tlm::tlm_base_protocol_types,
          unsigned int N=0,
          typename CB_BINDER_BASE=ocp_base_callback_binder_base,
          typename MULTI_MULTI_BASE = ocp_base_multi_to_multi_bind_base<TRAITS>
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
          ,sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND
#endif
          >
class target_socket_callback_base: public multi_target_base< BUSWIDTH,
                                                        TRAITS,
                                                        N,
                                                        CB_BINDER_BASE
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
                                                        ,POL
#endif
                                                        >
                              , public MULTI_MULTI_BASE
{

public:

  //typedefs
  //  tlm 2.0 types for nb_transport
  typedef TRAITS                                         traits_type;
  typedef typename TRAITS::tlm_payload_type              transaction_type;
  typedef typename TRAITS::tlm_phase_type                phase_type;
  typedef tlm::tlm_sync_enum                            sync_enum_type;

  typedef multi_target_base<BUSWIDTH,
                        TRAITS,
                        N,
                        CB_BINDER_BASE
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
                        ,POL
#endif
                        > base_type;

  typedef typename base_type::base_initiator_socket_type base_initiator_socket_type;
  typedef ocp_base_callback_binder_fw<TRAITS,N!=1,CB_BINDER_BASE> ocp_base_callback_binder_fw_type;

  //CTOR
  template<typename T>
  target_socket_callback_base(const char* name, T* owner)
      : base_type((std::string(name)).c_str())
      , m_hierarch_bind(0)
      , m_eoe_disabled(false)
      , m_beoe_done(false)
      , m_eoe_done(false)
      , m_dummy(42, static_cast<void*>(owner))
      , m_owner(static_cast<void*>(owner))
  {
  }

  ~target_socket_callback_base(){
    //clean up everything allocated by 'new'
    for (unsigned int i=0; i<m_binders.size(); i++) delete m_binders[i];
  }

  //simple helpers for warnings an errors to shorten in code notation
  void display_warning(const std::string& text){
    std::stringstream s;
    s<<"WARNING in instance "<<base_type::name()<<": "<<text;
    SC_REPORT_WARNING("ocp_base_multi_socket", s.str().c_str());
  }

  void display_error(const std::string& text){
    std::stringstream s;
    s<<"ERROR in instance "<<base_type::name()<<": "<<text;
    SC_REPORT_ERROR("ocp_base_multi_socket", s.str().c_str());
  }

  //register callback for nb transport of fw interface
  template<typename MODULE>
  void register_nb_transport_fw(MODULE* mod,
                              //sync_enum_type (MODULE::*cb)(int, transaction_type&, phase_type&, sc_core::sc_time&)
                              typename multi_socket_function_signatures<MODULE,TRAITS,N!=1>::nb_cb cb)
  {
    //if our export hasn't been bound yet (due to a hierarch binding)
    //  we bind it now.
    //We do that here as the user of the target port HAS to bind at least on callback,
    //otherwise the socket was useless. Nevertheless, the target socket may still
    // stay unbound afterwards.
    if (!sc_core::sc_export<tlm::tlm_fw_transport_if<TRAITS> >::get_interface())
      sc_core::sc_export<tlm::tlm_fw_transport_if<TRAITS> >::bind(m_dummy);
    
    m_nb_fw_functor.set_function(mod, cb);
  }

  //register callback for b transport of fw interface
  template<typename MODULE>
  void register_b_transport(MODULE* mod,
                              //void (MODULE::*cb)(int, transaction_type&, sc_core::sc_time&)
                              typename multi_socket_function_signatures<MODULE,TRAITS,N!=1>::b_cb cb)
  {
    //if our export hasn't been bound yet (due to a hierarch binding)
    //  we bind it now.
    //We do that here as the user of the target port HAS to bind at least on callback,
    //otherwise the socket was useless. Nevertheless, the target socket may still
    // stay unbound afterwards.
    if (!sc_core::sc_export<tlm::tlm_fw_transport_if<TRAITS> >::get_interface())
      sc_core::sc_export<tlm::tlm_fw_transport_if<TRAITS> >::bind(m_dummy);
    
    m_b_functor.set_function(mod,cb);
  }

  //register callback for debug transport of fw interface
  template<typename MODULE>
  void register_transport_dbg(MODULE* mod,
                              //unsigned int (MODULE::*cb)(int, transaction_type& txn)
                              typename multi_socket_function_signatures<MODULE,TRAITS,N!=1>::dbg_cb cb)
  {
    //if our export hasn't been bound yet (due to a hierarch binding)
    //  we bind it now.
    //We do that here as the user of the target port HAS to bind at least on callback,
    //otherwise the socket was useless. Nevertheless, the target socket may still
    // stay unbound afterwards.
    if (!sc_core::sc_export<tlm::tlm_fw_transport_if<TRAITS> >::get_interface())
      sc_core::sc_export<tlm::tlm_fw_transport_if<TRAITS> >::bind(m_dummy);
    m_dbg_functor.set_function(mod,cb);
  }

  //register callback for DMI of fw interface
  template<typename MODULE>
  void register_get_direct_mem_ptr(MODULE* mod,
                                   //bool (MODULE::*cb)(int, transaction_type& txn, tlm::tlm_dmi& dmi)
                                   typename multi_socket_function_signatures<MODULE,TRAITS,N!=1>::get_dmi_cb cb)
  {
    //if our export hasn't been bound yet (due to a hierarch binding)
    //  we bind it now.
    //We do that here as the user of the target port HAS to bind at least on callback,
    //otherwise the socket was useless. Nevertheless, the target socket may still
    // stay unbound afterwards.
    if (!sc_core::sc_export<tlm::tlm_fw_transport_if<TRAITS> >::get_interface())
      sc_core::sc_export<tlm::tlm_fw_transport_if<TRAITS> >::bind(m_dummy);
    m_get_dmi_functor.set_function(mod,cb);
  }


  //Override virtual functions of the tlm_target_socket:
  // this function is called whenever an sc_port (as part of a init socket)
  //  wants to bind to the export of the underlying tlm_target_socket
  //At this time a callback binder is created an returned to the sc_port
  // of the init socket, so that it binds to the callback binder
  virtual tlm::tlm_fw_transport_if<TRAITS>& get_base_interface()
  {
    //error if this socket is already bound hierarchically
    if (m_hierarch_bind) display_error("Socket already bound hierarchically.");
    m_binders.push_back(new ocp_base_callback_binder_fw_type(m_binders.size(), m_owner));
    return *m_binders[m_binders.size()-1];
  }

  //just return the export of the underlying tlm_target_socket in case of a hierarchical bind
  virtual sc_core::sc_export<tlm::tlm_fw_transport_if<TRAITS> >& get_base_export()
  {
    return *this;
  }

  void before_end_of_elaboration(){
    if (m_eoe_disabled | m_beoe_done) return;
    m_beoe_done=true;
    std::vector<ocp_base_callback_binder_fw_type* >& binders=get_hierarch_bind()->get_binders();
    for (unsigned int i=0; i<binders.size(); i++) {
      binders[i]->set_owner(m_owner);
      binders[i]->set_callbacks(m_nb_fw_functor, m_b_functor, m_get_dmi_functor, m_dbg_functor); //set the callbacks for the binder
    }
  }

  //the standard end of elaboration callback
  void end_of_elaboration(){
    //'break' here if the socket was told not to do callback binding
    if (m_eoe_disabled | m_eoe_done) return;
    m_eoe_done=true;

    //get the callback binders and the multi binds of the top of the hierachical bind chain
    // NOTE: this could be the same socket if there is no hierachical bind
    std::vector<ocp_base_callback_binder_fw_type* >& binders=get_hierarch_bind()->get_binders();
    std::map<unsigned int, tlm::tlm_bw_transport_if<TRAITS>*>&  multi_binds=get_hierarch_bind()->get_multi_binds();

    //iterate over all binders
    for (unsigned int i=0; i<binders.size(); i++) {
//      binders[i]->set_owner(m_owner);
//      binders[i]->set_callbacks(m_nb_fw_functor, m_b_functor, m_get_dmi_functor, m_dbg_functor); //set the callbacks for the binder
      if (multi_binds.find(i)!=multi_binds.end()) //check if this connection is multi-multi
        //if so remember the interface
        m_sockets.push_back(multi_binds[i]);
      else{ //if we are bound to a normal socket
        //get the calling port and try to cast it into a tlm socket base
        base_initiator_socket_type* test=dynamic_cast<base_initiator_socket_type*>(binders[i]->get_other_side());
        if (!test){display_error("Not bound to tlm_socket.");}
        m_sockets.push_back(&test->get_base_interface()); //remember the interface
      }
    }
  }

  //
  // Bind multi target socket to multi target socket (hierarchical bind)
  //
  void bind(base_type& s)
  {
    //warn if already bound hierarchically
    if (m_eoe_disabled){
      display_warning("Socket already bound hierarchically. Bind attempt ignored.");
      return;
    }

    //disable our own end of elaboration call
    disable_cb_bind();

    //inform the bound target socket that it is bound hierarchically now
    s.set_hierarch_bind((base_type*)this);
    base_type::bind(s); //satisfy SystemC
  }

  //operator notation for hierarchical bind
  void operator() (base_type& s)
  {
    bind(s);
  }

  //get access to sub port
  tlm::tlm_bw_transport_if<TRAITS>* operator[](int i){return m_sockets[i];}
  tlm::tlm_bw_transport_if<TRAITS>* operator->(){return m_sockets[0];}
  //get number of bound initiators
  // NOTE: this is only valid at end of elaboration!
  unsigned int size(){return get_hierarch_bind()->get_binders().size();}


  sc_core::sc_interface* get_bottom_of_hierarchy(){
    return ((base_type*)this)->get_interface();
  }

protected:

  //implementation of base class interface
  base_type* get_hierarch_bind(){if (m_hierarch_bind) return m_hierarch_bind->get_hierarch_bind(); else return this;}

  std::map<unsigned int, tlm::tlm_bw_transport_if<TRAITS>*>&  get_multi_binds(){return m_multi_binds;}
  void set_hierarch_bind(base_type* h){m_hierarch_bind=h;}
  tlm::tlm_fw_transport_if<TRAITS>* get_last_binder(tlm::tlm_bw_transport_if<TRAITS>* other){
    m_multi_binds[m_binders.size()-1]=other;
    return m_binders[m_binders.size()-1];
  }

  //map that stores to which index a multi init socket is connected
  // and the interface of the multi init socket
  std::map<unsigned int, tlm::tlm_bw_transport_if<TRAITS>*> m_multi_binds;

  void disable_cb_bind(){ m_eoe_disabled=true;}
  std::vector<ocp_base_callback_binder_fw_type* >& get_binders(){return m_binders;}
  //vector of connected sockets
  std::vector<tlm::tlm_bw_transport_if<TRAITS>*> m_sockets;
  //vector of binders that convert untagged interface into tagged interface
  std::vector<ocp_base_callback_binder_fw_type*> m_binders;

  base_type*  m_hierarch_bind; //pointer to hierarchical bound multi port
  bool m_eoe_disabled, m_beoe_done, m_eoe_done; //bool that diables callback bindings at end of elaboration
  ocp_base_callback_binder_fw_type m_dummy; //a dummy to bind to the export

  //callbacks as boost functions
  // (allows to pass the callback to another socket that does not know the type of the module that owns
  //  the callbacks)
  typename multi_socket_functor_types<TRAITS, N!=1>::nb_functor_type m_nb_fw_functor;
  typename multi_socket_functor_types<TRAITS, N!=1>::b_functor_type  m_b_functor;
  typename multi_socket_functor_types<TRAITS, N!=1>::get_dmi_functor_type m_get_dmi_functor;
  typename multi_socket_functor_types<TRAITS, N!=1>::dbg_functor_type m_dbg_functor;
  void* m_owner;
};

}
}

