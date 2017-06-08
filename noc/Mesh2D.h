/**
 *
 * @file req_rsp_mesh_2d.h
 * @author Ting-Shuo Hsu
 *
 * @file tlm_mesh_router.hh
 * @author Lasse Lehtonen
 *
 */

/*
 * Copyright 2010 Tampere University of Technology
 * 
 *  This file is part of Transaction Generator.
 *
 *  Transaction Generator is free software: you can redistribute it and/or modify
 *  it under the terms of the Lesser GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Transaction Generator is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public License
 *  along with Transaction Generator.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * 2013-12-21 03:32:20, tshsu:
 * Models a 2D mesh network at transaction-level, with request and response path supporting both read and write.
 *
 */

#ifndef REQ_RSP_MESH_2D_H_
#define REQ_RSP_MESH_2D_H_

#include "TlmMeshRouter.h"
using asebt::mesh_2d_sc_tlm_1::TlmMeshRouter;

#include "SimpleAddressMap.h"
#include "InitNI.h"
#include "TargetNI.h"

#include "tlm.h"
#ifdef MTI_SYSTEMC
#include "simple_initiator_socket.h"
#include "simple_target_socket.h"
#else
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#endif
#include <systemc>

#include "simulator.h"

#define TG2 1
#define NTS 2

//#define NOC TG2
#define NOC NTS

#define NTS_4_external_memory

namespace ispa
{
template<unsigned int data_width_g = 32,
	unsigned int rows_g = 4,
	unsigned int cols_g = 4>
	class Mesh2D : public sc_core::sc_module
//	public tlm::tlm_mm_interface
{
	public:
		SC_HAS_PROCESS(Mesh2D);	 

		//public functions
		Mesh2D(sc_core::sc_module_name name, sc_core::sc_time cycle_time = sc_core::sc_time(20000,sc_core::SC_PS) );

		~Mesh2D();

		void set_memory_map();

		void set_memory_map_shared_memory();

  public:
    //Interfaces
    InitNI<data_width_g>*   initNIs[rows_g * cols_g];
    TargetNI<data_width_g>* targetNIs[rows_g * cols_g];
#ifdef NTS_4_external_memory
    TargetNI<data_width_g>* ext_mem_ni[4];
#else
	TargetNI<data_width_g>* ext_mem_ni;
#endif
	private:
		//Member variables
		sc_core::sc_time cycleTime_;
		const unsigned int HEADER_LENGTH_IN_BYTE;


#if NOC == TG2
		//Use TG2 Routers.
		enum Direction {N, W, S, E, PE, NUM_DIR};
		TlmMeshRouter<data_width_g>* req_routers_[rows_g * cols_g];
		TlmMeshRouter<data_width_g>* rsp_routers_[rows_g * cols_g];

#elif NOC == NTS
    //Use NoC Timing Simulator
    Simulator<data_width_g, rows_g, cols_g>* simulator;

#endif

	
    SimpleAddressMap m_addr_map; //a pretty simple address map

    //--- Helper functions ---//
    std::string req_router_name(const int row, const int col);
    std::string rsp_router_name(const int row, const int col);
    std::string nocInitIF_name (const int row, const int col);
    std::string nocTargetIF_name (const int row, const int col);


};

}//end of namespace tshsu

#endif

// Local Variables:
// mode: c++
// c-file-style: "ellemtel"
// c-basic-offset: 3
// End:

