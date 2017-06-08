
#ifndef NOCPAYLOADEXTENSION_H

#define NOCPAYLOADEXTENSION_H
#include <systemc>
#include <tlm.h>

struct ip_trans_ext: public tlm::tlm_extension<ip_trans_ext>
{
  ip_trans_ext(tlm::tlm_generic_payload *ori): ptr(ori){}

  ip_trans_ext(tlm::tlm_generic_payload &ori): ptr(&ori){}

  virtual tlm_extension_base* clone() const {
    return new ip_trans_ext(this->ptr);
  }

  virtual void copy_from(tlm::tlm_extension_base const &ext) {
    ptr = static_cast<ip_trans_ext const&>(ext).ptr;
  }

  tlm::tlm_generic_payload *ptr;
};

struct ori_address: public tlm::tlm_extension<ori_address>
{
  ori_address(sc_dt::uint64 ori_addr): value(ori_addr){}

  virtual tlm_extension_base* clone() const {
    return new ori_address(this->value);
  }

  virtual void copy_from(tlm_extension_base const &ext) {
    value = static_cast<ori_address const&>(ext).value;
  }

  sc_dt::uint64 value;
};

struct source_id_ext: public tlm::tlm_extension<source_id_ext>
{
  source_id_ext(unsigned int s_id): value(s_id){}

  virtual tlm_extension_base* clone() const {
    return new source_id_ext(this->value);
  }

  virtual void copy_from(tlm_extension_base const &ext) {
    value = static_cast<source_id_ext const&>(ext).value;
  }

  unsigned int value;
};


//Helper functions

namespace NocPayloadExt
{
unsigned int get_source_id(tlm::tlm_generic_payload *noc_trans);

unsigned int get_source_id(tlm::tlm_generic_payload &noc_trans);

tlm::tlm_generic_payload* get_ip_trans(tlm::tlm_generic_payload *noc_trans);

tlm::tlm_generic_payload* get_ip_trans(tlm::tlm_generic_payload &noc_trans);

void set_source_id(tlm::tlm_generic_payload *noc_trans, unsigned int source_id);

void set_ip_trans(tlm::tlm_generic_payload *noc_trans, tlm::tlm_generic_payload *ip_trans);

}



#endif /* end of include guard: NOCPAYLOADEXTENSION_H */
