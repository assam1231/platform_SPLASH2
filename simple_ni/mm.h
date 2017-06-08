#ifndef MM_H
#define MM_H

//#define MM_DEBUG

#include "tlm.h"

class mm: public tlm::tlm_mm_interface
{
  typedef tlm::tlm_generic_payload gp_t;

public:
  mm() : free_list(0), empties(0)
#ifdef MM_DEBUG
  , count(0)
  , new_cnt(0)
#endif
  {}

  gp_t* allocate();
  void  free(gp_t* trans);

private:
  struct access
  {
    gp_t* trans;
    access* next;
    access* prev;
  };

  access* free_list;
  access* empties;

#ifdef MM_DEBUG
  int     count;
  int     new_cnt;
#endif
};

#endif /* end of include guard: MM_H */
