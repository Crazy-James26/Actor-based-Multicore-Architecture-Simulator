#include<iostream>
#include<fstream>
#include<iomanip>
#include<string>
#include<vector>
#include<algorithm>

using namespace std;

#define test_actor_num 6
#define file_name string("file_basic_Gpipe_2")

int get_total_time(string in_file_name) {
	string s;
	string time_s;
	string time_unit;
	int time = 0;

	ifstream inf(in_file_name);
	if (!inf.is_open()) {
		cout << "The file " << in_file_name << " is unable to open!\n";
	}
	else {
		while (inf.peek() != EOF) {
			inf >> s;

			if (s != "Time") {
				getline(inf, s);
			}
			else {
				inf >> time_s >> time_unit;
				time = stoi(time_s);
				if (time_unit[0] == 'u')
					time *= 1000;
				getline(inf, s);
			}
		}
	}
	inf.close();
	return time;
}

struct period {
	int begin_time;
	int end_time;
	period(int time1, int time2) : begin_time(time1), end_time(time2) {};
};

void period_set_sort(vector<period>& ps) {
	sort(ps.begin(), ps.end(), [&](const period& p1, const period& p2)->bool {
		return p1.begin_time != p2.begin_time ? p1.begin_time < p2.begin_time : p1.end_time < p2.end_time;
		});
}


void period_set_merge(vector<period>& ps1, vector<period>& ps2, vector<period>& merge_ps) {
	merge_ps.clear();
	for (int i = 0; i < ps1.size(); i++) {
		merge_ps.push_back(ps1[i]);
	}

	for (int i = 0; i < ps2.size(); i++) {
		merge_ps.push_back(ps2[i]);
	}

	period_set_sort(merge_ps);
}

void compute_compute_time(string in_file_name, vector<period>& compt_period, int& compute_time) {
	ifstream inf(in_file_name);
	if (!inf.is_open()) {
		cout << "The file " << in_file_name << " is unable to open!\n";
	}
	else {
		compt_period.clear();
		compute_time = 0;

		string s;
		string time_s;
		string time_unit;
		string ALU_s;
		string ALU_state;
		int time;
		int begin_time;
		int end_time;
		while (inf.peek() != EOF) {
			inf >> s;

			if (s != "Time") {
				getline(inf, s);
			}
			else {
				inf >> time_s >> time_unit >> ALU_s;
				if (ALU_s == "ALU") {
					time = stoi(time_s);
					if (time_unit[0] == 'u')
						time *= 1000;

					inf >> ALU_state;
					if (ALU_state == "starts") {
						begin_time = time;
						getline(inf, s);
					}
					else if (ALU_state == "finishes") {
						end_time = time;
						period new_period(begin_time, end_time);
						compt_period.push_back(new_period);
						getline(inf, s);
					}
				}
				else {
					getline(inf, s);
				}
			}
		}
		for (int i = 0; i < compt_period.size(); i++) {
			compute_time += compt_period[i].end_time - compt_period[i].begin_time;
		}
		inf.close();
	}
}

void compute_communicate_time(string in_file_name, vector<period>& commut_period, int& communicate_time) {
	ifstream inf(in_file_name);
	if (!inf.is_open()) {
		cout << "The file " << in_file_name << " is unable to open!\n";
	}
	else {
		commut_period.clear();
		communicate_time = 0;

		string s;
		string time_s;
		string time_unit;
		string communicate_s[4];
		string communicate_state;
		int time = 0;
		int begin_time = 0;
		int end_time = 0;
		int overlap_num = 0;
		while (inf.peek() != EOF) {
			inf >> s;

			if (s != "Time") {
				getline(inf, s);
			}
			else {
				inf >> time_s >> time_unit >> communicate_s[0];
				if (communicate_s[0][0] == 'r' || communicate_s[0] == "starts" || communicate_s[0] == "finishes") {
					inf >> communicate_s[1] >> communicate_s[2] >> communicate_s[3];

					if (communicate_s[1] == "sending" || communicate_s[3] == "sending") {
						time = stoi(time_s);
						if (time_unit[0] == 'u')
							time *= 1000;

						if (communicate_s[1] == "sending")
							communicate_state = communicate_s[0];
						else
							communicate_state = communicate_s[2];

						if (communicate_state == "starts") {
							if (overlap_num == 0) {
								begin_time = time;
							}
							overlap_num++;
							getline(inf, s);
						}
						else if (communicate_state == "finishes") {
							overlap_num--;
							if (overlap_num == 0) {
								end_time = time;
								period new_period(begin_time, end_time);
								commut_period.push_back(new_period);
							}
							getline(inf, s);
						}
					}
					else {
						getline(inf, s);
					}
				}
				else {
					getline(inf, s);
				}
			}
		}
		for (int i = 0; i < commut_period.size(); i++) {
			communicate_time += commut_period[i].end_time - commut_period[i].begin_time;
		}
		inf.close();
	}
};

void compute_sum_time(vector<period>& compt_period, vector<period>& commut_period, vector<period>& merge_period, int& sum_time) {
	sum_time = 0;
	merge_period.clear();
	vector<period> merge_period_overlap;
	period_set_merge(compt_period, commut_period, merge_period_overlap);

	int begin_pt;
	int end_pt;
	for (int i = 0; i < merge_period_overlap.size(); i++) {
		if (i == 0) {
			begin_pt = 0;
			end_pt = 0;
		}
		else {
			if (merge_period_overlap[i].begin_time >= merge_period_overlap[end_pt].end_time) {
				period new_period(merge_period_overlap[begin_pt].begin_time, merge_period_overlap[end_pt].end_time);
				merge_period.push_back(new_period);
				begin_pt = i;
				end_pt = i;
			}
			else if (merge_period_overlap[i].end_time >= merge_period_overlap[end_pt].end_time) {
				end_pt = i;
			}
		}
		if (i == merge_period_overlap.size() - 1) {
			period new_period(merge_period_overlap[begin_pt].begin_time, merge_period_overlap[end_pt].end_time);
			merge_period.push_back(new_period);
		}
	}

	for (int i = 0; i < merge_period.size(); i++) {
		sum_time += merge_period[i].end_time - merge_period[i].begin_time;
	}
};

struct alloc_data {
	int time;
	string alloc_rate;
	alloc_data(int time, string s) : time(time), alloc_rate(s) {};
};


void record_alloc_rate(string in_file_name, vector<alloc_data>& alloc_data_set) {
	ifstream inf(in_file_name);
	if (!inf.is_open()) {
		cout << "The file " << in_file_name << " is unable to open!\n";
	}
	else {
		alloc_data_set.clear();

		string s;
		string time_s;
		string time_unit;
		int time;
		string alloc_s;
		string alloc_rate;
		while (inf.peek() != EOF) {
			inf >> s;
			if (s != "Time") {
				getline(inf, s);
			}
			else {
				inf >> time_s >> time_unit >> alloc_s;
				if (alloc_s == "mem_alloc_rate:") {
					time = stoi(time_s);
					if (time_unit[0] == 'u')
						time *= 1000;
					inf >> alloc_rate;
					alloc_data new_alloc_data(time, alloc_rate);
					alloc_data_set.push_back(new_alloc_data);
					inf.get();
				}
				else {
					getline(inf, s);
				}
			}
		}
		inf.close();
	}
}

int main() {
	int batch_time;
	for (int i = 1; i <= test_actor_num; i++) {
		string in_file_name = "C:/Users/new/Desktop/Actor model 4/" + file_name + "/result/actor_"+ to_string(i) + ".txt";
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

		string out_file_name = "C:/Users/new/Desktop/Actor model 4/" + file_name + "/report/report_" + to_string(i) + ".txt";
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
}

