#pragma once
#include"systemc.h"
#include"basic.h"
#include"Instruction.h"

inline sc_uint<32> gather_engine(sc_uint<8> in_mode, Instruction ins, int gather_id, int gather_num, sc_uint<32> data_package_id) {
	sc_uint<32> addr = 0;
	sc_biguint<ALU_para_Width> para = sc_biguint<ALU_para_Width>(ins.range(ins_para_bits));
	sc_uint<16> para1 = sc_uint<16>(para.range(para1_bits));
	sc_uint<16> para2 = sc_uint<16>(para.range(para2_bits));
	sc_uint<16> para3 = sc_uint<16>(para.range(para3_bits));
	sc_uint<16> para4 = sc_uint<16>(para.range(para4_bits));
	if (in_mode == In_Gather1) {
		addr = gather_id * (para1 * para2 * para3 * para4 / gather_num) + data_package_id * Message_data_num;
	}
	else if (in_mode == In_Gather2) {
		addr = ((data_package_id * Message_data_num) / (para2 * para3 * para4 / gather_num)) * (para2 * para3 * para4) +
			gather_id * (para2 * para3 * para4 / gather_num) + (data_package_id * Message_data_num) % (para2 * para3 * para4 / gather_num);
	}
	return addr;
}

inline sc_uint<32> scatter_engine(sc_uint<8> out_mode, Instruction ins, int scatter_id, int scatter_num, sc_uint<32> data_package_id) {
	sc_uint<32> addr = 0;
	sc_biguint<ALU_para_Width> para = sc_biguint<ALU_para_Width>(ins.range(ins_para_bits));
	sc_uint<16> para1 = sc_uint<16>(para.range(para1_bits));
	sc_uint<16> para2 = sc_uint<16>(para.range(para2_bits));
	sc_uint<16> para3 = sc_uint<16>(para.range(para3_bits));
	sc_uint<16> para4 = sc_uint<16>(para.range(para4_bits));
	if (out_mode == Out_Split1) {
		addr = scatter_id * (para1 * para2 * para3 * para4 / scatter_num) + data_package_id * Message_data_num;
	}
	else if (out_mode == Out_Split2) {
		addr = ((data_package_id * Message_data_num) / (para2 * para3 * para4 / scatter_num)) * (para2 * para3 * para4) +
			scatter_id * (para2 * para3 * para4 / scatter_num) + (data_package_id * Message_data_num) % (para2 * para3 * para4 / scatter_num);
	}
	return addr;
}