#pragma once
#include<iostream>
#include<vector>
#include"basic.h"
#include"Instruction.h"
#include"Message.h"

using namespace std;

//memory table entry
struct mte {
	unsigned int addr;
	bool valid = false;
	unsigned int valid_block_num;
};


class Memory_Manager : public sc_module {
public:
	sc_in_clk clk;
	sc_in<bool> rst;

	sc_in<bool> alloc_req;
	sc_in<bool> alloc_valid[Max_op_num + 1];
	sc_in<sc_uint<32>> alloc_size[Max_op_num + 1];
	sc_out<sc_uint<2>> alloc_ack;
	sc_out<sc_uint<32>> alloc_addr[Max_op_num + 1];

	sc_in<bool> in_release_req;
	sc_in<bool> out_release_req;
	sc_in<bool> release_valid[Max_op_num + 1];
	sc_in<sc_uint<32>> release_addr[Max_op_num + 1];

	mte memory_table[block_num];

	void  memory_alloc();
	void  memory_release();

	ofstream* of;

	SC_HAS_PROCESS(Memory_Manager);

	Memory_Manager(sc_module_name name_, ofstream* of):sc_module(name_), of(of) {
		SC_CTHREAD(memory_alloc, clk.pos());
		SC_CTHREAD(memory_release, clk.pos());
		reset_signal_is(rst, true);
	}
};

void Memory_Manager::memory_alloc() {
	for (int i = 0; i < block_num; i++) {
		memory_table[i].addr = sc_uint<32>(i << block_size);
		memory_table[i].valid = false;
		memory_table[i].valid_block_num = 0;
	}

	alloc_ack.write(0);
	for (int i = 0; i < Max_op_num + 1; i++)
		alloc_addr[i].write(0);
	wait();

	while (true) {
		if (alloc_req.read() == true) {
			sc_uint<32>* addr = new sc_uint<32>[Max_op_num + 1] { 0 };
			sc_uint<32>* size = new sc_uint<32>[Max_op_num + 1] { 0 };
			for (int i = 0; i < Max_op_num + 1; i++)
				size[i] = (alloc_size[i].read() >> block_size) + (alloc_size[i].read().range(block_size - 1, 0) > 0 ? 1 : 0);

			bool* flag = new bool[Max_op_num + 1] { 0 };
			bool final_flag = 1;

			for (int i = 0; i < Max_op_num + 1; i++) {
				if (alloc_valid[i].read() == true) {
					unsigned int valid_size = 0;
					addr[i] = 0;
					for (unsigned int j = 0; j < block_num; j++) {
						if (memory_table[j].valid == true) {
							flag[i] = 0;
							addr[i] = j + 1;
							valid_size = 0;
						}
						else {
							valid_size++;
							if (valid_size >= size[i]) {
								flag[i] = 1;
								memory_table[addr[i]].valid_block_num = size[i];
								for (unsigned int k = 0; k < valid_size; k++)
									memory_table[addr[i] + k].valid = true;
								break;
							}
						}
					}
					final_flag = final_flag && flag[i];
				}
			}

			if (final_flag) {
				alloc_ack.write(sc_uint<2>(1));
				for (int i = 0; i < Max_op_num + 1; i++)
					alloc_addr[i].write(addr[i] << block_size);

				unsigned int alloc_num = 0;
				for (int i = 0; i < block_num; i++) {
					if (memory_table[i].valid) alloc_num++;
				}
				*of << "Time " << dec << sc_time_stamp() << ": mem_alloc_rate: " << float(alloc_num) / block_num * 100 << "%" << endl;
			}
			else {
				alloc_ack.write(sc_uint<2>(2));
				for (int i = 0; i < Max_op_num + 1; i++) {
					if (flag[i]) {
						for (unsigned int k = 0; k < size[i]; k++) {
							memory_table[addr[i] + k].valid = false;
						}
						memory_table[addr[i]].valid_block_num = 0;
					}
				}
			}
		}
		wait();
		alloc_ack.write(0);
		for (int i = 0; i < Max_op_num + 1; i++)
			alloc_addr[i].write(0);
	}
}

void Memory_Manager::memory_release() {
	wait();
	while (true) {
		if (in_release_req.read() == true) {
			for (int i = 0; i < Max_op_num; i++) {
				if (release_valid[i].read() == true) {
					unsigned int addr = release_addr[i].read() >> block_size;
					for (unsigned int k = 0; k < memory_table[addr].valid_block_num; k++) {
						memory_table[addr + k].valid = false;
					}
					memory_table[addr].valid_block_num = 0;
					//cout << "mem is released: " << hex << release_addr[0] << endl;
				}
			}
		}

		if (out_release_req.read() == true) {
			if (release_valid[Max_op_num].read() == true) {
				unsigned int addr = release_addr[Max_op_num].read() >> block_size;
				for (unsigned int k = 0; k < memory_table[addr].valid_block_num; k++) {
					memory_table[addr + k].valid = false;
				}
				memory_table[addr].valid_block_num = 0;
				//cout << "mem is released: " << hex << release_addr[2] << endl;
			}
		}

		if (in_release_req.read() || out_release_req.read()) {
			unsigned int alloc_num = 0;
			for (int i = 0; i < block_num; i++) {
				if (memory_table[i].valid) alloc_num++;
			}
			*of << "Time " << dec << sc_time_stamp() << ": mem_alloc_rate: " << float(alloc_num) / block_num * 100 << "%" << endl;
		}
		wait();
	}
}