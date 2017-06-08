#include "utils.h"
#include <iostream>
#include <iomanip>
#include "tlm"

#define NAME_WIDTH (12)

void display_transaction( const char * name, const tlm::tlm_generic_payload & trans ) {
	//std::cout <<name<< std::endl;
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
	std::cout<<std::setw(NAME_WIDTH)<<name << ": @"<<std::setfill(' ')<<std::setw(8)<<(sc_core::sc_time_stamp()).to_default_time_units()<<" ns:\n";
	std::cout<<std::setw(NAME_WIDTH)<<name << ": Command:      "   << commStr << std::endl;
	std::cout<<std::setw(NAME_WIDTH)<<name << ": Address:      0x" << std::setw( 8 ) << std::setfill( '0' )
			<<std::hex << trans.get_address() << std::setfill( ' ' ) << std::endl;
	std::cout<<std::setw(NAME_WIDTH)<<name << ": Data length:  " <<std::dec << trans.get_data_length() << std::endl;
	std::cout<<std::setw(NAME_WIDTH)<<name << ": Byte enables: 0x";
	for ( unsigned int i = 0; i< trans.get_byte_enable_length(); i++ ) {
		std::cout << std::setw( 2 ) << std::setfill( '0' )
			<< std::hex << (unsigned int)*( trans.get_byte_enable_ptr()+i ) ; 
		if ( i % 4 == 3 ) { std::cout << " "; }
	}
	std::cout << std::setfill( ' ' )<< std::endl;

	if( trans.get_command() == tlm::TLM_WRITE_COMMAND ) {
		std::cout << std::setfill( ' ' )<<std::setw(NAME_WIDTH)<<name << ": Data:         0x";
		for ( unsigned int i = 0; i< trans.get_data_length(); i++ ) {
			std::cout << std::setw( 2 ) << std::setfill( '0' )
				<< std::hex << (unsigned int)*( trans.get_data_ptr()+i ) ;
			if ( i % 4 == 3 ) { std::cout << " "; }
		}
	}
	std::cout << std::setfill( ' ' )<< std::endl;
}
