#ifndef PE_H
#define PE_H

#include "port.h"
#include "emu.h"
#include <fstream>
#include <sstream>

class PE_Rec_inf{
public:
	unsigned int Src_PE_ID;
	unsigned int Dst_PE_ID;
	long double send_time;
	long double finished_time;

	PE_Rec_inf(unsigned int Src, unsigned int Dst, long double send, long double finished){
		Src_PE_ID = Src;
		Dst_PE_ID = Dst;
		send_time = send;
		finished_time = finished;
	}
};

class PE
{
 public:
   unsigned int ID;
   unsigned int row_dim;
   unsigned int col_dim;
   long double freq;//MHz
   long double clk;//ns
   unsigned int data_monitor;//record total data through it
   port* tx_p;
   port* rx_p;
   
   PE(int pe_id,unsigned int row,unsigned int col,double f,unsigned int* s_value)
   {
     ID=pe_id;
     freq=f;
     row_dim=row;
     col_dim=col;
     clk=(1/f)*1000;//ns
     //set simulation resolution
     data_monitor = 0;
     clk=clk*1000;
     clk=int(clk);
     clk=clk/1000;
     tx_p = new port(ID,tx,IP,clk,FIFO,s_value);
     rx_p = new port(ID,rx,IP,clk,FIFO,s_value);
   }


   ~PE()
   {
     delete tx_p;
     delete rx_p;
   }

   void CorrectTxRxPort()
   {
		 if(!tx_p->re_id.empty())
	     tx_p->correct_last_flit();
		 if(!rx_p->re_id.empty())
       rx_p->correct_last_flit();
   }

   void ResetTxRxPort()
   {
     tx_p->release_port();
     rx_p->release_port();
   }

   void SetFreq(long double f)
   {
     freq=f;
     clk=(1.0/f)*1000;
     clk=clk*1000;
     clk=int(clk);
     clk=clk/1000;
   }
                                          

};

#endif
