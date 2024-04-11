/*#include"SU.h"

int rte_find_source(Instruction ins, sc_uint<32> source) {
	if(sc_uint<32>(ins.range(ins_op1_bits)==source))
		return 0;
	else if(sc_uint<32>(ins.range(ins_op2_bits) == source))
		return 1;
	else {
		cout << "Error: the source of message is wrong!";
		return -1;
	}
}


void SU_icache::SU_data_receive() {

	alloc_req.write(false);
	alloc_size[0].write(0);
	alloc_size[1].write(0);
	alloc_size[2].write(0);

	op_req_pt[0] = 0;
	op_req_pt[1] = 0;
	alloc_pt = 0;
	
	while (true) {
		alloc_req.write(false);
		alloc_size[0].write(0);
		alloc_size[1].write(0);
		alloc_size[2].write(0);

		if (current_ins_valid.read() == true) {
			Message mes = in_message.read();
			sc_uint<8> mes_head = sc_uint<8>(mes.range(mes_head_bits));
			if (mes_head == message_head) {

				sc_uint<8> mes_type = sc_uint<8>(mes.range(mes_type_bits));
				sc_uint<8> mes_seq = sc_uint<8>(mes.range(mes_seq_bits));
				sc_uint<32> mes_source = sc_uint<32>(mes.range(mes_source_bits));
				
				if (mes_type == message_req) {
					//when receive req message, SU enable memory manager to allocate memory
					Instruction ins = current_ins.read();
					int op = rte_find_source(ins, mes_source);
					if (((op == 0) && (op_req_pt[0] >= op_req_pt[1])) ||
						((op == 1) && (op_req_pt[1] >= op_req_pt[0]))) {
						if (mes_seq != rt.back().seq) {
							rte new_rte(current_ins, mes_seq);
							new_rte.in_state[op] = received_req;

							alloc_req.write(true);
							if (new_rte.ins_op_type[0] == actor_type)
								alloc_size[0].write(new_rte.in_mem_size[0]);
							else
								alloc_size[0].write(0);
							if (new_rte.ins_op_type[1] == actor_type)
								alloc_size[1].write(new_rte.in_mem_size[1]);
							else
								alloc_size[0].write(0);
							alloc_size[2].write(new_rte.out_mem_size);

							while (alloc_ack.read() == sc_uint<2>(0)) {
								wait();
								alloc_req.write(false);
								alloc_size[0].write(0);
								alloc_size[1].write(0);
								alloc_size[2].write(0);
							}
							//allocate memory successfully
							if (alloc_ack.read() == sc_uint<2>(1)) {
								if (new_rte.ins_op_type[0] == actor_type)
									new_rte.in_mem_addr[0] = alloc_addr[0].read();
								else
									new_rte.in_mem_addr[0] = ins.range(ins_op1_bits);
								if (new_rte.ins_op_type[1] == actor_type)
									new_rte.in_mem_addr[1] = alloc_addr[1].read();
								else
									new_rte.in_mem_addr[0] = ins.range(ins_op2_bits);
								new_rte.out_mem_addr = alloc_addr[2].read();
								cout << "rte" << op_req_pt[op] << ": " << new_rte.in_mem_addr[0] << " " << new_rte.in_mem_addr[1] << " " << new_rte.out_mem_addr << endl;
								cout << "rte" << op_req_pt[op] << ": " "op" << op << "'s state is  received_req" << endl;
								rt.push_back(new_rte);
								op_req_pt[op]++;
							}
						}
					}

					else {
						if (rt[op_req_pt[op]].seq == mes_seq) {
							if (rt[op_req_pt[op]].in_state[op] == idle) {
								rt[op_req_pt[op]].in_state[op] = received_req;
								cout << "rte" << op_req_pt[op] << ": " "op" << op << "'s state is  received_req" << endl;
								op_req_pt[op]++;

							}
							else {
								cout << "Error: rt[" << op_req_pt[op] << "].in_state[" << op << "] is not idle" << endl;
							}
						}
						else {
							cout << "Error: dataflow is out-of-order!" << endl;
						}
					}

				}
				
			}
		}
		wait();
	}
}*/

