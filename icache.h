#pragma once
#include<iostream>

#include"systemc.h"
#include"Instruction.h"
#include"Message.h"

#define icache_size 1024

using namespace std;

enum icache_state {
    ins_reset,
    ins_init,
    ins_init_done,
    ins_config,
    ins_config_done,
    ins_wait_config
};

SC_MODULE(icache)
{
    sc_in<bool> rst;

    sc_in<bool> icache_write;
    sc_in<sc_uint<32>> icache_write_addr;
    sc_in<Instruction> icache_write_ins;

    sc_in<bool> icache_read;
    sc_in<sc_uint<32>> icache_read_addr;
    sc_out<Instruction> icache_read_ins;

    Instruction ins_mem[icache_size] = { 0 };

    void icache_write_main();
    void icache_read_main();

    SC_CTOR(icache){
        SC_METHOD(icache_write_main);
        sensitive << rst << icache_write << icache_write_addr;
        SC_METHOD(icache_read_main);
        sensitive << icache_read << icache_read_addr;
    }
};

void icache::icache_write_main() {
    if (rst.read() == true) {
        for (int i = 0; i < icache_size; i++) {
            ins_mem[i] = 0;
        }
    }
    else if (icache_write.read() == true) {
        ins_mem[icache_write_addr.read()] = icache_write_ins.read();
    }
}

void icache::icache_read_main() {
    if (icache_read.read() == true) {
        icache_read_ins.write(ins_mem[icache_read_addr.read()]);
    }
    else {
        icache_read_ins.write(0);
    }
}

