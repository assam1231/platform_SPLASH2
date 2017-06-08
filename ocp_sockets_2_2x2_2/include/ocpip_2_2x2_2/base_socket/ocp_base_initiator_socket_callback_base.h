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
  #error ocp_base_initiator_socket_callback_base.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION {
namespace infr{

/*
This class implements a trivial multi initiator socket.
The triviality refers to the fact that the socket does not
do blocking to non-blocking or non-blocking to blocking conversions.

It allows to connect multiple targets to this socket.
The user has to register callbacks for the bw interface methods
he likes to use. The callbacks are basically equal to the bw interface
methods but carry an additional integer that indicates to which
index of this socket the calling target is connected.
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
class initiator_socket_callback_base: public multi_init_base< BUSWIDTH, 
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
  typedef multi_init_base<BUSWIDTH, 
                        TRAITS,
                        N,
                        CB_BINDER_BASE
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
                        ,POL
#endif
                        > base_type;

  typedef typename base_type::base_target_socket_type base_target_socket_type;
  typedef ocp_base_callback_binder_bw<TRAITS,N!=1,CB_BINDER_BASE> ocp_base_callback_binder_bw_type;

  //CTOR
  template <typename T>
  initiator_socket_callback_base(const char* name, T* owner)
      : base_type((std::string(name)).c_str())
      , m_hierarch_bind(0)
      , m_beoe_disabled(false)
      , m_beoe_done(false)
      , m_dummy(42, static_cast<void*>(owner))
      , m_owner(static_cast<void*>(owner))
  {
  }

  ~initiator_socket_callback_base(){
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


  //register callback for nb transport of bw interface
  template<typename MODULE>
  void register_nb_transport_bw(MODULE* mod,
                               typename multi_socket_function_signatures<MODULE, TRAITS, N!=1>::nb_cb cb
                                                           )
  {
    //make sure that only one module is registering callbacks with this socket
    m_nb_bw_functor.set_function(mod, cb);
  }

  //register callback for dmi function of bw interface
  template<typename MODULE>
  void register_invalidate_direct_mem_ptr(MODULE* mod,
                               typename multi_socket_function_signatures<MODULE, TRAITS, N!=1>::inval_dmi_cb cb
  )
  {
    m_inval_dmi_functor.set_function(mod, cb);
  }

  //Override virtual functions of the tlm_initiator_socket:
  // this function is called whenever an sc_port (as part of a target socket)
  //  wants to bind to the export of the underlying tlm_initiator_socket
  //At this time a callback binder is created an returned to the sc_port
  // of the target socket, so that it binds to the callback binder
  virtual tlm::tlm_bw_transport_if<TRAITS>& get_base_interface()
  {
    m_binders.push_back(new ocp_base_callback_binder_bw_type(m_binders.size(), m_owner));
    return *m_binders[m_binders.size()-1];
  }

  //Override virtual functions of the tlm_initiator_socket:
  // this function is called whenever an sc_export (as part of a initiator socket)
  //  wants to bind to the export of the underlying tlm_initiator_socket
  //   i.e. a hierarchical bind takes place
  virtual sc_core::sc_export<tlm::tlm_bw_transport_if<TRAITS> >& get_base_export()
  {
    if (!m_beoe_disabled) //we are not bound hierarchically
      base_type::m_export.bind(m_dummy);  //so we bind the dummy to avoid a SystemC error
    return base_type::m_export; //and then return our own export so that the hierarchical binding is set up properly
  }

  //bind against a target socket
  void bind(base_target_socket_type& s)
  {
    //error if this socket is already bound hierarchically
    if (m_hierarch_bind)
      display_error("Already hierarchically bound.");     
       
    base_type::bind(s); //satisfy systemC, leads to a call to get_base_interface()
    
    //try to cast the target socket into a fw interface
    sc_core::sc_export<tlm::tlm_fw_transport_if<TRAITS> >* p_ex_s=dynamic_cast<sc_core::sc_export<tlm::tlm_fw_transport_if<TRAITS> >*>(&s);
    if (!p_ex_s) display_error("Multi socket not bound to tlm_socket.");
    
    //try a cast into a multi sockets
    MULTI_MULTI_BASE* test=dynamic_cast<MULTI_MULTI_BASE*> (p_ex_s);
    if (test){ //did we just do a multi-multi bind??
      //if that is the case the multi target socket must have just created a callback binder
      // which we want to get from it.
      //Moreover, we also just created one, which we will pass to it.
      m_sockets.push_back(test->get_last_binder(m_binders[m_binders.size()-1]));
    }
    else{  // if not just bind normally
      sc_core::sc_export<tlm::tlm_fw_transport_if<TRAITS> >& ex_s=*p_ex_s;
      m_sockets.push_back(&((tlm::tlm_fw_transport_if<TRAITS>&)ex_s)); //store the interface we are bound against
    }
  }

  //operator notation for direct bind
  void operator() (base_target_socket_type& s)
  {
    bind(s);
  }
  
  //SystemC standard callback before end of elaboration
  void before_end_of_elaboration(){
    if (m_beoe_done) return;
    m_beoe_done=true;
    //if our export hasn't been bound yet (due to a hierarch binding)
    // we bind it now to avoid a SystemC error.
    //We must do that, because it is legal not to register a callback on this socket
    // as the user might only use b_transport
    if (!base_type::m_export.get_interface()){ 
      base_type::m_export.bind(m_dummy);      
    }

    //get the interfaces bound to the top of the hierachical bind chain
    // NOTE: this could be the same socket if there is no hierachical bind
    m_used_sockets=get_hierarch_bind()->get_sockets();
    
    //'break' here if the socket was told not to do callback binding
    if (m_beoe_disabled) return;
    
    //get the callback binders of the top of the hierachical bind chain
    // NOTE: this could be the same socket if there is no hierachical bind
    std::vector<ocp_base_callback_binder_bw_type* >& binders=get_hierarch_bind()->get_binders();

    
    //register the callbacks of this socket with the callback binders
    // we just got from the top of the hierachical bind chain
    for (unsigned int i=0; i<binders.size(); i++) {
      binders[i]->set_owner(m_owner);
      binders[i]->set_callbacks(m_nb_bw_functor, m_inval_dmi_functor);
    }
  }

  void end_of_elaboration(){}

  //
  // Bind multi initiator socket to multi initiator socket (hierarchical bind)
  //
  void bind(base_type& s)
  {
    if (m_binders.size()) //a multi socket is either bound hierarchically or directly
      display_error("Socket already directly bound.");
    if (m_hierarch_bind){
      display_warning("Socket already bound hierarchically. Bind attempt ignored.");
      return;
    }
    
    //remember to which socket we are hierarchically bound and disable it, 
    // so that it won't try to register callbacks itself
    s.disable_cb_bind();
    m_hierarch_bind=&s;    
    base_type::bind(s); //satisfy SystemC
  }

  //operator notation for hierarchical bind
  void operator() (base_type& s)
  {
    bind(s);
  }

  //get access to sub port
  tlm::tlm_fw_transport_if<TRAITS>* operator[](int i){
    return m_used_sockets[i];
  }
  tlm::tlm_fw_transport_if<TRAITS>* operator->(){return m_used_sockets[0];}
  
  //get the number of bound targets
  // NOTE: this is only valid at end of elaboration!
  unsigned int size() {return get_hierarch_bind()->get_sockets().size();}


  sc_core::sc_interface* get_bottom_of_hierarchy(){
    return base_type::m_export.get_interface();
  }


protected:
  //implementation of base class interface
  base_type* get_hierarch_bind(){if (m_hierarch_bind) return m_hierarch_bind->get_hierarch_bind(); else return this;}
  void disable_cb_bind(){ m_beoe_disabled=true;}
  std::vector<ocp_base_callback_binder_bw_type* >& get_binders(){return m_binders;}
  std::vector<tlm::tlm_fw_transport_if<TRAITS>*>& get_sockets(){return m_sockets;}
  //vector of connected sockets
  std::vector<tlm::tlm_fw_transport_if<TRAITS>*> m_sockets;
  std::vector<tlm::tlm_fw_transport_if<TRAITS>*> m_used_sockets;
  //vector of binders that convert untagged interface into tagged interface
  std::vector<ocp_base_callback_binder_bw_type*> m_binders;
  
  base_type*  m_hierarch_bind; //pointer to hierarchical bound multi port
  bool m_beoe_disabled, m_beoe_done;  // bool that remembers whether this socket shall bind callbacks or not
  ocp_base_callback_binder_bw_type m_dummy; //a callback binder that is bound to the underlying export
                                     // in case there was no real bind

  //callbacks as boost functions
  // (allows to pass the callback to another socket that does not know the type of the module that owns
  //  the callbacks)
  typename multi_socket_functor_types<TRAITS, N!=1>::nb_functor_type m_nb_bw_functor;
  typename multi_socket_functor_types<TRAITS, N!=1>::inval_dmi_functor_type m_inval_dmi_functor;
  void* m_owner;

};

}
}

