#ifndef TRAFFIC_PARSER_H_
#define TRAFFIC_PARSER_H_

enum MasterActionType
{
	TRANSACTION,
	WAIT,
	WAIT_UNTIL
};

enum MasterDataMode
{
	M_RANDOM,
	M_PREDEFINED, //id trans_count address data_count
	M_USER_DEFINED
};

enum SlaveDataMode
{
	S_RANDOM,
	S_PREDEFINED //id trans_count address data_count
//	,USER_DEFINED
};

#endif
