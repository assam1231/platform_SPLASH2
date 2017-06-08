#ifndef TARGETNIDEBUG_H

#define TARGETNIDEBUG_H

//Usage: TARGET_NI_DEBUG( std::cerr<< "message" << std::endl; );
//
#ifdef EN_TARGET_NI_DEBUG
#  define TARGET_NI_DEBUG(x) do { x } while (0)
#else
#  define TARGET_NI_DEBUG(x) do {} while (0)
#endif

#ifdef EN_CROSSBAR_DEBUG
#  define CROSSBAR_DEBUG(x) do { x } while (0)
#else
#  define CROSSBAR_DEBUG(x) do {} while (0)
#endif

#ifdef EN_TRACE
#  define TRACE(x) do { x } while (0)
#else
#  define TRACE(x) do {} while (0)
#endif

#endif /* end of include guard: TARGETNIDEBUG_H */
