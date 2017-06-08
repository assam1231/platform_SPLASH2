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
//  Description :  This file defines utility classes designed to be used as instance
//                 specific extensions on a tlm generic payload to ease OCP burst
//                 decoding
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_txn_inst_extensions.h may not be included directly. Use #include "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{

// Burst invariant fields storage utility
struct ocp_txn_burst_invariant:
public tlm_utils::instance_specific_extension<ocp_txn_burst_invariant> {
    sc_core::sc_time          start_time;
    
    // gp invariants
    uint32_t                  streaming_width;
    uint32_t                  data_length;
    
    // ocp request info invariants
    ocpip_legacy::OCPMCmdType cmd;
    uint32_t                  threadid;
    uint32_t                  tagid;
    bool                      taginorder;
    uint32_t                  connid;
    uint32_t                  addr_space;
    uint32_t                  atomic_length;
    uint32_t                  burst_length;
    bool                      precise;
    bool                      srmd;
    
    // burst sequence. Copy of the extension only defined when
    // burst_seq_ext_valid=true
    burst_seq_info            burst_seq_ext;
    bool                      burst_seq_ext_valid;
    burst_seqs                get_sequence() const {
	if ( burst_seq_ext_valid )
	    return burst_seq_ext.sequence;
	else
	    return (streaming_width < data_length) ? STRM : INCR;
    }

    uint64_t get_unkn_address( uint32_t beat, uint32_t byte_width ) const {
	burst_seqs seq = get_sequence();
	if (byte_width>=burst_seq_ext.unkn_dflt_bytes_per_address) {
	    if (seq == DFLT1) {
		uint32_t offset=burst_seq_ext.unkn_dflt_addresses[0]%byte_width;
		if (beat==1){ //first chunk
		    return burst_seq_ext.unkn_dflt_addresses[0]-offset; //align first address to our buswidth
		}
		offset/=burst_seq_ext.unkn_dflt_bytes_per_address;
		return burst_seq_ext.unkn_dflt_addresses[(beat-1)*(byte_width/burst_seq_ext.unkn_dflt_bytes_per_address)-offset];
	    }
	    //DFLT2, UNKN no packing
	    unsigned int offset=burst_seq_ext.unkn_dflt_addresses[beat-1]%byte_width;
	    assert(burst_seq_ext.unkn_dflt_addresses.size()>(beat-1));
	    return burst_seq_ext.unkn_dflt_addresses[beat-1]-offset;
	}
	else{ //DFLT1, UNKN split
	    uint32_t unkn_pseudo_b_len=(burst_seq_ext.unkn_dflt_bytes_per_address+byte_width-1)/byte_width;
	    unsigned int cnt, row;
	    row=(beat-1)/unkn_pseudo_b_len;
	    cnt=beat-(row*unkn_pseudo_b_len)-1;
	    assert(burst_seq_ext.unkn_dflt_addresses.size()>row);
	    return burst_seq_ext.unkn_dflt_addresses[row]+byte_width*cnt;
	}
    }

    static
    ocpip_legacy::OCPMCmdType decode_cmd( tlm::tlm_generic_payload& txn ) {
	lock* lck;
	semaphore * sem;
	if (extension_api::get_extension<lock>(lck, txn) && txn.is_read()) {
	    return ocpip_legacy::OCP_MCMD_RDEX;
	} else if (extension_api::get_extension<semaphore>(sem, txn)){
	    if (txn.is_read()) return ocpip_legacy::OCP_MCMD_RDL;
	    else               return ocpip_legacy::OCP_MCMD_WRC;
	} else if (extension_api::get_extension<posted>(txn)){
	    if (txn.is_write()) return ocpip_legacy::OCP_MCMD_WR;
	    else
		std::cerr<<"Warning: posted non-write cannot be decoded for OCP"
			 <<std::endl;
	} else if (extension_api::get_extension<broadcast>(txn)){
	    if (txn.is_write()) return ocpip_legacy::OCP_MCMD_BCST;
	    else
		std::cerr<<"Warning: Broadcast non-write cannot be decoded for OCP"
			 <<std::endl;
	} else if (txn.is_write()) {
	    return ocpip_legacy::OCP_MCMD_WRNP;
	} else if (txn.is_read()) {
	    return ocpip_legacy::OCP_MCMD_RD;
	}
	return ocpip_legacy::OCP_MCMD_IDLE;
    }
    // bkwc compatibility signature: deprecated
    static
    ocpip_legacy::OCPMCmdType decode_cmd( tlm::tlm_generic_payload& txn,
					  infr::bind_checker<tlm::tlm_base_protocol_types>::ext_support_type& ) {
	return decode_cmd( txn );
    }

    // extract an instance from a generic payload
    static 
    ocp_txn_burst_invariant init_from( tlm::tlm_generic_payload& txn,
				       const ocp_parameters& params ) {
	ocp_txn_burst_invariant inv;
	inv.streaming_width = txn.get_streaming_width();
	inv.data_length     = txn.get_data_length();

	inv.cmd = decode_cmd(txn);

	inv.threadid   = inv.tagid = 0;
	inv.taginorder = params.taginorder; // if enabled, 1 is default
	thread_id* th_id; 
	if ( extension_api::get_extension(th_id, txn) )
	    inv.threadid = th_id->value;
	tag_id* tg_id; 
	if ( extension_api::get_extension(tg_id, txn) ) {
	    inv.tagid = tg_id->value;
	    inv.taginorder = 0;
	}

	conn_id* conn_id_info;
	bool has_conn_id=extension_api::get_extension(conn_id_info, txn);
	inv.connid = has_conn_id ? conn_id_info->value : 0;

	address_space* address_space_info; 
	bool has_address_space=extension_api::get_extension(address_space_info, txn);
	inv.addr_space = has_address_space ? address_space_info->value : 0;

	OCPIP_VERSION::atomic_length* atomic_length_info;
	bool has_atomic_length=extension_api::get_extension(atomic_length_info, txn);
	inv.atomic_length = has_atomic_length ? atomic_length_info->value : 0;

	burst_sequence* b_seq;
	if ( inv.burst_seq_ext_valid=extension_api::get_extension(b_seq, txn) ) {
	    inv.burst_seq_ext      = b_seq->value;
	}

	OCPIP_VERSION::burst_length* b_len; 
	if ( extension_api::get_extension(b_len, txn) ) {
	    inv.burst_length      = b_len->value;
	} else {
	    const uint32_t byte_width=(params.data_wdth+7)>>3;
	    unsigned int offset = calculate_ocp_address_offset( txn, byte_width );
	    uint64_t dummy_addr;
	    inv.burst_length = calculate_ocp_burst_length(
		txn, byte_width, offset,
		params.burst_aligned && inv.get_sequence()==INCR, dummy_addr,
		inv.burst_seq_ext_valid ? b_seq : NULL );
	}

	inv.precise = !params.burstprecise ||
	    extension_api::get_extension<imprecise>(txn)==NULL;
	inv.srmd = params.burstsinglereq &&
	    extension_api::get_extension<OCPIP_VERSION::srmd>(txn);

	inv.start_time = sc_core::sc_time_stamp();
	return inv;
    }
    // bkwc compatibility signature: deprecated
    static 
    ocp_txn_burst_invariant init_from( tlm::tlm_generic_payload& txn,
				       const ocp_parameters& params,
				       infr::bind_checker<tlm::tlm_base_protocol_types>::ext_support_type& ) {
	return init_from( txn, params );
    }
};

struct ocp_txn_track_error : public std::exception {
    std::string m_msg;
    ocp_txn_track_error( const std::string& msg ) :
        m_msg( msg )
        {}
    ~ocp_txn_track_error() throw() {};
    const char* what() const throw() {
        return m_msg.c_str();
    }
};

// single threaded burst counter. Counts all bursts in progress
class ocp_txn_track_base {
  public:
    struct txn_position {
        uint16_t count;
        int16_t  remain; // neg count means unknown (imprecise in progress)
        uint16_t row_count;
        uint16_t row_remain;
	txn_position() {
	    count = 0; remain = 0; row_count = 0; row_remain = 0;
	}
    };
	
    struct txn_status {
        uint16_t m_row_length;
	bool     m_precise;
        int16_t  m_expect_req_count;
        int16_t  m_expect_data_count;
        int16_t  m_expect_resp_count;
        uint16_t m_req_count;
        uint16_t m_data_count;
        uint16_t m_resp_count;
        txn_status() :
            m_row_length        ( 1),
	    m_precise           ( 1),
            m_expect_req_count  (-1),
            m_expect_data_count (-1),
            m_expect_resp_count (-1),
            m_req_count         ( 0),
            m_data_count        ( 0),
            m_resp_count        ( 0)
            {}
        inline bool complete_req() const {
            return m_expect_req_count >= 0 &&
                static_cast<uint16_t>( m_expect_req_count ) == m_req_count;
        }
        inline bool complete_datahs() const {
            return m_expect_data_count >= 0 &&
                static_cast<uint16_t>( m_expect_data_count ) == m_data_count;
        }
        inline bool complete_resp() const {
            return m_expect_resp_count >= 0 &&
                static_cast<uint16_t>( m_expect_resp_count ) == m_resp_count;
        }
        inline bool complete() const {
	    return complete_req() && complete_datahs() && complete_resp();
        }
    };

    ocp_txn_track_base( const ocp_parameters* p_param ) :
        m_p_param( p_param ),
	m_byte_width( (m_p_param->data_wdth+7) >> 3 ) {
	reset();
    }
    // copy constructor because of the iterator members- initialized in reset()
    ocp_txn_track_base( const ocp_txn_track_base& other ) :
        m_p_param( other.m_p_param ),
	m_byte_width( other.m_byte_width ) {
	reset();
    }

    virtual ~ocp_txn_track_base() {}
    uint64_t num_open() const {
        return m_burst.size();
    }
    void reset() {
        m_burst.clear();
	m_first_open_dh   = m_burst.end();
	m_first_open_resp = m_burst.end();
    }

  protected:
    void purge() {
        // Erase up to the first incomplete burst
        while ( m_burst.begin() != m_burst.end() && m_burst.begin()->complete() ) {
	    // check for invalidated member iterators. Should not happen but invalid
	    // iterators are too big a disaster to risk.
	    if ( m_first_open_dh == m_burst.begin() )
		m_first_open_dh = m_burst.end();
	    if ( m_first_open_resp == m_burst.begin() )
		m_first_open_resp = m_burst.end();
	    m_burst.pop_front();
        }
    }

    std::list<txn_status>  m_burst;
    // these iterators are to speed up the search for the first open phase
    // during track_data and track_response. They must remain valid between
    // calls, which is the reason to pick a std::list as the burst container
    std::list<txn_status>::iterator m_first_open_dh;
    std::list<txn_status>::iterator m_first_open_resp;
    const ocp_parameters*  m_p_param;
    uint16_t               m_byte_width;
};

class ocp_txn_track : public ocp_txn_track_base {
  public:
    typedef ocp_txn_track_base::txn_position txn_position;
    typedef ocp_txn_track_base::txn_status txn_status;
    
    ocp_txn_track( const ocp_parameters* p_param,
		   bool tl1_n_tl2=false ) :
	ocp_txn_track_base( p_param )
	, m_tl1_n_tl2( tl1_n_tl2 )
    {	
    }

    bool is_tl1() const {
	return m_tl1_n_tl2;
    }
    bool is_tl2() const {
	return !m_tl1_n_tl2;
    }


    bool has_pending_request() {
	std::list<txn_status>::iterator pos = m_burst.begin();
	while (pos != m_burst.end()) {
	    if(pos->complete_req() == false)
		return true;
	    pos++;
	}
	return false;
    }
    bool has_pending_datahs() {
	std::list<txn_status>::iterator pos = m_burst.begin();
	while (pos != m_burst.end()) {
	    if(pos->complete_datahs() == false)
		return true;
	    pos++;
	}
	return false;
    }

    // The main method. track_phase returns a txn_position quadruplet with
    // count (beat count in the current burst)
    // remain (how many beats remain, negative number if imprecise)
    // row_count (beat count in the current row)
    // row_remain (how many beats remain in the current row)    
    txn_position track_phase ( tlm::tlm_generic_payload& txn, const tlm::tlm_phase& phase ) {
	switch ( phase ) {
	case tlm::BEGIN_REQ            : return track_request( txn );
	case tlm::BEGIN_RESP           : return track_response( txn );
	default:
	    if ( phase == BEGIN_DATA )
		return track_data( txn );
	    else 
		throw ( ocp_txn_track_error( "Can only track dataflow begin phases: BEGIN_REQ/BEGIN_DATA/BEGIN_RESP" ) );
	}
    }

    txn_position track_request( tlm::tlm_generic_payload& txn ) {
	bool new_req = ( m_burst.empty() || m_burst.back().complete_req() );
	if ( new_req ) {
	    m_burst.push_back( txn_status() );
	}
	txn_status& cur_burst = m_burst.back();

	if ( new_req ) {
	    // get invariant data now
	    ocp_txn_burst_invariant burst_inv = ocp_txn_burst_invariant::init_from( txn, *m_p_param );
	    bool blck_seq = burst_inv.get_sequence() == BLCK;
	    assert( !blck_seq || burst_inv.burst_seq_ext_valid );
	    uint16_t block_height = blck_seq ? burst_inv.burst_seq_ext.block_height : 1;
	    assert( block_height > 0 );
    
	    // Set up expected counts if precise.
	    cur_burst.m_precise = burst_inv.precise;
	    if ( burst_inv.precise ) {
		cur_burst.m_row_length = burst_inv.burst_length;
		uint16_t num_words     = burst_inv.burst_length * block_height;
		cur_burst.m_expect_req_count  = num_words;
		cur_burst.m_expect_data_count = num_words;
		cur_burst.m_expect_resp_count = num_words;
		if ( burst_inv.srmd ) {
		    cur_burst.m_expect_req_count = 1;
		    if ( txn.is_write() ) {
			cur_burst.m_expect_resp_count = 1;
		    }
		}
	    }

	    // Absent phases => expect 0
	    if ( txn.is_read() || !m_p_param->datahandshake )
		cur_burst.m_expect_data_count = 0;
	    if ( burst_inv.cmd == ocpip_legacy::OCP_MCMD_WR && !m_p_param->writeresp_enable )
		cur_burst.m_expect_resp_count = 0;	   
	}

	// Update request count
	if ( is_tl2() ) {
	    word_count* word_count_info;
	    bool has_word_count= extension_api::get_extension(word_count_info, txn);
	    if ( has_word_count )
		cur_burst.m_req_count += word_count_info->value.request_wc;
	    else {
		// completes the burst
		cur_burst.m_req_count = cur_burst.m_expect_req_count;
	    }
	} else {
	    cur_burst.m_req_count++;
	}

	// Update expected count if at the end of an imprecise burst
	if ( !cur_burst.m_precise ) {
	    // must have explicit burst_len
	    burst_length* b_len;
	    bool has_b_len=extension_api::get_extension<burst_length>(b_len, txn);
	    if ( !has_b_len )
		throw ( ocp_txn_track_error( "Ocp transaction for imprecise burst must define burst_length extension" ) );
	    if ( b_len->value == 1 ) {
		cur_burst.m_expect_req_count = cur_burst.m_req_count;
		// this request count is what to expect for data and resp as well (if we expect data/resp at all)
		if ( cur_burst.m_expect_data_count < 0 )
		    cur_burst.m_expect_data_count = cur_burst.m_req_count;
		if ( cur_burst.m_expect_resp_count < 0 )
		    cur_burst.m_expect_resp_count = cur_burst.m_req_count;
	    }
	}

	txn_position ret;
	ret.count     = cur_burst.m_req_count;
	ret.remain    = cur_burst.m_expect_req_count - ret.count;
	ret.row_count  = ( (int)( ret.count - 1 ) % cur_burst.m_row_length ) + 1;
	ret.row_remain = cur_burst.m_row_length - ret.row_count;

	purge();
	return ret;
    }
    
    txn_position track_data( tlm::tlm_generic_payload& txn ) {
	if ( m_burst.empty() )
	    throw ( ocp_txn_track_error( "Received Data phase with no open burst" ) );

	// Find the first burst still expecting data
	if ( m_first_open_dh == m_burst.end() )
	    m_first_open_dh = m_burst.begin();
	while ( m_first_open_dh->complete_datahs() ) {
	    if ( ++m_first_open_dh == m_burst.end() ) {
		throw( ocp_txn_track_error( "Received Data phase before first Request" ) );
	    }
	}
	// check within open burst that data count is behind request count
	txn_status& cur_burst = *m_first_open_dh;
	if ( !cur_burst.complete_req() && cur_burst.m_data_count >= cur_burst.m_req_count ) {
	    throw( ocp_txn_track_error( "Received Data phase before corresponding Request" ) );
	}    

	if ( is_tl2() ) {
	    word_count* word_count_info;
	    bool has_word_count= extension_api::get_extension(word_count_info, txn);
	    if ( has_word_count )
		cur_burst.m_data_count += word_count_info->value.data_wc;
	    else {
		// completes the burst
		cur_burst.m_data_count = cur_burst.m_expect_data_count;
	    }
	} else {
	    cur_burst.m_data_count++;
	}

	txn_position ret;
	ret.count     = cur_burst.m_data_count;
	ret.remain    = cur_burst.m_expect_data_count - ret.count;
	ret.row_count  = ( (int)( ret.count - 1 ) % cur_burst.m_row_length ) + 1;
	ret.row_remain = cur_burst.m_row_length - ret.row_count;

	if ( ret.remain == 0 ) {
	    // point to the next open DH or the end
	    // the iterator will remain valid because std::list only invalidates
	    // iterators when an element is deleted, and an burst with incomplete
	    // data cannot be erased until at least another call to this method
	    while ( m_first_open_dh->complete_datahs() ) {
		if ( ++m_first_open_dh == m_burst.end() )
		    break;
	    }
	}
	purge();
	return ret;
    }

    txn_position track_response( tlm::tlm_generic_payload& txn ) {
	if ( m_burst.empty() )
	    throw ( ocp_txn_track_error( "Received Response phase with no open burst" ) );

	// Find the first burst still expecting responses
	if ( m_first_open_resp == m_burst.end() )
	    m_first_open_resp = m_burst.begin();
	while ( m_first_open_resp->complete_resp() ) {
	    if ( ++m_first_open_resp == m_burst.end() ) {
		throw( ocp_txn_track_error(
			   "Received Response phase before first Request" ) );
	    }
	}
	txn_status& cur_burst = *m_first_open_resp;
	// check within open burst that resp count is behind request count and data count
	if ( !cur_burst.complete_req() && cur_burst.m_resp_count >= cur_burst.m_req_count ) {
	    throw( ocp_txn_track_error( "Received Response phase before corresponding Request" ) );
	}
	if ( !cur_burst.complete_datahs() && cur_burst.m_resp_count >= cur_burst.m_data_count ) {
	    throw( ocp_txn_track_error( "Received Response phase before corresponding Data" ) );
	}	    

	if ( is_tl2() ) {
	    word_count* word_count_info;
	    bool has_word_count= extension_api::get_extension(word_count_info, txn);
	    if ( has_word_count )
		cur_burst.m_resp_count += word_count_info->value.response_wc;
	    else {
		// completes the burst
		cur_burst.m_resp_count = cur_burst.m_expect_resp_count;
	    }
	} else {
	    cur_burst.m_resp_count++;
	}

	txn_position ret;
	ret.count     = cur_burst.m_resp_count;
	ret.remain    = cur_burst.m_expect_resp_count - ret.count;
	ret.row_count  = ( (int)( ret.count - 1 ) % cur_burst.m_row_length ) + 1;
	ret.row_remain = cur_burst.m_row_length - ret.row_count;

	if ( ret.remain == 0 ) {
            // point to the next open resp or the end
	    // the iterator will remain valid because std::list only invalidates
	    // iterators when an element is deleted, and an burst with incomplete
	    // responses cannot be erased until at least another call to this method
	    while ( m_first_open_resp->complete_resp() ) {
		if ( ++m_first_open_resp == m_burst.end() )
		    break;
	    }
	}

	purge();
	return ret;
    }

private:
    bool m_tl1_n_tl2;
};

struct ocp_txn_position: public tlm_utils::instance_specific_extension<ocp_txn_position> {
    ocp_txn_track_base::txn_position req_position;
    ocp_txn_track_base::txn_position dh_position;
    ocp_txn_track_base::txn_position resp_position;
    void clear() {
	req_position = dh_position = resp_position = ocp_txn_track_base::txn_position();
    }
};
}
