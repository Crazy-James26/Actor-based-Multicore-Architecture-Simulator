#pragma once
#include"systemc.h"
#include"Instruction.h"
#include"Message.h"
#include"producer_table.h"
#include"comsumer_table.h"

enum out_state {
	out_idle,
	out_wait_for_produce_data,
	out_producing_data,
	out_wait_for_send_data,
	out_sending_data,
	out_finish
};

inline void compute_mem_alloc_size(Instruction& ins, vector<pte>& producer_table, sc_uint<32> mem_alloc_size[]);

//reservation_table_entry
struct rte {
	Instruction op_ins;
	sc_uint<8> producer_in_mode;
	vector<pte> producer_table;
	sc_uint<2> out_mem_type;
	sc_uint<32> out_mem_addr;
	sc_uint<32> out_alloc_mem_addr;
	sc_uint<32> out_mem_size;
	sc_uint<32> out_data_package_size;
	out_state out_state;
	sc_uint<8> consumer_out_mode;
	vector<cte> consumer_table;
	bool mem_allocating;

	rte() {
		op_ins = Instruction(0);
		producer_in_mode = sc_uint<8>(0);
		out_mem_type = none_type;
		out_mem_addr = sc_uint<32>(0);
		out_mem_size = sc_uint<32>(0);
		out_state = out_idle;
		out_alloc_mem_addr = sc_uint<32>(0);
		consumer_out_mode = sc_uint<8>(0);
		mem_allocating = false;
	}

	void rte_out_config(Instruction& ins) {
		if (sc_uint<8>(ins.range(ins_type_bits)) != Out_Continue) {
			consumer_out_mode = sc_uint<8>(ins.range(ins_type_bits));
			if (sc_uint<2>(ins.range(ins_op1_type_bits)) == mem_type) {
				out_mem_type = mem_type;
				out_mem_addr = sc_uint<32>(ins.range(ins_op1_bits));
				out_state = out_idle;
			}
			else {
				out_mem_type = mem_type_release;
				out_mem_addr = sc_uint<32>(0);
				out_state = out_idle;
			}
			out_alloc_mem_addr = sc_uint<32>(0);
		}
		ct_out_config(consumer_table, ins);
	}

	void rte_in_cofig(Instruction& ins) {
		if (sc_uint<8>(ins.range(ins_type_bits)) != In_Continue) {
			producer_in_mode = sc_uint<8>(ins.range(ins_type_bits));
		}
		pt_in_config(producer_table, ins);
	}

	void rte_op_config(Instruction& ins) {
		op_ins = ins;
		sc_uint<32> mem_alloc_size[Max_op_num + 1] = { 0 };
		compute_mem_alloc_size(ins, producer_table, mem_alloc_size);
		pt_op_config(producer_table, mem_alloc_size);
		out_mem_size = mem_alloc_size[Max_op_num];
	}
};

struct mem_rte {
	bool valid = false;
	sc_uint<32> mem_addr = 0;
};


inline void compute_mem_alloc_size(Instruction& ins, vector<pte>& producer_table, sc_uint<32> mem_alloc_size[]) {
	sc_uint<8> ALU_type = sc_uint<8>(ins.range(ins_type_bits));
	sc_biguint<ALU_para_Width> para = sc_biguint<ALU_para_Width>(ins.range(ins_para_bits));
	sc_uint<16> para1 = sc_uint<16>(para.range(para1_bits));
	sc_uint<16> para2 = sc_uint<16>(para.range(para2_bits));
	sc_uint<16> para3 = sc_uint<16>(para.range(para3_bits));
	sc_uint<16> para4 = sc_uint<16>(para.range(para4_bits));
	sc_uint<16> para5 = sc_uint<16>(para.range(para5_bits));
	sc_uint<16> para6 = sc_uint<16>(para.range(para6_bits));
	sc_uint<16> para7 = sc_uint<16>(para.range(para7_bits));
	sc_uint<16> para8 = sc_uint<16>(para.range(para8_bits));
	sc_uint<8> para9 = sc_uint<8>(para.range(para9_bits));

	for (int i = 0; i < Max_op_num + 1; i++) {
		mem_alloc_size[i] = 0;
	}

	if ((ALU_type == ADD)) {
		for (int i = 0; i < producer_table.size(); i++) {
			mem_alloc_size[i] = para1 * para2 * para3 * para4;
		}
		mem_alloc_size[Max_op_num] = para1 * para2 * para3 * para4;
	}

	else if (ALU_type == LOSS) {
		mem_alloc_size[0] = para1 * para2 * para3 * para4;
		mem_alloc_size[1] = para1 * para2 * para3 * para4;
		mem_alloc_size[Max_op_num] = para1 * para2 * para3 * para4;
	}

	else if (ALU_type == MUL) {
		mem_alloc_size[0] = para1 * para2;
		mem_alloc_size[1] = para2 * para3;
		mem_alloc_size[Max_op_num] = para1 * para3;
	}

	else if (ALU_type == CONV) {
		unsigned int HW_out = (para3 + 2 * para7 - para4) / para6 + 1;
		if (para9 == CONV_fp) {
			mem_alloc_size[0] = para1 * para2 * para3 * para3;
			mem_alloc_size[1] = para5 * para2 * para4 * para4;
			mem_alloc_size[Max_op_num] = para1 * para5 * HW_out * HW_out;
		}
		else if (para9 == CONV_bp) {
			mem_alloc_size[0] = para1 * para5 * HW_out * HW_out; 
			mem_alloc_size[1] = para5 * para2 * para4 * para4;
			mem_alloc_size[Max_op_num] = para1 * para2 * para3 * para3;
		}
		else if (para9 == CONV_divw) {
			mem_alloc_size[0] = para1 * para2 * para3 * para3;
			mem_alloc_size[1] = para1 * para5 * HW_out * HW_out;
			mem_alloc_size[Max_op_num] = para5 * para2 * para4 * para4;
		}
	}

	else if (ALU_type == POOL) {
		unsigned int HW_out = (para3 + 2 * para7 - para4) / para6 + 1;
		if (para9 == POOL_max_fp || para9 == POOL_ave_fp) {
			mem_alloc_size[0] = para1 * para2 * para3 * para3;
			mem_alloc_size[Max_op_num] = para1 * para2 * HW_out * HW_out;
		}
		else if (para9 == POOL_max_bp) {
			mem_alloc_size[0] = para1 * para2 * HW_out * HW_out;
			mem_alloc_size[1] = para1 * para2 * para3 * para3;
			mem_alloc_size[Max_op_num] = para1 * para2 * para3 * para3;
		}
		else if (para9 == POOL_ave_bp) {
			mem_alloc_size[0] = para1 * para2 * HW_out * HW_out;
			mem_alloc_size[Max_op_num] = para1 * para2 * para3 * para3;
		}
	}

	else if (ALU_type == ACT) {
		mem_alloc_size[0] = para1 * para2 * para3 * para4;
		if(para9 == RELU_bp)
			mem_alloc_size[1] = para1 * para2 * para3 * para4;
		mem_alloc_size[Max_op_num] = para1 * para2 * para3 * para4;
	}

	else if (ALU_type == FC) {
		if (para9 == FC_fp) {
			mem_alloc_size[0] = para1 * para2;
			mem_alloc_size[1] = para2 * para3;
			mem_alloc_size[Max_op_num] = para1 * para3;
		}
		else if (para9 == FC_bp) {
			mem_alloc_size[0] = para1 * para3;
			mem_alloc_size[1] = para2 * para3;
			mem_alloc_size[Max_op_num] = para1 * para2;
		}
		else if (para9 == FC_divw) {
			mem_alloc_size[0] = para1 * para2;
			mem_alloc_size[1] = para1 * para3;
			mem_alloc_size[Max_op_num] = para2 * para3;
		}
	}

	else if ((ALU_type == STORE) || (ALU_type == LOAD)) {
		mem_alloc_size[Max_op_num] = para1 * para2 * para3 * para4;
	}
}


