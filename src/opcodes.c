#include "config.h"

#include <stdint.h>
#include "opcodes.h"

const char *
opcode_name(int8_t opcode) {
     switch (opcode) {
     case NOP: return "NOP";
     case PRN: return "PRN";
     case PR1: return "PR1";
     case RED: return "RED";
     case R3D: return "R3D";
     case BLO: return "BLO";
     case SBM: return "SBM";
     case POP: return "POP";
     case DPL: return "DPL";
     case SRN: return "SRN";
     case MRG: return "MRG";
     case DD4: return "4DD";
     case SUB: return "SUB";
     case MUL: return "MUL";
     case DIV: return "DIV";
     case CNT: return "CNT";
     case LBL: return "LBL";
     case JMP: return "JMP";
     case EQL: return "EQL";
     case LSS: return "LSS";
     case GR8: return "GR8";
     case EQZ: return "EQZ";
     case TRM: return "TRM";
     default:
          break;
     }
     return "<?>";
}

int
opcode_has_parameter(int8_t opcode) {
     switch (opcode) {
     case BLO:
     case SBM:
     case SRN:
     case LBL:
     case JMP:
     case EQL:
     case LSS:
     case GR8:
     case EQZ:
          return 1;
     default:
          break;
     }
     return 0;
}
