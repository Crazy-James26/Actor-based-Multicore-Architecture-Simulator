#pragma once
#include<iostream>
#include"systemc.h"
#include"Instruction.h"
#include"Message.h"
#include"memory.h"

using namespace std;

SC_MODULE(ALU) {
	sc_in_clk clk;
	sc_in<bool> rst;

	sc_out<bool> ALU_busy;
	sc_in<bool> ALU_start;
	sc_in<sc_uint<8>> ALU_type;
	sc_in<sc_uint<32>> ALU_in_op_num;
	sc_in<sc_uint<32>> ALU_in_mem_addr[Max_op_num];
	sc_in<sc_uint<32>> ALU_out_mem_addr;
	sc_in<sc_biguint<ALU_para_Width>> ALU_para;
	sc_out<bool> ALU_finish;

	sc_uint<16> para1;
	sc_uint<16> para2;
	sc_uint<16> para3;
	sc_uint<16> para4;
	sc_uint<16> para5;
	sc_uint<16> para6;
	sc_uint<16> para7;
	sc_uint<16> para8;
	sc_uint<8> para9;

	sc_out<bool> ALU_mem_read[Max_op_num];
	sc_out<sc_uint<32>> ALU_mem_read_addr[Max_op_num];
	sc_in<float> ALU_mem_read_data[Max_op_num][ALU_read_write_num];
	sc_out<bool> ALU_mem_write;
	sc_out<sc_uint<32>> ALU_mem_write_addr;
	sc_out<float> ALU_mem_write_data[ALU_read_write_num];

	void ALU_add(sc_uint<32>* in_addr, sc_uint<32> in_op_num, sc_uint<32> out_addr, sc_uint<16> B, sc_uint<16> C, sc_uint<16> H, sc_uint<16> W);
	void ALU_loss(sc_uint<32> in_addr1, sc_uint<32> in_ddr2, sc_uint<32> out_addr, sc_uint<16> B, sc_uint<16> C, sc_uint<16> H, sc_uint<16> W);
	void ALU_mul(sc_uint<32> in_addr1, sc_uint<32> in_addr2, sc_uint<32> out_addr, sc_uint<16> B, sc_uint<16> M, sc_uint<16> N);
	void ALU_conv(sc_uint<32> in_addr1, sc_uint<32> in_addr2, sc_uint<32> out_addr, sc_uint<16> B, sc_uint<16> C_in, sc_uint<16> HW_in,
					sc_uint<16> k, sc_uint<16> C_out, sc_uint<16> Stride, sc_uint<16> Padding, sc_uint<8> mode);
	void ALU_pool(sc_uint<32> in_addr1, sc_uint<32> in_addr2, sc_uint<32> out_addr, sc_uint<16> B, sc_uint<16> C, sc_uint<16> HW,
					sc_uint<16> k, sc_uint<16> Stride, sc_uint<16> Padding, sc_uint<8> mode);
	void ALU_act(sc_uint<32> in_addr1, sc_uint<32> in_addr2, sc_uint<32> out_addr, sc_uint<16> B, sc_uint<16> C, sc_uint<16> H, sc_uint<16> W, sc_uint<8> mode);
	void ALU_fc(sc_uint<32> in_addr1, sc_uint<32> in_addr2, sc_uint<32> out_addr, sc_uint<16> B, sc_uint<16> M, sc_uint<16> N, sc_uint<8> mode);

	void ALU_control();

	SC_CTOR(ALU) {
		SC_CTHREAD(ALU_control, clk.pos());
		reset_signal_is(rst, true);
	}
};

void ALU::ALU_control() {
	ALU_busy.write(false);
	ALU_finish.write(false);
	
	para1 = sc_uint<16>(0);
	para2 = sc_uint<16>(0);
	para3 = sc_uint<16>(0);
	para4 = sc_uint<16>(0);
	para5 = sc_uint<16>(0);
	para6 = sc_uint<16>(0);
	para7 = sc_uint<16>(0);
	para8 = sc_uint<16>(0);
	para9 = sc_uint<8>(0);

	for (int i = 0; i < Max_op_num; i++) {
		ALU_mem_read[i].write(false);
		ALU_mem_read_addr[i].write(sc_uint <32>(0));
	}
	ALU_mem_write.write(false);
	ALU_mem_write_addr.write(sc_uint <32>(0));
	for(int i=0;i<8;i++)
		ALU_mem_write_data[i].write(float(0));
	wait();

	while (true) {
		if (ALU_start.read() == true) {
			ALU_busy.write(true);
			//cout << hex << ALU_para.read() << endl;
			para1 = sc_uint<16>(ALU_para.read().range(para1_bits));
			para2 = sc_uint<16>(ALU_para.read().range(para2_bits));
			para3 = sc_uint<16>(ALU_para.read().range(para3_bits));
			para4 = sc_uint<16>(ALU_para.read().range(para4_bits));
			para5 = sc_uint<16>(ALU_para.read().range(para5_bits));
			para6 = sc_uint<16>(ALU_para.read().range(para6_bits));
			para7 = sc_uint<16>(ALU_para.read().range(para7_bits));
			para8 = sc_uint<16>(ALU_para.read().range(para8_bits));
			para9 = sc_uint<8>(ALU_para.read().range(para9_bits));

			if (ALU_type.read() == ADD) {
				sc_uint<32> in_op_num = ALU_in_op_num.read();
				sc_uint<32>* in_addr = new sc_uint<32>[in_op_num];
				for (unsigned int i = 0; i < in_op_num; i++) {
					in_addr[i] = ALU_in_mem_addr[i];
				}
				ALU_add(in_addr, in_op_num, ALU_out_mem_addr, para1, para2, para3, para4);
			}
			else if (ALU_type.read() == LOSS)
				ALU_loss(ALU_in_mem_addr[0], ALU_in_mem_addr[1], ALU_out_mem_addr, para1, para2, para3, para4);
			else if (ALU_type.read() == MUL)
				ALU_mul(ALU_in_mem_addr[0], ALU_in_mem_addr[1], ALU_out_mem_addr, para1, para2, para3);
			else if (ALU_type.read() == CONV)
				ALU_conv(ALU_in_mem_addr[0], ALU_in_mem_addr[1], ALU_out_mem_addr, para1, para2, para3, para4, para5, para6, para7, para9);
			else if (ALU_type.read() == POOL)
				ALU_pool(ALU_in_mem_addr[0], ALU_in_mem_addr[1], ALU_out_mem_addr, para1, para2, para3, para4, para6, para7, para9);
			else if (ALU_type.read() == ACT)
				ALU_act(ALU_in_mem_addr[0], ALU_in_mem_addr[1], ALU_out_mem_addr, para1, para2, para3, para4, para9);
			else if (ALU_type.read() == FC)
				ALU_fc(ALU_in_mem_addr[0], ALU_in_mem_addr[1], ALU_out_mem_addr, para1, para2, para3, para9);
			//cout << "ALU: computation is completed!" << hex << endl;
			ALU_busy.write(false);
			ALU_finish.write(true);
			wait();
			ALU_finish.write(false);
		}
		else {
			wait();
		}
	}
}

void ALU::ALU_add(sc_uint<32>* in_addr, sc_uint<32> in_op_num, sc_uint<32> out_addr, sc_uint<16> B, sc_uint<16> C, sc_uint<16> H, sc_uint<16> W) {
	//cout << "ALU: ADD begin!" << endl;
	vector<vector<float>> X(in_op_num, vector <float>(B * C * H * W, 0));
	vector<float> Y(B * C * H * W, 0);

	for (unsigned int i = 0; i < B * C * H * W; i+= ALU_read_write_num) {
		for (unsigned int k = 0; k < in_op_num; k++) {
			ALU_mem_read[k].write(true);
			ALU_mem_read_addr[k].write(sc_uint <32>(in_addr[k] + i));
		}
		wait();
		for (unsigned int j = 0; j < ALU_read_write_num; j++) {
			if (i + j < B * C * H * W) {
				for (unsigned int k = 0; k < in_op_num; k++) {
					X[k][i + j] = ALU_mem_read_data[k][j].read();
				}
			}
		}
	}
	for (unsigned int k = 0; k < in_op_num; k++) {
		ALU_mem_read[k].write(false);
		ALU_mem_read_addr[k].write(sc_uint <32>(0));
	}

	for (unsigned int k = 0; k < in_op_num; k++) {
		for (unsigned int i = 0; i < B * C * H * W; i++) {
			Y[i] += X[k][i];
		}
	}
	//unsigned int wait_cycle = B * C * H * W;
	//for (unsigned int i = 0; i < wait_cycle; i++) wait();

	for (unsigned int i = 0; i < B * C * H * W; i += ALU_read_write_num) {
		ALU_mem_write.write(true);
		ALU_mem_write_addr.write(sc_uint <32>(out_addr + i));
		for (unsigned int j = 0; j < ALU_read_write_num; j++) {
			if (i + j < B * C * H * W) {
				ALU_mem_write_data[j].write(Y[i + j]);
			}
			else {
				ALU_mem_write_data[j].write(float(0));
			}
		}
		wait();
	}
	ALU_mem_write.write(false);
	ALU_mem_write_addr.write(sc_uint <32>(0));
	for (int i = 0; i < ALU_read_write_num; i++)
		ALU_mem_write_data[i].write(float(0));
}

void ALU::ALU_loss(sc_uint<32> in_addr1, sc_uint<32> in_addr2, sc_uint<32> out_addr, sc_uint<16> B, sc_uint<16> C, sc_uint<16> H, sc_uint<16> W) {
	//cout << "ALU: SUB begin!" << endl;
	vector<float> X(B * C * H * W, 0);
	vector<float> Z(B * C * H * W, 0);
	vector<float> Y(B * C * H * W, 0);

	for (unsigned int i = 0; i < B * C * H * W; i += ALU_read_write_num) {
		ALU_mem_read[0].write(true);
		ALU_mem_read[1].write(true);
		ALU_mem_read_addr[0].write(sc_uint <32>(in_addr1 + i));
		ALU_mem_read_addr[1].write(sc_uint <32>(in_addr2 + i));
		wait();
		for (unsigned int j = 0; j < ALU_read_write_num; j++) {
			if (i + j < B * C * H * W) {
				X[i + j] = ALU_mem_read_data[0][j].read();
				Z[i + j] = ALU_mem_read_data[1][j].read();
			}
		}
	}
	ALU_mem_read[0].write(false);
	ALU_mem_read_addr[0].write(sc_uint <32>(0));
	ALU_mem_read[1].write(false);
	ALU_mem_read_addr[1].write(sc_uint <32>(0));

	for (unsigned int i = 0; i < B * C * H * W; i += C * H * W) {
		float sum = 0;
		for (unsigned int j = 0; j < C * H * W; j++) {
			X[i + j] = exp(X[i + j]);
			sum += X[i + j];
		}
		for (unsigned int j = 0; j < C * H * W; j++) {
			X[i + j] /= sum;
			Y[i + j] = X[i + j] - Z[i + j];
		}
	}
	//unsigned int wait_cycle = B * C * H * W;
	//for (unsigned int i = 0; i < wait_cycle; i++) wait();

	for (unsigned int i = 0; i < B * C * H * W; i += ALU_read_write_num) {
		ALU_mem_write.write(true);
		ALU_mem_write_addr.write(sc_uint <32>(out_addr + i));
		for (unsigned int j = 0; j < ALU_read_write_num; j++) {
			if (i + j < B * C * H * W) {
				ALU_mem_write_data[j].write(Y[i + j]);
			}
			else {
				ALU_mem_write_data[j].write(float(0));
			}
		}
		wait();
	}
	ALU_mem_write.write(false);
	ALU_mem_write_addr.write(sc_uint <32>(0));
	for (int i = 0; i < ALU_read_write_num; i++)
		ALU_mem_write_data[i].write(float(0));
}

void ALU::ALU_mul(sc_uint<32> in_addr1, sc_uint<32> in_addr2, sc_uint<32> out_addr, sc_uint<16> B, sc_uint<16> M, sc_uint<16> N) {
	//cout << "ALU: MUL begin!" << endl;
	vector<vector<float>> X(B, vector<float>(M, 0));
	vector<vector<float>> Z(M, vector<float>(N, 0));
	vector<vector<float>> Y(B, vector<float>(N, 0));

	for (unsigned int i = 0; (i < B * M) || (i < M * N); i += ALU_read_write_num) {
		if (i < B * M) {
			ALU_mem_read[0].write(true);
			ALU_mem_read_addr[0].write(sc_uint <32>(in_addr1 + i));
		}
		else 
			ALU_mem_read[0].write(false);
		if (i < M * N) {
			ALU_mem_read[1].write(true);
			ALU_mem_read_addr[1].write(sc_uint <32>(in_addr2 + i));
		}
		else
			ALU_mem_read[1].write(false);
		wait();
		for (unsigned int j = 0; j < ALU_read_write_num; j++) {
			if (i + j < B * M) {
				X[(i + j) / M][(i + j) % M] = ALU_mem_read_data[0][j].read();
			}
			if (i + j < M * N) {
				Z[(i + j) / N][(i + j) % N] = ALU_mem_read_data[1][j].read();
			}
		}
	}
	ALU_mem_read[0].write(false);
	ALU_mem_read_addr[0].write(sc_uint <32>(0));
	ALU_mem_read[1].write(false);
	ALU_mem_read_addr[1].write(sc_uint <32>(0));

		for (unsigned int b = 0; b < B; b++) {
			for (unsigned int n = 0; n < N; n++) {
				for (unsigned int m = 0; m < M; m++) {
					Y[b][n] += X[b][m] * Z[m][n];
				}
			}
		}

	unsigned int wait_cycle = B * M * N;
	for (unsigned int i = 0; i < wait_cycle; i++) wait();

	for (unsigned int i = 0; i < B * N; i += ALU_read_write_num) {
		ALU_mem_write.write(true);
		ALU_mem_write_addr.write(sc_uint <32>(out_addr + i));
		for (unsigned int j = 0; j < ALU_read_write_num; j++) {
			if (i + j < B * N) {
				ALU_mem_write_data[j].write(Y[(i + j) / N][(i + j) % N]);
			}
			else {
				ALU_mem_write_data[j].write(float(0));
			}
		}
		wait();
	}
	ALU_mem_write.write(false);
	ALU_mem_write_addr.write(sc_uint <32>(0));
	for (int i = 0; i < ALU_read_write_num; i++)
		ALU_mem_write_data[i].write(float(0));
}

void ALU::ALU_conv(sc_uint<32> in_addr1, sc_uint<32> in_addr2, sc_uint<32> out_addr, sc_uint<16> B, sc_uint<16> C_in, sc_uint<16> HW_in,
	sc_uint<16> k, sc_uint<16> C_out, sc_uint<16> Stride, sc_uint<16> Padding, sc_uint<8> mode) {
	//cout << "ALU: CONV begin!" << endl;
	if (mode == CONV_fp) {
		unsigned int HW_in_pad = HW_in + 2 * Padding;
		unsigned int HW_out = (HW_in_pad - k) / Stride + 1;
		vector<vector<vector<vector<float>>>> X(B, vector<vector<vector<float>>>(C_in, vector<vector<float> >(HW_in_pad, vector<float>(HW_in_pad, 0))));
		vector<vector<vector<vector<float>>>> Z(C_out, vector<vector<vector<float>>>(C_in, vector<vector<float>>(k, vector<float>(k, 0))));
		vector<vector<vector<vector<float>>>> Y(B, vector<vector<vector<float>>>(C_out, vector<vector<float> >(HW_out, vector<float>(HW_out, 0))));

		for (unsigned int i = 0; (i < B * C_in * HW_in * HW_in) || (i < C_out * C_in * k * k); i += ALU_read_write_num) {
			if (i < B * C_in * HW_in * HW_in) {
				ALU_mem_read[0].write(true);
				ALU_mem_read_addr[0].write(sc_uint <32>(in_addr1 + i));
			}
			else
				ALU_mem_read[0].write(false);
			if (i < C_out * C_in * k * k) {
				ALU_mem_read[1].write(true);
				ALU_mem_read_addr[1].write(sc_uint <32>(in_addr2 + i));
			}
			else
				ALU_mem_read[1].write(false);
			wait();
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C_in * HW_in * HW_in) {
					X[(i + j) / (C_in * HW_in * HW_in)][((i + j) / (HW_in * HW_in)) % C_in][((i + j) / HW_in) % HW_in + Padding][(i + j) % HW_in + Padding] = ALU_mem_read_data[0][j].read();
				}
				if (i + j < C_out * C_in * k * k) {
					Z[(i + j) / (C_in * k * k)][((i + j) / (k * k)) % C_in][((i + j) / k) % k][(i + j) % k] = ALU_mem_read_data[1][j].read();
				}
			}
		}

		ALU_mem_read[0].write(false);
		ALU_mem_read_addr[0].write(sc_uint <32>(0));
		ALU_mem_read[1].write(false);
		ALU_mem_read_addr[1].write(sc_uint <32>(0));

		for (unsigned int b = 0; b < B; b++) {
			for (unsigned int c_out = 0; c_out < C_out; c_out++) {
				for (unsigned int c_in = 0; c_in < C_in; c_in++) {
					for (unsigned int h = 0; h + k <= HW_in_pad; h += Stride) {
						for (unsigned int w = 0; w + k <= HW_in_pad; w += Stride) {
							for (unsigned int i = 0; i < k; i++) {
								for (unsigned int j = 0; j < k; j++) {
									Y[b][c_out][h / Stride][w / Stride] += X[b][c_in][h + i][w + j] * Z[c_out][c_in][i][j];
								}
							}
						}
					}
				}
			}
		}

		unsigned int wait_cycle = B * C_in * HW_out * HW_out * k * k * C_out / ALU_calulate_parallelism;
		for (unsigned int i = 0; i < wait_cycle; i++) wait();

		for (unsigned int i = 0; i < B * C_out * HW_out * HW_out; i += ALU_read_write_num) {
			ALU_mem_write.write(true);
			ALU_mem_write_addr.write(sc_uint <32>(out_addr + i));
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C_out * HW_out * HW_out) {
					ALU_mem_write_data[j].write(Y[(i + j) / (C_out * HW_out * HW_out)][((i + j) / (HW_out * HW_out)) % C_out][((i + j) / HW_out) % HW_out][(i + j) % HW_out]);
				}
			}
			wait();
		}
		ALU_mem_write.write(false);
		ALU_mem_write_addr.write(sc_uint <32>(0));
		for (int i = 0; i < ALU_read_write_num; i++)
			ALU_mem_write_data[i].write(float(0));
	}

	else if (mode == CONV_bp) {
		unsigned int HW_in_pad = HW_in + 2 * Padding;
		unsigned int HW_out = (HW_in_pad - k) / Stride + 1;
		unsigned int out_Stride = 1;
		unsigned int out_Padding = k - Padding - 1;
		unsigned int HW_out_pad = HW_in + k - 1;
		vector<vector<vector<vector<float>>>> X(B, vector<vector<vector<float>>>(C_in, vector<vector<float> >(HW_in, vector<float>(HW_in, 0))));
		vector<vector<vector<vector<float>>>> Z(C_out, vector<vector<vector<float>>>(C_in, vector<vector<float>>(k, vector<float>(k, 0))));
		vector<vector<vector<vector<float>>>> Y(B, vector<vector<vector<float>>>(C_out, vector<vector<float> >(HW_out_pad, vector<float>(HW_out_pad, 0))));

		for (unsigned int i = 0; (i < B * C_out * HW_out * HW_out) || (i < C_out * C_in * k * k); i += ALU_read_write_num) {
			if (i < B * C_out * HW_out * HW_out) {
				ALU_mem_read[0].write(true);
				ALU_mem_read_addr[0].write(sc_uint <32>(in_addr1 + i));
			}
			else
				ALU_mem_read[0].write(false);
			if (i < C_out * C_in * k * k) {
				ALU_mem_read[1].write(true);
				ALU_mem_read_addr[1].write(sc_uint <32>(in_addr2 + i));
			}
			else
				ALU_mem_read[1].write(false);
			wait();
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C_out * HW_out * HW_out) {
					Y[(i + j) / (C_out * HW_out * HW_out)][((i + j) / (HW_out * HW_out)) % C_out]
						[(((i + j) / HW_out) % HW_out) * Stride + out_Padding][((i + j) % HW_out) * Stride + out_Padding] = ALU_mem_read_data[0][j].read();
				}
				if (i + j < C_out * C_in * k * k) {
					Z[(i + j) / (C_in * k * k)][((i + j) / (k * k)) % C_in][((i + j) / k) % k][(i + j) % k] = ALU_mem_read_data[1][j].read();
				}
			}
		}

		ALU_mem_read[0].write(false);
		ALU_mem_read_addr[0].write(sc_uint <32>(0));
		ALU_mem_read[1].write(false);
		ALU_mem_read_addr[1].write(sc_uint <32>(0));

		for (unsigned int b = 0; b < B; b++) {
			for (unsigned int c_in = 0; c_in < C_in; c_in++) {
				for (unsigned int c_out = 0; c_out < C_out; c_out++) {
					for (unsigned int h = 0; h + k <= HW_out_pad; h += out_Stride) {
						for (unsigned int w = 0; w + k <= HW_out_pad; w += out_Stride) {
							for (unsigned int i = 0; i < k; i++) {
								for (unsigned int j = 0; j < k; j++) {
									X[b][c_in][h / out_Stride][w / out_Stride] += Y[b][c_out][h + i][w + j] * Z[c_out][c_in][k - 1 - i][k - 1 - j];
								}
							}
						}
					}
				}
			}
		}

		unsigned int wait_cycle = B * C_in * HW_out * HW_out * k * k * C_out / ALU_calulate_parallelism;
		for (unsigned int i = 0; i < wait_cycle; i++) wait();

		for (unsigned int i = 0; i < B * C_in * HW_in * HW_in; i += ALU_read_write_num) {
			ALU_mem_write.write(true);
			ALU_mem_write_addr.write(sc_uint <32>(out_addr + i));
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C_in * HW_in * HW_in) {
					ALU_mem_write_data[j].write(X[(i + j) / (C_in * HW_in * HW_in)][((i + j) / (HW_in * HW_in)) % C_in][((i + j) / HW_in) % HW_in][(i + j) % HW_in]);
				}
			}
			wait();
		}
		ALU_mem_write.write(false);
		ALU_mem_write_addr.write(sc_uint <32>(0));
		for (int i = 0; i < ALU_read_write_num; i++)
			ALU_mem_write_data[i].write(float(0));
	}

	else if (mode == CONV_divw) {
		unsigned int HW_in_pad = HW_in + 2 * Padding;
		unsigned int HW_out = (HW_in_pad - k) / Stride + 1;
		unsigned int out_Stride = 1;
		unsigned int HW_out_pad = HW_in_pad - k + 1 ;
		vector<vector<vector<vector<float>>>> X(B, vector<vector<vector<float>>>(C_in, vector<vector<float> >(HW_in_pad, vector<float>(HW_in_pad, 0))));
		vector<vector<vector<vector<float>>>> Z(C_out, vector<vector<vector<float>>>(C_in, vector<vector<float>>(k, vector<float>(k, 0))));
		vector<vector<vector<vector<float>>>> Y(B, vector<vector<vector<float>>>(C_out, vector<vector<float> >(HW_out_pad, vector<float>(HW_out_pad, 0))));

		for (unsigned int i = 0; (i < B * C_in * HW_in * HW_in) || (i < B * C_out * HW_out * HW_out); i += ALU_read_write_num) {
			if (i < B * C_in * HW_in * HW_in) {
				ALU_mem_read[0].write(true);
				ALU_mem_read_addr[0].write(sc_uint <32>(in_addr1 + i));
			}
			else
				ALU_mem_read[0].write(false);
			if (i < B * C_out * HW_out * HW_out) {
				ALU_mem_read[1].write(true);
				ALU_mem_read_addr[1].write(sc_uint <32>(in_addr2 + i));
			}
			else
				ALU_mem_read[1].write(false);
			wait();
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C_in * HW_in * HW_in) {
					X[(i + j) / (C_in * HW_in * HW_in)][((i + j) / (HW_in * HW_in)) % C_in]
						[((i + j) / HW_in) % HW_in + Padding][(i + j) % HW_in + Padding] = ALU_mem_read_data[0][j].read();
				}
				if (i + j < B * C_out * HW_out * HW_out) {
					Y[(i + j) / (C_out * HW_out * HW_out)][((i + j) / (HW_out * HW_out)) % C_out]
						[(((i + j) / HW_out) % HW_out)* Stride][((i + j) % HW_out) * Stride] = ALU_mem_read_data[1][j].read();
				}
			}
		}

		ALU_mem_read[0].write(false);
		ALU_mem_read_addr[0].write(sc_uint <32>(0));
		ALU_mem_read[1].write(false);
		ALU_mem_read_addr[1].write(sc_uint <32>(0));

		for (unsigned int b = 0; b < B; b++) {
			for (unsigned int c_in = 0; c_in < C_in; c_in++) {
				for (unsigned int c_out = 0; c_out < C_out; c_out++) {
					for (unsigned int h = 0; h + HW_out_pad <= HW_in_pad; h += out_Stride) {
						for (unsigned int w = 0; w + HW_out_pad <= HW_in_pad; w += out_Stride) {
							for (unsigned int i = 0; i < HW_out_pad; i++) {
								for (unsigned int j = 0; j < HW_out_pad; j++) {
									Z[c_out][c_in][h / out_Stride][w / out_Stride] += X[b][c_in][h + i][w + j] * Y[b][c_out][i][j];
								}
							}
						}
					}
				}
			}
		}

		

		unsigned int wait_cycle = B * C_in * HW_out * HW_out * k * k * C_out / ALU_calulate_parallelism;
		for (unsigned int i = 0; i < wait_cycle; i++) wait();

		for (unsigned int i = 0; i < C_out * C_in * k * k; i += ALU_read_write_num) {
			ALU_mem_write.write(true);
			ALU_mem_write_addr.write(sc_uint <32>(out_addr + i));
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < C_out * C_in * k * k) {
					ALU_mem_write_data[j].write(Z[(i + j) / (C_in * k * k)][((i + j) / (k * k)) % C_in][((i + j) / k) % k][(i + j) % k]);
				}
			}
			wait();
		}
		ALU_mem_write.write(false);
		ALU_mem_write_addr.write(sc_uint <32>(0));
		for (int i = 0; i < ALU_read_write_num; i++)
			ALU_mem_write_data[i].write(float(0));
	}
}

void ALU::ALU_pool(sc_uint<32> in_addr1, sc_uint<32> in_addr2, sc_uint<32> out_addr, sc_uint<16> B, sc_uint<16> C, sc_uint<16> HW,
	sc_uint<16> k, sc_uint<16> Stride, sc_uint<16> Padding, sc_uint<8> mode) {
	//cout << "ALU: POOL begin!" << endl;
	if (mode == POOL_max_fp || mode == POOL_ave_fp) {
		unsigned int HW_in_pad = HW + 2 * Padding;
		unsigned int HW_out = (HW + 2 * Padding - k) / Stride + 1;
		vector<vector<vector<vector<float>>>> X(B, vector<vector<vector<float>>>(C, vector<vector<float>>(HW_in_pad, vector<float>(HW_in_pad, 0))));
		vector<vector<vector<vector<float>>>> Y(B, vector<vector<vector<float>>>(C, vector<vector<float>>(HW_out, vector<float>(HW_out, 0))));

		for (unsigned int i = 0; i < B * C * HW * HW; i += ALU_read_write_num) {
			ALU_mem_read[0].write(true);
			ALU_mem_read_addr[0].write(sc_uint <32>(in_addr1 + i));
			wait();
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C * HW * HW) {
					X[(i + j) / (C * HW * HW)][((i + j) / (HW * HW)) % C][((i + j) / HW) % HW + Padding][(i + j) % HW + Padding] = ALU_mem_read_data[0][j].read();
				}
			}
		}
		ALU_mem_read[0].write(false);
		ALU_mem_read_addr[0].write(sc_uint <32>(0));

		for (unsigned int b = 0; b < B; b++) {
			for (unsigned int c = 0; c < C; c++) {
				for (unsigned int h = 0; h + k <= HW_in_pad; h += Stride) {
					for (unsigned int w = 0; w + k <= HW_in_pad; w += Stride) {
						float pool_out = 0;
						for (unsigned int i = 0; i < k; i++) {
							for (unsigned int j = 0; j < k; j++) {
								if (mode == POOL_max_fp)
									pool_out = X[b][c][h + i][w + j] > pool_out ? X[b][c][h + i][w + j] : pool_out;
								else if (mode == POOL_ave_fp)
									pool_out += X[b][c][h + i][w + j];
							}
						}
						if (mode == POOL_ave_fp)
							pool_out /= (k * k);
						Y[b][c][h / Stride][w / Stride] = pool_out;
					}
				}
			}
		}
		unsigned int wait_cycle = B * C * HW_out * HW_out * k * k / ALU_calulate_parallelism;
		for (unsigned int i = 0; i < wait_cycle; i++) wait();

		for (unsigned int i = 0; i < B * C * HW_out * HW_out; i += ALU_read_write_num) {
			ALU_mem_write.write(true);
			ALU_mem_write_addr.write(sc_uint <32>(out_addr + i));
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C * HW_out * HW_out) {
					ALU_mem_write_data[j].write(Y[(i + j) / (C * HW_out * HW_out)][((i + j) / (HW_out * HW_out)) % C][((i + j) / HW_out) % HW_out][(i + j) % HW_out]);
				}
			}
			wait();
		}
		ALU_mem_write.write(false);
		ALU_mem_write_addr.write(sc_uint <32>(0));
		for (int i = 0; i < ALU_read_write_num; i++)
			ALU_mem_write_data[i].write(float(0));
	}
	else if(mode == POOL_max_bp){
		unsigned int HW_in_pad = HW + 2 * Padding;
		unsigned int HW_out = (HW + 2 * Padding - k) / Stride + 1;

		vector<vector<vector<vector<float>>>> X(B, vector<vector<vector<float>>>(C, vector<vector<float>>(HW_in_pad, vector<float>(HW_in_pad, 0))));
		vector<vector<vector<vector<float>>>> Z(B, vector<vector<vector<float>>>(C, vector<vector<float>>(HW_in_pad, vector<float>(HW_in_pad, 0))));
		vector<vector<vector<vector<float>>>> Y(B, vector<vector<vector<float>>>(C, vector<vector<float>>(HW_out, vector<float>(HW_out, 0))));

		for (unsigned int i = 0; i < B * C * HW * HW || i < B * C * HW_out * HW_out; i += ALU_read_write_num) {
			if (i < B * C * HW_out * HW_out) {
				ALU_mem_read[0].write(true);
				ALU_mem_read_addr[0].write(sc_uint <32>(in_addr1 + i));
			}
			if (i < B * C * HW_out * HW_out) {
				ALU_mem_read[1].write(true);
				ALU_mem_read_addr[1].write(sc_uint <32>(in_addr2 + i));
			}
			wait();
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C * HW_out * HW_out) {
					Y[(i + j) / (C * HW_out * HW_out)][((i + j) / (HW_out * HW_out)) % C][((i + j) / HW_out) % HW_out][(i + j) % HW_out] = ALU_mem_read_data[0][j].read();
				}
				if (i + j < B * C * HW * HW) {
					Z[(i + j) / (C * HW * HW)][((i + j) / (HW * HW)) % C][((i + j) / HW) % HW + Padding][(i + j) % HW + Padding] = ALU_mem_read_data[1][j].read();
				}
			}
		}
		ALU_mem_read[0].write(false);
		ALU_mem_read_addr[0].write(sc_uint <32>(0));
		ALU_mem_read[1].write(false);
		ALU_mem_read_addr[1].write(sc_uint <32>(0));

		for (unsigned int b = 0; b < B; b++) {
			for (unsigned int c = 0; c < C; c++) {
				for (unsigned int h = 0; h + k <= HW_in_pad; h += Stride) {
					for (unsigned int w = 0; w + k <= HW_in_pad; w += Stride) {
						float max_location[2] = {0};
						for (unsigned int i = 0; i < k; i++) {
							for (unsigned int j = 0; j < k; j++) {
								if (Z[b][c][h + i][w + j] > Z[b][c][h + max_location[0]][w + max_location[1]]) {
									max_location[0] = i;
									max_location[1] = j;
								}
							}
						}
						X[b][c][h + max_location[0]][w + max_location[1]] = Y[b][c][h / Stride][w / Stride];
					}
				}
			}
		}
		unsigned int wait_cycle = B * C * HW_out * HW_out * k * k / ALU_calulate_parallelism;
		for (unsigned int i = 0; i < wait_cycle; i++) wait();

		for (unsigned int i = 0; i < B * C * HW * HW; i += ALU_read_write_num) {
			ALU_mem_write.write(true);
			ALU_mem_write_addr.write(sc_uint <32>(out_addr + i));
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C * HW * HW) {
					ALU_mem_write_data[j].write(X[(i + j) / (C * HW * HW)][((i + j) / (HW * HW)) % C][((i + j) / HW) % HW + Padding][(i + j) % HW + Padding]);
				}
			}
			wait();
		}
		ALU_mem_write.write(false);
		ALU_mem_write_addr.write(sc_uint <32>(0));
		for (int i = 0; i < ALU_read_write_num; i++)
			ALU_mem_write_data[i].write(float(0));
	}

	else if (mode == POOL_ave_bp) {
		unsigned int HW_in_pad = HW + 2 * Padding;
		unsigned int HW_out = (HW + 2 * Padding - k) / Stride + 1;

		vector<vector<vector<vector<float>>>> X(B, vector<vector<vector<float>>>(C, vector<vector<float>>(HW_in_pad, vector<float>(HW_in_pad, 0))));
		vector<vector<vector<vector<float>>>> Y(B, vector<vector<vector<float>>>(C, vector<vector<float>>(HW_out, vector<float>(HW_out, 0))));

		for (unsigned int i = 0; i < B * C * HW_out * HW_out; i += ALU_read_write_num) {
			ALU_mem_read[0].write(true);
			ALU_mem_read_addr[0].write(sc_uint <32>(in_addr1 + i));
			wait();
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C * HW_out * HW_out) {
					Y[(i + j) / (C * HW_out * HW_out)][((i + j) / (HW_out * HW_out)) % C][((i + j) / HW_out) % HW_out][(i + j) % HW_out] = ALU_mem_read_data[0][j].read();
				}

			}
		}
		ALU_mem_read[0].write(false);
		ALU_mem_read_addr[0].write(sc_uint <32>(0));

		for (unsigned int b = 0; b < B; b++) {
			for (unsigned int c = 0; c < C; c++) {
				for (unsigned int h = 0; h + k <= HW_in_pad; h += Stride) {
					for (unsigned int w = 0; w + k <= HW_in_pad; w += Stride) {
						for (unsigned int i = 0; i < k; i++) {
							for (unsigned int j = 0; j < k; j++) {
								X[b][c][h + i][w + j] = Y[b][c][h / Stride][w / Stride] / (k * k);
							}
						}
					}
				}
			}
		}
		unsigned int wait_cycle = B * C * HW_out * HW_out * k * k / ALU_calulate_parallelism;
		for (unsigned int i = 0; i < wait_cycle; i++) wait();

		for (unsigned int i = 0; i < B * C * HW * HW; i += ALU_read_write_num) {
			ALU_mem_write.write(true);
			ALU_mem_write_addr.write(sc_uint <32>(out_addr + i));
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C * HW * HW) {
					ALU_mem_write_data[j].write(X[(i + j) / (C * HW * HW)][((i + j) / (HW * HW)) % C][((i + j) / HW) % HW + Padding][(i + j) % HW + Padding]);
				}
			}
			wait();
		}
		ALU_mem_write.write(false);
		ALU_mem_write_addr.write(sc_uint <32>(0));
		for (int i = 0; i < ALU_read_write_num; i++)
			ALU_mem_write_data[i].write(float(0));
	}
}

void ALU::ALU_act(sc_uint<32> in_addr1, sc_uint<32> in_addr2, sc_uint<32> out_addr, sc_uint<16> B, sc_uint<16> C, sc_uint<16> H, sc_uint<16> W, sc_uint<8> mode) {
	//cout << "ALU: ACT begin!" << endl;

	if (mode == RELU_fp) {
		vector<float> X(B * C * H * W, 0);
		vector<float> Y(B * C * H * W, 0);

		for (unsigned int i = 0; i < B * C * H * W; i += ALU_read_write_num) {
			ALU_mem_read[0].write(true);
			ALU_mem_read_addr[0].write(sc_uint <32>(in_addr1 + i));
			wait();
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C * H * W) {
					X[i + j] = ALU_mem_read_data[0][j].read();
				}
			}
		}
		ALU_mem_read[0].write(false);
		ALU_mem_read_addr[0].write(sc_uint <32>(0));

		for (unsigned int i = 0; i < B * C * H * W; i++) {
			Y[i] = X[i] > 0 ? X[i] : 0;
		}

		//unsigned int wait_cycle = B * C * H * W;
		//for (unsigned int i = 0; i < wait_cycle; i++) wait();

		for (unsigned int i = 0; i < B * C * H * W; i += ALU_read_write_num) {
			ALU_mem_write.write(true);
			ALU_mem_write_addr.write(sc_uint <32>(out_addr + i));
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C * H * W) {
					ALU_mem_write_data[j].write(Y[i + j]);
				}
				else {
					ALU_mem_write_data[j].write(float(0));
				}
			}
			wait();
		}
		ALU_mem_write.write(false);
		ALU_mem_write_addr.write(sc_uint <32>(0));
		for (int i = 0; i < ALU_read_write_num; i++)
			ALU_mem_write_data[i].write(float(0));
	}
	else if (mode == RELU_bp) {
		vector<float> X(B * C * H * W, 0);
		vector<float> Z(B * C * H * W, 0);
		vector<float> Y(B * C * H * W, 0);

		for (unsigned int i = 0; i < B * C * H * W; i += ALU_read_write_num) {
			ALU_mem_read[0].write(true);
			ALU_mem_read_addr[0].write(sc_uint <32>(in_addr1 + i));
			ALU_mem_read[1].write(true);
			ALU_mem_read_addr[1].write(sc_uint <32>(in_addr2 + i));
			wait();
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C * H * W) {
					Y[i + j] = ALU_mem_read_data[0][j].read();
					Z[i + j] = ALU_mem_read_data[1][j].read();
				}
			}
		}
		ALU_mem_read[0].write(false);
		ALU_mem_read_addr[0].write(sc_uint <32>(0));
		ALU_mem_read[1].write(false);
		ALU_mem_read_addr[1].write(sc_uint <32>(0));


		for (unsigned int i = 0; i < B * C * H * W; i++) {
			X[i] = Z[i] > 0 ? Y[i] : 0;
		}

		//unsigned int wait_cycle = B * C * H * W;
		//for (unsigned int i = 0; i < wait_cycle; i++) wait();

		for (unsigned int i = 0; i < B * C * H * W; i += ALU_read_write_num) {
			ALU_mem_write.write(true);
			ALU_mem_write_addr.write(sc_uint <32>(out_addr + i));
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * C * H * W) {
					ALU_mem_write_data[j].write(X[i + j]);
				}
				else {
					ALU_mem_write_data[j].write(float(0));
				}
			}
			wait();
		}
		ALU_mem_write.write(false);
		ALU_mem_write_addr.write(sc_uint <32>(0));
		for (int i = 0; i < ALU_read_write_num; i++)
			ALU_mem_write_data[i].write(float(0));
	}
}

void ALU::ALU_fc(sc_uint<32> in_addr1, sc_uint<32> in_addr2, sc_uint<32> out_addr, sc_uint<16> B, sc_uint<16> M, sc_uint<16> N, sc_uint<8> mode) {
	//cout << "ALU: fc begin!" << endl;
	vector<vector<float>> X(B, vector<float>(M, 0));
	vector<vector<float>> Z(M, vector<float>(N, 0));
	vector<vector<float>> Y(B, vector<float>(N, 0));

	if (mode == FC_fp) {
		for (unsigned int i = 0; (i < B * M) || (i < M * N); i += ALU_read_write_num) {
			if (i < B * M) {
				ALU_mem_read[0].write(true);
				ALU_mem_read_addr[0].write(sc_uint <32>(in_addr1 + i));
			}
			else
				ALU_mem_read[0].write(false);
			if (i < M * N) {
				ALU_mem_read[1].write(true);
				ALU_mem_read_addr[1].write(sc_uint <32>(in_addr2 + i));
			}
			else
				ALU_mem_read[1].write(false);
			wait();
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * M) {
					X[(i + j) / M][(i + j) % M] = ALU_mem_read_data[0][j].read();
				}
				if (i + j < M * N) {
					Z[(i + j) / N][(i + j) % N] = ALU_mem_read_data[1][j].read();
				}
			}
		}
		ALU_mem_read[0].write(false);
		ALU_mem_read_addr[0].write(sc_uint <32>(0));
		ALU_mem_read[1].write(false);
		ALU_mem_read_addr[1].write(sc_uint <32>(0));

		for (unsigned int b = 0; b < B; b++) {
			for (unsigned int n = 0; n < N; n++) {
				for (unsigned int m = 0; m < M; m++) {
					Y[b][n] += X[b][m] * Z[m][n];
				}
			}
		}

		unsigned int wait_cycle = B * M * N / ALU_calulate_parallelism;
		for (unsigned int i = 0; i < wait_cycle; i++) wait();

		for (unsigned int i = 0; i < B * N; i += ALU_read_write_num) {
			ALU_mem_write.write(true);
			ALU_mem_write_addr.write(sc_uint <32>(out_addr + i));
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * N) {
					ALU_mem_write_data[j].write(Y[(i + j) / N][(i + j) % N]);
				}
				else {
					ALU_mem_write_data[j].write(float(0));
				}
			}
			wait();
		}
		ALU_mem_write.write(false);
		ALU_mem_write_addr.write(sc_uint <32>(0));
		for (int i = 0; i < ALU_read_write_num; i++)
			ALU_mem_write_data[i].write(float(0));
	}
	else if(mode == FC_bp){
		for (unsigned int i = 0; (i < B * N) || (i < M * N); i += ALU_read_write_num) {
			if (i < B * N) {
				ALU_mem_read[0].write(true);
				ALU_mem_read_addr[0].write(sc_uint <32>(in_addr1 + i));
			}
			else
				ALU_mem_read[0].write(false);
			if (i < M * N) {
				ALU_mem_read[1].write(true);
				ALU_mem_read_addr[1].write(sc_uint <32>(in_addr2 + i));
			}
			else
				ALU_mem_read[1].write(false);
			wait();
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * N) {
					Y[(i + j) / N][(i + j) % N] = ALU_mem_read_data[0][j].read();
				}
				if (i + j < M * N) {
					Z[(i + j) / N][(i + j) % N] = ALU_mem_read_data[1][j].read();
				}
			}
		}
		ALU_mem_read[0].write(false);
		ALU_mem_read_addr[0].write(sc_uint <32>(0));
		ALU_mem_read[1].write(false);
		ALU_mem_read_addr[1].write(sc_uint <32>(0));

		for (unsigned int b = 0; b < B; b++) {
			for (unsigned int m = 0; m < M; m++) {
				for (unsigned int n = 0; n < N; n++) {
					X[b][m] += Y[b][n] * Z[m][n];
				}
			}
		}


		unsigned int wait_cycle = B * M * N / ALU_calulate_parallelism;
		for (unsigned int i = 0; i < wait_cycle; i++) wait();

		for (unsigned int i = 0; i < B * M; i += ALU_read_write_num) {
			ALU_mem_write.write(true);
			ALU_mem_write_addr.write(sc_uint <32>(out_addr + i));
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * M) {
					ALU_mem_write_data[j].write(X[(i + j) / M][(i + j) % M]);
				}
				else {
					ALU_mem_write_data[j].write(float(0));
				}
			}
			wait();
		}
		ALU_mem_write.write(false);
		ALU_mem_write_addr.write(sc_uint <32>(0));
		for (int i = 0; i < ALU_read_write_num; i++)
			ALU_mem_write_data[i].write(float(0));
	}
	else if (mode == FC_divw) {
		for (unsigned int i = 0; (i < B * M) || (i < B * N); i += ALU_read_write_num) {
			if (i < B * M) {
				ALU_mem_read[0].write(true);
				ALU_mem_read_addr[0].write(sc_uint <32>(in_addr1 + i));
			}
			else
				ALU_mem_read[0].write(false);
			if (i < B * N) {
				ALU_mem_read[1].write(true);
				ALU_mem_read_addr[1].write(sc_uint <32>(in_addr2 + i));
			}
			else
				ALU_mem_read[1].write(false);
			wait();
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < B * M) {
					X[(i + j) / M][(i + j) % M] = ALU_mem_read_data[0][j].read();
				}
				if (i + j < B * N) {
					Y[(i + j) / N][(i + j) % N] = ALU_mem_read_data[1][j].read();
				}
			}
		}
		ALU_mem_read[0].write(false);
		ALU_mem_read_addr[0].write(sc_uint <32>(0));
		ALU_mem_read[1].write(false);
		ALU_mem_read_addr[1].write(sc_uint <32>(0));

		for (unsigned int m = 0; m < M; m++) {
			for (unsigned int n = 0; n < N; n++) {
				for (unsigned int b = 0; b < B; b++) {
					Z[m][n] += X[b][m] * Y[b][n];
				}
			}
		}

		unsigned int wait_cycle = B * M * N / ALU_calulate_parallelism;
		for (unsigned int i = 0; i < wait_cycle; i++) wait();

		for (unsigned int i = 0; i < M * N; i += ALU_read_write_num) {
			ALU_mem_write.write(true);
			ALU_mem_write_addr.write(sc_uint <32>(out_addr + i));
			for (unsigned int j = 0; j < ALU_read_write_num; j++) {
				if (i + j < M * N) {
					ALU_mem_write_data[j].write(Z[(i + j) / N][(i + j) % N]);
				}
				else {
					ALU_mem_write_data[j].write(float(0));
				}
			}
			wait();
		}
		ALU_mem_write.write(false);
		ALU_mem_write_addr.write(sc_uint <32>(0));
		for (int i = 0; i < ALU_read_write_num; i++)
			ALU_mem_write_data[i].write(float(0));
	}
}




