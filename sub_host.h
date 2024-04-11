#pragma once
#include<fstream>
#include<iostream>
#include<iomanip>
#include<string>

#include"systemc.h"
#include"basic.h"
#include"Instruction.h"
#include"Message.h"
#include"actor_interface.h"

using namespace std;

class sub_host : public sc_module {
public:
	sc_in_clk clk;
	sc_in<bool> rst;

	sc_port<host_actor_communicate> actor_port;

	sc_uint<32> host_addr;
	sc_uint<32> actor_addr;
	int sh_id;

	void sub_host_out();
	//void sub_host_in();

	SC_HAS_PROCESS(sub_host);

	sub_host(sc_module_name name_, sc_uint<32> host_addr, sc_uint<32> actor_addr, int sh_id) :
		sc_module(name_), host_addr(host_addr), actor_addr(actor_addr), sh_id(sh_id) {
		SC_CTHREAD(sub_host_out, clk.pos());
		reset_signal_is(rst, true);
	}
};

void sub_host::sub_host_out() {
	string inf_name = "./"+ file_name + "/script/host_" + to_string(sh_id) + ".txt";

	ifstream inf(inf_name);
	if (!inf.is_open()) {
		cout << "The file " << inf_name << " is unable to open!\n";
	}

	else {
		string s;
		inf >> s >> s >> s;
		int wait_cycle;

		sc_uint<8> Message_head;
		sc_uint<8> Message_type;
		sc_uint<32> Message_source;
		sc_uint<32> Message_end;
		sc_uint<32> Message_op;
		sc_uint<8> Message_seq;
		sc_uint<8> Message_hail;

		sc_uint<8> ins_head;
		sc_uint<8> ins_type;

		sc_uint<2> ins_op1_type;
		sc_uint<32> ins_op1;
		sc_uint<2> ins_op2_type;
		sc_uint<32> ins_op2;
		sc_uint<2> ins_op3_type;
		sc_uint<32> ins_op3;
		sc_uint<2> ins_op4_type;
		sc_uint<32> ins_op4;

		sc_uint<16> para1;
		sc_uint<16> para2;
		sc_uint<16> para3;
		sc_uint<16> para4;
		sc_uint<16> para5;
		sc_uint<16> para6;
		sc_uint<16> para7;
		sc_uint<16> para8;
		sc_uint<8> para9;


		actor_port->write(actor_addr, 0);
		wait();

		while (true) {
			actor_port->write(actor_addr, 0);
			inf >> s;
			if (s == "//")
				getline(inf, s);
			else
			{
				wait_cycle = stoi(s);
				if (wait_cycle >= 0) {
					for (int i = 0; i < wait_cycle; i++) wait();

					inf >> hex >> Message_head >> Message_type >> Message_source >>
						Message_end >> Message_op >> Message_seq >> Message_hail;

					Message m = Message((Message_head, Message_type, Message_source,
						Message_end, Message_op, Message_seq, Message_hail)) << (Message_Width - 128);

					if (Message_type == message_ins_init) {
						if (!actor_port->read_busy(actor_addr))
							cout << "Time " << dec << sc_time_stamp() << ": wait for actor " << hex << Message_end << " being not busy!" << endl;
						while (!actor_port->read_busy(actor_addr)) wait();
						actor_port->write(actor_addr, m);
						wait();

						inf >> s;

						for (unsigned int i = 0; i < Message_op; i++) {
							inf >> hex >> ins_head >> ins_type;
							if (ins_head == Out_Config || ins_head == In_Config) {
								inf >> hex >> ins_op1_type >> ins_op1 >> ins_op2_type >> ins_op2 >>
									ins_op3_type >> ins_op3 >> ins_op4_type >> ins_op4;
								Message m = (Message(sc_uint<32>(i + 1)) << (Message_Width - 32)) +
									Message((ins_head, ins_type, ins_op1_type, ins_op1, ins_op2_type, ins_op2,
										ins_op3_type, ins_op3, ins_op4_type, ins_op4));
								actor_port->write(actor_addr, m);
							}
							else if (ins_head == Op_Config) {
								inf >> hex >> para1 >> para2 >> para3 >> para4 >> para5 >> para6 >> para7 >> para8 >> para9;
								Message m = (Message(sc_uint<32>(i + 1)) << (Message_Width - 32)) +
									Message((ins_head, ins_type, para1, para2, para3, para4, para5, para6, para7, para8, para9));
								actor_port->write(actor_addr, m);
							}
							wait();
						}
					}


					else if (Message_type == message_start || Message_type == message_pause) {
						actor_port->write(actor_addr, m);
						wait();
					}

					else if (Message_type == message_req) {
						int wait_cycle = 0;
						actor_port->write(actor_addr, m);
						wait();

						while ((actor_port->read(actor_addr).range(mes_head_bits) != message_head) ||
							(actor_port->read(actor_addr).range(mes_type_bits) != message_ack)) {
							if (wait_cycle == req_ack_max_cycle) {
								wait_cycle = 0;
								actor_port->write(actor_addr, m);
								wait();
							}
							else {
								wait_cycle++;
								wait();
							}
						}

						cout << "Time " << dec << sc_time_stamp() << ": ack! source:" << hex << Message_end << endl;
					}

					else if (Message_type == message_trans_data) {
						actor_port->write(actor_addr, m);
						wait();

						inf >> s;
						float data[Message_data_num];
						if (s == "data_package:") {
							for (unsigned int i = 0; i < Message_op; i++) {
								Message m = Message(0);
								for (int j = 0; j < Message_data_num; j++) {
									inf >> dec >> data[j];
								}
								for (int j = Message_data_num - 1; j >= 0; j--) {
									m = Message((m, float_cov_sc_uint32(data[j])));
								}
								m += (Message(sc_uint<32>(i + 1)) << (Message_Width - 32));
								actor_port->write(actor_addr, m);
								wait();
							}
						}
						else if(s == "random_data_package"){
							for (unsigned int i = 0; i < Message_op; i++) {
								Message m = Message(0);
								for (int j = 0; j < Message_data_num; j++) {
									data[j] = float((rand() % 21 - 10)) / 100;
								}
								for (int j = Message_data_num - 1; j >= 0; j--) {
									m = Message((m, float_cov_sc_uint32(data[j])));
								}
								m += (Message(sc_uint<32>(i + 1)) << (Message_Width - 32));
								actor_port->write(actor_addr, m);
								wait();
							}
						}
						else if (s == "input_file:") {
							inf >> s;
							string inf0_name = "./" + file_name + "/script/" + s;
							ifstream inf0(inf0_name);
							if (!inf0.is_open()) {
								cout << "The file " << inf0_name << " is unable to open!\n";
							}
							else {
								for (unsigned int i = 0; i < Message_op; i++) {
									Message m = Message(0);
									for (int j = 0; j < Message_data_num; j++) {
										inf0 >> data[j];
									}
									for (int j = Message_data_num - 1; j >= 0; j--) {
										m = Message((m, float_cov_sc_uint32(data[j])));
									}
									m += (Message(sc_uint<32>(i + 1)) << (Message_Width - 32));
									actor_port->write(actor_addr, m);
									wait();
								}
								inf0.close();
							}
						}
					}
				}

				else if (wait_cycle == -2) {
					inf >> s;
					inf >> s;
					string of_name = "./" + file_name +"/result/" + s;
					ofstream of(of_name);
					if (!of.is_open()) {
						cout << "The file " << of_name << " is unable to open!\n";
					}
					else {
						while ((actor_port->read(actor_addr).range(mes_head_bits) != message_head) ||
							(actor_port->read(actor_addr).range(mes_type_bits) != message_req)) {
							wait();
						}
						sc_uint<8> mes_seq = sc_uint<8>(actor_port->read(actor_addr).range(mes_seq_bits));
						actor_port->write(actor_addr, creat_Message("ack", host_addr, actor_addr, mes_seq));
						of << "Time " << dec << sc_time_stamp() << ": ack for producer" << hex << actor_addr << "'s req!" << endl;
						wait();

						while ((actor_port->read(actor_addr).range(mes_head_bits) != message_head) ||
							(actor_port->read(actor_addr).range(mes_type_bits) != message_trans_data)) {
							wait();
						}
						sc_uint<32> in_data_package_size = sc_uint<32>(actor_port->read(actor_addr).range(mes_op_bits));
						of << "Time " << dec << sc_time_stamp() << ": producer" << hex << actor_addr << " is sending data!" << endl;
						wait();

						while (true) {
							Message m = actor_port->read(actor_addr);
							sc_uint<32> data_package_id = sc_uint<32>(m.range(mes_package_id));
							if (data_package_id != 0) {
								of << dec << "data[" << left << setw(10) << setfill(' ') << (data_package_id - 1) * Message_data_num << ": " <<
									left << setw(10) << setfill(' ') << data_package_id * Message_data_num - 1 << "]: ";
								for (int j = 0; j < Message_data_num; j++) {
									float float_data = sc_uint32_cov_float(sc_uint<32>(m >> (j * 32)));
									of << left << setw(10) << setfill(' ') << dec << float_data;
								}
								of << endl;
								wait();
								if (data_package_id == in_data_package_size) {
									break;
								}
							}
						}
						of << "Time " << dec << sc_time_stamp() << ": producer" << hex << actor_addr << " finish sending data!" << endl;
						of.close();
					}
				}

				else if (wait_cycle == -1)
					while (true) wait();
				
			}
		}
		inf.close();
	}
	
}




