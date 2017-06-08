/*
 * simulator.h
 * Copyright (C) 2014 m101061536 <m101061536@ws36>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <systemc>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <vector>
#include "tlm.h"
#ifdef MTI_SYSTEMC
#include "simple_initiator_socket.h"
#include "simple_target_socket.h"
#include "peq_with_get.h"
#else
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_get.h"
#endif

#include "PE.h"
#include "Switch.h"
#include "Packet.h"
//#include "NetworkPayload.h"
#include "PktPayload.h"
#include "config.h"


//#define EN_SIMULATOR_DEBUG
#include "NocTimingSimDebug.h"

#define NTS_4_external_memory
//#define LOG_SIMULATOR

using namespace sc_core;
using namespace std;

template<unsigned int data_width_g = 32,
	unsigned int rows_g = 4,
	unsigned int cols_g = 4>
class Simulator:
  public sc_module
{

//--- Public methods ---//
public:

	//Constructor & destructor
	SC_HAS_PROCESS(Simulator);
	Simulator(sc_module_name name);
	~Simulator();

#ifdef NTS_4_external_memory
	//terminals connect to IP
	tlm_utils::simple_initiator_socket_tagged<Simulator, data_width_g>* req_init[(rows_g * cols_g) + 4];
	tlm_utils::simple_initiator_socket_tagged<Simulator, data_width_g>* rsp_init[rows_g * cols_g];
	tlm_utils::simple_target_socket_tagged<Simulator, data_width_g>* req_target[rows_g * cols_g];
	tlm_utils::simple_target_socket_tagged<Simulator, data_width_g>* rsp_target[(rows_g * cols_g) + 4];
#else
	tlm_utils::simple_initiator_socket_tagged<Simulator, data_width_g>* req_init[(rows_g * cols_g) + 1];
	tlm_utils::simple_initiator_socket_tagged<Simulator, data_width_g>* rsp_init[rows_g * cols_g];
	tlm_utils::simple_target_socket_tagged<Simulator, data_width_g>* req_target[rows_g * cols_g];
	tlm_utils::simple_target_socket_tagged<Simulator, data_width_g>* rsp_target[(rows_g * cols_g) + 1];
#endif
private:
	void simulator_main();
	void simulator_send();
	void SearchCongFlit(Packet*);
	void DeleteLaterPkt(Packet*);
	static bool cmp_packet_ptr(const Packet*, const Packet*);
	static bool cmp_pe_pkt_ptr(const PE_Rec_inf*, const PE_Rec_inf*);

	vector<Packet*> Traffic_Packets;
	vector<Packet*> Unfinished_Packets;
	vector<Packet*> Deleted_Packets;
	unsigned int switch_value[9];
	#ifdef LOG_SIMULATOR
	vector<PE_Rec_inf*> PE_Rec;//PE receive packets inf
	#endif

	tlm_utils::peq_with_get<tlm::tlm_generic_payload>* pkt_peq;
	tlm_utils::peq_with_get<tlm::tlm_generic_payload>* complete_peq;
	tlm_utils::peq_with_get<tlm::tlm_generic_payload>* recompute_peq;

	//Puts incoming data into payload event queue (=FIFO)
	tlm::tlm_sync_enum nb_transport_fw(int router,
			tlm::tlm_generic_payload &trans,
			tlm::tlm_phase           &phase,
			sc_core::sc_time         &delay);

	//Acknowledge that data was received
	tlm::tlm_sync_enum nb_transport_bw(int router,
			tlm::tlm_generic_payload &trans,
			tlm::tlm_phase           &phase,
			sc_core::sc_time         &delay);
};

#endif /* !SIMULATOR_H */

