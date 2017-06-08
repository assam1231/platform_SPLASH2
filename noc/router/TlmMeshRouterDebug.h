#ifndef TLMMESHROUTERDEBUG_H

#define TLMMESHROUTERDEBUG_H


//Usage: ROUTER_DEBUG( std::cerr<< "message" << std::endl; );

#ifdef EN_ROUTER_DEBUG
#  define ROUTER_DEBUG(x) do { x } while (0)
#else
#  define ROUTER_DEBUG(x) do {} while (0)
#endif

#endif /* end of include guard: TLMMESHROUTERDEBUG_H */
