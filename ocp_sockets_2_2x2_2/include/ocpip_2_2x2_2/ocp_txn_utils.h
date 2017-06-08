///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// (c) Copyright OCP-IP 2008-2009
// OCP-IP Confidential and Proprietary
//
//
//============================================================================
//      Project : OCP SLD WG
//       Author : Herve Alexanian - Sonics, inc.
//
//          $Id:
//
//  Description :  This file defines utility classes to access tlm transactions
//                 through semantics familiar to OCP users
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_txn_utils.h may not be included directly. Use #include "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION {
class rdex_lock_object: public lock_object_base
{
private:
    friend class ocp_extension_pool<rdex_lock_object>;    
    void atomic_txn_completed(){
	get_pool()->recycle(this);
    }    
    rdex_lock_object(){
	lock_object_base::number_of_txns=2;
    }
public:    
    static ocp_extension_pool<rdex_lock_object>* get_pool(){
	static ocp_extension_pool<rdex_lock_object> s_pool(50);
	return &s_pool;
    }
    static lock_object_base*& null_lock() {
	static lock_object_base* s_p = NULL;
	return s_p;
    }
};

//! \brief sets the TLM command from an OCP MCmd code
//! \param ext    the object providing socket extension support (an ocp socket)
//! \param txn    a duly allocated TLM generic payload
//! \param cmd    MCmd enum matching OCP hardware codes
//! \param p_lock a RDEX lock object. Must be NULL for a RDEX and a new lock will be returned, or passed in for an unlocking write
template <typename ext_support>
void set_txn_cmd( ext_support& ext, tlm::tlm_generic_payload& txn, mcmd_codes cmd,
		  lock_object_base*& p_lock=rdex_lock_object::null_lock() )
{
    lock* p_lock_ext = NULL;
    switch(cmd) {
    case IDLE:
	txn.set_command(tlm::TLM_IGNORE_COMMAND); break;
    case WR:
	ext.template validate_extension<posted>(txn);
	// fall through
    case WRNP:
	txn.set_command(tlm::TLM_WRITE_COMMAND); 
	if (p_lock != NULL){
	    ext.template get_extension<lock>(p_lock_ext, txn);
	    p_lock_ext->value=p_lock;
	    ext.template validate_extension<lock>(txn);
	}
	break;
    case RDEX: 
	p_lock = rdex_lock_object::get_pool()->create();
	ext.template get_extension<lock>(p_lock_ext, txn);
	p_lock_ext->value=p_lock;
	ext.template validate_extension<lock>(txn);
	// fall through
    case RD:
	txn.set_command(tlm::TLM_READ_COMMAND); break;
    case RDL:
	txn.set_command(tlm::TLM_READ_COMMAND);
	ext.template validate_extension<semaphore>(txn);
	break;
    case WRC: {
	txn.set_command(tlm::TLM_WRITE_COMMAND);
	semaphore* sem;
	ext.template get_extension<semaphore>(sem, txn);
	sem->value=false;
	ext.template validate_extension<semaphore>(txn);
    }
	break;
    case BCST:
	txn.set_command(tlm::TLM_WRITE_COMMAND);
	ext.template validate_extension<broadcast>(txn);
	break;
    }
}


//! \brief sets the TLM burstseq and burstlength extensions for a (non-BLCK) ocp burst
//! \param ext    the object providing socket extension support (an ocp socket)
//! \param txn    a duly allocated TLM generic payload
//! \param length unsigned integer for MBurstLength
//! \param seq    MBurstSeq enum matching OCP hardware codes
template <typename ext_support>
burst_sequence* set_txn_row_burst( ext_support& ext, tlm::tlm_generic_payload& txn, uint32_t length, burst_seqs seq = INCR )
{
    // first set burst_length extension, it should be defined for all bursts
    burst_length* bl;
    ext.template get_extension<burst_length>(bl, txn);
    bl->value=length;
    ext.template validate_extension<burst_length>(txn);

    burst_sequence* b_seq;
    ext.template get_extension<burst_sequence>(b_seq, txn);
    b_seq->value.sequence=seq;
    ext.template validate_extension<burst_sequence>(txn);
    return b_seq;
}
//! \brief sets the TLM burstseq and burstlength extensions for a BLCK ocp burst
//! \param ext    the object providing socket extension support (an ocp socket)
//! \param txn    a duly allocated TLM generic payload
//! \param length unsigned integer for MBurstLength
//! \param height unsigned integer for MBlockHeight
//! \param stride unsigned integer for MBlockStride
template <typename ext_support>
burst_sequence* set_txn_block_burst( ext_support& ext, tlm::tlm_generic_payload& txn, uint32_t length, uint32_t height, uint32_t stride )
{
    // first set burst_length extension, it should be defined for all bursts
    burst_length* bl;
    ext.template get_extension<burst_length>(bl, txn);
    bl->value=length;
    ext.template validate_extension<burst_length>(txn);

    burst_sequence* b_seq;
    ext.template get_extension<burst_sequence>(b_seq, txn);
    b_seq->value.sequence=BLCK;
    b_seq->value.block_height=height;
    b_seq->value.block_stride=stride;
    ext.template validate_extension<burst_sequence>(txn);
    return b_seq;
}


//! \brief sets the TLM address space extension to a given value
//! \param ext    the object providing socket extension support (an ocp socket)
//! \param txn    a duly allocated TLM generic payload
//! \param space  unsigned integer specifying the ocp address space
template <typename ext_support>
void set_txn_address_space( ext_support& ext, tlm::tlm_generic_payload& txn, unsigned int space )
{
    address_space* as;
    ext.template get_extension<address_space>(as, txn);
    as->value=space;
    ext.template validate_extension<address_space>(txn);
}
//! \brief sets the TLM atomic length extension to a given value
//! \param ext     the object providing socket extension support (an ocp socket)
//! \param txn     a duly allocated TLM generic payload
//! \param alength unsigned integer specifying the ocp atomic length
template <typename ext_support>
void set_txn_atomic_length( ext_support& ext, tlm::tlm_generic_payload& txn, unsigned int alength )
{
    atomic_length* al;
    ext.template get_extension<atomic_length>(al, txn);
    al->value=alength;
    ext.template validate_extension<atomic_length>(txn);
}
//! \brief sets the TLM thread id extension to a given value
//! \param ext    the object providing socket extension support (an ocp socket)
//! \param txn    a duly allocated TLM generic payload
//! \param thread unsigned integer specifying the ocp thread
template <typename ext_support>
void set_txn_thread_id( ext_support& ext, tlm::tlm_generic_payload& txn, unsigned int thread )
{
    thread_id* th_id;
    ext.template get_extension<thread_id>(th_id, txn);
    th_id->value=thread;
    ext.template validate_extension<thread_id>(txn);
}
//! \brief sets the TLM conn id extension to a given value
//! \param ext    the object providing socket extension support (an ocp socket)
//! \param txn    a duly allocated TLM generic payload
//! \param connid unsigned integer specifying the ocp conn id
template <typename ext_support>
void set_txn_conn_id( ext_support& ext, tlm::tlm_generic_payload& txn, unsigned int connid )
{
    conn_id* co_id;
    ext.template get_extension<conn_id>(co_id, txn);
    co_id->value=connid;
    ext.template validate_extension<conn_id>(txn);
}
//! \brief sets the TLM tag id extension to a given value
//! \param ext    the object providing socket extension support (an ocp socket)
//! \param txn    a duly allocated TLM generic payload
//! \param tag unsigned integer specifying the ocp tag
template <typename ext_support>
void set_txn_tag_id( ext_support& ext, tlm::tlm_generic_payload& txn, unsigned int tag )
{
    tag_id* tg_id;
    ext.template get_extension<tag_id>(tg_id, txn);
    tg_id->value=tag;
    ext.template validate_extension<tag_id>(txn);
}

inline
uint32_t get_num_phases( tlm::tlm_generic_payload& txn, const tlm::tlm_phase& ph,
			 tlm_utils::instance_specific_extension_accessor& acc )
{
    ocp_txn_burst_invariant* p_inv = NULL;
    acc(txn).get_extension( p_inv );
    assert( p_inv != NULL );

    uint32_t num_beats = p_inv->burst_length;	
    if ( p_inv->get_sequence() == BLCK ) {
	assert( p_inv->burst_seq_ext_valid );			
	num_beats *= p_inv->burst_seq_ext.block_height;
    }

    switch ( ph ) {
    case tlm::BEGIN_REQ:
    case tlm::END_REQ  :
	return ( p_inv->srmd ) ? 1 : num_beats;	
    case tlm::END_RESP  :
    case tlm::BEGIN_RESP:
	return ( p_inv->srmd && txn.is_write() ) ? 1 : num_beats;
    default:
	if ( ph == BEGIN_DATA || ph == END_DATA )
	    return num_beats;
    }

    assert( false && "unknown ocp data flow phase" );
    return 0;
}
}

