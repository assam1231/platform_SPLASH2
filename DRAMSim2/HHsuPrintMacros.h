#ifndef HHSU_PRINT_MACROS_H
#define HHSU_PRINT_MACROS_H

#define NO_OUTPUT
//#define USE_DEBUG
#define print_4_phase

#ifdef print_4_phase
	#define CCWU_PRINT(str) std::cout << str << std::endl ;
#else
	#define CCWU_RPINT(str) ;
#endif




#ifdef NO_OUTPUT
  #define HHSU_PRINT(str) ;
  #define HHSU_PRINTN(str) ;
#else
	#define HHSU_PRINT(str)  std::cout<< str <<std::endl;
	#define HHSU_PRINTN(str)  std::cout<< str;
#endif

#ifdef USE_DEBUG
  #define HHSU_DEBUG(str) std::cout<< str << std::endl;
  #define HHSU_DEBUGN(str) std::cout<< str;
#else
  #define HHSU_DEBUG(str) ;
  #define HHSU_DEBUGN(str) ;
#endif
  
#define HHSU_ERROR(str) std::cerr<< str << std::endl;

#endif



#ifdef NO_OUTPUT
	#undef DEBUG
	#undef DEBUGN
	#define DEBUG(str) ;
	#define DEBUGN(str) ;
	#define PRINT(str) ;
	#define PRINTN(str) ;
#else
	#ifdef LOG_OUTPUT
		#define PRINT(str)  { dramsim_log <<str<<std::endl; }
		#define PRINTN(str) { dramsim_log <<str; }
	#else
		#define PRINT(str)  if(SHOW_SIM_OUTPUT) { std::cout <<str<<std::endl; }
		#define PRINTN(str) if(SHOW_SIM_OUTPUT) { std::cout <<str; }
	#endif
#endif
