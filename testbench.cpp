#include"systemc.h"
#include"host.h"
#include"Actor.h"
#include"result_analysis.h"

using namespace std;

int sc_main(int argc, char* argv[])
{
	sc_clock clk("CLK", 10, SC_NS);
	sc_signal<bool> rst;
	sc_signal<Message> message[test_actor_num + 1][test_actor_num + 1];
	sc_signal<bool> actor_busy[test_actor_num + 1];

	sc_uint<32> host_addr = sc_uint<32>(0x80000000);
	

	rst.write(true);

	sc_uint<32>* addr_list = new sc_uint<32>[test_actor_num + 1];
	for (unsigned int i = 0; i < test_actor_num + 1; i++) {
		addr_list[i] = sc_uint<32>(host_addr + i);
	}



	host* h0;
	sc_uint<32>* h0_adj = new sc_uint<32>[test_actor_num];
	for (int i = 0; i < test_actor_num; i++) {
		h0_adj[i] = addr_list[i + 1];
	}
	h0 = new host("host0", host_addr, h0_adj, test_actor_num);
	h0->clk(clk);
	h0->rst(rst);
	for (int i = 0; i < test_actor_num; i++) {
		h0->message_in.bind(message[i+1][0]);
		h0->message_out.bind(message[0][i+1]);
		h0->actor_busy_port.bind(actor_busy[i+1]);
	}

	Actor** actor =  new Actor*[test_actor_num];
	sc_uint<32>** actor_adj = new sc_uint<32>*[test_actor_num];
	for (int i = 0; i < test_actor_num; i++) {
		int actor_id = i + 1;

		actor_adj[i] = new sc_uint<32>[test_actor_num];
		int adj_id = 0;
		for (int j = 0; j < test_actor_num + 1; j++) {
			if (j != actor_id) {
				actor_adj[i][adj_id] = addr_list[j];
				adj_id++;
			}
		}

		string actor_name_str = "actor" + to_string(actor_id);
		sc_module_name actor_name = actor_name_str.c_str();
		actor[i] = new Actor(actor_name, addr_list[actor_id], actor_adj[i], test_actor_num, host_addr);
		actor[i]->clk(clk);
		actor[i]->rst(rst);
		for (int j = 0; j < test_actor_num + 1; j++) {
			if (j != actor_id) {
				actor[i]->message_in.bind(message[j][actor_id]);
				actor[i]->message_out.bind(message[actor_id][j]);
			}
		}
		actor[i]->actor_busy(actor_busy[i + 1]);
	}

	
	sc_start(100, SC_NS);
	rst.write(false);
	sc_start(test_time, SC_NS);

	int batch_time;
	for (int i = 1; i <= test_actor_num; i++) {
		string in_file_name = "./" + file_name + "/result/actor_" + to_string(i) + ".txt";
		vector<period> compt_period;
		vector<period> commut_period;
		vector<period> merge_period;
		int compute_time;
		int communicate_time;
		int overlap_time;
		int sum_time;

		if (i == 1) {
			batch_time = get_total_time(in_file_name);
		}
		compute_compute_time(in_file_name, compt_period, compute_time);
		compute_communicate_time(in_file_name, commut_period, communicate_time);
		compute_sum_time(compt_period, commut_period, merge_period, sum_time);
		overlap_time = compute_time + communicate_time - sum_time;

		string out_file_name = "./" + file_name + "/report/report_" + to_string(i) + ".txt";
		ofstream of(out_file_name);
		if (!of.is_open()) {
			cout << "The file " << out_file_name << " is unable to open!\n";
		}
		else {
			of << "Timing Report" << endl;
			of << "--------------------------------------------------------------------------------" << endl;
			of << "Total_time: " << batch_time << endl;
			of << endl;
			of << "Compute_period: " << endl;
			for (int i = 0; i < compt_period.size(); i++) {
				of << "(" << compt_period[i].begin_time << ", " << compt_period[i].end_time << ")" << endl;
			}
			of << "Compute_time: " << compute_time << " Compute_utilization: " << float(compute_time) / batch_time * 100 << "%" << endl;

			of << endl;
			of << "Communicate_period: " << endl;
			for (int i = 0; i < commut_period.size(); i++) {
				of << "(" << commut_period[i].begin_time << ", " << commut_period[i].end_time << ")" << endl;
			}
			of << "Communicate_time: " << communicate_time << " Communicate_utilization: " << float(communicate_time) / batch_time * 100 << "%" << endl;

			of << endl;
			of << "merge_period: " << endl;
			for (int i = 0; i < merge_period.size(); i++) {
				of << "(" << merge_period[i].begin_time << ", " << merge_period[i].end_time << ")" << endl;
			}
			of << "Overlap_time: " << overlap_time << " overlap_utilization: " << float(overlap_time) / batch_time * 100 << "%" << endl;
			of << "Sum_time: " << sum_time << " Sum_utilization: " << float(sum_time) / batch_time * 100 << "%" << endl;
		}

		vector<alloc_data> alloc_data_set;
		record_alloc_rate(in_file_name, alloc_data_set);
		of << endl;
		of << "Alloc_rate: " << endl;
		for (int i = 0; i < alloc_data_set.size(); i++) {
			of << "(" << alloc_data_set[i].time << ", " << alloc_data_set[i].alloc_rate << ")" << endl;
		}

		of.close();
	}

	return 0;
}