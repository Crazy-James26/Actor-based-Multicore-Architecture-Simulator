#pragma once
#include "systemc.h"
#include"basic.h"



//instruction head field
#define RESET		sc_uint<8>(0xff)	
#define Out_Config	sc_uint<8>(0xf1)
#define In_Config	sc_uint<8>(0xf2)
#define Op_Config	sc_uint<8>(0xf3)


//Out_Config instruction field
#define Out_Broadcast	sc_uint<8>(0xf0)
#define Out_Split1		sc_uint<8>(0xf1)
#define Out_Split2		sc_uint<8>(0xf2)
#define Out_Continue	sc_uint<8>(0xff)

//In_Config instruction field
#define In_Singal		sc_uint<8>(0xf0)
#define In_Gather1		sc_uint<8>(0xf1)
#define In_Gather2		sc_uint<8>(0xf2)
#define In_Continue		sc_uint<8>(0xff)



//In_Config instruction field
#define ADD		sc_uint<8>(0xf1)
#define LOSS	sc_uint<8>(0xf2)
#define MUL		sc_uint<8>(0xf3)
#define CONV	sc_uint<8>(0xf4)
#define POOL	sc_uint<8>(0xf5)
#define ACT		sc_uint<8>(0xf6)
#define FC		sc_uint<8>(0xf7)
#define STORE	sc_uint<8>(0xf8)
#define LOAD	sc_uint<8>(0xf9)

//op_address type
#define none_type			sc_uint<2>(0x0)
#define actor_type			sc_uint<2>(0x1)
#define mem_type			sc_uint<2>(0x2)
#define mem_type_release	sc_uint<2>(0x3)

//conv mode
#define CONV_fp			sc_uint<8>(0xf1)
#define CONV_bp			sc_uint<8>(0xf2)
#define CONV_divw		sc_uint<8>(0xf3)

//pool mode
#define POOL_max_fp		sc_uint<8>(0xf1)
#define POOL_max_bp		sc_uint<8>(0xf2)
#define POOL_ave_fp		sc_uint<8>(0xf3)
#define POOL_ave_bp		sc_uint<8>(0xf4)

//active mode
#define RELU_fp		sc_uint<8>(0xf1)
#define RELU_bp		sc_uint<8>(0xf2)

//conv mode
#define FC_fp		sc_uint<8>(0xf1)
#define FC_bp		sc_uint<8>(0xf2)
#define FC_divw		sc_uint<8>(0xf3)




typedef sc_biguint<Ins_Width> Instruction;

#define ins_head_bits Ins_Width-1,Ins_Width-8
#define ins_type_bits Ins_Width-9,Ins_Width-16
#define ins_mode_bits Ins_Width-145,Ins_Width-152

#define ins_op1_type_bits Ins_Width-17,Ins_Width-18
#define ins_op1_bits Ins_Width-19,Ins_Width-50
#define ins_op2_type_bits Ins_Width-51,Ins_Width-52
#define ins_op2_bits Ins_Width-53,Ins_Width-84
#define ins_op3_type_bits Ins_Width-85,Ins_Width-86
#define ins_op3_bits Ins_Width-87,Ins_Width-118
#define ins_op4_type_bits Ins_Width-119,Ins_Width-120
#define ins_op4_bits Ins_Width-121,Ins_Width-152

#define ins_para_bits Ins_Width-17,Ins_Width-152

#define para1_bits ALU_para_Width-1,ALU_para_Width-16
#define para2_bits ALU_para_Width-17,ALU_para_Width-32
#define para3_bits ALU_para_Width-33,ALU_para_Width-48
#define para4_bits ALU_para_Width-49,ALU_para_Width-64
#define para5_bits ALU_para_Width-65,ALU_para_Width-80
#define para6_bits ALU_para_Width-81,ALU_para_Width-96
#define para7_bits ALU_para_Width-97,ALU_para_Width-112
#define para8_bits ALU_para_Width-113,ALU_para_Width-128
#define para9_bits ALU_para_Width-129,ALU_para_Width-136


