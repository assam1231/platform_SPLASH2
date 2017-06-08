#include "NocPayloadExtension.h"

//Helper functions
namespace NocPayloadExt {

unsigned int get_source_id(tlm::tlm_generic_payload *noc_trans)
{
  source_id_ext *source_id;
  noc_trans->get_extension( source_id );
  assert(source_id != NULL);
  return source_id->value;
}

unsigned int get_source_id(tlm::tlm_generic_payload &noc_trans)
{
  source_id_ext *source_id;
  noc_trans.get_extension( source_id );
  assert(source_id != NULL);
  return source_id->value;
}

tlm::tlm_generic_payload* get_ip_trans(tlm::tlm_generic_payload *noc_trans)
{
  ip_trans_ext *ip_trans;
  noc_trans->get_extension( ip_trans );
  assert(ip_trans!= NULL);
  return ip_trans->ptr;
}

tlm::tlm_generic_payload* get_ip_trans(tlm::tlm_generic_payload &noc_trans)
{
  ip_trans_ext *ip_trans;
  noc_trans.get_extension( ip_trans );
  assert(ip_trans!= NULL);
  return ip_trans->ptr;
}

void set_source_id(tlm::tlm_generic_payload *noc_trans, unsigned int source_id)
{
    noc_trans->set_extension( new source_id_ext(source_id) );
}

void set_ip_trans(tlm::tlm_generic_payload *noc_trans, tlm::tlm_generic_payload *ip_trans)
{
    noc_trans->set_extension( new ip_trans_ext(ip_trans) );
}

}
