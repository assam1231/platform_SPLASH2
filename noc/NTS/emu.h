#ifndef EMU_H
#define EMU_H

enum PortType {tx=0,rx=1};
enum DIR {IP=0,N=1,E=2,S=3,W=4,EMPTY=5};
enum STATE {Idle=0,Busy=1};
enum Arbitration {FIFO=0,LRU=1,ROUND_ROBIN=2,RANDOM=3,FIXED=4};
//#define BandWidth 4 //PE and Switch BandWidth=32(bits)=4(bytes)
#define FIFO_size 2
#define WAIT_WINDOW_SIZE 40
//#define D_DEBUG
//#define D_DEBUG1
//#define D_DEBUG_SW

//#define VERIFY_LOADING
//#define VERIFY_WINDOW
//#define VERIFY_WINDOW1
//#define VERIFY_PKT

#endif
