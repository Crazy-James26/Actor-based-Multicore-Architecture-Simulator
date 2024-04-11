#pragma once
#include"systemc.h"
#include"basic.h"
#include"Message.h"

struct in_mem_pack {
	int id;
	sc_uint<32> in_mem_data [mem_bw_size];
	sc_uint<32> in_mem_addr;
};

class in_mem_fifo : public sc_module {
	sc_fifo<in_mem_pack>** fifo;

	in_mem_fifo() {
		fifo = new sc_fifo<in_mem_pack>*[in_mem_fifo_num];
		for (int i = 0; i < in_mem_fifo_num; i++) {
			fifo[i] = new sc_fifo<in_mem_pack>(in_mem_fifo_depth);
		}
	}

	~in_mem_fifo() {
		for (int i = 0; i < in_mem_fifo_num; i++) {
			if (fifo[i]) { delete fifo[i]; fifo[i] = 0;}
		}
		if(fifo) { delete fifo; fifo = 0;}
	}

	void fifo_input(Message m, int id) {

	}

};