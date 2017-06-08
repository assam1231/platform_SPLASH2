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
//  Description :  This file contains a class that is used by the monitor
//                 and backward compatibility adapters and helps
//                 converting legacy data structures into new structures
//                 and vice versa.
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_tl1_legacy_convertion_helper.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{

inline unsigned int calculate_ocp_address_offset(tlm::tlm_generic_payload& txn, const unsigned int bus_byte_width){
  return (txn.get_address() % bus_byte_width);
}

template <typename Ta>
inline unsigned int calculate_ocp_burst_aligned_INCR_address( const unsigned int words_in_txn
                                                            , const sc_dt::uint64 address
                                                            , const unsigned int bus_byte_width
                                                            , const unsigned int offset
                                                            , const unsigned int txn_data_length
                                                            , Ta& new_ocp_address)
{
  unsigned int min_pow_2=(1U)<<(ocpip_legacy::OcpIp::ceilLog2(words_in_txn)); //the minimal power of two number to transmit the data
  Ta addr_mask=((unsigned int)-1)<<(ocpip_legacy::OcpIp::ceilLog2(words_in_txn)+ocpip_legacy::OcpIp::ceilLog2(bus_byte_width));
  new_ocp_address=(address-offset) & addr_mask;
  
  while(new_ocp_address+(min_pow_2*bus_byte_width)<(address+txn_data_length)){
    min_pow_2<<=1; //blen was too small, increase
    addr_mask<<=1;
    new_ocp_address=address & addr_mask; //recalculate ocp address
  }
  return min_pow_2;
}

template <typename Ta>
inline unsigned int calculate_ocp_burst_length( tlm::tlm_generic_payload& txn
                                              , const unsigned int bus_byte_width
                                              , unsigned int offset
                                              , bool burst_aligned_INCR
                                              , Ta& new_ocp_address
                                              , burst_sequence* b_seq)
{
  if (b_seq){
    switch(b_seq->value.sequence){
      case BLCK:
        return (b_seq->value.blck_row_length_in_bytes+offset+bus_byte_width-1)/bus_byte_width;
      case UNKN:{
        unsigned int num_unkn_beat=(txn.get_data_length()+b_seq->value.unkn_dflt_bytes_per_address-1)/b_seq->value.unkn_dflt_bytes_per_address;
        return (b_seq->value.unkn_dflt_bytes_per_address+bus_byte_width-1)/bus_byte_width*num_unkn_beat;
      }
      case DFLT2:
        return (txn.get_data_length()+b_seq->value.unkn_dflt_bytes_per_address-1)/b_seq->value.unkn_dflt_bytes_per_address;
      default: break;
    }
  }
  if (txn.get_streaming_width()>=txn.get_data_length()) //incr and dflt1
  {
    unsigned int d_len=txn.get_data_length()+offset;
    unsigned int retVal=(d_len+(bus_byte_width-1))/bus_byte_width;
    
    if (burst_aligned_INCR){
      retVal=calculate_ocp_burst_aligned_INCR_address(retVal, txn.get_address(), bus_byte_width, offset, txn.get_data_length(), new_ocp_address);
    }
    return retVal; //how many words are in the data array?
  }
  else
  if((txn.get_streaming_width()+offset)<=bus_byte_width){ //real stream
    return (txn.get_data_length()+txn.get_streaming_width()-1)/txn.get_streaming_width(); //how many strm words are in the data array
  }
  else { //implicit blck
    assert(0 && "Streaming width + Offset exceeds data width.");
    exit(1);
  }
  return 0;
}


template <typename DataCl, typename OWNER, typename EXT_SUPPORT, unsigned int BUSWIDTH=DataCl::SizeCalc::bit_size>
//sizeof(typename DataCl::DataType)*8>
struct ocp_tl1_legacy_convertion_helper
  : public EXT_SUPPORT
{
public:  
  typedef ocp_tl1_legacy_convertion_helper<DataCl, OWNER, EXT_SUPPORT, BUSWIDTH> my_type;
  typedef EXT_SUPPORT ext_base;
  typedef EXT_SUPPORT ext_support_type;
  typedef typename DataCl::DataType	Td;
  typedef typename DataCl::AddrType	Ta;
  typedef ocpip_legacy::OCPRequestGrp<Td,Ta>      request_type;
  typedef ocpip_legacy::OCPDataHSGrp<Td>          datahs_type;
  typedef ocpip_legacy::OCPResponseGrp<Td>        response_type;
  typedef ocpip_legacy::ParamCl<DataCl>           paramcl_type;
  typedef sc_dt::uint64 (*ext_to_bit_cb)(tlm::tlm_generic_payload&); //signature with phase and phase count?
  typedef void (*bit_to_ext_cb)(sc_dt::uint64, tlm::tlm_generic_payload&);

  struct have_seen_it : public tlm_utils::instance_specific_extension<have_seen_it>{
    have_seen_it(): seq_calc(BUSWIDTH, sizeof(Ta)*8, 0,0), data_seq_calc(BUSWIDTH, sizeof(Ta)*8, 0,0){}
    request_type  req;
    datahs_type   dhs;
    response_type rsp;  
    unsigned int req_cnt, dhs_cnt, rsp_cnt, offset, dead_leading_words, blck_row_length_in_bytes, unkn_pseudo_b_len;
    std::vector<sc_dt::uint64>* p_addr_vector;
    ocpip_legacy::OcpIp::BurstSequence<Ta> seq_calc, data_seq_calc;
    bool unlocking_wr;
  };
private:
  ocp_extension_pool<have_seen_it> m_extPool;
  OWNER* m_owner;
  paramcl_type& (OWNER::*m_get_param_cb)();
  unsigned int m_byte_width;
  const unsigned int m_byte_enable_mask;
  std::string m_name;
  ext_to_bit_cb m_req_info_to_bit, m_mdata_info_to_bit, m_sdata_info_to_bit, m_resp_info_to_bit;
  bit_to_ext_cb m_req_info_to_ext, m_mdata_info_to_ext, m_sdata_info_to_ext, m_resp_info_to_ext;
  static unsigned int m_dummy_uint;
  static Td m_dummy_dt;

public:
  tlm_utils::instance_specific_extension_accessor acc;

  ocp_tl1_legacy_convertion_helper(const std::string& name, OWNER* param_provider, paramcl_type& (OWNER::*get_param_cb)())
    : //ext_base(BUSWIDTH)
     m_extPool(5)
    , m_owner(param_provider)
    , m_get_param_cb(get_param_cb)
    , m_byte_width((BUSWIDTH+7)>>3)
    , m_byte_enable_mask(((unsigned int)(-1))>>(sizeof(unsigned int)*8-m_byte_width))
    , m_name(name)
    , m_req_info_to_bit(0)
    , m_mdata_info_to_bit(0)
    , m_sdata_info_to_bit(0)
    , m_resp_info_to_bit(0)
    , m_req_info_to_ext(0)
    , m_mdata_info_to_ext(0)
    , m_sdata_info_to_ext(0)
    , m_resp_info_to_ext(0)
  {
  }

  ~ocp_tl1_legacy_convertion_helper(){
  }
  
#define GEN_EXT2BIT_CB_REG_FNS(type)\
  void assign_##type##_info_extension_to_bit_mask_cb(ext_to_bit_cb cb){m_##type##_info_to_bit=cb;} \
  void assign_##type##_info_bit_mask_to_extension_cb(bit_to_ext_cb cb){m_##type##_info_to_ext=cb;}
  
  GEN_EXT2BIT_CB_REG_FNS(req)
  GEN_EXT2BIT_CB_REG_FNS(mdata)
  GEN_EXT2BIT_CB_REG_FNS(sdata)
  GEN_EXT2BIT_CB_REG_FNS(resp)
#undef GEN_EXT2BIT_CB_REG_FNS

  std::string& name(){return m_name;}


  /*
  This function extracts a byte enable mask from a given byte enable array.
  Sometimes it is necessary to treat a section of the data array as if it
  was the data array itself. Hence we sometimes treat a given index as the
  zero index (so we did an index shift), and sometimes we cannot use
  the data length as THE data length, so we call it effective length.
  
  To fully understand what that means I strongly recommend to have a look
  at the use of this function.
  */
  void get_BE_from_txn(
      unsigned int& ByteEN //byte enable mask that will be filled
    , unsigned char* byte_enable_ptr //the pointer to the char array that represents the TLM 2.0 byte enable array
    , unsigned int chunk //the number of the word for which to extract the byte enable mask (first is zero)
    , unsigned int offset //the offset of the transaction address from the nearest lower word aligned address
    , bool is_last //true if the current chunk is the last one in the data array
    , unsigned int effective_length //the effective length of the current data array (just treat it as if it was the data length)
                                    //  see the use of this function to find out why it is called effective length
                                    //  (That is really a long story...)
    , unsigned int index_shift //the index of the first byte of the current word in the data array
    , unsigned int byte_enable_length //the byte enable length from the txn (just to do the modulo operation, as defined by TLM2.0)
    )
  {
    /*
    loop_min shall be set to the first index of the current chunk in the data array (yes. I mean the data array)
    loop_max shall be set to the last index of the current chunk in the data array (yes. I mean the data array)
    Let offset be N:
      - The first chunk (0) starts at index 0 and ends at byte_width-N, because the lowest N bytes of the chunk are not within the data array
      - All other chunks start at start at byte_width*chunk-N, because the first chunk did only occupy byte_widt-N bytes
      - The last chunk ends at the effective length (prevents to read bytes that are outside the array)
    */
    unsigned int loop_min=(chunk)?m_byte_width*chunk-offset : 0; 
    unsigned int loop_max=(is_last)? effective_length:
                          (chunk)  ? loop_min+m_byte_width : m_byte_width-offset;
    //we gotta have the line below because of imprecise bursts.
    // There the data length can be any number larger than the real number of transmitted bytes
    //  and if the caller identifies the last chunk, loop_max could be too large as it is computed using the data_length
    if ((loop_max-loop_min)>m_byte_width) loop_max=loop_min+m_byte_width; 
                          
    if (byte_enable_ptr){ //okay, we have explicit byte enables
      ByteEN=0; //we initialize the byte enable mask
      /*
      The loop below goes from the first to the last index of the current chunk in the data array.
      It first computes the position in the real data array (loop index+index shift) the applies the
      modulo operation according to the TLM2 standard.
      
      Depending on endianess, the mask is either built from left to right or right to left.
      */	
      if (tlm::get_host_endianness()==tlm::TLM_BIG_ENDIAN)
        for (unsigned int i=loop_min, mask=0x1<<(((chunk>0)?m_byte_width : m_byte_width-offset)-1); i<loop_max; i++, mask>>=1){
          if (byte_enable_ptr[(i+index_shift)%byte_enable_length]) ByteEN|=mask;
        }
      else
        for (unsigned int i=loop_min, mask=0x1<<((chunk>0)?0:offset); i<loop_max; i++, mask<<=1){
          if(byte_enable_ptr[(i+index_shift)%byte_enable_length])  ByteEN|=mask;
        }
    }
    else{ 
      //well, we do not have explicit byte enables, so we treat every byte that is in the
      // data array as enabled.
      //So we only gotta make sure, we disable the bytes that are missing at the start (offset)
      // and the bytes that are missing at the end 
      
      ByteEN=m_byte_enable_mask; //this thing will mask out the offset bytes
      unsigned int ByteEN2=m_byte_enable_mask; //this one will mask out the bytes that are missing at the end
      
      if (!chunk){ //first chunk, disable the offset bytes
        if (tlm::get_host_endianness()==tlm::TLM_BIG_ENDIAN)
          ByteEN>>=offset;
        else
          ByteEN<<=offset;
      }
      
      if (is_last) //last chunk, disable the missing bytes at the end
                   // since loop_min is zero for the first chunk we have to treat it specially
      {
        if (chunk)
          if (tlm::get_host_endianness()==tlm::TLM_BIG_ENDIAN)
            ByteEN2<<=((loop_min+m_byte_width)-loop_max);
          else
            ByteEN2>>=((loop_min+m_byte_width)-loop_max);
        else 
          if (tlm::get_host_endianness()==tlm::TLM_BIG_ENDIAN)
            ByteEN2<<=(m_byte_width-(offset+effective_length));
          else
            ByteEN2>>=(m_byte_width-(offset+effective_length));        
      } 
      ByteEN&=ByteEN2; //merge the two masks
    }
  }

  //that's much simpler, because it could never ever unaligned or modified in any other mean ways, because
  // it always originates in the legacy world, where everything is always word aligned
  void set_BE_into_txn(unsigned int ByteEN, tlm::tlm_generic_payload* txn, unsigned int chunk){
    if (tlm::get_host_endianness()==tlm::TLM_BIG_ENDIAN){
      unsigned int mask=0x1<<(m_byte_width-1);
      for (unsigned int i=0; i<m_byte_width; i++){
        if (ByteEN & mask) txn->get_byte_enable_ptr()[i+chunk*m_byte_width]=0xFF;
        else txn->get_byte_enable_ptr()[i+chunk*m_byte_width]=0x0;
        mask>>=1;
      }
    }
    else{
      unsigned int mask=0x1;
      for (unsigned int i=0; i<m_byte_width; i++){
        if (ByteEN & mask) {txn->get_byte_enable_ptr()[i+chunk*m_byte_width]=0xFF;}
        else txn->get_byte_enable_ptr()[i+chunk*m_byte_width]=0x0;
        mask<<=1;
      }
    }  
  }
  
  /*
  see get_BE_from_txn for a description of the arguments.
  The only different things are 'word' which is the word to get,
  and 'data_ptr' which points to the entry into the data array,
  which is not necessarily the data_ptr of the transaction.
  */
  #define CREATE_GET_WORD_FROM_TXN_FUNCTION(type) \
  void get_word_from_txn(type& word, \
                         unsigned char* data_ptr, \
                         unsigned int chunk, \
                         unsigned int offset, \
                         bool is_last, \
                         unsigned int effective_length)\
  { \
    if (chunk){ \
      unsigned int index=chunk*m_byte_width-offset; \
      unsigned int num_bytes=(is_last)? effective_length-index : m_byte_width; \
      if (num_bytes>m_byte_width) num_bytes=m_byte_width; \
      memcpy(&word, &data_ptr[index], num_bytes);\
    } \
    else {\
      unsigned int num_bytes=(is_last)? effective_length: m_byte_width-offset; \
      if (num_bytes>m_byte_width) num_bytes=m_byte_width; \
      memcpy(&word, data_ptr, num_bytes);\
      if (tlm::get_host_endianness()==tlm::TLM_BIG_ENDIAN) \
        word>>=(offset*8); \
      else \
        word<<=(offset*8); \
    }\
  }

  /*
  see get_BE_from_txn for a description of the arguments.
  The only different things are 'word' which is the word to get,
  and 'data_ptr' which points to the entry into the data array,
  which is not necessarily the data_ptr of the transaction.
  */

  #define CREATE_PUT_WORD_INTO_TXN_FUNCTION(type) \
  void put_word_into_txn(type& word, \
                         unsigned char* data_ptr, \
                         unsigned int chunk, \
                         unsigned int offset, \
                         bool is_last, \
                         unsigned int effective_length\
                         )\
  { \
    if (chunk){ \
      unsigned int index=chunk*m_byte_width-offset; \
      unsigned int num_bytes=(is_last)? effective_length-index : m_byte_width; \
      if (num_bytes>m_byte_width) num_bytes=m_byte_width; \
      memcpy(&data_ptr[index], &word, num_bytes);\
    } \
    else {\
      unsigned int num_bytes=(is_last)? effective_length : m_byte_width-offset; \
      if (num_bytes>m_byte_width) num_bytes=m_byte_width; \
      type tmp=word; \
      if (tlm::get_host_endianness()==tlm::TLM_BIG_ENDIAN) \
        tmp<<=(offset*8); \
      else \
        tmp>>=(offset*8); \
      memcpy(data_ptr, &tmp, num_bytes);\
    }\
  }
  
  CREATE_GET_WORD_FROM_TXN_FUNCTION(int8_t)
  CREATE_GET_WORD_FROM_TXN_FUNCTION(int16_t)
  CREATE_GET_WORD_FROM_TXN_FUNCTION(int32_t)
  CREATE_GET_WORD_FROM_TXN_FUNCTION(int64_t)

  CREATE_GET_WORD_FROM_TXN_FUNCTION(uint8_t)
  CREATE_GET_WORD_FROM_TXN_FUNCTION(uint16_t)
  CREATE_GET_WORD_FROM_TXN_FUNCTION(uint32_t)
  CREATE_GET_WORD_FROM_TXN_FUNCTION(uint64_t)
  
  //we need special versions for the sc types.
  // they work very similar to get_BE_from_txn()
  template <int W>
  void get_word_from_txn(sc_dt::sc_bigint<W>& word, 
                         unsigned char* data_ptr, 
                         unsigned int chunk, 
                         unsigned int offset, 
                         bool is_last, 
                         unsigned int effective_length)
  {
    unsigned int loop_min=(chunk)?m_byte_width*chunk-offset : 0;
    unsigned int loop_max=(is_last)? effective_length:
                          (chunk)  ? loop_min+m_byte_width : m_byte_width-offset;
    //we gotta have the line below because of imprecise bursts
    // there the data length can be any number larger than the real number of transmitted bytes
    //  and if the caller identifies the last chunk, loop_max could be too large as it is computed using the data_length
    if ((loop_max-loop_min)>m_byte_width) loop_max=loop_min+m_byte_width; 
    if (tlm::get_host_endianness()==tlm::TLM_BIG_ENDIAN)
      for (unsigned int i=loop_min, j=(chunk)?m_byte_width : m_byte_width-offset; i<loop_max; i++, j--)
        word.range(j*8-1,(j-1)*8)=data_ptr[i];
    else
      for (unsigned int i=loop_min, j=(chunk)? 1: offset+1; i<loop_max; i++, j++){
        word.range(j*8-1,(j-1)*8)=data_ptr[i];
      }
  }

  template <int W>
  void get_word_from_txn(sc_dt::sc_biguint<W>& word, 
                         unsigned char* data_ptr, 
                         unsigned int chunk, 
                         unsigned int offset, 
                         bool is_last, 
                         unsigned int effective_length)
  {
    unsigned int loop_min=(chunk)?m_byte_width*chunk-offset : 0;
    unsigned int loop_max=(is_last)? effective_length:
                          (chunk)  ? loop_min+m_byte_width : m_byte_width-offset;
    //we gotta have the line below because of imprecise bursts
    // there the data length can be any number larger than the real number of transmitted bytes
    //  and if the caller identifies the last chunk, loop_max could be too large as it is computed using the data_length
    if ((loop_max-loop_min)>m_byte_width) loop_max=loop_min+m_byte_width; 
    if (tlm::get_host_endianness()==tlm::TLM_BIG_ENDIAN)
      for (unsigned int i=loop_min, j=(chunk)?m_byte_width : m_byte_width-offset; i<loop_max; i++, j--)
        word.range(j*8-1,(j-1)*8)=data_ptr[i];
    else
      for (unsigned int i=loop_min, j=(chunk)? 1: offset+1; i<loop_max; i++, j++){
        word.range(j*8-1,(j-1)*8)=data_ptr[i];
      }
  }

  CREATE_PUT_WORD_INTO_TXN_FUNCTION(int8_t)
  CREATE_PUT_WORD_INTO_TXN_FUNCTION(int16_t)
  CREATE_PUT_WORD_INTO_TXN_FUNCTION(int32_t)
  CREATE_PUT_WORD_INTO_TXN_FUNCTION(int64_t)

  CREATE_PUT_WORD_INTO_TXN_FUNCTION(uint8_t)
  CREATE_PUT_WORD_INTO_TXN_FUNCTION(uint16_t)
  CREATE_PUT_WORD_INTO_TXN_FUNCTION(uint32_t)
  CREATE_PUT_WORD_INTO_TXN_FUNCTION(uint64_t)

  template <int W>
  void put_word_into_txn(sc_dt::sc_biguint<W>& word, unsigned char* data_ptr, unsigned int chunk, unsigned int offset, bool is_last, unsigned int effective_length){
    unsigned int loop_min=(chunk)?m_byte_width*chunk-offset : 0;
    unsigned int loop_max=(is_last)? effective_length:
                          (chunk)  ? loop_min+m_byte_width : m_byte_width-offset;
    //we gotta have the line below because of imprecise bursts
    // there the data length can be any number larger than the real number of transmitted bytes
    //  and if the caller identifies the last chunk, loop_max could be too large as it is computed using the data_length
    if ((loop_max-loop_min)>m_byte_width) loop_max=loop_min+m_byte_width; 
    if (tlm::get_host_endianness()==tlm::TLM_BIG_ENDIAN)
      for (unsigned int i=loop_min, j=(chunk)?m_byte_width : m_byte_width-offset; i<loop_max; i++, j--)
        data_ptr[i]=word.range(j*8-1,(j-1)*8).to_uint();
    else
      for (unsigned int i=loop_min, j=(chunk)? 1: offset+1; i<loop_max; i++, j++){
        data_ptr[i]=word.range(j*8-1,(j-1)*8).to_uint();
      }
  }

  template <int W>
  void put_word_into_txn(sc_dt::sc_bigint<W>& word, unsigned char* data_ptr, unsigned int chunk, unsigned int offset, bool is_last, unsigned int effective_length){
    unsigned int loop_min=(chunk)?m_byte_width*chunk-offset : 0;
    unsigned int loop_max=(is_last)? effective_length:
                          (chunk)  ? loop_min+m_byte_width : m_byte_width-offset;
    //we gotta have the line below because of imprecise bursts
    // there the data length can be any number larger than the real number of transmitted bytes
    //  and if the caller identifies the last chunk, loop_max could be too large as it is computed using the data_length
    if ((loop_max-loop_min)>m_byte_width) loop_max=loop_min+m_byte_width; 
    if (tlm::get_host_endianness()==tlm::TLM_BIG_ENDIAN)
      for (unsigned int i=loop_min, j=(chunk)?m_byte_width : m_byte_width-offset; i<loop_max; i++, j--)
        data_ptr[i]=word.range(j*8-1,(j-1)*8).to_uint();
    else
      for (unsigned int i=loop_min, j=(chunk)? 1: offset+1; i<loop_max; i++, j++){
        data_ptr[i]=word.range(j*8-1,(j-1)*8).to_uint();
      }
  }
  
  #undef CREATE_GET_WORD_FROM_TXN_FUNCTION
  #undef CREATE_PUT_WORD_INTO_TXN_FUNCTION

  typedef void (my_type::*cb_type)(Td& word,
                         unsigned char* data_ptr,
                         unsigned int chunk,
                         unsigned int offset,
                         bool is_last,
                         unsigned int effective_length);
  
  
  
  /*
  I used this programming style for two reasons:
   1. I just wanted a single function. I started with more than one, and quickly found myself fixing the same bugs
      at many different places. So I merged all the functions into this one, because they just differed in the
      function they called. However, the cost is that the CB function may be not inlineable, as it is called
      as a function pointer (but maybe it can be inlined because the function pointer is fixed...)
   2. I used templates to avoid runtime if-else evaluation.
  */
  
  
  template< typename WORD_TYPE //data type of word
          , bool WITH_DT //shall the word be (de)serialized
          , bool WITH_BE //shall the byte enable be (de)serialized
          , cb_type CB   //what function shall be called to perform the operation (determines whether we serialize or deserialize)
          >
  //read the function name as (de)serialize
  // the only combinations that work are:
  //  WITH_DT WITH_BE CB
  //  true    true    get
  //  true    false   get
  //  false   true    get
  //  true    false   put
  //that's why the get_BE is hard coded
  void de_serialize(ocpip_legacy::OCPMBurstSeqType seq, //the current burst sequence
                   tlm::tlm_generic_payload& txn,  //the transaction from which we want to (de)serialze stuff
                   have_seen_it* have_seen,        //the current context of the transaction
                   WORD_TYPE& word, //the word to (de)serialze
                   unsigned int cnt, //the number of the word to extract (first is one !!!! Sorry for the inconsistency)
                   unsigned int& byteEN=m_dummy_uint //the byte enable mask to (de)serialize
                   )
  {
    unsigned int row=0; //we assume row sorry for a start
    switch(seq){
    
      /*
        For WRAP and XOR we cannot use the chunk counter, because the words are not extracted
        in order, but following the WRAP or XOR sequence
      */
      case ocpip_legacy::OCP_MBURSTSEQ_WRAP:
      case ocpip_legacy::OCP_MBURSTSEQ_XOR:
        //calculate the chunk index out of current address and lowest address in burst
        if (!WITH_DT && WITH_BE) //only happens for reqBE
          cnt=(have_seen->req.MAddr-have_seen->seq_calc.get_low_addr())/m_byte_width+1;
        else{
          cnt=(have_seen->data_seq_calc.next()-have_seen->data_seq_calc.get_low_addr())/m_byte_width+1;        
        }
        //fall through now!
        
      /*
      For INCR we gotta check if we got dead leading words and handle them.
      Dead leading words can only appear with burst_aligned_incr bursts (if the data array starts
      in the middle of the fitting burst_aligned_incr_burst)
      */
      case ocpip_legacy::OCP_MBURSTSEQ_INCR:
        if (cnt-1<have_seen->dead_leading_words){
          byteEN=0;
          return;
        }
        //okay, we are at a living word
        // that is out first living word, so we substract the number of dead leading words from the counter
        cnt-=have_seen->dead_leading_words; 
        //fall through now!
        
      /*
      For DFLT1, INCR, XOR, WRAP (all packing, i.e. we can split and merge words) we gotta check if we fall over the edge of the data array.
      We will now check if we are not at the first chunk, and if we are
       check if this chunk is outside the data array.
       Note: the first chunk HAS to contain at least one byte according to the TLM2 standard.
      */
      case ocpip_legacy::OCP_MBURSTSEQ_DFLT1:
        if ((cnt-1) && ((((cnt-1)*m_byte_width)-have_seen->offset)>=txn.get_data_length())){
          byteEN=0;
          return;
        }
        //okay. so we have at least one valid byte for the current chunk in the data array,
        // let's go (de)serialize it!
        break;
        
      /*
      For DFLT2 (non-packing, i.e. no split or merge) make sure the user did not send DFLT2 words over a link that is smaller 
      than the DFLT2 words (because we cannot split them)
      */
      case ocpip_legacy::OCP_MBURSTSEQ_DFLT2:
        //NOTE: blck_row_length_in_bytes is the number of bytes of a DFLT2 word
        assert((m_byte_width>=have_seen->blck_row_length_in_bytes)&&" DFLT2 Size must not be larger than connection width.");
        //fall through
        
      /*
      For UNKN (splittable but not mergeable) we gotta treat split and merge differently
      DFLT2 can only arrive here it is sane.
      The code far below will treat UNKN and DFLT2 as BLCK, so we gotta prepare the variables for BLCK decode
      */      
      case ocpip_legacy::OCP_MBURSTSEQ_UNKN:{
        //NOTE: unkn_pseudo_b_len contains the number of bus words into which we will split one unkn word
        //      blck_row_length_in_bytes is the number of bytes of a unkn word
        
        if (m_byte_width>=have_seen->blck_row_length_in_bytes){ //bus is wider than or the same size as the unkn words. So we won't merge or split
          row=cnt-1; //we only have one word per pseudo row, so the chunk count matches the row count
          cnt=1;     //we only have one word per pseudo row
          assert(((*(have_seen->p_addr_vector))[row]%have_seen->blck_row_length_in_bytes)==0); //make sure addresses are aligned with size of UNKN words
          have_seen->offset=(*(have_seen->p_addr_vector))[row]%m_byte_width; //determine the offset
        }
        else{ //the bus is smaller than the UNKN words, so we'll split (cannot happen if seq==DFLT2)
          row=(cnt-1)/have_seen->unkn_pseudo_b_len;   //the row increases every pseudo_b_len-th word
          cnt=cnt-(row*have_seen->unkn_pseudo_b_len); //the word count is taken modulo pseudo_b_len
        }
        //if we have a unkn or DFLT2 beat, it has at least as many bytes as specified in bytes_per_address
        // the only exception to this is the last beat
        
        //we may have disabled unkn beats if the burst length extension was used and its value exceeds what
        // would have been calculated by calculate_ocp_burst_length
        
        //the very first word must be there (cnt=1 && row=0) no need to check
        
        //all other words in a row could fall over edge of the data array
        // we gotta treat the first of a row differently, because it starts at an integer multiple
        //  of blck_row_length_in_bytes, while all others might start at offset integers multiples
        //   of blck_row_length_in_bytes
        if ((cnt-1)&&((row*have_seen->blck_row_length_in_bytes+((cnt-1)*m_byte_width)-have_seen->offset)>=txn.get_data_length())){
          byteEN=0;
          return;
        }
        if ((row && cnt==1)&&((row*have_seen->blck_row_length_in_bytes)>=txn.get_data_length())){ //check the first word
          byteEN=0;
          return;
        }

        break;
      }
      
      /*
      For STRM we just gotta test if we fell over the edge of the data array
      */
      case ocpip_legacy::OCP_MBURSTSEQ_STRM:      
        if ((cnt-1) && ((txn.get_streaming_width()*(cnt-1))>=txn.get_data_length())){
          byteEN=0;
          //std::cout<<"Inactive word! strm"<<std::endl;
          return;
        }
        break;
        
      /*
      For BLCK we gotta determine the row we are in (normalize chunk count to burst length),
      and we have to determine which chunk of the row it is (chunk count modulo burst length)
      */
      case ocpip_legacy::OCP_MBURSTSEQ_BLCK:
        row=(cnt-1)/have_seen->req.MBurstLength;
        cnt=cnt-(row*have_seen->req.MBurstLength);
        
        //we might have disabled words:
        
        //the very first word must be there (cnt=1 && row=0) no need to check
        
        //all other words in a row could fall over edge of the data array, or over the (virtual) edge of blck_row_length_in_bytes
        // (this can happen if the burst length extension is set to a larger value than what calculate_ocp_burst_length would tell us
        //also we gotta treat the first of a row differently, because it starts at an integer multiple
        //  of blck_row_length_in_bytes, while all others might start at offset integers multiples
        //   of blck_row_length_in_bytes. Note that we do not need to check if the first of a row falls over the edge of blck_row_length_in_bytes
        //    because blck_row_length_in_bytes can never be zero
        if ((cnt-1) && ((((cnt-1)*m_byte_width)-have_seen->offset)>=have_seen->blck_row_length_in_bytes)){
          byteEN=0;
          return;
        }
        if ((cnt-1)&&((row*have_seen->blck_row_length_in_bytes+((cnt-1)*m_byte_width)-have_seen->offset)>=txn.get_data_length())){
          byteEN=0;
          return;
        }
        if ((row && cnt==1)&&((row*have_seen->blck_row_length_in_bytes)>=txn.get_data_length())){ //check first word
          byteEN=0;
          return;
        }
        break;
        
      /*
      Let's make sure no one emitted a non-ocp sequence.
      */
      case ocpip_legacy::OCP_MBURSTSEQ_RESERVED:
        assert(0 && "Reserved stream sequence detected.");
        exit(1);
    }
    
    /*
    We gotta know if this is the last word that we can extract from the data/be array, because the
    extractor/inserter function needs to know that to prevent data/be array accesses out of bound.
    Note that this is not necessarily the last chunk of the burst (the others could be implicitly
    disabled, by just not adding them to the data array).
    
    Note: cnt starts at one, while the chunk counts of the extractor/inserter functions start at zero.
          Hence we will always use cnt-1 when calling the extractor/inserter functions. When we use
          cnt directly, it is always relating to the NEXT chunk.
    
    A STRM has reached its last word when the next word falls over the edge of the data array.
    A BLCK, UNKN or DFLT2 has reached its last word of the current row (not overall!) if the next word falls over the
    edge of blck_row_length_in_bytes (note for UNKN and DFLT2 blck_row_length_in_bytes represents unkn_dflt_bytes_per_address).
    All other sequences (the packing ones: INCR, XOR, WRAP, DFLT1) have reached the end when the next word falls over
    the edge of the data array.
    */
    bool last=(seq==ocpip_legacy::OCP_MBURSTSEQ_STRM)? (txn.get_streaming_width()*(cnt))>=txn.get_data_length() :
              (seq==ocpip_legacy::OCP_MBURSTSEQ_BLCK | seq==ocpip_legacy::OCP_MBURSTSEQ_UNKN | seq==ocpip_legacy::OCP_MBURSTSEQ_DFLT2)? 
                                         (((cnt)*m_byte_width)-have_seen->offset)>=have_seen->blck_row_length_in_bytes
                                       : (((cnt)*m_byte_width)-have_seen->offset)>=txn.get_data_length();
    
    /*
    Now let's do the job.
    */
    switch(seq){
      case ocpip_legacy::OCP_MBURSTSEQ_STRM:
        /*
        The basic idea for STRM is:
          We treat every single word as a data array of its own (length==streaming_width).
          We know that streaming_width<=data_width, so every fake data array has only one
          word, hence every word is the last and the first (chunk=0) in its fake data array.
          
          We gotta treat the last word (w.r.t. the real data array) special.
          There the effective length of the fake data array could be less than
          streaming_width because the word could be just partially in the data array.
          Hence we calculate the effective length in this case.
        */
      {
        unsigned int effective_length=(last)? (txn.get_data_length()-txn.get_streaming_width()*(cnt-1)) : txn.get_streaming_width();
        if (WITH_DT)
          (this->*CB)(word, 
                            &txn.get_data_ptr()[txn.get_streaming_width()*(cnt-1)], //calculate and get the start of the fake data array
                            0, //always the first chunk
                            have_seen->offset, //always the same offset (it's a stream, right?)
                            true,  //always the last word of fake data array
                            effective_length); //length of fake data array
        if (WITH_BE)
          get_BE_from_txn(byteEN, 
                            txn.get_byte_enable_ptr(), //always use real start of BE array (cannot move it here, because that would mess up the
                                                       //  modulo operation within get_BE
                            0, //always first chunk
                            have_seen->offset, //always the same offset (it's a stream, right?) 
                            true, //always the last word of fake data array
                            effective_length, //length of fake data array
                            txn.get_streaming_width()*(cnt-1), //index shift to the start of the fake data array
                            txn.get_byte_enable_length()); //just get the byte_enable length
        break;
      } 
      /*
      We treat DFLT2, UNKN and BLCK all as BLCK. The row and cnt values were perpared above.
      We will use a fake data array per row.
      */
      case ocpip_legacy::OCP_MBURSTSEQ_DFLT2:
      case ocpip_legacy::OCP_MBURSTSEQ_UNKN:
      case ocpip_legacy::OCP_MBURSTSEQ_BLCK:{
        assert(txn.get_streaming_width()>=txn.get_data_length() && "Block, Unkn, or Dflt2 sequences may not have streaming widths");
        /*
        We need to calculate the effective length. Note that we can be sure to blck_row_length_in_bytes bytes per row.
        The only exception is the last which may only be partially available.
        */
        unsigned int effective_length;
        if((row+1)*have_seen->blck_row_length_in_bytes<txn.get_data_length()){
          //there is a byte for the next row, so the effective length is the full row, and the last value is correct
          effective_length=have_seen->blck_row_length_in_bytes;
        }
        else{
          //there is no byte for the next row, so the effective length is the bytes left for this row
          // and the last word is reached when the next word of this row falls out of the data length
          effective_length=txn.get_data_length()-(row*have_seen->blck_row_length_in_bytes);
          last=(row*have_seen->blck_row_length_in_bytes+cnt*m_byte_width-have_seen->offset)>=txn.get_data_length();
        }
        if (WITH_DT)
          (this->*CB)(word,
                            &txn.get_data_ptr()[row*have_seen->blck_row_length_in_bytes], //the fake data array starts at integer multiple of the row length
                            cnt-1, //local count starts at one, the count used be the CBs start at zero, so we correct that 
                            have_seen->offset, //provide offset
                            last, //provide last information
                            effective_length); //provide the length of the fake data array
        if (WITH_BE)
          get_BE_from_txn(byteEN,
                           txn.get_byte_enable_ptr(), //always use real start of BE array
                            cnt-1, //local count starts at one, the count used be the CBs start at zero, so we correct that 
                            have_seen->offset, //provide offset
                            last, //provide last information
                            effective_length,//provide the length of the fake data array
                            row*have_seen->blck_row_length_in_bytes, //provide the index of the start of the fake data array
                            txn.get_byte_enable_length()); //provide the byte_enable_length
        break;    
      }
      
      /*
      DFLT1, XOR, WRAP and INCR are all packing. So the treatment of their data and byte enable arrays
      is the same. They just differ in the way how they calculate addresses.
      We do not need to fake anything here, as there is just one contiguous data array,
      not a set of separate fake data arrays. 
      */      
      case ocpip_legacy::OCP_MBURSTSEQ_DFLT1:      
      case ocpip_legacy::OCP_MBURSTSEQ_XOR:
      case ocpip_legacy::OCP_MBURSTSEQ_WRAP:
      case ocpip_legacy::OCP_MBURSTSEQ_INCR:
        if (WITH_DT)
          (this->*CB)(word,
                            txn.get_data_ptr(), //provide start of data array
                            cnt-1, //correct diff in local chunk counting (start at 1) and chunk counting in CBs (start at zero)
                            have_seen->offset, //provide the offset
                            last, //provide the last info
                            txn.get_data_length()); //effective length is always the whole data length
        if (WITH_BE)
          get_BE_from_txn(byteEN,
                           txn.get_byte_enable_ptr(), //provide start of BE array
                            cnt-1, //correct diff in local chunk counting (start at 1) and chunk counting in CBs (start at zero)
                            have_seen->offset, //provide the offset
                            last, //provide the last info
                            txn.get_data_length(),//effective length is always the whole data length
                            0, //we do not have an index shift
                            txn.get_byte_enable_length());  //provide the BE length    
        break;       
      case ocpip_legacy::OCP_MBURSTSEQ_RESERVED:
        assert(0 && "Reserved stream sequence detected.");
        exit(1);
    }
  }

  ocpip_legacy::OCPMCmdType decode_cmd(tlm::tlm_generic_payload& txn, bool check_lock=true){
    lock* lck;
    semaphore * sem;
    if (ext_base::template get_extension<lock>(lck, txn) && check_lock){
      if (txn.is_read()) return ocpip_legacy::OCP_MCMD_RDEX;
      else {
        have_seen_it* have_seen;
        acc(txn).get_extension(have_seen);
        assert(have_seen);
        have_seen->unlocking_wr=true;
        return decode_cmd(txn, false); //decode the write and ignore the lock
      } 
    }
    else
    if (ext_base::template get_extension<semaphore>(sem, txn)){
      if (txn.is_read()) return ocpip_legacy::OCP_MCMD_RDL;
      else
      if (txn.is_write()) return ocpip_legacy::OCP_MCMD_WRC;
    }
    else
    if (ext_base::template get_extension<posted>(txn)){
      if (txn.is_write()) return ocpip_legacy::OCP_MCMD_WR;
      else
        std::cerr<<"Monitor warning: posted non-write cannot be decoded for OCP on forward call of link "
                <<m_name<<std::endl;
    }
    else
    if (ext_base::template get_extension<broadcast>(txn)){
      if (txn.is_write()) return ocpip_legacy::OCP_MCMD_BCST;
      else
        std::cerr<<"Monitor warning: Broadcast non-write cannot be decoded for OCP on forward call of link "
                <<m_name<<std::endl;                  
    }
    else
    if (txn.is_write()) return ocpip_legacy::OCP_MCMD_WRNP;
    else
    if (txn.is_read()) return ocpip_legacy::OCP_MCMD_RD;
    return ocpip_legacy::OCP_MCMD_IDLE;
  }

  Ta get_unkn_address(have_seen_it* have_seen){
    if (m_byte_width>=have_seen->blck_row_length_in_bytes){ //DFLT1, DFLT2, UNKN
      if (have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_DFLT1){ //DFLT1 packing
        unsigned int offset=(*(have_seen->p_addr_vector))[0]%m_byte_width;
        if (have_seen->req_cnt==1){ //first chunk
          return (*(have_seen->p_addr_vector))[0]-offset; //allign first address to our buswidth
        }
        offset/=have_seen->blck_row_length_in_bytes;
        return (*(have_seen->p_addr_vector))[(have_seen->req_cnt-1)*(m_byte_width/have_seen->blck_row_length_in_bytes)-offset];
      }
      //DFLT2, UNKN no packing
      unsigned int offset=(*(have_seen->p_addr_vector))[have_seen->req_cnt-1]%m_byte_width;
      assert(have_seen->p_addr_vector->size()>(have_seen->req_cnt-1));
      return (*(have_seen->p_addr_vector))[have_seen->req_cnt-1]-offset;
    }
    else{ //m_byte_width<have_seen->blck_row_length_in_bytes //DFLT1, UNKN split
      unsigned int cnt, row;
      row=(have_seen->req_cnt-1)/have_seen->unkn_pseudo_b_len;
      cnt=have_seen->req_cnt-(row*have_seen->unkn_pseudo_b_len)-1;
      assert(have_seen->p_addr_vector->size()>row);
      return  (*(have_seen->p_addr_vector))[row]+m_byte_width*cnt;
    }
  
  }

  request_type& fill_req(tlm::tlm_generic_payload& txn){
    have_seen_it* have_seen=m_extPool.create(); 
    acc(txn).set_extension(have_seen); 
    burst_length* b_len;
    bool has_b_len=ext_base::template get_extension<burst_length>(b_len, txn);
    have_seen->offset=calculate_ocp_address_offset(txn, m_byte_width);
    assert((have_seen->offset && (m_owner->*m_get_param_cb)().byteen) | (have_seen->offset==0)); //ensure we have byteen if we have an offset
    burst_sequence* b_seq; 
    bool has_b_seq=ext_base::get_extension(b_seq, txn);
    thread_id* th_id; 
    bool has_th_id=ext_base::get_extension(th_id, txn);
    tag_id* tg_id; 
    bool has_tg_id=ext_base::get_extension(tg_id, txn);
    address_space* addr_sp; 
    bool has_addr_sp=ext_base::get_extension(addr_sp, txn);
    conn_id* con_id; 
    bool has_conn_id=ext_base::get_extension(con_id, txn);
    atomic_length* at_len; 
    bool has_at_len=ext_base::get_extension(at_len, txn);
    
    have_seen->req_cnt=1;
    have_seen->dhs_cnt=0;
    have_seen->rsp_cnt=0;
    have_seen->unlocking_wr=false;

    have_seen->req.MCmd=decode_cmd(txn);
    have_seen->req.MCmdSaved=have_seen->req.MCmd;


    have_seen->req.MBurstSeq= has_b_seq                                       ? (ocpip_legacy::OCPMBurstSeqType)b_seq->value.sequence : 
                             (txn.get_streaming_width()<txn.get_data_length())? ocpip_legacy::OCP_MBURSTSEQ_STRM : ocpip_legacy::OCP_MBURSTSEQ_INCR ;

    Ta ocp_address;
    if (!has_b_len){ //we add it and validate it to make sure others don't do the same job again
      b_len->value=calculate_ocp_burst_length(txn, 
                                              m_byte_width, 
                                              have_seen->offset, 
                                              (m_owner->*m_get_param_cb)().burst_aligned && have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_INCR, 
                                              ocp_address,
                                              b_seq);
      ext_base::template validate_extension<burst_length>(txn);
    }
    
    have_seen->req.MAddrSpace=has_addr_sp? addr_sp->value : 0;

    have_seen->req.MThreadID=has_th_id? th_id->value : 0;
    have_seen->req.MConnID=has_conn_id? con_id->value : 0;
    //req_info* rq_inf;
    //bool has_req_info=ext_base::template get_extension(rq_inf, txn);
    have_seen->req.MBurstSingleReq=((m_owner->*m_get_param_cb)().burstsinglereq) && ext_base::template get_extension<srmd>(txn);
    if ((m_owner->*m_get_param_cb)().reqinfo){
      if (!m_req_info_to_bit) {std::cerr<<"No req info conversion registered in "<<m_name<<"."<<std::endl; abort();}
      have_seen->req.MReqInfo=m_req_info_to_bit(txn);
    }
    else
      have_seen->req.MReqInfo=0;
/*
    have_seen->req.MBlockHeight=(has_b_seq && ((OCPMBurstSeqType)b_seq->value.sequence)==OCP_MBURSTSEQ_BLCK)? b_seq->value.block_height :
                                ((txn.get_streaming_width()<txn.get_data_length()) && ((txn.get_streaming_width()+have_seen->offset)>m_byte_width))?
                                (txn.get_data_length()+txn.get_streaming_width()-1)/txn.get_streaming_width() : 1;
*/
    have_seen->req.MBlockHeight=(has_b_seq && ((ocpip_legacy::OCPMBurstSeqType)b_seq->value.sequence)==ocpip_legacy::OCP_MBURSTSEQ_BLCK)? b_seq->value.block_height : 1;

    have_seen->req.MBlockStride=(has_b_seq && ((ocpip_legacy::OCPMBurstSeqType)b_seq->value.sequence)==ocpip_legacy::OCP_MBURSTSEQ_BLCK)? b_seq->value.block_stride : 0;
    have_seen->req.MReqLast=(b_len->value==1 || have_seen->req.MBurstSingleReq)? true : false;
    have_seen->req.MReqRowLast=(have_seen->req.MReqLast)? true : false;
    have_seen->req.MTagID=has_tg_id?tg_id->value:0;
    have_seen->req.MTagInOrder=!has_tg_id;
    have_seen->req.MAtomicLength=has_at_len? at_len->value : 1;
    have_seen->req.MBurstLength=b_len->value;
    have_seen->req.MBurstPrecise=(ext_base::template get_extension<imprecise>(txn)==NULL);
/*
    have_seen->req.MBurstSeq= has_b_seq                                       ? (OCPMBurstSeqType)b_seq->value.sequence : 
                             (txn.get_streaming_width()<txn.get_data_length())? 
                               ((txn.get_streaming_width()+have_seen->offset)>m_byte_width)? OCP_MBURSTSEQ_BLCK : OCP_MBURSTSEQ_STRM 
                               : OCP_MBURSTSEQ_INCR;
*/
    
    
    have_seen->req.HasMData=(!(m_owner->*m_get_param_cb)().datahandshake) && txn.is_write();//txn.get_data_ptr()!=0;
    have_seen->dead_leading_words=0;

    if ( have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_UNKN || have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_DFLT2 || have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_DFLT1){
      assert((have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_UNKN && b_seq->value.unkn_dflt_addresses_valid)
             || have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_DFLT2 || have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_DFLT1);
      have_seen->p_addr_vector=(b_seq->value.unkn_dflt_addresses_valid)?&b_seq->value.unkn_dflt_addresses : NULL;
      have_seen->req.MAddr=(have_seen->p_addr_vector)? 
                                    get_unkn_address(have_seen) 
                                    : txn.get_address(); //if we do not have a vector the address from the txn is the best guess...
      have_seen->blck_row_length_in_bytes=b_seq->value.unkn_dflt_bytes_per_address;
      have_seen->unkn_pseudo_b_len=(b_seq->value.unkn_dflt_bytes_per_address+m_byte_width-1)/m_byte_width;
    }
    else
    {
      have_seen->blck_row_length_in_bytes=b_seq->value.blck_row_length_in_bytes;
      if ((m_owner->*m_get_param_cb)().burst_aligned && have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_INCR){
        if (has_b_len) //we did not calculate it together with the b_len
          calculate_ocp_burst_aligned_INCR_address( b_len->value
                                                  , txn.get_address()
                                                  , m_byte_width
                                                  , have_seen->offset
                                                  , txn.get_data_length()
                                                  , ocp_address);
        have_seen->dead_leading_words=(txn.get_address()-ocp_address)/m_byte_width;
      }
      else
        ocp_address=((have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_WRAP || have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_XOR) && b_seq->value.xor_wrap_address!=txn.get_address())? b_seq->value.xor_wrap_address
                        : txn.get_address()-have_seen->offset;
      have_seen->seq_calc.init( ocp_address, have_seen->req.MBurstSeq, have_seen->req.MBurstLength,
                     have_seen->req.MBurstPrecise, have_seen->req.MBlockHeight, have_seen->req.MBlockStride);
      have_seen->data_seq_calc.init( ocp_address, have_seen->req.MBurstSeq, have_seen->req.MBurstLength,
                     have_seen->req.MBurstPrecise, have_seen->req.MBlockHeight, have_seen->req.MBlockStride);
      have_seen->req.MAddr=have_seen->seq_calc.next();
    }
    
    if (have_seen->req.HasMData && txn.get_command()==tlm::TLM_WRITE_COMMAND) {
      if ((m_owner->*m_get_param_cb)().mdatabyteen)
        de_serialize<Td,true,true,&my_type::get_word_from_txn>(have_seen->req.MBurstSeq, 
                         txn, 
                         have_seen, 
                         have_seen->req.MData,
                         1,
                         have_seen->req.MByteEn                         
                         );
      else
        de_serialize<Td,true,false,&my_type::get_word_from_txn>(have_seen->req.MBurstSeq, 
                         txn, 
                         have_seen, 
                         have_seen->req.MData,
                         1
                         );
      
    }
    if ((m_owner->*m_get_param_cb)().byteen)
      de_serialize<Td,false,true,&my_type::get_word_from_txn>(have_seen->req.MBurstSeq, 
                       txn, 
                       have_seen, 
                       m_dummy_dt,
                       1,
                       have_seen->req.MByteEn
                       );

    if ((m_owner->*m_get_param_cb)().mdatainfo && !(m_owner->*m_get_param_cb)().datahandshake && txn.is_write()){
      if (!m_mdata_info_to_bit) {std::cerr<<"No mdata info conversion registered in "<<m_name<<"."<<std::endl; abort();}
      have_seen->req.MDataInfo=m_mdata_info_to_bit(txn);
    }
    else
      have_seen->req.MDataInfo=0;  
    have_seen->req.MDataTagID=have_seen->req.MTagID;  
    return have_seen->req;
  }
  
  request_type& update_req(tlm::tlm_generic_payload& txn){
    have_seen_it* have_seen;
    acc(txn).get_extension(have_seen);
    assert(have_seen);
    have_seen->req_cnt++;
    burst_length* b_len;
    bool has_b_len=ext_base::template get_extension<burst_length>(b_len, txn);
    if (!have_seen->req.MBurstPrecise) {
      assert(has_b_len);
      have_seen->req.MBurstLength=b_len->value;
    }
    //does not change
    //have_seen->req.MCmd=decode_cmd(txn);
    //have_seen->req.MCmdSaved=have_seen->req.MCmd;
    have_seen->req.MReqLast=(have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_BLCK)
      ? (have_seen->req_cnt==(have_seen->req.MBlockHeight*have_seen->req.MBurstLength))
      : (have_seen->req.MBurstPrecise)
        ? have_seen->req.MBurstLength==have_seen->req_cnt
        : b_len->value==1; //when imprecise the last req is reached when blen is 1


    have_seen->req.MReqRowLast=have_seen->req.MReqLast || !(have_seen->req_cnt % have_seen->req.MBurstLength);

    if (have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_UNKN ||  have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_DFLT2|| have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_DFLT1){
      have_seen->req.MAddr=(have_seen->p_addr_vector)? 
                            get_unkn_address(have_seen) 
                            : txn.get_address(); //if we do not have a vector the address from the txn is the best guess...
    }    
    else{
      have_seen->req.MAddr=have_seen->seq_calc.next();
    }

    if ((m_owner->*m_get_param_cb)().reqinfo){
      if (!m_req_info_to_bit) {std::cerr<<"No req info conversion registered in "<<m_name<<"."<<std::endl; abort();}
      have_seen->req.MReqInfo=m_req_info_to_bit(txn);
    }
    else
      have_seen->req.MReqInfo=0;

    if ((m_owner->*m_get_param_cb)().byteen)
      de_serialize<Td,false,true,&my_type::get_word_from_txn>(have_seen->req.MBurstSeq, 
                       txn, 
                       have_seen, 
                       m_dummy_dt,
                       have_seen->req_cnt,
                       have_seen->req.MByteEn
                       );

    if (have_seen->req.HasMData && txn.get_command()==tlm::TLM_WRITE_COMMAND) {
      if ((m_owner->*m_get_param_cb)().mdatabyteen)
        de_serialize<Td,true,true,&my_type::get_word_from_txn>(have_seen->req.MBurstSeq, 
                         txn, 
                         have_seen, 
                         have_seen->req.MData,
                         have_seen->req_cnt,
                         have_seen->req.MByteEn
                         );
      else
        de_serialize<Td,true,false,&my_type::get_word_from_txn>(have_seen->req.MBurstSeq, 
                         txn, 
                         have_seen, 
                         have_seen->req.MData,
                         have_seen->req_cnt
                        );
    }

    if ((m_owner->*m_get_param_cb)().mdatainfo && !(m_owner->*m_get_param_cb)().datahandshake && txn.is_write()){
      if (!m_mdata_info_to_bit) {std::cerr<<"No mdata info conversion registered in "<<m_name<<"."<<std::endl; abort();}
      have_seen->req.MDataInfo=m_mdata_info_to_bit(txn);
    }
    else
      have_seen->req.MDataInfo=0;  

    return have_seen->req;
  }
  
  datahs_type& update_dhs(tlm::tlm_generic_payload& txn){
    have_seen_it* have_seen;
    acc(txn).get_extension(have_seen);
    assert(have_seen);
    have_seen->dhs_cnt++;
    have_seen->dhs.MDataThreadID=have_seen->req.MThreadID;
    have_seen->dhs.MDataTagID=have_seen->req.MTagID;
    have_seen->dhs.MDataValid=true;
    have_seen->dhs.MDataValidSaved=true;
    have_seen->dhs.MDataInfo=0;  

    have_seen->dhs.MDataLast=(have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_BLCK)
      ? (have_seen->dhs_cnt==(have_seen->req.MBlockHeight*have_seen->req.MBurstLength))
      : (have_seen->req.MBurstPrecise)
        ? have_seen->req.MBurstLength==have_seen->dhs_cnt
        : have_seen->req.MReqLast && (have_seen->dhs_cnt==have_seen->req_cnt);
    
    have_seen->dhs.MDataRowLast=have_seen->dhs.MDataLast || !(have_seen->dhs_cnt % have_seen->req.MBurstLength) ;
    
    //setBE_from_txn(have_seen->dhs.MDataByteEn, &txn, have_seen->dhs_cnt-1, have_seen->offset, have_seen->dhs.MDataLast, txn.get_data_length());
    //get_word_from_txn(have_seen->dhs.MData, &txn, have_seen->dhs_cnt-1, have_seen->offset, have_seen->dhs.MDataLast, txn.get_data_length());
    if ((m_owner->*m_get_param_cb)().mdatabyteen)
      de_serialize<Td,true,true,&my_type::get_word_from_txn>(have_seen->req.MBurstSeq, 
                       txn, 
                       have_seen, 
                       have_seen->dhs.MData,
                       have_seen->dhs_cnt,
                       have_seen->dhs.MDataByteEn                       
                       );
    else
      de_serialize<Td,true,false,&my_type::get_word_from_txn>(have_seen->req.MBurstSeq, 
                       txn, 
                       have_seen, 
                       have_seen->dhs.MData,
                       have_seen->dhs_cnt
                       );

    if ((m_owner->*m_get_param_cb)().mdatainfo){
      if (!m_mdata_info_to_bit) {std::cerr<<"No mdata info conversion registered in "<<m_name<<"."<<std::endl; abort();}
      have_seen->dhs.MDataInfo=m_mdata_info_to_bit(txn);
    }
    else
      have_seen->dhs.MDataInfo=0;  
                       
    return have_seen->dhs;
  }
  

  response_type& update_rsp(tlm::tlm_generic_payload& txn){
    have_seen_it* have_seen;
    acc(txn).get_extension(have_seen);
    assert(have_seen);
    have_seen->rsp_cnt++;
    have_seen->rsp.SThreadID=have_seen->req.MThreadID;
    have_seen->rsp.STagID=have_seen->req.MTagID;
    have_seen->rsp.STagInOrder=have_seen->req.MTagInOrder;
    have_seen->rsp.SDataInfo=0;  
    have_seen->rsp.SRespInfo=0;    
    switch(txn.get_response_status())
    {
    case tlm::TLM_OK_RESPONSE: have_seen->rsp.SResp=ocpip_legacy::OCP_SRESP_DVA; break;
    case tlm::TLM_INCOMPLETE_RESPONSE:  have_seen->rsp.SResp=ocpip_legacy::OCP_SRESP_NULL; break;
    default: have_seen->rsp.SResp=ocpip_legacy::OCP_SRESP_ERR; break;
    }
    if (have_seen->req.MCmd==ocpip_legacy::OCP_MCMD_WRC){
      semaphore* sem;
      ext_base::template get_extension<semaphore>(sem, txn);
      if (sem->value==false) have_seen->rsp.SResp=ocpip_legacy::OCP_SRESP_FAIL;
    }

    have_seen->rsp.SRespSaved=have_seen->rsp.SResp;

    have_seen->rsp.SRespLast=((have_seen->req.MCmd==ocpip_legacy::OCP_MCMD_WR || have_seen->req.MCmd==ocpip_legacy::OCP_MCMD_WRNP) && have_seen->req.MBurstSingleReq)
      ? true
      :(have_seen->req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_BLCK)
        ? (have_seen->rsp_cnt==(have_seen->req.MBlockHeight*have_seen->req.MBurstLength))
        : (have_seen->req.MBurstPrecise)
          ? have_seen->req.MBurstLength==have_seen->rsp_cnt
          : have_seen->req.MReqLast && (have_seen->rsp_cnt==have_seen->req_cnt);// b_len->value==1; //when imprecise the last req is reached when blen is 1

    have_seen->rsp.SRespRowLast=have_seen->rsp.SRespLast || !(have_seen->rsp_cnt % have_seen->req.MBurstLength);
    
    if (txn.get_command()!=tlm::TLM_WRITE_COMMAND){
      de_serialize<Td,true,false,&my_type::get_word_from_txn>(have_seen->req.MBurstSeq, 
                       txn, 
                       have_seen, 
                       have_seen->rsp.SData,
                       have_seen->rsp_cnt
                       );    
    }
    else
      have_seen->rsp.SData=0;

    if ((m_owner->*m_get_param_cb)().sdatainfo && txn.is_read()){
      if (!m_sdata_info_to_bit) {std::cerr<<"No sdata info conversion registered in "<<m_name<<"."<<std::endl; abort();}
      have_seen->rsp.SDataInfo=m_sdata_info_to_bit(txn);
    }
    else
      have_seen->rsp.SDataInfo=0;  

    if ((m_owner->*m_get_param_cb)().respinfo){
      if (!m_resp_info_to_bit) {std::cerr<<"No rsp info conversion registered in "<<m_name<<"."<<std::endl; abort();}
      have_seen->rsp.SRespInfo=m_resp_info_to_bit(txn);
    }
    else
      have_seen->rsp.SRespInfo=0;  
    
    return have_seen->rsp;
  }

  bool check_end_of_monitoring (tlm::tlm_generic_payload& txn, int check_what){
    have_seen_it* have_seen;
    acc(txn).get_extension(have_seen);
    assert(have_seen);
    if (check_what==3){ //forced end
      m_extPool.recycle(have_seen); //put our extension back to the pool
      acc(txn).clear_extension(have_seen_it::priv_id);      
      return true;
    }
    if (!have_seen->req.MBurstPrecise){
      if (have_seen->req.MBurstLength==1) {//imprecise burst is over
        switch(check_what){
          case 0: break;
          case 1: if (have_seen->dhs_cnt!=have_seen->req_cnt) return false; break;
          case 2: if (have_seen->rsp_cnt!=have_seen->req_cnt) return false; break;
          default: std::cout<<"FFFF"<<std::endl; assert(0); exit(1);
        }
        m_extPool.recycle(have_seen); //put our extension back to the pool
        acc(txn).clear_extension(have_seen_it::priv_id);
        return true;
      }
    }
    else{
      unsigned int cmp;
      switch(check_what){
        case 0: //req count check
                cmp=have_seen->req_cnt; break;
        case 1: //dhs count check
                cmp=have_seen->dhs_cnt; break;
        case 2: //resp count check
                if (txn.get_command()==tlm::TLM_WRITE_COMMAND && have_seen->req.MBurstSingleReq)
                    cmp=have_seen->dhs_cnt; //if there was a SRMD write with resp
                                            // the resp after the last dhs is the final one
                else
                  cmp=have_seen->rsp_cnt;
                break;
        default: std::cout<<"BBBBB"<<std::endl; assert(0); exit(1);
      }
      if (cmp==have_seen->req.MBlockHeight*have_seen->req.MBurstLength){ //we can always use BlockHeight here, as it defaults to 1 for non BLCK bursts      
      //if (cmp==have_seen->req.MBurstLength){
        //precise burst is over
        m_extPool.recycle(have_seen); //put our extension back to the pool
        acc(txn).clear_extension(have_seen_it::priv_id);
        return true;               
      }
    }
    return false;    
  }  
  
  void init_txn_from_legacy_req(request_type& req, tlm::tlm_generic_payload& txn, lock_object_base* lock_object=NULL){
    paramcl_type& params=(m_owner->*m_get_param_cb)();
    have_seen_it* have_seen=m_extPool.create();
    have_seen->req=req;
    acc(txn).set_extension(have_seen); 
    have_seen->offset=0;
    switch(req.MCmd){
      case ocpip_legacy::OCP_MCMD_IDLE: txn.set_command(tlm::TLM_IGNORE_COMMAND); break;
      case ocpip_legacy::OCP_MCMD_WR:   
        txn.set_command(tlm::TLM_WRITE_COMMAND); 
        ext_base::template validate_extension<posted>(txn);
        if (lock_object){
          lock* lck;
          ext_base::template get_extension<lock>(lck, txn);
          lck->value=lock_object;
          ext_base::template validate_extension<lock>(txn);
        }
        break;
      case ocpip_legacy::OCP_MCMD_RD:   txn.set_command(tlm::TLM_READ_COMMAND); break;
      case ocpip_legacy::OCP_MCMD_RDEX: 
        txn.set_command(tlm::TLM_READ_COMMAND);
        lock* lck;
        ext_base::template get_extension<lock>(lck, txn);
        lck->value=lock_object;
        ext_base::template validate_extension<lock>(txn);
        break;
      case ocpip_legacy::OCP_MCMD_RDL:
        txn.set_command(tlm::TLM_READ_COMMAND);
        ext_base::template validate_extension<semaphore>(txn);
        break;
      case ocpip_legacy::OCP_MCMD_WRNP:
        txn.set_command(tlm::TLM_WRITE_COMMAND);
        //ext_base::template validate_extension<nonposted>(txn);
        if (lock_object){
          lock* lck;
          ext_base::template get_extension<lock>(lck, txn);
          lck->value=lock_object;
          ext_base::template validate_extension<lock>(txn);
        }        
        break;
      case ocpip_legacy::OCP_MCMD_WRC:{
        txn.set_command(tlm::TLM_WRITE_COMMAND);
        semaphore* sem;
        ext_base::template get_extension<semaphore>(sem, txn);
        sem->value=false;
        ext_base::template validate_extension<semaphore>(txn);
        }
        break;
      case ocpip_legacy::OCP_MCMD_BCST:
        txn.set_command(tlm::TLM_WRITE_COMMAND);
        ext_base::template validate_extension<broadcast>(txn);
        break;
    }

    txn.set_streaming_width(txn.get_data_length()); //assume no streaming
    txn.set_address(req.MAddr); //assume a INCR addressing scheme
    

    if (params.burstseq){
      burst_sequence* b_seq;
      ext_base::template get_extension<burst_sequence>(b_seq, txn);
      b_seq->value.sequence=(burst_seqs)req.MBurstSeq;
      ext_base::template validate_extension<burst_sequence>(txn);
      
      switch (req.MBurstSeq){
        case ocpip_legacy::OCP_MBURSTSEQ_BLCK:
          b_seq->value.block_height=req.MBlockHeight;
          b_seq->value.block_stride=req.MBlockStride;
          b_seq->value.blck_row_length_in_bytes=m_byte_width*req.MBurstLength;
          have_seen->blck_row_length_in_bytes=m_byte_width*req.MBurstLength;
          break;
        case ocpip_legacy::OCP_MBURSTSEQ_DFLT1:
        case ocpip_legacy::OCP_MBURSTSEQ_DFLT2:
        case ocpip_legacy::OCP_MBURSTSEQ_UNKN:
          if (b_seq->value.unkn_dflt_addresses.size()<1) b_seq->value.unkn_dflt_addresses.resize(req.MBurstLength);
          b_seq->value.unkn_dflt_addresses[0]=req.MAddr;
          b_seq->value.unkn_dflt_bytes_per_address=m_byte_width;
          b_seq->value.unkn_dflt_addresses_valid=true;
          have_seen->p_addr_vector=&b_seq->value.unkn_dflt_addresses;
          have_seen->unkn_pseudo_b_len=1;
          have_seen->blck_row_length_in_bytes=m_byte_width;
          break;
        case ocpip_legacy::OCP_MBURSTSEQ_WRAP:
        case ocpip_legacy::OCP_MBURSTSEQ_XOR:
          b_seq->value.xor_wrap_address=req.MAddr;
          have_seen->data_seq_calc.init( req.MAddr, req.MBurstSeq, req.MBurstLength,
                         req.MBurstPrecise, req.MBlockHeight, req.MBlockStride); 
          txn.set_address(have_seen->data_seq_calc.get_low_addr());
        case ocpip_legacy::OCP_MBURSTSEQ_INCR:
          break;
        case ocpip_legacy::OCP_MBURSTSEQ_STRM:
          txn.set_streaming_width(m_byte_width);
          break;
        case ocpip_legacy::OCP_MBURSTSEQ_RESERVED:
          assert(0); exit(1);
      }
    }
    
    if (params.addrspace)  {
      address_space* addr_sp;
      ext_base::get_extension(addr_sp, txn);
      addr_sp->value=req.MAddrSpace;
      ext_base::template validate_extension<address_space>(txn);
    }
    
    if (params.byteen){
      set_BE_into_txn(req.MByteEn, &txn, 0);
    }
    if (params.threads>1){
      thread_id* th_id;
      ext_base::template get_extension<thread_id>(th_id, txn);
      th_id->value=req.MThreadID;
      ext_base::template validate_extension<thread_id>(txn);
    }
    if (params.connid){
      conn_id* cn_id;
      ext_base::template get_extension<conn_id>(cn_id, txn);
      cn_id->value=req.MConnID;
      ext_base::template validate_extension<conn_id>(txn);
    }
    if (params.tags>1){
      tag_id* tg_id;
      ext_base::template get_extension<tag_id>(tg_id, txn);
      tg_id->value=req.MTagID;
      ext_base::template validate_extension<tag_id>(txn);  //we have a tag, we set it and mark it as valid    
    }
    if (params.taginorder && req.MTagInOrder)
      ext_base::template invalidate_extension<tag_id>(txn); //tags are in order? so the tag becomes don't care and we invalidate it
    if (params.atomiclength){
      atomic_length* at_len;
      ext_base::template get_extension<atomic_length>(at_len, txn);
      at_len->value=req.MAtomicLength;
      ext_base::template validate_extension<atomic_length>(txn);      
    }
    if (params.burstlength){
      burst_length* b_len;
      ext_base::template get_extension<burst_length>(b_len, txn);
      b_len->value=req.MBurstLength;
      ext_base::template validate_extension<burst_length>(txn); 
    }
    if (params.burstprecise && !req.MBurstPrecise){
      ext_base::template validate_extension<imprecise>(txn);
    }
    
    if (req.HasMData && (req.MCmd==ocpip_legacy::OCP_MCMD_WR  || req.MCmd==ocpip_legacy::OCP_MCMD_WRNP)){
      this->template de_serialize<Td,true,false,&my_type::put_word_into_txn>(req.MBurstSeq,txn, have_seen, req.MData, 1); //cannot happen for SRMD
    }
    if (req.MBurstSingleReq && params.burstsinglereq)
      ext_base::template validate_extension<srmd>(txn);
    //MReqLast is derivable information
    have_seen->req_cnt=1;
    have_seen->dhs_cnt=0;
    have_seen->rsp_cnt=0;
    
    //we have set addr, cmd and streaming width
    // the caller must have set data_ptr, data_length, byte_enable_ptr and byte_enable_length
    // so we still need resp and dmi
    txn.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    txn.set_dmi_allowed(false);
    
    if (params.reqinfo){
      if (!m_req_info_to_ext){
        std::cerr<<"No req info conversion registered in "<<m_name<<"."<<std::endl; abort();
      }
      m_req_info_to_ext(req.MReqInfo, txn);
    }

    if (params.mdatainfo && txn.is_write() && !params.datahandshake){
      if (!m_mdata_info_to_ext){
        std::cerr<<"No mdata info conversion registered in "<<m_name<<"."<<std::endl; abort();
      }
      m_mdata_info_to_ext(req.MDataInfo, txn);
    }

  }
  
  bool update_txn_from_legacy_req(request_type& req, tlm::tlm_generic_payload& txn){
    paramcl_type& params=(m_owner->*m_get_param_cb)();
    have_seen_it* have_seen;
    acc(txn).get_extension(have_seen);
    assert(have_seen);
    //MCmd must remain stable
    if (req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_UNKN || req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_DFLT1 || req.MBurstSeq==ocpip_legacy::OCP_MBURSTSEQ_DFLT2){
      burst_sequence* b_seq;
      ext_base::template get_extension<burst_sequence>(b_seq, txn);
      if (b_seq->value.unkn_dflt_addresses.size()<have_seen->req_cnt+1) b_seq->value.unkn_dflt_addresses.resize(have_seen->req_cnt+10);
      b_seq->value.unkn_dflt_addresses[have_seen->req_cnt]=req.MAddr;
    }
    //MAddrSpace must remain stable    
    if (params.byteen){
      set_BE_into_txn(req.MByteEn, &txn, have_seen->req_cnt);
    }
    //MThreadID is only valid with first phase
    //MConnID stays stable
    //MReqRowLast is derivable information
    //MTagID is only valid with first phase
    //MTagInOrder is only valid with first phase
    //MAtomicLength is only valid with first phase
    if (params.burstlength){
      burst_length* b_len;
      ext_base::template get_extension<burst_length>(b_len, txn);
      b_len->value=req.MBurstLength;
      ext_base::template validate_extension<burst_length>(txn); 
    }
    have_seen->req.MBurstLength=req.MBurstLength;//remeber last b_length
    //MBurstPrecise is only valid with first phase
    //MBurstSeq is only valid with first phase
    have_seen->req_cnt++;    
    if (req.HasMData && (req.MCmd==ocpip_legacy::OCP_MCMD_WR || req.MCmd==ocpip_legacy::OCP_MCMD_WRNP)){
      //if we arrive here with burst length==1 we know it's an imprecise burst
      de_serialize<Td,true,false,&my_type::put_word_into_txn>(req.MBurstSeq,txn, have_seen, req.MData, have_seen->req_cnt);
    }
    //MReqLast is derivable information

    if (params.reqinfo){
      if (!m_req_info_to_ext){
        std::cerr<<"No req info conversion registered in "<<m_name<<"."<<std::endl; abort();
      }
      m_req_info_to_ext(req.MReqInfo, txn);
    }

    if (params.mdatainfo && txn.is_write() && !params.datahandshake){
      if (!m_mdata_info_to_ext){
        std::cerr<<"No mdata info conversion registered in "<<m_name<<"."<<std::endl; abort();
      }
      m_mdata_info_to_ext(req.MDataInfo, txn);
    }

    if (params.reqinfo){
      if (!m_req_info_to_ext){
        std::cerr<<"No req info conversion registered in "<<m_name<<"."<<std::endl; abort();
      }
      m_req_info_to_ext(req.MReqInfo, txn);
    }

    if (params.mdatainfo && txn.is_write() && !params.datahandshake){
      if (!m_mdata_info_to_ext){
        std::cerr<<"No mdata info conversion registered in "<<m_name<<"."<<std::endl; abort();
      }
      m_mdata_info_to_ext(req.MDataInfo, txn);
    }

    return ( (req.MBurstPrecise && (have_seen->req_cnt == (req.MBlockHeight*req.MBurstLength)))
           | (!req.MBurstPrecise && (req.MBurstLength == 1)) );
  }
  
  void update_txn_from_legacy_datahs(datahs_type& dhs, tlm::tlm_generic_payload& txn){
    paramcl_type& params=(m_owner->*m_get_param_cb)();
    have_seen_it* have_seen;
    acc(txn).get_extension(have_seen);
    assert(have_seen);
    if (params.mdatabyteen){
      set_BE_into_txn(dhs.MDataByteEn, &txn, have_seen->dhs_cnt);
    }
    have_seen->dhs_cnt++;
    //put_word_into_txn(dhs.MData,&txn, have_seen->dhs_cnt, 0, last, txn.get_data_length());
    de_serialize<Td,true,false,&my_type::put_word_into_txn>(have_seen->req.MBurstSeq,txn, have_seen, dhs.MData, have_seen->dhs_cnt);
    //MDataThreadID does not change compared to MThreadID
    //MDataRowLast is derivable information
    //MDataTagID matches MTagID, will not be set
    //MDataLast is derivable information
    assert(dhs.MDataValid); //should be valid right?
    if (params.mdatainfo){
      if (!m_mdata_info_to_ext){
        std::cerr<<"No mdata info conversion registered in "<<m_name<<"."<<std::endl; abort();
      }
      m_mdata_info_to_ext(dhs.MDataInfo, txn);
    }
    
  }
  
  void update_txn_from_legacy_rsp(response_type& rsp, tlm::tlm_generic_payload& txn){
    //paramcl_type& params=(m_owner->*m_get_param_cb)();
    have_seen_it* have_seen;
    acc(txn).get_extension(have_seen);
    assert(have_seen);
    have_seen->rsp_cnt++;
    if (have_seen->req.MCmd==ocpip_legacy::OCP_MCMD_RDEX){
      lock* lck;
      ext_base::template get_extension<lock>(lck, txn);
      lck->value->lock_is_understood_by_slave=(m_owner->*m_get_param_cb)().readex_enable;
    }
    switch(rsp.SResp){
      case ocpip_legacy::OCP_SRESP_DVA: txn.set_response_status(tlm::TLM_OK_RESPONSE); break;
      case ocpip_legacy::OCP_SRESP_NULL: txn.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE); break;
      case ocpip_legacy::OCP_SRESP_FAIL: //txn.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE); break;
                           txn.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
                           {
                           semaphore* sem;
                           ext_base::template get_extension<semaphore>(sem, txn);
                           ext_base::template validate_extension<semaphore>(txn);
                           sem->value=false;
                           }
                           break;
      case ocpip_legacy::OCP_SRESP_ERR:  txn.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    }
    if (!txn.is_write()){
      //bool last=(have_seen->req.MBurstPrecise)? (have_seen->rsp_cnt+1)==have_seen->req.MBurstLength 
      //                                        : (have_seen->req.MBurstLength==1) && (have_seen->req_cnt == (have_seen->rsp_cnt+1));    
      //put_word_into_txn(rsp.SData,&txn, have_seen->rsp_cnt, have_seen->offset, last, txn.get_data_length());
      de_serialize<Td,true,false,&my_type::put_word_into_txn>(have_seen->req.MBurstSeq, txn, have_seen, rsp.SData, have_seen->rsp_cnt);
    }

    //unsigned int STagID; matches tagID that was set by initiator
    //unsigned int STagInOrder; matches what was set by initiator
    //unsigned int SThreadID; matches what was set by initiator
    //bool         SRespLast; derivable info
    //bool         SRespRowLast; derivable info
    if ((m_owner->*m_get_param_cb)().respinfo){
      if (!m_resp_info_to_ext){
        std::cerr<<"No resp info conversion registered in "<<m_name<<"."<<std::endl; abort();
      }
      m_resp_info_to_ext(rsp.SRespInfo, txn);
    }

    if ((m_owner->*m_get_param_cb)().sdatainfo && txn.is_read()){
      if (!m_sdata_info_to_ext){
        std::cerr<<"No sdata info conversion registered in "<<m_name<<"."<<std::endl; abort();
      }
      m_sdata_info_to_ext(rsp.SDataInfo, txn);
    }

  }
};

template <typename DataCl, typename OWNER, typename EXT_SUPPORT, unsigned int BUSWIDTH>
unsigned int ocp_tl1_legacy_convertion_helper<DataCl, OWNER, EXT_SUPPORT, BUSWIDTH>::m_dummy_uint=0xdeadbeef;

template <typename DataCl, typename OWNER, typename EXT_SUPPORT, unsigned int BUSWIDTH>
typename ocp_tl1_legacy_convertion_helper<DataCl, OWNER, EXT_SUPPORT, BUSWIDTH>::Td ocp_tl1_legacy_convertion_helper<DataCl, OWNER, EXT_SUPPORT, BUSWIDTH>::m_dummy_dt=0;

}
