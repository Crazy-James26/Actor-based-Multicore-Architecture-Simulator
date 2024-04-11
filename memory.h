#pragma once
#include<iomanip>
#include<iostream>

#include"systemc.h"
#include"basic.h"
#include"Message.h"
#include"Memory_Manager.h"


SC_MODULE(memory) {
	sc_in_clk clk;
	sc_in<bool> rst;

	sc_in<bool> SU_in_write[SU_write_num];
	sc_in<sc_uint<32>> SU_in_write_addr[SU_write_num];
	sc_in<float> SU_in_write_data[SU_write_num][Message_data_num];

	sc_in<bool> ALU_mem_read[Max_op_num];
	sc_in<sc_uint<32>> ALU_mem_read_addr[Max_op_num];
	sc_out<float> ALU_mem_read_data[Max_op_num][ALU_read_write_num];
	
	sc_in<bool> ALU_mem_write;
	sc_in<sc_uint<32>> ALU_mem_write_addr;
	sc_in<float> ALU_mem_write_data[ALU_read_write_num];

	sc_in<bool> SU_out_read[SU_read_num];
	sc_in<sc_uint<32>> SU_out_read_addr[SU_read_num];
	sc_out<float> SU_out_read_data[SU_read_num][Message_data_num];


	float mem[1<<mem_size] = { 0 };
	void SU_in_write_main();
	void ALU_write_main();
	void ALU_read_main();
	void SU_out_read_main();

	SC_CTOR(memory) {
		SC_METHOD(SU_in_write_main);
		sensitive << SU_in_write[0] << SU_in_write_addr[0] << SU_in_write[1] << SU_in_write_addr[1];

		SC_METHOD(ALU_write_main);
		sensitive << ALU_mem_write << ALU_mem_write_addr;

		SC_METHOD(ALU_read_main);
		sensitive << ALU_mem_read[0] << ALU_mem_read_addr[0] << ALU_mem_read[1] << ALU_mem_read_addr[1];

		SC_METHOD(SU_out_read_main);
		sensitive << SU_out_read[0] << SU_out_read_addr[0] << SU_out_read[1] << SU_out_read_addr[1];
	};
};

void memory::SU_in_write_main() {
	for (int i = 0; i < SU_write_num; i++) {
		if (SU_in_write[i].read() == true) {
			sc_uint<32> addr = SU_in_write_addr[i].read();
			//cout << dec << "mem[" << left << setw(5) << setfill(' ') << addr << ": " << left << setw(5) << setfill(' ') << addr + 7 << "]: ";
			for (unsigned int j = 0; j < Message_data_num; j++) {
				mem[addr + j] = SU_in_write_data[i][j].read();
				//cout << left << setw(5) << setfill(' ') << dec << mem[addr + j];
			}
			//cout << endl;
		}
	}
}

void memory::ALU_read_main() {
	for (int i = 0; i < Max_op_num; i++) {
		if (ALU_mem_read[i].read() == true) {
			sc_uint<32> addr = ALU_mem_read_addr[i].read();
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				ALU_mem_read_data[i][j].write(mem[addr + j]);
			}
		}
		else {
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				ALU_mem_read_data[i][j].write(float(0));
			}
		}
	}
}

void memory::ALU_write_main() {
	if (ALU_mem_write.read() == true) {
		sc_uint<32> addr = ALU_mem_write_addr.read();
		//cout << dec << "mem[" << left << setw(5) << setfill(' ') << addr << ": " << left << setw(5) << setfill(' ') << addr + 7 << "]: ";
		for (unsigned int i = 0; i < ALU_read_write_num; i++) {
			mem[addr + i] = ALU_mem_write_data[i].read();
			//cout << left << setw(5) << setfill(' ') << dec << mem[addr + i];
		}
		//cout << endl;
	}
}

void memory::SU_out_read_main() {
	for (int i = 0; i < SU_read_num; i++) {
		if (SU_out_read[i].read() == true) {
			sc_uint<32> addr = SU_out_read_addr[i].read();
			for (unsigned int j = 0; j < Message_data_num; j++) {
				SU_out_read_data[i][j].write(mem[addr + j]);
			}
		}
		else {
			for (unsigned int j = 0; j < Message_data_num; j++) {
				SU_out_read_data[i][j].write(float(0));
			}
		}
	}
}
