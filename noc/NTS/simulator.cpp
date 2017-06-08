#include "simulator.h"
#include "NocTimingSimDebug.h"
#include "NocPayloadExtension.h"


//Constructor

template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g> 
Simulator<data_width_g, rows_g, cols_g>::
Simulator(sc_core::sc_module_name name)
	: sc_core::sc_module(name)
{
	for(unsigned int i=0;i<rows_g * cols_g;i++){
		req_init[i] = new tlm_utils::simple_initiator_socket_tagged<Simulator, data_width_g>;
		rsp_init[i] = new tlm_utils::simple_initiator_socket_tagged<Simulator, data_width_g>;
		req_target[i] = new tlm_utils::simple_target_socket_tagged<Simulator, data_width_g>;
		rsp_target[i] = new tlm_utils::simple_target_socket_tagged<Simulator, data_width_g>;

		// Register callback functions to sockets.
		req_init[i]->register_nb_transport_bw(this, &Simulator::nb_transport_bw, 0);
		rsp_init[i]->register_nb_transport_bw(this, &Simulator::nb_transport_bw, 1);
		req_target[i]->register_nb_transport_fw(this, &Simulator::nb_transport_fw, 0);
		rsp_target[i]->register_nb_transport_fw(this, &Simulator::nb_transport_fw, 1);

	}
	
#ifdef NTS_4_external_memory
	// 4 channel for ext memory by along
	req_init[16] = new tlm_utils::simple_initiator_socket_tagged<Simulator, data_width_g>;
	rsp_target[16] = new tlm_utils::simple_target_socket_tagged<Simulator, data_width_g>;
	req_init[16]->register_nb_transport_bw(this, &Simulator::nb_transport_bw, 0);
	rsp_target[16]->register_nb_transport_fw(this, &Simulator::nb_transport_fw, 1);
	
	req_init[17] = new tlm_utils::simple_initiator_socket_tagged<Simulator, data_width_g>;
	rsp_target[17] = new tlm_utils::simple_target_socket_tagged<Simulator, data_width_g>;
	req_init[17]->register_nb_transport_bw(this, &Simulator::nb_transport_bw, 0);
	rsp_target[17]->register_nb_transport_fw(this, &Simulator::nb_transport_fw, 1);
	
	req_init[18] = new tlm_utils::simple_initiator_socket_tagged<Simulator, data_width_g>;
	rsp_target[18] = new tlm_utils::simple_target_socket_tagged<Simulator, data_width_g>;
	req_init[18]->register_nb_transport_bw(this, &Simulator::nb_transport_bw, 0);
	rsp_target[18]->register_nb_transport_fw(this, &Simulator::nb_transport_fw, 1);
	
	req_init[19] = new tlm_utils::simple_initiator_socket_tagged<Simulator, data_width_g>;
	rsp_target[19] = new tlm_utils::simple_target_socket_tagged<Simulator, data_width_g>;
	req_init[19]->register_nb_transport_bw(this, &Simulator::nb_transport_bw, 0);
	rsp_target[19]->register_nb_transport_fw(this, &Simulator::nb_transport_fw, 1);
#else
	req_init[16] = new tlm_utils::simple_initiator_socket_tagged<Simulator, data_width_g>;
	rsp_target[16] = new tlm_utils::simple_target_socket_tagged<Simulator, data_width_g>;
	req_init[16]->register_nb_transport_bw(this, &Simulator::nb_transport_bw, 0);
	rsp_target[16]->register_nb_transport_fw(this, &Simulator::nb_transport_fw, 1);
#endif	

	pkt_peq = new tlm_utils::peq_with_get<tlm::tlm_generic_payload>("pkt_peq");
	complete_peq = new tlm_utils::peq_with_get<tlm::tlm_generic_payload>("complete_peq");
	recompute_peq = new tlm_utils::peq_with_get<tlm::tlm_generic_payload>("recompute_peq");
	sc_spawn(sc_bind(&Simulator::simulator_main, this));
	sc_spawn(sc_bind(&Simulator::simulator_send, this));
}
    
template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g> 
Simulator<data_width_g, rows_g, cols_g>::
~Simulator()
{
	//cout << "finish" << endl;
	#ifdef LOG_SIMULATOR
	sort(PE_Rec.begin(), PE_Rec.end(), cmp_pe_pkt_ptr);
	cout << fixed << setprecision(0);
  for(unsigned int i=0; i<PE_Rec.size(); i++)
  {
		cout <<"s:"<<PE_Rec[i]->Src_PE_ID;
		cout <<"d:"<<PE_Rec[i]->Dst_PE_ID<<",";
		cout <<PE_Rec[i]->send_time<<",";
		cout <<PE_Rec[i]->finished_time<<",";
		cout << endl;
  }
	#endif
};

template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g> 
void
Simulator<data_width_g, rows_g, cols_g>::
simulator_send(){
	bool status;
	tlm::tlm_generic_payload* noc_trans = 0;
	tlm::tlm_generic_payload* recompute_trans = 0;
	vector <tlm::tlm_generic_payload*> recompute_list;
	PktPayload* pkt_payload = 0;
	tlm::tlm_phase phase;
	tlm::tlm_sync_enum retval;
	sc_core::sc_time delay = SC_ZERO_TIME;
	sc_core::sc_time sc_send;
	Packet* pkt;
	while(true){
		wait(complete_peq->get_event());
		recompute_list.clear();
		while((recompute_trans = recompute_peq->get_next_transaction()) != 0){
			recompute_list.push_back(recompute_trans);
		}
		while((noc_trans = complete_peq->get_next_transaction()) != 0){
#ifdef LOG_SIMULATOR
      cout << endl;
			pkt_payload = reinterpret_cast<PktPayload* >( noc_trans->get_data_ptr() );
			cout << "Get trans from complete_peq: \n" 
           << "  source: " << pkt_payload->src << endl
           << "  target: " << pkt_payload->dst << endl
           << "  send time: " << pkt_payload->send_time << endl
           << "  data size: " << pkt_payload->data_size << endl
           << "  requese(0) or response(1)? :" << pkt_payload->req_rsp << endl << endl;
#endif
			status = false;
			for(unsigned int i=0;i<recompute_list.size();i++){
				//cout << sc_time_stamp() << " recompute:" << recompute_list[i] << " " << noc_trans << endl;
				if(recompute_list[i] == noc_trans){
					recompute_list.erase(recompute_list.begin() + i);
					status = true;
					break;
				}
			}
			if(status == true)
				continue;
			pkt_payload = reinterpret_cast<PktPayload* >( noc_trans->get_data_ptr() );
			//printf("simulator_send data_ptr:%X\n", pkt_payload->network_payload);
	#ifdef LOG_SIMULATOR
      cout << endl;
			cout << "Simulator send: @" << sc_time_stamp() << "\n" 
           << "  source: " << pkt_payload->src << endl
           << "  target: " << pkt_payload->dst << endl
           << "  send time: " << pkt_payload->send_time << endl
           << "  data size: " << pkt_payload->data_size << endl
					 << "  finish time: " << pkt_payload->finish_time << endl
           << "  requese(0) or response(1)? :" << pkt_payload->req_rsp << endl << endl;
#endif
			sc_send = sc_core::sc_time(pkt_payload->finish_time, SC_NS);
			if(sc_send < sc_time_stamp())
				cout << "error with simulator_send\n";
			if(sc_send == sc_time_stamp()){
				//delete those packets which are out of noc
				pkt = (Packet*)pkt_payload->pkt_ptr;
#ifdef TRACER
				long double temp_header;
				for(unsigned int i=1;i<pkt->TrPath_rx.size();i++){
					temp_header = pkt->header_time_list[i-1];
					temp_header = Through_BiSyncFIFO(temp_header, pkt->TrPath_rx[i-1]->init_clk, pkt->TrPath_rx[i]->init_clk);
					if(temp_header != pkt->header_time_list[i]){
						out << "src_id dst_id sw_id next_sw_id congested_in congested_out\n";
						cout << pkt->Src_PE->ID << " " << pkt->Dst_PE->ID << " " << pkt->TrPath_rx[i]->sw_id << " " << pkt->TrPath_rx[i+1]->sw_id  << " " << temp_header << " " << pkt->header_time_list[i] << endl;
					}
				}
#endif
				#ifdef LOG_SIMULATOR
				PE_Rec.push_back(new PE_Rec_inf(pkt->Src_PE->ID, pkt->Dst_PE->ID, pkt->send_time, pkt->finished_time));
				#endif
				pkt->DeletePortFlitInf();
				Deleted_Packets.push_back(pkt);
				//printf("simulator_send data_ptr:%X\n", pkt_payload->network_payload);
				noc_trans->set_data_ptr( pkt_payload->network_payload );
				//cout << "set complete!" << endl;
				// Forward the transaction to router's outport (=init socket)	       
				phase = tlm::BEGIN_REQ;
				if(pkt_payload->req_rsp == 0)
					retval = (*req_init[pkt_payload->dst])->nb_transport_fw(*noc_trans, phase, delay);
				else
					retval = (*rsp_init[pkt_payload->dst])->nb_transport_fw(*noc_trans, phase, delay);
				if(retval == tlm::TLM_ACCEPTED || retval == tlm::TLM_UPDATED)
				{
					if(phase == tlm::BEGIN_REQ)
					{
						//cout << "simulator_send: phase == tlm::BEGIN_REQ" << endl;
						//wait(txCompleteEvent_[dir]);		
					}
					else if(phase == tlm::END_REQ)
					{
						std::ostringstream oss;
						oss << "TlmMeshRouter::thread : END_REQ not supported";
						throw std::runtime_error(oss.str().c_str());
					}
					else if(phase == tlm::BEGIN_RESP)
					{
						std::ostringstream oss;
						oss << "TlmMeshRouter::thread : BEGIN_RESP not supported";
						throw std::runtime_error(oss.str().c_str());
					}
					else
					{
						std::ostringstream oss;
						oss << "TlmMeshRouter::thread : invalid PHASE";
						throw std::runtime_error(oss.str().c_str());
					}	       
				}
				else if(retval == tlm::TLM_COMPLETED)
				{
					//cout << "simulator_send: retval == tlm::TLM_COMPLETED" << endl;
					//if(delay != sc_core::SC_ZERO_TIME)
					//{
						//wait(delay);
					//}
				}
				else
				{
					std::ostringstream oss;
					oss << "TlmMeshRouter::thread : invalid SYNC_ENUM";
					throw std::runtime_error(oss.str().c_str());
				}	       
				noc_trans->release();
			}
		}
	}
}

template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g> 
void
Simulator<data_width_g, rows_g, cols_g>::
simulator_main(){
  //[row_dim][col_dim]
  //PE* _PE[rows_g+1][cols_g];
  Switch* _SW[rows_g][cols_g];
  
  // add 4 extmem by Lai along  16 17 18 19
  PE* _PE[rows_g+1][cols_g];
  
 
  ifstream PE_freq_File;
  ifstream SW_freq_File;
  ifstream switch_config_File;
  //PE0 PE1 PE2 .......PEN
  PE_freq_File.open("PE_freq",ifstream::in);
  
  if(!PE_freq_File.is_open())
    cout<<"Open PE_freq File Failure."<<endl;
  
  //SW0 SW1 SW2 .......SWN
  SW_freq_File.open("SW_freq",ifstream::in);
  
  if(!SW_freq_File.is_open())
    cout<<"Open SW_freq File Failure."<<endl;

	switch_config_File.open("switch_config",ifstream::in);

	if(!switch_config_File.is_open())
		cout<<"Open switch_config File Failure."<<endl;

  double freq;
	double fastest_clk=0;
	double slowest_clk=0;
	string tmp_string;
	Arbitration arb_value=FIFO;
	for(unsigned int i=0;i<6;i++){
		switch_config_File >> tmp_string;
		//cout << tmp_string << endl;
		if(i == 0){
			switch_config_File >> tmp_string;
			if(tmp_string == "FIFO")
				arb_value = FIFO; 
			else if(tmp_string == "LRU")
				arb_value = LRU;
			else if(tmp_string == "ROUND_ROBIN")
				arb_value = ROUND_ROBIN;
			else if(tmp_string == "FIXED")
				arb_value = FIXED;
			else if(tmp_string == "RANDOM")
				arb_value = RANDOM;
			else
				cout << "Wrong Arbitration Type\n";
		}else if(i == 1){
			for(unsigned int j=0;j<5;j++)
				switch_config_File >> switch_value[j];
		}else
			switch_config_File >> switch_value[i+3];
	}
	if(switch_value[6] == 1 && switch_value[7] == 1)
		cout << "Error: crossbarPipe and lateArb cannot be true at the same time" << endl;

  for(unsigned int row=0;row<rows_g;row++)
  {
    for(unsigned int col=0;col<cols_g;col++)
    {
      PE_freq_File >> freq;
      _PE[row][col]= new PE((_col*row+col),row,col,freq,switch_value);//ID,x_dim,y_dim,freq,switch_config
			#ifdef LOG_SIMULATOR
      cout<<"Success create PE["<<row<<"]["<<col<<"] in "<<freq<<"MHz"<<endl;
			#endif
      SW_freq_File >> freq;
      _SW[row][col]= new Switch((_col*row+col),row,col,freq,arb_value,switch_value);//ID,x_dim,y_dim,freq,Arbitration,switch_config
			//To find the minimum dynamic window size
			if(fastest_clk==0 || _SW[row][col]->clk < fastest_clk)
				fastest_clk=_SW[row][col]->clk;
			if(slowest_clk==0 || _SW[row][col]->clk > slowest_clk)
				slowest_clk=_SW[row][col]->clk;
			#ifdef LOG_SIMULATOR
      cout<<"Success create SW["<<row<<"]["<<col<<"] in "<<freq<<"MHz"<<" arbitration="<<FIFO<<endl;
			cout<<"Clk:"<<_SW[row][col]->clk<<"ns Slowest_clk:"<<fastest_clk<<"ns"<<endl;
			#endif
    }
  }
	//ext_mem
	//PE_freq_File >> freq;
	//_PE[4][0] = new PE(16,0,0,freq,switch_value);//ID,x_dim,y_dim,freq,switch_config

#ifdef NTS_4_external_memory	
	// add 4 extmem
	PE_freq_File >> freq;
	_PE[4][0] = new PE(16,0,1,freq,switch_value);//ID,x_dim,y_dim,freq,switch_config
	PE_freq_File >> freq;
	_PE[4][1] = new PE(17,1,3,freq,switch_value);//ID,x_dim,y_dim,freq,switch_config
	PE_freq_File >> freq;
	_PE[4][2] = new PE(18,3,2,freq,switch_value);//ID,x_dim,y_dim,freq,switch_config
	PE_freq_File >> freq;
	_PE[4][3] = new PE(19,2,0,freq,switch_value);//ID,x_dim,y_dim,freq,switch_config
#else
	PE_freq_File >> freq;
	_PE[4][0] = new PE(16,0,1,freq,switch_value);//ID,x_dim,y_dim,freq,switch_config
#endif	
	
	#ifdef LOG_SIMULATOR
	cout<<"Success create EXTMEM in "<<freq<<"MHz"<<endl;
	#endif

  PE_freq_File.close();
  SW_freq_File.close();
	#ifdef LOG_SIMULATOR
  cout<<"Construct platform successfully"<<endl;
	#endif

	unsigned int packet_id=0;
	unsigned int newPacket_num=1;

	//Start simulation
	vector<Packet*> Finish_Packets;
	Unfinished_Packets.reserve(100);
	Traffic_Packets.reserve(100);
	Deleted_Packets.reserve(100);
	#ifdef LOG_SIMULATOR
	cout<<"Starting simulation......"<<endl;
	#endif
	#ifdef LOG_TRAFFIC
	ofstream oPtr1;
	oPtr1.open("traffic_file1", ios::out);
	oPtr1.setf(ios::fixed, ios::floatfield);
	oPtr1.precision(0);
	#endif

	tlm::tlm_generic_payload* noc_trans = 0;
	sc_core::sc_time sc_finish;
	sc_core::sc_time delay;
	PktPayload* pkt_payload = 0;
	while(true){
		wait(pkt_peq->get_event());
		wait(fastest_clk*4, SC_NS);
		wait(SC_ZERO_TIME);
		if(!Finish_Packets.empty()){
			for(unsigned int i=0;i<Finish_Packets.size();i++){
				#ifdef LOG_SIMULATOR
				PE_Rec.push_back(new PE_Rec_inf(Finish_Packets[i]->Src_PE->ID, Finish_Packets[i]->Dst_PE->ID, Finish_Packets[i]->send_time, Finish_Packets[i]->finished_time));
				#endif
				Finish_Packets[i]->DeletePortFlitInf();
				Deleted_Packets.push_back(Finish_Packets[i]);
			}
			Finish_Packets.clear();
		}
		Unfinished_Packets.clear();
		newPacket_num = 0;
		//cout << Traffic_Packets.size() << " " << Deleted_Packets.size() << endl;
		while((noc_trans = pkt_peq->get_next_transaction()) != 0){
			
			#ifdef LOG_SIMULATOR
				cout << "  NTS_sc_time_stamp = " << sc_time_stamp() << endl;
				tlm::tlm_generic_payload* trans = 0;
				trans = NocPayloadExt::get_ip_trans( noc_trans );
				cout << "  In NTS simulator_main " << endl;
				cout << "  cmd: " << trans->get_command() << endl;
				cout << "  NTS_noc_trans_target: " << dec << noc_trans->get_address() << endl << endl;
			#endif
			
			pkt_payload = reinterpret_cast<PktPayload* >( noc_trans->get_data_ptr() );
			#ifdef LOG_TRAFFIC
			oPtr1 << pkt_payload->src << " " << pkt_payload->dst << " " << pkt_payload->data_size-4 << " " << pkt_payload->send_time << endl;
			#endif
#ifdef LOG_SIMULATOR
			cout << "New packet: " << pkt_payload->src << " " << pkt_payload->dst << " " << pkt_payload->send_time << " " << pkt_payload->data_size << " " << pkt_payload->req_rsp << " " << noc_trans << endl;
#endif
			//Do memory manage in memory pool
			if(Deleted_Packets.empty()){
				Traffic_Packets.push_back(new Packet(_PE[(pkt_payload->src/cols_g)][(pkt_payload->src%cols_g)],_PE[(pkt_payload->dst/cols_g)][(pkt_payload->dst%cols_g)],pkt_payload->data_size,pkt_payload->send_time,packet_id,noc_trans));
				pkt_payload->pkt_ptr = Traffic_Packets.back(); 
				Unfinished_Packets.push_back(Traffic_Packets.back());
			}else{
				Deleted_Packets[0]->SetPacket(_PE[(pkt_payload->src/cols_g)][(pkt_payload->src%cols_g)],_PE[(pkt_payload->dst/cols_g)][(pkt_payload->dst%cols_g)],pkt_payload->data_size,pkt_payload->send_time,packet_id,noc_trans);
				pkt_payload->pkt_ptr = Deleted_Packets[0]; 
				Unfinished_Packets.push_back(Deleted_Packets[0]);
				Deleted_Packets.erase(Deleted_Packets.begin());
			}
			packet_id++;
			newPacket_num++;
			continue;
		}
		//Search each Packet transfer path(Switch path)
		//base on X-Y routing(cross col first then cross row)
		for(unsigned int i=0;i<Unfinished_Packets.size();i++){
		//cout << "123321" << endl;
			unsigned int step_col;
			unsigned int step_row;
			step_col = Unfinished_Packets[i]->Src_PE->col_dim;
			step_row = Unfinished_Packets[i]->Src_PE->row_dim;	
			//push started switch into path vector
			Unfinished_Packets[i]->TrPath_SW.push_back(_SW[step_row][step_col]);//push started switch into path vector
			
			while(step_col!=Unfinished_Packets[i]->Dst_PE->col_dim || step_row!=Unfinished_Packets[i]->Dst_PE->row_dim)
			{
				if(step_col > Unfinished_Packets[i]->Dst_PE->col_dim)
					step_col--;
				else if(step_col < Unfinished_Packets[i]->Dst_PE->col_dim)
					step_col++;
				else if(step_row > Unfinished_Packets[i]->Dst_PE->row_dim)
					step_row--;
				else if(step_row < Unfinished_Packets[i]->Dst_PE->row_dim)
					step_row++;
				else
					cout<<"Error happened in searching packet path"<<endl;
				Unfinished_Packets[i]->TrPath_SW.push_back(_SW[step_row][step_col]);//push switch into path vector
			}
			//Search each packet tx & rx port path by Switch path and find slowest clk in path
		}//each Packet has own Swith tranfer path
		//Search each packet tx & rx port path by Switch path and find slowest clk in path
		for(unsigned int i=0;i<Unfinished_Packets.size();i++){
			Unfinished_Packets[i]->SearchTxRxPortPath();
		}
		//carlrich:use the new packet path to pick out the possible congestion packet
		//cout<<"Starting pick out the possible involved packets..."<<endl;
		for(unsigned int i=0;i<Unfinished_Packets.size();i++){
			Unfinished_Packets[i]->SimulateSimpleHeaderTimetoRx(slowest_clk);
		}
		//According to the new Packet
		//Search for all of the possible congestion involved packets on its path
		//find it and throw all of the possible packets into the unfinished packet
		//do the static timing simulator again
		for(unsigned int i=0;i<newPacket_num;i++){
			//start from step 1
			SearchCongFlit(Unfinished_Packets[i]);
		}
	
		//make packets sequence mapping to packet id
		sort(Unfinished_Packets.begin(), Unfinished_Packets.end(), cmp_packet_ptr);
		//cout << endl;
		for(unsigned int i=0;i<Unfinished_Packets.size();i++){
			Unfinished_Packets[i]->ID = i;
			//cout << "Going to compute " << Unfinished_Packets[i]->Src_PE->ID << " " << Unfinished_Packets[i]->Dst_PE->ID << " " << Unfinished_Packets[i]->send_time << endl;
		}
		
		for(unsigned int row=0;row<rows_g;row++){
			for(unsigned int col=0;col<cols_g;col++)
			{
				_SW[row][col]->CorrectTxRxPort();
				_PE[row][col]->CorrectTxRxPort();
			}
		}
		//_PE[4][0]->CorrectTxRxPort();

#ifdef NTS_4_external_memory		
		// Lai along
		_PE[4][0]->CorrectTxRxPort();
		_PE[4][1]->CorrectTxRxPort();
		_PE[4][2]->CorrectTxRxPort();
		_PE[4][3]->CorrectTxRxPort();
#else
		_PE[4][0]->CorrectTxRxPort();
#endif
		
			
		//Start simulation
		//cout<<"Starting static simulation......"<<endl;
		while(Unfinished_Packets.size()!=0)
		{
			//First simulate each header and tail to rx_port on it's path
			for(unsigned int i=0;i<Unfinished_Packets.size();i++){
				Unfinished_Packets[i]->SimulateHeaderTimetoRx();
			}
			//Check each packet if it hold all tx ports on it's path
			//if packet got wormhole path then can finished transfer
			for(unsigned int i=0;i<Unfinished_Packets.size();i++)
			{
				Unfinished_Packets[i]->CheckWormHolePath();
				if(Unfinished_Packets[i]->transfer_enable==true)
				{ 
					Unfinished_Packets[i]->SetPortFunctionalClk();
					
					//finish tranfer packet
					if(Unfinished_Packets[i]->vec_cong_port.empty())//transfer path without congestion
						Unfinished_Packets[i]->SimulateTailTimetoRxWithoutCongestion();
					else{
						if(FIFO_size == 0)
							Unfinished_Packets[i]->SimulateTailTimetoRxWithCongestion();
						else
							Unfinished_Packets[i]->SimulateFIFOSizeTime();
					}
					//cout << "Finish SimulateFIFOSizeTime\n";

					Unfinished_Packets[i]->UpdateTxRxReleaseTime();//record tx & rx release time by this packet
					//push the finished packet to the complete_peq
					//Unfinished_Packets[i]->ShowSWPath();
					//cout << Unfinished_Packets[i]->finished_time << endl;
					noc_trans = reinterpret_cast<tlm::tlm_generic_payload* >( Unfinished_Packets[i]->trans_ptr );
					pkt_payload = reinterpret_cast<PktPayload* >( noc_trans->get_data_ptr() );
					sc_finish = sc_core::sc_time(Unfinished_Packets[i]->finished_time, SC_NS);
					if(sc_finish < sc_time_stamp()){
						cout << sc_finish << " < " << sc_time_stamp() << endl;
						Finish_Packets.push_back(Unfinished_Packets[i]);
						sc_finish = sc_core::sc_time(pkt_payload->finish_time, SC_NS);
						delay = sc_finish - sc_time_stamp();
						recompute_peq->notify(*Unfinished_Packets[i]->trans_ptr, delay);
						//cout << pkt_payload->finish_time << ":" << Unfinished_Packets[i]->finished_time << endl;
					}else if(pkt_payload->finish_time != Unfinished_Packets[i]->finished_time){
						if(pkt_payload->finish_time > Unfinished_Packets[i]->finished_time){
							//wrong finish time
							sc_finish = sc_core::sc_time(pkt_payload->finish_time, SC_NS);
							delay = sc_finish - sc_time_stamp();
							recompute_peq->notify(*Unfinished_Packets[i]->trans_ptr, delay);
							//cout << pkt_payload->finish_time << ":" << Unfinished_Packets[i]->finished_time << endl;
						}
						sc_finish = sc_core::sc_time(Unfinished_Packets[i]->finished_time, SC_NS);
						delay = sc_finish - sc_time_stamp();
						pkt_payload->finish_time = Unfinished_Packets[i]->finished_time;

	#ifdef LOG_SIMULATOR
						cout << sc_finish << " - " << sc_time_stamp() << " = " << delay << endl;
						cout << "Finish packet: " << pkt_payload->src << " " << pkt_payload->dst << " " << pkt_payload->send_time << " " << pkt_payload->data_size << " " << pkt_payload->req_rsp << " " << pkt_payload->finish_time << " " << Unfinished_Packets[i]->trans_ptr << endl;
#endif
						//printf("Finish packet data_ptr:%X\n", pkt_payload->network_payload);
						complete_peq->notify(*Unfinished_Packets[i]->trans_ptr, delay);
					}
					//carlrich: update header_time_list
					for(unsigned int j=0;j<Unfinished_Packets[i]->TrPath_rx.size();j++){
						Unfinished_Packets[i]->header_time_list[j]=Unfinished_Packets[i]->TrPath_rx[j]->header_time;
						Unfinished_Packets[i]->tail_time_list[j]=Unfinished_Packets[i]->TrPath_rx[j]->tail_time;
					}
					Unfinished_Packets[i]->reCompute = false;
					Unfinished_Packets.erase(Unfinished_Packets.begin()+i);
				}
			}
			//Clean all tx & rx ports's header and tail time
			//Then simulation again
			for(unsigned int row=0;row<rows_g;row++){
				for(unsigned int col=0;col<cols_g;col++)
				{
					_SW[row][col]->ResetTxRxPort();
					_PE[row][col]->ResetTxRxPort();
				}
			}
			//_PE[4][0]->ResetTxRxPort();
#ifdef NTS_4_external_memory			
			// Lai along
			_PE[4][0]->ResetTxRxPort();
			_PE[4][1]->ResetTxRxPort();
			_PE[4][2]->ResetTxRxPort();
			_PE[4][3]->ResetTxRxPort();
#else
			_PE[4][0]->ResetTxRxPort();
#endif

		}//end while loop(simulation one time)
	}
	#ifdef LOG_TRAFFIC
	oPtr1.close();
	#endif
}

template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g> 
void
Simulator<data_width_g, rows_g, cols_g>::
SearchCongFlit(Packet* p){
	//cout<<"Search for congestion flit:\n";
	unsigned int cmp_flit_id;
	long double cmp_delay;
	Packet* cmp_p;
	//p->ShowSWPath();
	//p->ShowTxRxPortPath();
	DeleteLaterPkt(p);
	for(unsigned int i=1;i<p->TrPath_rx.size();i++){
		for(int j=0;j<(int)p->TrPath_rx[i]->passed_pkt.size();j++){
			cmp_p = (Packet *)p->TrPath_rx[i]->passed_pkt[j];
			//printf("Checking for reCompute:step:%u@SW_%u port_%u pid(%u,%u)\n", i, p->TrPath_rx[i]->sw_id, p->TrPath_rx[i]->port_dir, p->packet_ID, cmp_p->packet_ID);
			if(cmp_p->packet_ID != p->packet_ID){
				cmp_flit_id=p->TrPath_rx[i]->flit_id[j];
				//printf("Checking for header_t:step:%u@SW_%u port_%u pid(%u,%u) time(%.2llf-%.2llf,%.2llf-%.2llf)\n", i, p->TrPath_rx[i]->sw_id, p->TrPath_rx[i]->port_dir, p->packet_ID, cmp_p->packet_ID, p->header_time_list[i], p->tail_time_list[i], cmp_p->header_time_list[cmp_flit_id], cmp_p->tail_time_list[cmp_flit_id]);
				//cmp_p win => delay p
				if(cmp_p->header_time_list[cmp_flit_id] <= p->header_time_list[i] && p->header_time_list[i] < cmp_p->tail_time_list[cmp_flit_id]){
					if(cmp_p->reCompute==false){
						cmp_p->reCompute=true;
						Unfinished_Packets.push_back(cmp_p);
						cmp_p->InsertRecomputeId();
						DeleteLaterPkt(cmp_p);
					}
					if(p->reCompute==false){
						p->reCompute=true;
						Unfinished_Packets.push_back(p);
						p->InsertRecomputeId();
						DeleteLaterPkt(p);
					}
					//the loser will wait at the i-1 rx_port header time
					cmp_delay=cmp_p->tail_time_list[cmp_flit_id] - p->header_time_list[i-1];
					//printf("first packet win: cmp_delay=%.2llf %.2llf-%.2llf\n", cmp_delay, cmp_p->tail_time_list[cmp_flit_id], p->header_time_list[i-1]);
					for(unsigned int k=0;k<(unsigned int)i;k++){
						//printf("Delay all packets before:step:%u@SW_%u port_%u pid:%u (%.2llf,%.2llf) -> ", k, p->TrPath_rx[k]->sw_id, p->TrPath_rx[k]->port_dir, p->packet_ID, p->header_time_list[k], p->tail_time_list[k]);
						p->tail_time_list[k]=p->tail_time_list[k]+cmp_delay;
						//printf("(%.2llf,%.2llf)\n", p->header_time_list[k], p->tail_time_list[k]);
					}
					for(unsigned int k=i;k<p->TrPath_rx.size();k++){
						//printf("Delay all packets behind:step:%u@SW_%u port_%u pid:%u (%.2llf,%.2llf) -> ", k, p->TrPath_rx[k]->sw_id, p->TrPath_rx[k]->port_dir, p->packet_ID, p->header_time_list[k], p->tail_time_list[k]);
						p->header_time_list[k]=p->header_time_list[k]+cmp_delay;
						p->tail_time_list[k]=p->tail_time_list[k]+cmp_delay;
						//printf("(%.2llf,%.2llf)\n", p->header_time_list[k], p->tail_time_list[k]);
					}
					//check all packets pass this port again(j=-1)
					i=0;
					j=-1;
				//p win => delay cmp_p
				}else if(p->header_time_list[i] <= cmp_p->header_time_list[cmp_flit_id] && cmp_p->header_time_list[cmp_flit_id] < p->tail_time_list[i]){
					if(cmp_p->reCompute==false){
						cmp_p->reCompute=true;
						Unfinished_Packets.push_back(cmp_p);
						cmp_p->InsertRecomputeId();
						DeleteLaterPkt(cmp_p);
					}
					if(p->reCompute==false){
						p->reCompute=true;
						Unfinished_Packets.push_back(p);
						p->InsertRecomputeId();
						DeleteLaterPkt(p);
					}
					//the loser will wait at the i-1 rx_port header time
					cmp_delay=p->tail_time_list[i] - cmp_p->header_time_list[cmp_flit_id-1];
					//printf("first packet lose: cmp_delay=%.2llf %.2llf-%.2llf\n", cmp_delay, p->tail_time_list[i], cmp_p->header_time_list[cmp_flit_id-1]);
					for(unsigned int k=0;k<cmp_flit_id;k++){
						//printf("Delay all packets before:step:%u@SW_%u port_%u pid:%u (%.2llf,%.2llf) -> ", k, cmp_p->TrPath_rx[k]->sw_id, cmp_p->TrPath_rx[k]->port_dir, cmp_p->packet_ID, cmp_p->header_time_list[k], cmp_p->tail_time_list[k]);
						cmp_p->tail_time_list[k]=cmp_p->tail_time_list[k]+cmp_delay;
						//printf("(%.2llf,%.2llf)\n", cmp_p->header_time_list[k], cmp_p->tail_time_list[k]);
					}
					for(unsigned int k=cmp_flit_id;k<cmp_p->TrPath_rx.size();k++){
						//printf("Delay all packets behind:step:%u@SW_%u port_%u pid:%u (%.2llf,%.2llf) -> ", k, cmp_p->TrPath_rx[k]->sw_id, cmp_p->TrPath_rx[k]->port_dir, cmp_p->packet_ID, cmp_p->header_time_list[k], cmp_p->tail_time_list[k]);
						cmp_p->header_time_list[k]=cmp_p->header_time_list[k]+cmp_delay;
						cmp_p->tail_time_list[k]=cmp_p->tail_time_list[k]+cmp_delay;
						//printf("(%.2llf,%.2llf)\n", cmp_p->header_time_list[k], cmp_p->tail_time_list[k]);
					}
					//check all the following path of the cmp_p
					//SearchCongFlit(cmp_p, cmp_flit_id);
					SearchCongFlit(cmp_p);
				}
			}
		}
	}
}

template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g> 
void
Simulator<data_width_g, rows_g, cols_g>::
DeleteLaterPkt(Packet* p){
	Packet* passed_p;
	unsigned int passed_flit_id;
	for(unsigned int i=1;i<p->TrPath_rx.size();i++){
		for(unsigned int j=0;j<p->TrPath_rx[i]->passed_pkt.size();j++){
			passed_p = (Packet *)p->TrPath_rx[i]->passed_pkt[j];
			passed_flit_id = p->TrPath_rx[i]->flit_id[j];
			if(p->header_time_list[i] < passed_p->header_time_list[passed_flit_id]){
				//cout << p->header_time_list[i] << " < " << passed_p->header_time_list[passed_flit_id] << endl;
				if(passed_p->reCompute==false){
					//cout << "First Time\n";
					passed_p->reCompute=true;
					Unfinished_Packets.push_back(passed_p);
					passed_p->InsertRecomputeId();
					DeleteLaterPkt(passed_p);
				}
			}
		}
	}
}

template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g> 
bool
Simulator<data_width_g, rows_g, cols_g>::
cmp_packet_ptr(const Packet* left, const Packet* right){
	if(left->send_time < right->send_time)
		return true;
	else
		return false;
}

template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g> 
bool
Simulator<data_width_g, rows_g, cols_g>::
cmp_pe_pkt_ptr(const PE_Rec_inf* left, const PE_Rec_inf* right){
	if(left->send_time < right->send_time)
		return true;
	else
		return false;
}

//Puts incoming data into payload event queue (=FIFO)
template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g>
tlm::tlm_sync_enum
Simulator<data_width_g, rows_g, cols_g>::
nb_transport_fw(int router,
		tlm::tlm_generic_payload &trans,
		tlm::tlm_phase           &phase,
		sc_core::sc_time         &delay)
{
	//ispa::NetworkPayload* received_payload = 0;
	tlm::tlm_phase bw_phase;
	tlm::tlm_sync_enum retval;
	
	#ifdef LOG_SIMULATOR
	cout 
		<< endl
		<< "  NTS_fw_sc_time_stamp = " << sc_time_stamp() << endl
		<< "  " << sc_core::sc_time_stamp() << endl
		<< "  fw() " << name() //<< "(" << row_ << "," << col_ << ") "  
		<< router << " " << phase << ", del " << delay << endl
		<< "  source: " << NocPayloadExt::get_source_id(trans) << endl
		<< "  target: " << dec << trans.get_address() << endl << endl;
	#endif

	// Check the command
	if(trans.get_command() != tlm::TLM_WRITE_COMMAND)
	{
		ostringstream oss;
		oss << "TlmMeshRouter::nb_tranport_fw " << router
			<< ": only write command is supported";
		throw std::runtime_error(oss.str().c_str());
	}

	// Two phases of transfer
	if(phase == tlm::BEGIN_REQ)
	{
		trans.acquire();
			
    tlm::tlm_generic_payload *ip_trans = NocPayloadExt::get_ip_trans(trans);
    unsigned int source_id = NocPayloadExt::get_source_id(trans);
    unsigned int target_id = trans.get_address();

		//send pkt inf to pkt_peq
		PktPayload* sent_payload = new PktPayload((unsigned char*)ip_trans, source_id, target_id, (long double)(sc_time_stamp().to_seconds() + delay.to_seconds())*1000000000, (unsigned int)trans.get_data_length(), router);
#ifdef LOG_SIMULATOR
		cout << "@fw function: "<< sent_payload->src << " " << sent_payload->dst << " " << sent_payload->send_time << " " << sent_payload->data_size << " " << sent_payload->req_rsp << endl;
#endif
		//printf("socket data_ptr:%X\n", trans.get_data_ptr());
		trans.set_data_ptr( reinterpret_cast<unsigned char*>( sent_payload ) );
		pkt_peq->notify(trans, delay); // put data to queue
	}
	else if(phase == tlm::END_RESP)
	{
		trans.set_response_status(tlm::TLM_OK_RESPONSE);
		return tlm::TLM_COMPLETED;
	}
	else
	{
		ostringstream oss;
		oss << "TlmMeshRouter::nb_tranport_fw " << router 
			<< ": got invalid PHASE";
		throw runtime_error(oss.str().c_str());
	}

	trans.set_response_status( tlm::TLM_OK_RESPONSE );
	return tlm::TLM_COMPLETED;
}

//Acknowledge that data was received
template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g>
tlm::tlm_sync_enum
Simulator<data_width_g, rows_g, cols_g>::
nb_transport_bw(int router,
		tlm::tlm_generic_payload &trans,
		tlm::tlm_phase           &phase,
		sc_core::sc_time         &delay)
{

  
	NTS_DEBUG(
  source_id_ext *source_id;
  trans.get_extension( source_id );

	cout 
  << endl
	<< sc_core::sc_time_stamp() << endl
	<<"bw() " << name() //<< "(" << row_ << "," << col_ << ") "  
	<< router << " "  << phase << ", del " << delay << endl
  << "  source: " << source_id.value << endl
  << "  target: " << trans.get_address() << endl;
	);

	if(phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
	{
		ostringstream oss;
		oss << "TlmMeshRouter::nb_tranport_bw " << router << " got wrong phase";
		throw runtime_error(oss.str().c_str());
	}
	//cout << "  NTS_bw_sc_time_stamp = " << sc_time_stamp() << endl << endl;
	//txCompleteEvent_[id].notify(delay);

	trans.set_response_status( tlm::TLM_OK_RESPONSE );
	return tlm::TLM_COMPLETED;
}

//template class Simulator<_data_width, _row, _col>;
template class Simulator<32, 4, 4>;
