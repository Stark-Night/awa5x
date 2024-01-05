#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>

/* An enum might appear more type safe, but due to comparisons with
   sized types (e.g. int8_t) things might fall through
   implementation-specific details and I don't want to keep track of
   that. */

#define NOP 0x00
#define PRN 0x01
#define PR1 0x02
#define RED 0x03
#define R3D 0x04
#define BLO 0x05
#define SBM 0x06
#define POP 0x07
#define DPL 0x08
#define SRN 0x09
#define MRG 0x0A
#define DD4 0x0B
#define SUB 0x0C
#define MUL 0x0D
#define DIV 0x0E
#define CNT 0x0F
#define LBL 0x10
#define JMP 0x11
#define EQL 0x12
#define LSS 0x13
#define GR8 0x14
#define EQZ 0x15

// this is the last valid opcode
#define TRM 0x1F

// the valid alphabet (AwaSCII)
#define ALPHABET ("AWawJELYHOSIUMjelyhosiumPCNTpcntBDFGRbdfgr0123456789 .,!'()~_/;\n")

// length of the alphabet; it's always less than 128 so it can be safely casted to int8_t
#define NALNUM ((int8_t)(sizeof(ALPHABET)-1))

const char *
opcode_name(int8_t opcode);

int
opcode_has_parameter(int8_t opcode);

#endif
