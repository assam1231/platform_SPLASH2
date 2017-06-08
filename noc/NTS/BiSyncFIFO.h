#ifndef BISYNCFIFO_H
#define BISYNCFIFO_H

#include <iostream>
using namespace std;
//#define FIFO_size 2
//BiSyncFIFO behavior
//FIFO input frequence1 to FIFO output frequence2 have delay overhead(ns).
//Input clock + three times output positive trigger(not 3 cycles)
//is delay overhead
//Return output time base on input time.
long double Through_BiSyncFIFO(long double intput_time,long double input_clk,long double output_clk);

#endif
