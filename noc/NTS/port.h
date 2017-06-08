#ifndef PORT_H
#define PORT_H

#include "emu.h"
#include <vector>
#include <limits.h>
#include <iostream>
#include <fstream>
#include "stdio.h"
#include "sys/time.h"
using namespace std;

class rec_inf
{ 
  public:
    long double rec_time;
    long double bound_clk;
    bool congestion;//true for receive this flit when congestion release
		unsigned int flit_packet_id;

  rec_inf(long double t,long double clk)
  {
    rec_time = t;
    bound_clk = clk;//bound by slow clk the other port in TrPath_rx
    congestion = false;
		flit_packet_id = INT_MAX;
  }
  
  void set(long double t,long double clk, bool cong, unsigned int id)
  {
    rec_time = t;
    bound_clk = clk;//bound by slow clk the other port in TrPath_rx
    congestion = cong;
		flit_packet_id = id;
  }
	
  void reset_rec()
  {
    rec_time = 0;
    bound_clk = 0;
    congestion = false;
		flit_packet_id = INT_MAX;
  }
  
	void clone_rec(rec_inf* a){
    rec_time = a->rec_time;
    bound_clk = a->bound_clk;
    congestion = a->congestion;
		flit_packet_id = a->flit_packet_id;
	}
};

class port
{
  public:
    unsigned int sw_id;
    PortType type;
    DIR port_dir;
    DIR output_tx_dir;//for rx port,this is record this rx port's output tx port's direction
    long double functional_clk;//send/receive data clock which depend on other clk on packet's path.
    long double init_clk;
    vector<rec_inf> vec_rec_inf;//record each flit receive time
    vector<rec_inf> total_last_flits;//record each flit receive time
    //To know other packet's flit leave FIFO time
    vector<rec_inf> last_flits;//record last flits receive for other packet's flit
		//vector<flit_inf> flit_list;//record all of the flit inf from each packets that have passed this port
		vector<void*> passed_pkt;
		vector<unsigned int> flit_id;
		vector<unsigned int> re_id;
    long double header_time;//the time header receive by port(not yet correct)
    long double tail_time;//the time tail receive by port
    long double header_arrival_rx;//the time hearder arrival rx port, which is final and correct.(Not the same with rx port receive header time)
    long double release_time;//ns packet release port at this time
		long double min_header_arrival_rx;//the min header_arrival_rx when multiple packet pass this port at the same time
		vector<long double> total_release_time;
		vector<unsigned int> total_release_id;
    unsigned int occupy_p_id;//record which packet occupy this port
		unsigned int occupy_g_id;
    STATE state;
    long double LastBoundCLK;//the clk which transfer of tail
		bool inputPipe;
		bool bwdPipe;
		bool crossbarPipe;
		bool lateArb;
		Arbitration arbiter_port;
		unsigned int arbiter_seq[5];
		unsigned int arbiter_ptr;
		unsigned int next_arbiter_seq[5];
		unsigned int next_arbiter_ptr;

    port(unsigned int id,PortType t,DIR d,long double clk,Arbitration A,unsigned int* s_value)
    {
			inputPipe = s_value[5];
			lateArb = s_value[6];
			crossbarPipe = s_value[7];
			bwdPipe = s_value[8];
			//cout << "I:" << inputPipe << " B:" << bwdPipe << " C:" << crossbarPipe << " A:" << lateArb << endl;

      sw_id=id;
      type=t;
      port_dir=d;
      functional_clk=clk;
      init_clk = clk;
      vec_rec_inf.clear();
      header_time=0;
      tail_time=0;
      header_arrival_rx=0;
      release_time=0;
      occupy_p_id=INT_MAX;
      state=Idle;
      LastBoundCLK=0;
      output_tx_dir=EMPTY;
      passed_pkt.clear();
			flit_id.clear();
			total_release_time.clear();
			total_release_id.clear();
			total_last_flits.clear();
			last_flits.reserve(FIFO_size+inputPipe+1);

      rec_inf tmp(0,0);
      for(unsigned int i =0; i< FIFO_size+(unsigned int)inputPipe+1;i++)
        last_flits.push_back(tmp);

			arbiter_port = A;
			if(type == tx){
				for(unsigned int i=0;i<5;i++){
					if(A == LRU){
						arbiter_seq[i] = i;
					}if(A == FIXED){
						arbiter_seq[i] = s_value[i];
					}
				}
				arbiter_ptr = 0;
			} 
    }
    ~port(){}

		void correct_last_flit(){
      /*header_time=0;
      tail_time=0;
      header_arrival_rx=0;
      state=Idle;*/
			int last_flit_tag = last_flits.size()-1;
			#ifdef D_DEBUG
			for(unsigned int m=0;m<re_id.size();m++){
				printf("re_id:%u %u\n", m, re_id[m]);
			}
			for(unsigned int m=0;m<total_last_flits.size();m++){
				printf("original total:%u (%.2llf,%u)\n", m, total_last_flits[m].rec_time, total_last_flits[m].flit_packet_id);
			}
			#endif

			//delete the recomputed packet's total_last_flits
			for(unsigned int j=0;j<re_id.size();j++){
				for(int i=total_last_flits.size()-1;i>=0;i--){
					if(total_last_flits[i].flit_packet_id == re_id[j]){
						for(unsigned int k=0;k<FIFO_size+(unsigned int)inputPipe+1;k++){
							if(total_last_flits[i-k].flit_packet_id == re_id[j]){
								total_last_flits.erase(total_last_flits.begin() + i - k);
							}else{
								break;
							}
						}
						break;
					}
				}
			}
			//delete the recomputed packet's release_time
			for(unsigned int j=0;j<re_id.size();j++){
				for(int i=total_release_time.size()-1;i>=0;i--){
					if(total_release_id[i] == re_id[j]){
						total_release_time.erase(total_release_time.begin() + i);
						total_release_id.erase(total_release_id.begin() + i);
						break;
					}
				}
			}
			if(!total_release_time.empty())
				release_time = total_release_time.back();
			else
				release_time = 0;

			//correct the last_filts
			if(!total_last_flits.empty()){
				#ifdef D_DEBUG
				for(unsigned int m=0;m<last_flits.size();m++){
					printf("last_flits:%u (%.2llf,%u)\n", m, last_flits[m].rec_time, last_flits[m].flit_packet_id);
				}
				for(unsigned int m=0;m<total_last_flits.size();m++){
					printf("total:%u (%.2llf,%u)\n", m, total_last_flits[m].rec_time, total_last_flits[m].flit_packet_id);
				}
				#endif
				//fill in the last_flits with total_vec_inf
				for(int i=total_last_flits.size()-1;i>=0;i--){
					//printf("Replace with (%.2llf,%u)\n", total_last_flits[i].rec_time, total_last_flits[i].flit_packet_id);
					//last_flits[last_flit_tag].set(total_last_flits[i].rec_time, total_last_flits[i].bound_clk, total_last_flits[i].congestion, total_last_flits[i].flit_packet_id);
					last_flits[last_flit_tag].clone_rec(&total_last_flits[i]);
					//printf("Fill in the last_flit(%.2llf,%u)\n", last_flits[last_flit_tag].rec_time, last_flits[last_flit_tag].flit_packet_id);
					last_flit_tag--;
					if(last_flit_tag<0)
						break;
				}
				//if not filled yet, fill with empty
				for(;last_flit_tag>=0;last_flit_tag--){
					last_flits[last_flit_tag].reset_rec();
				}
			}else if(last_flits.back().flit_packet_id != INT_MAX){
				//if the last_flits is not empty, but the total_vec_inf is empty -> means that all last_flits is filled with recomputed flits -> clear them all
				for(;last_flit_tag>=0;last_flit_tag--){
					last_flits[last_flit_tag].reset_rec();
				}
			}
			#ifdef D_DEBUG
			cout << sw_id << " - " << port_dir << endl;
			for(unsigned int m=0;m<last_flits.size();m++){
				printf("final last_flits in empty:%u (%.2llf,%u)\n", m, last_flits[m].rec_time, last_flits[m].flit_packet_id);
			}
			#endif
		}
    
    void release_port()
    {
      functional_clk = init_clk;
      //last_flits.clear();
      //copy flit inf from vec_rec_inf, the number of flits have to record is the as FIFO size
      //last_flits[0] last_flits[1] last_flits[2]=> flit[n-2] flit[n-1] flit[n]tail
			#ifdef D_DEBUG
			printf("vec_rec_inf size=%d ", vec_rec_inf.size());
			cout << sw_id << " - " << port_dir << endl;
			#endif
      if(!vec_rec_inf.empty())
      {
        //last_flits.clear();
				#ifdef D_DEBUG1
				for(unsigned int i=0;i<vec_rec_inf.size();i++){
					printf("vec_rec_inf[%d]:%d release:%llf ", i, vec_rec_inf[i].flit_packet_id, vec_rec_inf[i].rec_time);
					cout << sw_id << " - " << port_dir << endl;
					//total_last_flits.push_back(vec_rec_inf[i]);
				}
				#endif
        int tag=0;
				unsigned int last_flit_tag=0;
        if(vec_rec_inf.size() > (FIFO_size+(unsigned int)inputPipe+1))
        {  
         	tag=(int)(vec_rec_inf.size()-(FIFO_size+inputPipe+1));

         	for(;tag < (int)vec_rec_inf.size();tag++,last_flit_tag++){
						//todo:change the last_flits to pointer to the total_last_flits since the total_last_flits won't delete
           	//last_flits.push_back(vec_rec_inf[tag]);
						//last_flits[last_flit_tag].set(vec_rec_inf[tag].rec_time, vec_rec_inf[tag].bound_clk, vec_rec_inf[tag].congestion, vec_rec_inf[tag].flit_packet_id);
						last_flits[last_flit_tag].clone_rec(&vec_rec_inf[tag]);
						total_last_flits.push_back(vec_rec_inf[tag]);
         	}
        }else//vec_rec_inf.size() <= FIFO_size+inputPipe
        {
          tag=(int)((FIFO_size+inputPipe+1)-vec_rec_inf.size());
          
          while(tag>0)
          {
						last_flits[last_flit_tag].reset_rec();
            //last_flits.push_back(tmp);
            tag--;
						last_flit_tag++;
          }
          for(;tag < (int)vec_rec_inf.size();tag++,last_flit_tag++){
            //last_flits.push_back(vec_rec_inf[tag]);
						//last_flits[last_flit_tag].set(vec_rec_inf[tag].rec_time, vec_rec_inf[tag].bound_clk, vec_rec_inf[tag].congestion, vec_rec_inf[tag].flit_packet_id);
						last_flits[last_flit_tag].clone_rec(&vec_rec_inf[tag]);
						total_last_flits.push_back(vec_rec_inf[tag]);
					}
        }
     }
			/*#ifdef D_DEBUG
			for(unsigned int i=0;i<last_flits.size();i++){
				if(last_flits[i].rec_time != 0)
					cout<<"clear:last_flits "<<last_flits[i].rec_time<<","<<last_flits.size()<<","<<last_flits[i].flit_packet_id<<endl;
			}
			#endif*/
      vec_rec_inf.clear();
			re_id.clear();
      header_time=0;
      tail_time=0;
      header_arrival_rx=0;
      min_header_arrival_rx=0;
      occupy_p_id=INT_MAX;
      state=Idle;
      output_tx_dir=EMPTY;
    }
    
};


class congestion_inf
{
  public:
    port* cong_port;
    unsigned int cong_step;
    //after tail write in FIFO there are number of flits in FIFO which still congest other traffic
    //Use the difference between congestion "tx port" and "next rx port" header arrival time
    //to calculate FIFO status
    unsigned int cong_flits;
    long double cong_clk;

  congestion_inf(port* p,unsigned int step)
  {
    cong_port = p;
    cong_step = step;
    cong_flits=0;
    cong_clk=0;
  }
  void set(port* p,unsigned int step)
  {
    cong_port = p;
    cong_step = step;
  }
};


#endif
