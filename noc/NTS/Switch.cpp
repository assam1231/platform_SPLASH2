#include <cstdlib>
#include "Switch.h"


void Switch::ArbitrateTxPort(port* tx_port)
{
  port* temp_port=NULL;
	unsigned int tmp_ptr=0;
	long double tmp_header=0;
	Arbitration tmp_arbiter = arbiter;
	for(unsigned int rx_dir=0; rx_dir < 5; rx_dir++){
		if(rx_p[rx_dir]->state==Busy && tx_port->port_dir==rx_p[rx_dir]->output_tx_dir){
			//cout << rx_p[rx_dir]->header_arrival_rx << endl;
			if(tmp_header == 0)
				tmp_header = rx_p[rx_dir]->header_arrival_rx;
			else if(tmp_header != rx_p[rx_dir]->header_arrival_rx){
				tmp_arbiter = FIFO;
				break;
			}
		}
	}
  //current all tx port in the same switch has the same arbitration
  switch(tmp_arbiter)//current only coding FIFO
  {
   case FIFO://first rx req first use tx port
//    for(unsigned int tx_dir=0;tx_dir < 5;tx_dir++)
//    { 
      //only Busy tx port need to arbitrate 
//      if(tx_p[tx_dir]->state == Busy)
//      {
        temp_port=NULL;
				//jlchiu:rx check seq:0->1->2->3->4
				//ckyu:rx check seq:1->2->3->4->0
        for(unsigned int rx_dir=0; rx_dir < 5; rx_dir++)
        {
         if(rx_p[rx_dir]->state==Busy && tx_port->port_dir==rx_p[rx_dir]->output_tx_dir)
         {
           //cout<<"rx"<<rx_dir<<" try to win tx"<<rx_p[rx_dir]->output_tx_dir<<endl;
					 //cout<<rx_p[rx_dir]->header_arrival_rx<<endl;
           if(temp_port==NULL)
             temp_port = rx_p[rx_dir];
					 else
           //else if(temp_port->port_dir != 0)
           {
             if(temp_port->header_arrival_rx >= rx_p[rx_dir]->header_arrival_rx)//header arrival early will win the tx
               temp_port = rx_p[rx_dir];
           }
         }
        }//check all rx ports request to the same tx port
        tx_port->header_time = temp_port->header_time;
        tx_port->occupy_p_id = temp_port->occupy_p_id;
//      }
//      else
//        continue;
      
//    }//check all tx ports 
    break;//end FIFO
   case RANDOM:
			while(1){
				tmp_ptr = rand() % 5;
				if(rx_p[tmp_ptr]->state==Busy && tx_port->port_dir==rx_p[tmp_ptr]->output_tx_dir){
					tx_port->header_time = rx_p[tmp_ptr]->header_time;
					tx_port->occupy_p_id = rx_p[tmp_ptr]->occupy_p_id;
					//tx_port->arbiter_ptr = (tx_port->arbiter_ptr+1)%5;
      		break;
				}
			}
      break;//end RANDOM
   case LRU:
      for(unsigned int rx_dir = 0; rx_dir < 5; rx_dir++){
				tmp_ptr = tx_port->arbiter_seq[rx_dir];
				//cout << "tx_port->arbiter_ptr " << tmp_ptr << endl;
				if(rx_p[tmp_ptr]->state==Busy && tx_port->port_dir==rx_p[tmp_ptr]->output_tx_dir){
					tx_port->header_time = rx_p[tmp_ptr]->header_time;
					tx_port->occupy_p_id = rx_p[tmp_ptr]->occupy_p_id;
					for(unsigned int i=0;i<4;i++){
						if(i<rx_dir)
							tx_port->next_arbiter_seq[i] = tx_port->arbiter_seq[i];
						else
							tx_port->next_arbiter_seq[i] = tx_port->arbiter_seq[i+1];
						//cout << "after " << i << " :" << tx_port->next_arbiter_seq[i] << endl;
					}
					tx_port->next_arbiter_seq[4] = tmp_ptr;
					//cout << "after 4 :" << tx_port->next_arbiter_seq[4] << endl;
      		break;
				}
			}
      break;//end LRU
   case ROUND_ROBIN:
      for(unsigned int rx_dir = 0; rx_dir < 5; rx_dir++){
				tmp_ptr = (rx_dir+tx_port->arbiter_ptr)%5;
				if(rx_p[tmp_ptr]->state==Busy && tx_port->port_dir==rx_p[tmp_ptr]->output_tx_dir){
					tx_port->header_time = rx_p[tmp_ptr]->header_time;
					tx_port->occupy_p_id = rx_p[tmp_ptr]->occupy_p_id;
					tx_port->next_arbiter_ptr = (tmp_ptr + 1)%5;
      		break;
				}
			}
			break;//end ROUND_ROBIN
   case FIXED:
      for(unsigned int rx_dir = 0; rx_dir < 5; rx_dir++){
				tmp_ptr = tx_port->arbiter_seq[rx_dir];
				if(rx_p[tmp_ptr]->state==Busy && tx_port->port_dir==rx_p[tmp_ptr]->output_tx_dir){
					tx_port->header_time = rx_p[tmp_ptr]->header_time;
					tx_port->occupy_p_id = rx_p[tmp_ptr]->occupy_p_id;
      		break;
				}
			}
			break;//end FIXED
    default:
      cout<<"Error in Switch arbitration!!"<<endl;
  }
}

void Switch::CorrectTxRxPort()
{
	for(unsigned int i=0;i<5;i++){
		if(!tx_p[i]->re_id.empty())
			tx_p[i]->correct_last_flit();
		if(!rx_p[i]->re_id.empty())
			rx_p[i]->correct_last_flit();
	}
}

void Switch::ResetTxRxPort()
{
	#ifdef D_DEBUG1
	printf("Switch ID %d\n", ID);
	#endif
  for(unsigned int i=0;i<5;i++)
  {
   tx_p[i]->release_port();
   rx_p[i]->release_port();
  }
}

