#pragma once
#include"systemc.h"

#define Message_Width 1056
#define Data_package_Width 1024
#define Ins_Width 152
#define ALU_para_Width 136

#define interface_size 16
#define Max_op_num 4

#define Max_rte_num 5
#define Message_data_num (Data_package_Width/32)
#define SU_write_num 4
#define SU_read_num 4

#define mem_size 23
#define block_size 16
#define block_num (1<<(mem_size-block_size))

#define ALU_read_write_num 32
#define ALU_calulate_parallelism 1024

#define req_ack_max_cycle 5

#define max_actor_num 20

#define test_actor_num 15
#define file_name string("file_dis_4_dmm/MemWidth_4")
#define test_time 15000000


inline sc_uint<32> float_cov_sc_uint32(float& value)
{
	uint32_t uint_data;
	*(float*)(&uint_data) = value;
	return sc_uint<32>(uint_data);
}

inline float sc_uint32_cov_float(sc_uint<32>& value) {
	float float_data;
	uint32_t uint_data = value;
	*(uint32_t*)(&float_data) = uint_data;
	return float_data;
}