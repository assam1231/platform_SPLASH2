#include <iostream>
#include "Packet.h"

#ifdef D_DEBUG_SW
unsigned int tmp_sw_id = 1;
unsigned int tmp_port_dir = 4;
#endif

using namespace std;
//example(IP=0,N=1,E=2,S=3,W=4)
//SW_path:SW3->SW2->SW1->SW5->SW9->SW13
//rx_path: IP  E     E    N    N    N
//tx_port: W   W     S    S    S    IP
void Packet::SearchTxRxPortPath()
{ //Base on X-Y routing
  //put Src PE tx port in TrPath_rx;
  TrPath_rx.push_back(Src_PE->tx_p);
  TrPath_tx.push_back(Src_PE->tx_p);
  //put switch IP rx_port first,then put each SW rx port in vec
  for(unsigned int i=0;i<TrPath_SW.size();i++)
  {
    //decide rx port first,compare to PreSwitch
    /*if(i==0){//first Swith rx port must be IP dir or extmem
		 	if(Src_PE->ID == 16)
	     	TrPath_rx.push_back(TrPath_SW[0]->rx_p[1]);
		 	else
	     	TrPath_rx.push_back(TrPath_SW[0]->rx_p[0]);
	}*/
	
	// 4 ext mem 16(0,1) 17(1,3) 18(2,0) 19(3,2) by along
	if(i == 0){
		if     (Src_PE->ID == 16) TrPath_rx.push_back(TrPath_SW[0]->rx_p[1]);
#ifdef NTS_4_external_memory
		else if(Src_PE->ID == 17) TrPath_rx.push_back(TrPath_SW[0]->rx_p[2]);
		else if(Src_PE->ID == 18) TrPath_rx.push_back(TrPath_SW[0]->rx_p[3]);
		else if(Src_PE->ID == 19) TrPath_rx.push_back(TrPath_SW[0]->rx_p[4]);
#endif
		else                      TrPath_rx.push_back(TrPath_SW[0]->rx_p[0]);
	}
    else
    {//col compare first then compare row second
      if(TrPath_SW[i]->col_dim > TrPath_SW[i-1]->col_dim)
        TrPath_rx.push_back(TrPath_SW[i]->rx_p[4]);
      else if(TrPath_SW[i]->col_dim < TrPath_SW[i-1]->col_dim)
        TrPath_rx.push_back(TrPath_SW[i]->rx_p[2]);
      else if(TrPath_SW[i]->row_dim > TrPath_SW[i-1]->row_dim)
        TrPath_rx.push_back(TrPath_SW[i]->rx_p[1]);
      else if(TrPath_SW[i]->row_dim < TrPath_SW[i-1]->row_dim)
        TrPath_rx.push_back(TrPath_SW[i]->rx_p[3]);
      else
        cout<<"Error in find Trffic Rx port Path"<<endl;
    }

    //decide tx port second,compare to Next Switch
    /*if(i==(TrPath_SW.size()-1)){//last switch tx port must be IP dir or ext mem
		 	if(Dst_PE->ID == 16)
	      TrPath_tx.push_back(TrPath_SW[i]->tx_p[1]);
			else
	      TrPath_tx.push_back(TrPath_SW[i]->tx_p[0]);
		}*/
	
	// 4 ext mem 16(0,1) 17(1,3) 18(2,0) 19(3,2) by along
	if(i == (TrPath_SW.size()-1)){
		if     (Dst_PE->ID == 16)  TrPath_tx.push_back(TrPath_SW[i]->tx_p[1]);
#ifdef NTS_4_external_memory
		else if(Dst_PE->ID == 17)  TrPath_tx.push_back(TrPath_SW[i]->tx_p[2]);
		else if(Dst_PE->ID == 18)  TrPath_tx.push_back(TrPath_SW[i]->tx_p[3]);
		else if(Dst_PE->ID == 19)  TrPath_tx.push_back(TrPath_SW[i]->tx_p[4]);
#endif
		else                       TrPath_tx.push_back(TrPath_SW[i]->tx_p[0]);
	}
    else
    {//col compare first then compare row second
      if(TrPath_SW[i]->col_dim > TrPath_SW[i+1]->col_dim)
        TrPath_tx.push_back(TrPath_SW[i]->tx_p[4]);
      else if(TrPath_SW[i]->col_dim < TrPath_SW[i+1]->col_dim)
        TrPath_tx.push_back(TrPath_SW[i]->tx_p[2]);
      else if(TrPath_SW[i]->row_dim > TrPath_SW[i+1]->row_dim)
        TrPath_tx.push_back(TrPath_SW[i]->tx_p[1]);
      else if(TrPath_SW[i]->row_dim < TrPath_SW[i+1]->row_dim)
        TrPath_tx.push_back(TrPath_SW[i]->tx_p[3]);
      else
        cout<<"Error in find Trffic Tx port Path"<<endl;
    }

  }
  //put Dst_PE rx port in vec at last
  TrPath_rx.push_back(Dst_PE->rx_p);
  TrPath_tx.push_back(Dst_PE->rx_p);

}

//when header write in FIFO, FIFO may have other congestion traffic's flit
//consider this flit's congestion effect and record in congestion infformation
//Use the difference between congestion "tx port" and "next rx port" header arrival time
//to calculate FIFO status
void Packet::FIFOStatusAfterCongestion()
{
  unsigned int cong_step = 0;
  unsigned int cong_flits = 0;
  long double cong_clk=0;
  long double time_stamp1 =0;
  long double time_stamp2 =0;
  long double time_stamp3 =0;

  for(unsigned int i=0;i<vec_cong_port.size();i++)
  {
    cong_flits =0;
    cong_step = vec_cong_port.begin()->cong_step;
    //cong_clk  = vec_cong_port.begin()->cong_port->LastBoundCLK;
    cong_clk  = TrPath_rx[cong_step+1]->LastBoundCLK;
    vec_cong_port.begin()->cong_clk = cong_clk;
    //time_stamp = TrPath_rx[cong_step]->header_time;
    time_stamp1 = TrPath_rx[cong_step]->header_time+ (TrPath_rx[cong_step]->init_clk/2);//header write in FIFO
    time_stamp2 = TrPath_rx[cong_step+1]->header_time+ (TrPath_rx[cong_step+1]->init_clk/2);//next port header write in FIFO
    //diff_time = Through_BiSyncFIFO(time_stamp, TrPath_rx[cong_step]->init_clk,TrPath_rx[cong_step+1]->init_clk)-time_stamp;
    cong_flits = (unsigned int)((time_stamp2-time_stamp1)/cong_clk);
    if(cong_flits>0)
      cong_flits +=1;
    else
      cong_flits=0;
    //while((TrPath_rx[cong_step+1]->header_time-time_stamp) > diff_time)
    //{
    //  time_stamp += cong_clk;
    //  cong_flits ++;
    //}
    if(cong_flits > FIFO_size+(unsigned int)TrPath_rx[cong_step]->inputPipe)
      cong_flits = FIFO_size+TrPath_rx[cong_step]->inputPipe;

    if((TrPath_rx[cong_step+1]->header_time - TrPath_rx[cong_step]->header_time) <=(TrPath_rx[cong_step]->init_clk+TrPath_rx[cong_step+1]->init_clk))
      cong_flits = 0;
   
    time_stamp3 = ((time_stamp1/TrPath_rx[cong_step+1]->init_clk)+1)*time_stamp1/TrPath_rx[cong_step+1]->init_clk;//If FIFO is empty
    if((time_stamp2 = TrPath_rx[cong_step+1]->header_time - TrPath_rx[cong_step+1]->init_clk))
      cong_flits = 0;

    vec_cong_port.begin()->cong_flits = cong_flits;
    //cout<<"congestion at step["<<cong_step<<"] congestion clk="<<cong_clk<<"congestion flits="<<cong_flits<<endl;
  }
}
//According to the new Packet 
//Search for all of the possible congestion involved packets on its path
//find it and throw all of the possible packets into the unfinished packet
//do the static timing simulator again
void Packet::SimulateSimpleHeaderTimetoRx(double slowest_clk)
{//calculate header receive time by rx port on switch in path
  long double tmp_header_t;
	header_time_list = new long double[TrPath_rx.size()];
	tail_time_list = new long double[TrPath_rx.size()];
	for(unsigned int i=0;i<TrPath_rx.size();i++){
    //First simulate each header and tail to rx_port on it's path
    if(i==0){
      tmp_header_t=send_time + Src_PE->clk*1;
      header_time_list[i] = tmp_header_t;
      tail_time_list[i] = tmp_header_t + total_flits * Src_PE->clk;
      //printf("stage%u@PE_%u tx_port_%u %.2llf->%.2llf\n", i, TrPath_rx[i]->sw_id, TrPath_rx[i]->port_dir, header_time_list.back(), tail_time_list.back());
    }else{
      tmp_header_t=header_time_list[i-1];
      tmp_header_t=Through_BiSyncFIFO(tmp_header_t,TrPath_rx[i-1]->init_clk,TrPath_rx[i]->init_clk);
      header_time_list[i] = tmp_header_t;
      tail_time_list[i] = tmp_header_t + total_flits * slowest_clk;
      //printf("stage%u@SW_%u rx_port_%u %.2llf->%.2llf\n", i, TrPath_rx[i]->sw_id, TrPath_rx[i]->port_dir, header_time_list.back(), tail_time_list.back());
			//Record passed_pkt and flit_id to its according port
			TrPath_rx[i]->passed_pkt.push_back(this);
			TrPath_rx[i]->flit_id.push_back(i);
    }
	}
}
//calculate header time to rx port
//if rx port busy for other packet compare header arrival time
//only the least header arrival time can use the rx port
//if tx port has release_time must stack on to next rx port
void Packet::SimulateHeaderTimetoRx()
{//calculate header receive time by rx port on switch in path
  long double temp_header_t;
  congestion_inf cong_port(NULL,0);
  long double sync_time=0;
  long double Data_Read_from_FIFO =0;
  //set the direction of rx's output tx port
  for(unsigned int i=0; i<TrPath_rx.size();i++)
  {
    if(i==0 or i==(TrPath_rx.size()-1))
      continue;
    else
      TrPath_rx[i]->output_tx_dir = TrPath_tx[i]->port_dir;
  }
  vec_cong_port.clear();//for record congestion step

  for(unsigned int i=0;i<TrPath_rx.size();i++)
  {
   //cout<<"start to calculate step"<<i<<" header time"<<endl;
   temp_header_t=0;
   
   //calculate rx port header arrival time
   if(i==0)//Src PE tx port to first Switch rx port
   {
     //PE send Write command into NIU then NIU output header need two cycles depend on OCP BurstSeq
     temp_header_t= send_time + (Src_PE->clk*1) /*+ 0.001*/;//0.001(ns) is SystemC simulation resolution
     //Src_PE->tx_p->header_time =temp_header_t;//set Src_PE tx header & tail time
     //Src_PE->tx_p->header_arrival_rx=temp_header_t;
		 #ifdef D_DEBUG1
 		 ShowSWPath();
		 cout << "Src_PE header_arrival_rx = " << temp_header_t << " header_time = " << TrPath_rx[i]->header_time << " release_time = " << TrPath_rx[i]->release_time << " size = " << TrPath_rx.size() << endl;
		 for(unsigned int j=0;j<TrPath_rx[i]->last_flits.size();j++)
			 printf("last_flits %d:(%.2llf,%u)\n", j, TrPath_rx[i]->last_flits[j].rec_time, TrPath_rx[i]->last_flits[j].flit_packet_id);
		 #endif
   }
   else//Switch and Dst_PE header arrival rx port 
   {//consider tx port be used which has release_time
     
     temp_header_t = TrPath_rx[i-1]->header_time;
		 #ifdef D_DEBUG_SW
		 if(TrPath_rx[i]->sw_id == tmp_sw_id && TrPath_rx[i]->port_dir == tmp_port_dir){
		 	 cout<<"@("<<TrPath_rx[i]->sw_id<<","<<TrPath_rx[i]->port_dir<<")\n";
		 	 for(unsigned int j=0;j<TrPath_rx[i]->last_flits.size();j++)
			 	 printf("last_flits %d:(%.2llf,%u)\n", j, TrPath_rx[i]->last_flits[j].rec_time, TrPath_rx[i]->last_flits[j].flit_packet_id);
			 cout<<"temp_header_t = TrPath_rx[i-1]->header_time = "<<temp_header_t<<endl;
		 }
		 #endif
     temp_header_t = Through_BiSyncFIFO(temp_header_t,TrPath_rx[i-1]->init_clk,TrPath_rx[i]->init_clk);//Cross BiSyncFIFO
		 #ifdef D_DEBUG_SW
		 if(TrPath_rx[i]->sw_id == tmp_sw_id && TrPath_rx[i]->port_dir == tmp_port_dir){
		 	 cout<<"temp_header_t = Through_BiSyncFIFO = "<< temp_header_t << endl;
		 }
		 #endif
     TrPath_rx[i]->header_arrival_rx=temp_header_t;//if FIFO empty
		 #ifdef D_DEBUG_SW
		 if(TrPath_rx[i]->sw_id == tmp_sw_id && TrPath_rx[i]->port_dir == tmp_port_dir){
		 	 cout << i << " header_arrival_rx = " << temp_header_t << endl;
		 }
		 #endif
   }
   
   //assign temp_header_t into TrPath_rx[i]->header_time
   //then calculate tail arrival time
   if((TrPath_rx[i]->state == Busy) && (TrPath_rx[i]->header_time < temp_header_t)){//header rob rx port unsuccessfully
		 #ifdef D_DEBUG1
		 printf("sw %d header %d header_time:%.2llf < temp_header_t:%.2llf\n", TrPath_rx[i]->sw_id, i, TrPath_rx[i]->header_time, temp_header_t);
		 for(unsigned int j=0;j<TrPath_rx[i]->last_flits.size();j++)
			 printf("last_flits %d:(%.2llf,%u)\n", j, TrPath_rx[i]->last_flits[j].rec_time, TrPath_rx[i]->last_flits[j].flit_packet_id);
		 cout << "header rob rx port unsuccessfully! break!!" << endl;
		 #endif
     break;//break for loop no need to calculate forward.
	 }else if(temp_header_t > TrPath_rx[i]->min_header_arrival_rx && TrPath_rx[i]->min_header_arrival_rx != 0)//arbitrate for multi packets pass the same port and the release_time is the same
	 {
		 #ifdef D_DEBUG1
		 cout << "arbitrate for multi packets, header rob rx port unsuccessfully! break!!" << endl;
		 #endif
		 break;
   }else//header rob rx port successfully(rx=Idle or header arrival early)
   {
     TrPath_rx[i]->state=Busy;
		 TrPath_rx[i]->min_header_arrival_rx = temp_header_t;
     if((i==(TrPath_rx.size()-1)))//Dst PE
     {
       if(TrPath_rx[i]->release_time >=temp_header_t)
       {
         TrPath_rx[i]->header_time=TrPath_rx[i]->release_time + TrPath_rx[i]->init_clk;
         //cong_port.set(TrPath_rx[i],i);
         //vec_cong_port.push_back(cong_port);
       }
       else
         TrPath_rx[i]->header_time= temp_header_t;
     }
		 else if(i==0)//Src PE
		 {
			 if(temp_header_t < Src_PE->tx_p->header_arrival_rx || Src_PE->tx_p->header_arrival_rx == 0){
       	 if(TrPath_rx[i]->release_time/*+TrPath_rx[i]->init_clk*/ >=temp_header_t)
         {
					 //carlrich: may cause bug, if release_time+init_clk>temp_header_t:750+20>760
           TrPath_rx[i]->header_time=TrPath_rx[i]->release_time + TrPath_rx[i]->init_clk;
           //cong_port.set(TrPath_rx[i],i);
           //vec_cong_port.push_back(cong_port);
         }
         else
           TrPath_rx[i]->header_time= temp_header_t;
				 Src_PE->tx_p->header_arrival_rx=temp_header_t;
			 }else{
				 //printf("%d\n", Src_PE->ID);
				 //cout<<temp_header_t<<":"<<Src_PE->tx_p->header_arrival_rx<<endl;
				 break;
			 }
		 }
     else//SW
     {
			 #ifdef D_DEBUG_SW
			 printf("@(%d,%d) release_time %.2llf temp_header_t %.2llf\n", TrPath_rx[i]->sw_id, TrPath_rx[i]->port_dir, TrPath_rx[i]->release_time, temp_header_t);
			 #endif
       if(TrPath_rx[i]->release_time >=temp_header_t)
       {
         TrPath_rx[i]->header_time=TrPath_rx[i]->release_time + TrPath_rx[i]->init_clk;
         TrPath_rx[i]->header_arrival_rx=TrPath_rx[i]->release_time;
         //cong_port.set(TrPath_rx[i],i);//congestion at rx port
         //vec_cong_port.push_back(cong_port);
//carlrich:pipe
				 if(TrPath_rx[i]->release_time == temp_header_t && (int)TrPath_rx[i]->inputPipe + (int)TrPath_rx[i]->crossbarPipe + (int)TrPath_rx[i]->lateArb > 1){
					 TrPath_rx[i]->header_time += TrPath_rx[i]->init_clk;
				 }
       }
       else{
         TrPath_rx[i]->header_time= temp_header_t;
//carlrich:pipe
				 TrPath_rx[i]->header_time += TrPath_rx[i]->inputPipe * TrPath_rx[i]->init_clk;
				 TrPath_rx[i]->header_time += TrPath_rx[i]->crossbarPipe * TrPath_rx[i]->init_clk;
				 TrPath_rx[i]->header_time += TrPath_rx[i]->lateArb * TrPath_rx[i]->init_clk;
			 }
			 #ifdef D_DEBUG_SW
			 printf("result: hearder_arrival_rx %.2llf header_time %.2llf\ntx_release_time %.2llf\n", TrPath_rx[i]->header_arrival_rx, TrPath_rx[i]->header_time, TrPath_tx[i]->release_time);
			 #endif
       
       //after header already at rx port, check tx port
       if(TrPath_tx[i]->release_time >= TrPath_rx[i]->header_time)
       {

         TrPath_rx[i]->header_time = TrPath_tx[i]->release_time+TrPath_rx[i]->init_clk;
         //consider FIFO has space
         sync_time =((int)(TrPath_rx[i+1]->last_flits[0].rec_time/TrPath_rx[i]->init_clk)+1)*TrPath_rx[i]->init_clk; 
         if((sync_time-TrPath_rx[i+1]->last_flits[0].rec_time) <= (TrPath_rx[i]->init_clk/2))
           Data_Read_from_FIFO = sync_time + TrPath_rx[i]->init_clk/2;
         else
           Data_Read_from_FIFO = TrPath_rx[i+1]->last_flits[0].rec_time;
				 #ifdef D_DEBUG_SW
		 		 if(TrPath_rx[i]->sw_id == tmp_sw_id && TrPath_rx[i]->port_dir == tmp_port_dir){
				 	 cout<<"In SimulateHeaderTimetoRx: sync_time:"<<sync_time<<" last_flits:"<<TrPath_rx[i+1]->last_flits[0].rec_time<<","<<TrPath_rx[i+1]->last_flits.size()<<endl;
			 	 }
				 #endif

				 if(FIFO_size+TrPath_rx[i]->inputPipe != 0){
					 while(TrPath_rx[i]->header_time <= Data_Read_from_FIFO)
						 TrPath_rx[i]->header_time += TrPath_rx[i]->init_clk;
			   }

         //TrPath_rx[i]->header_time=TrPath_tx[i]->release_time+ TrPath_rx[i]->init_clk;
         cong_port.set(TrPath_tx[i],i);//congestion at tx port
         vec_cong_port.push_back(cong_port);
       }
//carlrich:pipe
			 if(TrPath_rx[i]->lateArb == true && TrPath_tx[i]->release_time + TrPath_rx[i]->init_clk == TrPath_rx[i]->header_time){
				 TrPath_rx[i]->header_time += TrPath_rx[i]->init_clk;
				 //cout<<"lateArb:"<<TrPath_rx[i]->header_time<<endl;
			 }

     }
		 #ifdef D_DEBUG_SW
		 if(TrPath_rx[i]->sw_id == tmp_sw_id && TrPath_rx[i]->port_dir == tmp_port_dir){
		 	 cout << "port rob successfully!\n" << endl;
		 }
		 #endif
     TrPath_rx[i]->occupy_p_id=ID;
     TrPath_rx[i]->occupy_g_id=packet_ID;
     //cout<<"step"<<i<<" packet"<<ID<<"win rx port at"<<TrPath_rx[i]->header_time<<endl;
    
     //Arbitrate output tx port
     if(i!=0 && i!=(TrPath_rx.size()-1))//for all SW(except Src_PE & Dst_PE)
     {
       TrPath_tx[i]->state=Busy;
       //Check if it can get output tx port
       //cout<<"start arbi tx port"<<endl;
       TrPath_SW[i-1]->ArbitrateTxPort(TrPath_tx[i]);
       //cout<<"end arbi tx port"<<endl;
       if(TrPath_tx[i]->occupy_p_id == ID) 
         continue;
       else
       {
		 		 #ifdef D_DEBUG_SW
				 if(TrPath_rx[i]->sw_id == tmp_sw_id && TrPath_rx[i]->port_dir == tmp_port_dir){
         	 cout<<"can't transfer forward!!!"<<endl;
		 		 }
			 	 #endif
         break;
       }
     }
   }
   
    
  }//end calculate header & tail arrival time rx port on switch in path
 
  //if congestion happend, consider congestion traffic full FIFO and effect header time
  //congestion always at tx port
/*  unsigned int cong_step=0;
  long double diff_time=0;
  if(!vec_cong_port.empty())
  {
    for(unsigned int i=0; i<vec_cong_port.size(); i++)
    {
      //if((i != (vec_cong_port.size()-1)) && (vec_cong_port[i].cong_step==(vec_cong_port[i+1].cong_step-1)))
      //{ 
      //  cout<<"skip"<<endl;
      //  continue;//serial congestion has no this effection
      //}
      cong_step = vec_cong_port[i].cong_step;
      diff_time = TrPath_rx[cong_step+1]->header_time - TrPath_rx[cong_step]->header_time;
      //cout<<"diff_time="<<diff_time<<endl;
      //cout<<"cong_step="<<cong_step<<"tx port, last bound clk= "<<TrPath_tx[cong_step]->LastBoundCLK<<endl;
      cout<<int(diff_time/(TrPath_rx[cong_step+1]->LastBoundCLK))<<endl;
      if(int(diff_time/(TrPath_rx[cong_step+1]->LastBoundCLK)) >= FIFO_size)
      {
        //cout<<"before = "<<TrPath_rx[cong_step]->header_time<<endl;
        TrPath_rx[cong_step]->header_time -= TrPath_rx[cong_step]->init_clk;
        TrPath_rx[cong_step]->header_time += TrPath_rx[cong_step+1]->LastBoundCLK;
        //cout<<"LastBoundCLK="<<TrPath_rx[cong_step+1]->LastBoundCLK<<endl;
        //cout<<"after = "<<TrPath_rx[cong_step]->header_time<<endl;
      }
    }
    //set congestion flits in FIFO and congestion clk
    FIFOStatusAfterCongestion();
  }
  */
}

//the port clk maybe bound by forward component's clk
void Packet::SetPortFunctionalClk()
{
  long double temp_clk=0;
  temp_clk = TrPath_rx[0]->functional_clk;
  for(unsigned int i=0;i< TrPath_rx.size();i++)
  {
    if(temp_clk >= TrPath_rx[i]->functional_clk)
      TrPath_rx[i]->functional_clk = temp_clk;
    else
      temp_clk = TrPath_rx[i]->functional_clk;

    //cout<<"port"<<i<<" functional clk="<<temp_clk;
  }
	//set arbiter_seq and arbiter_ptr
	for(unsigned int i=1;i<TrPath_tx.size()-1;i++){
		if(TrPath_tx[i]->arbiter_port == LRU){
			for(unsigned int j=0;j<5;j++){
				TrPath_tx[i]->arbiter_seq[j] = TrPath_tx[i]->next_arbiter_seq[j];
				//cout << "after 4 :" << TrPath_tx[i]->arbiter_seq[4] << endl;
			}
		}else if(TrPath_tx[i]->arbiter_port == ROUND_ROBIN){
			TrPath_tx[i]->arbiter_ptr = TrPath_tx[i]->next_arbiter_ptr;
		}
	}

  //cout<<endl;
}


//let fifo effect match the same clock phase
unsigned int Packet::UnboundData(long double input_time,long double input_clk,long double output_clk,unsigned int fifo_size,unsigned int data_already_in)
{
  unsigned int real_fifo_size=0;
  long double first_data_out=0;
  long double diff_time=0;
  unsigned int input_datas=0;
  
  input_time = ((unsigned int)(input_time/input_clk))*input_clk;//make input_time=N x input_clk
  real_fifo_size = fifo_size - data_already_in;
  
  first_data_out =((unsigned int)(input_time/output_clk)+1)*output_clk;
  
  diff_time = first_data_out - input_time;
  
  input_datas =(unsigned int)(diff_time / input_clk);
  //cout<<"input_clk="<<input_clk<<" output_clk="<<output_clk<<endl;
  //cout<<"input_time="<<input_time<<" first_data_out="<<first_data_out<<endl;
  //cout<<"real fifo size="<<real_fifo_size<<" input_datas="<<input_datas<<endl;
  if((input_datas-1) > real_fifo_size)
    return real_fifo_size;
  else
    return (unsigned int)(((real_fifo_size-(input_datas-1))/(1-(input_clk/output_clk))) + input_datas);
 
}


//When packet's path without congestion the tail arrival
//each SW time can calculate easy.
void Packet::SimulateTailTimetoRxWithoutCongestion()
{
  long double slowest_clk=0;
  unsigned int slowest_component=0;
  slowest_clk = SlowestClkinPath();
  long double rec_time=0;
  rec_inf rec_inf_tmp(0,0);
  //after slowest component, speed all bound by slowes clk
  for(unsigned int i=0;i<TrPath_rx.size();i++)
  {  
    if(TrPath_rx[i]->init_clk == slowest_clk)
    {
      slowest_component = i;
      for(unsigned int j=i;j<TrPath_rx.size();j++)
      {
        rec_time=TrPath_rx[j]->header_time;
				//printf("header_time %.2llf\n", rec_time);
        for(unsigned int flit=0;flit < total_flits ;flit++)
        { //record each flit receive time;
          rec_inf_tmp.set(rec_time,slowest_clk,false,packet_ID);
          TrPath_rx[j]->vec_rec_inf.push_back(rec_inf_tmp);
					#ifdef D_DEBUG
					//cout<<"In SimulateTailTimetoRxWithoutCongestion: flit_num:"<<flit<<" vec_rec_inf:"<<TrPath_rx[j]->vec_rec_inf.back().rec_time<<","<<TrPath_rx[j]->vec_rec_inf.size()<<endl;
					#endif
          rec_time += slowest_clk;
        }
        TrPath_rx[j]->tail_time = TrPath_rx[j]->vec_rec_inf[total_flits-1].rec_time;
      }
        
      break;
    }
  }
  //calculate tail time for component before slowest component
  //unsigned int remaining_flits;
  unsigned int flits_buf;//functional data FIFO between target and bounded component
  long double bound_clk=0;
  int target_flit=0;
  int compare_flit=0;
  for(int target_port=(slowest_component-1);target_port>=0;target_port--)
  {
    bound_clk = TrPath_rx[target_port]->functional_clk;
    rec_time = TrPath_rx[target_port]->header_time;
    rec_inf_tmp.set(rec_time,bound_clk,false,packet_ID);
    TrPath_rx[target_port]->vec_rec_inf.push_back(rec_inf_tmp);
    //remaining_flits = total_flits - 1;//header is receive by rx

    flits_buf = FIFO_size+TrPath_rx[target_port]->inputPipe + 1;//syncFIFO + rx_buffer,between two rx port.
    
    //calculate each flit receive time by flit buffer size and "the port after target port"
    while(TrPath_rx[target_port]->vec_rec_inf.size() != total_flits)
    { 
      //the flit we want ti know the receive time
      target_flit = TrPath_rx[target_port]->vec_rec_inf.size()+1;
      //the flit is in the port after target port,compare_flit may bound target flit
      compare_flit = target_flit - flits_buf;//consider flit buffer effect
      
      if(compare_flit <= 0)//data put in FIFO
      {
        rec_time += bound_clk;
        //rec_inf_tmp.set(rec_time,bound_clk);
        rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[target_port]->vec_rec_inf.back().rec_time,false,packet_ID);
        TrPath_rx[target_port]->vec_rec_inf.push_back(rec_inf_tmp);
      }
      else
      {
        if((rec_time - TrPath_rx[target_port]->vec_rec_inf[target_flit-1-1].rec_time) >= bound_clk )
          rec_time += TrPath_rx[target_port]->init_clk;
        else
          rec_time += bound_clk;
        
        if(rec_time > TrPath_rx[target_port+1]->vec_rec_inf[compare_flit-1].rec_time)
        {
          if((rec_time - TrPath_rx[target_port+1]->vec_rec_inf[compare_flit-1].rec_time) <= (TrPath_rx[target_port]->init_clk/2))
            rec_time += TrPath_rx[target_port]->init_clk;//special case:FIFO full and next rx receive after half of target rx clk

          //rec_inf_tmp.set(rec_time,(rec_time - TrPath_rx[target_port]->vec_rec_inf[target_flit-1-1].rec_time));
          rec_inf_tmp.set(rec_time,  rec_time-TrPath_rx[target_port]->vec_rec_inf.back().rec_time,false,packet_ID);
          TrPath_rx[target_port]->vec_rec_inf.push_back(rec_inf_tmp);
          
          if(rec_inf_tmp.bound_clk >= slowest_clk)//after target flit all flits bound by slowest clk
           bound_clk = slowest_clk; 
        }
      }
    }//end while loop
    //cout<<"tail time="<<TrPath_rx[target_port]->vec_rec_inf[target_flit-1].rec_time<<endl;
    TrPath_rx[target_port]->tail_time = TrPath_rx[target_port]->vec_rec_inf[target_flit-1].rec_time;
  }
  //debug
//  for(unsigned int i=0;i<TrPath_rx.size();i++)
//    for(unsigned int j=0;j<TrPath_rx[i]->vec_rec_inf.size();j++)
//      cout<<"port="<<i<<" flit="<<j<<" receive time="<<TrPath_rx[i]->vec_rec_inf[j].rec_time<<endl;
}

//Find slowest clk between two component
long double Packet::FindSlowestCLK(unsigned int start_step, unsigned int end_step)
{
  long double slowest_clk=0;
  slowest_clk = TrPath_rx[start_step]->init_clk;
  for(unsigned int i=start_step; i<=end_step;i++)
    if(slowest_clk < TrPath_rx[i]->init_clk)
      slowest_clk = TrPath_rx[i]->init_clk;

  return slowest_clk;
}
//NEW method, focus on stage rx port to next stage's rx port, consider FIFO size
//20120917
void Packet::SimulateFIFOSizeTime()
{
  //For this packet, all header arrival time are decided
  //consider last flits(number of FIFO size) into this packet vec_rec_inf
  rec_inf rec_inf_tmp(0,0);
	vector<unsigned int> bwd_stage;
	vector<long double> tmp_release_time;
	vector<long double> tmp_release_header_time;
  for(unsigned int stage=0;stage < TrPath_rx.size();stage++)
  {
    if(TrPath_rx[stage]->last_flits.size() != FIFO_size+(unsigned int)TrPath_rx[stage]->inputPipe+1)//check
      cout<<"error to initial vec_rec_inf"<<endl;

    //flit_tag=0 is compare to header, assume that fifo is full
    for(unsigned int flit_tag=1;flit_tag<TrPath_rx[stage]->last_flits.size();flit_tag++){
      TrPath_rx[stage]->vec_rec_inf.push_back(TrPath_rx[stage]->last_flits[flit_tag]);
			#ifdef D_DEBUG
			cout<<"In SimulateFIFOSizeTime: vec_rec_inf_size:"<<TrPath_rx[stage]->vec_rec_inf.size()<<" flit_tag:"<<flit_tag<<" last_flits:"<<TrPath_rx[stage]->last_flits[flit_tag].rec_time<<","<<TrPath_rx[flit_tag]->last_flits.size()<<endl;
			#endif
		}
    rec_inf_tmp.set(TrPath_rx[stage]->header_time,0,false,packet_ID);
    TrPath_rx[stage]->vec_rec_inf.push_back(rec_inf_tmp);
		#ifdef D_DEBUG3
    cout<<"stage"<<stage<<" has "<<TrPath_rx[stage]->vec_rec_inf.size()<<" flits"<<endl;
		cout<<TrPath_rx[stage]->release_time<<" "<<TrPath_rx[stage]->vec_rec_inf.back().rec_time<<endl;
		#endif
		if(TrPath_rx[stage]->release_time + TrPath_rx[stage]->init_clk == TrPath_rx[stage]->vec_rec_inf.back().rec_time && (int)stage-2 >= 0 && TrPath_rx[stage-1]->last_flits.back().rec_time == 0){
			tmp_release_header_time.push_back(TrPath_rx[stage-2]->vec_rec_inf.back().rec_time);
			tmp_release_time.push_back(TrPath_rx[stage-1]->vec_rec_inf.back().rec_time);
//carlrich:pipe
			tmp_release_header_time.back() += TrPath_rx[stage]->inputPipe * TrPath_rx[stage]->init_clk;
			tmp_release_time.back() -= TrPath_rx[stage]->crossbarPipe * TrPath_rx[stage]->init_clk;
			bwd_stage.push_back(stage - 2);
			#ifdef D_DEBUG3
			cout<<"wormhole\n";
			cout<<TrPath_rx[stage-2]->vec_rec_inf.back().rec_time<<endl;
			#endif
		}else if(TrPath_rx[stage]->lateArb == true && TrPath_rx[stage]->release_time + TrPath_rx[stage]->init_clk*2 == TrPath_rx[stage]->vec_rec_inf.back().rec_time && (int)stage-2 >= 0 && TrPath_rx[stage-1]->last_flits.back().rec_time == 0){
			tmp_release_header_time.push_back(TrPath_rx[stage-2]->vec_rec_inf.back().rec_time);
			tmp_release_time.push_back(TrPath_rx[stage-1]->vec_rec_inf.back().rec_time);
//carlrich:pipe
			tmp_release_header_time.back() += TrPath_rx[stage]->init_clk;
			tmp_release_header_time.back() += TrPath_rx[stage]->inputPipe * TrPath_rx[stage]->init_clk;
			tmp_release_time.back() -= TrPath_rx[stage]->crossbarPipe * TrPath_rx[stage]->init_clk;
			bwd_stage.push_back(stage-2);
			#ifdef D_DEBUG3
			cout<<"wormhole 2\n";
			cout<<TrPath_rx[stage-2]->vec_rec_inf.back().rec_time<<endl;
			cout<<TrPath_rx[stage-1]->vec_rec_inf.back().rec_time<<endl;
			cout<<tmp_release_header_time.back()<<endl;
			cout<<tmp_release_time.back()<<endl;
			#endif
		
		}
  }//after this loop, vec_rec_inf.size() = FIFO_size+TrPath_rx[stage]->inputPipe+1

  //cout<<"finished initial vec_rec_inf"<<endl;
  long double rec_time=0;
  long double Data_Write_into_FIFO =0;
  long double sync_time=0;
  long double Data_Read_from_FIFO =0;
  unsigned int target_flit =0;
  while(TrPath_rx.back()->vec_rec_inf.size() < (total_flits+FIFO_size+TrPath_rx.back()->inputPipe))
  {
    for(unsigned int stage=0; stage<TrPath_rx.size();stage++)
    {
      target_flit = TrPath_rx[stage]->vec_rec_inf.size();//calculate the receive time of target flit
      rec_time = TrPath_rx[stage]->vec_rec_inf.back().rec_time + TrPath_rx[stage]->init_clk;
//      cout<<"calculate stage"<<stage<<" flit"<<target_flit<<endl;
//carlrich:pipe
			for(unsigned int i=0;i<tmp_release_header_time.size();i++){
				if(TrPath_rx[stage]->bwdPipe == true){
					if(rec_time < (tmp_release_time[i]+((bwd_stage[i]-stage)*TrPath_rx[stage]->init_clk)) && rec_time > (tmp_release_header_time[i]+((bwd_stage[i]-stage)*TrPath_rx[stage]->init_clk))){
						rec_time = tmp_release_time[i]+((bwd_stage[i]-stage)*TrPath_rx[stage]->init_clk);
						break;
					}
				}else{
					if(rec_time < tmp_release_time[i] && rec_time > tmp_release_header_time[i]){
						rec_time = tmp_release_time[i];
						break;
					}

				}
			}

      if(stage == 0)//SrcPE
      {
       //only to consider FIFO has space, compare to next stage's rx port
       //synchronize
        sync_time =((int)(TrPath_rx[stage+1]->vec_rec_inf[target_flit-(FIFO_size+TrPath_rx[stage+1]->inputPipe+1)].rec_time/TrPath_rx[stage]->init_clk)+1)*TrPath_rx[stage]->init_clk; 
        if((sync_time-TrPath_rx[stage+1]->vec_rec_inf[target_flit-(FIFO_size+TrPath_rx[stage+1]->inputPipe+1)].rec_time) <= (TrPath_rx[stage]->init_clk/2))
          Data_Read_from_FIFO = sync_time + TrPath_rx[stage]->init_clk/2;
        else
         Data_Read_from_FIFO = TrPath_rx[stage+1]->vec_rec_inf[target_flit-(FIFO_size+TrPath_rx[stage+1]->inputPipe+1)].rec_time;

        do{
          if(rec_time > Data_Read_from_FIFO)
          {
            rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[stage]->vec_rec_inf.back().rec_time,false,packet_ID);
            TrPath_rx[stage]->vec_rec_inf.push_back(rec_inf_tmp);
            if(TrPath_rx[stage]->vec_rec_inf[target_flit].bound_clk > TrPath_rx[stage]->vec_rec_inf[target_flit-1].bound_clk)
            {
//              cout<<"stage="<<stage<<" flit"<<target_flit<<"call check"<<endl;
//              cout<<"before check, stage"<<stage+1<<" flit"<<target_flit-1<<" rec_time="<<TrPath_rx[stage+1]->vec_rec_inf[target_flit-1].rec_time;
              CheckNextStage(stage);//congestion effect
//              cout<<"after check, stage"<<stage+1<<" flit"<<target_flit-1<<" rec_time="<<TrPath_rx[stage+1]->vec_rec_inf[target_flit-1].rec_time<<endl;
            }
          }
          else
            rec_time += TrPath_rx[stage]->init_clk;
        }while(TrPath_rx[stage]->vec_rec_inf.size() == target_flit);//send flit successfully
      }
      else if(stage == TrPath_rx.size()-1)//DstPE
      {//only to consider if pre-stage's target_flit write into FIFO
        Data_Write_into_FIFO = TrPath_rx[stage-1]->vec_rec_inf[target_flit].rec_time + (TrPath_rx[stage-1]->init_clk / 2);
        do{
         if(rec_time > Data_Write_into_FIFO)
         {
           rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[stage]->vec_rec_inf.back().rec_time,false,packet_ID);
           TrPath_rx[stage]->vec_rec_inf.push_back(rec_inf_tmp);
         }
         else
           rec_time += TrPath_rx[stage]->init_clk;
        }while(TrPath_rx[stage]->vec_rec_inf.size() == target_flit);//send flit successfully
      }
      else//SW
      {//Firstly,consider if pre-stage's target_flit write into FIFO
        Data_Write_into_FIFO = TrPath_rx[stage-1]->vec_rec_inf[target_flit].rec_time + (TrPath_rx[stage-1]->init_clk / 2);
        while(rec_time <= Data_Write_into_FIFO)
          rec_time += TrPath_rx[stage]->init_clk;
  
       //Secondly,consider FIFO has space, compare to next stage's rx port
        sync_time =((int)(TrPath_rx[stage+1]->vec_rec_inf[target_flit-(FIFO_size+TrPath_rx[stage+1]->inputPipe+1)].rec_time/TrPath_rx[stage]->init_clk)+1)*TrPath_rx[stage]->init_clk; 
        if((sync_time-TrPath_rx[stage+1]->vec_rec_inf[target_flit-(FIFO_size+TrPath_rx[stage+1]->inputPipe+1)].rec_time) <= (TrPath_rx[stage]->init_clk/2))
          Data_Read_from_FIFO = sync_time + TrPath_rx[stage]->init_clk/2;
        else
          Data_Read_from_FIFO = TrPath_rx[stage+1]->vec_rec_inf[target_flit-(FIFO_size+TrPath_rx[stage+1]->inputPipe+1)].rec_time;
        
        //if(stage==2 && target_flit==3)
        //{
        //  cout<<"sync_time="<<sync_time<<endl;
        //  cout<<"Data_Read_from_FIFO="<<TrPath_rx[stage+1]->vec_rec_inf[target_flit-(FIFO_size+TrPath_rx[stage+1]->inputPipe+1)].rec_time<<endl;
        //}
        do{
         if(rec_time > Data_Read_from_FIFO)
         {
           rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[stage]->vec_rec_inf.back().rec_time,false,packet_ID);
           TrPath_rx[stage]->vec_rec_inf.push_back(rec_inf_tmp);
           if(TrPath_rx[stage]->vec_rec_inf[target_flit].bound_clk > TrPath_rx[stage]->vec_rec_inf[target_flit-1].bound_clk)
           {
//             cout<<"stage="<<stage<<" flit"<<target_flit<<"call check"<<endl;
//             cout<<"before check, stage"<<stage+1<<" flit"<<target_flit-1<<" rec_time="<<TrPath_rx[stage+1]->vec_rec_inf[target_flit-1].rec_time;
             if(TrPath_rx[stage]->vec_rec_inf[target_flit].bound_clk <= TrPath_rx[stage-1]->vec_rec_inf[target_flit].bound_clk)
             {} //do nothing,this flit doesn't congestion, just bunk by pr stage
             else
              CheckNextStage(stage);//congestion effect
//             cout<<"after check, stage"<<stage+1<<" flit"<<target_flit-1<<" rec_time="<<TrPath_rx[stage+1]->vec_rec_inf[target_flit-1].rec_time<<endl;    
           }
         }
         else
          rec_time += TrPath_rx[stage]->init_clk;
        }while(TrPath_rx[stage]->vec_rec_inf.size() == target_flit);//send flit successfully
      }
    }//end loop, each component send a flit
  }
  for(unsigned int i=0;i<TrPath_rx.size();i++)
    TrPath_rx[i]->tail_time = TrPath_rx[i]->vec_rec_inf.back().rec_time;
  //debug
  //for(unsigned int flit_tag=0;flit_tag<TrPath_rx[0]->vec_rec_inf.size();flit_tag++)
  //{
  //  for(unsigned int stage=0;stage<TrPath_rx.size();stage++)
  //  {
  //    cout<<"stage="<<stage<<" flit="<<flit_tag<<" receive time="<<TrPath_rx[stage]->vec_rec_inf[flit_tag].rec_time<<endl;
  //    
  //  }
  //}
  //debug
//  for(unsigned int stage=0;stage<TrPath_rx.size();stage++)
//    for(unsigned int flit_tag=0;flit_tag<TrPath_rx[0]->vec_rec_inf.size();flit_tag++)
//      cout<<"stage="<<stage<<" flit="<<flit_tag<<" receive time="<<TrPath_rx[stage]->vec_rec_inf[flit_tag].rec_time<<endl;

}
//Check if next stage get data when data not write in FIFO yet
void Packet::CheckNextStage(unsigned int stage)
{
  rec_inf rec_inf_tmp(0,0);
  long double flit_check_Write_into_FIFO = 0;
  long double Data_Write_into_FIFO =0;
  long double Data_Read_from_FIFO = 0;
  unsigned int flit_last=0;
  unsigned int flit_check=0;
  unsigned int target_flit=0;
  long double rec_time=0;
  long double sync_time=0;
  flit_last=TrPath_rx[stage]->vec_rec_inf.size()-1;
  flit_check = flit_last-1;
  flit_check_Write_into_FIFO = TrPath_rx[stage]->vec_rec_inf[flit_last].rec_time - TrPath_rx[stage]->init_clk/2;
//  cout<<endl;
//  cout<<"flit_check="<<flit_check<<" flit_check_Write_into_FIFO="<<flit_check_Write_into_FIFO<<endl;
//  cout<<"fix_stage="<<stage+1<<" flit_check="<<TrPath_rx[stage+1]->vec_rec_inf[flit_check].rec_time<<endl;
//  cout<<"start CheckNextStage"<<endl;

  if(flit_check-FIFO_size-TrPath_rx[stage]->inputPipe <= 0)
    return;//header no need to fix

//  if(flit_check_Write_into_FIFO >= TrPath_rx[stage+1]->vec_rec_inf[flit_check].rec_time)//error
//  {
//    for(unsigned int fix_stage=stage+1;fix_stage<TrPath_rx.size();fix_stage++)
//      TrPath_rx[fix_stage]->vec_rec_inf.pop_back();//pop flit_check
   
    //recalculation flit_check from fix_stage to Dst_PE
    for(unsigned int fix_stage=stage+1; fix_stage<TrPath_rx.size();fix_stage++)
    {
      flit_last=TrPath_rx[fix_stage-1]->vec_rec_inf.size()-1;//compare to pre-stage
      flit_check=flit_last-1;
      flit_check_Write_into_FIFO = TrPath_rx[fix_stage-1]->vec_rec_inf[flit_last].rec_time - TrPath_rx[fix_stage]->init_clk/2;
      if(flit_check_Write_into_FIFO < TrPath_rx[fix_stage]->vec_rec_inf[flit_check].rec_time)
      {
        break;//no need to fix next stage, break for loop
      }
      else//flit_check_Write_into_FIFO >= TrPath_rx[stage+1]->vec_rec_inf[flit_check].rec_time
      {
        TrPath_rx[fix_stage]->vec_rec_inf.pop_back();//pop flit_check
      }
      //recaulate flit_check
      target_flit =  TrPath_rx[fix_stage]->vec_rec_inf.size();
      rec_time = TrPath_rx[fix_stage]->vec_rec_inf.back().rec_time + TrPath_rx[fix_stage]->init_clk;

      if(fix_stage==stage+1)
        Data_Write_into_FIFO=flit_check_Write_into_FIFO;
      else
        Data_Write_into_FIFO= TrPath_rx[fix_stage-1]->vec_rec_inf[target_flit].rec_time + (TrPath_rx[fix_stage-1]->init_clk / 2);
      
      if(fix_stage==TrPath_rx.size()-1)//Dst_PE
      {
        do{
         if(rec_time > Data_Write_into_FIFO)
         {
           rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[fix_stage]->vec_rec_inf.back().rec_time,false,packet_ID);
           TrPath_rx[fix_stage]->vec_rec_inf.push_back(rec_inf_tmp);
         }
         else
           rec_time += TrPath_rx[fix_stage]->init_clk;
        }while(TrPath_rx[fix_stage]->vec_rec_inf.size() == target_flit);//send flit successfully
      }
      else//SW
      {
        //Firstly,consider if pre-stage's target_flit write into FIFO
        while(rec_time <= Data_Write_into_FIFO)
          rec_time += TrPath_rx[fix_stage]->init_clk;
    
        //Secondly,consider FIFO has space, compare to next stage's rx port
        sync_time =((int)(TrPath_rx[fix_stage+1]->vec_rec_inf[target_flit-(FIFO_size+TrPath_rx[fix_stage+1]->inputPipe+1)].rec_time/TrPath_rx[fix_stage]->init_clk)+1)*TrPath_rx[fix_stage]->init_clk; 
        if((sync_time-TrPath_rx[fix_stage+1]->vec_rec_inf[target_flit-(FIFO_size+TrPath_rx[fix_stage+1]->inputPipe+1)].rec_time) <= (TrPath_rx[fix_stage]->init_clk/2))
          Data_Read_from_FIFO = sync_time + TrPath_rx[stage]->init_clk/2;
        else
          Data_Read_from_FIFO = TrPath_rx[stage+1]->vec_rec_inf[target_flit-(FIFO_size+TrPath_rx[fix_stage+1]->inputPipe+1)].rec_time;

        do{
         if(rec_time > Data_Read_from_FIFO)
         {
           rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[fix_stage]->vec_rec_inf.back().rec_time,false,packet_ID);
           TrPath_rx[fix_stage]->vec_rec_inf.push_back(rec_inf_tmp);
         }
         else
          rec_time += TrPath_rx[fix_stage]->init_clk;
        }while(TrPath_rx[fix_stage]->vec_rec_inf.size() == target_flit);//send flit successfully
      }
    }
//    cout<<"end CheckNextStage"<<endl;
//  }
//  else
//    return;

  return;
}


//When packet's path with congestion the tail arrival
//calculate from sloewst component
void Packet::SimulateTailTimetoRxWithCongestion()
{
  long double slowest_clk=0;
  unsigned int slowest_component=0;
  slowest_clk = SlowestClkinPath();
  long double rec_time=0;
  rec_inf rec_inf_tmp(0,0);
  unsigned int cong_step=0;
  int cong_flits=0;
  //long double diff_time=0;
  
  //debug for congestion port number and position
/*
  for(unsigned int i=0;i<vec_cong_port.size();i++)
  {
    cout<<"Congestion port at SW "<<vec_cong_port[i].cong_port->sw_id;
    cout<<" dir["<<vec_cong_port[i].cong_port->port_dir<<"]";
    cout<<" at step["<<vec_cong_port[i].cong_step<<"] in path."<<endl;   
  }
*/
  //find slowest component
  for(unsigned int i=0;i<TrPath_rx.size();i++)
  {  
    if(TrPath_rx[i]->init_clk == slowest_clk)
    {
      slowest_component = i;
      break;
    }
  }

  //remain congestion port which effect slowest component
  effect_cong.clear();
  for(unsigned int i=0;i<vec_cong_port.size();i++)
  { 
    if(vec_cong_port[i].cong_step >= slowest_component)
      effect_cong.push_back(vec_cong_port[i]);
  }

  //calculate tail time for slowest component
  int flits_buf=0;//functional data FIFO between target and bounded component
  long double bound_clk=0;
  long double release_time=0;
  //long double release_tmp=0;
  
  bound_clk = TrPath_rx[slowest_component]->init_clk;
  rec_time = TrPath_rx[slowest_component]->header_time;
  rec_inf_tmp.set(rec_time,bound_clk,false,packet_ID);
  TrPath_rx[slowest_component]->vec_rec_inf.push_back(rec_inf_tmp);//header for slowest component
   
  //cout<<"Slowest component is step "<<slowest_component<<endl;
  if(effect_cong.empty())//congestion doesn't effect slowest component
  { //all flit receive by slowest clk
    while(TrPath_rx[slowest_component]->vec_rec_inf.size() != total_flits)
    {
      rec_time += slowest_clk;
      rec_inf_tmp.set(rec_time,slowest_clk,false,packet_ID);
      TrPath_rx[slowest_component]->vec_rec_inf.push_back(rec_inf_tmp);
    }
  }
  else//congestion effect slowest component
  {
    flits_buf =0;
    while(TrPath_rx[slowest_component]->vec_rec_inf.size() != total_flits)
    { 
      //data in buffer 
      if(!effect_cong.empty())
        flits_buf = (effect_cong.begin()->cong_step - slowest_component)*(FIFO_size+TrPath_rx[slowest_component]->inputPipe + 1);

      while((int)TrPath_rx[slowest_component]->vec_rec_inf.size() < flits_buf)
      {
        rec_time += slowest_clk;
        //rec_inf_tmp.set(rec_time,slowest_clk);
        rec_inf_tmp.set(rec_time, rec_time-TrPath_rx[slowest_component]->vec_rec_inf.back().rec_time,false,packet_ID);
        TrPath_rx[slowest_component]->vec_rec_inf.push_back(rec_inf_tmp);
        
        if(TrPath_rx[slowest_component]->vec_rec_inf.size()==total_flits)//buffer tolerant congestion
          break;//finish calculation

      }
      //cout<<"slowest component has "<<TrPath_rx[slowest_component]->vec_rec_inf.size()<<" flits"<<endl;
      
      //calculate congestion end time(consider congsetion propagation) 
      if(!effect_cong.empty())
      {
        cong_step = effect_cong.begin()->cong_step;
        release_time = TrPath_rx[cong_step]->header_time;
        //cout<<"slowest component congestion port release time="<<release_time<<endl;
/*
        for(int step=(int)effect_cong.begin()->cong_step-1; step >= (int)slowest_component;step--)
        { 
          release_tmp = release_time;
          release_time = ((int)(release_time/TrPath_rx[step]->init_clk)+1)*TrPath_rx[step]->init_clk;
          
          if(release_time-release_tmp <= (TrPath_rx[step]->init_clk/2))
            release_time += TrPath_rx[step]->init_clk;
          
          //cout<<"step="<<step<<endl;
          //cout<<"release_tmp="<<release_tmp<<endl;
          //cout<<"release_time="<<release_time<<endl;
        }
*/
      }
      else
        release_time=0;

      //cout<<"slowest component release congestion at "<<release_time<<endl;

      if(TrPath_rx[slowest_component]->vec_rec_inf.size()==total_flits)//buffer tolerant congestion
      {
        if(slowest_component==0)//special case for slowest is Src PE and all FIFO full
        {//Src PE can send data without Rxready signal is high
          if((int)TrPath_rx[slowest_component]->vec_rec_inf.size() == flits_buf && !effect_cong.empty())
          {//repair last flit receive information
            if(release_time >= TrPath_rx[slowest_component]->vec_rec_inf[total_flits-1].rec_time)
            {
              TrPath_rx[slowest_component]->vec_rec_inf.pop_back();
              rec_time = release_time;
              rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[slowest_component]->vec_rec_inf.back().rec_time,true,packet_ID);
              TrPath_rx[slowest_component]->vec_rec_inf.push_back(rec_inf_tmp);
            }
          }
        }

        break;//finish calculation
      }


      //cout<<"rec_time="<<rec_time<<endl;
      //cout<<"target(slowest) port release time= "<<release_time<<endl;
      //cout<<"effect_cong.empty()="<<effect_cong.empty()<<endl;
      //congestion effect
      if(rec_time < release_time && (!effect_cong.empty()))
      {
        if(rec_time == TrPath_rx[slowest_component]->header_time)
          rec_time += slowest_clk;//slowest component send header
        else
          rec_time = release_time;

        rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[slowest_component]->vec_rec_inf.back().rec_time,true,packet_ID);
        TrPath_rx[slowest_component]->vec_rec_inf.push_back(rec_inf_tmp);
        cong_flits = int(effect_cong.begin()->cong_flits);
        //consider when next flit write in FIFO, other congestion traffic still in FIFO and bound target traffic
        while(cong_flits > 0 && TrPath_rx[slowest_component]->vec_rec_inf.size() < total_flits)
        {
          if(slowest_clk > effect_cong.begin()->cong_clk)
            rec_time += slowest_clk;//next flit receuve time
          else
            rec_time += effect_cong.begin()->cong_clk;

          cong_flits--;
          rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[slowest_component]->vec_rec_inf.back().rec_time,false,packet_ID);
          TrPath_rx[slowest_component]->vec_rec_inf.push_back(rec_inf_tmp);
        }
        effect_cong.erase(effect_cong.begin());//delet this congstion effect
        continue;//consider next congestion step
      }
      else if(rec_time >= release_time && (!effect_cong.empty()))
        effect_cong.erase(effect_cong.begin());//delet this congstion effect

      if(effect_cong.empty())//no congestion,all flit receive by slowest clk
      {
        rec_time = TrPath_rx[slowest_component]->vec_rec_inf.back().rec_time;
        while(TrPath_rx[slowest_component]->vec_rec_inf.size() != total_flits)
        {
         rec_time += slowest_clk;
         //rec_inf_tmp.set(rec_time,slowest_clk);
         rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[slowest_component]->vec_rec_inf.back().rec_time,false,packet_ID);
         TrPath_rx[slowest_component]->vec_rec_inf.push_back(rec_inf_tmp);
        }
      }
      else
        continue;//continue while loop
    }//end while loop
    
  }
  TrPath_rx[slowest_component]->tail_time = TrPath_rx[slowest_component]->vec_rec_inf.back().rec_time;
  //end calculate slowest component tail time
  
  int target_flit=0;
  int compare_flit=0;
  //foward calculate from slowest component(component behind slowest component)
  //
  //cout<<"start forward calculattion"<<endl;
  for(int target_port=(slowest_component-1);target_port>=0;target_port--)
  {
    bound_clk = TrPath_rx[target_port]->init_clk;
    rec_time = TrPath_rx[target_port]->header_time;
    rec_inf_tmp.set(rec_time,bound_clk,false,packet_ID);
    TrPath_rx[target_port]->vec_rec_inf.push_back(rec_inf_tmp);
    //remaining_flits = total_flits - 1;//header is receive by rx
    
    flits_buf = FIFO_size + TrPath_rx[target_port]->inputPipe + 1;//syncFIFO + rx_buffer,between two rx port.
   
    //remain congestion port which effect target SW
    effect_cong.clear();
    for(unsigned int i=0;i<vec_cong_port.size();i++)
     if(int(vec_cong_port[i].cong_step) >= target_port)
       effect_cong.push_back(vec_cong_port[i]);

    //consider when next flit write in FIFO, other congestion traffic still in FIFO and bound target traffic
    if(!effect_cong.empty())
    {
      cong_step = effect_cong.begin()->cong_step;
      if(int(cong_step) == target_port)
      {
        cong_flits = effect_cong.begin()->cong_flits;
        while(cong_flits > 0 && TrPath_rx[target_port]->vec_rec_inf.size() < total_flits)
        {
          if(TrPath_rx[target_port]->init_clk > effect_cong.begin()->cong_clk)
            rec_time += TrPath_rx[target_port]->init_clk;//next flit receive time
          else
            rec_time += effect_cong.begin()->cong_clk;
   
          cong_flits--;
          rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[target_port]->vec_rec_inf.back().rec_time,false,packet_ID);
          TrPath_rx[target_port]->vec_rec_inf.push_back(rec_inf_tmp);
        }
        effect_cong.erase(effect_cong.begin());//delet this congstion effect
      }
    }
    //calculate each flit receive time by flit buffer size and "the port after target port"
    while(TrPath_rx[target_port]->vec_rec_inf.size() != total_flits)
    { 
      //the flit we want ti know the receive time
      target_flit = TrPath_rx[target_port]->vec_rec_inf.size()+1;
      //the flit is in the port after target port,compare_flit may bound target flit
      compare_flit = target_flit - flits_buf;//consider flit buffer effect
      
      if(compare_flit <= 0)//data put in FIFO
      {
        rec_time += bound_clk;
        //rec_inf_tmp.set(rec_time,bound_clk);
        rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[target_port]->vec_rec_inf.back().rec_time,false,packet_ID);
        TrPath_rx[target_port]->vec_rec_inf.push_back(rec_inf_tmp);
      }
      else
      {
        if((rec_time - TrPath_rx[target_port]->vec_rec_inf[target_flit-1-1].rec_time) >= bound_clk )
          rec_time += TrPath_rx[target_port]->init_clk;
        else
          rec_time += bound_clk;
        
        if(rec_time > TrPath_rx[target_port+1]->vec_rec_inf[compare_flit-1].rec_time)
        {
          if((rec_time - TrPath_rx[target_port+1]->vec_rec_inf[compare_flit-1].rec_time) <= (TrPath_rx[target_port]->init_clk/2))
            rec_time += TrPath_rx[target_port]->init_clk;//special case:FIFO full and next rx receive after half of target rx clk

          //rec_inf_tmp.set(rec_time,(rec_time - TrPath_rx[target_port]->vec_rec_inf[target_flit-1-1].rec_time));
          rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[target_port]->vec_rec_inf.back().rec_time,false,packet_ID);
          TrPath_rx[target_port]->vec_rec_inf.push_back(rec_inf_tmp);
          
          if(rec_inf_tmp.bound_clk >= slowest_clk)//after target flit all flits bound by slowest clk
           bound_clk = slowest_clk; 
        }
      }
    }//end while loop
    //cout<<"tail time="<<TrPath_rx[target_port]->vec_rec_inf[target_flit-1].rec_time<<endl;
    TrPath_rx[target_port]->tail_time = TrPath_rx[target_port]->vec_rec_inf[target_flit-1].rec_time;
  }

  //backward calculate from slowest component(component after slowest component)
  //
  int last_cong_flit =0;//record last congestion flit position in current all flits
  bool target_port_cong = false;//record the status target port has header congestion
  unsigned int last_flit_num=0;
  //cout<<"start backward calculation"<<endl;
  long double   D_W_in_FIFO_time;//the flit send to FIFO actually write in FIFO time
  for(unsigned int target_port=(slowest_component+1); target_port<= (TrPath_rx.size()-1); target_port++)
  {

    target_port_cong = false;
    last_flit_num =0;
    //remain congestion port which effect target SW
    effect_cong.clear();
    for(unsigned int i=0;i<vec_cong_port.size();i++)
     if(vec_cong_port[i].cong_step >= target_port)
       effect_cong.push_back(vec_cong_port[i]);

     bound_clk = TrPath_rx[target_port]->init_clk;
     rec_time = TrPath_rx[target_port]->header_time;
     rec_inf_tmp.set(rec_time,bound_clk,false,packet_ID);
     TrPath_rx[target_port]->vec_rec_inf.push_back(rec_inf_tmp);

    if(effect_cong.empty())
    {
      while(TrPath_rx[target_port]->vec_rec_inf.size() != total_flits)
      {
        target_flit = TrPath_rx[target_port]->vec_rec_inf.size()+1;
        D_W_in_FIFO_time = TrPath_rx[target_port-1]->vec_rec_inf[target_flit-1].rec_time;
        D_W_in_FIFO_time += (TrPath_rx[target_port-1]->init_clk / 2);
 
        rec_time += TrPath_rx[target_port]->init_clk;
       
        if(rec_time > D_W_in_FIFO_time)
        {
          //rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[target_port]->vec_rec_inf.back().rec_time);
          rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[target_port]->vec_rec_inf.back().rec_time,false,packet_ID);
          TrPath_rx[target_port]->vec_rec_inf.push_back(rec_inf_tmp);
        }
        else
          continue;//continue while loop
      }
    }
    else//have congestion effect
    {
 
      //consider when header write in FIFO, other congestion traffic still in FIFO and bound target traffic
      cong_step = effect_cong.begin()->cong_step;
      if(cong_step == target_port)
      {
        target_port_cong = true;
        cong_flits = effect_cong.begin()->cong_flits;
        while(cong_flits > 0 && TrPath_rx[target_port]->vec_rec_inf.size() < total_flits)
        {
          if(TrPath_rx[target_port]->init_clk > effect_cong.begin()->cong_clk)
            rec_time += TrPath_rx[target_port]->init_clk;//next flit receive time
          else
            rec_time += effect_cong.begin()->cong_clk;
  
          cong_flits--;
          rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[target_port]->vec_rec_inf.back().rec_time,false,packet_ID);
          TrPath_rx[target_port]->vec_rec_inf.push_back(rec_inf_tmp);
        }
        effect_cong.erase(effect_cong.begin());//delet this congstion effect
      }
 
 //     for(unsigned int i=0;i<TrPath_rx[target_port]->vec_rec_inf.size();i++)
 //       if(TrPath_rx[target_port]->vec_rec_inf[i].rec_time  < release_time)
 //         flits_buf--;//there are already flits in FIFO
 
      //cout<<"step="<<target_port<<" cong at "<<effect_cong.begin()->cong_step<<" buffer="<<flits_buf<<endl;
      while(TrPath_rx[target_port]->vec_rec_inf.size() != total_flits)
      {
        if(!effect_cong.empty())
        {
          flits_buf = (effect_cong.begin()->cong_step - target_port)*(FIFO_size + TrPath_rx[target_port]->inputPipe + 1);
          cong_step = effect_cong.begin()->cong_step;
          release_time = TrPath_rx[cong_step]->header_time;
      
          //consider congestion propagate
          //cout<<"congestion port release time= "<<release_time<<endl;
/*
          for(int step=(int)effect_cong.begin()->cong_step-1; step >= (int)target_port;step--)
          { 
            release_tmp = release_time;
            release_time = ((int)(release_time/TrPath_rx[step]->init_clk)+1)*TrPath_rx[step]->init_clk;
         
            if(release_time-release_tmp <= (TrPath_rx[step]->init_clk/2))
              release_time += TrPath_rx[step]->init_clk;
        
            //cout<<"step="<<step<<" target release time="<<release_time<<endl;
          }
*/
        }
        for(unsigned int i=0;i<TrPath_rx[target_port]->vec_rec_inf.size();i++)
          if(TrPath_rx[target_port]->vec_rec_inf[i].rec_time  < release_time)
            flits_buf--;//there are already flits in FIFO
 
        target_flit = TrPath_rx[target_port]->vec_rec_inf.size()+1;
        
        //Calculate target flit write in FIFO time
        if(TrPath_rx[target_port-1]->vec_rec_inf[target_flit].congestion == true && target_flit < int(total_flits-1))
        {
          //cout<<"target_flit="<<target_flit<<endl;
          //cout<<"step="<<target_port-1<<endl;
          D_W_in_FIFO_time = TrPath_rx[target_port-1]->vec_rec_inf[target_flit].rec_time;
          //cout<<"D_W_in_FIFO_time="<<D_W_in_FIFO_time<<endl;
          D_W_in_FIFO_time -= (TrPath_rx[target_port-1]->init_clk/2);
          //cout<<"D_W_in_FIFO_time="<<D_W_in_FIFO_time<<endl;
        }
        else if(target_flit==int(total_flits) && TrPath_rx[target_port-1]->vec_rec_inf[target_flit-1].congestion == true)
        {
          //cout<<"target_flit="<<target_flit<<endl;
          D_W_in_FIFO_time = TrPath_rx[target_port-1]->vec_rec_inf[target_flit-1].rec_time;
          D_W_in_FIFO_time -= (TrPath_rx[target_port-1]->init_clk/2);
          //cout<<"special case Src PE tail in FIFO at "<<D_W_in_FIFO_time<<endl;
        }
        else
        {
          D_W_in_FIFO_time = TrPath_rx[target_port-1]->vec_rec_inf[target_flit-1].rec_time;
          D_W_in_FIFO_time += (TrPath_rx[target_port-1]->init_clk / 2);
        }
       

        //////////////////////////////////////////////////////////////////////////////////
        //add time scale
        if(TrPath_rx[target_port]->vec_rec_inf.back().bound_clk == slowest_clk)
          rec_time += slowest_clk;
        else if(target_port_cong==true)
        {
          if(TrPath_rx[target_port]->init_clk < TrPath_rx[target_port+1]->init_clk)
          {
            if(last_flit_num == TrPath_rx[target_port]->vec_rec_inf.size())
              rec_time += TrPath_rx[target_port]->init_clk;//last time get no flit, check after init_clk next time
            else
              rec_time += TrPath_rx[target_port+1]->init_clk;
          }
          else
            rec_time += TrPath_rx[target_port]->init_clk;
        }
        else if((last_cong_flit) !=0 && (last_cong_flit < target_flit))
          rec_time += bound_clk;//slowest clk between target_port and last congestion port
        else
          rec_time += TrPath_rx[target_port]->init_clk;
        
        last_flit_num = TrPath_rx[target_port]->vec_rec_inf.size();


        if((flits_buf > 0) && (rec_time > D_W_in_FIFO_time) && (rec_time < release_time) && (!effect_cong.empty()))//data in buffer
        {
          rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[target_port]->vec_rec_inf.back().rec_time,false,packet_ID);
          TrPath_rx[target_port]->vec_rec_inf.push_back(rec_inf_tmp);
          flits_buf--;
          continue;//continue while loop
        }
        else if((flits_buf == 0) && (rec_time > D_W_in_FIFO_time) && (rec_time < release_time) && (!effect_cong.empty()))//congestion effect
        {
          rec_time = release_time;
          //rec_inf_tmp.set(rec_time,(rec_time - TrPath_rx[target_port]->vec_rec_inf[target_flit-1-1].rec_time));
          rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[target_port]->vec_rec_inf.back().rec_time,true,packet_ID);
          TrPath_rx[target_port]->vec_rec_inf.push_back(rec_inf_tmp);
          last_cong_flit = TrPath_rx[target_port]->vec_rec_inf.size();
          cong_flits = effect_cong.begin()->cong_flits;
          //cout<<"flit="<<TrPath_rx[target_port]->vec_rec_inf.size()-1<<" cong_flits="<<cong_flits<<endl;
          while(cong_flits > 0 && TrPath_rx[target_port]->vec_rec_inf.size() < total_flits)
          {
            if(TrPath_rx[target_port]->init_clk > effect_cong.begin()->cong_clk)
              rec_time += TrPath_rx[target_port]->init_clk;//next flit receive time
            else
              rec_time += effect_cong.begin()->cong_clk;
    
            cong_flits--;
            rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[target_port]->vec_rec_inf.back().rec_time,false,packet_ID);
            TrPath_rx[target_port]->vec_rec_inf.push_back(rec_inf_tmp);
          }
          bound_clk = FindSlowestCLK(target_port,effect_cong.begin()->cong_step);
          effect_cong.erase(effect_cong.begin());//delet this congstion effect 
      
          continue;//continue wfile loop
        }
        else if(rec_time > D_W_in_FIFO_time)//no congestion effect
        {
          rec_inf_tmp.set(rec_time,rec_time-TrPath_rx[target_port]->vec_rec_inf.back().rec_time,false,packet_ID);
          TrPath_rx[target_port]->vec_rec_inf.push_back(rec_inf_tmp);
 
          continue;
        } 
        else
          continue;
      }//end while loop
 
    }
      
   TrPath_rx[target_port]->tail_time = TrPath_rx[target_port]->vec_rec_inf[target_flit-1].rec_time;
  }
  //debug
  
  for(unsigned int i=0;i<TrPath_rx.size();i++)
    for(unsigned int j=0;j<TrPath_rx[i]->vec_rec_inf.size();j++)
    {
      //cout<<"port="<<i<<" flit="<<j<<" receive time="<<TrPath_rx[i]->vec_rec_inf[j].rec_time;
      //cout<<" diff time="<<TrPath_rx[i]->vec_rec_inf[j].bound_clk<<endl;
    }

}

//find the slowest clk before current step path
//assume path:0->1->2->3,current step=2,then 
//pe0,sw0,sw1,sw2 clk have slowest clk
//In practical,sw03 and pe03 may effect too.
long double Packet::ForwardFindSlowestClk(unsigned int current_step)
{
  long double slowest_clk;
  slowest_clk = TrPath_rx[0]->functional_clk;

  for(unsigned int i=0;i<=current_step;i++)
  {
    if(slowest_clk < TrPath_rx[i]->functional_clk)
     slowest_clk = TrPath_rx[i]->functional_clk;
  }

  return slowest_clk;
}

//if packet has all tx port on it's path, set transfer_enable true
void Packet::CheckWormHolePath()
{
 #ifdef D_DEBUG1
 cout<<"---\n";
 ShowSWPath();
 printf("enable:%d size:%d\n", transfer_enable, TrPath_rx.size());
 #endif
 for(unsigned int step=0;step<TrPath_rx.size();step++)
 {
	#ifdef D_DEBUG1
	printf("packet ID %d stage %d (%d-%d,%d-%d) @ (%d,%d,%d)\n", packet_ID, step, ID, packet_ID, TrPath_rx[step]->occupy_p_id, TrPath_rx[step]->occupy_g_id, TrPath_rx[step]->sw_id, TrPath_rx[step]->port_dir, TrPath_rx[step]->type);
	#endif
  if(TrPath_rx[step]->occupy_p_id == ID)
    transfer_enable = true;
  else
  {
    transfer_enable = false;
    break;//break for loop
  }
 }
}

void Packet::UpdateTxRxReleaseTime()//must do after one packet done transfer
{
  long double lastboundclk=0;

  for(unsigned int i=0;i<TrPath_rx.size();i++)
  {
    lastboundclk = TrPath_rx[i]->vec_rec_inf.back().bound_clk;
    if(i==0)//Src_PE tx port
    {
      TrPath_rx[i]->release_time = Src_PE->tx_p->tail_time;
			TrPath_rx[i]->total_release_time.push_back(TrPath_rx[i]->release_time);
			TrPath_rx[i]->total_release_id.push_back(packet_ID);
      TrPath_rx[i]->LastBoundCLK=lastboundclk;
    }
    else if(i==TrPath_rx.size()-1)
    {   
      TrPath_rx[i]->release_time = Dst_PE->rx_p->tail_time;
			TrPath_rx[i]->total_release_time.push_back(TrPath_rx[i]->release_time);
			TrPath_rx[i]->total_release_id.push_back(packet_ID);
      TrPath_rx[i]->LastBoundCLK=lastboundclk;
    }
    else
    {
     TrPath_rx[i]->release_time = TrPath_rx[i]->tail_time;
			TrPath_rx[i]->total_release_time.push_back(TrPath_rx[i]->release_time);
			TrPath_rx[i]->total_release_id.push_back(packet_ID);
     TrPath_rx[i]->LastBoundCLK=lastboundclk;
     TrPath_tx[i]->release_time = TrPath_rx[i]->tail_time;
			TrPath_tx[i]->total_release_time.push_back(TrPath_tx[i]->release_time);
			TrPath_tx[i]->total_release_id.push_back(packet_ID);
     TrPath_tx[i]->LastBoundCLK=lastboundclk;
    }

  }

  finished_time = Dst_PE->rx_p->tail_time;
}

void Packet::ShowTxRxPortPath()
{
 for(unsigned int i=0;i<TrPath_rx.size();i++)
 { 
   if(i==0)
     cout<<"Path: PE"<<Src_PE->ID<<" ";
   else if(i==TrPath_rx.size()-1)
     cout<<"PE"<<Dst_PE->ID<<endl;
   else
   {
     //cout<<"rx_clk="<<TrPath_rx[i]->init_clk<<" ";
     cout<<"SW"<<TrPath_rx[i]->sw_id<<":rx_"<<TrPath_rx[i]->port_dir;
     cout<<"->";
     cout<<"SW"<<TrPath_tx[i]->sw_id<<":tx_"<<TrPath_tx[i]->port_dir<<" ";
   }
 }
}


void Packet::ShowSWPath()
{
  cout<<"Packet "<<packet_ID<<"("<<ID<<") src="<<Src_PE->ID<<" Dst="<<Dst_PE->ID<<" @"<<send_time<<" :";
  cout<<"PE"<<Src_PE->ID<<"->";
  for(unsigned int i=0;i<TrPath_SW.size();i++)
     cout<<"SW"<<TrPath_SW[i]->ID<<"->";
  
  cout<<"PE"<<Dst_PE->ID<<endl;
}


void Packet::ShowRxPortTime()
{
  for(unsigned int i=0;i<TrPath_rx.size();i++)
  {
   if(i==0)
   {
     cout<<"Src_PE tx_header:"<<Src_PE->tx_p->header_time;
     cout<<" Src_PE tx_tail:"<<Src_PE->tx_p->tail_time<<endl;;
   }
   else if(i==TrPath_rx.size()-1)
   {
     cout<<"Dst_PE rx_header:"<<Dst_PE->rx_p->header_time;
     cout<<" Dst_PE rx_tail:"<<Dst_PE->rx_p->tail_time<<endl;
   }
   else
   { cout<<"SW"<<TrPath_rx[i]->sw_id<<" header arrival rx port time="<<TrPath_rx[i]->header_arrival_rx<<endl;
     cout<<"SW"<<TrPath_rx[i]->sw_id<<" Rx_header:"<<TrPath_rx[i]->header_time;
     cout<<" Rx_tail:"<<TrPath_rx[i]->tail_time<<endl;
   }
  }
}


long double Packet::BackwardFindSlowestClk(unsigned int start)
{
  long double slowest_clk;
  slowest_clk = TrPath_rx[start]->functional_clk;

  for(int i=start;i >=0 ;i--)
    if(slowest_clk < TrPath_rx[i]->functional_clk)
      slowest_clk = TrPath_rx[i]->functional_clk;
  
  return slowest_clk;
}

long double Packet::SlowestClkinPath()
{
  long double slowest_clk;
  slowest_clk = Src_PE->clk;

  for(unsigned int i=0;i<TrPath_rx.size();i++)
    if(slowest_clk < TrPath_rx[i]->functional_clk)
      slowest_clk = TrPath_rx[i]->functional_clk;

  return slowest_clk;
}

void Packet::RecordTailTime()
{
  timing_file <<"Src:"<<Src_PE->ID;
  timing_file <<"Dst:"<<Dst_PE->ID<<",";
  for(unsigned int i=0;i<TrPath_rx.size();i++)
    timing_file <<TrPath_rx[i]->tail_time<<",";

  timing_file <<endl;
}

void Packet::InsertRecomputeId(){
	for(unsigned int i=0;i<TrPath_rx.size();i++){
		TrPath_rx[i]->re_id.push_back(packet_ID);
	}
	for(unsigned int i=1;i<TrPath_tx.size()-1;i++){
		TrPath_tx[i]->re_id.push_back(packet_ID);
	}
}

void Packet::DeletePortFlitInf(){
	for(unsigned int i=0;i<TrPath_rx.size();i++){
		for(unsigned int j=0;j<TrPath_rx[i]->flit_id.size();j++){
			if(TrPath_rx[i]->passed_pkt[j] == this){
				TrPath_rx[i]->passed_pkt.erase(TrPath_rx[i]->passed_pkt.begin() + j);
				TrPath_rx[i]->flit_id.erase(TrPath_rx[i]->flit_id.begin() + j);
				break;
			}
		}
		for(unsigned int j=0;j<TrPath_rx[i]->total_last_flits.size();j++){
			if(TrPath_rx[i]->total_last_flits[j].flit_packet_id == packet_ID){
				for(unsigned int k=0;k<FIFO_size+(unsigned int)TrPath_rx[i]->inputPipe+1;k++){
					if(TrPath_rx[i]->total_last_flits[j].flit_packet_id == packet_ID){
						TrPath_rx[i]->total_last_flits.erase(TrPath_rx[i]->total_last_flits.begin() + j);
					}else{
						break;
					}
				}
				break;
			}
		}
	}
	for(unsigned int i=1;i<TrPath_tx.size()-1;i++){
		for(unsigned int j=0;j<TrPath_tx[i]->total_last_flits.size();j++){
			if(TrPath_tx[i]->total_last_flits[j].flit_packet_id == packet_ID){
				for(unsigned int k=0;k<FIFO_size+(unsigned int)TrPath_tx[i]->inputPipe+1;k++){
					if(TrPath_tx[i]->total_last_flits[j].flit_packet_id == packet_ID){
						TrPath_tx[i]->total_last_flits.erase(TrPath_tx[i]->total_last_flits.begin() + j);
					}else{
						break;
					}
				}
				break;
			}
		}
	}
}

void Packet::SetPacket(PE* src,PE* dst,unsigned int size,long double send,unsigned int id,tlm::tlm_generic_payload* ptr){
	trans_ptr = ptr;
	Src_PE = src;
	Dst_PE = dst;
	data_size=size+4+4;//consider headr & necker
	send_time=send;
	packet_ID = id;
	total_flits = data_size/(_data_width/8);
	finished_time=0;
	transfer_enable=false;
	reCompute=true;
	TrPath_SW.clear();
	TrPath_rx.clear();
	TrPath_tx.clear();
	delete [] header_time_list;
	delete [] tail_time_list;
	vec_cong_port.clear();
}
