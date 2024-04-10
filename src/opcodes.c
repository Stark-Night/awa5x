/* awa5x - Extended AWA5.0
   Copyright © 2024 Starknights

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>. */

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
          return 1;
     default:
          break;
     }
     return 0;
}
