///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// (c) Copyright OCP-IP 2008
// OCP-IP Confidential and Proprietary
//
//
//============================================================================
//      Project : OCP SLD WG
//       Author : Robert Guenzel (from TU of Braunschweig) for Greensocs Ltd.
//
//          $Id:
//
//  Description :  This file sets up all the extensions for OCP
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_tlm_extensions.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif
namespace OCPIP_VERSION{

enum mcmd_codes{
IDLE  = ocpip_legacy::OCP_MCMD_IDLE,
WR    = ocpip_legacy::OCP_MCMD_WR,
RD    = ocpip_legacy::OCP_MCMD_RD,
RDEX  = ocpip_legacy::OCP_MCMD_RDEX,
RDL   = ocpip_legacy::OCP_MCMD_RDL,
WRNP  = ocpip_legacy::OCP_MCMD_WRNP,
WRC   = ocpip_legacy::OCP_MCMD_WRC,
BCST  = ocpip_legacy::OCP_MCMD_BCST
};

enum burst_seqs{
INCR  = ocpip_legacy::OCP_MBURSTSEQ_INCR,
DFLT1 = ocpip_legacy::OCP_MBURSTSEQ_DFLT1,
WRAP  = ocpip_legacy::OCP_MBURSTSEQ_WRAP,
DFLT2 = ocpip_legacy::OCP_MBURSTSEQ_DFLT2,
XOR   = ocpip_legacy::OCP_MBURSTSEQ_XOR,
STRM  = ocpip_legacy::OCP_MBURSTSEQ_STRM,
UNKN  = ocpip_legacy::OCP_MBURSTSEQ_UNKN,
BLCK  = ocpip_legacy::OCP_MBURSTSEQ_BLCK
};

struct burst_seq_info{
  burst_seqs sequence;

  //height and stride are only used with sequence==BLCK
  // so to use them, the sequence extension must be used
  // so we can put them into the sequence extension
  unsigned int block_height;
  unsigned int block_stride;
  unsigned int blck_row_length_in_bytes;
  unsigned int unkn_dflt_bytes_per_address;
  sc_dt::uint64 xor_wrap_address;
  bool unkn_dflt_addresses_valid;
  std::vector<sc_dt::uint64> unkn_dflt_addresses;
};

inline std::ostream& operator<< (std::ostream & os, const burst_seq_info & seq){
  switch(seq.sequence){
    case INCR: os<<"INCR"; break;
    case DFLT1: os<<"DFLT1"; break;
    case WRAP: os<<"WRAP (address="<<seq.xor_wrap_address<<")"; break;
    case DFLT2: os<<"DFLT2"; break;
    case XOR: os<<"XOR (address="<<seq.xor_wrap_address<<")"; break;
    case STRM: os<<"STRM"; break;
    case UNKN: os<<"UNKN"; break;
    case BLCK: os<<"BLCK (height="<<seq.block_height<<", stride="<<seq.block_stride<<")"; break;
  }
  return os;
}

struct lock_object_base{
  virtual ~lock_object_base(){}
  virtual void atomic_txn_completed()=0;
  bool    lock_is_understood_by_slave;
  unsigned int number_of_txns;
};

///////////////////////////////////////////
// Timing annotation for TL2

struct tl2_master_timing_group {
    unsigned int RqSndI; // Request Send Interval
    unsigned int DSndI;  // Data Send Interval
    unsigned int RpAL;   // Response Accept Latency
};

struct tl2_slave_timing_group {
    unsigned int RqAL;   // Request Accept Latency
    unsigned int DAL;    // Data Accept Latency
    unsigned int RpSndI; // Response Send Interval
};

inline std::ostream& operator<< (std::ostream & os, const tl2_master_timing_group & tl2_MGrp)
{
    os << "Request Send Interval is "   << tl2_MGrp.RqSndI << std::endl;
    os << "RData Send Interval is "     << tl2_MGrp.DSndI  << std::endl;
    os << "Response Accept Latency is " << tl2_MGrp.RpAL   << std::endl;
  return os;
}

inline std::ostream& operator<< (std::ostream & os, const tl2_slave_timing_group & tl2_SGrp)
{
    os << "Request Accept Latency is "   << tl2_SGrp.RqAL    << std::endl;
    os << "Data Accept Latency is "      << tl2_SGrp.DAL     << std::endl;
    os << "Response Send Interval is "   << tl2_SGrp.RpSndI  << std::endl;
  return os;
}

enum tl2_timing_type {MASTER_TIMING, SLAVE_TIMING};

struct tl2_timing_group {
    tl2_master_timing_group   master_timing;
    tl2_slave_timing_group    slave_timing;

    tl2_timing_type type;
};

inline std::ostream& operator<< (std::ostream & os, const tl2_timing_group & tl2_time)
{
    os   << tl2_time.master_timing    << std::endl;
    os   << tl2_time.slave_timing     << std::endl;

  return os;
}

// Extension for word count in TL2
struct tl2_burst_word_count {
    unsigned int request_wc;
    unsigned int data_wc;
    unsigned int response_wc;
};

inline std::ostream& operator<< (std::ostream & os, const tl2_burst_word_count & burst_wc)
{
    os << "Word count for request phase is " << burst_wc.request_wc  << std::endl;
    os << "Word count for date phase is "    << burst_wc.data_wc     << std::endl;
    os << "Word count for response phase is "<< burst_wc.response_wc << std::endl;

  return os;
}







//can those be converted to data only? checking the ocpconfig
// will tell you whether you may access the extension or not
// (if two connected OCP modules agreed to use a certain extension
//  they agreed to set it for EVERY transaction...)

struct address_space : public infr::ocp_single_member_guarded_data<address_space, unsigned int>{};
struct atomic_length : public infr::ocp_single_member_guarded_data<atomic_length, unsigned int>{};
struct broadcast : public infr::ocp_guard_only_extension<broadcast>{};
struct burst_length : public infr::ocp_single_member_guarded_data<burst_length, unsigned int>{};
struct burst_sequence : public infr::ocp_single_member_guarded_data<burst_sequence, burst_seq_info>{};
struct conn_id : public infr::ocp_single_member_guarded_data<conn_id, unsigned int>{};
struct imprecise : public infr::ocp_guard_only_extension<imprecise>{};
struct lock : public infr::ocp_single_member_guarded_data<lock, lock_object_base*>{};
struct posted : public infr::ocp_guard_only_extension<posted>{};
struct semaphore : public infr::ocp_single_member_guarded_data<semaphore, bool>{};
struct srmd : public infr::ocp_guard_only_extension<srmd>{};
struct tag_id : public infr::ocp_single_member_guarded_data<tag_id, unsigned int>{};
struct cmd_thread_busy : public infr::ocp_single_member_data<cmd_thread_busy, unsigned int>{};
struct data_thread_busy : public infr::ocp_single_member_data<data_thread_busy, unsigned int>{};
struct resp_thread_busy : public infr::ocp_single_member_data<resp_thread_busy, unsigned int>{};
struct thread_id : public infr::ocp_single_member_guarded_data<thread_id, unsigned int>{};
struct tl2_timing : public infr::ocp_single_member_data<tl2_timing, tl2_timing_group>{};
struct word_count : public infr::ocp_single_member_guarded_data<word_count, tl2_burst_word_count>{};

struct tl1_id : public infr::ocp_single_member_data<tl1_id, bool>{};

/*
OCP_SINGLE_MEMBER_GUARDED_DATA(OCPIP_VERSION, atomic_length, unsigned int);
OCP_GUARD_ONLY_EXTENSION(OCPIP_VERSION, broadcast);
OCP_SINGLE_MEMBER_GUARDED_DATA(OCPIP_VERSION, burst_length, unsigned int);
OCP_SINGLE_MEMBER_GUARDED_DATA(OCPIP_VERSION, burst_sequence, burst_seq_info);
OCP_SINGLE_MEMBER_GUARDED_DATA(OCPIP_VERSION, conn_id, unsigned int);
OCP_GUARD_ONLY_EXTENSION(OCPIP_VERSION, imprecise);
OCP_SINGLE_MEMBER_GUARDED_DATA(OCPIP_VERSION, lock, lock_object_base*);
OCP_GUARD_ONLY_EXTENSION(OCPIP_VERSION, nonposted);
OCP_SINGLE_MEMBER_GUARDED_DATA(OCPIP_VERSION, semaphore, bool);
OCP_GUARD_ONLY_EXTENSION(OCPIP_VERSION, srmd);
OCP_SINGLE_MEMBER_GUARDED_DATA(OCPIP_VERSION, tag_id, unsigned int);
OCP_SINGLE_MEMBER_DATA(OCPIP_VERSION, thread_busy, thread_busy_update);
OCP_SINGLE_MEMBER_GUARDED_DATA(OCPIP_VERSION, thread_id, unsigned int);
OCP_SINGLE_MEMBER_DATA(OCPIP_VERSION, tl2_timing, tl2_timing_group);
OCP_SINGLE_MEMBER_GUARDED_DATA(OCPIP_VERSION, word_count, tl2_burst_word_count);
*/


DECLARE_EXTENDED_PHASE(CMD_THREAD_BUSY_CHANGE);
DECLARE_EXTENDED_PHASE(DATA_THREAD_BUSY_CHANGE);
DECLARE_EXTENDED_PHASE(RESP_THREAD_BUSY_CHANGE);
DECLARE_EXTENDED_PHASE(BEGIN_RESET);
DECLARE_EXTENDED_PHASE(END_RESET);
DECLARE_EXTENDED_PHASE(BEGIN_INTERRUPT);
DECLARE_EXTENDED_PHASE(END_INTERRUPT);
DECLARE_EXTENDED_PHASE(MFLAG_CHANGE);
DECLARE_EXTENDED_PHASE(SFLAG_CHANGE);
DECLARE_EXTENDED_PHASE(BEGIN_ERROR);
DECLARE_EXTENDED_PHASE(END_ERROR);
DECLARE_EXTENDED_PHASE(TL2_TIMING_CHANGE);

DECLARE_EXTENDED_PHASE(BEGIN_DATA);
DECLARE_EXTENDED_PHASE(END_DATA);

template<int I> static int kill_compiler_warnings() {
  return I + CMD_THREAD_BUSY_CHANGE + DATA_THREAD_BUSY_CHANGE + RESP_THREAD_BUSY_CHANGE +
    BEGIN_RESET + END_RESET + BEGIN_INTERRUPT + END_INTERRUPT + MFLAG_CHANGE + SFLAG_CHANGE +
    BEGIN_ERROR + END_ERROR + TL2_TIMING_CHANGE;
}

}
