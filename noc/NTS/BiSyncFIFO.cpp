#include "BiSyncFIFO.h"

//BiSyncFIFO Table.
//FIFO input frequence1 to FIFO output frequence2 must
//have delay overhead(ns).
//This delay overhead is experimental data from Ateris NTTP tool.
long double Through_BiSyncFIFO(long double input_time,long double input_clk,long double output_clk)
{
  long double output_time;
  int sim_cycle;
  //input data write into fifo at negtive input clock
  output_time = input_time + (input_clk/2);

  //first output positive clock trigger can get data
  sim_cycle = int (output_time / output_clk) + 1;

  //0.001ns because SystemC simulation resolution is 1ps(0.001ns)
  output_time = sim_cycle * output_clk /*+ 0.001*/;
  
  return output_time;
}

