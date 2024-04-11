#pragma once
#include<fstream>
#include<iostream>
#include<string>

#include"systemc.h"
#include"actor_interface.h"
#include"icache.h"
#include"SU.h"
#include"Memory_Manager.h"
#include"memory.h"
#include"ALU.h"

using namespace std;


class Actor : public sc_module{
public:
	sc_in_clk clk;
	sc_in<bool> rst;

	sc_port<sc_signal<Message>, interface_size> message_in;
	sc_port<sc_signal<Message>, interface_size> message_out;
	sc_out<bool> actor_busy;

	sc_signal<bool> icache_write;
	sc_signal<sc_uint<32>> icache_write_addr;
	sc_signal<Instruction> icache_write_ins;
	sc_signal<bool> icache_read;
	sc_signal<sc_uint<32>> icache_read_addr;
	sc_signal<Instruction> icache_read_ins;

	sc_signal<Instruction> current_ins;
	sc_signal<bool> current_ins_valid;

	sc_signal<bool> alloc_req;
	sc_signal<bool> alloc_valid[Max_op_num + 1];
	sc_signal<sc_uint<32>> alloc_size[Max_op_num + 1];
	sc_signal<sc_uint<2>> alloc_ack;
	sc_signal<sc_uint<32>> alloc_addr[Max_op_num + 1];

	sc_signal<bool> SU_in_write[SU_write_num];
	sc_signal<sc_uint<32>> SU_in_write_addr[SU_write_num];
	sc_signal<float> SU_in_write_data[SU_write_num][Message_data_num];

	sc_signal<bool> ALU_busy;
	sc_signal<bool> ALU_start;
	sc_signal<sc_uint<8>> ALU_type;
	sc_signal<sc_uint<32>> ALU_in_op_num;
	sc_signal<sc_uint<32>> ALU_in_mem_addr[Max_op_num];
	sc_signal<sc_uint<32>> ALU_out_mem_addr;
	sc_signal<sc_biguint<ALU_para_Width>> ALU_para;
	sc_signal<bool> ALU_finish;
	sc_signal<bool> ALU_mem_read[Max_op_num];
	sc_signal<sc_uint<32>> ALU_mem_read_addr[Max_op_num];
	sc_signal<float> ALU_mem_read_data[Max_op_num][ALU_read_write_num];
	sc_signal<bool> ALU_mem_write;
	sc_signal<sc_uint<32>> ALU_mem_write_addr;
	sc_signal<float> ALU_mem_write_data[ALU_read_write_num];

	sc_signal<bool> in_release_req;
	sc_signal<bool> out_release_req;
	sc_signal<bool> release_valid[Max_op_num + 1];
	sc_signal<sc_uint<32>> release_addr[Max_op_num + 1];

	sc_signal<bool> SU_out_read[SU_read_num];
	sc_signal<sc_uint<32>> SU_out_read_addr[SU_read_num];
	sc_signal<float> SU_out_read_data[SU_read_num][Message_data_num];

	sc_uint<32> actor_addr;
	sc_uint<32> host_addr;

	actor_interface* actor_if;
	icache* ic;
	SU* su;
	Memory_Manager* mm;
	memory* mem;
	ALU* alu;


	SC_HAS_PROCESS(Actor);

	Actor(sc_module_name name_, sc_uint<32> actor_addr, sc_uint<32>* adj_actor_addr, int adj_actor_num, sc_uint<32> host_addr):
		sc_module(name_), actor_addr(actor_addr), host_addr(host_addr){

		int actor_id = int(actor_addr - sc_uint<32>(0x80000000));
		string of_name = "./"+ file_name +"/result/actor_" + to_string(actor_id) + ".txt";
		ofstream* of = new ofstream(of_name);
		if (!of->is_open()) {
			cout << "The file " << of_name << " is unable to open!\n";
		}
		
		actor_if = new actor_interface("actor_if", adj_actor_addr, adj_actor_num);
		actor_if->actor_port_in(message_in);
		actor_if->actor_port_out(message_out);

		ic = new icache("icache");
		ic->rst(rst);
		ic->icache_write(icache_write);
		ic->icache_write_addr(icache_write_addr);
		ic->icache_write_ins(icache_write_ins);
		ic->icache_read(icache_read);
		ic->icache_read_addr(icache_read_addr);
		ic->icache_read_ins(icache_read_ins);
		

		su = new SU("su", actor_addr, host_addr, adj_actor_addr, adj_actor_num, of);
		su->clk(clk);
		su->rst(rst);
		su->actor_port((*actor_if));
		su->actor_busy(actor_busy);
		
		su->icache_write(icache_write);
		su->icache_write_addr(icache_write_addr);
		su->icache_write_ins(icache_write_ins);
		su->icache_read(icache_read);
		su->icache_read_addr(icache_read_addr);
		su->icache_read_ins(icache_read_ins);

		su->alloc_req(alloc_req);
		su->alloc_ack(alloc_ack);
		for (int i = 0; i < Max_op_num + 1; i++) {
			su->alloc_valid[i](alloc_valid[i]);
			su->alloc_size[i](alloc_size[i]);
			su->alloc_addr[i](alloc_addr[i]);
		}

		for (int i = 0; i < SU_write_num; i++) {
			su->SU_in_write[i](SU_in_write[i]);
			su->SU_in_write_addr[i](SU_in_write_addr[i]);
			for (int j = 0; j < Message_data_num; j++) {
				su->SU_in_write_data[i][j](SU_in_write_data[i][j]);
			}
		}

		su->ALU_busy(ALU_busy);
		su->ALU_start(ALU_start);
		su->ALU_type(ALU_type);
		su->ALU_in_op_num(ALU_in_op_num);
		for (int i = 0; i < Max_op_num; i++) {
			su->ALU_in_mem_addr[i](ALU_in_mem_addr[i]);
		}
		su->ALU_out_mem_addr(ALU_out_mem_addr);
		su->ALU_para(ALU_para);
		su->ALU_finish(ALU_finish);

		su->in_release_req(in_release_req);
		su->out_release_req(out_release_req);
		for (int i = 0; i < Max_op_num + 1; i++) {
			su->release_valid[i](release_valid[i]);
			su->release_addr[i](release_addr[i]);
		}

		for (int i = 0; i < SU_read_num; i++) {
			su->SU_out_read[i](SU_out_read[i]);
			su->SU_out_read_addr[i](SU_out_read_addr[i]);
			for (int j = 0; j < Message_data_num; j++) {
				su->SU_out_read_data[i][j](SU_out_read_data[i][j]);
			}
		}


		mm = new Memory_Manager("Mem_Manager", of);
		mm->clk(clk);
		mm->rst(rst);
		mm->alloc_req(alloc_req);
		mm->alloc_ack(alloc_ack);
		for (int i = 0; i < Max_op_num + 1; i++) {
			mm->alloc_valid[i](alloc_valid[i]);
			mm->alloc_size[i](alloc_size[i]);
			mm->alloc_addr[i](alloc_addr[i]);
		}
		mm->in_release_req(in_release_req);
		mm->out_release_req(out_release_req);
		for (int i = 0; i < Max_op_num + 1; i++) {
			mm->release_valid[i](release_valid[i]);
			mm->release_addr[i](release_addr[i]);
		}


		mem = new memory("memory");
		mem->clk(clk);
		mem->rst(rst);
		for (int i = 0; i < SU_write_num; i++) {
			mem->SU_in_write[i](SU_in_write[i]);
			mem->SU_in_write_addr[i](SU_in_write_addr[i]);
			for (int j = 0; j < Message_data_num; j++) {
				mem->SU_in_write_data[i][j](SU_in_write_data[i][j]);
			}
		}
		for (int i = 0; i < Max_op_num; i++) {
			mem->ALU_mem_read[i](ALU_mem_read[i]);
			mem->ALU_mem_read_addr[i](ALU_mem_read_addr[i]);
		}
		mem->ALU_mem_write(ALU_mem_write);
		mem->ALU_mem_write_addr(ALU_mem_write_addr);
		for (int i = 0; i < ALU_read_write_num; i++) {
			for (int j = 0; j < Max_op_num; j++)
				mem->ALU_mem_read_data[j][i](ALU_mem_read_data[j][i]);
			mem->ALU_mem_write_data[i](ALU_mem_write_data[i]);
		}
		for (int i = 0; i < SU_read_num; i++) {
			mem->SU_out_read[i](SU_out_read[i]);
			mem->SU_out_read_addr[i](SU_out_read_addr[i]);
			for (int j = 0; j < Message_data_num; j++) {
				mem->SU_out_read_data[i][j](SU_out_read_data[i][j]);
			}
		}


		alu = new ALU("ALU");
		alu->clk(clk);
		alu->rst(rst);
		alu->ALU_busy(ALU_busy);
		alu->ALU_start(ALU_start);
		alu->ALU_type(ALU_type);
		alu->ALU_in_op_num(ALU_in_op_num);
		for (int i = 0; i < Max_op_num; i++) {
			alu->ALU_in_mem_addr[i](ALU_in_mem_addr[i]);
		}
		alu->ALU_out_mem_addr(ALU_out_mem_addr);
		alu->ALU_para(ALU_para);
		alu->ALU_finish(ALU_finish);
		for (int i = 0; i < Max_op_num; i++) {
			alu->ALU_mem_read[i](ALU_mem_read[i]);
			alu->ALU_mem_read_addr[i](ALU_mem_read_addr[i]);
		}
		alu->ALU_mem_write(ALU_mem_write);
		alu->ALU_mem_write_addr(ALU_mem_write_addr);
		for (int i = 0; i < ALU_read_write_num; i++) {
			for (int j = 0; j < Max_op_num; j++)
				alu->ALU_mem_read_data[j][i](ALU_mem_read_data[j][i]);
			alu->ALU_mem_write_data[i](ALU_mem_write_data[i]);
		}
	}

	~Actor() {
		if (ic) { delete ic;  ic = 0; }
		if (su) { delete su;  su = 0; }
		if (mm) { delete mm;  mm = 0; }
		if (mem) { delete mem;  mem = 0; }
		if (alu) { delete alu;  alu = 0; }
	}
};