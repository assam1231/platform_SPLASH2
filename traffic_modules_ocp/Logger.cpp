#include <iostream> 
#include <sstream>
#include <string>
#include "tinyxml2.h"
#include "Logger.h"

//#define LOG_TO_FILE

template<unsigned int DATA_WIDTH>
Logger<DATA_WIDTH>::Logger( sc_module_name n, int id ): sc_module( n )
  , target_socket( "target_socket" )
  , m_id( id )
  , m_read_latency(sc_time(2, SC_NS))
  , m_write_latency(sc_time(1, SC_NS))
  , m_slave_element( NULL )
  , m_print( false )
  , m_transaction_count( 0 )
  , m_resp_peq("m_resp_peq")
	, MEM_SIZE( 2147483648 ) //in byte. 64 words, 1 word=4 bytes
{
	target_socket.set_ocp_config( m_ocp_parameters );
	target_socket.register_b_transport( this, &Logger::b_transport);
	target_socket.register_nb_transport_fw( this, &Logger::nb_transport_fw);

    SC_METHOD( begin_resp_process );
    sensitive << m_resp_peq.get_event();
    dont_initialize();

  m_memory = new unsigned char[MEM_SIZE];
}
	
template<unsigned int DATA_WIDTH>
Logger<DATA_WIDTH>::~Logger()
{
  
#ifdef LOG_TO_FILE
	if ( log_file.is_open() )
		log_file.close();
#endif
}


template<unsigned int DATA_WIDTH>
void Logger<DATA_WIDTH>::load_traffic( tinyxml2::XMLElement * xml_ele_ptr ) 
{
	m_slave_element = xml_ele_ptr;
	assert( strcmp( xml_ele_ptr->Value(), "slave" ) == 0 );

	if ( xml_ele_ptr->QueryBoolAttribute( "print", &m_print ) != tinyxml2::XML_SUCCESS ) 
	{
		m_print = true; //print out received transaction by default. 
	}
	
	std::string log_name;
	if ( xml_ele_ptr->Attribute( "log" ) != NULL )
	{
		log_name = std::string( xml_ele_ptr->Attribute( "log") );
	}
	else //No attribute log
	{
		std::ostringstream oss;
		oss<< "s" << m_id << ".log";
		log_name = oss.str();
        std::cout<< "xml_ele_ptr->Attribute( \"log\" ) == NULL -> " << log_name << std::endl;
	}
#ifdef LOG_TO_FILE
	log_file.open( log_name.c_str(), std::ofstream::out );
	if ( ! log_file.is_open() ) {
    std::cout<< name() << ": Logger::load_traffic() fails to open "<< log_name << std::endl;
		exit( 1 );
	}
	log_file << "#addr      #data" << std::endl;
#endif
}


template<unsigned int DATA_WIDTH>
void Logger<DATA_WIDTH>::b_transport( tlm::tlm_generic_payload &trans, sc_core::sc_time &delay )
{
	wait(delay); //wait at target
  if (trans.is_read()) 
  {
    do_read(trans);
  }

  else if (trans.is_write())
  {
    do_write(trans);
  }
}


template<unsigned int DATA_WIDTH>
tlm::tlm_sync_enum Logger<DATA_WIDTH>::nb_transport_fw( tlm::tlm_generic_payload &trans, tlm::tlm_phase& phase, sc_time & delay)
{

  print_transaction(trans, delay);

  if (phase == tlm::BEGIN_REQ)
  {
    if (trans.is_read()) 
    {
      do_read(trans);
      m_transaction_count ++;
      m_resp_peq.notify(trans, delay + m_read_latency);

      return tlm::TLM_ACCEPTED;
    }

    else if (trans.is_write())
    {
      do_write(trans);
      delay += m_write_latency;
      m_transaction_count ++;

      return tlm::TLM_COMPLETED;
    }

    else
    {
      std::cerr << "Unknown command\n";
      trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
      return tlm::TLM_COMPLETED;
    }

  }
  else {
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    return tlm::TLM_COMPLETED;
  }
}


template<unsigned int DATA_WIDTH>
void Logger<DATA_WIDTH>::begin_resp_process() 
{
  tlm::tlm_generic_payload *trans = NULL;
  trans = m_resp_peq.get_next_transaction();
  assert(trans->is_read());

  tlm::tlm_phase phase = tlm::BEGIN_RESP;
  sc_time        delay = SC_ZERO_TIME;
  tlm::tlm_sync_enum status;

  status  = target_socket->nb_transport_bw( *trans, phase, delay );

  if (status != tlm::TLM_COMPLETED)
  {
    std::cerr << "LOGGER::begin_resp_process: Unexpexted response. \n";
  }
}

template<unsigned int DATA_WIDTH>
void Logger<DATA_WIDTH>::do_read(tlm::tlm_generic_payload &trans)  
{
  unsigned int addr = trans.get_address();
  unsigned int data_len = trans.get_data_length();
  unsigned char *data_prt = trans.get_data_ptr();
  assert(addr + data_len < MEM_SIZE -1);
  memcpy( data_prt, (m_memory+addr), data_len );

  trans.set_response_status(tlm::TLM_OK_RESPONSE);

}

template<unsigned int DATA_WIDTH>
void Logger<DATA_WIDTH>::do_write(tlm::tlm_generic_payload &trans)  
{
#ifdef LOG_TO_FILE
  log_file << "0x" << std::setw( 8 ) << std::setfill( '0' )<< std::hex << trans.get_address() << " ";
  log_file << "0x";
  for ( unsigned int i = 0; i< trans.get_data_length(); i++ ) {
    log_file << std::setw( 2 ) << std::setfill( '0' ) << std::hex << (unsigned int)*( trans.get_data_ptr()+i ); 
    if ( i % 4 == 3 ) log_file << " ";
  }
  log_file << std::endl;
#endif

  //copy data to memory
  unsigned int addr = trans.get_address();
  unsigned int data_len = trans.get_data_length();
  unsigned char *data_prt = trans.get_data_ptr();
  assert(addr + data_len < MEM_SIZE -1);
  memcpy( (m_memory+addr), data_prt, data_len );


  trans.set_response_status(tlm::TLM_OK_RESPONSE);

}

template <unsigned int DATA_WIDTH>
void Logger<DATA_WIDTH>::print_transaction
( const tlm::tlm_generic_payload & trans, const sc_core::sc_time delay ) {
  const char *commStr;
  switch( trans.get_command() )
  {
  case tlm::TLM_READ_COMMAND:
    commStr = "Read";
    break;
  case tlm::TLM_WRITE_COMMAND:
    commStr = "Write";
    break;
  case tlm::TLM_IGNORE_COMMAND:
    commStr = "Ignore";
    break;
  default:
    commStr = "Unknown command";
    break;
  }
  if (m_print) {
    std::cout<<std::setw(8)<<name() << ": @"<<std::setfill(' ')<<std::setw(8)<<(sc_core::sc_time_stamp()+delay).to_default_time_units()<<" ns:\n";
    std::cout<<std::setw(8)<<name() << ": Command:      "   << commStr << std::endl;
    std::cout<<std::setw(8)<<name() << ": Address:      0x" << std::setw( 8 ) << std::setfill( '0' )
        <<std::hex << trans.get_address() << std::endl;
    std::cout << std::setfill( ' ' )<<std::setw(8)<<name() << ": Data length:  " 
        <<std::dec << trans.get_data_length() << std::endl;
    std::cout<<std::setw(8)<<name() << ": Byte enables: 0x";
    for ( unsigned int i = 0; i< trans.get_data_length(); i++ ) {
      std::cout << std::setw( 2 ) << std::setfill( '0' )
        << std::hex << (unsigned int)*( trans.get_byte_enable_ptr()+i ) ; 
    }
    if( trans.get_command() == tlm::TLM_WRITE_COMMAND ) {
      std::cout << std::endl;
      std::cout << std::setfill( ' ' )<<std::setw(8)<<name() << ": Data:         0x";
      for ( unsigned int i = 0; i< trans.get_data_length(); i++ ) {
        std::cout << std::setw( 2 ) << std::setfill( '0' )
          << std::hex << (unsigned int)*( trans.get_data_ptr()+i ) ;
      }
    }
    std::cout << std::endl << std::endl;
    std::cout << std::setfill(' ');
  }
}

template class Logger<32>;
