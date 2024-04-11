#pragma once
#include<fstream>
#include<iostream>
#include<string>

#include"systemc.h"
#include"Instruction.h"
#include"Message.h"
#include"actor_interface.h"

using namespace std; 

class host: public sc_module {
public:
	sc_in_clk clk;
	sc_in<bool> rst;
	actor_interface* actor_if;

	sc_port<sc_signal<Message>, interface_size> message_in;
	sc_port<sc_signal<Message>, interface_size> message_out;
	sc_port<sc_signal<bool>, max_actor_num> actor_busy;

	sc_uint<32> host_addr;
	void host_main();

	SC_HAS_PROCESS(host);

	host(sc_module_name name_, sc_uint<32> host_addr, sc_uint<32>* adj_actor_addr, int adj_actor_num) :
		sc_module(name_), host_addr(host_addr) {
		actor_if = new actor_interface("actor_if", adj_actor_addr, adj_actor_num);
		actor_if->actor_port_in(message_in);
		actor_if->actor_port_out(message_out);

		SC_CTHREAD(host_main, clk.pos());
		reset_signal_is(rst, true);
	}
};

void host::host_main() {
	int host_id = int(host_addr - sc_uint<32>(0x80000000));

	string inf_name = "host_" + to_string(host_id) + ".txt";

	ifstream inf(inf_name);
	if (!inf.is_open()) {
		cout << "The file " << inf_name << " is unable to open!\n";
	}

	else {
		string s;
		inf >> s >> s >> s >> s;
		int wait_cycle;
		int actor_id;

		sc_uint<8> Message_head;
		sc_uint<8> Message_type;
		sc_uint<32> Message_source;
		sc_uint<32> Message_end;
		sc_uint<32> Message_op;
		sc_uint<8> Message_seq;
		sc_uint<8> Message_hail;

		sc_uint<8> ins_head;
		sc_uint<8> ins_type;
		sc_uint<32> ins_op1;
		sc_uint<32> ins_op2;
		sc_uint<32> ins_op3;
		sc_uint<32> ins_op4;
		sc_uint<32> ins_op5;
		sc_uint<32> ins_op6;
		sc_uint<8> ins_para1;
		sc_uint<8> ins_para2;

		sc_uint<32> data_package_traffic = 0;

		actor_if->write_all(Message(0));
		wait();

		while (true) {
			actor_if->write_all(Message(0));
			inf >> s;
			if (s == "//")
				getline(inf, s);
			else
			{
				wait_cycle = stoi(s);
				if (wait_cycle == -1)
					while (true) wait();

				for (int i = 0; i < wait_cycle; i++) wait();
				inf >> actor_id;
				inf >> hex >> Message_head >> Message_type >> Message_source >>
					Message_end >> Message_op >> Message_seq >> Message_hail;

				Message m = Message((Message_head, Message_type, Message_source,
					Message_end, Message_op, Message_seq, Message_hail)) << (Message_Width - 128);


				if (Message_type == message_ack) {
					actor_if->write(Message_end, m);
					cout << "host " << hex << host_addr << ": consumer " << hex << Message_source << " ack!" << endl;
					wait();
				}
				else {
					actor_if->write(Message_end, m);
					wait();

					if (Message_type == message_ins_init) {
						inf >> s;

						for (unsigned int i = 0; i < Message_op; i++) {
							inf >> hex >> ins_head >> ins_type >> ins_op1 >> ins_op2 >> ins_op3 >> ins_op4
								>> ins_op5 >> ins_op6 >> ins_para1 >> ins_para2;

							Message m = (Message(sc_uint<32>(i+1))<<(Message_Width-32)) + 
										Message((ins_head, ins_type, ins_op1, ins_op2, ins_op3, ins_op4,ins_op5, ins_op6, ins_para1, ins_para2));

							actor_if->write(Message_end, m);
							wait();
						}
					}


					else if (Message_type == message_req) {
						actor_if->write(Message_end, Message(0));
						int wait_cycle = 0;
						bool ack = true;
						while ((actor_if->read(Message_end).range(mes_head_bits) != message_head) ||
							(actor_if->read(Message_end).range(mes_type_bits) != message_ack)) {
							wait_cycle++;
							if (wait_cycle >= 5) {
								ack = false;
								break;
							}
							wait();
						}
						if (ack) {
							data_package_traffic = actor_if->read(Message_end).range(mes_op_bits);
							cout << "host " << host_addr << ": ack! source:" << hex << Message_end << 
								" traffic:" << dec << data_package_traffic << " seq:" << hex << Message_seq << endl;
						}
					}

					else if (Message_type == message_trans_data) {
						inf >> s;
						sc_int<32> data[Message_data_num];
						for (unsigned int i = 0; i < Message_op; i++) {
							Message m = Message(0);
							for (int j = 0; j < Message_data_num; j++) {
								inf >> dec >> data[j];
							}
							for (int j = Message_data_num - 1; j >= 0; j--) {
								m = Message((m, data[j]));
							}
							m += (Message(sc_uint<32>(i + 1)) << (Message_Width - 32));
							actor_if->write(Message_end, m);
							for (int i = 0; i < data_package_traffic; i++) wait();
						}
						
					}
				}

			}
		}
	}
}

