///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// (c) Copyright OCP-IP 2003, 2004, 2005
// OCP-IP Confidential and Proprietary
//
//
//============================================================================
//      Project : OCP SLD WG
//       Author : Tim Kogel, CoWare Inc.
//                
//          $Id:
//
//  Description : Wrapper for the OCP TL 1 SystemC Channel Model 
//  This wrapper provides a clean SystemC interface for the TL1
//  channel, i.e. it has a clock port instead of a pointer to a
//  clock port/object/signal as constructor argument.  The wrapper also
//  provides a setConfiguration method, which takes the name of the
//  parameters file as an argument and passes the map made from this
//  file to the setConfiguration method of the OCP TL1 Channel.
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __OCP_TL1_CHANNEL_CLOCKED_LEGACY_H__
#define __OCP_TL1_CHANNEL_CLOCKED_LEGACY_H__


template<typename TdataCl>
class OCP_TL1_Channel_Clocked : public  OCP_TL1_Channel<TdataCl>

{
  void readMapFromFile(const std::string &myFileName, MapStringType &myParamMap)
  {
    /* read pairs of data from the passed file */
    std::string leftside;
    std::string rightside;

    /* open the file */
    ifstream inputfile(myFileName.c_str());
    assert( inputfile );

    /* set the formatting */
    inputfile.setf(std::ios::skipws);

    /* Now read through all the pairs of values and add them to the passed map */
    while ( inputfile ) {
      inputfile >> leftside;
      inputfile >> rightside;
      myParamMap.insert(std::make_pair(leftside,rightside));
    }

    /* All done, close up */
    inputfile.close();
  }

public:
  sc_core::sc_in<bool> p_clk;

  OCP_TL1_Channel_Clocked( sc_core::sc_module_name name,
			   /* Name of Monitor file for TL1 channel. */
			   std::string monFileName = ""):
    OCP_TL1_Channel<TdataCl>(name),
    p_clk("p_clk")
  {
    this->m_in_clk = &p_clk;
    if (! monFileName.empty() ) {
	cerr << "Integrated Trace Monitor is deprecated, \n" 
	     << "please download monitor package from www.ocpip.org.\n"
	     << "Explicitly instantiate trace monitor and "
	     << "bind it to the channel and the clock\n";
    }
    this->m_sthreadbusy_pipelined=new OCP_TL1_Thread_pipelined_support("sthreadbusys_pipelined", p_clk);
    this->m_sdatathreadbusy_pipelined=new OCP_TL1_Thread_pipelined_support("sdatathreadbusys_pipelined", p_clk);
    this->m_mthreadbusy_pipelined=new OCP_TL1_Thread_pipelined_support("mthreadbusys_pipelined", p_clk);
  }

  /* Set TL1 channel parameters. */
  void setConfiguration( std::string configFileName ) {
    MapStringType              ocpParamMap;
    readMapFromFile(configFileName, ocpParamMap);
    OCP_TL1_Channel<TdataCl>::setConfiguration(ocpParamMap);
  }

  void setConfiguration( MapStringType& passedMap ) {
    OCP_TL1_Channel<TdataCl>::setConfiguration(passedMap);
  }
};
#endif
