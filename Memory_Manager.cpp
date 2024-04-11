/*#include"Memory_Manager.h"

void Memory_Manager::memory_alloc() {
	for (int i = 0; i < block_num; i++) {
		memory_table[i].valid = false;
	}
	mte_pt = 0;

	alloc_ack.write(0);
	alloc_addr[0].write(0);
	alloc_addr[0].write(0);
	alloc_addr[0].write(0);

	while (true) {
		alloc_ack.write(0);
		alloc_addr[0].write(0);
		alloc_addr[0].write(0);
		alloc_addr[0].write(0);
		if (alloc_req.read() == true) {
			sc_uint<32> addr[3] = { 0 };
			sc_uint<32> size[3];
			size[0] = (alloc_size[0].read() >> block_size) + alloc_size[0].read().range(block_size - 1, 0) > 0 ? 1 : 0;
			size[1] = (alloc_size[1].read() >> block_size) + alloc_size[0].read().range(block_size - 1, 0) > 0 ? 1 : 0;
			size[2] = (alloc_size[2].read() >> block_size) + alloc_size[0].read().range(block_size - 1, 0) > 0 ? 1 : 0;
			
			bool flag[3] = { 0 };
			
			for (int i = 0; i < 3; i++) {
				if (size[i] == 0) {
					flag[i] = 1;
					continue;
				}
				else {
					unsigned int valid_size = 0;
					addr[i] = 0;
					for (unsigned int j = 1; j < block_num; j++) {
						if (memory_table[j].valid == true) {
							addr[i] = j + 1;
							valid_size = 0;
						}
						else {
							valid_size++;
							if (valid_size >= size[0]) {
								flag[i] = 1;
								for (unsigned int k = 0; k < valid_size; k++)
									memory_table[addr[i] + k].valid == true;
								break;
							}
						}
					}
				}
			}
			if (flag[0] && flag[1] && flag[2]) {
				alloc_ack.write(sc_uint<2>(1));
				alloc_addr[0].write(addr[0]);
				alloc_addr[0].write(addr[1]);
				alloc_addr[0].write(addr[2]);
			}
			else {
				alloc_ack.write(sc_uint<2>(2));
			}
		}

	}
}
*/