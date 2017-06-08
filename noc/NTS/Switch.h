#ifndef SWITCH_H
#define SWITCH_H
#include <iostream>
#include "emu.h"
#include "port.h"
using namespace std;

class Switch
{
  public:
   unsigned int ID;
   unsigned int row_dim;
   unsigned int col_dim;
   long double freq;//MHz
   long double clk;//ns
   unsigned int data_monitor;//record total data through it
   //DIR {IP=0,N=1,E=2,S=3,W=4};
   port* tx_p[5];
   port* rx_p[5];

   Arbitration arbiter;

   Switch(int sw_id,unsigned int row,unsigned int col,double f,Arbitration A,unsigned int* s_value)
   {
     ID=sw_id;
     row_dim=row;
     col_dim=col;
     freq=f;
     arbiter=A;
     data_monitor = 0;
     clk=(1/f)*1000;
     //set simulation resolution
     clk = clk*1000;
     clk = int(clk);
     clk = clk/1000;
     //create each tx & rx port
     tx_p[0]= new port(sw_id,tx,IP,clk,A,s_value);//switch id & port direction
     rx_p[0]= new port(sw_id,rx,IP,clk,A,s_value);
     tx_p[1]= new port(sw_id,tx,N,clk,A,s_value);
     rx_p[1]= new port(sw_id,rx,N,clk,A,s_value);
     tx_p[2]= new port(sw_id,tx,E,clk,A,s_value);
     rx_p[2]= new port(sw_id,rx,E,clk,A,s_value);
     tx_p[3]= new port(sw_id,tx,S,clk,A,s_value);
     rx_p[3]= new port(sw_id,rx,S,clk,A,s_value);
     tx_p[4]= new port(sw_id,tx,W,clk,A,s_value);
     rx_p[4]= new port(sw_id,rx,W,clk,A,s_value);
   }
   ~Switch()
   {
     for(unsigned int i=0;i<5;i++)
     {
       delete tx_p[i];
       delete rx_p[i];
     }
   }


   void SetFreq(long double f)
   {
     freq=f;
     clk=(1.0/f)*1000;
     clk=clk*1000;
     clk=int(clk);
     clk=clk/1000;
   }
                                          
   void ArbitrateTxPort(port* tx_port);
	 void CorrectTxRxPort();
   void ResetTxRxPort();

};
#endif
