#pragma once
#include"systemc.h"
#include"Instruction.h"
#include"Message.h"

enum pte_state {
	pte_idle,
	pte_received_req,
	pte_sent_ack,
	pte_receiving_data,
	pte_wait_for_consume_data,
	pte_consuming_data,
	pte_consumed_data,
	pte_finish
};



//reservation_table_entry
struct pte {
	sc_uint<2> in_op_type;
	sc_uint<32> actor_addr;
	sc_uint<32> in_mem_addr;
	sc_uint<32> in_alloc_mem_addr;
	sc_uint<32> in_mem_size;
	sc_uint<32> in_data_package_size;
	pte_state in_state;

	pte(sc_uint<2> type, sc_uint<32> addr) {
		if (type == actor_type) {
			in_op_type = type;
			actor_addr = addr;
			in_mem_addr = sc_uint<32>(0);
			in_state = pte_idle;
		}
		else if (type == mem_type || type == mem_type_release) {
			in_op_type = type;
			actor_addr = sc_uint<32>(0);
			in_mem_addr = addr;
			in_state = pte_wait_for_consume_data;
		}
		in_data_package_size = 0;
		in_mem_size = 0;
		in_alloc_mem_addr = 0;
	}
};

inline void pt_in_config(vector<pte>& pt, Instruction& ins) {
	if (sc_uint<8>(ins.range(ins_type_bits)) != In_Continue) {
		pt.clear();
	}

	if (pt.size() < Max_op_num && sc_uint<2>(ins.range(ins_op1_type_bits)) != none_type) {
		pt.push_back(pte(sc_uint<2>(ins.range(ins_op1_type_bits)), sc_uint<32>(ins.range(ins_op1_bits))));
	}
	if (pt.size() < Max_op_num && sc_uint<2>(ins.range(ins_op2_type_bits)) != none_type) {
		pt.push_back(pte(sc_uint<2>(ins.range(ins_op2_type_bits)), sc_uint<32>(ins.range(ins_op2_bits))));
	}
	if (pt.size() < Max_op_num && sc_uint<2>(ins.range(ins_op3_type_bits)) != none_type) {
		pt.push_back(pte(sc_uint<2>(ins.range(ins_op3_type_bits)), sc_uint<32>(ins.range(ins_op3_bits))));
	}
	if (pt.size() < Max_op_num && sc_uint<2>(ins.range(ins_op4_type_bits)) != none_type) {
		pt.push_back(pte(sc_uint<2>(ins.range(ins_op4_type_bits)), sc_uint<32>(ins.range(ins_op4_bits))));
	}

}

inline void pt_op_config(vector<pte>& pt, sc_uint<32>* mem_alloc_size) {
	for (int i = 0; i < pt.size(); i++) {
		if (pt[i].in_op_type != none_type) {
			pt[i].in_mem_size = mem_alloc_size[i];
		}
	}
}


