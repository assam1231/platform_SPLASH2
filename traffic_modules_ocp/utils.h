#ifndef TS_UTILS_H_
#define TS_UTILS_H_

namespace tlm {
class tlm_generic_payload;
}

void display_transaction( const char * name, const tlm::tlm_generic_payload & trans );

#endif
