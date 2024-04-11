#pragma once
#include<iostream>
#include<vector>

#include"systemc.h"
#include"Instruction.h"
#include"Message.h"
#include"icache.h"
#include"reservation_table.h"
#include"comsumer_table.h"

using namespace std;


sc_uint<8> consumer_SBP_mode;
vector<cte> consumer_table;//record the cosumers
vector<rte> rt;//reservation station

unsigned int op_req_pt[2] = { 0 };
unsigned int op_rec_data_pt[2] = { 0 };
unsigned int rt_ALU_pt = 0;


enum SU_state {
	SU_reset,
	SU_ins_init,
	SU_ins_init_done,
	SU_ins_config,
	SU_ins_config_done,
	SU_start,
	SU_pause
};

SC_MODULE(SU_config) {
	sc_in_clk clk;
	sc_in<bool> rst;

	sc_signal<SU_state> State;

	sc_in<Message> consume_message_in;

	sc_in<Instruction> icache_ouput_ins;
	sc_in<icache_state> icache_State;
	sc_out<sc_uint<8>> icache_control;
	sc_out<bool> icache_control_valid;
	sc_out<sc_uint<32>> icache_op;
	sc_out<Instruction> icache_input_ins;

	sc_out<Instruction> current_ins;
	sc_out<bool> current_ins_valid;

	void SU_config_main();

	SC_CTOR(SU_config) {
		SC_CTHREAD(SU_config_main, clk.pos());
		reset_signal_is(rst, true);
	}
};



void SU_config::SU_config_main() {

	State = SU_reset;
	consumer_SBP_mode = sc_uint<8>(0);


	icache_control.write(sc_uint<8>(0));
	icache_control_valid.write(false);
	icache_op.write(sc_uint <32>(0));
	icache_input_ins.write(Instruction(0));

	current_ins.write(Instruction(0));
	current_ins_valid.write(false);

	while (true) {
		Message mes = consume_message_in.read();
		sc_uint<8> mes_head = sc_uint<8>(mes.range(mes_head_bits));

		if (mes_head == message_head) {
			sc_uint<8> mes_type = sc_uint<8>(mes.range(mes_type_bits));
			sc_uint<32> mes_source = sc_uint<32>(mes.range(mes_source_bits));
			sc_uint<32> mes_op = sc_uint<32>(mes.range(mes_op_bits));
			sc_uint<8> mes_seq = sc_uint<8>(mes.range(mes_seq_bits));
			cout << State << " " << hex << mes_type << endl;

			if (mes_type == message_ins_init) {
				if ((State == SU_reset) || (State == SU_pause)) {
					State = SU_ins_init;
					icache_control_valid.write(true);
					icache_control.write(mes_type);
					icache_op.write(mes_op);
					wait();
					icache_control_valid.write(false);
					for (unsigned int i = 1; i <= mes_op; i++) {
						Instruction input_ins = Instruction(consume_message_in.read().range(mes_ins_package_bits));
						icache_input_ins.write(input_ins);
						//cout << hex << input_ins << endl;
						wait();
					}
					while (icache_State.read() != ins_init_done)
						wait();
					State = SU_ins_init_done;

					//consume_message_out.write(creat_Message("ack",sc_uint<32>(0x80000001),mes_source,mes_seq));
				}
				else {
					cout << "The icache initialization is invalid, because the SU has not been reset or paused!" << endl;
				}
			}

			else if ((mes_type == message_config) || (mes_type == message_jumpconf)) {
				if ((State == SU_ins_init_done) || (State == SU_ins_config_done) || (State == SU_pause)) {
					State = SU_ins_config;
					icache_control_valid.write(true);
					icache_control.write(mes_type);
					icache_op.write(mes_op);
					wait();
					icache_control_valid.write(false);
					while (icache_State.read() != ins_config_done)
						wait();
					Instruction ins = icache_ouput_ins.read();
					current_ins.write(ins);
					cout << "current_ins: " << ins << endl;

					if (ins.range(ins_head_bits) == Out_Config) {
						if (ins.range(ins_type_bits) != Continue) {
							consumer_SBP_mode = ins.range(ins_type_bits);
							consumer_table.clear();
						}
						if (sc_uint<32>(ins.range(ins_op1_bits)) != sc_uint<32>(0)) {
							consumer_table.push_back(cte(sc_uint<32>(ins.range(ins_op1_bits))));
						}
						if (sc_uint<32>(ins.range(ins_op2_bits)) != sc_uint<32>(0)) {
							consumer_table.push_back(cte(sc_uint<32>(ins.range(ins_op2_bits))));
						}
						if (sc_uint<32>(ins.range(ins_op3_bits)) != sc_uint<32>(0)) {
							consumer_table.push_back(cte(sc_uint<32>(ins.range(ins_op3_bits))));
						}
						if (sc_uint<32>(ins.range(ins_op4_bits)) != sc_uint<32>(0)) {
							consumer_table.push_back(cte(sc_uint<32>(ins.range(ins_op4_bits))));
						}
						if (sc_uint<32>(ins.range(ins_op5_bits)) != sc_uint<32>(0)) {
							consumer_table.push_back(cte(sc_uint<32>(ins.range(ins_op5_bits))));
						}
						if (sc_uint<32>(ins.range(ins_op6_bits)) != sc_uint<32>(0)) {
							consumer_table.push_back(cte(sc_uint<32>(ins.range(ins_op6_bits))));
						}
						cout << "consumer_table: ";
						for (int i = 0; i < consumer_table.size(); i++) {
							cout << consumer_table[i].consumer_addr << " ";
						}
						cout << endl;
					}
					State = SU_ins_config_done;
				}
				else {
					cout << "The configuration is invalid, because the icache has not been initialized or the SU has not been paused!" << endl;
				}
			}

			else if (mes_type == message_start) {
				if ((State == SU_ins_config_done) || (State == SU_pause)) {
					State = SU_start;
					current_ins_valid.write(true);
					cout << "current_ins gets started!" << endl;
				}
				else
					cout << "The SU start is invalid, because SU has not finished the configuration or been paused" << endl;
			}

			else if (mes_type == message_pause) {
				if (State == SU_start) {
					State = SU_pause;
					current_ins_valid.write(false);
					cout << "current_ins is paused!" << endl;
				}
				else
					cout << "The SU pause is invalid, because SU has not got started!" << endl;
			}
		}
		wait();
	}
}

SC_MODULE(SU_in) {
	sc_in_clk clk;
	sc_in<bool> rst;

	sc_in<Message> consume_message_in;
	sc_out<Message> consume_message_out;

	sc_out<Message> produce_message_out;
	sc_in<Message> produce_message_in;

	sc_in<Instruction> current_ins;
	sc_in<bool> current_ins_valid;

	sc_out<bool> alloc_req;
	sc_out<sc_uint<32>> alloc_size[3];
	sc_in<sc_uint<2>> alloc_ack;
	sc_in<sc_uint<32>> alloc_addr[3];

	sc_out<bool> mem_write;
	sc_out<sc_uint<32>> mem_write_addr;
	sc_out<Message> mem_write_data;

	sc_in<bool> ALU_busy;
	sc_out<bool> ALU_start;
	sc_out<sc_uint<8>> ALU_type;
	sc_out<sc_uint<32>> ALU_in_mem_addr[3];
	sc_out<sc_uint<32>> ALU_out_mem_addr;
	sc_out<sc_biguint<144>> ALU_para;
	sc_in<bool> ALU_finish;

	void SU_in_data_receive();
	void SU_in_ALU_control();

	SC_CTOR(SU_in) {
		SC_CTHREAD(SU_in_data_receive, clk.pos());
		SC_CTHREAD(SU_in_ALU_control, clk.pos());
		reset_signal_is(rst, true);
	}
};


void SU_in::SU_in_data_receive() {

	alloc_req.write(false);
	alloc_size[0].write(0);
	alloc_size[1].write(0);
	alloc_size[2].write(0);
	consume_message_out.write(0);

	mem_write.write(false);
	mem_write_addr.write(0);
	mem_write_data.write(0);

	op_req_pt[0] = 0;
	op_req_pt[1] = 0;
	op_rec_data_pt[0] = 0;
	op_rec_data_pt[1] = 0;
	wait();

	while (true) {
		alloc_req.write(false);
		alloc_size[0].write(0);
		alloc_size[1].write(0);
		alloc_size[2].write(0);
		consume_message_out.write(0);

		mem_write.write(false);
		mem_write_addr.write(0);
		mem_write_data.write(0);

		
		if (current_ins_valid.read() == true) {
			
			Message mes = consume_message_in.read();
			sc_uint<8> mes_head = sc_uint<8>(mes.range(mes_head_bits));
			if (mes_head == message_head) {

				sc_uint<8> mes_type = sc_uint<8>(mes.range(mes_type_bits));
				sc_uint<8> mes_seq = sc_uint<8>(mes.range(mes_seq_bits));
				sc_uint<32> mes_source = sc_uint<32>(mes.range(mes_source_bits));
				
				//req and ack
				if (mes_type == message_req) {
					//when receive req message, SU enable memory manager to allocate memory
					Instruction ins = current_ins.read();

					if (ins.range(ins_head_bits) == In_Config) {
						int op = rte_find_source(ins, mes_source);
						if (((op == 0) && (op_req_pt[0] >= op_req_pt[1])) ||
							((op == 1) && (op_req_pt[1] >= op_req_pt[0]))){
							if (rt.empty() || (mes_seq != rt.back().seq)) {
								rte new_rte(ins, mes_seq);
								new_rte.in_state[op] = received_req;

								alloc_req.write(true);
								alloc_size[0].write(new_rte.in_mem_size[0]);
								alloc_size[1].write(new_rte.in_mem_size[1]);
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
									if ((new_rte.in_state[0] != none) && (new_rte.ins_op_type[0] == actor_type))
											new_rte.in_mem_addr[0] = alloc_addr[0].read();
					
									if ((new_rte.in_state[1] != none) && (new_rte.ins_op_type[1] == actor_type)) 
											new_rte.in_mem_addr[1] = alloc_addr[1].read();
										
									new_rte.out_mem_addr = alloc_addr[2].read();
									new_rte.out_ref_reg = consumer_table.size();
									new_rte.SBP_mode = consumer_SBP_mode;

									cout << "rte" << op_req_pt[op] << ": " << new_rte.in_mem_addr[0] << " " << new_rte.in_mem_addr[1] << " " << new_rte.out_mem_addr << endl;
									cout << "rte" << op_req_pt[op] << ": " "op" << op << "'s state is  received_req" << endl;
									rt.push_back(new_rte);
									op_req_pt[op]++;

									consume_message_out.write(creat_Message("ack", sc_uint<32>(0x80000001), mes_source, mes_seq));
								}
								else {
									cout << "memory allocation failed!" << endl;
								}
							}
						}

						else if((op != -1)){
							if (rt[op_req_pt[op]].seq == mes_seq) {
								if (rt[op_req_pt[op]].in_state[op] == idle) {
									rt[op_req_pt[op]].in_state[op] = received_req;
									cout << "rte" << op_req_pt[op] << ": " "op" << op << "'s state is  received_req" << endl;
									op_req_pt[op]++;
									consume_message_out.write(creat_Message("ack", sc_uint<32>(0x80000001), mes_source, mes_seq));
								}
								else {
									cout << "Error: rt[" << op_req_pt[op] << "].in_state[" << op << "] is not idle" << endl;
								}
							}
							else {
								cout << "Error: rte" << op_req_pt[op] << ": " "op" << op << "'s req_dataflow is out-of-order!" << endl;
							}
						}

						else if (ins.range(ins_type_bits) == LOAD) {
							rte new_rte(ins, mes_seq);

							alloc_req.write(true);
							alloc_size[0].write(new_rte.in_mem_size[0]);
							alloc_size[1].write(new_rte.in_mem_size[1]);
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
								new_rte.out_mem_addr = alloc_addr[2].read();
								rt.push_back(new_rte);
								cout << "rte" << rt.size()-1 << ": " << new_rte.in_mem_addr[0] << endl;
								cout << "preparing for loading data!" << endl;
							}
							else {
								cout << "memory allocation failed!" << endl;
							}
						}
					}
				}
			
			
				//receive data package
				else if (mes_type == message_trans_data) {
					//when receive req message, SU enable memory manager to allocate memory
					sc_uint<32> mes_op = sc_uint<32>(mes.range(mes_op_bits));
					Instruction ins = current_ins.read();

					if (ins.range(ins_head_bits) == In_Config) {
						int op = rte_find_source(ins, mes_source);
						int op_index = op_rec_data_pt[op];

						if (mes_seq == rt[op_index].seq) {
							rt[op_index].in_state[op] = receiving_data;
							cout << "rte" << op_index << ": " "op" << op << " is receiving data! seq:" << mes_seq << endl;
							for (unsigned int j = 0; j < mes_op; j++) {
								wait();
								mem_write.write(true);
								mem_write_data.write(consume_message_in.read());
								mem_write_addr.write(sc_uint<32>(rt[op_index].in_mem_addr[op] + sc_uint<32>(j * (Message_Width / 32))));
								//cout << "data: " << hex << consume_message_in.read() << endl;
								//cout << "addr: " << hex << sc_uint<32>(rt[op_index].in_mem_addr[op] + sc_uint<32>(j * (Message_Width / 32))) << endl;
							}
							rt[op_index].in_state[op] = wait_for_consume_data;
							op_rec_data_pt[op]++;
						}
						else {
							cout << "Error: rte" << op_index << ": " "op" << op << "'s dataflow is out-of-order!" << endl;
							for (unsigned int j = 0; j < mes_op; j++)
								wait();
						}
					}
				}
			}
		}
		wait();
	}
}

void SU_in::SU_in_ALU_control() {
	ALU_start.write(false);
	ALU_type.write(sc_uint<8>(0));
	ALU_para.write(sc_biguint<144>(0));
	ALU_in_mem_addr[0].write(sc_uint<32>(0));
	ALU_in_mem_addr[1].write(sc_uint<32>(0));
	ALU_out_mem_addr.write(sc_uint<32>(0));
	rt_ALU_pt = 0;

	while (true) {
		ALU_start.write(false);
		ALU_in_mem_addr[0].write(sc_uint<32>(0));
		ALU_in_mem_addr[1].write(sc_uint<32>(0));
		ALU_out_mem_addr.write(sc_uint<32>(0));
		ALU_type.write(sc_uint<8>(current_ins.read().range(ins_type_bits)));
		ALU_para.write(sc_biguint<144>(current_ins.read().range(ins_para_bits)));

		if ((rt.size()> rt_ALU_pt) && (ALU_busy.read()==false)) {
			if (((rt[rt_ALU_pt].in_state[0] == wait_for_consume_data) && (rt[rt_ALU_pt].in_state[1] == wait_for_consume_data)) ||
				((rt[rt_ALU_pt].in_state[0] == wait_for_consume_data) && (rt[rt_ALU_pt].in_state[1] == none))) {
				ALU_start.write(true);
				ALU_in_mem_addr[0].write(rt[rt_ALU_pt].in_mem_addr[0]);
				ALU_in_mem_addr[1].write(rt[rt_ALU_pt].in_mem_addr[1]);
				ALU_out_mem_addr.write(rt[rt_ALU_pt].out_mem_addr);
				rt_ALU_pt++;
			}
		}
		wait();
	}
}
