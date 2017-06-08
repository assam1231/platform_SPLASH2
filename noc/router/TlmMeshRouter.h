/**
 *
 * @file tlm_mesh_router.hh
 * @author Lasse Lehtonen
 *
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
 * $Id: tlm_mesh_router.hh 206 2013-08-28 12:45:29Z ege $
 *
 */

#ifndef ASEBT_TLM_MESH_ROUTER_HH
#define ASEBT_TLM_MESH_ROUTER_HH

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

#include <systemc>
#include <stdexcept>
#include <iostream>
#include <iomanip>

//#define EN_ROUTER_DEBUG
#include "TlmMeshRouterDebug.h"

namespace asebt
{
	namespace mesh_2d_sc_tlm_1
	{

		template<unsigned int data_width_g = 32>
			class TlmMeshRouter : public sc_core::sc_module
		{
			private:
				enum direction {N, W, S, E, PE, NUM_DIR, DIR_ERROR};
			public:

				SC_HAS_PROCESS(TlmMeshRouter);

        enum router_type {req, rsp};

				// Sockets: N, W, S, E, PE
				tlm_utils::simple_target_socket_tagged<TlmMeshRouter,
					data_width_g>* targetSockets[NUM_DIR];
				tlm_utils::simple_initiator_socket_tagged<TlmMeshRouter,
					data_width_g>* initSockets[NUM_DIR];

				//* Constructor
				TlmMeshRouter(sc_core::sc_module_name name,
						unsigned int row,
						unsigned int col,
						unsigned int n_rows,
						unsigned int n_cols, 
            router_type type,    
						sc_core::sc_time cycle_time = sc_core::sc_time(20000,sc_core::SC_PS) )
					: sc_module(name),
					row_(row),
					col_(col),
					cycleTime_(cycle_time),
					n_cols_(n_cols), 
          type_(type)
			{

				// Instantiate a queue (=FIFO) for each src-dst port pair
				for(unsigned int d = 0; d < NUM_DIR; ++d)
				{
					for(unsigned int s = 0; s < NUM_DIR; ++s)
					{
						std::ostringstream oss;
						oss << "tlm_router_peq_" << s << "_" << d;
						peq_[s][d] = 
							new tlm_utils::peq_with_get<tlm::tlm_generic_payload>
							(oss.str().c_str());
					}
				}

        for (int i=0; i<NUM_DIR; i++) {
          top_priority[i] = 0;
        }



				// Create ports = sockets. Inports are targets because PE
				// writes to them. Outport are initiators.

				// Create first the ports for the PE
				targetSockets[PE] = new tlm_utils::simple_target_socket_tagged
					<TlmMeshRouter, data_width_g>;
				initSockets[PE] = new tlm_utils::simple_initiator_socket_tagged
					<TlmMeshRouter, data_width_g>;


				// Create other ports but network boundaries do not need
				// all of them
				if(row != 0)
				{
					initSockets[N] = new tlm_utils::simple_initiator_socket_tagged
						<TlmMeshRouter, data_width_g>;
					targetSockets[N] = new tlm_utils::simple_target_socket_tagged
						<TlmMeshRouter, data_width_g>;
				}
				else
				{
					initSockets[N] = 0;
					targetSockets[N] = 0;
				}

				if(row != (n_rows - 1))
				{
					initSockets[S] = new tlm_utils::simple_initiator_socket_tagged
						<TlmMeshRouter, data_width_g>;
					targetSockets[S] = new tlm_utils::simple_target_socket_tagged
						<TlmMeshRouter, data_width_g>;
				}
				else
				{
					initSockets[S] = 0;
					targetSockets[S] = 0;
				}

				if(col != 0)
				{
					initSockets[W] = new tlm_utils::simple_initiator_socket_tagged
						<TlmMeshRouter, data_width_g>;
					targetSockets[W] = new tlm_utils::simple_target_socket_tagged
						<TlmMeshRouter, data_width_g>;
				}
				else
				{
					initSockets[W] = 0;
					targetSockets[W] = 0;
				}

				if(col != (n_cols - 1))
				{
					initSockets[E] = new tlm_utils::simple_initiator_socket_tagged
						<TlmMeshRouter, data_width_g>;
					targetSockets[E] = new tlm_utils::simple_target_socket_tagged
						<TlmMeshRouter, data_width_g>;
				}
				else
				{
					initSockets[E] = 0;
					targetSockets[E] = 0;
				}

        //for external memory
        if(col==0 && row==0 && type_==req)
        {
					initSockets[N] = new tlm_utils::simple_initiator_socket_tagged
						<TlmMeshRouter, data_width_g>;
        }
        else if(col==0 && row==0 && type_==rsp)
        {
					targetSockets[N] = new tlm_utils::simple_target_socket_tagged
						<TlmMeshRouter, data_width_g>;
        }


				// Register callback functions to sockets. Launch a thread
				// for each outport.
				for(unsigned int i = 0; i < NUM_DIR; ++i)
				{
					if(targetSockets[i])
					{
						targetSockets[i]->register_nb_transport_fw
							(this, &TlmMeshRouter::nb_transport_fw, i);
          }
					if(initSockets[i])
					{
						initSockets[i]->register_nb_transport_bw
							(this, &TlmMeshRouter::nb_transport_bw, i);
						sc_spawn(sc_bind(&TlmMeshRouter::thread, this, i));
            ROUTER_DEBUG(
              if (type_==req){
                std::cout << "Request router[" << row_ << "][" << col_ << "]" 
                          << "Spawn thread for dir " << dir_name[i] << std::endl;
              }
              else {
                std::cout << "Response router[" << row_ << "][" << col_ <<"]" 
                          << "Spawn thread for dir " << dir_name[i] << std::endl;
              }
            );
					}
				}
			}

				//* Destructor
				~TlmMeshRouter()
				{
					for(unsigned int i = 0; i < NUM_DIR; ++i)
					{
						if(initSockets[i]) {delete initSockets[i]; initSockets[i] = 0;}
						if(targetSockets[i]) {delete targetSockets[i]; targetSockets[i] = 0;}

						for(unsigned int j = 0; j < NUM_DIR; ++j)
						{
							delete peq_[i][j]; peq_[i][j] = 0;
						}
					}
				}



				/*
				 * Main functionality. Gets data from FIFO and forwards it to
				 * outport 'my_dir'.
				 */
				void thread(unsigned int my_dir)
				{
					tlm::tlm_generic_payload* trans = 0;
					tlm::tlm_phase            phase;
					sc_core::sc_time          delay;
					tlm::tlm_sync_enum        retval;
					unsigned int              source = 0;


					while(true)
					{
						// Check for pending transactions
						source = round_robin_switch_arbitration( my_dir, &trans );

						if(trans == 0)
						{
							// Wait for transactions going towards 'my_dir'
							ROUTER_DEBUG(
							std::cout << sc_core::sc_time_stamp() << " thr " << name() 
                        << "(" << row_ << "," << col_ << ") " 
                        << dir_name[my_dir] << " wait peq_event" << std::endl;
							);
							wait(peq_[N][my_dir]->get_event() |
									peq_[W][my_dir]->get_event() |
									peq_[S][my_dir]->get_event() |
									peq_[E][my_dir]->get_event() |
									peq_[PE][my_dir]->get_event());
							continue; // jump back to beginning of while(true)
						}

						ROUTER_DEBUG(
						std::cout << 
						std::setw(12) << sc_core::sc_time_stamp() 
						<< std::setw(8) << " thr " << name() << "(" << row_ << "," << col_ << ") " 
						<< dir_name[my_dir] << " proceeds" << std::endl;
						);

						// Send acknowledge
						if(source == PE)
						{
							// 2013-08-26 (ES): why is ip-outport path faster than others???
							delay = sc_core::SC_ZERO_TIME;
						}
						else
						{
							delay = cycleTime_ * 
								((trans->get_data_length() / trans->get_streaming_width())
								 + 3); // pkt latency = hdr latency + data copying
						}

						phase = tlm::END_REQ;
						ROUTER_DEBUG(
						std::cout 
						<< std::setw(12) << sc_core::sc_time_stamp() 
						<< std::setw(8) << " thr " << name() << "(" << row_ << "," << col_ << ") " 
						<< dir_name[my_dir] << "  sends ack[" << source << "]->bw() with delay = " << delay << std::endl;
						);
						retval = (*targetSockets[source])->nb_transport_bw(*trans, phase, delay);

						if(retval != tlm::TLM_COMPLETED)
						{
							std::ostringstream oss;
							oss << "TlmMeshRouter::thread : Not supporting responses";
							throw std::runtime_error(oss.str().c_str());
						}


						// Forward the transaction to router's outport (=init socket)	       
						phase = tlm::BEGIN_REQ;
						delay = cycleTime_ * 3;	// header latency    
						ROUTER_DEBUG(
						std::cout 
						<< std::setw(12) << sc_core::sc_time_stamp() 
						<< std::setw(8) << " thr " << name() << "(" << row_ << "," << col_ << ") " 
						<< dir_name[my_dir] << "  calls fw() with delay = " << delay << std::endl;
						);
						retval = (*initSockets[my_dir])->nb_transport_fw(*trans, phase, delay);

						if(retval == tlm::TLM_ACCEPTED || retval == tlm::TLM_UPDATED)
						{
							if(phase == tlm::BEGIN_REQ)
							{		  
								wait(txCompleteEvent_[my_dir]);		
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
							if(delay != sc_core::SC_ZERO_TIME)
							{
								wait(delay);
							}
						}
						else
						{
							std::ostringstream oss;
							oss << "TlmMeshRouter::thread : invalid SYNC_ENUM";
							throw std::runtime_error(oss.str().c_str());
						}	       

						trans->release();
					} // end of while(true)
				}      


			private:

				/*
				 * Puts incoming data into payload event queue (=FIFO)
				 */
				tlm::tlm_sync_enum nb_transport_fw(int my_dir,
						tlm::tlm_generic_payload &trans,
						tlm::tlm_phase           &phase,
						sc_core::sc_time         &delay)
				{

					ROUTER_DEBUG(
					std::cout 
					<< std::setw(12) << sc_core::sc_time_stamp() 
					<< std::setw(8) << " fw() " << name() << "(" << row_ << "," << col_ << ") "  
					<< dir_name[my_dir] << " " << phase << ", del " << delay << std::endl
          << std::setw(20) << "Src: " << get_source_id(trans) 
          << ", Dest: " << trans.get_address() << std::endl;
					);

					// Check the command
					if(trans.get_command() != tlm::TLM_WRITE_COMMAND)
					{
						std::ostringstream oss;
						oss << "TlmMeshRouter::nb_tranport_fw " << my_dir 
							<< ": only write command is supported";
						throw std::runtime_error(oss.str().c_str());
					}


					// Two phases of transfer
					if(phase == tlm::BEGIN_REQ)
					{
						trans.acquire();
						unsigned int out_dir = yx_routing(trans.get_address());
						peq_[my_dir][out_dir]->notify(trans, delay); // put data to queue
					}
					else if(phase == tlm::END_RESP)
					{
						trans.set_response_status(tlm::TLM_OK_RESPONSE);
						return tlm::TLM_COMPLETED;
					}
					else
					{
						std::ostringstream oss;
						oss << "TlmMeshRouter::nb_tranport_fw " << my_dir 
							<< ": got invalmy_dir PHASE";
						throw std::runtime_error(oss.str().c_str());
					}

					trans.set_response_status( tlm::TLM_OK_RESPONSE );
					return tlm::TLM_ACCEPTED;
				}


				/*
				 * Acknowledge that data was received
				 */
				tlm::tlm_sync_enum nb_transport_bw(int my_dir,
						tlm::tlm_generic_payload &trans,
						tlm::tlm_phase           &phase,
						sc_core::sc_time         &delay)
				{

					ROUTER_DEBUG(
					std::cout 
					<< std::setw(12) << sc_core::sc_time_stamp() 
					<< std::setw(8) << " bw() " << name() << "(" << row_ << "," << col_ << ") "  
					<< dir_name[my_dir] << " "  << phase << ", del " << delay << std::endl;
					);

					if(phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
					{
						std::ostringstream oss;
						oss << "TlmMeshRouter::nb_tranport_bw " << my_dir << " got wrong phase";
						throw std::runtime_error(oss.str().c_str());
					}

					txCompleteEvent_[my_dir].notify(delay);

					trans.set_response_status( tlm::TLM_OK_RESPONSE );
					return tlm::TLM_COMPLETED;
				}

				unsigned int top_priority[NUM_DIR];
				unsigned int round_robin_switch_arbitration( const int dir, tlm::tlm_generic_payload** trans_ptr_ptr )
				{
					for(unsigned int i = 0; i < NUM_DIR; ++i)
					{
					  unsigned int source = (top_priority[dir]+i) % NUM_DIR;
						if((*trans_ptr_ptr = peq_[source][dir]->get_next_transaction()) != 0)
						{ 
							top_priority[dir] = (source+1) % NUM_DIR;
							ROUTER_DEBUG(
							std::cout 
							<< std::setw(12) << sc_core::sc_time_stamp() 
							<< std::setw(8) << " thr " << name() << "(" << row_ << "," << col_ << ") " 
							<< dir_name[dir] << " got next transact from " << dir_name[source] << std::endl
							<< std::setw(20) << "Src: " 
              << (reinterpret_cast<NetworkPayload*> ((*trans_ptr_ptr)->get_data_ptr()))->source_id 
							<< ", Dest: " 
              << (*trans_ptr_ptr)->get_address() 
              << ", top priority: " << dir_name[top_priority[dir]] << std::endl;
							);
							return source;
						}
					}
          return DIR_ERROR;
				}

				unsigned int fixed_priority_switch_arbitration( const int dir, tlm::tlm_generic_payload** trans_ptr_ptr )
				{
					for(unsigned int i = 0; i < NUM_DIR; ++i)
					{
						if((*trans_ptr_ptr = peq_[i][dir]->get_next_transaction()) != 0)
						{ 
							ROUTER_DEBUG(
							std::cout 
							<< std::setw(12) << sc_core::sc_time_stamp() 
							<< std::setw(8) << " thr " << name() << "(" << row_ << "," << col_ << ") " 
							<< dir << " got next transcact from " << i << std::endl;
							);
							return i; 
						}
					}
				}


				/*
				 * Route the packet. i.e. decide which outport it is
				 * heading. Simple deterministic, algorithmic dimension-order
				 * routing. Guarantees in-order delivery for packets.
				 */
				unsigned int yx_routing(unsigned long int rank_id)
				{
					unsigned long int rowPart = rank_id / n_cols_;
					unsigned long int colPart = rank_id - row_*n_cols_;
          if (rank_id == 16) //external memory
          {
					  rowPart = 0 / n_cols_; //external memory is connected at router 0's North
					  colPart = 0 - row_*n_cols_;
          }
          else 
          {
					  rowPart = rank_id / n_cols_;
					  colPart = rank_id - row_*n_cols_;
          }
        

					if(row_ == rowPart && col_ == colPart)
					{
            if (rank_id == 16) //external memory
              return N;
            else
						  return PE; 
					}
					else if(row_ < rowPart)
					{
						return S; 
					}
					else if(row_ > rowPart)
					{
						return N; 
					}
					else if(col_ < colPart)
					{
						return E; 
					}
					else
					{
						return W; 
					}
				}



				// Member variables
      private:
				unsigned int row_;
				unsigned int col_;
				sc_core::sc_time cycleTime_;
				int n_cols_;
        router_type type_;

				sc_core::sc_event txCompleteEvent_[NUM_DIR];
				tlm_utils::peq_with_get<tlm::tlm_generic_payload>* peq_[NUM_DIR][NUM_DIR];
				static const char dir_name[NUM_DIR];

		};
	}
}

template<unsigned int data_width_g>
const char asebt::mesh_2d_sc_tlm_1::TlmMeshRouter<data_width_g>::dir_name[] = {'N', 'W', 'S', 'E', 'P'};


#endif


// Local Variables:
// mode: c++
// c-file-style: "ellemtel"
// c-basic-offset: 3
// End:

