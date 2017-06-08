/*
 * PktPayload.h
 * Copyright (C) 2014 m101061536 <m101061536@ws36>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef PKTPAYLOAD_H
#define PKTPAYLOAD_H

struct PktPayload {
  PktPayload(unsigned char* payload, unsigned int s, unsigned int d, long double send, unsigned int size, int r):
    network_payload( payload ), src( s ), dst( d ), send_time( send ), data_size( size ), req_rsp( r ), finish_time ( 0 ), pkt_ptr ( NULL ){ }
  unsigned char* network_payload;
  unsigned int src;
  unsigned int dst;
	long double send_time;
	unsigned int data_size;
	int req_rsp;
	long double finish_time;
	void* pkt_ptr;
};

#endif /* !PKTPAYLOAD_H */

