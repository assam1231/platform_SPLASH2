#ifndef STORAGE_H
#define STORAGE_H

#include <map>
#include <systemc.h>

class Storage
{
public:
  void read(sc_dt::uint64 address, unsigned char* data_ptr);
  void write(const sc_dt::uint64 address, const unsigned char* data_ptr);
  
private:
  typedef  std::map<sc_dt::uint64, unsigned char*> storage_mem_t;
  storage_mem_t storage_mem;

  unsigned dramsim_log2(unsigned);

};
#endif
