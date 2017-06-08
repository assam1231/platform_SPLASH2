//////////////////////////////////////////////////////////
//
//  (c) Copyright OCP-IP 2003, 2004, 2005, 2008
//  OCP-IP Confidential and Proprietary
//  $Id:
//
//  Structures to pass data to and from the OCP Channel
//  for both the TL1 and TL2 OCP specific channels.
//
// Joe Chou, Sonics Inc.
// Alan Kamas, for Sonics Inc.
// Yann Bajot, Prosilog
// Stephane Guntz, Prosilog
//
// 10/01/2003 Original revision
// 06/23/2004 Added TL2 specific Request and Response Structures
//////////////////////////////////////////////////////////

#ifndef __OCP_GLOBALS_LEGACY_H__
#define __OCP_GLOBALS_LEGACY_H__

// needed for gcc 3.3:
//using namespace std;

// -------------------------------------------------
// OCP 2.0
// -------------------------------------------------

// MCmd encoding
enum OCPMCmdType {
    OCP_MCMD_IDLE = 0,
    OCP_MCMD_WR,
    OCP_MCMD_RD,
    OCP_MCMD_RDEX,
    OCP_MCMD_RDL,
    OCP_MCMD_WRNP,
    OCP_MCMD_WRC,
    OCP_MCMD_BCST
};

// SResp encoding
enum OCPSRespType {
    OCP_SRESP_NULL = 0,
    OCP_SRESP_DVA,
    OCP_SRESP_FAIL,
    OCP_SRESP_ERR
};

// MBurstSeq encoding
enum OCPMBurstSeqType {
    OCP_MBURSTSEQ_INCR = 0,
    OCP_MBURSTSEQ_DFLT1,
    OCP_MBURSTSEQ_WRAP,
    OCP_MBURSTSEQ_DFLT2,
    OCP_MBURSTSEQ_XOR,
    OCP_MBURSTSEQ_STRM,
    OCP_MBURSTSEQ_UNKN,
    OCP_MBURSTSEQ_BLCK,
    OCP_MBURSTSEQ_RESERVED
};


/////////////////////////////////////////////////////////////
// DataHSGrp holds the dataflow signals for a data hand shake
// TL1 Only
/////////////////////////////////////////////////////////////
template <class Td>
class OCPDataHSGrp
{
  public:
    typedef Td DataType;

  public:
    //
    DataType     MData;
    unsigned int MDataThreadID;
    unsigned int MDataByteEn;
    unsigned int MDataInfo;
    
    // OCP 2.2 2-D Burst
    bool MDataRowLast;

    // OCP 2.1 Tag
    unsigned int MDataTagID;

    // OCP 2.0 Burst
    bool         MDataLast;

    // synchronization signal
    bool         MDataValid;
    bool         MDataValidSaved;

    // constructor
    OCPDataHSGrp(void) { reset(); }

    // define a copy constructor
    OCPDataHSGrp(const OCPDataHSGrp& src)
      : MData(src.MData),
        MDataThreadID(src.MDataThreadID),
        MDataByteEn(src.MDataByteEn),
        MDataInfo(src.MDataInfo),
        MDataRowLast(src.MDataRowLast),
        MDataTagID(src.MDataTagID),
        MDataLast(src.MDataLast),
        MDataValid(src.MDataValid),
        MDataValidSaved(src.MDataValidSaved)
    {}

    // define a copy assignment
    OCPDataHSGrp& operator=(const OCPDataHSGrp& rhs)
    {
        if (this == &rhs) { return *this; }
        copy(rhs);
        return *this;
    }

    // define a explicit copy
    void copy(const OCPDataHSGrp& src)
    {
        MData = src.MData;
        MDataThreadID = src.MDataThreadID;
        MDataByteEn = src.MDataByteEn;
        MDataInfo = src.MDataInfo;
	MDataTagID = src.MDataTagID;
        MDataLast = src.MDataLast;
        MDataRowLast = src.MDataRowLast;
        MDataValid = src.MDataValid;
	MDataValidSaved = src.MDataValidSaved;
    }

    void reset(void)
    {
        MData = 0;
        MDataThreadID = 0;
        MDataByteEn = 0xFFFFFFFF;
        MDataInfo = 0;
	MDataTagID = 0;
        MDataLast = true;
        MDataRowLast=true;
        MDataValid = false;
        MDataValidSaved = false;
    }

    std::string print(void) const 
    { 
        assert(0); 
        return "print() not support for class OCPDataHSGrp";
    }

	//operator overloading, so that class can be passed through ports 
	inline bool operator == (const OCPDataHSGrp& rhs) const {
	  return ((rhs.MData == MData)
		  && (MDataThreadID == rhs.MDataThreadID)  
		  && (MDataByteEn == rhs.MDataByteEn)  
		  && (MDataInfo == rhs.MDataInfo) 
		  && (MDataTagID == MDataTagID)
		  && (MDataLast == rhs.MDataLast)  
          && (MDataRowLast == rhs.MDataRowLast)
                  && (MDataValid == rhs.MDataValid)
		  && (MDataValidSaved == rhs.MDataValidSaved));
	}

	inline friend std::ostream& operator << ( std::ostream& os, OCPDataHSGrp const & v)  {
//		os << " -MData: "<<v.MData << " -MDataThreadID: "<<v.MDataThreadID << " -MDataByteEn: "<<v.MDataByteEn << " -MDataInfo: "<<v.MDataInfo<< " -MDataTagID: " v.MDataTagID<< " -MDataLast: " v.MDataLast<< " -MDataValid: "<<v.MDataValid<< " ";
//		os << "toto" ;
		return os;
	}

#ifdef OSCI20
	inline friend void sc_trace( sc_core::sc_trace_file *tf, const OCPDataHSGrp& v, const sc_string& NAME)   {
#else
inline friend void sc_trace( sc_core::sc_trace_file *tf, const OCPDataHSGrp& v, const std::string& NAME)   {
#endif
		sc_core::sc_trace(tf, v.MData, NAME+".MData");
		sc_core::sc_trace(tf, v.MDataThreadID, NAME+".MDataThreadID");
		sc_core::sc_trace(tf, v.MDataByteEn, NAME+".MDataByteEn");
		sc_core::sc_trace(tf, static_cast<long unsigned int>(v.MDataInfo), NAME+".MDataInfo");
		sc_core::sc_trace(tf, v.MDataTagID, NAME+".MDataTagID");
		sc_core::sc_trace(tf, v.MDataLast, NAME+".MDataLast");
        sc_core::sc_trace(tf, v.MDataRowLast, NAME+".MDataRowLast");
		sc_core::sc_trace(tf, v.MDataValid, NAME+".MDataValid");	
	}
};

template <class Td, class Ta> class OCPTL2RequestGrp;
///////////////////////////////////////////////
// RequestGrp holds the OCP request information 
///////////////////////////////////////////////
template <class Td, class Ta>
class OCPRequestGrp
{
  public:
    typedef Td DataType;
    typedef Ta AddrType;

  public:
    //
    OCPMCmdType   MCmd;
    OCPMCmdType   MCmdSaved;
    AddrType      MAddr;
    unsigned int  MAddrSpace;
    unsigned int  MByteEn;
    unsigned int  MThreadID;
    unsigned int  MConnID;
    sc_dt::uint64  MReqInfo;
    
    // OCP 2.2 2-D Burst
    unsigned int MBlockHeight;
    unsigned int MBlockStride;
    bool MReqRowLast;    
    
    // OCP 2.1 tag
    unsigned int  MTagID;
    bool          MTagInOrder;

    // for OCP 2.0 Burst
    unsigned int     MAtomicLength;
    unsigned int     MBurstLength;
    bool             MBurstPrecise;
    OCPMBurstSeqType MBurstSeq;
    bool             MBurstSingleReq;
    bool             MReqLast;

    // request group with data?
    bool          HasMData;
    DataType      MData;        // TL1 Only     
    sc_dt::uint64  MDataInfo;
    unsigned int MDataTagID;
    DataType      *MDataPtr;    // TL2 Only

    mutable long long TrHandle;

    // constructor
    OCPRequestGrp(bool has_mdata = true) { reset(has_mdata); }

    // define a copy constructor
    OCPRequestGrp(const OCPRequestGrp& src)
      : MCmd(src.MCmd),
        MCmdSaved(src.MCmdSaved),
        MAddr(src.MAddr),
        MAddrSpace(src.MAddrSpace),
        MByteEn(src.MByteEn),
        MThreadID(src.MThreadID),
        MConnID(src.MConnID),
        MReqInfo(src.MReqInfo),
        MBlockHeight(src.MBlockHeight),
        MBlockStride(src.MBlockStride),
        MReqRowLast(src.MReqRowLast),
        MTagID(src.MTagID),
        MTagInOrder(src.MTagInOrder),
        // MBurst(src.MBurst),  // Old OCP1.0 - no longer supported
        MAtomicLength(src.MAtomicLength),
        MBurstLength(src.MBurstLength),
        MBurstPrecise(src.MBurstPrecise),
        MBurstSeq(src.MBurstSeq),
        MBurstSingleReq(src.MBurstSingleReq),
        MReqLast(src.MReqLast),
        HasMData(src.HasMData),
        MData(src.MData),
        MDataInfo(src.MDataInfo),
        MDataTagID(src.MDataTagID),      
        MDataPtr(src.MDataPtr),
	TrHandle(src.TrHandle)
    {}

    // define a constructor from TL2 Specific Request Group
    OCPRequestGrp(const OCPTL2RequestGrp<Td,Ta>& src)
      : MCmd(src.MCmd),
        MCmdSaved(src.MCmd),
        MAddr(src.MAddr),
        MAddrSpace(src.MAddrSpace),
        MByteEn(src.MByteEn),
        MThreadID(src.MThreadID),
        MConnID(src.MConnID),
        MReqInfo(src.MReqInfo),
        MBlockHeight(src.MBlockHeight),
        MBlockStride(src.MBlockStride),
		MReqRowLast(src.LastOfRow!=0),
        MTagID(src.MTagID),
        MTagInOrder(src.MTagInOrder),
        MAtomicLength(1),
        MBurstLength(src.MBurstLength),
        MBurstPrecise(src.MBurstPrecise),
        MBurstSeq(src.MBurstSeq),
        MBurstSingleReq(src.MBurstSingleReq),
        MReqLast(src.LastOfBurst),
        HasMData(true),
        MData(0),
        MDataInfo(src.MDataInfo),
        MDataTagID(src.MDataTagID),
        MDataPtr(src.MDataPtr),
	TrHandle(src.TrHandle)
    {
        if (src.DataLength > 0) {
            // Use the pointer values if provided
            if (src.MByteEnPtr != NULL) {
                MByteEn = src.MByteEnPtr[0];
            }
            if (src.MDataInfoPtr != NULL) {
                MDataInfo = src.MDataInfoPtr[0];
            }
        }
    }


    // define a copy assignment
    OCPRequestGrp& operator= (const OCPRequestGrp& rhs)
    {
        if (this == &rhs) { return *this; }
        copy(rhs);
        return *this;
    }

    // define a explicit copy
    void copy(const OCPRequestGrp& src)
    {
        MCmd = src.MCmd;
	MCmdSaved = src.MCmdSaved;
        MAddr = src.MAddr;
        MAddrSpace = src.MAddrSpace;
        MByteEn = src.MByteEn;
        MThreadID = src.MThreadID;
        MConnID = src.MConnID;
        MReqInfo = src.MReqInfo;
	MTagID = src.MTagID;
	MTagInOrder = src.MTagInOrder;
        // MBurst = src.MBurst; // Old OCP 1.0 - no longer supported
        MAtomicLength = src.MAtomicLength;
        MBurstLength = src.MBurstLength;
        MBlockHeight = src.MBlockHeight;
        MBlockStride = src.MBlockStride;
        MBurstPrecise = src.MBurstPrecise;
        MBurstSeq = src.MBurstSeq;
        MBurstSingleReq = src.MBurstSingleReq;
        MReqLast = src.MReqLast;
        MReqRowLast = src.MReqRowLast;
        HasMData = src.HasMData;
        MData = src.MData;
        MDataInfo = src.MDataInfo;
	MDataTagID = src.MDataTagID;
        MDataPtr = src.MDataPtr;
	TrHandle = src.TrHandle;
    }

    // define a converter from the TL2 Specific Request Group
    void copyFrom(const OCPTL2RequestGrp<Td,Ta>& src)
    {
        // Since the TL1 Request Group has more signals than the TL2 group,
        // Some TL1 values are set to default
        MCmd = src.MCmd;
	MCmdSaved = src.MCmd;
        MAddr = src.MAddr;
        MAddrSpace = src.MAddrSpace;
        MByteEn = src.MByteEn;
        MThreadID = src.MThreadID;
        MConnID = src.MConnID;
        MReqInfo = src.MReqInfo;
	MTagID = src.MTagID;
	MTagInOrder = src.MTagInOrder;
        MAtomicLength = 1;
        MBurstLength = src.MBurstLength;
        MBlockHeight = src.MBlockHeight;
        MBlockStride = src.MBlockStride;
        MBurstPrecise = src.MBurstPrecise;
        MBurstSeq = src.MBurstSeq;
        MBurstSingleReq = src.MBurstSingleReq;
        MReqLast = src.LastOfBurst;
		MReqRowLast = src.LastOfRow!=0;
        HasMData = true;
        MData = 0;
        MDataInfo = src.MDataInfo;
        MDataTagID = src.MDataTagID;
        MDataPtr = src.MDataPtr;
	TrHandle = src.TrHandle;

        // Use the pointer values if provided
        if (src.DataLength > 0) {
            if (src.MByteEnPtr != NULL) {
                MByteEn = src.MByteEnPtr[0];
            }
            if (src.MDataInfoPtr != NULL) {
                MDataInfo = src.MDataInfoPtr[0];
            }
        }
    }

    void reset(bool has_mdata = true)
    {
        MCmd = OCP_MCMD_IDLE;
	MCmdSaved = OCP_MCMD_IDLE;
        MAddr = 0;
        MAddrSpace = 0;
        MByteEn = 0xFFFFFFFF;
        MThreadID = 0;
        MConnID = 0;
        MReqInfo = 0;
	MTagID = 0;
	MTagInOrder = 0;
        // MBurst = OCP_MBURST_LAST; // Old OCP 1.0 - no longer supported
        MAtomicLength = 1;
        MBurstLength = 1;
        MBlockHeight = 1;
        MBlockStride = 1;
        MBurstPrecise = true;
        MBurstSeq = OCP_MBURSTSEQ_INCR;
        MBurstSingleReq = false;
        MReqLast = true;
        MReqRowLast = true;
        HasMData = has_mdata;
        MData = 0;
        MDataInfo = 0;
        MDataTagID = 0;
        MDataPtr = NULL;
	TrHandle = -1;
    }

    std::string print(void) const 
    { 
        assert(0); 
        return "print() not supported for class OCPRequestGrp";
    }

	//operator overloading, so that class can be passed through ports 
	inline bool operator == (const OCPRequestGrp& rhs) const {
	  return ( (MCmd == rhs.MCmd)
		          && (MCmdSaved == rhs.MCmdSaved)
			  && (MAddr == rhs.MAddr)
			  && (MAddrSpace == rhs.MAddrSpace)
			  && (MByteEn == rhs.MByteEn)
			  && (MThreadID == rhs.MThreadID)
			  && (MConnID == rhs.MConnID)
			  && (MReqInfo == rhs.MReqInfo)
			  && (MTagID == rhs.MTagID)
			  && (MTagInOrder == rhs.MTagInOrder)
			  && (MAtomicLength == rhs.MAtomicLength)
			  && (MBurstLength == rhs.MBurstLength)
			  && (MBlockHeight == rhs.MBlockHeight)
			  && (MBlockStride == rhs.MBlockStride)
			  && (MBurstPrecise == rhs.MBurstPrecise)
			  && (MBurstSeq == rhs.MBurstSeq)
			  && (MBurstSingleReq == rhs.MBurstSingleReq)
			  && (MReqLast == rhs.MReqLast)
              && (MReqRowLast == rhs.MReqRowLast)
			  && (HasMData == rhs.HasMData)
			  && (MData == rhs.MData)
			  && (MDataInfo == rhs.MDataInfo)
			  && (MDataTagID == rhs.MDataTagID)
			  && (MDataPtr == rhs.MDataPtr));
	}

	inline friend std::ostream& operator << ( std::ostream & os, OCPRequestGrp const & v)   {
		os << " -MCmd: "<<v.MCmd << " -MAddr: "<<v.MAddr << " -MAddrSpace: "<<v.MAddrSpace
			<< " -MByteEn: "<<v.MByteEn <<" -MThreadID: "<<v.MThreadID<<	" -MConnID: "<<v.MConnID
			<<" -MReqInfo: "<<v.MReqInfo<<" -MAtomicLength: "<<v.MAtomicLength
		        <<" -MTagID: "<<v.MTagID
		        <<" -MTagInOrder: "<<v.MTagInOrder
			<<" -MBurstLength: "<<v.MBurstLength<<" -MBlockHeight: "<<v.MBlockHeight<<" -MBlockStride: "<<v.MBlockStride
            <<" -MBurstPrecise: "<<v.MBurstPrecise
			<<" -MBurstSeq: "<<v.MBurstSeq<<" -MBurstSingleReq: "<<v.MBurstSingleReq
			<<" -MReqLast: "<<v.MReqLast<<" -MReqRowLast: "<<v.MReqRowLast<<" -HasMData: "<<v.HasMData
			<<" -MData: "<<v.MData<<" -MDataInfo: "<<v.MDataInfo<<" -MDataTagID: "<<v.MDataTagID
			<<" -MDataPtr: "<<v.MDataPtr<< " ";
		return os;
	}
#ifdef OSCI20
	inline friend void sc_trace( sc_core::sc_trace_file *tf, const OCPRequestGrp& v, const sc_string& NAME)   {
#else
	inline friend void sc_trace( sc_core::sc_trace_file *tf, const OCPRequestGrp& v, const std::string& NAME)   {
#endif
		sc_core::sc_trace(tf, v.MCmd, NAME+".MCmd");
		sc_core::sc_trace(tf, v.MAddr, NAME+".MAddr");
		sc_core::sc_trace(tf, v.MAddrSpace, NAME+".MAddrSpace");
		sc_core::sc_trace(tf, v.MByteEn, NAME+".MByteEn");
	        sc_core::sc_trace(tf, v.MThreadID, NAME+".MThreadID");
		sc_core::sc_trace(tf, v.MConnID, NAME+".MConnID");
		sc_core::sc_trace(tf, static_cast<long unsigned int>(v.MReqInfo), NAME+".MReqInfo");	
		sc_core::sc_trace(tf, v.MTagID, NAME+".MTagID");
		sc_core::sc_trace(tf, v.MTagInOrder, NAME+".MTagInOrder");
		sc_core::sc_trace(tf, v.MAtomicLength, NAME+".MAtomicLength");
		sc_core::sc_trace(tf, v.MBurstLength, NAME+".MBurstLength");
		sc_core::sc_trace(tf, v.MBlockHeight, NAME+".MBlockHeight");
		sc_core::sc_trace(tf, v.MBlockStride, NAME+".MBlockStride");
		sc_core::sc_trace(tf, v.MBurstPrecise, NAME+".MBurstPrecise");
		sc_core::sc_trace(tf, v.MBurstSeq, NAME+".MBurstSeq");
		sc_core::sc_trace(tf, v.MBurstSingleReq, NAME+".MBurstSingleReq");
		sc_core::sc_trace(tf, v.MReqLast, NAME+".MReqLast");	
		sc_core::sc_trace(tf, v.MReqRowLast, NAME+".MReqRowLast");	
		sc_core::sc_trace(tf, v.HasMData, NAME+".HasMData");
		sc_core::sc_trace(tf, v.MData, NAME+".MData");
		sc_core::sc_trace(tf, static_cast<long unsigned int>(v.MDataInfo), NAME+".MDataInfo");
		sc_core::sc_trace(tf, v.MDataTagID, NAME+".MDataTagID");
		// sc_trace(tf, v.MDataPtr, NAME+".MDataPtr");	
	}
};

template <class Td> class OCPTL2ResponseGrp;
/////////////////////////////////////////////////
// ResponseGrp holds the OCP response information
/////////////////////////////////////////////////
template <class Td>
class OCPResponseGrp
{
  public:
    typedef Td DataType;

  public:
    OCPSRespType SResp;
    OCPSRespType SRespSaved;
    DataType     SData;         // TL1 Only
    DataType     *SDataPtr;     // TL2 Only
    sc_dt::uint64 SDataInfo;
    unsigned int STagID;
    unsigned int STagInOrder;
    unsigned int SThreadID;
    sc_dt::uint64 SRespInfo;
    bool         SRespLast;
    bool         SRespRowLast;

    mutable long long TrHandle;

    // constructor
    OCPResponseGrp(void) { reset(); }
 
    // define a copy constructor
    OCPResponseGrp(const OCPResponseGrp& src)
      : SResp(src.SResp),
        SRespSaved(src.SRespSaved),
        SData(src.SData),
        SDataPtr(src.SDataPtr),
        SDataInfo(src.SDataInfo),
        STagID(src.STagID),
        STagInOrder(src.STagInOrder),
        SThreadID(src.SThreadID),
        SRespInfo(src.SRespInfo),
        SRespLast(src.SRespLast),
        SRespRowLast(src.SRespRowLast),
	TrHandle(src.TrHandle)
    {}

    // define a constructor from the TL2 Specific Response Group
    OCPResponseGrp(const OCPTL2ResponseGrp<Td>& src)
      : SResp(src.SResp),
        SRespSaved(src.SResp),
        SData(0),
        SDataPtr(src.SDataPtr),
        SDataInfo(src.SDataInfo),
        STagID(src.STagID),
        STagInOrder(src.STagInOrder),
        SThreadID(src.SThreadID),
        SRespInfo(src.SRespInfo),
        SRespLast(true),
		SRespRowLast(src.LastOfRow!=0),
	TrHandle(src.TrHandle)
    {}
 
    // define a copy assignment
    OCPResponseGrp& operator= (const OCPResponseGrp& rhs)
    {
        if (this == &rhs) { return *this; }
        copy(rhs);
        return *this;
    }

    // define a explicit copy
    void copy(const OCPResponseGrp& src)
    {
        SResp = src.SResp;
	SRespSaved = src.SRespSaved;
        SData = src.SData;
        SDataPtr = src.SDataPtr;
        SDataInfo = src.SDataInfo;
	STagID = src.STagID;
	STagInOrder = src.STagInOrder;
        SThreadID = src.SThreadID;
        SRespInfo = src.SRespInfo;
        SRespLast = src.SRespLast;
        SRespRowLast = src.SRespRowLast;
	TrHandle = src.TrHandle;
    }

    // define a converter from the TL2 specific Response group
    void copyFrom(const OCPTL2ResponseGrp<Td>& src)
    {
        SResp = src.SResp;
	SRespSaved = src.SResp;
        SData = 0;
        SDataPtr = src.SDataPtr;
        SDataInfo = src.SDataInfo;
	STagID = src.STagID;
	STagInOrder = src.STagInOrder;
        SThreadID = src.SThreadID;
        SRespInfo = src.SRespInfo;
        SRespLast = true;
		SRespRowLast = src.LastOfRow!=0;
	TrHandle = src.TrHandle;
    }

    void reset(void)
    {
        SResp = OCP_SRESP_NULL;
	SRespSaved = OCP_SRESP_NULL;
        SData = 0;
        SDataPtr = NULL;
        SDataInfo = 0;
	STagID = 0;
	STagInOrder = 0;
        SThreadID = 0;
        SRespInfo = 0;
        SRespLast = true;
	TrHandle = -1;
    }

    std::string print(void) const 
    { 
        assert(0); 
        return "print() not supported for class OCPResponseGrp";
    }

	//operator overloading, so that class can be passed through ports 
	inline bool operator == (const OCPResponseGrp& rhs) const {
	  return ( (SResp == rhs.SResp)
		   && (SRespSaved == rhs.SRespSaved)
		   && (SData == rhs.SData)
		   && (SDataPtr == rhs.SDataPtr)
		   && (SDataInfo == rhs.SDataInfo)
		   && (STagID == rhs.STagID)
		   && (STagInOrder == rhs.STagInOrder)
		   && (SThreadID == rhs.SThreadID)
		   && (SRespInfo == rhs.SRespInfo)
		   && (SRespLast == rhs.SRespLast)
		   && (SRespRowLast == rhs.SRespRowLast));
	}

	inline friend std::ostream& operator << ( std::ostream& os, OCPResponseGrp const & v)  {
		os << " -SResp: "<<v.SResp << " -SData: "<<v.SData << " -SDataPtr: "<<v.SDataPtr
			<< " -SDataInfo: "<<v.SDataInfo << " -STagID: "<<v.STagID 
		        << " -STagInOrder: "<<v.STagInOrder <<" -SThreadID: "<<v.SThreadID
			<<" -SRespInfo: "<<v.SRespInfo	<<" -SRespLast: "<<v.SRespLast<<" -SRespRowLast: "<<v.SRespRowLast<< " ";
		return os;
	}
#ifdef OSCI20
	inline friend void sc_trace( sc_core::sc_trace_file *tf, const OCPResponseGrp& v, const sc_string& NAME)   {
#else
	inline friend void sc_trace( sc_core::sc_trace_file *tf, const OCPResponseGrp& v, const std::string& NAME)   {
#endif
		sc_core::sc_trace(tf, v.SResp, NAME+".SResp");
		sc_core::sc_trace(tf, v.SData, NAME+".SData");
		// sc_trace(tf, v.SDataPtr, NAME+".SDataPtr");
		sc_core::sc_trace(tf, static_cast<long unsigned int>(v.SDataInfo), NAME+".SDataInfo");
		sc_core::sc_trace(tf, v.STagID, NAME+".STagID");
		sc_core::sc_trace(tf, v.STagInOrder, NAME+".STagInOrder");
		sc_core::sc_trace(tf, v.SThreadID, NAME+".SThreadID");
		sc_core::sc_trace(tf, static_cast<long unsigned int>(v.SRespInfo), NAME+".SRespInfo");	
		sc_core::sc_trace(tf, v.SRespLast, NAME+".SRespLast");
		sc_core::sc_trace(tf, v.SRespRowLast, NAME+".SRespRowLast");        
	}
};

//===============================================================//
// Code Below this line used by the performance TL2 Channel only
//===============================================================//

////////////////////////////////////////////////////
// TL2 RequestGrp holds the OCP request information 
////////////////////////////////////////////////////
template <class Td, class Ta>
class OCPTL2RequestGrp
{
  public:
    typedef Td DataType;
    typedef Ta AddrType;

  public:
    // OCP Basics
    OCPMCmdType   MCmd;
    AddrType      MAddr;
    DataType      *MDataPtr;
    unsigned int  DataLength;
    // OCP Basics +
    unsigned int  MAddrSpace;
    unsigned int  MByteEn;
    unsigned int  *MByteEnPtr;
    sc_dt::uint64  MReqInfo;
    unsigned int  MTagID;
    unsigned int  MTagInOrder;
    sc_dt::uint64  MDataInfo;    
    sc_dt::uint64  *MDataInfoPtr;
    unsigned int  MDataTagID;    
    // OCP Threads
    unsigned int  MThreadID;
    unsigned int  MConnID;
    // OCP Burst Information
    unsigned int MBurstLength;
    unsigned int MBlockHeight;
    unsigned int MBlockStride;
    bool MBurstPrecise;
    OCPMBurstSeqType MBurstSeq;
    bool MBurstSingleReq;

    // Partial Burst Help
    bool LastOfBurst;
	unsigned int LastOfRow;
    // system level transaction recording
    mutable long long TrHandle;

    // constructor
    OCPTL2RequestGrp() 
    { 
        reset(); 
    }

    // define a copy constructor
    OCPTL2RequestGrp(const OCPTL2RequestGrp& src)
      : MCmd(src.MCmd),
        MAddr(src.MAddr),
        MDataPtr(src.MDataPtr),
        DataLength(src.DataLength),
        MAddrSpace(src.MAddrSpace),
        MByteEn(src.MByteEn),
        MByteEnPtr(src.MByteEnPtr),
        MReqInfo(src.MReqInfo),
        MTagID(src.MTagID),
        MTagInOrder(src.MTagInOrder),
        MDataInfo(src.MDataInfo),
        MDataInfoPtr(src.MDataInfoPtr),
        MDataTagID(src.MDataTagID),
        MThreadID(src.MThreadID),
        MConnID(src.MConnID),
        MBurstLength(src.MBurstLength),
        MBlockHeight(src.MBlockHeight),
        MBlockStride(src.MBlockStride),
        MBurstPrecise(src.MBurstPrecise),
        MBurstSeq(src.MBurstSeq),
        MBurstSingleReq(src.MBurstSingleReq),
        LastOfBurst(src.LastOfBurst),
		LastOfRow(src.LastOfRow),
	TrHandle(src.TrHandle)
    {}

    // define a constructor based on TL1 Request Group
    OCPTL2RequestGrp(const OCPRequestGrp<Td,Ta>& src, 
            unsigned int ReqChunkLength=0, 
            bool ReqChunkLast=true)
      : MCmd(src.MCmd),
        MAddr(src.MAddr),
        MDataPtr(src.MDataPtr),
        DataLength(ReqChunkLength),
        MAddrSpace(src.MAddrSpace),
        MByteEn(src.MByteEn),
        MByteEnPtr(NULL),
        MReqInfo(src.MReqInfo),
        MTagID(src.MTagID),
        MTagInOrder(src.MTagInOrder),
        MDataInfo(src.MDataInfo),
        MDataInfoPtr(NULL),
        MDataTagID(src.MDataTagID),
        MThreadID(src.MThreadID),
        MConnID(src.MConnID),
        MBurstLength(src.MBurstLength),
        MBlockHeight(src.MBlockHeight),
        MBlockStride(src.MBlockStride),        
        MBurstPrecise(src.MBurstPrecise),
        MBurstSeq(src.MBurstSeq),
        MBurstSingleReq(src.MBurstSingleReq),
        LastOfBurst(ReqChunkLast),
		LastOfRow((src.MReqRowLast)? 1 :0),
	TrHandle(src.TrHandle)
    {}
    
    // define a copy assignment
    OCPTL2RequestGrp& operator= (const OCPTL2RequestGrp& rhs)
    {
        if (this == &rhs) { return *this; }
        copy(rhs);
        return *this;
    }

    // define a explicit copy
    void copy(const OCPTL2RequestGrp& src)
    {
        MCmd = src.MCmd;
        MAddr = src.MAddr;
        MDataPtr = src.MDataPtr;
        DataLength = src.DataLength;
        MAddrSpace = src.MAddrSpace;
        MByteEn = src.MByteEn;
        MByteEnPtr = src.MByteEnPtr;
        MReqInfo = src.MReqInfo;
	MTagID = src.MTagID;
	MTagInOrder = src.MTagInOrder;
        MDataInfo = src.MDataInfo;
        MDataTagID = src.MDataTagID;
        MDataInfoPtr = src.MDataInfoPtr;
        MThreadID = src.MThreadID;
        MConnID = src.MConnID;
        MBurstLength = src.MBurstLength;
        MBlockHeight = src.MBlockHeight;
        MBlockStride = src.MBlockStride;
        MBurstPrecise = src.MBurstPrecise;
        MBurstSeq = src.MBurstSeq;
        MBurstSingleReq = src.MBurstSingleReq;
        LastOfBurst = src.LastOfBurst;
		LastOfRow =src.LastOfRow;
	TrHandle = src.TrHandle;
    };

    // define a converter from TL1 Request
    void copyFrom(const OCPRequestGrp<Td,Ta>& src,
            unsigned int ReqChunkLength=0, 
            bool ReqChunkLast=true)
    {
        MCmd = src.MCmd;
        MAddr = src.MAddr;
        MDataPtr = src.MDataPtr;
        DataLength = ReqChunkLength;
        MAddrSpace = src.MAddrSpace;
        MByteEn = src.MByteEn;
        MByteEnPtr = NULL;
        MReqInfo = src.MReqInfo;
	MTagID = src.MTagID;
	MTagInOrder = src.MTagInOrder;
        MDataInfo = src.MDataInfo;
        MDataInfoPtr = NULL; 
        MDataTagID = src.MDataTagID;
        MThreadID = src.MThreadID;
        MConnID = src.MConnID;
        MBurstLength = src.MBurstLength;
        MBlockHeight = src.MBlockHeight;
        MBlockStride = src.MBlockStride;
        MBurstPrecise = src.MBurstPrecise;
        MBurstSeq = src.MBurstSeq;
        MBurstSingleReq = src.MBurstSingleReq;
        LastOfBurst = ReqChunkLast;
		LastOfRow= (src.MReqRowLast)? 1 : 0;
	TrHandle = src.TrHandle;
    };

    void reset()
    {
        MCmd = OCP_MCMD_IDLE;
        MAddr = 0;
        MDataPtr = NULL;
        DataLength = 0;
        MAddrSpace = 0;
        MByteEn = 0xFFFFFFFF;
        MByteEnPtr = NULL;
        MReqInfo = 0;
	MTagID = 0;
	MTagInOrder = 0;
        MDataInfo = 0;
        MDataInfoPtr = NULL;
	MDataTagID = 0;
        MThreadID = 0;
        MConnID = 0;
        MBurstLength = 0;
        MBlockHeight = 0;
        MBlockStride = 0;
        MBurstPrecise = false;
        MBurstSeq = OCP_MBURSTSEQ_INCR;
        MBurstSingleReq = false;
        LastOfBurst = true;
		LastOfRow =0;
	TrHandle = -1;
    }

    std::string print(void) const 
    { 
        assert(0); 
        return "print() not supported for class OCPTL2RequestGrp";
    }

    //operator overloading, so that class can be passed through ports 
    inline bool operator == (const OCPTL2RequestGrp& rhs) const 
    {
      if ((MCmd == rhs.MCmd)
	  && (MAddr == rhs.MAddr)
	  && (DataLength == rhs.DataLength)
	  && (MAddrSpace == rhs.MAddrSpace)
	  && (MByteEn == rhs.MByteEn)
	  && (MReqInfo == rhs.MReqInfo)
	  && (MTagID == rhs.MTagID)
	  && (MTagInOrder == rhs.MTagInOrder)
	  && (MDataInfo == rhs.MDataInfo)
	  && (MDataTagID == rhs.MDataTagID)
	  && (MThreadID == rhs.MThreadID)
	  && (MConnID == rhs.MConnID)
	  && (MBurstLength == rhs.MBurstLength)
	  && (MBlockHeight == rhs.MBlockHeight)
	  && (MBlockStride == rhs.MBlockStride)
	  && (MBurstPrecise == rhs.MBurstPrecise)
	  && (MBurstSeq == rhs.MBurstSeq)
	  && (MBurstSingleReq == rhs.MBurstSingleReq)
	  && (LastOfBurst == rhs.LastOfBurst)
	  && (LastOfRow == rhs.LastOfRow)) 
        { 
            // All of the member values are the same. 
            // Now compare the data pointer fields 
            if (DataLength >0) 
            {
                // Compare MDataPtr (if used)
                if (MDataPtr != NULL) {
                    if (rhs.MDataPtr == NULL) {
                        // Only one side using MDataPtr - no match
                        return false;
                    }
                    // Both are using pointer - check values
                    for (unsigned int i=0; i<DataLength; i++)
                    {
                        if (MDataPtr[i] != rhs.MDataPtr[i])
                        {
                            // Data not the same
                            return false;
                        }
                    }
                } else {
                    if (rhs.MDataPtr != NULL) {
                        // Only one side using MDataPtr - no match
                        return false;
                    }
                }

                // Compare MByteEnPtr (if used)
                if (MByteEnPtr != NULL) {
                    if (rhs.MByteEnPtr == NULL) {
                        // Only one side using MByteEnPtr - no match
                        return false;
                    }
                    // Both are using pointer - check values
                    for (unsigned int i=0; i<DataLength; i++)
                    {
                        if (MByteEnPtr[i] != rhs.MByteEnPtr[i])
                        {
                            // Data not the same
                            return false;
                        }
                    }
                } else {
                    if (rhs.MByteEnPtr != NULL) {
                        // Only one side using MByteEnPtr - no match
                        return false;
                    }
                }

                // Compare MDataInfoPtr (if used)
                if (MByteEnPtr != NULL) {
                    if (rhs.MDataInfoPtr == NULL) {
                        // Only one side using MDataInfoPtr - no match
                        return false;
                    }
                    // Both are using pointer - check values
                    for (unsigned int i=0; i<DataLength; i++)
                    {
                        if (MDataInfoPtr[i] != rhs.MDataInfoPtr[i])
                        {
                            // Data not the same
                            return false;
                        }
                    }
                } else {
                    if (rhs.MDataInfoPtr != NULL) {
                        // Only one side using MDataInfoPtr - no match
                        return false;
                    }
                }
            }
            // Everything's the same
            return true;
        }
        // Member values not the same
        return false;
    }

    inline friend std::ostream& operator << ( std::ostream & os, OCPTL2RequestGrp const & v)   
    {
        os << " -MCmd: " << v.MCmd 
           << " -MAddr: " << v.MAddr 
	   << " -MDataPtr: " << v.MDataPtr
           << " -DataLength: " << v.DataLength
           << " -MAddrSpace: " << v.MAddrSpace
	   << " -MByteEn: " << v.MByteEn 
	   << " -MByteEnPtr: " << v.MByteEnPtr 
	   << " -MReqInfo: " << v.MReqInfo
	   << " -MTagID: " << v.MTagID
	   << " -MTagInOrder: " << v.MTagInOrder
           << " -MDataInfo: " << v.MDataInfo
           << " -MDataInfoPtr: " << v.MDataInfoPtr
           << " -MDataTagID: " << v.MDataTagID
           << " -MThreadID: " << v.MThreadID
           << " -MConnID: " << v.MConnID
           << " -MBurstLength: " << v.MBurstLength
           << " -MBlockHeight: " << v.MBlockHeight
           << " -MBlockStride: " << v.MBlockStride
           << " -MBurstPrecise: " << v.MBurstPrecise
	   << " -MBurstSeq: " << v.MBurstSeq
           << " -MBurstSingleReq: " << v.MBurstSingleReq
           << " -LastOfBurst: " << v.LastOfBurst
		   << " -LastOfRow: " <<v.LastOfRow
	   << " -handle " << v.TrHandle <<" ";
	return os;
    }
#ifdef OSCI20
    inline friend void sc_trace( sc_core::sc_trace_file *tf, 
            const OCPTL2RequestGrp& v, const sc_string& NAME)   
#else
    inline friend void sc_trace( sc_core::sc_trace_file *tf, 
            const OCPTL2RequestGrp& v, const std::string& NAME)   
#endif
    {
        sc_core::sc_trace(tf, v.MCmd, NAME+".MCmd");
	sc_core::sc_trace(tf, v.MAddr, NAME+".MAddr");
	// sc_trace(tf, v.MDataPtr, NAME+".MDataPtr");	
	sc_core::sc_trace(tf, v.DataLength, NAME+".DataLength");	
	sc_core::sc_trace(tf, v.MAddrSpace, NAME+".MAddrSpace");
	sc_core::sc_trace(tf, v.MByteEn, NAME+".MByteEn");
	// sc_trace(tf, v.MByteEnPtr, NAME+".MByteEnPtr");
	sc_core::sc_trace(tf, static_cast<long unsigned int>(v.MReqInfo), NAME+".MReqInfo");	
	sc_core::sc_trace(tf, v.MTagID, NAME+".MTagID");	
	sc_core::sc_trace(tf, v.MTagInOrder, NAME+".MTagInOrder");	
	sc_core::sc_trace(tf, static_cast<long unsigned int>(v.MDataInfo), NAME+".MDataInfo");
	// sc_trace(tf, v.MDataInfoPtr, NAME+".MDataInfoPtr");
	sc_core::sc_trace(tf, v.MDataTagID, NAME+".MDataTagID");	
	sc_core::sc_trace(tf, v.MThreadID, NAME+".MThreadID");
	sc_core::sc_trace(tf, v.MConnID, NAME+".MConnID");
	sc_core::sc_trace(tf, v.MBurstLength, NAME+".MBurstLength");
	sc_core::sc_trace(tf, v.MBlockHeight, NAME+".MBlockHeight");
	sc_core::sc_trace(tf, v.MBlockStride, NAME+".MBlockStride");        
	sc_core::sc_trace(tf, v.MBurstPrecise, NAME+".MBurstPrecise");
	sc_core::sc_trace(tf, v.MBurstSeq, NAME+".MBurstSeq");
	sc_core::sc_trace(tf, v.MBurstSingleReq, NAME+".MBurstSingleReq");
	sc_core::sc_trace(tf, v.LastOfBurst, NAME+".LastOfBurst");
	sc_core::sc_trace(tf, v.LastOfRow, NAME+".LastOfRow");	
    }

};

//////////////////////////////////////////////////////////
// TL2 ResponseGrp holds TL2 specific response information
//////////////////////////////////////////////////////////
template <class Td>
class OCPTL2ResponseGrp
{
  public:
    typedef Td DataType;

  public:
    // OCP Basics
    OCPSRespType SResp;
    DataType     *SDataPtr;
    unsigned int DataLength;
    // OCP Basics +
    sc_dt::uint64 SDataInfo;
    unsigned int STagID;
    bool STagInOrder;
    sc_dt::uint64 SRespInfo;
    // OCP Threads
    unsigned int SThreadID;
    // Partial Burst Support
    bool         LastOfBurst;
	unsigned int LastOfRow;
    // system level transaction recording
    mutable long long TrHandle;

    // constructor
    OCPTL2ResponseGrp(void) 
    { 
        reset(); 
    }
 
    // define a copy constructor
    OCPTL2ResponseGrp(const OCPTL2ResponseGrp& src)
      : SResp(src.SResp),
        SDataPtr(src.SDataPtr),
        DataLength(src.DataLength),
        SDataInfo(src.SDataInfo),
        STagID(src.STagID),
        STagInOrder(src.STagInOrder),
        SRespInfo(src.SRespInfo),
        SThreadID(src.SThreadID),
        LastOfBurst(src.LastOfBurst),
		LastOfRow(src.LastOfRow),
	TrHandle(src.TrHandle)
    { }

    // define a constructor based on TL1 Response Group
    OCPTL2ResponseGrp(const OCPResponseGrp<Td>& src, 
            unsigned int RespChunkLength=0, 
            bool RespChunkLast=true)
      : SResp(src.SResp),
        SDataPtr(src.SDataPtr),
        DataLength(RespChunkLength),
        SDataInfo(src.SDataInfo),
        STagID(src.STagID),
        STagInOrder(src.STagInOrder),
        SRespInfo(src.SRespInfo),
        SThreadID(src.SThreadID),
        LastOfBurst(RespChunkLast),
		LastOfRow((src.SRespRowLast)? 1 : 0 ),
	TrHandle(src.TrHandle)
    { }
 
    // define a copy assignment
    OCPTL2ResponseGrp& operator= (const OCPTL2ResponseGrp& rhs)
    {
        if (this == &rhs) { return *this; }
        copy(rhs);
        return *this;
    }

    // define a explicit copy
    void copy(const OCPTL2ResponseGrp& src)
    {
        SResp = src.SResp;
        SDataPtr = src.SDataPtr;
        DataLength = src.DataLength;
        SDataInfo = src.SDataInfo;
        STagID = src.STagID;
	STagInOrder = src.STagInOrder;
        SRespInfo = src.SRespInfo;
        SThreadID = src.SThreadID;
        LastOfBurst = src.LastOfBurst;
		LastOfRow = src.LastOfRow;
        TrHandle = src.TrHandle;
    }

    // define a converter from TL1 Request Group
    void copyFrom(const OCPResponseGrp<Td>& src,
            unsigned int RespChunkLength =0,
            bool RespChunkLast =true)
    {
        SResp = src.SResp;
        SDataPtr = src.SDataPtr;
        DataLength = RespChunkLength;
        SDataInfo = src.SDataInfo;
        STagID = src.STagID;
	STagInOrder = src.STagInOrder;
        SRespInfo = src.SRespInfo;
        SThreadID = src.SThreadID;
        LastOfBurst = RespChunkLast;
		LastOfRow = (src.SRespRowLast)? 1 :0;
        TrHandle = src.TrHandle;
    }

    void reset(void)
    {
        SResp = OCP_SRESP_NULL;
        SDataPtr = NULL;
        DataLength = 0;
        SDataInfo = 0;
        STagID = 0;
	STagInOrder = 0;
        SRespInfo = 0;
        SThreadID = 0;
        LastOfBurst = true;
		LastOfRow = 0;
	TrHandle = -1;
    }

    std::string print(void) const 
    { 
        assert(0); 
        return "print() not supported for class OCPTL2ResponseGrp";  
    }

    //operator overloading, so that class can be passed through ports 
    inline bool operator == (const OCPTL2ResponseGrp& rhs) const 
    {
      if ( (SResp == rhs.SResp)
	   && (DataLength == rhs.DataLength)
	   && (SDataInfo == rhs.SDataInfo)
	   && (STagID == rhs.STagID)
	   && (STagInOrder == rhs.STagInOrder)
	   && (SRespInfo == rhs.SRespInfo)
	   && (SThreadID == rhs.SThreadID)
	   && (LastOfBurst == rhs.LastOfBurst)
	   && (LastOfRow == rhs.LastOfRow))
        { 
            // All of the member values are the same. Now compare the data.
            if (DataLength >0) 
            {
                for (unsigned int i=0; i<DataLength; i++)
                {
                    if (SDataPtr[i] != rhs.SDataPtr[i])
                    {
                        // Data not the same
                        return false;
                    }
                }
            }
            // Everything's the same
            return true;
        }
        // Member values not the same
        return false;
    }

    inline friend std::ostream& operator << ( std::ostream& os, OCPTL2ResponseGrp const & v)  
    {
        os << " -SResp: " << v.SResp
           << " -SDataPtr: " << v.SDataPtr
           << " -DataLength: " << v.DataLength
	   << " -SDataInfo: " << v.SDataInfo 
	   << " -STagID: " << v.STagID 
	   << " -STagInOrder: " << v.STagInOrder
	   << " -SRespInfo: " << v.SRespInfo	
           << " -SThreadID: " << v.SThreadID
           << " -LastOfBurst: " << v.LastOfBurst << " "
		   << " -LastOfRow: "<< v.LastOfRow
           << " -TrHandle: " << v.TrHandle << " ";
        return os;
    }
#ifdef OSCI20
    inline friend void sc_trace( sc_core::sc_trace_file *tf, 
            const OCPTL2ResponseGrp& v, 
            const sc_string& NAME)   
#else
    inline friend void sc_trace( sc_core::sc_trace_file *tf, 
            const OCPTL2ResponseGrp& v, 
            const std::string& NAME)   
#endif
    {
        sc_core::sc_trace(tf, v.SResp, NAME+".SResp");
	// sc_trace(tf, v.SDataPtr, NAME+".SDataPtr");
	sc_core::sc_trace(tf, v.DataLength, NAME+".DataLength");
	sc_core::sc_trace(tf, static_cast<long unsigned int>(v.SDataInfo), NAME+".SDataInfo");
	sc_core::sc_trace(tf, static_cast<long unsigned int>(v.SRespInfo), NAME+".SRespInfo");	
	sc_core::sc_trace(tf, v.STagID, NAME+".STagID");
	sc_core::sc_trace(tf, v.STagInOrder, NAME+".STagInOrder");
	sc_core::sc_trace(tf, v.SThreadID, NAME+".SThreadID");
	sc_core::sc_trace(tf, v.LastOfBurst, NAME+".LastOfBurst");
	sc_core::sc_trace(tf, v.LastOfRow, NAME+".LastOfRow");
    }
};

/////////////////////////////////////////////////
// Master Timing Group holds transfer timing
// variables for the TL2 channel
/////////////////////////////////////////////////
class MTimingGrp
{
  public:
    int RqDL;   // Request Data Latency
    int RqSndI; // Request Send Interval
    int DSndI;  // Data Send Interval
    int RpAL;   // Response Accept Latency

    // constructor
    MTimingGrp(void) 
    { 
        reset(); 
    }
 
    // define a copy constructor
    MTimingGrp(const MTimingGrp& src)
      : RqDL(src.RqDL),
        RqSndI(src.RqSndI),
        DSndI(src.DSndI),
        RpAL(src.RpAL)
    { }

    // define a copy assignment
    MTimingGrp& operator= (const MTimingGrp& rhs)
    {
        if (this == &rhs) { return *this; }
        copy(rhs);
        return *this;
    }

    // define a explicit copy
    void copy(const MTimingGrp& src)
    {
        RqDL = src.RqDL;
        RqSndI = src.RqSndI;
        DSndI = src.DSndI;
        RpAL = src.RpAL;
    }

    void reset(void)
    {
        RqDL = 0;
        RqSndI = 1;
        DSndI = 1;
        RpAL = 1;
    }

    std::string print(void) const 
    { 
        assert(0); 
        return "print() not supported for class MTimingGrp"; 
    }

    //operator overloading, so that class can be passed through ports 
    inline bool operator == (const MTimingGrp& rhs) const 
    {
      if ( (RqDL == rhs.RqDL)
	   && (RqSndI == rhs.RqSndI)
	   && (DSndI == rhs.DSndI)
	     && (RpAL == rhs.RpAL) )
        { 
            // Everything's the same
            return true;
        }
        // Member values not the same
        return false;
    }

    inline friend std::ostream& operator << ( std::ostream& os, MTimingGrp const & v)  
    {
        os << " -RqDL: " << v.RqDL
           << " -RqSndI: " << v.RqSndI
           << " -DSndI: " << v.DSndI
	   << " -RpAL: " << v.RpAL << " "; 
        return os;
    }
#ifdef OSCI20
    inline friend void sc_trace( sc_core::sc_trace_file *tf, 
            const MTimingGrp& v, 
            const sc_string& NAME)   
#else
    inline friend void sc_trace( sc_core::sc_trace_file *tf, 
            const MTimingGrp& v, 
            const std::string& NAME)   
#endif
    {
        sc_core::sc_trace(tf, v.RqDL, NAME+".RqDL");
	sc_core::sc_trace(tf, v.RqSndI, NAME+".RqSndI");
	sc_core::sc_trace(tf, v.DSndI, NAME+".DSndI");
	sc_core::sc_trace(tf, v.RpAL, NAME+".RpAL");
    }
};

/////////////////////////////////////////////////
// Slave Timing Group holds transfer timing
// variables for the TL2 channel
/////////////////////////////////////////////////
class STimingGrp
{
  public:
    int RqAL;   // Request Accept Latency
    int DAL;    // Data Accept Latency
    int RpSndI; // Response Send Interval

    // constructor
    STimingGrp(void) 
    { 
        reset(); 
    }
 
    // define a copy constructor
    STimingGrp(const STimingGrp& src)
      : RqAL(src.RqAL),
        DAL(src.DAL),
        RpSndI(src.RpSndI)
    { }

    // define a copy assignment
    STimingGrp& operator= (const STimingGrp& rhs)
    {
        if (this == &rhs) { return *this; }
        copy(rhs);
        return *this;
    }

    // define a explicit copy
    void copy(const STimingGrp& src)
    {
        RqAL = src.RqAL;
        DAL = src.DAL;
        RpSndI = src.RpSndI;
    }

    void reset(void)
    {
        RqAL = 1;
        DAL = 1;
        RpSndI = 1;
    }

    std::string print(void) const 
    { 
        assert(0); 
        return "print() not supported for class STimingGrp"; 
    }

    //operator overloading, so that class can be passed through ports 
    inline bool operator == (const STimingGrp& rhs) const 
    {
      if ( (RqAL == rhs.RqAL)
	   && (DAL == rhs.DAL)
             && (RpSndI == rhs.RpSndI) )
        { 
            // Everything's the same
            return true;
        }
        // Member values not the same
        return false;
    }

    inline friend std::ostream& operator << ( std::ostream& os, STimingGrp const & v)  
    {
        os << " -RqAL: " << v.RqAL
           << " -DAL: " << v.DAL
           << " -RpSndI: " << v.RpSndI << " ";
        return os;
    }
#ifdef OSCI20
    inline friend void sc_trace( sc_core::sc_trace_file *tf, 
            const STimingGrp& v, 
            const sc_string& NAME)   
#else
    inline friend void sc_trace( sc_core::sc_trace_file *tf, 
            const STimingGrp& v, 
            const std::string& NAME)   
#endif
    {
        sc_core::sc_trace(tf, v.RqAL, NAME+".RqAL");
	sc_core::sc_trace(tf, v.DAL, NAME+".DAL");
	sc_core::sc_trace(tf, v.RpSndI, NAME+".RpSndI");
    }
};

/////////////////////////////////////////////////
// SideBandGrp holds the OCP sideband information
/////////////////////////////////////////////////

class OCPSidebandGrp {
  public:

    // reset signals
    bool MReset_n;
    bool SReset_n;

    // system/core signals
    unsigned int  Control;
    bool ControlWr;
    bool ControlBusy;
    unsigned int  Status;
    bool StatusRd;
    bool StatusBusy;

    // sideband signals
    sc_dt::uint64  MFlag;
    bool MError;
    sc_dt::uint64  SFlag;
    bool SError;
    bool SInterrupt;


    void reset()
    {
      SInterrupt = false;
      SError = false;
      MError = false;
      MFlag = 0;
      SFlag = 0;
      Control = 0;;
      ControlWr = false;
      ControlBusy = false;
      Status = 0;
      StatusRd = false;
      StatusBusy = false;
    }

    OCPSidebandGrp()
    {
      MReset_n = true;
      SReset_n = true;
      reset();
    }

	//operator overloading, so that class can be passed through ports 
	inline bool operator == (const OCPSidebandGrp& rhs) const {
	  return ( (MReset_n == rhs.MReset_n)
		   && (SReset_n == rhs.SReset_n)
		   && (Control == rhs.Control)
		   && (ControlWr == rhs.ControlWr)
		   && (ControlBusy == rhs.ControlBusy)
		   && (Status == rhs.Status)
		   && (StatusRd == rhs.StatusRd)
		   && (StatusBusy == rhs.StatusBusy)
		   && (MFlag == rhs.MFlag)
		   && (SFlag == rhs.SFlag)
		   && (MError == rhs.MError)
		   && (SError == rhs.SError)
		   && (SInterrupt == rhs.SInterrupt));
	}

	//define a copy operator
	inline OCPSidebandGrp& operator = (const OCPSidebandGrp& rhs)   {
        if (this == &rhs) { return *this; }
		MReset_n = rhs.MReset_n;
		SReset_n = rhs.SReset_n;
		Control = rhs.Control;
		ControlWr = rhs.ControlWr;
		ControlBusy = rhs.ControlBusy;
		Status = rhs.Status;
		StatusRd = rhs.StatusRd;
		StatusBusy = rhs.StatusBusy;
		MFlag = rhs.MFlag;
		SFlag = rhs.SFlag;
		MError = rhs.MError;
		SError = rhs.SError;
		SInterrupt = rhs.SInterrupt;
        return *this;
	}

	inline friend std::ostream& operator << ( std::ostream& os, OCPSidebandGrp const & v)  {
		os << " -MReset_n: "<<v.MReset_n << " -SReset_n: "<<v.SReset_n
			<< " -Control: "<<v.Control<< " -ControlWr: "<<v.ControlWr 
			<<" -ControlBusy: "<<v.ControlBusy<<" -Status: "<<v.Status
			<<" -StatusRd: "<<v.StatusRd<<" -StatusBusy: "<<v.StatusBusy
			<<" -MFlag: "<<v.MFlag<<" -SFlag: "<<v.SFlag
			<<" -MError: "<<v.MError<<" -SError: "<<v.SError
			<<" -SInterrupt: "<<v.SInterrupt<< " ";
		return os;
	}
#ifdef OSCI20
	inline friend void sc_trace( sc_core::sc_trace_file *tf, const OCPSidebandGrp& v, const sc_string& NAME)   {
#else
	inline friend void sc_trace( sc_core::sc_trace_file *tf, const OCPSidebandGrp& v, const std::string& NAME)   {
#endif
		sc_core::sc_trace(tf, v.MReset_n, NAME+".MReset_n");
		sc_core::sc_trace(tf, v.SReset_n, NAME+".SReset_n");
		sc_core::sc_trace(tf, v.Control, NAME+".Control");
		sc_core::sc_trace(tf, v.ControlWr, NAME+".ControlWr");
		sc_core::sc_trace(tf, v.ControlBusy, NAME+".ControlBusy");
		sc_core::sc_trace(tf, v.Status, NAME+".Status");
		sc_core::sc_trace(tf, v.StatusRd, NAME+".StatusRd");
		sc_core::sc_trace(tf, v.StatusBusy, NAME+".StatusBusy");
		sc_core::sc_trace(tf, static_cast<long unsigned int>(v.MFlag), 
                        NAME+".MFlag");
		sc_core::sc_trace(tf, static_cast<long unsigned int>(v.SFlag), 
                        NAME+".SFlag");
		sc_core::sc_trace(tf, v.MError, NAME+".MError");
		sc_core::sc_trace(tf, v.SError, NAME+".SError");
		sc_core::sc_trace(tf, v.SInterrupt, NAME+".SInterrupt");	
	}
};

/////////////////////////////////////////////////
// ThreadBusyGrp holds the OCP Threadbusy information
// Original TL2 Channel Only
/////////////////////////////////////////////////

class OCPThreadBusyGrp {
  public:

    // Threadbusy signals
    unsigned int MThreadBusy;
    unsigned int MDataThreadBusy;
    unsigned int SThreadBusy;

    void reset()
    {
      MThreadBusy = false;
      MDataThreadBusy = false;
      SThreadBusy = false;
    }

    OCPThreadBusyGrp()
    {
      reset();
    }

	//operator overloading, so that class can be passed through ports 
	inline bool operator == (const OCPThreadBusyGrp& rhs) const {
	  return ( (MThreadBusy == rhs.MThreadBusy)
		   && (MDataThreadBusy == rhs.MDataThreadBusy)
				&& (SThreadBusy == rhs.SThreadBusy));
	}

	//define a copy operator
	inline OCPThreadBusyGrp& operator = (const OCPThreadBusyGrp& rhs)   {
        if (this == &rhs) { return *this; }
		MThreadBusy = rhs.MThreadBusy;
		MDataThreadBusy = rhs.MDataThreadBusy;
		SThreadBusy = rhs.SThreadBusy;
        return *this;
	}

	inline friend std::ostream& operator << ( std::ostream& os, OCPThreadBusyGrp const & v)  {
		os << " -MThreadBusy: "<<v.MThreadBusy << " -MDataThreadBusy: "<<v.MDataThreadBusy
			<< " -SThreadBusy: "<<v.SThreadBusy<< " ";
		return os;
	}
#ifdef OSCI20
	inline friend void sc_trace( sc_core::sc_trace_file *tf, const OCPThreadBusyGrp& v, const sc_string& NAME)   {
#else
	inline friend void sc_trace( sc_core::sc_trace_file *tf, const OCPThreadBusyGrp& v, const std::string& NAME)   {
#endif
		sc_core::sc_trace(tf, v.MThreadBusy, NAME+".MThreadBusy");
		sc_core::sc_trace(tf, v.MDataThreadBusy, NAME+".MDataThreadBusy");
		sc_core::sc_trace(tf, v.SThreadBusy, NAME+".SThreadBusy");	
	}
};


#endif
