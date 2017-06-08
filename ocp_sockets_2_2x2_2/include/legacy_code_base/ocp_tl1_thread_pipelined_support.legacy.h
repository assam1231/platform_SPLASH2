///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// (c) Copyright OCP-IP 2006
// OCP-IP Confidential and Proprietary
//
//
//============================================================================
//      Project : OCP SLD WG
//       Author : Robert Guenzel (from Technical Univerity of Braunschweig) for Greensocs Ltd.
//
//          $Id:
//
//  Description :  This class encapsulates all functionality that enables
//                 a clocked TL1 channel to keep track of tthe pipelined
//                 thread busy protocol
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
#ifndef __OCP_TL1_THREAD_PIPELINED_SUPPORT_LEGACY_H__
#define __OCP_TL1_THREAD_PIPELINED_SUPPORT_LEGACY_H__

class OCP_TL1_Thread_pipelined_support : public sc_core::sc_module {
  public:
    SC_HAS_PROCESS(OCP_TL1_Thread_pipelined_support);
    OCP_TL1_Thread_pipelined_support(sc_core::sc_module_name name, sc_core::sc_signal<bool>& _Clk) :
      sc_core::sc_module(name),
      m_pClk(&_Clk),
      m_ThreadBusy_pipelined(0),
      m_ThreadBusy_pipelined_next(0),
      update_threadbusy_pipelined_state(false)
      {
        SC_METHOD(update_threadbusy_pipelined);
        sensitive<<update_threadbusy_pipelined_event;
        dont_initialize();
        update_threadbusy_pipelined_state=false;
        m_do_cast=false;
        m_skip_update=false;
      }
      
    OCP_TL1_Thread_pipelined_support(sc_core::sc_module_name name, sc_core::sc_in_clk& _Clk) :
      sc_core::sc_module(name),
      m_ThreadBusy_pipelined(0),
      m_ThreadBusy_pipelined_next(0),
      update_threadbusy_pipelined_state(false)
      {
        m_pInClkTmp=&_Clk;
        SC_METHOD(update_threadbusy_pipelined);
        sensitive<<update_threadbusy_pipelined_event;
        dont_initialize();
        update_threadbusy_pipelined_state=false;
        m_do_cast=true;
        m_skip_update=false;        
      }

#ifdef OSCI20
    OCP_TL1_Thread_pipelined_support(sc_core::sc_module_name name, sc_clock& _Clk) :
      sc_core::sc_module(name),
      m_ThreadBusy_pipelined(0),
      m_ThreadBusy_pipelined_next(0),
      update_threadbusy_pipelined_state(false)
      {
        m_pScClk=&_Clk;
        SC_METHOD(update_threadbusy_pipelined);
        sensitive<<update_threadbusy_pipelined_event;
        dont_initialize();
        update_threadbusy_pipelined_state=false;
        m_skip_update=false;
        m_pClk=NULL;    
      }
#endif

  void end_of_elaboration(){
    if(m_do_cast){
#ifdef OSCI20
      m_pScClk=dynamic_cast<sc_clock*>(m_pInClkTmp->get_interface());
      if(!m_pScClk) { std::cout<<"Could not retrieve an sc_clock from given port"<<std::endl<<std::flush; abort();}
      m_pClk=NULL;
#else
      m_pClk=dynamic_cast<sc_core::sc_signal<bool>*>(m_pInClkTmp->get_interface());
      if(!m_pClk) { std::cout<<"Could not retrieve an sc_core::sc_signal from given port"<<std::endl<<std::flush; abort();}
#endif
    }
 
  }
  
  void update_threadbusy_pipelined(){
    if(update_threadbusy_pipelined_state){
      update_threadbusy_pipelined_state=false;
      if(!m_skip_update) {
        m_ThreadBusy_pipelined=m_ThreadBusy_pipelined_next;
        changeEvent.notify();
      }
      else m_skip_update=false;
    }
    else{
      update_threadbusy_pipelined_state=true;
#ifdef OSCI20
      if(m_pClk==NULL)
        next_trigger((*m_pScClk).posedge_event());      
      else
        next_trigger((*m_pClk).posedge_event());        
#else
      next_trigger((*m_pClk).posedge_event());
#endif
    }
  }
  
  bool isThisThreadBusy( int threadNum)
  {
    unsigned int threadField;
#ifdef OSCI20
    if(m_pClk==NULL)
      threadField=(m_pScClk->event() && m_pScClk->read() && update_threadbusy_pipelined_state && !m_skip_update)? m_ThreadBusy_pipelined_next : m_ThreadBusy_pipelined;    
    else
      threadField=(m_pClk->event() && m_pClk->read() && update_threadbusy_pipelined_state && !m_skip_update)? m_ThreadBusy_pipelined_next : m_ThreadBusy_pipelined;    
#else
    //                                    posedge clk         BUT    update has not taken place yet
    threadField=(m_pClk->event() && m_pClk->read() && update_threadbusy_pipelined_state && !m_skip_update)? m_ThreadBusy_pipelined_next : m_ThreadBusy_pipelined;
#endif
    return ( 1 & (threadField >> threadNum) );
  }
  
  unsigned int getThisCyclesThreadBusy(){
#ifdef OSCI20
    if(m_pClk==NULL)
      return (m_pScClk->event() && m_pScClk->read() && update_threadbusy_pipelined_state && !m_skip_update)? m_ThreadBusy_pipelined_next : m_ThreadBusy_pipelined;    
    else
      return (m_pClk->event() && m_pClk->read() && update_threadbusy_pipelined_state && !m_skip_update)? m_ThreadBusy_pipelined_next : m_ThreadBusy_pipelined;    
#else
    return (m_pClk->event() && m_pClk->read() && update_threadbusy_pipelined_state && !m_skip_update)? m_ThreadBusy_pipelined_next : m_ThreadBusy_pipelined;
#endif
  }
  
  void reset(){
    m_ThreadBusy_pipelined=0;
    m_ThreadBusy_pipelined_next=0;
  }
  
  void setNewValue(unsigned int newValue){
#ifdef OSCI20
    if(m_pClk==NULL){
      if (m_pScClk->event() && m_pScClk->read() && update_threadbusy_pipelined_state && !m_skip_update) {//pending update
        m_ThreadBusy_pipelined=m_ThreadBusy_pipelined_next;
        changeEvent.notify(); 
        m_skip_update=true;
      }
    }
    else{
      if (m_pClk->event() && m_pClk->read() && update_threadbusy_pipelined_state && !m_skip_update) {//pending update
        m_ThreadBusy_pipelined=m_ThreadBusy_pipelined_next;
        changeEvent.notify(); 
        m_skip_update=true;
      }
    }
#else
    if (m_pClk->event() && m_pClk->read() && update_threadbusy_pipelined_state && !m_skip_update) {//pending update
      m_ThreadBusy_pipelined=m_ThreadBusy_pipelined_next;
      changeEvent.notify(); 
      m_skip_update=true;
    }
#endif
    m_ThreadBusy_pipelined_next=newValue;
    update_threadbusy_pipelined_event.notify(sc_core::SC_ZERO_TIME);  
  }
  
  sc_core::sc_event& delayedChangeEvent(){return changeEvent;}
      
  private:
    sc_core::sc_signal<bool>* m_pClk;
#ifdef OSCI20    
    sc_clock* m_pScClk;
#endif
    sc_core::sc_in_clk* m_pInClkTmp;
    unsigned int m_ThreadBusy_pipelined;
    unsigned int m_ThreadBusy_pipelined_next;
    bool update_threadbusy_pipelined_state;
    sc_core::sc_event update_threadbusy_pipelined_event, changeEvent;
    bool m_do_cast, m_skip_update;
};

#endif
