/*
    Copyright 2016-2017 StapleButter

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#ifndef ARMJIT_ALU_H
#define ARMJIT_ALU_H

namespace ARMJIT
{

#define A_PROTO_ALU_OP(x) \
\
void A_##x##_IMM(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_LSL_IMM(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_LSR_IMM(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_ASR_IMM(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_ROR_IMM(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_LSL_REG(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_LSR_REG(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_ASR_REG(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_ROR_REG(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_IMM_S(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_LSL_IMM_S(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_LSR_IMM_S(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_ASR_IMM_S(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_ROR_IMM_S(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_LSL_REG_S(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_LSR_REG_S(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_ASR_REG_S(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_ROR_REG_S(ARM* cpu, u32 pc, u32 instr);

#define A_PROTO_ALU_TEST(x) \
\
void A_##x##_IMM(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_LSL_IMM(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_LSR_IMM(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_ASR_IMM(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_ROR_IMM(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_LSL_REG(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_LSR_REG(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_ASR_REG(ARM* cpu, u32 pc, u32 instr); \
void A_##x##_REG_ROR_REG(ARM* cpu, u32 pc, u32 instr);

A_PROTO_ALU_OP(AND)
A_PROTO_ALU_OP(EOR)
A_PROTO_ALU_OP(SUB)
A_PROTO_ALU_OP(RSB)
A_PROTO_ALU_OP(ADD)
A_PROTO_ALU_OP(ADC)
A_PROTO_ALU_OP(SBC)
A_PROTO_ALU_OP(RSC)
A_PROTO_ALU_TEST(TST)
A_PROTO_ALU_TEST(TEQ)
A_PROTO_ALU_TEST(CMP)
A_PROTO_ALU_TEST(CMN)
A_PROTO_ALU_OP(ORR)
A_PROTO_ALU_OP(MOV)
A_PROTO_ALU_OP(BIC)
A_PROTO_ALU_OP(MVN)

void A_MUL(ARM* cpu, u32 pc, u32 instr);
void A_MLA(ARM* cpu, u32 pc, u32 instr);
void A_UMULL(ARM* cpu, u32 pc, u32 instr);
void A_UMLAL(ARM* cpu, u32 pc, u32 instr);
void A_SMULL(ARM* cpu, u32 pc, u32 instr);
void A_SMLAL(ARM* cpu, u32 pc, u32 instr);
void A_SMLAxy(ARM* cpu, u32 pc, u32 instr);
void A_SMLAWy(ARM* cpu, u32 pc, u32 instr);
void A_SMULxy(ARM* cpu, u32 pc, u32 instr);
void A_SMULWy(ARM* cpu, u32 pc, u32 instr);
void A_SMLALxy(ARM* cpu, u32 pc, u32 instr);

void A_CLZ(ARM* cpu, u32 pc, u32 instr);
void A_QADD(ARM* cpu, u32 pc, u32 instr);
void A_QSUB(ARM* cpu, u32 pc, u32 instr);
void A_QDADD(ARM* cpu, u32 pc, u32 instr);
void A_QDSUB(ARM* cpu, u32 pc, u32 instr);


void T_LSL_IMM(ARM* cpu, u32 pc, u32 instr);
void T_LSR_IMM(ARM* cpu, u32 pc, u32 instr);
void T_ASR_IMM(ARM* cpu, u32 pc, u32 instr);

void T_ADD_REG_(ARM* cpu, u32 pc, u32 instr);
void T_SUB_REG_(ARM* cpu, u32 pc, u32 instr);
void T_ADD_IMM_(ARM* cpu, u32 pc, u32 instr);
void T_SUB_IMM_(ARM* cpu, u32 pc, u32 instr);

void T_MOV_IMM(ARM* cpu, u32 pc, u32 instr);
void T_CMP_IMM(ARM* cpu, u32 pc, u32 instr);
void T_ADD_IMM(ARM* cpu, u32 pc, u32 instr);
void T_SUB_IMM(ARM* cpu, u32 pc, u32 instr);

void T_AND_REG(ARM* cpu, u32 pc, u32 instr);
void T_EOR_REG(ARM* cpu, u32 pc, u32 instr);
void T_LSL_REG(ARM* cpu, u32 pc, u32 instr);
void T_LSR_REG(ARM* cpu, u32 pc, u32 instr);
void T_ASR_REG(ARM* cpu, u32 pc, u32 instr);
void T_ADC_REG(ARM* cpu, u32 pc, u32 instr);
void T_SBC_REG(ARM* cpu, u32 pc, u32 instr);
void T_ROR_REG(ARM* cpu, u32 pc, u32 instr);
void T_TST_REG(ARM* cpu, u32 pc, u32 instr);
void T_NEG_REG(ARM* cpu, u32 pc, u32 instr);
void T_CMP_REG(ARM* cpu, u32 pc, u32 instr);
void T_CMN_REG(ARM* cpu, u32 pc, u32 instr);
void T_ORR_REG(ARM* cpu, u32 pc, u32 instr);
void T_MUL_REG(ARM* cpu, u32 pc, u32 instr);
void T_BIC_REG(ARM* cpu, u32 pc, u32 instr);
void T_MVN_REG(ARM* cpu, u32 pc, u32 instr);

void T_ADD_HIREG(ARM* cpu, u32 pc, u32 instr);
void T_CMP_HIREG(ARM* cpu, u32 pc, u32 instr);
void T_MOV_HIREG(ARM* cpu, u32 pc, u32 instr);

void T_ADD_PCREL(ARM* cpu, u32 pc, u32 instr);
void T_ADD_SPREL(ARM* cpu, u32 pc, u32 instr);
void T_ADD_SP(ARM* cpu, u32 pc, u32 instr);

}

#endif