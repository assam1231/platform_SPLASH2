#include <systemc>
#include "Crossbar.h"
#include "NocPayloadExtension.h"

#include "pe_mmap.h"
#include "tlm.h"

//#define EN_CROSSBAR_DEBUG
#include "CrossbarDebug.h"

using namespace std;

Crossbar::Crossbar ( sc_core::sc_module_name name, const unsigned int channel_num, sc_core::sc_time cycle_time )
    : name(name)
    , channel_num(channel_num)
    , cycleTime_(cycle_time) // 10ns = 100 MHz
{

    for(unsigned int i = 0; i < channel_num; i++)
    {
        in_if_tomem[i]  = new sc_port<sc_fifo_in_if<tlm::tlm_generic_payload*> >;
        out_if_tomem[i] = new sc_port<sc_fifo_out_if<tlm::tlm_generic_payload*> >;
        in_if_tonoc[i]  = new sc_port<sc_fifo_in_if<tlm::tlm_generic_payload*> >;
        out_if_tonoc[i] = new sc_port<sc_fifo_out_if<tlm::tlm_generic_payload*> >;
    }
    
    for(unsigned int i = 0; i < channel_num; i++)
        fore_fifo[i] = new sc_fifo<tlm::tlm_generic_payload*>(1);
        
    for(unsigned int i = 0; i < channel_num; i++)
        back_fifo[i] = new sc_fifo<tlm::tlm_generic_payload*>(1);
    
    
    SC_THREAD( fore0_router );
    SC_THREAD( fore1_router );
    SC_THREAD( fore2_router );
    SC_THREAD( fore3_router );

    SC_THREAD( fore0_selecter );
    SC_THREAD( fore1_selecter );
    SC_THREAD( fore2_selecter );
    SC_THREAD( fore3_selecter );

    SC_THREAD( back0_router );
    SC_THREAD( back1_router );
    SC_THREAD( back2_router );
    SC_THREAD( back3_router );
    
    SC_THREAD( back0_selecter );
    SC_THREAD( back1_selecter );
    SC_THREAD( back2_selecter );
    SC_THREAD( back3_selecter );
    
}


void Crossbar::fore0_router()
{
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_generic_payload* noc_trans = 0;
    
    while(true)
    {
        if((*in_if_tomem[0])->num_available() > 0)
        {
            noc_trans = (*in_if_tomem[0])->read();
            trans = NocPayloadExt::get_ip_trans( noc_trans );
            
            CROSSBAR_DEBUG(
                cout << "  got noc_trans in fore0_router" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: "  << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
            
            if(trans->get_address() >= pe_global_mmap::EXTERNAL_MEMORY1_BEGIN && trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY2_BEGIN)
            {
                while(true)
                {
                    if((*fore_fifo[0]).num_free() > 0)
                    {
                        (*fore_fifo[0]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans to fore_fifo[0] " << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else 
					{
                        //wait( cycleTime_ );
						wait((*fore_fifo[0]).data_read_event());
                    }
                }
            } 
            else if(trans->get_address() >= pe_global_mmap::EXTERNAL_MEMORY2_BEGIN && trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY3_BEGIN)
            {
                while(true)
                {
                    if((*fore_fifo[1]).num_free() > 0)
                    {
                        (*fore_fifo[1]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in fore_fifo[1]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[1]).data_read_event());
                    }
                }
            } 
            else if(trans->get_address() >= pe_global_mmap::EXTERNAL_MEMORY3_BEGIN && trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY4_BEGIN)
            {
                while(true)
                {
                    if((*fore_fifo[2]).num_free() > 0)
                    {
                        (*fore_fifo[2]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in fore_fifo[2]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[2]).data_read_event());
                    }
                }
            } 
            else {
                while(true)
                {
                    if((*fore_fifo[3]).num_free() > 0)
                    {
                        (*fore_fifo[3]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in fore_fifo[3]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[3]).data_read_event());
                    }
                }
            }
        } 
        else {
            wait((*in_if_tomem[0])->data_written_event());
            /*cout<<"wait_sc_time_stamp = " << sc_time_stamp() << endl;
            wait( cycleTime_ );*/
        }
    }
}

void Crossbar::fore1_router()
{
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_generic_payload* noc_trans = 0;
    
    while(true)
    {
        if((*in_if_tomem[1])->num_available() > 0)
        {
            noc_trans = (*in_if_tomem[1])->read();
            trans = NocPayloadExt::get_ip_trans( noc_trans );
            
            CROSSBAR_DEBUG(
                cout << "  got noc_trans in fore1_router" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: "  << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
            
            if(trans->get_address() >= pe_global_mmap::EXTERNAL_MEMORY1_BEGIN && trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY2_BEGIN)
            {
                while(true)
                {
                    if((*fore_fifo[0]).num_free() > 0)
                    {
                        (*fore_fifo[0]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans to fore_fifo[0] " << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[0]).data_read_event());
                    }
                }
            } 
            else if(trans->get_address() >= pe_global_mmap::EXTERNAL_MEMORY2_BEGIN && trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY3_BEGIN)
            {
                while(true)
                {
                    if((*fore_fifo[1]).num_free() > 0)
                    {
                        (*fore_fifo[1]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in fore_fifo[1]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[1]).data_read_event());
                    }
                }
            } 
            else if(trans->get_address() >= pe_global_mmap::EXTERNAL_MEMORY3_BEGIN && trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY4_BEGIN)
            {
                while(true)
                {
                    if((*fore_fifo[2]).num_free() > 0)
                    {
                        (*fore_fifo[2]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in fore_fifo[2]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[2]).data_read_event());
                    }
                }
            } 
            else {
                while(true)
                {
                    if((*fore_fifo[3]).num_free() > 0)
                    {
                        (*fore_fifo[3]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in fore_fifo[3]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[3]).data_read_event());
                    }
                }
            }
        } 
        else {
            wait((*in_if_tomem[1])->data_written_event());
            /*cout<<"wait_sc_time_stamp = " << sc_time_stamp() << endl;
            wait( cycleTime_ );*/
        }
    }
}

void Crossbar::fore2_router()
{
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_generic_payload* noc_trans = 0;
    
    while(true)
    {
        if((*in_if_tomem[2])->num_available() > 0)
        {   
            noc_trans = (*in_if_tomem[2])->read();
            trans = NocPayloadExt::get_ip_trans( noc_trans );
            
            CROSSBAR_DEBUG(
                cout << "  got noc_trans in fore2_router" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: "  << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
            
            if(trans->get_address() >= pe_global_mmap::EXTERNAL_MEMORY1_BEGIN && trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY2_BEGIN)
            {
                while(true)
                {
                    if((*fore_fifo[0]).num_free() > 0)
                    {
                        (*fore_fifo[0]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans to fore_fifo[0] " << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[0]).data_read_event());
                    }
                }
            } 
            else if(trans->get_address() >= pe_global_mmap::EXTERNAL_MEMORY2_BEGIN && trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY3_BEGIN)
            {
                while(true)
                {
                    if((*fore_fifo[1]).num_free() > 0)
                    {
                        (*fore_fifo[1]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in fore_fifo[1]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[1]).data_read_event());
                    }
                }
            } 
            else if(trans->get_address() >= pe_global_mmap::EXTERNAL_MEMORY3_BEGIN && trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY4_BEGIN)
            {
                while(true)
                {
                    if((*fore_fifo[2]).num_free() > 0)
                    {
                        (*fore_fifo[2]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in fore_fifo[2]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[2]).data_read_event());
                    }
                }
            } 
            else {
                while(true)
                {
                    if((*fore_fifo[3]).num_free() > 0)
                    {
                        (*fore_fifo[3]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in fore_fifo[3]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[3]).data_read_event());
                    }
                }
            }
        } 
        else {
            wait((*in_if_tomem[2])->data_written_event());
            /*cout<<"wait_sc_time_stamp = " << sc_time_stamp() << endl;
            wait( cycleTime_ );*/
        }
    }
}

void Crossbar::fore3_router()
{
    tlm::tlm_generic_payload* trans = 0;
    tlm::tlm_generic_payload* noc_trans = 0;
    
    while(true)
    {
        if((*in_if_tomem[3])->num_available() > 0)
        {
            noc_trans = (*in_if_tomem[3])->read();
            trans = NocPayloadExt::get_ip_trans( noc_trans );
            
            CROSSBAR_DEBUG(
                cout << "  got noc_trans in fore3_router" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: "  << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
            
            if(trans->get_address() >= pe_global_mmap::EXTERNAL_MEMORY1_BEGIN && trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY2_BEGIN)
            {
                while(true)
                {
                    if((*fore_fifo[0]).num_free() > 0)
                    {
                        (*fore_fifo[0]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans to fore_fifo[0] " << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[0]).data_read_event());
                    }
                }
            } 
            else if(trans->get_address() >= pe_global_mmap::EXTERNAL_MEMORY2_BEGIN && trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY3_BEGIN)
            {
                while(true)
                {
                    if((*fore_fifo[1]).num_free() > 0)
                    {
                        (*fore_fifo[1]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in fore_fifo[1]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[1]).data_read_event());
                    }
                }
            } 
            else if(trans->get_address() >= pe_global_mmap::EXTERNAL_MEMORY3_BEGIN && trans->get_address() < pe_global_mmap::EXTERNAL_MEMORY4_BEGIN)
            {
                while(true)
                {
                    if((*fore_fifo[2]).num_free() > 0)
                    {
                        (*fore_fifo[2]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in fore_fifo[2]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[2]).data_read_event());
                    }
                }
            } 
            else {
                while(true)
                {
                    if((*fore_fifo[3]).num_free() > 0)
                    {
                        (*fore_fifo[3]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in fore_fifo[3]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    } 
                    else {
                        wait((*fore_fifo[3]).data_read_event());
                    }
                }
            }
        } 
        else {
            wait((*in_if_tomem[3])->data_written_event());
            /*cout<<"wait_sc_time_stamp = " << sc_time_stamp() << endl;
            wait( cycleTime_ );*/
        }
    }
}


void Crossbar::fore0_selecter()
{
    tlm::tlm_generic_payload *noc_trans = 0;
    
    while(true)
    {
        if((*fore_fifo[0]).num_available() > 0)
        {
            noc_trans = (*fore_fifo[0]).read();
            
            CROSSBAR_DEBUG(
                tlm::tlm_generic_payload *trans = 0;
                trans = NocPayloadExt::get_ip_trans( noc_trans );
                cout << "  got noc_trans in fore0_selecter" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
            
            while(true)
            {
                if((*out_if_tomem[0])->num_free() > 0)
                {
                    //wait( cycleTime_ );
                    (*out_if_tomem[0])->write(noc_trans);
                    CROSSBAR_DEBUG( cout << "  write noc_trans in out_if_tomem[0]" << endl << endl; );
                    break;
                } 
                else {
                    wait((*out_if_tomem[0])->data_read_event());
                }
            }
        }
        else {
            wait((*fore_fifo[0]).data_written_event());
            //wait( cycleTime_ );
        }
    }
}

void Crossbar::fore1_selecter()
{
    tlm::tlm_generic_payload *noc_trans = 0;
    
    while(true)
    {
        if((*fore_fifo[1]).num_available() > 0)
        {
            noc_trans = (*fore_fifo[1]).read();
            
            CROSSBAR_DEBUG(
                tlm::tlm_generic_payload *trans = 0;
                trans = NocPayloadExt::get_ip_trans( noc_trans );
                cout << "  got noc_trans in fore1_selecter" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
            
            while(true)
            {
                if((*out_if_tomem[1])->num_free() > 0)
                {
                     //wait( cycleTime_ );
                     (*out_if_tomem[1])->write(noc_trans);
                     CROSSBAR_DEBUG( cout << "  write noc_trans in out_if_tomem[1]" << endl << endl; );
                     break;
                } 
                else {
                   wait((*out_if_tomem[1])->data_read_event());
                }
            }
        }
        else {
            wait((*fore_fifo[1]).data_written_event());
            //wait( cycleTime_ );
        }
    }
}

void Crossbar::fore2_selecter()
{
    tlm::tlm_generic_payload *noc_trans = 0;
    
    while(true)
    {
        if((*fore_fifo[2]).num_available() > 0)
        {
            noc_trans = (*fore_fifo[2]).read();
            
            CROSSBAR_DEBUG(
                tlm::tlm_generic_payload *trans = 0;
                trans = NocPayloadExt::get_ip_trans( noc_trans );
                cout << "  got noc_trans in fore2_selecter" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
            
            while(true)
            {
                if((*out_if_tomem[2])->num_free() > 0)
                {
                     //wait( cycleTime_ );
                     (*out_if_tomem[2])->write(noc_trans);
                     CROSSBAR_DEBUG( cout << "  write noc_trans in out_if_tomem[2]" << endl << endl; );
                     break;
                } 
                else {
                    wait((*out_if_tomem[2])->data_read_event());
                }
            }
        }
        else {
            wait((*fore_fifo[2]).data_written_event());
            //wait( cycleTime_ );
        }
    }
}

void Crossbar::fore3_selecter()
{
    tlm::tlm_generic_payload *noc_trans = 0;
    
    while(true)
    {
        if((*fore_fifo[3]).num_available() > 0)
        {
            noc_trans = (*fore_fifo[3]).read();
            
            CROSSBAR_DEBUG(
                tlm::tlm_generic_payload *trans = 0;
                trans = NocPayloadExt::get_ip_trans( noc_trans );
                cout << "  got noc_trans in fore3_selecter" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
            
            while(true)
            {
                if((*out_if_tomem[3])->num_free() > 0)
                {
                     //wait( cycleTime_ );
                     (*out_if_tomem[3])->write(noc_trans);
                     CROSSBAR_DEBUG( cout << "  write noc_trans in out_if_tomem[3]" << endl << endl; );
                     break;
                } 
                else {
                    wait((*out_if_tomem[3])->data_read_event());
                }
            }
        }
        else {
            wait((*fore_fifo[3]).data_written_event());
            //wait( cycleTime_ );
        }
    }
}



void Crossbar::back0_router()
{
    tlm::tlm_generic_payload *noc_trans = 0;
    
    while(true)
    {   
        if((*in_if_tonoc[0])->num_available() > 0)
        {
            noc_trans = (*in_if_tonoc[0])->read();
            
            CROSSBAR_DEBUG(
                tlm::tlm_generic_payload *trans = 0;
                trans = NocPayloadExt::get_ip_trans( noc_trans );
                cout << "  got noc_trans in back0_router" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
        
            unsigned int source_id = NocPayloadExt::get_source_id( noc_trans );
            
            if(source_id == 0 || source_id == 1 || source_id == 2 || source_id == 5)
            {
                while(true)
                {
                    if((*back_fifo[0]).num_free() > 0)
                    {
                        (*back_fifo[0]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[0]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else {
                        wait((*back_fifo[0]).data_read_event());
                    }
                }
            }
            else if(source_id == 3 || source_id == 6 || source_id == 7 || source_id == 11)
            {
                while(true)
                {
                    if((*back_fifo[1]).num_free() > 0)
                    {
                        (*back_fifo[1]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[1]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else
                    {
                        wait((*back_fifo[1]).data_read_event());
                    }
                }
            }
            else if(source_id == 10 || source_id == 13 || source_id == 14 || source_id == 15)
            {
                while(true)
                {
                    if((*back_fifo[2]).num_free() > 0)
                    {
                        (*back_fifo[2]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[2]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else {
                        wait((*back_fifo[2]).data_read_event());
                    }
                }
            }
            else {
                while(true)
                {
                    if((*back_fifo[3]).num_free() > 0)
                    {
                        (*back_fifo[3]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[3]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else {
                        wait((*back_fifo[3]).data_read_event());
                    }
                }
            }
        }
        else
        {
            wait((*in_if_tonoc[0])->data_written_event());
        }
    }
}

void Crossbar::back1_router()
{
    tlm::tlm_generic_payload *noc_trans = 0;
    
    while(true)
    {
        if((*in_if_tonoc[1])->num_available() > 0)
        {
            noc_trans = (*in_if_tonoc[1])->read();
            
            CROSSBAR_DEBUG(
                tlm::tlm_generic_payload *trans = 0;
                trans = NocPayloadExt::get_ip_trans( noc_trans );
                cout << "  got noc_trans in back1_router" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
        
            unsigned int source_id = NocPayloadExt::get_source_id( noc_trans );
            
            if(source_id == 0 || source_id == 1 || source_id == 2 || source_id == 5)
            {
                while(true)
                {
                    if((*back_fifo[0]).num_free() > 0)
                    {
                        (*back_fifo[0]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[0]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else {
                        wait((*back_fifo[0]).data_read_event());
                    }
                }
            }
            else if(source_id == 3 || source_id == 6 || source_id == 7 || source_id == 11)
            {
                while(true)
                {
                    if((*back_fifo[1]).num_free() > 0)
                    {
                        (*back_fifo[1]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[1]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else
                    {
                        wait((*back_fifo[1]).data_read_event());
                    }
                }
            }
            else if(source_id == 10 || source_id == 13 || source_id == 14 || source_id == 15)
            {
                while(true)
                {
                    if((*back_fifo[2]).num_free() > 0)
                    {
                        (*back_fifo[2]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[2]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else {
                        wait((*back_fifo[2]).data_read_event());
                    }
                }
            }
            else {
                while(true)
                {
                    if((*back_fifo[3]).num_free() > 0)
                    {
                        (*back_fifo[3]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[3]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else {
                        wait((*back_fifo[3]).data_read_event());
                    }
                }
            }
        }
        else
        {
            wait((*in_if_tonoc[1])->data_written_event());
        }
    }
}

void Crossbar::back2_router()
{
    tlm::tlm_generic_payload *noc_trans = 0;
    
    while(true)
    {
        if((*in_if_tonoc[2])->num_available() > 0)
        {
            noc_trans = (*in_if_tonoc[2])->read();
            
            CROSSBAR_DEBUG(
                tlm::tlm_generic_payload *trans = 0;
                trans = NocPayloadExt::get_ip_trans( noc_trans );
                cout << "  got noc_trans in back2_router" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
        
            unsigned int source_id = NocPayloadExt::get_source_id( noc_trans );
            
            if(source_id == 0 || source_id == 1 || source_id == 2 || source_id == 5)
            {
                while(true)
                {
                    if((*back_fifo[0]).num_free() > 0)
                    {
                        (*back_fifo[0]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[0]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else {
                        wait((*back_fifo[0]).data_read_event());
                    }
                }
            }
            else if(source_id == 3 || source_id == 6 || source_id == 7 || source_id == 11)
            {
                while(true)
                {
                    if((*back_fifo[1]).num_free() > 0)
                    {
                        (*back_fifo[1]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[1]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else
                    {
                        wait((*back_fifo[1]).data_read_event());
                    }
                }
            }
            else if(source_id == 10 || source_id == 13 || source_id == 14 || source_id == 15)
            {
                while(true)
                {
                    if((*back_fifo[2]).num_free() > 0)
                    {
                        (*back_fifo[2]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[2]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else {
                        wait((*back_fifo[2]).data_read_event());
                    }
                }
            }
            else {
                while(true)
                {
                    if((*back_fifo[3]).num_free() > 0)
                    {
                        (*back_fifo[3]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[3]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else {
                        wait((*back_fifo[3]).data_read_event());
                    }
                }
            }
        }
        else
        {
            wait((*in_if_tonoc[2])->data_written_event());
        }
    }
}

void Crossbar::back3_router()
{
    tlm::tlm_generic_payload *noc_trans = 0;
    
    while(true)
    {
        if((*in_if_tonoc[3])->num_available() > 0)
        {
            noc_trans = (*in_if_tonoc[3])->read();
            
            CROSSBAR_DEBUG(
                tlm::tlm_generic_payload *trans = 0;
                trans = NocPayloadExt::get_ip_trans( noc_trans );
                cout << "  got noc_trans in back3_router" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
        
            unsigned int source_id = NocPayloadExt::get_source_id( noc_trans );
            
            if(source_id == 0 || source_id == 1 || source_id == 2 || source_id == 5)
            {
                while(true)
                {
                    if((*back_fifo[0]).num_free() > 0)
                    {
                        (*back_fifo[0]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[0]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else {
                        wait((*back_fifo[0]).data_read_event());
                    }
                }
            }
            else if(source_id == 3 || source_id == 6 || source_id == 7 || source_id == 11)
            {
                while(true)
                {
                    if((*back_fifo[1]).num_free() > 0)
                    {
                        (*back_fifo[1]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[1]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else
                    {
                        wait((*back_fifo[1]).data_read_event());
                    }
                }
            }
            else if(source_id == 10 || source_id == 13 || source_id == 14 || source_id == 15)
            {
                while(true)
                {
                    if((*back_fifo[2]).num_free() > 0)
                    {
                        (*back_fifo[2]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[2]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else {
                        wait((*back_fifo[2]).data_read_event());
                    }
                }
            }
            else {
                while(true)
                {
                    if((*back_fifo[3]).num_free() > 0)
                    {
                        (*back_fifo[3]).write(noc_trans);
                        CROSSBAR_DEBUG( cout << "  write noc_trans in back_fifo[3]" << endl << endl; );
                        //wait( cycleTime_ );
                        break;
                    }
                    else {
                        wait((*back_fifo[3]).data_read_event());
                    }
                }
            }
        }
        else
        {
            wait((*in_if_tonoc[3])->data_written_event());
        }
    }
}


void Crossbar::back0_selecter(){

    tlm::tlm_generic_payload *noc_trans = 0;
    
    while(true)
    {
        if((*back_fifo[0]).num_available() > 0)
        {
            noc_trans = (*back_fifo[0]).read();
            
            CROSSBAR_DEBUG(
                tlm::tlm_generic_payload *trans = 0;
                trans = NocPayloadExt::get_ip_trans( noc_trans );
                cout << "  got noc_trans in back0_selecter" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
            
            while(true)
            {
                if((*out_if_tonoc[0])->num_free() > 0)
                {
                    (*out_if_tonoc[0])->write(noc_trans);
                    CROSSBAR_DEBUG( cout << "  write noc_trans in out_if_tonoc[0]" << endl << endl << endl; );
                    //wait( cycleTime_ );
                    break;
                }
                else {
                    wait((*out_if_tonoc[0])->data_read_event());
                }
            }
        }
        else {
            wait((*back_fifo[0]).data_written_event());
            //wait( cycleTime_ );
        }
    }
}

void Crossbar::back1_selecter(){

    tlm::tlm_generic_payload *noc_trans = 0;
    
    while(true)
    {
        if((*back_fifo[1]).num_available())
        {
            noc_trans = (*back_fifo[1]).read();
            
            CROSSBAR_DEBUG(
                tlm::tlm_generic_payload *trans = 0;
                trans = NocPayloadExt::get_ip_trans( noc_trans );
                cout << "  got noc_trans in back1_selecter" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
            
            while(true)
            {
                if((*out_if_tonoc[1])->num_free() > 0)
                {
                    (*out_if_tonoc[1])->write(noc_trans);
                    CROSSBAR_DEBUG( cout << "  write noc_trans in out_if_tonoc[0]" << endl << endl << endl; );
                    //wait( cycleTime_ );
                    break;
                }
                else {
                    wait((*out_if_tonoc[1])->data_read_event());
                }
            }
        }
        else {
            wait((*back_fifo[1]).data_written_event());
            //wait( cycleTime_ );
        }
    }
}

void Crossbar::back2_selecter(){

    tlm::tlm_generic_payload *noc_trans = 0;
    
    while(true)
    {
        if((*back_fifo[2]).num_available())
        {
            noc_trans = (*back_fifo[2]).read();
            
            CROSSBAR_DEBUG(
                tlm::tlm_generic_payload *trans = 0;
                trans = NocPayloadExt::get_ip_trans( noc_trans );
                cout << "  got noc_trans in back2_selecter" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
            
            while(true)
            {
                if((*out_if_tonoc[2])->num_free() > 0)
                {
                    (*out_if_tonoc[2])->write(noc_trans);
                     CROSSBAR_DEBUG( cout << "  write noc_trans in out_if_tonoc[2]" << endl << endl << endl; );
                     //wait( cycleTime_ );
                    break;
                }
                else {
                    wait((*out_if_tonoc[2])->data_read_event());
                }
            }
        }
        else {
            wait((*back_fifo[2]).data_written_event());
            //wait( cycleTime_ );
        }
    }
}

void Crossbar::back3_selecter(){

    tlm::tlm_generic_payload *noc_trans = 0;
    
    while(true)
    {
        if((*back_fifo[3]).num_available())
        {
            noc_trans = (*back_fifo[3]).read();
            
            CROSSBAR_DEBUG(
                tlm::tlm_generic_payload *trans = 0;
                trans = NocPayloadExt::get_ip_trans( noc_trans );
                cout << "  got noc_trans in back3_selecter" << endl;
                cout << "  cmd: " << trans->get_command() << endl;
                cout << "  address: " << hex << trans->get_address() << endl;
                cout << "  source: " << dec << NocPayloadExt::get_source_id( noc_trans ) << endl;
                cout << "  target: " << dec << noc_trans->get_address() << endl;
            );
            
            while(true)
            {
                if((*out_if_tonoc[3])->num_free() > 0)
                {
                    (*out_if_tonoc[3])->write(noc_trans);
                     CROSSBAR_DEBUG( cout << "  write noc_trans in out_if_tonoc[3]" << endl << endl << endl; );
                     //wait( cycleTime_ );
                    break;
                }
                else {
                    wait((*out_if_tonoc[3])->data_read_event());
                }
            }
        }
        else {
            wait((*back_fifo[3]).data_written_event());
            //wait( cycleTime_ );
        }
    }
}

