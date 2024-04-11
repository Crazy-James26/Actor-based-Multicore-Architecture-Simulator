#pragma once
#include<fstream>
#include<iostream>
#include<iomanip>
#include<vector>
#include"systemc.h"
#include"basic.h"
#include"Instruction.h"
#include"Message.h"
#include"actor_interface.h"
#include"reservation_table.h"
#include"Gather_Scatter_engine.h"


using namespace std;

enum SU_state {
	SU_reset,
	SU_ins_init,
	SU_ins_init_done,
	SU_run,
	SU_pause,
	SU_finish
};

class SU : public sc_module {
public:
	sc_in_clk clk;
	sc_in<bool> rst;

	sc_port<actor_communicate> actor_port;
	sc_out<bool> actor_busy;

	sc_out<bool> icache_write;
	sc_out<sc_uint<32>> icache_write_addr;
	sc_out<Instruction> icache_write_ins;
	sc_out<bool> icache_read;
	sc_out<sc_uint<32>> icache_read_addr;
	sc_in<Instruction> icache_read_ins;

	sc_out<bool> alloc_req;
	sc_in<sc_uint<2>> alloc_ack;
	sc_out<bool> alloc_valid[Max_op_num + 1];
	sc_out<sc_uint<32>> alloc_size[Max_op_num + 1];
	sc_in<sc_uint<32>> alloc_addr[Max_op_num + 1];

	sc_out<bool> SU_in_write[SU_write_num];
	sc_out<sc_uint<32>> SU_in_write_addr[SU_write_num];
	sc_out<float> SU_in_write_data[SU_write_num][Message_data_num];

	sc_in<bool> ALU_busy;
	sc_out<bool> ALU_start;
	sc_out<sc_uint<8>> ALU_type;
	sc_out<sc_uint<32>> ALU_in_op_num;
	sc_out<sc_uint<32>> ALU_in_mem_addr[Max_op_num];
	sc_out<sc_uint<32>> ALU_out_mem_addr;
	sc_out<sc_biguint<ALU_para_Width>> ALU_para;
	sc_in<bool> ALU_finish;

	sc_out<bool> in_release_req;
	sc_out<bool> out_release_req;
	sc_out<bool> release_valid[Max_op_num + 1];
	sc_out<sc_uint<32>> release_addr[Max_op_num + 1];

	sc_out<bool> SU_out_read[SU_read_num];
	sc_out<sc_uint<32>> SU_out_read_addr[SU_read_num];
	sc_in<float> SU_out_read_data[SU_read_num][Message_data_num];

	sc_uint<32> actor_addr;
	sc_uint<32> host_addr;
	sc_uint<32> adj_actor_addr[interface_size];
	SU_state State;

	sc_uint<32> ins_addr = 0;
	sc_uint<32> ins_num = 0;
	sc_uint<32> ins_iteration = 0;

	rte new_rte;//record the producers

	vector<rte> rt;//reservation station
	mem_rte mem_rt[32];

	unsigned int rt_producer_pt = 0;
	unsigned int rt_write_pt[SU_write_num] = { 0 };
	unsigned int rt_ALU_in_pt = 0;
	unsigned int rt_ALU_out_pt = 0;
	vector<unsigned int> rt_read_pt[SU_read_num];

	void SU_config_main();
	void SU_in_control();
	void SU_in_data_receive();
	void SU_in_ALU_control();
	void SU_out_ALU_control();
	void SU_out_control();
	void SU_out_data_send();

	ofstream* of;

	SC_HAS_PROCESS(SU);

	SU(sc_module_name name_, sc_uint<32> actor_addr, sc_uint<32> host_addr, sc_uint<32>* adj_actor_addr, int adj_actor_num,  ofstream* of) :
		sc_module(name_), actor_addr(actor_addr), host_addr(host_addr),of(of) {

		for (int i = 0; i < interface_size; i++) {
			if (i < adj_actor_num) {
				this->adj_actor_addr[i] = adj_actor_addr[i];
			}
			else {
				this->adj_actor_addr[i] = sc_uint<32>(0);
			}
		}

		SC_CTHREAD(SU_config_main, clk.pos());
		SC_CTHREAD(SU_in_control, clk.pos());
		SC_CTHREAD(SU_in_data_receive, clk.pos());
		SC_CTHREAD(SU_in_ALU_control, clk.pos());
		SC_CTHREAD(SU_out_ALU_control, clk.pos());
		SC_CTHREAD(SU_out_control, clk.pos());
		SC_CTHREAD(SU_out_data_send, clk.pos());
		reset_signal_is(rst, true);
	}

	~SU() {
		
	}
};

void SU::SU_config_main() {
	State = SU_reset;
	actor_busy.write(false);

	ins_addr = 0;
	ins_num = 0;
	ins_iteration = 0;

	rt.clear();

	icache_write.write(false);
	icache_write_addr.write(0);
	icache_write_ins.write(0);
	icache_read.write(false);
	icache_read_addr.write(0);

	alloc_req.write(false);
	for (int i = 0; i < Max_op_num + 1; i++) {
		alloc_valid[i].write(false);
		alloc_size[i].write(0);
	}

	Message mes = 0;
	sc_uint<8> mes_head = 0;
	sc_uint<8> mes_type = 0;
	sc_uint<32> mes_source = 0;
	sc_uint<32> mes_op = 0;
	sc_uint<8> mes_seq = 0;
	wait();

	while (true) {
		actor_busy.write(rt.empty());
		
		mes = actor_port->read(host_addr);
		mes_head = sc_uint<8>(mes.range(mes_head_bits));
		mes_type = sc_uint<8>(mes.range(mes_type_bits));
		mes_source = sc_uint<32>(mes.range(mes_source_bits));
		mes_op = sc_uint<32>(mes.range(mes_op_bits));
		mes_seq = sc_uint<8>(mes.range(mes_seq_bits));

		if ((State == SU_reset || State == SU_pause || State == SU_finish) &&
			mes_head == sc_uint<8>(0xff) && mes_type == message_ins_init) {
			State = SU_ins_init;
			ins_num = mes_op;
			ins_addr = 0;
			*of << "Time " << dec << sc_time_stamp() << ": icache initialization:" << endl;
			wait();
			while (true) {
				sc_uint<32> ins_package_id = Instruction(actor_port->read(host_addr).range(mes_package_id));
				Instruction input_ins = Instruction(actor_port->read(host_addr).range(mes_ins_package_bits));
				icache_write.write(true);
				icache_write_addr.write(ins_package_id - 1);
				icache_write_ins.write(input_ins);
				*of << dec << "ins" << left << setw(10) << setfill(' ') << ins_package_id - 1 << ": " << hex << input_ins << endl;
				wait();
				if (ins_package_id == ins_num) {
					icache_write.write(false);
					icache_write_addr.write(0);
					icache_write_ins.write(0);
					State = SU_ins_init_done;
					break;
				}
			}
		}
		else if ((State == SU_ins_init_done || State == SU_pause || State == SU_finish) &&
			mes_head == sc_uint<8>(0xff) && mes_type == message_start) {
			State = SU_run;
			ins_addr = 0;
			ins_iteration = mes_op;
			*of << "Time " << dec << sc_time_stamp() << ": begins running!" << endl;
			wait();
		}

		else if (State == SU_pause && mes_head == sc_uint<8>(0xff) && mes_type == message_continue) {
			State = SU_run;
			*of << "Time " << dec << sc_time_stamp() << ": continues running!" << endl;
			wait();
		}

		else if (State == SU_run) {
			if (mes_head == sc_uint<8>(0xff) && mes_type == message_pause) {
				State = SU_pause;
				icache_read.write(false);
				alloc_req.write(false);
				for (int i = 0; i < Max_op_num + 1; i++) {
					alloc_valid[i].write(false);
					alloc_size[i].write(0);
				}
				*of << "Time " << dec << sc_time_stamp() << ": puases running!" << endl;
				wait();
			}
			else {
				icache_read.write(true);
				icache_read_addr.write(ins_addr);
				if (rt.size() < Max_rte_num) {
					wait();
					Instruction ins = icache_read_ins.read();

					if (sc_uint<8>(ins.range(ins_head_bits)) == Out_Config) {
						new_rte.rte_out_config(ins);
						ins_addr++;
						if (ins_addr == ins_num) {
							ins_addr = 0;
							ins_iteration--;
							if (ins_iteration == 0) {
								State = SU_finish;
								*of << "Time " << dec << sc_time_stamp() << ": completes loading instructions!" << endl;
							}
						}
						/**of << "Time " << dec << sc_time_stamp() << ": ";

						if (new_rte.out_mem_type == mem_type) {
							*of << "store_addr: mem_rt" << hex << new_rte.out_mem_addr << " ";
						}
						else {
							*of << "no store! ";
						}
						if (!new_rte.consumer_table.empty()) {
							*of << "consumer_table: {";
							for (int i = 0; i < new_rte.consumer_table.size(); i++) {
								*of << "actor" << hex << new_rte.consumer_table[i].consumer_addr << " ";
							}
							*of << "} ";
							*of << "out_mode: " << new_rte.consumer_out_mode;
						}
						else {
							*of << "no consumer! ";
						}
						*of << endl;*/
					}

					else if (sc_uint<8>(ins.range(ins_head_bits)) == In_Config) {
						new_rte.rte_in_cofig(ins);
						ins_addr++;
						if (ins_addr == ins_num) {
							ins_addr = 0;
							ins_iteration--;
							if (ins_iteration == 0) {
								State = SU_finish;
								*of << "Time " << dec << sc_time_stamp() << ": completes loading instructions!" << endl;
							}
						}
						/**of << "Time " << dec << sc_time_stamp() << ": producer_table: {";
						for (int i = 0; i < new_rte.producer_table.size(); i++)
							if (new_rte.producer_table[i].in_op_type == actor_type) {
								*of << "actor" << hex << new_rte.producer_table[i].actor_addr << " ";
							}
							else if (new_rte.producer_table[i].in_op_type == mem_type || new_rte.producer_table[i].in_op_type == mem_type_release) {
								*of << "mem_rt" << hex << new_rte.producer_table[i].in_mem_addr << " ";
							}
						*of << "}"<< endl;*/
					}

					else if (sc_uint<8>(ins.range(ins_head_bits) == Op_Config)){
						if (new_rte.mem_allocating == false) {
							new_rte.rte_op_config(ins);
							new_rte.mem_allocating = true;
							for (int j = 0; j < new_rte.producer_table.size(); j++) {
								if (sc_uint<8>(ins.range(ins_type_bits)) != STORE) {
									if (new_rte.producer_table[j].in_op_type == actor_type) {
										alloc_valid[j].write(true);
										alloc_size[j].write(new_rte.producer_table[j].in_mem_size);
									}
									else if (new_rte.producer_table[j].in_op_type == mem_type || new_rte.producer_table[j].in_op_type == mem_type_release) {
										if (!mem_rt[new_rte.producer_table[j].in_mem_addr].valid) {
											*of << "Time " << dec << sc_time_stamp() << ": Read Eroor: mem_rt" << hex << new_rte.producer_table[j].in_mem_addr << " is invalid!" << endl;
											new_rte.mem_allocating = false;
											State = SU_pause;
										}
									}
								}
							}
							if (sc_uint<8>(ins.range(ins_type_bits)) != LOAD) {
								if (new_rte.out_mem_type != mem_type) {
									alloc_valid[Max_op_num].write(true);
									alloc_size[Max_op_num].write(new_rte.out_mem_size);
								}
								if (new_rte.out_mem_type == mem_type) {
									if (mem_rt[new_rte.out_mem_addr].valid){
										*of << "Time " << dec << sc_time_stamp() << ": Write Eroor: mem_rt" << hex << new_rte.out_mem_addr << " is valid!" << endl;
										new_rte.mem_allocating = false;
										State = SU_pause;
									}
									else {
										alloc_valid[Max_op_num].write(true);
										alloc_size[Max_op_num].write(new_rte.out_mem_size);
									}
								}
							}
							if (new_rte.mem_allocating == true) {
								alloc_req.write(true);
							}
						}
						else {
							alloc_req.write(false);
							for (int i = 0; i < Max_op_num + 1; i++) {
								alloc_valid[i].write(false);
								alloc_size[i].write(0);
							}

							if (alloc_ack.read() == sc_uint<2>(1)) {
								if (sc_uint<8>(ins.range(ins_type_bits)) == STORE) {
									for (int i = 0; i < new_rte.producer_table.size(); i++) {
										new_rte.producer_table[i].in_alloc_mem_addr = alloc_addr[Max_op_num].read();
									}
								}

								else {
									for (int i = 0; i < new_rte.producer_table.size(); i++) {
										if (new_rte.producer_table[i].in_op_type == actor_type) {
											new_rte.producer_table[i].in_alloc_mem_addr = alloc_addr[i].read();
										}
										else {
											new_rte.producer_table[i].in_alloc_mem_addr = mem_rt[new_rte.producer_table[i].in_mem_addr].mem_addr;
											if (new_rte.producer_table[i].in_op_type == mem_type_release) {
												mem_rt[new_rte.producer_table[i].in_mem_addr].valid = false;
											}
										}
									}
							
									sc_uint<32> out_data_size;
									if (new_rte.consumer_out_mode == Out_Split1 || new_rte.consumer_out_mode == Out_Split2)
										out_data_size = new_rte.out_mem_size / new_rte.consumer_table.size();
									else
										out_data_size = new_rte.out_mem_size;
									new_rte.out_data_package_size = (out_data_size % Message_data_num == 0) ?
										out_data_size / Message_data_num : out_data_size / Message_data_num + 1;
								}

								if (sc_uint<8>(ins.range(ins_type_bits)) == LOAD) {
									new_rte.out_alloc_mem_addr = mem_rt[new_rte.producer_table[0].in_mem_addr].mem_addr;
									new_rte.out_mem_type = new_rte.producer_table[0].in_op_type;
								}

								else {
									new_rte.out_alloc_mem_addr = alloc_addr[Max_op_num].read();
									if (new_rte.out_mem_type == mem_type) {
										mem_rt[new_rte.out_mem_addr].valid = true;
										mem_rt[new_rte.out_mem_addr].mem_addr = new_rte.out_alloc_mem_addr;
									}
								}
								
								*of << "Time " << dec << sc_time_stamp() << ": rte" << dec << rt.size() << ": rte is built!";
								/**of << " in_alloc_mem_addr: {";
								for (int i = 0; i < new_rte.producer_table.size(); i++)
									*of << "mem" << hex << new_rte.producer_table[i].in_alloc_mem_addr<< 
									"(size: " << new_rte.producer_table[i].in_mem_size << ") ";
								*of << "} out_alloc_mem_addr: mem" << hex << new_rte.out_alloc_mem_addr <<
									"(size: " << new_rte.out_mem_size << ") "*/
								*of <<" ins_type: " << new_rte.op_ins.range(ins_type_bits) << " ins_mode : " << new_rte.op_ins.range(ins_mode_bits) << endl;

								rt.push_back(new_rte);
								new_rte.mem_allocating = false;
								ins_addr++;
								if (ins_addr == ins_num) {
									ins_addr = 0;
									ins_iteration--;
									if (ins_iteration == 0) {
										State = SU_finish;
										*of << "Time " << dec << sc_time_stamp() << ": completes loading instructions!" << endl;
									}
								}
							}

							else if (alloc_ack.read() == sc_uint<2>(2)) {
								//*of << "Time " << dec << sc_time_stamp() << ": memory allocation failed!" << endl;
								new_rte.mem_allocating = false;
							}
						}
					}
					else {
						*of << "Time " << dec << sc_time_stamp() << ": Error: invalid instruction!" << endl;
					}
				}
				else {
					wait();
				}
			}
		}
		else {
			wait();
		}
	}
}

void SU::SU_in_control() {
	rt_producer_pt = 0;
	for (int i = 0; i < SU_write_num; i++) {
		rt_write_pt[i] = -1;
	}
	rt.clear();
	wait();

	while (true) {
		if (!rt.empty() && rt_producer_pt<rt.size()) {
			for (int i = 0; i < rt[rt_producer_pt].producer_table.size(); i++) {
				if (rt[rt_producer_pt].producer_table[i].in_state == pte_idle) {
					Message mes = actor_port->read(rt[rt_producer_pt].producer_table[i].actor_addr);
					if (sc_uint<8>(mes.range(mes_head_bits)) == message_head && sc_uint<8>(mes.range(mes_type_bits)) == message_req) {
						sc_uint<8> mes_seq = sc_uint<8>(mes.range(mes_seq_bits));
						for (int j = 0; j < SU_write_num; j++) {
							if (rt_write_pt[j] == -1) {
								rt[rt_producer_pt].producer_table[i].in_state = pte_received_req;
								*of << "Time " << dec << sc_time_stamp() << ": rte" << rt_producer_pt << ": receives req from producer" <<
									hex << rt[rt_producer_pt].producer_table[i].actor_addr << "'s req! ins_type: " 
									<< rt[rt_producer_pt].op_ins.range(ins_type_bits) << " ins_mode: " << sc_uint<8>(rt[rt_producer_pt].op_ins.range(ins_mode_bits)) << endl;
								rt_write_pt[j] = i;
								break;
							}
						}
					}
				}

				else {
					if (rt[rt_producer_pt].producer_table[i].in_state == pte_sent_ack) {
						Message mes = actor_port->read(rt[rt_producer_pt].producer_table[i].actor_addr);
						if (sc_uint<8>(mes.range(mes_head_bits)) == message_head && sc_uint<8>(mes.range(mes_type_bits)) == message_trans_data) {
							rt[rt_producer_pt].producer_table[i].in_state = pte_receiving_data;
							rt[rt_producer_pt].producer_table[i].in_data_package_size = sc_uint<32>(mes.range(mes_op_bits));
							*of << "Time " << dec << sc_time_stamp() << ": rte" << rt_producer_pt << ": producer" <<
								hex << rt[rt_producer_pt].producer_table[i].actor_addr << " starts sending data! ins_type: " 
								<< rt[rt_producer_pt].op_ins.range(ins_type_bits) << " ins_mode: " << sc_uint<8>(rt[rt_producer_pt].op_ins.range(ins_mode_bits)) << endl;
						}
					}
				}
			}

			bool flag = true;
			for (int i = 0; i < rt[rt_producer_pt].producer_table.size(); i++) {
				if (rt[rt_producer_pt].producer_table[i].in_state < pte_wait_for_consume_data) {
					flag = false;
				}
			}
			if (flag) {
				if (sc_uint<8>(rt[rt_producer_pt].op_ins.range(ins_type_bits)) == LOAD) {
					rt[rt_producer_pt].producer_table[0].in_state = pte_finish;
					rt[rt_producer_pt].out_state = out_sending_data;
				}
				else if (sc_uint<8>(rt[rt_producer_pt].op_ins.range(ins_type_bits)) == STORE) {
					for (int i = 0; i < rt[rt_producer_pt].producer_table.size(); i++) {
						rt[rt_producer_pt].producer_table[i].in_state = pte_finish;
					}
					rt[rt_producer_pt].out_state = out_finish;
				}
				else {
					rt[rt_producer_pt].out_state = out_wait_for_produce_data;
				}
				rt_producer_pt++;
			}
		}
		wait();
	}
}

void SU::SU_in_data_receive() {
	for (int i = 0; i < SU_write_num; i++) {
		SU_in_write[i].write(false);
		SU_in_write_addr[i].write(0);
		for (int j = 0; j < SU_write_num; j++) {
			SU_in_write_data[i][j].write(0);
		}
	}
	wait();

	while (true) {
		if (!rt.empty() && rt_producer_pt < rt.size()) {
			for (int i = 0; i < SU_write_num; i++) {
				if (rt_write_pt[i] != -1) {
					if (rt[rt_producer_pt].producer_table[rt_write_pt[i]].in_state == pte_receiving_data) {
						Message m = actor_port->read(rt[rt_producer_pt].producer_table[rt_write_pt[i]].actor_addr);
						sc_uint<32> data_package_id = sc_uint<32>(m.range(mes_package_id));
						if (data_package_id != 0) {
							SU_in_write[i].write(true);
							sc_uint<32> addr;
							if (rt[rt_producer_pt].producer_in_mode == In_Gather1 || rt[rt_producer_pt].producer_in_mode == In_Gather2)
								addr = rt[rt_producer_pt].producer_table[rt_write_pt[i]].in_alloc_mem_addr +
									gather_engine(rt[rt_producer_pt].producer_in_mode, rt[rt_producer_pt].op_ins, rt_write_pt[i], rt[rt_producer_pt].producer_table.size(), data_package_id - 1);
							else
								addr = rt[rt_producer_pt].producer_table[rt_write_pt[i]].in_alloc_mem_addr + sc_uint<32>((data_package_id - 1) * Message_data_num);
							
							SU_in_write_addr[i].write(addr);
							//*of << dec << "mem[" << left << setw(10) << setfill(' ') << addr << ": " << left << setw(10) << setfill(' ') << addr + Message_data_num - 1 << "]: ";
							for (int j = 0; j < Message_data_num; j++) {
								float data = sc_uint32_cov_float(sc_uint<32>(m >> (j * 32)));
								SU_in_write_data[i][j].write(data);
								//*of << left << setw(10) << setfill(' ') << dec << data;
							}
							//*of << endl;
							if (data_package_id == rt[rt_producer_pt].producer_table[rt_write_pt[i]].in_data_package_size) {
								rt[rt_producer_pt].producer_table[rt_write_pt[i]].in_state = pte_wait_for_consume_data;
								*of << "Time " << dec << sc_time_stamp() << ": rte" << rt_producer_pt << ": producer" <<
									hex << rt[rt_producer_pt].producer_table[rt_write_pt[i]].actor_addr << " finishes sending data! ins_type: "
									<< rt[rt_producer_pt].op_ins.range(ins_type_bits) << " ins_mode: " << sc_uint<8>(rt[rt_producer_pt].op_ins.range(ins_mode_bits)) << endl;
								rt_write_pt[i] = -1;
								
							}
						}
					}
				}
			}
		}
		wait();
		for (int i = 0; i < SU_write_num; i++) {
			SU_in_write[i].write(false);
			SU_in_write_addr[i].write(0);
			for (int j = 0; j < Message_data_num; j++) {
				SU_in_write_data[i][j].write(0);
			}
		}
	}
}

void SU::SU_in_ALU_control() {
	ALU_start.write(false);
	ALU_type.write(0);
	ALU_in_op_num.write(0);
	ALU_para.write(0);
	for (int i = 0; i < Max_op_num; i++) 
		ALU_in_mem_addr[i].write(0);
	ALU_out_mem_addr.write(0);
	rt_ALU_in_pt = 0;
	wait();

	while (true) {
		if (rt_ALU_in_pt < rt_producer_pt && ALU_busy.read() == false) {
			if (rt[rt_ALU_in_pt].out_state == out_wait_for_produce_data) {
				ALU_start.write(true);
				ALU_type.write(sc_uint<8>(rt[rt_ALU_in_pt].op_ins.range(ins_type_bits)));
				ALU_in_op_num.write(rt[rt_ALU_in_pt].producer_table.size());
				ALU_para.write(sc_biguint<ALU_para_Width>(rt[rt_ALU_in_pt].op_ins.range(ins_para_bits)));
				for (int i = 0; i < rt[rt_ALU_in_pt].producer_table.size(); i++)
					ALU_in_mem_addr[i].write(rt[rt_ALU_in_pt].producer_table[i].in_alloc_mem_addr);
				ALU_out_mem_addr.write(rt[rt_ALU_in_pt].out_alloc_mem_addr);
				*of << "Time " << dec << sc_time_stamp() << ": ALU starts compuatation! ins_type: " 
					<< hex << sc_uint<8>(rt[rt_ALU_in_pt].op_ins.range(ins_type_bits)) << " ins_mode: " << sc_uint<8>(rt[rt_ALU_in_pt].op_ins.range(ins_mode_bits)) << endl;
				for (int i = 0; i < rt[rt_ALU_in_pt].producer_table.size(); i++) {
					if (rt[rt_ALU_in_pt].producer_table[i].in_op_type != none_type)
						rt[rt_ALU_in_pt].producer_table[i].in_state = pte_consuming_data;
				}
				rt[rt_ALU_in_pt].out_state = out_producing_data;
				rt_ALU_in_pt++;

				wait();
				ALU_start.write(false);
				ALU_type.write(0);
				ALU_in_op_num.write(0);
				ALU_para.write(0);
				for (int i = 0; i < Max_op_num; i++)
					ALU_in_mem_addr[i].write(0);
				ALU_out_mem_addr.write(0);
				wait();
			}
			else if (rt[rt_ALU_in_pt].out_state > out_wait_for_produce_data) {
				rt_ALU_in_pt++;
				wait();
			}
		}
		else {
			wait();
		}
	}
}

void SU::SU_out_ALU_control() {
	rt_ALU_out_pt = 0;
	in_release_req.write(false);
	for (int i = 0; i < Max_op_num; i++) {
		release_valid[i].write(false);
		release_addr[i].write(sc_uint<32>(0));
	}
	wait();

	while (true) {
		if (rt_ALU_out_pt < rt_ALU_in_pt && ALU_finish.read() == true) {
			if (rt[rt_ALU_out_pt].out_state == out_producing_data) {
				rt[rt_ALU_out_pt].out_state = out_sending_data;
				*of << "Time " << dec << sc_time_stamp() << ": ALU finishes computation! ins_type: " 
					<< hex << sc_uint<8>(rt[rt_ALU_out_pt].op_ins.range(ins_type_bits)) << " ins_mode: " << sc_uint<8>(rt[rt_ALU_out_pt].op_ins.range(ins_mode_bits)) << endl;
				in_release_req.write(true);
				//*of << "Time " << dec << sc_time_stamp() << ": in_mem is released: {";
				for (int i = 0; i < rt[rt_ALU_out_pt].producer_table.size(); i++) {
					if (rt[rt_ALU_out_pt].producer_table[i].in_op_type == actor_type || rt[rt_ALU_out_pt].producer_table[i].in_op_type == mem_type_release) {
						release_valid[i].write(true);
						release_addr[i].write(rt[rt_ALU_out_pt].producer_table[i].in_alloc_mem_addr);
						/**of << "mem" << hex << rt[rt_ALU_out_pt].producer_table[i].in_alloc_mem_addr;
						if (rt[rt_ALU_out_pt].producer_table[i].in_op_type == mem_type_release) {
							*of << "(mem_rt" << hex << rt[rt_ALU_out_pt].producer_table[i].in_mem_addr << ") ";
						}
						else {
							*of << "(actor" << hex << rt[rt_ALU_out_pt].producer_table[i].actor_addr << ") ";
						}*/
					}
				}
				//*of << "} ins_type: " << sc_uint<8>(rt[rt_ALU_out_pt].op_ins.range(ins_type_bits)) << " ins_mode: " << sc_uint<8>(rt[rt_ALU_out_pt].op_ins.range(ins_mode_bits)) << endl;;
				for (int i = 0; i < rt[rt_ALU_out_pt].producer_table.size(); i++) {
					rt[rt_ALU_out_pt].producer_table[i].in_state = pte_finish;
				}
			}
			rt_ALU_out_pt++;
		}
		wait();
		in_release_req.write(false);
		for (int i = 0; i < Max_op_num; i++) {
			release_valid[i].write(false);
			release_addr[i].write(sc_uint<32>(0));
		}
	}
}

void SU::SU_out_control() {
	out_release_req.write(false);
	release_valid[Max_op_num].write(false);
	release_addr[Max_op_num].write(sc_uint<32>(0));
	for (int i = 0; i < SU_read_num; i++) {
		rt_read_pt[i].clear();
	}
	wait();

	while (true) {
		if (!rt.empty()) {
			if (rt[0].out_state == out_sending_data) {
				for (int i = 0; i < rt[0].consumer_table.size(); i++) {
					if (rt[0].consumer_table[i].state == cte_wait_for_req) {
						rt[0].consumer_table[i].state = cte_wait_for_ack;
						rt[0].consumer_table[i].req_wait_cycle = 0;
					}

					else if (rt[0].consumer_table[i].state == cte_wait_for_ack) {
						Message mes = actor_port->read(rt[0].consumer_table[i].consumer_addr);
						if (sc_uint<8>(mes.range(mes_head_bits)) == message_head && sc_uint<8>(mes.range(mes_type_bits)) == message_ack) {
							rt[0].consumer_table[i].state = cte_wait_for_send_data;
							sc_uint<8> data_package_traffic = sc_uint<8>(mes.range(mes_op_bits));
							*of << "Time " << dec << sc_time_stamp() << ": consumer actor" << hex << 
								rt[0].consumer_table[i].consumer_addr << " ack! ins_type: " << sc_uint<8>(rt[0].op_ins.range(ins_type_bits)) << " ins_mode: " << sc_uint<8>(rt[0].op_ins.range(ins_mode_bits)) << endl;
						}
						else {
							if (rt[0].consumer_table[i].req_wait_cycle > 0) {
								rt[0].consumer_table[i].req_wait_cycle--;
							}
						}
					}
				}

				for (int i = 0; i < SU_read_num; i++) {
					if (rt_read_pt[i].empty()) {
						for (int j = 0; j < rt[0].consumer_table.size(); j++) {
							if (rt[0].consumer_table[j].state == cte_wait_for_send_data) {
								rt_read_pt[i].push_back(j);
								rt[0].consumer_table[j].state = cte_sending_data;
								if (rt[0].consumer_out_mode != Out_Broadcast) {
									break;
								}
							}
						
						}
					}
				}

				bool flag = true;
				for (int i = 0; i < rt[0].consumer_table.size(); i++) {
					if (rt[0].consumer_table[i].state != cte_finish) {
						flag = false;
					}
				}
				if (flag) {
					if (rt[0].out_mem_type == mem_type_release) {
						out_release_req.write(true);
						release_valid[Max_op_num].write(true);
						release_addr[Max_op_num].write(rt[0].out_alloc_mem_addr);
						rt[0].out_state = out_finish;
						//*of << "Time " << dec << sc_time_stamp() << ": out_mem is released: mem" << hex << rt[0].out_alloc_mem_addr <<
							//" ins_type : " << sc_uint<8>(rt[0].op_ins.range(ins_type_bits)) << " ins_mode: " << sc_uint<8>(rt[0].op_ins.range(ins_mode_bits)) << endl;
					}
					rt[0].out_state = out_finish;
				}
			}

			if (rt[0].out_state == out_finish) {
				*of << "Time " << dec << sc_time_stamp() << ": rte0: rte is erased! ins_type:" << hex << rt[0].op_ins.range(ins_type_bits) << " ins_mode: " << sc_uint<8>(rt[0].op_ins.range(ins_mode_bits)) << endl;
				rt.erase(rt.begin());
				if (rt_producer_pt > 0) rt_producer_pt--;
				if (rt_ALU_in_pt > 0) rt_ALU_in_pt--;
				if (rt_ALU_out_pt > 0) rt_ALU_out_pt--;
			}
		}
		wait();
		out_release_req.write(false);
		release_valid[Max_op_num].write(false);
		release_addr[Max_op_num].write(sc_uint<32>(0));
	}
}


void SU::SU_out_data_send() {
	for (int i = 0; i < SU_read_num; i++) {
		SU_out_read[i].write(false);
		SU_out_read_addr[i].write(0);
		wait();
	}

	while (true) {
		if (!rt.empty() && rt_producer_pt < rt.size()) {
			for (int i = 0; i < rt[rt_producer_pt].producer_table.size(); i++) {
				if (rt[rt_producer_pt].producer_table[i].in_state == pte_received_req) {
					actor_port->write(rt[rt_producer_pt].producer_table[i].actor_addr,
						creat_Message("ack", actor_addr, rt[rt_producer_pt].producer_table[i].actor_addr, 0));
					*of << "Time " << dec << sc_time_stamp() << ": rte" << rt_producer_pt << ": asks for producer" <<
						hex << rt[rt_producer_pt].producer_table[i].actor_addr << "'s req! ins_type: " 
						<< rt[rt_producer_pt].op_ins.range(ins_type_bits) << " ins_mode: " << sc_uint<8>(rt[0].op_ins.range(ins_mode_bits)) << endl;
					rt[rt_producer_pt].producer_table[i].in_state = pte_sent_ack;
				}
				else if (rt[rt_producer_pt].producer_table[i].in_state == pte_sent_ack) {
					actor_port->write(rt[rt_producer_pt].producer_table[i].actor_addr, Message(0));
				}
			}
		}

		if (!rt.empty() && rt[0].out_state == out_sending_data) {
			for (int i = 0; i < rt[0].consumer_table.size(); i++) {
				if (rt[0].consumer_table[i].state == cte_wait_for_ack){
					if (rt[0].consumer_table[i].req_wait_cycle == 0) {
						actor_port->write(rt[0].consumer_table[i].consumer_addr, creat_Message("req", actor_addr, rt[0].consumer_table[i].consumer_addr, 0));
						rt[0].consumer_table[i].req_wait_cycle = req_ack_max_cycle;
					}
					else {
						actor_port->write(rt[0].consumer_table[i].consumer_addr, 0);
					}
				}
			}

			for (int i = 0; i < SU_read_num; i++) {
				if (!rt_read_pt[i].empty()) {
					sc_uint<32> out_data_package_id = rt[0].consumer_table[rt_read_pt[i][0]].out_data_package_id;
					if (out_data_package_id == 0) {
						*of << "Time " << dec << sc_time_stamp() << ": starts sending data to consumer{";
						for (int j = 0; j < rt_read_pt[i].size(); j++) {
							actor_port->write(rt[0].consumer_table[rt_read_pt[i][j]].consumer_addr,
								creat_Message("trans_data", actor_addr, rt[0].consumer_table[rt_read_pt[i][j]].consumer_addr, 0, rt[0].out_data_package_size));
							*of << "actor" << hex << rt[0].consumer_table[rt_read_pt[i][j]].consumer_addr << " ";

						}
						*of << "} ins_type: " << sc_uint<8>(rt[0].op_ins.range(ins_type_bits)) << " ins_mode: " << sc_uint<8>(rt[0].op_ins.range(ins_mode_bits)) << endl;
					}

					if (out_data_package_id < rt[0].out_data_package_size) {
						SU_out_read[i].write(true);
						if (rt[0].consumer_out_mode == Out_Split1 || rt[0].consumer_out_mode == Out_Split2)
							SU_out_read_addr[i].write(rt[0].out_alloc_mem_addr +
								scatter_engine(rt[0].consumer_out_mode, rt[0].op_ins, rt_read_pt[i][0], rt[0].consumer_table.size(), out_data_package_id));
						else
							SU_out_read_addr[i].write(rt[0].out_alloc_mem_addr + sc_uint<32>(out_data_package_id * Message_data_num));
					}
					else {
						SU_out_read[i].write(false);
					}

					if (out_data_package_id > 0 && out_data_package_id <= rt[0].out_data_package_size) {
						Message m = Message(0);
						for (int k = Message_data_num - 1; k >= 0; k--) {
							float float_data = SU_out_read_data[i][k].read();
							sc_uint<32> uint_data = float_cov_sc_uint32(float_data);
							m = Message((m, uint_data));
						}
						m += (Message(out_data_package_id) << (Message_Width - 32));

						for (int j = 0; j < rt_read_pt[i].size(); j++) {
							actor_port->write(rt[0].consumer_table[rt_read_pt[i][j]].consumer_addr, m);
						}

						/**of << dec << "SU_read_pt"<< i <<": data[" << left << setw(10) << setfill(' ') << (out_data_package_id - 1) * Message_data_num << ": " <<
							left << setw(10) << setfill(' ') << out_data_package_id * Message_data_num - 1 << "]: ";
						for (int k = 0; k < Message_data_num; k++) {
							*of << left << setw(10) << setfill(' ') << dec << SU_out_read_data[i][k].read();
						}
						*of << endl;*/
					}

					if (out_data_package_id <= rt[0].out_data_package_size) {
						rt[0].consumer_table[rt_read_pt[i][0]].out_data_package_id++;
					}
					else {
						for (int j = 0; j < rt_read_pt[i].size(); j++) {
							actor_port->write(rt[0].consumer_table[rt_read_pt[i][j]].consumer_addr, 0);
							rt[0].consumer_table[rt_read_pt[i][j]].state = cte_finish;
						}
						*of << "Time " << dec << sc_time_stamp() << ": finishes sending data to consumer{";
						for (int j = 0; j < rt_read_pt[i].size(); j++) {
							*of << "actor" << hex << rt[0].consumer_table[rt_read_pt[i][j]].consumer_addr << " ";
						}
						*of << "} ins_type: " << sc_uint<8>(rt[0].op_ins.range(ins_type_bits)) << " ins_mode: " << sc_uint<8>(rt[0].op_ins.range(ins_mode_bits)) << endl;
						rt_read_pt[i].clear();
					}
				}
			}
		}

		wait();
	}
}

