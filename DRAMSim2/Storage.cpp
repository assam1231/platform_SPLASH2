#include <stdlib.h>
#include "Storage.h"
#include "SystemConfiguration.h"
#include "HHsuPrintMacros.h"


void Storage::read(sc_dt::uint64 address, unsigned char* data_ptr)
{

  //align address
  sc_dt::uint64 aligned_addr;
  unsigned  byteOffsetWidth = dramsim_log2((JEDEC_DATA_BUS_BITS/8));
  aligned_addr = address >> byteOffsetWidth;
  
  //JEDEC_DATA_BUS_BITS = 64;
  //BL = 4
  
  //find corresponding space and copy data to data_ptr
  unsigned transactionSize = (JEDEC_DATA_BUS_BITS/8)*BL;
  storage_mem_t::iterator it;
  it = storage_mem.find(aligned_addr);
  unsigned char *mem_space;
  
  if (it == storage_mem.end()) {
    mem_space = reinterpret_cast<unsigned char*> (malloc(transactionSize));
    storage_mem[aligned_addr] = mem_space;
	//printf("123 %x\n",mem_space);
  }
  else {
    mem_space = it->second;
  }
  
  it = storage_mem.find(aligned_addr);

  
  if (it != storage_mem.end())
  {
    memcpy(data_ptr, it->second, transactionSize);
  }
  else 
  {
    HHSU_ERROR("Cannot find address in storage space")
    exit(1);
  }
 
	
}

void Storage::write(const sc_dt::uint64 address, const unsigned char* data_ptr)
{

  //align address
  sc_dt::uint64 aligned_addr;
  unsigned  byteOffsetWidth = dramsim_log2((JEDEC_DATA_BUS_BITS/8));
  aligned_addr = address >> byteOffsetWidth;

  //check if the storage space has already created,
  //if not, new a block of memory space for it.
  unsigned transactionSize = (JEDEC_DATA_BUS_BITS/8)*BL;
  storage_mem_t::iterator it;
  it = storage_mem.find(aligned_addr);
  unsigned char *mem_space;
  if (it == storage_mem.end()) {
    mem_space = reinterpret_cast<unsigned char*> (malloc(transactionSize));
    storage_mem[aligned_addr] = mem_space;
  }
  else {
    mem_space = it->second;
  }

  //memcopy the data to the corresponding space
  memcpy(mem_space, data_ptr, transactionSize);

  //TODO: Add data mask function

  
}

unsigned Storage::dramsim_log2(unsigned value)
{
  unsigned logbase2 = 0;
  unsigned orig = value;
  value>>=1;
  while (value>0)
  {
    value >>= 1;
    logbase2++;
  }
  if ((unsigned)1<<logbase2<orig)logbase2++;
  return logbase2;
}
