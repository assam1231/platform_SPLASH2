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
//  Description :  This file contains provides a centralized type definition proxy.
//                 Using this proxy every module will use the same data types,
//                 when using the same BUSWIDTH template argument.
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef OCPIP_VERSION
  #error ocp_data_class.h may not be included directly. Use #inculde "ocpip.h" or #include "ocpip_X_X_X.h" (where desired ocp version is X.X.X)
#endif

namespace OCPIP_VERSION{

template <unsigned int WIDTH>
struct unsigned_type{
  typedef sc_dt::sc_biguint<WIDTH> type;
  static const unsigned int bit_size=WIDTH;
};

#define UP_TO_8(number) \
template <> \
struct unsigned_type< number >{ \
  typedef uint8_t type; \
  static const unsigned int bit_size=8; \
}
UP_TO_8(1);UP_TO_8(2);UP_TO_8(3);UP_TO_8(4);UP_TO_8(5);UP_TO_8(6);UP_TO_8(7);UP_TO_8(8);
#undef UP_TO_8

#define UP_TO_16(number) \
template <> \
struct unsigned_type< number >{ \
  typedef uint16_t type; \
  static const unsigned int bit_size=16; \
}
UP_TO_16(9);UP_TO_16(10);UP_TO_16(11);UP_TO_16(12);UP_TO_16(13);UP_TO_16(14);UP_TO_16(15);UP_TO_16(16);
#undef UP_TO_16

#define UP_TO_32(number)\
template <>\
struct unsigned_type< number >{ \
  typedef uint32_t type;\
  static const unsigned int bit_size=32; \
}
UP_TO_32(17);UP_TO_32(18);UP_TO_32(19);UP_TO_32(20);UP_TO_32(21);UP_TO_32(22);UP_TO_32(23);UP_TO_32(24);
UP_TO_32(25);UP_TO_32(26);UP_TO_32(27);UP_TO_32(28);UP_TO_32(29);UP_TO_32(30);UP_TO_32(31);UP_TO_32(32);
#undef UP_TO_32

#define UP_TO_64(number)\
template <>\
struct unsigned_type< number >{ \
  typedef uint64_t type; \
  static const unsigned int bit_size=64; \
}
UP_TO_64(33);UP_TO_64(34);UP_TO_64(35);UP_TO_64(36);UP_TO_64(37);UP_TO_64(38);UP_TO_64(39);UP_TO_64(40);
UP_TO_64(41);UP_TO_64(42);UP_TO_64(43);UP_TO_64(44);UP_TO_64(45);UP_TO_64(46);UP_TO_64(47);UP_TO_64(48);
UP_TO_64(49);UP_TO_64(50);UP_TO_64(51);UP_TO_64(52);UP_TO_64(53);UP_TO_64(54);UP_TO_64(55);UP_TO_64(56);
UP_TO_64(57);UP_TO_64(58);UP_TO_64(59);UP_TO_64(60);UP_TO_64(61);UP_TO_64(62);UP_TO_64(63);UP_TO_64(64);
#undef UP_TO_64

template <unsigned int WIDTH>
struct signed_type{
  typedef sc_dt::sc_bigint<WIDTH> type;
  static const unsigned int bit_size=WIDTH;
};

#define UP_TO_8(number) \
template <> \
struct signed_type< number >{ \
  typedef int8_t type; \
  static const unsigned int bit_size=8; \
}
UP_TO_8(1);UP_TO_8(2);UP_TO_8(3);UP_TO_8(4);UP_TO_8(5);UP_TO_8(6);UP_TO_8(7);UP_TO_8(8);
#undef UP_TO_8

#define UP_TO_16(number) \
template <> \
struct signed_type< number >{ \
  typedef int16_t type; \
  static const unsigned int bit_size=16; \
}
UP_TO_16(9);UP_TO_16(10);UP_TO_16(11);UP_TO_16(12);UP_TO_16(13);UP_TO_16(14);UP_TO_16(15);UP_TO_16(16);
#undef UP_TO_16

#define UP_TO_32(number)\
template <>\
struct signed_type< number >{ \
  typedef int32_t type;\
  static const unsigned int bit_size=32; \
}
UP_TO_32(17);UP_TO_32(18);UP_TO_32(19);UP_TO_32(20);UP_TO_32(21);UP_TO_32(22);UP_TO_32(23);UP_TO_32(24);
UP_TO_32(25);UP_TO_32(26);UP_TO_32(27);UP_TO_32(28);UP_TO_32(29);UP_TO_32(30);UP_TO_32(31);UP_TO_32(32);
#undef UP_TO_32

#define UP_TO_64(number)\
template <>\
struct signed_type< number >{ \
  typedef int64_t type; \
  static const unsigned int bit_size=64; \
}

UP_TO_64(33);UP_TO_64(34);UP_TO_64(35);UP_TO_64(36);UP_TO_64(37);UP_TO_64(38);UP_TO_64(39);UP_TO_64(40);
UP_TO_64(41);UP_TO_64(42);UP_TO_64(43);UP_TO_64(44);UP_TO_64(45);UP_TO_64(46);UP_TO_64(47);UP_TO_64(48);
UP_TO_64(49);UP_TO_64(50);UP_TO_64(51);UP_TO_64(52);UP_TO_64(53);UP_TO_64(54);UP_TO_64(55);UP_TO_64(56);
UP_TO_64(57);UP_TO_64(58);UP_TO_64(59);UP_TO_64(60);UP_TO_64(61);UP_TO_64(62);UP_TO_64(63);UP_TO_64(64);
#undef UP_TO_64


template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH>
struct ocp_data_class_unsigned{
  typedef unsigned_type<BUSWIDTH>        SizeCalc;
  typedef typename unsigned_type<BUSWIDTH>::type  DataType;
  typedef typename unsigned_type<ADDRWIDTH>::type AddrType;
  std::string to_string(){
    std::stringstream s; 
    s<<typeid(DataType).name()<<std::endl
     <<typeid(AddrType).name();
    return s.str();
  }
};

template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH>
struct ocp_data_class_signed{
  typedef signed_type<BUSWIDTH>        SizeCalc;
  typedef typename   signed_type<BUSWIDTH>::type  DataType;
  typedef typename unsigned_type<ADDRWIDTH>::type AddrType;
  std::string to_string(){
    std::stringstream s; 
    s<<typeid(DataType).name()<<std::endl
     <<typeid(AddrType).name();
    return s.str();
  }
};

}
