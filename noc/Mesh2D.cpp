#include "Mesh2D.h"
#include "pe_mmap.h"

namespace ispa
{

/** 
 * Constructor
 */
template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g>
Mesh2D<data_width_g, rows_g, cols_g>::
Mesh2D(sc_core::sc_module_name name, sc_core::sc_time cycle_time )
: sc_core::sc_module(name)
, cycleTime_(cycle_time) // 20ns = 50 MHz
, HEADER_LENGTH_IN_BYTE( data_width_g/8 ) 
{
  // New interfaces
	for(unsigned int row = 0; row < rows_g; ++row)
	{
		for(unsigned int col = 0; col < cols_g; ++col)
		{
			unsigned int mesh_index = row * cols_g + col; // runnning index number, 0,1,2...
      
      initNIs[mesh_index]  = new InitNI<data_width_g>
          (nocInitIF_name(row, col).c_str(), mesh_index, &m_addr_map, cycleTime_);
      targetNIs[mesh_index]= new TargetNI<data_width_g>
          (nocTargetIF_name(row, col).c_str(), mesh_index, &m_addr_map, cycleTime_);
        }
    }


#if NOC == TG2
	for(unsigned int row = 0; row < rows_g; ++row)
	{
		for(unsigned int col = 0; col < cols_g; ++col)
		{
			unsigned int mesh_index = row * cols_g + col; // runnning index number, 0,1,2...

			// New routers
			req_routers_[mesh_index] = new TlmMeshRouter<data_width_g>
          (req_router_name(row, col).c_str(), row, col, rows_g, cols_g, TlmMeshRouter<data_width_g>::req, (cycleTime_) );
			rsp_routers_[mesh_index] = new TlmMeshRouter<data_width_g> 
          (rsp_router_name(row, col).c_str(), row, col, rows_g, cols_g, TlmMeshRouter<data_width_g>::rsp, (cycleTime_) );

      // Connect interfaces with routers
      //   NI -> Router
      initNIs[mesh_index]->req_init_.bind(*(req_routers_[mesh_index]->targetSockets[PE]));
      targetNIs[mesh_index]->rsp_init_.bind(*(rsp_routers_[mesh_index]->targetSockets[PE]));

      //  Routests -> NI
			(*(rsp_routers_[mesh_index]->initSockets[PE])).bind(initNIs[mesh_index]->rsp_target_);
			(*(req_routers_[mesh_index]->initSockets[PE])).bind(targetNIs[mesh_index]->req_target_);

			// Connect the routers' W <-> E ports
			if(col > 0)
			{
				unsigned int t = row * cols_g + col - 1;
				(*req_routers_[mesh_index]->initSockets[W]).bind(*req_routers_[t]->targetSockets[E]);
				(*req_routers_[t]->initSockets[E]).bind(*req_routers_[mesh_index]->targetSockets[W]);
				(*rsp_routers_[mesh_index]->initSockets[W]).bind(*rsp_routers_[t]->targetSockets[E]);
				(*rsp_routers_[t]->initSockets[E]).bind(*rsp_routers_[mesh_index]->targetSockets[W]);
			}

			// Connect the routers' N <-> S ports
			if(row > 0)
			{
				unsigned int t = (row-1) * cols_g + col;
				(*req_routers_[mesh_index]->initSockets[N]).bind(*req_routers_[t]->targetSockets[S]);
				(*req_routers_[t]->initSockets[S]).bind(*req_routers_[mesh_index]->targetSockets[N]);
				(*rsp_routers_[mesh_index]->initSockets[N]).bind(*rsp_routers_[t]->targetSockets[S]);
				(*rsp_routers_[t]->initSockets[S]).bind(*rsp_routers_[mesh_index]->targetSockets[N]);
			}

		}
	}	 	 

  // New network interface for external memeory
  ext_mem_ni = new TargetNI<data_width_g> ("ExtMemNI", 16, &m_addr_map, cycleTime_);
  // Connect with router 0's North port
  ext_mem_ni->rsp_init_.bind(*(rsp_routers_[0]->targetSockets[N]));
  (*(req_routers_[0]->initSockets[N])).bind(ext_mem_ni->req_target_);

#elif NOC == NTS
  //New simulator
  simulator = new Simulator<data_width_g, rows_g, cols_g> ("simulator");

	for(unsigned int row = 0; row < rows_g; ++row)
	{
		for(unsigned int col = 0; col < cols_g; ++col)
		{
      unsigned int mesh_index = row * cols_g + col; // runnning index number, 0,1,2...
      // Connect interfaces with the simulator
      //   NI -> Simulator
      initNIs[mesh_index]->req_init_.bind(*(simulator->req_target[mesh_index]));
      targetNIs[mesh_index]->rsp_init_.bind(*(simulator->rsp_target[mesh_index]));

      //   Simulator -> NI
      (*(simulator->rsp_init[mesh_index])).bind(initNIs[mesh_index]->rsp_target_);
      (*(simulator->req_init[mesh_index])).bind(targetNIs[mesh_index]->req_target_);
    }
  }

#ifdef NTS_4_external_memory
  // New network interface for external memeory
  ext_mem_ni[0] = new TargetNI<data_width_g> ("ExtMemNI0", 16, &m_addr_map, cycleTime_);
  ext_mem_ni[1] = new TargetNI<data_width_g> ("ExtMemNI1", 17, &m_addr_map, cycleTime_);
  ext_mem_ni[2] = new TargetNI<data_width_g> ("ExtMemNI2", 18, &m_addr_map, cycleTime_);
  ext_mem_ni[3] = new TargetNI<data_width_g> ("ExtMemNI3", 19, &m_addr_map, cycleTime_);
  
  // Connect with router memory port by along
  ext_mem_ni[0]->rsp_init_.bind(*(simulator->rsp_target[16]));
  (*(simulator->req_init[16])).bind(ext_mem_ni[0]->req_target_);
  ext_mem_ni[1]->rsp_init_.bind(*(simulator->rsp_target[17]));
  (*(simulator->req_init[17])).bind(ext_mem_ni[1]->req_target_);
  ext_mem_ni[2]->rsp_init_.bind(*(simulator->rsp_target[18]));
  (*(simulator->req_init[18])).bind(ext_mem_ni[2]->req_target_);
  ext_mem_ni[3]->rsp_init_.bind(*(simulator->rsp_target[19]));
  (*(simulator->req_init[19])).bind(ext_mem_ni[3]->req_target_);
#else  
	ext_mem_ni = new TargetNI<data_width_g> ("ExtMemNI0", 16, &m_addr_map, cycleTime_);
	ext_mem_ni->rsp_init_.bind(*(simulator->rsp_target[16]));
	(*(simulator->req_init[16])).bind(ext_mem_ni->req_target_);
	
#endif

#endif

	set_memory_map();
}


//* Destructor
template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g>
Mesh2D<data_width_g, rows_g, cols_g>::
~Mesh2D()
{
	for(unsigned int i = 0; i < rows_g * cols_g; ++i)
	{
#if NOC == TG2
		delete req_routers_[i]; req_routers_[i] = 0;
		delete rsp_routers_[i]; rsp_routers_[i] = 0;
#endif
		delete initNIs[i];  initNIs[i]  = 0;
		delete targetNIs[i]; targetNIs[i] = 0;
	}
#if NOC == NTS
    delete simulator;
#endif
}

/* Transmitting funcitonality */
template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g>
void 
Mesh2D<data_width_g, rows_g, cols_g>::
set_memory_map()
{
  using pe_global_mmap::GLOBAL_VIEW_MEMORY_SIZE_PER_CORE;
  using pe_global_mmap::GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN;
  using pe_global_mmap::EXTERNAL_MEMORY_SIZE;
  using pe_global_mmap::EXTERNAL_MEMORY_SIZE_PER_ONE;

	for(unsigned int row = 0; row < rows_g; ++row)
	{
		for(unsigned int col = 0; col < cols_g; ++col)
		{
			unsigned int i = row * cols_g + col; // runnning index number, 0,1,2...
			//tshsu: hard code to define memory map
			m_addr_map.insert(
				i * GLOBAL_VIEW_MEMORY_SIZE_PER_CORE, 
				i * GLOBAL_VIEW_MEMORY_SIZE_PER_CORE+ (GLOBAL_VIEW_MEMORY_SIZE_PER_CORE-1),
				i);
		}
	}
  //external memory
  /*m_addr_map.insert(
      GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN,
      GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN + EXTERNAL_MEMORY_SIZE-1,
      16);*/


#ifdef NTS_4_external_memory 
	//4 external memory by along
	for(unsigned int i = 0 ; i < 4 ; i++){
		m_addr_map.insert(
		GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN + i*EXTERNAL_MEMORY_SIZE_PER_ONE,
		GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN + ((1+i)*EXTERNAL_MEMORY_SIZE_PER_ONE)-1,
		16+i);
	}
#else
	m_addr_map.insert(
      GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN,
      GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN + EXTERNAL_MEMORY_SIZE-1,
      16);
#endif
           
}


template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g>
std::string 
Mesh2D<data_width_g, rows_g, cols_g>::
req_router_name (const int row, const int col)
{
  std::ostringstream oss;
  oss << "req_router_r" << row << "_c" << col;
  return oss.str();
}

template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g>
std::string 
Mesh2D<data_width_g, rows_g, cols_g>::
rsp_router_name (const int row, const int col)
{
  std::ostringstream oss;
  oss << "rsp_router_r" << row << "_c" << col;
  return oss.str();
}

template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g>
std::string 
Mesh2D<data_width_g, rows_g, cols_g>::
nocInitIF_name (const int row, const int col)
{
  std::ostringstream oss;
  oss << "nocInitIF_r" << row << "_c" << col;
  return oss.str();
}

template<unsigned int data_width_g, unsigned int rows_g, unsigned int cols_g>
std::string 
Mesh2D<data_width_g, rows_g, cols_g>::
nocTargetIF_name (const int row, const int col)
{
  std::ostringstream oss;
  oss << "nocTargetIF_r" << row << "_c" << col;
  return oss.str();
}

template class Mesh2D<32, 4, 4>;

}//end of namespace ispa
