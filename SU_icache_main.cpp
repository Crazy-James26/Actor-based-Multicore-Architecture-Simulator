/*#include"SU.h"

void SU_icache::SU_icache_main() {

	State = SU_reset;
	Instruction config_ins = Instruction(0);

	icache_control.write(sc_uint<8>(0));
	icache_control_valid.write(false);
	icache_op.write(sc_uint <32>(0));
	icache_input_ins.write(Instruction(0));

	current_ins.write(Instruction(0));
	current_ins_valid.write(false);

	while (true) {
		Message mes = in_message.read();
		sc_uint<8> mes_head = sc_uint<8>(mes.range(mes_head_bits));

		if (mes_head == message_head) {
			sc_uint<8> mes_type = sc_uint<8>(mes.range(mes_type_bits));
			sc_uint<32> mes_op = sc_uint<32>(mes.range(mes_op_bits));
			cout << State << " " << hex << mes_type << endl;

			if (mes_type == message_ins_init) {
				if ((State == SU_reset) || (State == SU_pause)) {
					State = SU_ins_init;
					icache_control_valid.write(true);
					icache_control.write(mes_type);
					icache_op.write(mes_op);
					wait();
					icache_control_valid.write(false);
					for (unsigned int i = 1; i <= mes_op; i++) {
						Instruction input_ins = Instruction(in_message.read().range(mes_ins_package_bits));
						icache_input_ins.write(input_ins);
						cout << hex << input_ins << endl;
						wait();
					}
					while (icache_State.read() != ins_init_done)
						wait();
					State = SU_ins_init_done;

					in_message.write(Message(0xffff));
				}
				else {
					cout << "The icache initialization is invalid, because the SU has not been reset or paused!" << endl;
				}
			}

			else if ((mes_type == message_config) || (mes_type == message_jumpconf)) {
				if ((State == SU_ins_init_done) || (State == SU_pause)) {
					State = SU_ins_config;
					icache_control_valid.write(true);
					icache_control.write(mes_type);
					icache_op.write(mes_op);
					wait();
					icache_control_valid.write(false);
					while (icache_State.read() != ins_config_done)
						wait();
					current_ins.write(icache_ouput_ins.read());
					cout << "current_ins: " << icache_ouput_ins.read() << endl;
					State = SU_ins_config_done;
				}
				else {
					cout << "The configuration is invalid, because the icache has not been initialized or the SU has not been paused!" << endl;
				}

			}

			else if (mes_type == message_start) {
				if ((State == SU_ins_config_done) || (State == SU_pause)) {
					State = SU_start;
					current_ins_valid.write(true);
					cout << "current_ins gets started!" << endl;
				}
				else
					cout << "The SU start is invalid, because SU has not finished the configuration or been paused" << endl;
			}

			else if (mes_type == message_pause) {
				if (State == SU_start) {
					State = SU_pause;
					current_ins_valid.write(false);
					cout << "current_ins is paused!" << endl;
				}
				else
					cout << "The SU pause is invalid, because SU has not got started!" << endl;
			}
		}
		wait();
	}
}*/
