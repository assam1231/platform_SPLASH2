#ifndef _PE_MMAP_H_
#define _PE_MMAP_H_

namespace pe_local_mmap {

static const unsigned int MAIN_BEGIN	= 0;					//0x0000_0000~0x0200_0000 (mask = 0x01ff_ffff)
static const unsigned int MAIN_SIZE		= 0x02000000u;	//32 MBytes, 48MBytes
static const unsigned int MAIN_END		= MAIN_BEGIN + MAIN_SIZE - 1;

//wschen add memory2 for ctro.0 20121125
static const unsigned int SUB_MEM_BEGIN	    = 0xffff0000u;					//0x0000_0000~0x0200_0000 (mask = 0x01ff_ffff)
static const unsigned int SUB_MEM_SIZE		= 0x10000u;	//32 MBytes, 48MBytes
static const unsigned int SUB_MEM_END		= SUB_MEM_BEGIN + SUB_MEM_SIZE -1;


static const unsigned int RESV_BEGIN	= 0x02000000u;	//0x0200_0000~0x0200_007f (mask = 0x7f)
//static const unsigned int RESV_BEGIN	= 0x03000000u;	//0x0200_0000~0x0200_007f (mask = 0x7f)
static const unsigned int RESV_SIZE		= 0x00000080u;	//128 bytes, 32 words
static const unsigned int RESV_END		= RESV_BEGIN + RESV_SIZE - 1;

// seem to not use by along
/*
static const unsigned int TARGET_BEGIN	= 0x02000000u;	//0x0200_0080~0x0200_00ff (mask = 0x7f)
//static const unsigned int TARGET_BEGIN	= 0x03000080u;	//0x0200_0080~0x0200_00ff (mask = 0x7f)
static const unsigned int TARGET_SIZE	= 0x00000100u;	//512 bytes, 64 words
static const unsigned int TARGET_END		= TARGET_BEGIN + TARGET_SIZE - 1;
*/

//static const unsigned int PE2PE_BEGIN	= 0x02000100u;	//0x0200_0100~0x0200_01ff (mask = 0xff )
//static const unsigned int PE2PE_SIZE	= 0x00000100u;	//256 bytes
/*
static const unsigned int PE2PE_BEGIN	= 0x20000000u;	//0x0200_0100~0x0200_01ff (mask = 0xff )
static const unsigned int PE2PE_SIZE	= 0x00000100u;	//256 bytes
static const unsigned int PE2PE_END		= PE2PE_BEGIN + PE2PE_SIZE - 1;
*/

//20121019 add communication
static const unsigned int COMM_BEGIN = 0x20000000u;  //0x2000_0100~0x2000_01ff (mask = 0xff )
static const unsigned int COMM_SIZE  = 0x00000100u;  //256 bytes
static const unsigned int COMM_END   = COMM_BEGIN + COMM_SIZE - 1;

//static const unsigned int	DMA_BEGIN		= 0x02000400u;	//0x0200_0400~0x0200_07ff (mask = 0x3ff)
//static const unsigned int DMA_SIZE		= 0x00000400u;	//1024 bytes
static const unsigned int	DMA_BEGIN		= 0x30000000u;	//0x0200_0400~0x0200_07ff (mask = 0x3ff)
static const unsigned int DMA_SIZE		= 0x00000400u;	//1024 bytes
static const unsigned int DMA_END			= DMA_BEGIN + DMA_SIZE - 1;

//static const unsigned int EXTERNAL_MEMORY_BEGIN = 0x02010000;
static const unsigned int EXTERNAL_MEMORY_BEGIN = 0x40000000;

static const unsigned int RANK = (RESV_BEGIN);
//! jylai, for file output
static const unsigned int OR1K_OUT_FILE_BASE = (RESV_BEGIN + 0x04);
static const unsigned int OR1K_OUT_FILE_START = (RESV_BEGIN + 0x08);
static const unsigned int OR1K_OUT_FILE_END = (RESV_BEGIN + 0x0c);

//for clock setting
static const unsigned int BASE_CLOCK_PERIOD = (RESV_BEGIN + 0x10);
static const unsigned int CLOCK_PERIOD = (RESV_BEGIN + 0x14);
static const unsigned int CLOCK_DIVISOR = (RESV_BEGIN + 0x18);
//for time display
static const unsigned int PRINT_CYCLE_COUNT = (RESV_BEGIN + 0x1c);
static const unsigned int PRINT_INSTR_COUNT = (RESV_BEGIN + 0x1d);
static const unsigned int PRINT_TIME = (RESV_BEGIN + 0x1e);

//to get cycle/time
static const unsigned int UPPER_INSTR_COUNT = (RESV_BEGIN + 0x20);
static const unsigned int LOWER_INSTR_COUNT = (RESV_BEGIN + 0x24);
static const unsigned int UPPER_TIME_STAMP  = (RESV_BEGIN + 0x28);
static const unsigned int LOWER_TIME_STAMP  = (RESV_BEGIN + 0x2c);

//user timers
static const unsigned int USER_TIMER_START = (RESV_BEGIN + 0x40);
static const unsigned int USER_TIMER_STOP  = (RESV_BEGIN + 0x41);
static const unsigned int USER_TIMER_RESET = (RESV_BEGIN + 0x42);
static const unsigned int USER_TIMER_PRINT = (RESV_BEGIN + 0x43);
}

namespace pe_global_mmap {

static const unsigned int GLOBAL_VIEW_MEMORY_SIZE_PER_CORE = 0x00020000u;// this is for ACK in blocking

static const unsigned int GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN = 0x10000000u;
static const unsigned int EXTERNAL_MEMORY_SIZE = 0x10000000u; //size is no difference in global view or local view


// one of four external memory size by along
static const unsigned int EXTERNAL_MEMORY_SIZE_PER_ONE = 0x04000000;
static const unsigned int EXTERNAL_MEMORY1_BEGIN = GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN;
static const unsigned int EXTERNAL_MEMORY2_BEGIN = GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN + EXTERNAL_MEMORY_SIZE_PER_ONE;
static const unsigned int EXTERNAL_MEMORY3_BEGIN = GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN + 2*EXTERNAL_MEMORY_SIZE_PER_ONE;
static const unsigned int EXTERNAL_MEMORY4_BEGIN = GLOBAL_VIEW_EXTERNAL_MEMORY_BEGIN + 3*EXTERNAL_MEMORY_SIZE_PER_ONE;


}
#endif
