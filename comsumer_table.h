#pragma once
#include"systemc.h"
#include"Instruction.h"
#include"Message.h"

enum cte_state {
	cte_wait_for_req,
	cte_wait_for_ack,
	cte_wait_for_send_data,
	cte_sending_data,
	cte_finish
};

//cosumer table element
struct cte {
	sc_uint<2> consumer_type;
	sc_uint<32> consumer_addr;
	sc_uint<32> out_data_package_id;
	cte_state state;
	int req_wait_cycle;


	cte(sc_uint<32> addr) {
		consumer_addr = addr;
		consumer_type = actor_type;
		state = cte_wait_for_req;
		out_data_package_id = 0;
		req_wait_cycle = 0;
	}
};


inline void ct_out_config(vector<cte>& ct, Instruction& ins) {
	if (sc_uint<8>(ins.range(ins_type_bits)) != Out_Continue) {
		ct.clear();
	}

	if (sc_uint<2>(ins.range(ins_op1_type_bits)) == actor_type) {
		ct.push_back(cte(sc_uint<32>(ins.range(ins_op1_bits))));
	}
	if (sc_uint<2>(ins.range(ins_op2_type_bits)) == actor_type) {
		ct.push_back(cte(sc_uint<32>(ins.range(ins_op2_bits))));
	}
	if (sc_uint<2>(ins.range(ins_op3_type_bits)) == actor_type) {
		ct.push_back(cte(sc_uint<32>(ins.range(ins_op3_bits))));
	}
	if (sc_uint<2>(ins.range(ins_op4_type_bits)) == actor_type) {
		ct.push_back(cte(sc_uint<32>(ins.range(ins_op4_bits))));
	}
}