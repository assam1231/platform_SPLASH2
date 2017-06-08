#include "mm.h"
#include <iostream>

mm::gp_t* mm::allocate()
{
#ifdef MM_DEBUG
  std::cout << "----------------------------- Called allocate(), #trans = " << ++count << std::endl;
#endif
  gp_t* ptr;
  if (free_list)
  {
    ptr = free_list->trans;
    empties = free_list;
    free_list = free_list->next;
  }
  else
  {
    ptr = new gp_t(this);
#ifdef MM_DEBUG
    std::cout << "New gp, new_cnt:" << ++new_cnt << "\n";
#endif
  }
  ptr->acquire();
  return ptr;
}

void mm::free(gp_t* trans)
{
#ifdef MM_DEBUG
  std::cout << "----------------------------- Called free(), #trans = " << --count << std::endl;
#endif
  if (!empties)
  {
    empties = new access;
    empties->next = free_list;
    empties->prev = 0;
    if (free_list)
      free_list->prev = empties;
  }
  free_list = empties;
  free_list->trans = trans;
  empties = free_list->prev;
}

