#pragma once
#include "systemc.h"
#include"basic.h"
#include"Instruction.h"
#include<string>

using namespace std;

//specifies fields in message structure
#define message_head  sc_uint<8>(0xff)
#define message_tail  sc_uint<8>(0xff)

#define message_ins_init	sc_uint<8>(0xf1)
#define message_start		sc_uint<8>(0xf2)
#define message_pause		sc_uint<8>(0xf3)
#define message_continue	sc_uint<8>(0xf4)
#define message_req			sc_uint<8>(0xf5)
#define message_ack			sc_uint<8>(0xf6)
#define message_trans_data	sc_uint<8>(0xf7)

//specifies message structure

typedef sc_biguint<Message_Width> Message;

#define mes_head_bits Message_Width-1,Message_Width-8
#define mes_type_bits Message_Width-9,Message_Width-16
#define mes_source_bits Message_Width-17,Message_Width-48
#define mes_end_bits Message_Width-49,Message_Width-80
#define mes_op_bits Message_Width-81,Message_Width-112
#define mes_seq_bits Message_Width-113,Message_Width-120
#define mes_tial_bits Message_Width-121,Message_Width-128

#define mes_ins_package_bits Ins_Width-1,0
#define mes_package_id Message_Width-1,Message_Width-32


inline Message creat_Message(string mes_type_string, sc_uint<32> source, sc_uint<32> end, sc_uint<8> mes_seq, sc_uint<32> mes_op = 0) {
	sc_uint<8> mes_type;
	if (mes_type_string == "ins_init")
		mes_type = message_ins_init;
	else if (mes_type_string == "start")
		mes_type = message_start;
	else if (mes_type_string == "pause")
		mes_type = message_pause;
	else if (mes_type_string == "continue")
		mes_type = message_continue;
	else if (mes_type_string == "req")
		mes_type = message_req;
	else if (mes_type_string == "ack")
		mes_type = message_ack;
	else if (mes_type_string == "trans_data")
		mes_type = message_trans_data;
	else
		mes_type = 0;
	Message m = Message((message_head, mes_type, source, end, mes_op, mes_seq, message_tail)) << (Message_Width - 128);
	return m;
}
