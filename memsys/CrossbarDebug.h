#ifndef CROSSBARDEBUG_H
#define CROSSBARDEBUG_H

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

#ifdef EN_MEM_TRACE
#define MEM_TRACE(x) do { x } while (0)
#else
#define MEM_TRACE(x) do {} while (0)
#endif

#endif
