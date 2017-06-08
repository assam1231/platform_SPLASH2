//////////////////////////////////////////////////////////////////////////////
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
//  Description :  
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_interface_wrapper.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif


namespace OCPIP_VERSION{


template <typename BASE_SOCKET, typename PEQ, bool TAGGED>
class fw_interface_wrapper:
  public tlm::tlm_fw_transport_if<tlm::tlm_base_protocol_types>
{
public:
  typedef tlm::tlm_base_protocol_types::tlm_payload_type payload_type;
  typedef tlm::tlm_base_protocol_types::tlm_phase_type   phase_type;
  
  fw_interface_wrapper(
    BASE_SOCKET* socket,
    PEQ*&         peq,
    ocp_layer_ids layer
  )
    : m_socket(socket)
    , m_peq(peq)
    , index(0)
    , m_layer(layer)
  {
  }

  virtual tlm::tlm_sync_enum nb_transport_fw(payload_type& trans,
                                        phase_type& phase,
                                        sc_core::sc_time& t)
  {
    //if (m_layer>ocp_tl2) //from tl3 and above everything may happen
      //return m_socket->operator[](index)->nb_transport_fw(trans,phase,t);
    sc_core::sc_time in_time=t;
    tlm::tlm_sync_enum retVal=m_socket->operator[](index)->nb_transport_fw(trans,phase,t);
    if (retVal==tlm::TLM_COMPLETED && m_layer<ocp_tl3){ 
      SC_REPORT_ERROR(m_socket->name(), "Got forbidden TLM_COMPLETED.");
    }
    if (retVal==tlm::TLM_UPDATED && (t!=in_time) && m_peq!=NULL){
      if (TAGGED)
        m_peq->notify(index, trans, phase, t);
      else
        m_peq->notify(trans, phase, t);
      return tlm::TLM_ACCEPTED;
    }
    return retVal;
  }
                                        
                                        
  virtual void b_transport(payload_type& trans,sc_core::sc_time& t){
    m_socket->operator[](index)->b_transport(trans, t);
  }
                    
  virtual bool get_direct_mem_ptr(payload_type& trans, tlm::tlm_dmi&  dmi_data){
    return m_socket->operator[](index)->get_direct_mem_ptr(trans, dmi_data);
  }
                                  
  virtual unsigned int transport_dbg(payload_type& trans){
    return m_socket->operator[](index)->transport_dbg(trans);
  }
  
//  void set_has_peq(bool val){m_has_peq=val;}
  
  
  fw_interface_wrapper* operator[](unsigned int i){ index=i; return this;}
protected:
  BASE_SOCKET* m_socket;
  PEQ*&        m_peq;
  unsigned int index;
  ocp_layer_ids m_layer;
};

template <typename BASE_SOCKET, typename PEQ, bool TAGGED>
class bw_interface_wrapper:
  public tlm::tlm_bw_transport_if<tlm::tlm_base_protocol_types>
{
public:
  typedef tlm::tlm_base_protocol_types::tlm_payload_type payload_type;
  typedef tlm::tlm_base_protocol_types::tlm_phase_type   phase_type;
  
  bw_interface_wrapper(
    BASE_SOCKET* socket,
    PEQ*& peq,
    ocp_layer_ids layer
  )
    : m_socket(socket)
    , m_peq(peq)
    , index(0)
    , m_layer(layer)
  {
  }

  virtual tlm::tlm_sync_enum nb_transport_bw(payload_type& trans,
                                        phase_type& phase,
                                        sc_core::sc_time& t)
  {
    //if (m_layer>ocp_tl2) //from tl3 and above everything may happen
      //return m_socket->operator[](index)->nb_transport_bw(trans,phase,t);
    sc_core::sc_time in_time=t;
    tlm::tlm_sync_enum retVal=m_socket->operator[](index)->nb_transport_bw(trans,phase,t);
    if (retVal==tlm::TLM_COMPLETED && m_layer<ocp_tl3){
      SC_REPORT_ERROR(m_socket->name(), "Got forbidden TLM_COMPLETED.");
    }
    if (retVal==tlm::TLM_UPDATED && (t!=in_time) && m_peq!=NULL){
      if (TAGGED)
        m_peq->notify(index, trans, phase, t);
      else
        m_peq->notify(trans, phase, t);
      return tlm::TLM_ACCEPTED;
    }
    return retVal;
  }


  virtual void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                         sc_dt::uint64 end_range){
    m_socket->operator[](index)->invalidate_direct_mem_ptr(start_range, end_range);
  }
  
//  void set_has_peq(bool val){m_has_peq=val;}
  
  bw_interface_wrapper* operator[](unsigned int i){ index=i; return this;}
protected:
  BASE_SOCKET* m_socket;
  PEQ*&        m_peq;
  unsigned int index;
  ocp_layer_ids m_layer;
};

}
