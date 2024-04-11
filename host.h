#pragma once
#include<iostream>
#include<string>

#include"systemc.h"
#include"Instruction.h"
#include"Message.h"
#include"sub_host.h"

using namespace std;


class host : public sc_module {
public:
	sc_in_clk clk;
	sc_in<bool> rst;
	

	sc_port<sc_signal<Message>, max_actor_num> message_in;
	sc_port<sc_signal<Message>, max_actor_num> message_out;
	sc_port<sc_signal<bool>, max_actor_num> actor_busy_port;

	host_actor_interface* host_actor_if;

	sc_uint<32> host_addr;
	sc_uint<32>* adj_actor_addr;
	sub_host** sh;
	int adj_actor_num;

	SC_HAS_PROCESS(host);

	host(sc_module_name name_, sc_uint<32> host_addr, sc_uint<32>* adj_actor_addr, int adj_actor_num) :
		sc_module(name_), host_addr(host_addr), adj_actor_num(adj_actor_num) {
		this->adj_actor_addr = new sc_uint<32>[adj_actor_num];
		for (int i = 0; i < adj_actor_num; i++) {
			this->adj_actor_addr[i] = adj_actor_addr[i];
		}

		host_actor_if = new host_actor_interface("host_actor_if", this->adj_actor_addr, adj_actor_num);
		host_actor_if->actor_port_in(message_in);
		host_actor_if->actor_port_out(message_out);
		host_actor_if->actor_busy_port(actor_busy_port);

		sh = new sub_host* [this->adj_actor_num];
		for (int i = 0; i < this->adj_actor_num; i++) {
			string sh_name_str = "sub_host"+ to_string(i);
			sc_module_name sh_name = sh_name_str.c_str();
			sh[i] = new sub_host(sh_name, host_addr, this->adj_actor_addr[i], i+1);
			sh[i]->clk(clk);
			sh[i]->rst(rst);
			sh[i]->actor_port((*host_actor_if));
		}
	}

	~host() {
		for (int i = 0; i < adj_actor_num; i++) {
			if (sh[i]) { delete sh[i]; sh[i] = 0; }
		}
		if (sh) { delete sh; sh = 0; }
		if (adj_actor_addr) { delete adj_actor_addr; adj_actor_addr = 0; }
	}
};


