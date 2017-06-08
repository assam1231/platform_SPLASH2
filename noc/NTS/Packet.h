#ifndef PACKET_H
#define PACKET_H
#include <vector>
#include "tlm.h"
#include "Switch.h"
#include "port.h"
#include "PE.h"
#include "BiSyncFIFO.h"
#include "emu.h"
#include "config.h"
#include <fstream>
#include <sstream>
#include <algorithm>

#define NTS_4_external_memory

using namespace std;
class Packet
{
 private:

 public:
	 tlm::tlm_generic_payload* trans_ptr;
   unsigned int ID;//start at number 0, used in static simulation
	 unsigned int packet_ID;//static packet ID, one packet only has one packet_ID
	 bool reCompute;

   PE* Src_PE;
   PE* Dst_PE;
   unsigned int data_size;//bytes
   unsigned int total_flits;//flits = data_size/(_data_width/8) (32(bits)=4(bytes))
   long double send_time;//ns
   bool transfer_enable;
   long double finished_time;//ns

   vector<Switch*> TrPath_SW;
   vector<port*> TrPath_rx;
   vector<port*> TrPath_tx;
	 long double* header_time_list;
	 long double* tail_time_list;

   vector<congestion_inf> vec_cong_port;
   vector<congestion_inf> effect_cong;//congestion port which behind slowest component
   
   ofstream timing_file;
   string file_name;
   stringstream ss1; 
   
   Packet(PE* src,PE* dst,unsigned int size,long double send,unsigned int id,tlm::tlm_generic_payload* ptr)
   {
     /*ss1 << src->ID;
     file_name = "PE"+ ss1.str();
     file_name += "_to_PE";
     ss1.str(" ");//clean stringstream
     ss1 << dst->ID;
     file_name += ss1.str();
     file_name += ".csv";*/
     //timing_file.open(file_name.c_str(),ios::app);
		 trans_ptr = ptr;
     Src_PE=src;
     Dst_PE=dst;
     data_size=size+4+4;//consider necker
     send_time=send;
		 packet_ID = id;
     total_flits = data_size/(_data_width/8);
     finished_time=0;
     transfer_enable=false;
		 reCompute=true;
     TrPath_SW.clear();
     TrPath_rx.clear();
     TrPath_tx.clear();
     vec_cong_port.clear();
   }
   
   ~Packet()
   {
     //timing_file.close();
   }
   
   void ShowSWPath();//for debug
	 void SimulateSimpleHeaderTimetoRx(double);
   long double ForwardFindSlowestClk(unsigned int current_step);//tail arrival time bound by slowest_clk
   void SearchTxRxPortPath();//search tx rx port on tranfer path by TrPath_SW
   void ShowTxRxPortPath();//for debug
   void ShowRxPortTime();
   void SimulateHeaderTimetoRx();//calculate header arrival time to rx port
   void SetPortFunctionalClk();//the port clk maybe bound by others
   unsigned int UnboundData(long double input_time,long double input_clk,long double output_clk,unsigned int fifo_size,unsigned int data_already_in);//let fifo effect match the same clock phase
   void SimulateTailTimetoRxWithoutCongestion();//calculate tail arrival time to rx port without congestion on path
   void SimulateTailTimetoRxWithCongestion();//calculate tail arrival time to rx port with congestion on path
   void CheckWormHolePath();//check if packet has all tx port on it's path
   void UpdateTxRxReleaseTime();//must do after one packet done transfer
   
   long double BackwardFindSlowestClk(unsigned int start);
   long double SlowestClkinPath();
 
   //when header write in FIFO, FIFO may have other congestion traffic's flit
   //consider this flit's congestion effect and record in congestion infformation
   void FIFOStatusAfterCongestion();
   //record tail arrival time at each component
   void RecordTailTime();

   //Find slowest clk between two component
   long double FindSlowestCLK(unsigned int start_step, unsigned int end_step);

   void SimulateFIFOSizeTime();
   void CheckNextStage(unsigned int stage);
   void InsertRecomputeId();
   void DeletePortFlitInf();

	 void SetPacket(PE* src,PE* dst,unsigned int size,long double send,unsigned int id, tlm::tlm_generic_payload* ptr);
};

#endif
