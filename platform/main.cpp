#include <iostream>
#include <string>
#include <sstream>
using namespace std;

#include <systemc>
using namespace sc_core;

#include "tinyxml2.h"
#include <sys/times.h>
#include "TrafficGen.h"
#include "Logger.h"

#include "Mesh2D.h"
#include "simulator.h"

#include "OCPWrappedMemory.h"
#include "CrossbarTop.h"

#define NTS_4_DRAMSIM2_crossbar
//#define NTS_4_DRAMSIM2
//#define NTS_DRAMSIM2


TrafficGen<32>* createTrafficGen(const int id) {
  ostringstream tgName;
  tgName << "trafficGen_" << id;
  return new TrafficGen<32>(tgName.str().c_str(), id, NON_BLOCKING_MODE);
}

Logger<32>* createLogger(const int id) {
  ostringstream loggerName;
  loggerName << "logger_" << id;
  return new Logger<32>(loggerName.str().c_str(), id);
}

int sc_main( int argc, char **argv ) {
  
	if (argc != 2) {
    cout << "usage: <exe> <trace_file>\n";
    exit(1);
  }
  
  string xml_file_name(argv[1]);

  TrafficGen<32> *trafficGens[16];
  Logger<32> *loggers[16];
  ispa::Mesh2D<32, 4, 4>* mesh;
  
  mesh = new ispa::Mesh2D<32, 4, 4> ("mesh_2d", sc_time(4,SC_NS));

#ifdef NTS_4_DRAMSIM2_crossbar      
  OCPWrappedMemory * mem[4];
  CrossbarTop * crossbar_top;
  for ( unsigned int i = 0 ; i < 4 ; i++ ){
      stringstream mm ( stringstream::in | stringstream::out );
      mm << "memory_";
      mm << i ;
      mem[i] = new OCPWrappedMemory(mm.str().c_str());
  }
  
  crossbar_top = new CrossbarTop("Crossbar_top", 4, sc_time(4,SC_NS));
#endif  
  
#ifdef NTS_4_DRAMSIM2
  for ( unsigned int i = 0 ; i < 4 ; i++ ){
      stringstream mm ( stringstream::in | stringstream::out );
      mm << "memory_";
      mm << i ;
      mem[i] = new OCPWrappedMemory(mm.str().c_str());
  }   
  SHOW_SIM_OUTPUT = true;
#endif


#ifdef NTS_DRAMSIM2
  stringstream mm ( stringstream::in | stringstream::out );
  mm << "ExternalMemory";
  mem = new OCPWrappedMemory(mm.str().c_str());
#endif
  

#ifdef NTS_4_DRAMSIM2_crossbar  
  for(int i = 0 ; i < 4 ; i++){
      mesh->ext_mem_ni[i]->initSocket.bind( (*crossbar_top->target_crossbar_socket[i]) );
      (*crossbar_top->init_crossbar_socket[i]).bind( mem[i]->ocp_slave_socket );
  }
#endif

#ifdef NTS_4_DRAMSIM2
  for(int i = 0 ; i < 4 ; i++){
      mesh->ext_mem_ni[i]->initSocket.bind( mem[i]->ocp_slave_socket );
  }
#endif

#ifdef NTS_DRAMSIM2
  mesh->ext_mem_ni->initSocket.bind( mem->ocp_slave_socket );
#endif


  for (int i=0; i < 16; i++) {
    trafficGens[i] = createTrafficGen(i);
    loggers[i] = createLogger(i);

    //Bind sockets
    trafficGens[i]->init_socket.bind( mesh->initNIs[i]->targetSocket);
    mesh->targetNIs[i]->initSocket.bind( loggers[i]->target_socket);

  }

  //Load traffics from config file
  tinyxml2::XMLDocument doc;
	if ( tinyxml2::XML_NO_ERROR != doc.LoadFile( xml_file_name.c_str() ) )
	{
		doc.PrintError();
		exit(1);
	}

	for ( tinyxml2::XMLElement * element = doc.FirstChildElement(); 
		    element != 0; 
        element = element->NextSiblingElement() )
	{
		if ( strcmp( element->Value(), "master" ) == 0 ) 
        {
			int id;
			if ( element->QueryIntAttribute( "id", &id ) == tinyxml2::XML_NO_ERROR ) 
            {
              trafficGens[id]->load_traffic( element );
			}
		}
		else if ( strcmp( element->Value(), "slave" ) == 0 )
		{
			int id;
			if ( element->QueryIntAttribute( "id", &id ) == tinyxml2::XML_NO_ERROR ) 
            {
			  loggers[id]->load_traffic( element );
			}
		}
    }

	sc_core::sc_start();
    
	return 0;
}
