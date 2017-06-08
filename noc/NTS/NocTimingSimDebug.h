#ifndef NOCTIMINGSIMDEBUG_H

#define NOCTIMINGSIMDEBUG_H


//Usage: NTS_DEBUG( std::cerr<< "message" << std::endl; );

#ifdef EN_NTS_DEBUG
#  define NTS_DEBUG(x) do { x } while (0)
#else
#  define NTS_DEBUG(x) do {} while (0)
#endif

#endif /* end of include guard: NOCTIMINGSIMDEBUG_H */

