#pragma once
#include"systemc.h"
#include"basic.h"
#include"Message.h"
#include"Instruction.h"

class actor_communicate : public sc_interface {
public:
	virtual int write(sc_uint<32> actor_addr, Message m) = 0;
	virtual int write_all(Message m) = 0;
	virtual Message read(sc_uint<32> actor_addr) = 0;
};


class actor_interface : public sc_module, public actor_communicate {
public:
	sc_port<sc_signal<Message>, interface_size> actor_port_in;
	sc_port<sc_signal<Message>, interface_size> actor_port_out;

	actor_interface(sc_module_name name_, sc_uint<32>* actor_addr, int actor_num):
		sc_module(name_){
		sc_assert(actor_num <= interface_size);

		adj_actor_addr = new sc_uint<32>[actor_num];
		for (int i = 0; i < actor_num; i++) {
			adj_actor_addr[i] = actor_addr[i];
		}
	}

	~actor_interface() {
		if (adj_actor_addr) { delete adj_actor_addr; adj_actor_addr = 0; }
	}

	int write(sc_uint<32> actor_addr, Message m) {
		for (int i = 0; i < actor_port_in.size(); i++) {
			if (adj_actor_addr[i] == actor_addr) {
				actor_port_out[i]->write(m);
				return 1;
			}
		}
		return 0;
	}

	int write_all(Message m) {
		for (int i = 0; i < actor_port_in.size(); i++) {
				actor_port_out[i]->write(m);
		}
		return 1;
	}

	Message read(sc_uint<32> actor_addr) {
		Message m;
		for (int i = 0; i < actor_port_in.size(); i++) {
			if (adj_actor_addr[i] == actor_addr) {
				m = actor_port_in[i]->read();
				return m;
			}
		}
		return 0;
	}

private:
	sc_uint<32>* adj_actor_addr;
};

class host_actor_communicate : public actor_communicate {
public:
	virtual bool read_busy(sc_uint<32> actor_addr) = 0;
};

class host_actor_interface : public sc_module, public host_actor_communicate {
public:
	sc_port<sc_signal<Message>, max_actor_num> actor_port_in;
	sc_port<sc_signal<Message>, max_actor_num> actor_port_out;
	sc_port<sc_signal<bool>, max_actor_num> actor_busy_port;

	host_actor_interface(sc_module_name name_, sc_uint<32>* actor_addr, int actor_num) :
		sc_module(name_) {
		sc_assert(actor_num <= max_actor_num);

		adj_actor_addr = new sc_uint<32>[actor_num];
		for (int i = 0; i < actor_num; i++) {
			adj_actor_addr[i] = actor_addr[i];
		}
	}

	~host_actor_interface() {
		if (adj_actor_addr) { delete adj_actor_addr; adj_actor_addr = 0; }
	}

	int write(sc_uint<32> actor_addr, Message m) {
		for (int i = 0; i < actor_port_in.size(); i++) {
			if (adj_actor_addr[i] == actor_addr) {
				actor_port_out[i]->write(m);
				return 1;
			}
		}
		return 0;
	}

	int write_all(Message m) {
		for (int i = 0; i < actor_port_in.size(); i++) {
			actor_port_out[i]->write(m);
		}
		return 1;
	}

	Message read(sc_uint<32> actor_addr) {
		Message m;
		int i;
		for (i = 0; i < actor_port_in.size(); i++) {
			if (adj_actor_addr[i] == actor_addr) {
				m = actor_port_in[i]->read();
				return m;
			}
		}
		sc_assert(i < actor_port_in.size());
		return 0;
	}

	bool read_busy(sc_uint<32> actor_addr) {
		bool busy;
		int i;
		for (i = 0; i < actor_busy_port.size(); i++) {
			if (adj_actor_addr[i] == actor_addr) {
				busy = actor_busy_port[i]->read();
				return busy;
			}
		}
		sc_assert(i < actor_port_in.size());
		return true;
	}

private:
	sc_uint<32>* adj_actor_addr;
};