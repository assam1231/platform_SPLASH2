#include <iostream>
#include <iomanip>
#include <stdexcept> 
#include <cstring>
#include <string>

#include "tinyxml2.h"

#include "TrafficGen.h"

#define LOG_TO_FILE
	
template<unsigned int DATA_WIDTH>
TrafficGen<DATA_WIDTH>::TrafficGen( sc_core::sc_module_name name, int id, TrafficGenMode mode ):
	sc_core::sc_module( name ),
	init_socket( "init_socket", ocpip::ocp_master_socket_tl3<>::mm_txn_with_be_and_data() ),
	m_id( id ),
	m_transaction_count( 0 ),
	MAX_DATA_LENGTH( 64*4 ),    //in byte. 64 words, 1 word=4 bytes
    complete(0)
{
	init_socket.set_ocp_config( m_ocp_parameters );
	init_socket.register_nb_transport_bw (this, &TrafficGen::nb_transport_bw);
	
	switch ( mode ) 
  {
		case BLOCKING_MODE:
			SC_THREAD( generate_traffic_b );
			break;

		case NON_BLOCKING_MODE: 
			SC_THREAD( generate_traffic_nb );
			break; 
	}
  
}

template<unsigned int DATA_WIDTH>
TrafficGen<DATA_WIDTH>::~TrafficGen()
{
	if ( m_log_file.is_open() )
		m_log_file.close();
}

/*
 *
 *  Socket callback function
 *
 */

template<unsigned int DATA_WIDTH>
tlm::tlm_sync_enum TrafficGen<DATA_WIDTH>::nb_transport_bw 
(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay)
{

  switch (phase) {

    case tlm::BEGIN_RESP: 
      // Only read command has response 
      assert(trans.is_read());
      
      back_addr = trans.get_address();
      //cout << hex << back_addr << endl;
      //m_resp_event.notify();
      
      back_time = sc_time_stamp();
      
      send_time = mapping[&trans];
      mapping.erase(&trans);
      
      request_time = back_time - send_time;
      
#ifdef LOG_TO_FILE
      /*m_log_file << "0x" << std::setw( 8 ) << std::setfill( '0' )<< std::hex << trans.get_address() << " ";
      m_log_file << "0x";
      for ( unsigned int i = 0; i < trans.get_data_length(); i++ ) {
        m_log_file << std::setw( 2 ) << std::setfill( '0' ) << std::hex << (unsigned int)*( trans.get_data_ptr()+i ); 
        if ( i % 4 == 3 ) m_log_file << " ";
      }
      m_log_file << endl;*/
      m_log_file << send_time << " " << back_time << " " << request_time << endl;

#endif 
      
      break;

    case tlm::END_REQ:
      assert(trans.is_write());
      m_resp_event.notify();
      break;

    //Invalid phases
    default:
      cout << m_id << " Illegal phase " << phase << " on backward path" << endl;
      trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
      break;
  }

	return tlm::TLM_COMPLETED;
}


/*
 *
 * Public methods
 *
 */

template<unsigned int DATA_WIDTH>
int TrafficGen<DATA_WIDTH>::id() 
{
  return m_id;
}

template<unsigned int DATA_WIDTH>
void TrafficGen<DATA_WIDTH>::load_traffic( tinyxml2::XMLElement * xml_ele )
{
	m_xml_element = xml_ele;
	assert( strcmp( xml_ele->Value(), "master" ) == 0 );
	
  std::string data_mode = xml_ele->Attribute( "data_mode");
  if ( data_mode == "RANDOM" ) 
  {
		m_data_mode = M_RANDOM;
  }

  else if ( data_mode == "PREDEFINED" ) {
    m_data_mode = M_PREDEFINED;
  }

  else if ( data_mode == "USER_DEFINED" ) {
    m_data_mode = M_USER_DEFINED;
  }

  else {
    cout<< "xml_ele->Attribute( \"data_mode\" ) is invalid -> RANDOM" << endl;
    m_data_mode = M_RANDOM;
  }
	
	std::string log_name;
	if ( xml_ele->Attribute( "log" ) != NULL )
	{
		log_name = std::string( xml_ele->Attribute( "log") );
	}
	else //No attribute data_mode
	{
		std::ostringstream oss;
		oss<< "m" << m_id << ".log";
		log_name = oss.str();
		cout<< "xml_ele->Attribute( \"log\" ) == NULL -> " << log_name << endl;
	}
#ifdef LOG_TO_FILE
	m_log_file.open( log_name.c_str(), std::ofstream::out );
	if ( ! m_log_file.is_open() ) {
		cout<< name() << ": TrafficGen::load_traffic() fails to open "<< log_name << endl;
		exit( 1 );
	}
	//m_log_file << "#addr      #data" << endl;
#endif
}

template<unsigned int DATA_WIDTH>
void TrafficGen<DATA_WIDTH>::generate_traffic_b()
{
	assert( m_xml_element != NULL );
	m_transaction_count = 0;

	tlm::tlm_generic_payload* trans = init_socket.get_transaction();
	unsigned char *mask_buffer = new unsigned char [MAX_DATA_LENGTH];
	unsigned char *data_buffer = new unsigned char [MAX_DATA_LENGTH];
	sc_core::sc_time          delay;

	for ( const tinyxml2::XMLElement *action = m_xml_element->FirstChildElement(); 
		    action != 0; 
        action = action->NextSiblingElement() )
	{
		if ( strcmp( action->Value(), "master_action" ) != 0 )
			continue;

    std::string type_name = action->Attribute( "type" );
		if ( type_name == "TRANSACTION" ) 
		{
			// Initialize generic payload
			const char * command = action->Attribute("command");
			assert( command != NULL );
			if ( strcmp( command, "WR" ) == 0 ) {
				trans->set_write();
			}
			else if( strcmp( command, "RD" ) == 0 ) {
				trans->set_read();
			}
			else {
				tinyxml2::XMLPrinter printer;
				action->Accept( &printer );
				cerr<< "Error: TrafficGen::generate_traffic: invalid command " << printer.CStr();
			}

			int data_length; 
			assert( action->QueryIntAttribute( "data_length", &data_length ) == tinyxml2::XML_SUCCESS );
			trans->set_data_length( data_length );
			assert( data_length <= (int)MAX_DATA_LENGTH );

			memset( mask_buffer, 0xff, data_length );
			trans->set_byte_enable_ptr( mask_buffer );

			trans->set_streaming_width(DATA_WIDTH / 8);

			const char * address_cstr = action->Attribute( "address" );
			assert( address_cstr != NULL );
			trans->set_address( strtoul( address_cstr, NULL, 16 ) );
			trans->set_data_ptr( data_buffer );
			trans->set_dmi_allowed(false);
			trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
			delay = sc_core::SC_ZERO_TIME;
			
	    if ( trans->is_write() )
		  {	
				generate_and_set_data( trans, m_data_mode, action );
			}
			// Send packet to NoC
			//trans->acquire();
			//retval = init_socket->nb_transport_fw(*trans, phase, delay);
			init_socket->b_transport( *trans, delay );

#ifdef LOG_TO_FILE
			/*if ( trans->is_read() ) {
				m_log_file << "0x" << std::setw( 8 ) << std::setfill( '0' )<< std::hex << trans->get_address() << " ";
				m_log_file << "0x";
				for ( unsigned int i = 0; i< trans->get_data_length(); i++ ) {
					//if ( i % 4 == 0 ) m_log_file << "0x";
					m_log_file << std::setw( 2 ) << std::setfill( '0' ) << std::hex << (unsigned int)*( trans->get_data_ptr()+i ); 
					//if ( i % 4 == 3 ) m_log_file << endl;
					if ( i % 4 == 3 ) m_log_file << " ";
				}
				m_log_file << endl;
			}*/
#endif

			m_transaction_count++;
		}
		else if ( type_name == "WAIT" )
		{
			test_gen_wait( action );
		}
		else if ( type_name == "WAIT_UNTIL" )
		{
			test_gen_wait_until( action );
		}
		else
		{
			cout << "Undefined master_action type=\"" << type_name <<"\""<< endl;
		}
	}
}

template<unsigned int DATA_WIDTH>
void TrafficGen<DATA_WIDTH>::generate_traffic_nb()
{
	assert( m_xml_element != NULL );
	m_transaction_count = 0;

	tlm::tlm_generic_payload* trans = NULL;

	for (const tinyxml2::XMLElement *action = m_xml_element->FirstChildElement(); 
		    action != NULL; 
        action = action->NextSiblingElement() 
      )
	{
    
		if ( strcmp( action->Value(), "master_action" ) != 0 )
			continue;

		assert(action->Attribute( "type" ) != NULL);

    std::string action_type = action->Attribute( "type" );
    if ( action_type == "TRANSACTION" )
	{
			// Initialize generic payload
      // (Use ocp memory management)
      trans = init_socket.get_transaction();
      setup_payload(trans, action);
      
	  // Send packet to NoC
      tlm::tlm_phase phase = tlm::BEGIN_REQ;
      sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
      tlm::tlm_sync_enum retval;
	  retval = init_socket->nb_transport_fw(*trans, phase, delay);
      
      time = sc_time_stamp();
      mapping.insert(std::pair<tlm::tlm_generic_payload*,sc_core::sc_time>(trans,time));
   
      check_nb_fw_error_response(*trans);
						
	  switch (retval) 
      {
		case tlm::TLM_ACCEPTED:
          assert(trans->is_read());
          //wait(m_resp_event);
          
		break;

        case tlm::TLM_COMPLETED:
          assert(trans->is_write());
          wait(delay);
          break;

		default:
          std::cerr << "TrafficGen::generate_traffic_nb: Unexpexted response. ";
		break;
			}

			m_transaction_count++;
		}
		else if (action_type == "WAIT" )
		{
			test_gen_wait( action );
		}
		else if ( action_type == "WAIT_UNTIL" )
		{
			test_gen_wait_until( action );
		}
		else
		{
			cout << "Undefined master_action type=\"" << action_type <<"\""<< endl;
		}
	}
}




/*
 *
 *   Handy methods
 *
 */


template<unsigned int DATA_WIDTH>
void TrafficGen<DATA_WIDTH>::test_gen_wait( const tinyxml2::XMLElement * action ){
  double time_value;
  assert( action->QueryDoubleAttribute( "time", &time_value) == tinyxml2::XML_SUCCESS );

  sc_time_unit time_unit = query_time_unit( action );
  wait( time_value, time_unit );
  
}

template<unsigned int DATA_WIDTH>
void TrafficGen<DATA_WIDTH>::test_gen_wait_until( const tinyxml2::XMLElement * action ){
  double time_value;
  assert( action->QueryDoubleAttribute( "time", &time_value) == tinyxml2::XML_SUCCESS );

  sc_time_unit time_unit = query_time_unit( action );
  if ( sc_time_stamp() < sc_time( time_value, time_unit ) ) {
    sc_core::wait( sc_time( time_value, time_unit)-sc_time_stamp() );
  }
}

template<unsigned int DATA_WIDTH>
sc_core::sc_time_unit TrafficGen<DATA_WIDTH>::query_time_unit( const tinyxml2::XMLElement * action ) {
  const char * time_unit_str = action->Attribute( "time_unit" ); 
  if ( time_unit_str == NULL ) {
    cout<< "action(WAIT)->Attribute( \"time_unit\" ) == NULL -> SC_PS" << endl;
    return sc_core::SC_PS;
  }
  else {
    if ( strcmp( time_unit_str, "SC_FS" ) == 0 )
      return sc_core::SC_FS;
    else if ( strcmp( time_unit_str, "SC_PS" ) == 0 )
      return sc_core::SC_PS;
    else if ( strcmp( time_unit_str, "SC_NS" ) == 0 )
      return sc_core::SC_NS;
    else if ( strcmp( time_unit_str, "SC_US" ) == 0 )
      return sc_core::SC_US;
    else if ( strcmp( time_unit_str, "SC_MS" ) == 0 )
      return sc_core::SC_MS;
    else if ( strcmp( time_unit_str, "SC_SEC" ) == 0 )
      return sc_core::SC_SEC;
    else {
      cout<< "action(WAIT)->Attribute( \"time_unit\" ) is invalid -> SC_PS" << endl;
      return sc_core::SC_PS;
    }
  }
}

template<unsigned int DATA_WIDTH>
void TrafficGen<DATA_WIDTH>::setup_payload(
    tlm::tlm_generic_payload *trans, 
    const tinyxml2::XMLElement *action)
{
    assert( action->Attribute( "command" ) != NULL);
    assert( action->Attribute( "address" ) != NULL );
    assert( action->Attribute( "data_length" ) != NULL );

    //Get transaction info from xml element 
    const std::string cmd = action->Attribute("command");
    const char * address_cstr = action->Attribute( "address" );
    int data_length; 
    assert( action->QueryIntAttribute( "data_length", &data_length ) == tinyxml2::XML_SUCCESS );
	  assert( data_length <= (int)MAX_DATA_LENGTH );

    //use ocp-provided memory management for data buffer and byte enable
    init_socket.reserve_data_size(*trans, data_length); 
    init_socket.reserve_be_size(*trans, data_length);


    //Set up payload 
    unsigned int *mask_buffer = reinterpret_cast<unsigned int *>(trans->get_byte_enable_ptr());
    memset( mask_buffer, 0xff, data_length );

    trans->set_address( strtoul( address_cstr, NULL, 16 ) );
    trans->set_data_length( data_length );
    trans->set_streaming_width(DATA_WIDTH / 8);
    trans->set_dmi_allowed(false);	    
    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    if ( cmd == "WR" ) {
      trans->set_write();
      generate_and_set_data( trans, m_data_mode, action ); 
    }
    else if( cmd == "RD" ) {
      trans->set_read();
    }
    else {
      tinyxml2::XMLPrinter printer;
      action->Accept( &printer );
      cerr<< "Error: TrafficGen::generate_traffic_nb: invalid command " << printer.CStr();
    }
}

//
//Generate data base on MasterDataMode and info setted in trans:
//There has three modes:
// 1. M_USER_DEGINED: 
//    this will use the data value that defined in xml file.
//
// 2. M_PREDEFINED: 
//    the data is constructed base on id, transaction count,
//    address, and data count.
//
// 3. M_RANDOM:
//    the data is generated generated.
//
//
// NOTE:This function have to be called after setup payload (trans)
//   address and data_length
//   (Should re-write this part
//
template<unsigned int DATA_WIDTH>
void 
TrafficGen<DATA_WIDTH>::
generate_and_set_data( tlm::tlm_generic_payload* trans, MasterDataMode data_mode, const tinyxml2::XMLElement * action )
{
  int data_length; 
  assert( action->QueryIntAttribute( "data_length", &data_length ) == tinyxml2::XML_SUCCESS );

	switch( data_mode ) { 
		case M_USER_DEFINED: 
			for ( const tinyxml2::XMLElement * data = action->FirstChildElement(); 
				    data != 0; 
            data = data->NextSiblingElement() )
			{
				unsigned int data_value;
				int data_index = 0;
				assert( strcmp( data->Value(), "data" ) == 0 );
				assert( data->Attribute( "value" ) != NULL );
				assert( data->QueryIntAttribute( "index", &data_index ) == tinyxml2::XML_SUCCESS );
				assert( data_index >= 0 );
				assert( data_index < data_length );
				
				const char * data_value_cstr = data->Attribute( "value" );
				assert( data_value_cstr != NULL );
				data_value = strtoul( data_value_cstr, NULL, 16 );
				trans->get_data_ptr()[ data_index ] = static_cast<unsigned char>(data_value);
			}
			break;
		case M_PREDEFINED: {
			word stamp;
			unsigned int data_count = 0;
			for ( unsigned int i = 0; i< data_length; i+=4 ) {
				stamp.uint = (m_id<<28) + ( (m_transaction_count << 20)&0x0ff00000u) + (trans->get_address()) + data_count;
				for ( unsigned int j = 0; j< 4; j++ ) {
					if ( i+j < data_length ) {
						trans->get_data_ptr()[ i+j ] = stamp.uc[3-j];
					}
				}
				data_count+=4;
			}
			break;
		}
		case M_RANDOM:
			for ( unsigned int i = 0; i< trans->get_data_length(); i++ ) {
				trans->get_data_ptr()[ i ] = static_cast<unsigned char>( ( 255.0 )*( rand()/(RAND_MAX + 1.0) ) );
			}
		default:
			break;
	}
}

template<unsigned int DATA_WIDTH>
void TrafficGen<DATA_WIDTH>::check_nb_fw_error_response(tlm::tlm_generic_payload &trans)
{
  if(trans.is_response_error())
  {
    std::ostringstream oss;
    oss << "TrafficGen::generate_traffic_nb: got error response: "
      << trans.get_response_string().c_str();
    throw std::runtime_error(oss.str().c_str());
  }
}


template class TrafficGen<32>;
