#include<iostream>
#include<fstream>
#include<iomanip>
#include<string>

using namespace std;

void write_data_label(string data_file_name, string label_file_name, int N, int C, int H, int W, int totol_cate) {

	ofstream of1(data_file_name);
	if (!of1.is_open()) {
		cout << "The file " << data_file_name << " is unable to open!\n";
	}

	ofstream of2(label_file_name);
	if (!of2.is_open()) {
		cout << "The file " << data_file_name << " is unable to open!\n";
	}
	

	for (int i = 0; i < N; i++) {
		int cate_id = rand() % totol_cate;
		for (int j = 0; j < C * H; j++) {
			for (int k = 0; k < W; k++) {
				if (k % totol_cate == cate_id) {
					of1 << left << setw(10) << setfill(' ') << float((rand() % 11));
				}
				else {
					of1 << left << setw(10) << setfill(' ') << float(-(rand() % 11));
				}
			}
			of1 << endl;
		}
		for (int j = 0; j < totol_cate; j++) {
			if (j == cate_id) {
				of2 << left << setw(10) << setfill(' ') << 1;
			}
			else {
				of2 << left << setw(10) << setfill(' ') << 0;
			}
		}
		of2 << endl;
	}

	for (int i = 0; i < N * C * H * W % 32; i++) {
		of1<< left << setw(10) << setfill(' ') << 0;
	}

	for (int i = 0; i < N * totol_cate % 32; i++) {
		of2 << left << setw(10) << setfill(' ') << 0;
	}

	of1.close();
	of2.close();
}

void write_weight_label(string weihgt_file_name, int N, int C, int H, int W) {
	ofstream of(weihgt_file_name);
	if (!of.is_open()) {
		cout << "The file " << weihgt_file_name << " is unable to open!\n";
	}

	for (int i = 0; i < N * C; i++) {
		for (int j = 0; j < H; j++) {
			for (int k = 0; k < W; k++) {
				of << left << setw(10) << setfill(' ') << float((rand() % 21 - 10)) / 100;
			}
			if (H * W > 32) of << endl;
		}
		if (H * W <= 32 && C * H * W > 32) of << endl;
	}

	for (int i = 0; i < N * C * H * W % 32; i++) {
		of << left << setw(10) << setfill(' ') << 0;
	}

	of.close();
}

void data_divide(string in_file_name, string* out_file_name, int N, int C, int H, int W, int div, int num) {
	ifstream inf(in_file_name);
	if (!inf.is_open()) {
		cout << "The file " << in_file_name << " is unable to open!\n";
	}

	ofstream* of = new ofstream[num];
	for (int i = 0; i < num; i++) {
		of[i].open(out_file_name[i]);
		if (!of[i].is_open()) {
			cout << "The file " << out_file_name[i] << " is unable to open!\n";
		}
	}

	if (div == 1) {
		float data;
		for (int i = 0; i < num; i++) {
			for (int n = 0; n < N/num; n++) {
				for (int c = 0; c < C; c++) {
					for (int h = 0; h < H; h++) {
						for (int w = 0; w < W; w++) {
							inf >> data;
							of[i] << left << setw(10) << setfill(' ') << data;
						}
						if (H * W > 32) of[i] << endl;
					}
					if (H * W <= 32 && C * H * W > 32) of[i] << endl;
				}
			}
		}
	}

	else if (div == 2) {
		float data;
		for (int n = 0; n < N; n++) {
			for (int i = 0; i < num; i++) {
				for (int c = 0; c < C/num; c++) {
					for (int h = 0; h < H; h++) {
						for (int w = 0; w < W; w++) {
							inf >> data;
							of[i] << left << setw(10) << setfill(' ') << data;
						}
						if (H * W > 32) of[i] << endl;
					}
					if (H * W <= 32 && C * H * W > 32) of[i] << endl;
				}
			}
		}
	}

	for (int i = 0; i < num; i++) {
		for (int j = 0; j < (N * C * H * W / num) % 32; j++) {
			of[i] << left << setw(10) << setfill(' ') << 0;
		}
	}

	for (int i = 0; i < num; i++) {
		of[i].close();
	}
	inf.close();
}


int main() {
	//write_data_label("./data_basic/data.txt", "./data_basic/label.txt", 32, 16, 56, 56, 16);
	//write_weight_label("./data_basic/weight1.txt", 16, 16, 3, 3);
	//write_weight_label("./data_basic/weight2.txt", 32, 16, 3, 3);
	//write_weight_label("./data_basic/weight3.txt", 32, 32, 3, 3);
	//write_weight_label("./data_basic/weight5.txt", 1, 1, 512, 16);

	//string dis_data_file_name[4] = { "./data_dis_4/data1.txt","./data_dis_4/data2.txt", "./data_dis_4/data3.txt","./data_dis_4/data4.txt" };
	//data_divide("./data_basic/data.txt", dis_data_file_name, 32, 16, 56, 56, 1, 4);

	//string dis_weight3_file_name[4] = { "./data_dis_4/weight3_1.txt","./data_dis_4/weight3_2.txt", "./data_dis_4/weight3_3.txt","./data_dis_4/weight3_4.txt" };
	//data_divide("./data_basic/weight3.txt", dis_weight3_file_name, 32, 32, 3, 3, 1, 4);

	//string dis_weight2_file_name[2] = { "./data_dis_2/weight2_1.txt","./data_dis_2/weight2_2.txt"};
	//data_divide("./data_basic/weight2.txt", dis_weight2_file_name, 32, 16, 3, 3, 1, 2);

	string dis_weight2_file_name[4] = { "./data_dis_4/weight2_1.txt","./data_dis_4/weight2_2.txt", "./data_dis_4/weight2_3.txt","./data_dis_4/weight2_4.txt" };
	data_divide("./data_basic/weight2.txt", dis_weight2_file_name, 32, 16, 3, 3, 1, 4);

	return 0;
}