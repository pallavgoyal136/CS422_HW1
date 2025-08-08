#include <math.h>
#include "mips.h"
#include "opcodes.h"
#include <assert.h>
#include "app_syscall.h"

#define MAGIC_EXEC 0xdeadbeef

/*------------------------------------------------------------------------
 *
 *  Instruction exec 
 *
 *------------------------------------------------------------------------
 */
Bypass_information
Mipc::getBypassInfo (unsigned int ins)
{
    MipsInsn i;
    signed int a1, a2;
    unsigned int ar1, ar2, s1, s2, r1, r2, t1, t2;
    LL addr;
    unsigned int val;
    LL value, mask;
    int sa,j;
    Word dummy;
    unsigned     _decodedSRC1 = 0, _decodedSRC2 = 0;      // Reg fetch output (source values)
    unsigned from_where;
    Bool isSRC2FP = FALSE, isSRC1FP = FALSE;
    i.data = ins;
    Bool data_dependency;
    Bool isHi = FALSE, isLo = FALSE;
    Bypass_information bypass_info;
    Bool isSRC2STORE = FALSE;
    bypass_info.SRC1Bypass = NO_BYPASS;
    bypass_info.SRC2Bypass = NO_BYPASS;
    bypass_info.SRC1Reg = NO_BYPASS;
    bypass_info.SRC2Reg = NO_BYPASS;
    bypass_info.HiBypass = NO_BYPASS;
    bypass_info.LoBypass = NO_BYPASS;
    bypass_info.isSRC1FP = FALSE;
    bypass_info.isSRC2FP = FALSE;
    bypass_info.isStall = FALSE;
    bypass_info.isJR = FALSE;
    bypass_info.isSRC2STORE = FALSE;
#define SIGN_EXTEND_BYTE(x)  do { x <<= 24; x >>= 24; } while (0)
#define SIGN_EXTEND_IMM(x)   do { x <<= 16; x >>= 16; } while (0)

    switch (i.reg.op) {
    case 0:
      // SPECIAL (ALU format)
      _decodedSRC1 = i.reg.rs;
      _decodedSRC2 = i.reg.rt;
      switch (i.reg.func){
        case 0x10:			// mfhi
         isHi = TRUE;
	 break;

      case 0x12:			// mflo
         isLo = TRUE;
	 break;
      case 0x9:			// jalr
         bypass_info.isJR = TRUE;
    break;
      case 0x8:			// jr
         bypass_info.isJR = TRUE;
    break;
      }
      break;    // ALU format

    case 8:          // addi
    case 9:          // addiu
      // ignore overflow: no exceptions
      _decodedSRC1 = i.imm.rs;
      break;

    case 0xc:          // andi
      _decodedSRC1 = i.imm.rs;
      break;

    case 0xf:          // lui
      break;

    case 0xd:          // ori
      _decodedSRC1 = i.imm.rs;
      break;

    case 0xa:          // slti
      _decodedSRC1 = i.imm.rs;
      break;

    case 0xb:          // sltiu
      _decodedSRC1 = i.imm.rs;
      break;

    case 0xe:          // xori
      _decodedSRC1 = i.imm.rs;
      break;

    case 4:          // beq
      _decodedSRC1 = i.imm.rs;
      _decodedSRC2 = i.imm.rt;
      break;

    case 1:
      // REGIMM
      _decodedSRC1 = i.reg.rs;

      switch (i.reg.rt) {
      case 1:          // bgez
      break;

      case 0x11:         // bgezal
      break;

      case 0x10:         // bltzal
      break;

      case 0x0:          // bltz
      break;

      default:
      break;
      }
      break;

    case 7:          // bgtz
      _decodedSRC1 = i.reg.rs;
      break;

    case 6:          // blez
      _decodedSRC1 = i.reg.rs;
      break;

    case 5:          // bne
      _decodedSRC1 = i.reg.rs;
      _decodedSRC2 = i.reg.rt;
      break;

    case 2:          // j
      break;

    case 3:          // jal
      break;

    case 0x20:         // lb
      _decodedSRC1 = i.reg.rs;
      break;

    case 0x24:         // lbu
      _decodedSRC1 = i.reg.rs;
      break;

    case 0x21:         // lh
      _decodedSRC1 = i.reg.rs;
      break;

    case 0x25:         // lhu
      _decodedSRC1 = i.reg.rs;
      break;

    case 0x22:         // lwl
      _decodedSRC1 = i.reg.rs;
      break;

    case 0x23:         // lw
      _decodedSRC1 = i.reg.rs;
      break;

    case 0x26:         // lwr
      _decodedSRC1 = i.reg.rs;
      break;

    case 0x31:         // lwc1
      _decodedSRC1 = i.reg.rs;
      break;

    case 0x39:         // swc1
      _decodedSRC1 = i.reg.rs;
      _decodedSRC2 = i.reg.rt;
      isSRC2FP = TRUE;
      isSRC2STORE = TRUE;
      break;

    case 0x28:         // sb
      _decodedSRC1 = i.reg.rs;
      _decodedSRC2 = i.reg.rt;
      isSRC2STORE = TRUE;
      break;

    case 0x29:         // sh  store half word
      _decodedSRC1 = i.reg.rs;
      _decodedSRC2 = i.reg.rt;
      isSRC2STORE = TRUE;
      break;

    case 0x2a:         // swl
      _decodedSRC1 = i.reg.rs;
      _decodedSRC2 = i.reg.rt;
      isSRC2STORE = TRUE;
      break;

    case 0x2b:         // sw
      _decodedSRC1 = i.reg.rs;
      _decodedSRC2 = i.reg.rt;
      isSRC2STORE = TRUE;
      break;

    case 0x2e:         // swr
      _decodedSRC1 = i.reg.rs;
      _decodedSRC2 = i.reg.rt;
      isSRC2STORE = TRUE;
      break;

    case 0x11:         // floating-point
      switch (i.freg.fmt) {
      case 4:          // mtc1
        _decodedSRC1 = i.freg.ft;
      break;

      case 0:          // mfc1
        _decodedSRC1 = i.freg.fs;
        isSRC1FP = TRUE;
      break;
      default:
      break;
      }
      break;
    default:
      break;
    }
    bypass_info.SRC1Reg = _decodedSRC1;
    bypass_info.SRC2Reg = _decodedSRC2;
    bypass_info.isSRC1FP = isSRC1FP;
    bypass_info.isSRC2FP = isSRC2FP;
    unsigned DST1 = 0,DST2 = 0; //these are the potential data hazard candidates we need to check the destination of past instructions with src of current instruction
    Bool isDST1FP = FALSE, isDST2FP = FALSE;
    Bool isDST1LOAD = _memControl_p[ID_EX] && (_writeREG_p[ID_EX] || _writeFREG_p[ID_EX]);
    if(_writeREG_p[ID_EX]) DST1 = _decodedDST_p[ID_EX];
    if(_writeREG_p[EX_MEM]) DST2 = _decodedDST_p[EX_MEM];
    if(_writeFREG_p[ID_EX]) {
        DST1 = _decodedDST_p[ID_EX];
        isDST1FP = TRUE;
    }
    if(_writeFREG_p[EX_MEM]) {
        DST2 = _decodedDST_p[EX_MEM];
        isDST2FP = TRUE;
    }
    if(_decodedSRC1 > 0){
        if((isDST2FP == isSRC1FP) && _decodedSRC1 == DST2) bypass_info.SRC1Bypass = MEM_EX;
        if((isDST1FP == isSRC1FP) && _decodedSRC1 == DST1) bypass_info.SRC1Bypass = EX_EX;
    }
    if(_decodedSRC2 > 0){
        if((isDST2FP == isSRC2FP) && _decodedSRC2 == DST2) bypass_info.SRC2Bypass = MEM_EX;
        if((isDST1FP == isSRC2FP) && _decodedSRC2 == DST1){
            if(isDST1LOAD && isSRC2STORE){
                bypass_info.SRC2Bypass = MEM_MEM;
            }
            else{
                bypass_info.SRC2Bypass = EX_EX;
            }
        }
    }
    if(isHi){
        if(_hiWPort_p[EX_MEM]) bypass_info.HiBypass = MEM_EX;
        if(_hiWPort_p[ID_EX]) bypass_info.HiBypass = EX_EX;
    }
    if(isLo){
        if(_loWPort_p[EX_MEM]) bypass_info.LoBypass = MEM_EX;
        if(_loWPort_p[ID_EX]) bypass_info.LoBypass = EX_EX;
    }
    if(isDST1LOAD){
        if(_decodedSRC1>0){
            if((isDST1FP == isSRC1FP) && DST1 == _decodedSRC1) bypass_info.isStall = TRUE;
        }
        if(_decodedSRC2>0){
            if((isDST1FP == isSRC1FP) && (DST1 == _decodedSRC2) && !isSRC2STORE) bypass_info.isStall = TRUE; //just preivious is load, and cu
        }
    }
    bypass_info.isSRC2STORE = isSRC2STORE;
    if(bypass_info.isStall){
        bypass_info.SRC1Bypass = NO_BYPASS;
        bypass_info.SRC2Bypass = NO_BYPASS;
        bypass_info.SRC1Reg = NO_BYPASS;
        bypass_info.SRC2Reg = NO_BYPASS;
        bypass_info.HiBypass = NO_BYPASS;
        bypass_info.LoBypass = NO_BYPASS;
        bypass_info.isJR = FALSE;
        bypass_info.isSRC2STORE = FALSE;
    }
    return bypass_info;
}
void
Mipc::Dec (unsigned int ins)
{
   MipsInsn i;
   signed int a1, a2;
   unsigned int ar1, ar2, s1, s2, r1, r2, t1, t2;
   LL addr;
   unsigned int val;
   LL value, mask;
   int sa,j;
   Word dummy;
   signed int   _decodedSRC1, _decodedSRC2;     // Reg fetch output (source values)
   unsigned     _decodedDST;                    // Decoder output (dest reg no)
   unsigned     _subregOperand;                 // Needed for lwl and lwr
   Bool         _memControl;                    // Memory instruction?
   Bool         _writeREG, _writeFREG;          // WB control
   signed int   _branchOffset;
   Bool         _hiWPort, _loWPort;             // WB control
   unsigned     _decodedShiftAmt;               // Shift amount
   Bool         _isIllegalOp;
   void (*_opControl)(Mipc*, unsigned);
   void (*_memOp)(Mipc*);
   _isIllegalOp = FALSE;
   //_isSyscall = FALSE;
   _isSyscall[ID_EX] = FALSE;
   int          _bdslot = 0;                           // 1 if the next ins is delay slot
   unsigned int _btgt; 
   i.data = ins;
  
#define SIGN_EXTEND_BYTE(x)  do { x <<= 24; x >>= 24; } while (0)
#define SIGN_EXTEND_IMM(x)   do { x <<= 16; x >>= 16; } while (0)

   switch (i.reg.op) {
   case 0:
      // SPECIAL (ALU format)
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = _gpr[i.reg.rt];
      _decodedDST = i.reg.rd;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;

      switch (i.reg.func) {
      case 0x20:			// add
      case 0x21:			// addu
         _opControl = func_add_addu;
	 break;

      case 0x24:			// and
         _opControl = func_and;
	 break;

      case 0x27:			// nor
         _opControl = func_nor;
	 break;

      case 0x25:			// or
         _opControl = func_or;
	 break;

      case 0:			// sll
         _opControl = func_sll;
         _decodedShiftAmt = i.reg.sa;
	 break;

      case 4:			// sllv
         _opControl = func_sllv;
	 break;

      case 0x2a:			// slt
         _opControl = func_slt;
	 break;

      case 0x2b:			// sltu
         _opControl = func_sltu;
	 break;

      case 0x3:			// sra
         _opControl = func_sra;
         _decodedShiftAmt = i.reg.sa;
	 break;

      case 0x7:			// srav
         _opControl = func_srav;
	 break;

      case 0x2:			// srl
         _opControl = func_srl;
         _decodedShiftAmt = i.reg.sa;
	 break;

      case 0x6:			// srlv
         _opControl = func_srlv;
	 break;

      case 0x22:			// sub
      case 0x23:			// subu
	 // no overflow check
         _opControl = func_sub_subu;
	 break;

      case 0x26:			// xor
         _opControl = func_xor;
	 break;

      case 0x1a:			// div
         _opControl = func_div;
         _hiWPort = TRUE;
         _loWPort = TRUE;
         _writeREG = FALSE;
         _writeFREG = FALSE;
	 break;

      case 0x1b:			// divu
         _opControl = func_divu;
         _hiWPort = TRUE;
         _loWPort = TRUE;
         _writeREG = FALSE;
         _writeFREG = FALSE;
	 break;

      case 0x10:			// mfhi
         _opControl = func_mfhi;
	 break;

      case 0x12:			// mflo
         _opControl = func_mflo;
	 break;

      case 0x11:			// mthi
         _opControl = func_mthi;
         _hiWPort = TRUE;
         _writeREG = FALSE;
         _writeFREG = FALSE;
	 break;

      case 0x13:			// mtlo
         _opControl = func_mtlo;
         _loWPort = TRUE;
         _writeREG = FALSE;
         _writeFREG = FALSE;
	 break;

      case 0x18:			// mult
         _opControl = func_mult;
         _hiWPort = TRUE;
         _loWPort = TRUE;
         _writeREG = FALSE;
         _writeFREG = FALSE;
	 break;

      case 0x19:			// multu
         _opControl = func_multu;
         _hiWPort = TRUE;
         _loWPort = TRUE;
         _writeREG = FALSE;
          _writeFREG = FALSE;
	 break;

      case 9:			// jalr
         _opControl = func_jalr;
         _btgt = _decodedSRC1;
         _bdslot = 1;
         break;

      case 8:			// jr
         _opControl = func_jr;
         _writeREG = FALSE;
         _writeFREG = FALSE;
         _btgt = _decodedSRC1;
         _bdslot = 1;
	 break;

      case 0xd:			// await/break
         _opControl = func_await_break;
         _writeREG = FALSE;
         _writeFREG = FALSE;
	 break;

      case 0xc:			// syscall
         _opControl = func_syscall;
         _writeREG = FALSE;
         _writeFREG = FALSE;
         //_isSyscall = TRUE;
         _isSyscall[ID_EX] = TRUE;
	 break;

      default:
	 _isIllegalOp = TRUE;
         _writeREG = FALSE;
         _writeFREG = FALSE;
	 break;
      }
      break;	// ALU format

   case 8:			// addi
   case 9:			// addiu
      // ignore overflow: no exceptions
      _opControl = func_addi_addiu;
      _decodedSRC1 = _gpr[i.imm.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.imm.rt;
      _writeREG = TRUE;
       _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;

   case 0xc:			// andi
      _opControl = func_andi;
      _decodedSRC1 = _gpr[i.imm.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.imm.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;

   case 0xf:			// lui
      _opControl = func_lui;
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.imm.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;

   case 0xd:			// ori
      _opControl = func_ori;
      _decodedSRC1 = _gpr[i.imm.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.imm.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;

   case 0xa:			// slti
      _opControl = func_slti;
      _decodedSRC1 = _gpr[i.imm.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.imm.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;

   case 0xb:			// sltiu
      _opControl = func_sltiu;
      _decodedSRC1 = _gpr[i.imm.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.imm.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;

   case 0xe:			// xori
      _opControl = func_xori;
      _decodedSRC1 = _gpr[i.imm.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.imm.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;

   case 4:			// beq
      _opControl = func_beq;
      _decodedSRC1 = _gpr[i.imm.rs];
      _decodedSRC2 = _gpr[i.imm.rt];
      _branchOffset = i.imm.imm;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      _branchOffset <<= 16; _branchOffset >>= 14; _bdslot = 1; _btgt = (unsigned)((signed)_pc_if_id+_branchOffset+4);
      break;

   case 1:
      // REGIMM
      _decodedSRC1 = _gpr[i.reg.rs];
      _branchOffset = i.imm.imm;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;

      switch (i.reg.rt) {
      case 1:			// bgez
         _opControl = func_bgez;
         _branchOffset <<= 16; _branchOffset >>= 14; _bdslot = 1; _btgt = (unsigned)((signed)_pc_if_id+_branchOffset+4);
	 break;

      case 0x11:			// bgezal
         _opControl = func_bgezal;
         _decodedDST = 31;
         _writeREG = TRUE;
         _branchOffset <<= 16; _branchOffset >>= 14; _bdslot = 1; _btgt = (unsigned)((signed)_pc_if_id+_branchOffset+4);
	 break;

      case 0x10:			// bltzal
         _opControl = func_bltzal;
         _decodedDST = 31;
         _writeREG = TRUE;
         _branchOffset <<= 16; _branchOffset >>= 14; _bdslot = 1; _btgt = (unsigned)((signed)_pc_if_id+_branchOffset+4);
	 break;

      case 0x0:			// bltz
         _opControl = func_bltz;
         _branchOffset <<= 16; _branchOffset >>= 14; _bdslot = 1; _btgt = (unsigned)((signed)_pc_if_id+_branchOffset+4);
	 break;

      default:
	 _isIllegalOp = TRUE;
	 break;
      }
      break;

   case 7:			// bgtz
      _opControl = func_bgtz;
      _decodedSRC1 = _gpr[i.reg.rs];
      _branchOffset = i.imm.imm;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      _branchOffset <<= 16; _branchOffset >>= 14; _bdslot = 1; _btgt = (unsigned)((signed)_pc_if_id+_branchOffset+4);
      break;

   case 6:			// blez
      _opControl = func_blez;
      _decodedSRC1 = _gpr[i.reg.rs];
      _branchOffset = i.imm.imm;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      _branchOffset <<= 16; _branchOffset >>= 14; _bdslot = 1; _btgt = (unsigned)((signed)_pc_if_id+_branchOffset+4);
      break;

   case 5:			// bne
      _opControl = func_bne;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = _gpr[i.reg.rt];
      _branchOffset = i.imm.imm;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      _branchOffset <<= 16; _branchOffset >>= 14; _bdslot = 1; _btgt = (unsigned)((signed)_pc_if_id+_branchOffset+4);
      break;

   case 2:			// j
      _opControl = func_j;
      _branchOffset = i.tgt.tgt;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      _btgt = ((_pc_if_id+4) & 0xf0000000) | (_branchOffset<<2); _bdslot = 1;
      break;

   case 3:			// jal
      _opControl = func_jal;
      _branchOffset = i.tgt.tgt;
      _decodedDST = 31;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      _btgt = ((_pc_if_id+4) & 0xf0000000) | (_branchOffset<<2); _bdslot = 1;
      break;

   case 0x20:			// lb  
      _opControl = func_lb;
      _memOp = mem_lb;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x24:			// lbu
      _opControl = func_lbu;
      _memOp = mem_lbu;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x21:			// lh
      _opControl = func_lh;
      _memOp = mem_lh;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x25:			// lhu
      _opControl = func_lhu;
      _memOp = mem_lhu;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x22:			// lwl
      _opControl = func_lwl;
      _memOp = mem_lwl;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _subregOperand = _gpr[i.reg.rt];
      _decodedDST = i.reg.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x23:			// lw
      _opControl = func_lw;
      _memOp = mem_lw;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x26:			// lwr
      _opControl = func_lwr;
      _memOp = mem_lwr;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _subregOperand = _gpr[i.reg.rt];
      _decodedDST = i.reg.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x31:			// lwc1
      _opControl = func_lwc1;
      _memOp = mem_lwc1;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      _writeREG = FALSE;
      _writeFREG = TRUE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x39:			// swc1
      _opControl = func_swc1;
      _memOp = mem_swc1;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x28:			// sb
      _opControl = func_sb;
      _memOp = mem_sb;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x29:			// sh  store half word
      _opControl = func_sh;
      _memOp = mem_sh;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x2a:			// swl
      _opControl = func_swl;
      _memOp = mem_swl;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x2b:			// sw
      _opControl = func_sw;
      _memOp = mem_sw;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x2e:			// swr
      _opControl = func_swr;
      _memOp = mem_swr;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x11:			// floating-point
      _fpinst++;
      switch (i.freg.fmt) {
      case 4:			// mtc1
         _opControl = func_mtc1;
         _decodedSRC1 = _gpr[i.freg.ft];
         _decodedDST = i.freg.fs;
         _writeREG = FALSE;
         _writeFREG = TRUE;
         _hiWPort = FALSE;
         _loWPort = FALSE;
         _memControl = FALSE;
	 break;

      case 0:			// mfc1
         _opControl = func_mfc1;
         _decodedSRC1 = _fpr[(i.freg.fs)>>1].l[FP_TWIDDLE^((i.freg.fs)&1)];
         _decodedDST = i.freg.ft;
         _writeREG = TRUE;
         _writeFREG = FALSE;
         _hiWPort = FALSE;
         _loWPort = FALSE;
         _memControl = FALSE;
	 break;
      default:
         _isIllegalOp = TRUE;
         _writeREG = FALSE;
         _writeFREG = FALSE;
         _hiWPort = FALSE;
         _loWPort = FALSE;
         _memControl = FALSE;
	 break;
      }
      break;
   default:
      _isIllegalOp = TRUE;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;
   }
_decodedSRC1_p[ID_EX] = _decodedSRC1;
_decodedSRC2_p[ID_EX] = _decodedSRC2;
_decodedDST_p[ID_EX] = _decodedDST;
_subregOperand_p[ID_EX] = _subregOperand;
_memControl_p[ID_EX] = _memControl;
_writeREG_p[ID_EX] = _writeREG;
_writeFREG_p[ID_EX] = _writeFREG;
_branchOffset_p[ID_EX] = _branchOffset;
_hiWPort_p[ID_EX] = _hiWPort;
_loWPort_p[ID_EX] = _loWPort;
_decodedShiftAmt_p[ID_EX] = _decodedShiftAmt;
_isIllegalOp_p[ID_EX] = _isIllegalOp;
_opControl_p[ID_EX] = _opControl;
_memOp_p[ID_EX] = _memOp;
_bdslot_p[ID_EX] = _bdslot;                           // 1 if the next ins is delay slot
_btgt_p[ID_EX] = _btgt; 
}


/*
 *
 * Debugging: print registers
 *
 */
void 
Mipc::dumpregs (void)
{
   int i;

   //printf ("\n--- PC = %08x ---\n", _pc);
   printf ("\n--- PC = %08x ---\n", _pc_fetch);
   for (i=0; i < 32; i++) {
      if (i < 10)
	 printf (" r%d: %08x (%ld)\n", i, _gpr[i], _gpr[i]);
      else
	 printf ("r%d: %08x (%ld)\n", i, _gpr[i], _gpr[i]);
   }
   //printf ("taken: %d, bd: %d\n", _btaken, _bdslot);
   //printf ("target: %08x\n", _btgt);
   printf ("taken: %d, bd: %d\n", _btaken_p[MEM_WB], _bdslot_p[MEM_WB]);
   printf ("target: %08x\n", _btgt_p[MEM_WB]);
}

void
Mipc::func_add_addu (Mipc *mc, unsigned ins)
{
   //printf("Encountered unimplemented instruction: add or addu.\n");
   //printf("You need to fill in func_add_addu in exec_helper.cc to proceed forward.\n");
   mc->_opResultLo_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX] + mc->_decodedSRC2_p[ID_EX]);
   //exit(0);
}

void
Mipc::func_and (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = mc->_decodedSRC1_p[ID_EX] & mc->_decodedSRC2_p[ID_EX];
}

void
Mipc::func_nor (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = ~(mc->_decodedSRC1_p[ID_EX] | mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_or (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = mc->_decodedSRC1_p[ID_EX] | mc->_decodedSRC2_p[ID_EX];
}

void
Mipc::func_sll (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = mc->_decodedSRC2_p[ID_EX] << mc->_decodedShiftAmt_p[ID_EX];
}

void
Mipc::func_sllv (Mipc *mc, unsigned ins)
{
   //printf("Encountered unimplemented instruction: sllv.\n");
   //printf("You need to fill in func_sllv in exec_helper.cc to proceed forward.\n");
   mc->_opResultLo_ex_mem_wr = mc->_decodedSRC2_p[ID_EX] << (mc->_decodedSRC1_p[ID_EX] & 0x1f);
   //exit(0);
}

void
Mipc::func_slt (Mipc *mc, unsigned ins)
{
   if (mc->_decodedSRC1_p[ID_EX] < mc->_decodedSRC2_p[ID_EX]) {
      mc->_opResultLo_ex_mem_wr = 1;
   }
   else {
      mc->_opResultLo_ex_mem_wr = 0;
   }
}

void
Mipc::func_sltu (Mipc *mc, unsigned ins)
{
   if ((unsigned)mc->_decodedSRC1_p[ID_EX] < (unsigned)mc->_decodedSRC2_p[ID_EX]) {
      mc->_opResultLo_ex_mem_wr = 1;
   }
   else {
      mc->_opResultLo_ex_mem_wr = 0;
   }
}

void
Mipc::func_sra (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = mc->_decodedSRC2_p[ID_EX] >> mc->_decodedShiftAmt_p[ID_EX];
}

void
Mipc::func_srav (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = mc->_decodedSRC2_p[ID_EX] >> (mc->_decodedSRC1_p[ID_EX] & 0x1f);
}

void
Mipc::func_srl (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = (unsigned)mc->_decodedSRC2_p[ID_EX] >> mc->_decodedShiftAmt_p[ID_EX];
}

void
Mipc::func_srlv (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = (unsigned)mc->_decodedSRC2_p[ID_EX] >> (mc->_decodedSRC1_p[ID_EX] & 0x1f);
}

void
Mipc::func_sub_subu (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = (unsigned)mc->_decodedSRC1_p[ID_EX] - (unsigned)mc->_decodedSRC2_p[ID_EX];
}

void
Mipc::func_xor (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = mc->_decodedSRC1_p[ID_EX] ^ mc->_decodedSRC2_p[ID_EX];
}

void
Mipc::func_div (Mipc *mc, unsigned ins)
{
   if (mc->_decodedSRC2_p[ID_EX] != 0) {
      mc->_opResultHi_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX] % mc->_decodedSRC2_p[ID_EX]);
      mc->_opResultLo_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX] / mc->_decodedSRC2_p[ID_EX]);
   }
   else {
      mc->_opResultHi_ex_mem_wr = 0x7fffffff;
      mc->_opResultLo_ex_mem_wr = 0x7fffffff;
   }
}

void
Mipc::func_divu (Mipc *mc, unsigned ins)
{
   if ((unsigned)mc->_decodedSRC2_p[ID_EX] != 0) {
      mc->_opResultHi_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]) % (unsigned)(mc->_decodedSRC2_p[ID_EX]);
      mc->_opResultLo_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]) / (unsigned)(mc->_decodedSRC2_p[ID_EX]);
   }
   else {
      mc->_opResultHi_ex_mem_wr = 0x7fffffff;
      mc->_opResultLo_ex_mem_wr = 0x7fffffff;
   }
}

void
Mipc::func_mfhi (Mipc *mc, unsigned ins)
{
   //mc->_opResultLo_ex_mem_wr = mc->_hi;
   mc->_opResultLo_ex_mem_wr = mc->_hi_EX;
}

void
Mipc::func_mflo (Mipc *mc, unsigned ins)
{
   //mc->_opResultLo_ex_mem_wr = mc->_lo;
   mc->_opResultLo_ex_mem_wr = mc->_lo_EX;
}

void
Mipc::func_mthi (Mipc *mc, unsigned ins)
{
   mc->_opResultHi_ex_mem_wr = mc->_decodedSRC1_p[ID_EX];
}

void
Mipc::func_mtlo (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = mc->_decodedSRC1_p[ID_EX];
}

void
Mipc::func_mult (Mipc *mc, unsigned ins)
{
   unsigned int ar1, ar2, s1, s2, r1, r2, t1, t2;
                                                                                
   ar1 = mc->_decodedSRC1_p[ID_EX];
   ar2 = mc->_decodedSRC2_p[ID_EX];
   s1 = ar1 >> 31; if (s1) ar1 = 0x7fffffff & (~ar1 + 1);
   s2 = ar2 >> 31; if (s2) ar2 = 0x7fffffff & (~ar2 + 1);
                                                                                
   t1 = (ar1 & 0xffff) * (ar2 & 0xffff);
   r1 = t1 & 0xffff;              // bottom 16 bits
                                                                                
   // compute next set of 16 bits
   t1 = (ar1 & 0xffff) * (ar2 >> 16) + (t1 >> 16);
   t2 = (ar2 & 0xffff) * (ar1 >> 16);
                                                                                
   r1 = r1 | (((t1+t2) & 0xffff) << 16); // bottom 32 bits
   r2 = (ar1 >> 16) * (ar2 >> 16) + (t1 >> 16) + (t2 >> 16) +
            (((t1 & 0xffff) + (t2 & 0xffff)) >> 16);
                                                                                
   if (s1 ^ s2) {
      r1 = ~r1;
      r2 = ~r2;
      r1++;
      if (r1 == 0)
         r2++;
   }
   mc->_opResultHi_ex_mem_wr = r2;
   mc->_opResultLo_ex_mem_wr = r1;
}

void
Mipc::func_multu (Mipc *mc, unsigned ins)
{
   unsigned int ar1, ar2, s1, s2, r1, r2, t1, t2;
                                                                                
   ar1 = mc->_decodedSRC1_p[ID_EX];
   ar2 = mc->_decodedSRC2_p[ID_EX];
                                                                                
   t1 = (ar1 & 0xffff) * (ar2 & 0xffff);
   r1 = t1 & 0xffff;              // bottom 16 bits
                                                                                
   // compute next set of 16 bits
   t1 = (ar1 & 0xffff) * (ar2 >> 16) + (t1 >> 16);
   t2 = (ar2 & 0xffff) * (ar1 >> 16);
                                                                                
   r1 = r1 | (((t1+t2) & 0xffff) << 16); // bottom 32 bits
   r2 = (ar1 >> 16) * (ar2 >> 16) + (t1 >> 16) + (t2 >> 16) +
            (((t1 & 0xffff) + (t2 & 0xffff)) >> 16);
                            
   mc->_opResultHi_ex_mem_wr = r2;
   mc->_opResultLo_ex_mem_wr = r1;                                                    
}

void
Mipc::func_jalr (Mipc *mc, unsigned ins)
{
   mc->_btaken_ex_mem_wr = 1;
   mc->_num_jal++;
   //mc->_opResultLo = mc->_pc + 8;
   mc->_opResultLo_ex_mem_wr = mc->_pc[ID_EX] + 8;
}

void
Mipc::func_jr (Mipc *mc, unsigned ins)
{
   mc->_btaken_ex_mem_wr = 1;
   mc->_num_jr++;
}

void
Mipc::func_await_break (Mipc *mc, unsigned ins)
{
}

void
Mipc::func_syscall (Mipc *mc, unsigned ins)
{
   mc->fake_syscall (ins);
}

void
Mipc::func_addi_addiu (Mipc *mc, unsigned ins)
{
   //printf("Encountered unimplemented instruction: addi or addiu.\n");
   //printf("You need to fill in func_addi_addiu in exec_helper.cc to proceed forward.\n");
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_opResultLo_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX] + mc->_decodedSRC2_p[ID_EX]);
   //exit(0);
}

void
Mipc::func_andi (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = mc->_decodedSRC1_p[ID_EX] & mc->_decodedSRC2_p[ID_EX];
}

void
Mipc::func_lui (Mipc *mc, unsigned ins)
{
   //printf("Encountered unimplemented instruction: lui.\n");
   //printf("You need to fill in func_lui in exec_helper.cc to proceed forward.\n");
   mc->_opResultLo_ex_mem_wr = mc->_decodedSRC2_p[ID_EX] << 16;
   //exit(0);
}

void
Mipc::func_ori (Mipc *mc, unsigned ins)
{
   //printf("Encountered unimplemented instruction: ori.\n");
   //printf("You need to fill in func_ori in exec_helper.cc to proceed forward.\n");
   mc->_opResultLo_ex_mem_wr = mc->_decodedSRC1_p[ID_EX] | mc->_decodedSRC2_p[ID_EX];
   //exit(0);
}

void
Mipc::func_slti (Mipc *mc, unsigned ins)
{
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   if (mc->_decodedSRC1_p[ID_EX] < mc->_decodedSRC2_p[ID_EX]) {
      mc->_opResultLo_ex_mem_wr = 1;
   }
   else {
      mc->_opResultLo_ex_mem_wr = 0;
   }
}

void
Mipc::func_sltiu (Mipc *mc, unsigned ins)
{
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   if ((unsigned)mc->_decodedSRC1_p[ID_EX] < (unsigned)mc->_decodedSRC2_p[ID_EX]) {
      mc->_opResultLo_ex_mem_wr = 1;
   }
   else {
      mc->_opResultLo_ex_mem_wr = 0;
   }
}

void
Mipc::func_xori (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = mc->_decodedSRC1_p[ID_EX] ^ mc->_decodedSRC2_p[ID_EX];
}

void
Mipc::func_beq (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   //printf("Encountered unimplemented instruction: beq.\n");
   //printf("You need to fill in func_beq in exec_helper.cc to proceed forward.\n");
   mc->_btaken_ex_mem_wr = (mc->_decodedSRC1_p[ID_EX] == mc->_decodedSRC2_p[ID_EX]);
   //exit(0);
}

void
Mipc::func_bgez (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->_btaken_ex_mem_wr = !(mc->_decodedSRC1_p[ID_EX] >> 31);
}

void
Mipc::func_bgezal (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->_btaken_ex_mem_wr = !(mc->_decodedSRC1_p[ID_EX] >> 31);
   mc->_opResultLo_ex_mem_wr = mc->_pc[ID_EX] + 8;
}

void
Mipc::func_bltzal (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->_btaken_ex_mem_wr = (mc->_decodedSRC1_p[ID_EX] >> 31);
   mc->_opResultLo_ex_mem_wr = mc->_pc[ID_EX] + 8;
}

void
Mipc::func_bltz (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->_btaken_ex_mem_wr = (mc->_decodedSRC1_p[ID_EX] >> 31);
}

void
Mipc::func_bgtz (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->_btaken_ex_mem_wr = (mc->_decodedSRC1_p[ID_EX] > 0);
}

void
Mipc::func_blez (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->_btaken_ex_mem_wr = (mc->_decodedSRC1_p[ID_EX] <= 0);
}

void
Mipc::func_bne (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->_btaken_ex_mem_wr = (mc->_decodedSRC1_p[ID_EX] != mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_j (Mipc *mc, unsigned ins)
{
   mc->_btaken_ex_mem_wr = 1;
}

void
Mipc::func_jal (Mipc *mc, unsigned ins)
{
   mc->_num_jal++;
   //printf("Encountered unimplemented instruction: jal.\n");
   //printf("You need to fill in func_jal in exec_helper.cc to proceed forward.\n");
   mc->_opResultLo_ex_mem_wr = mc->_pc[ID_EX] + 8;
   mc->_btaken_ex_mem_wr = 1;
   //exit(0);
}

void
Mipc::func_lb (Mipc *mc, unsigned ins)
{
   signed int a1;

   mc->_num_load++;
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_memory_addr_reg_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]+mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_lbu (Mipc *mc, unsigned ins)
{
   mc->_num_load++;
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_memory_addr_reg_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]+mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_lh (Mipc *mc, unsigned ins)
{
   signed int a1;
                                                                                
   mc->_num_load++;
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_memory_addr_reg_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]+mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_lhu (Mipc *mc, unsigned ins)
{
   mc->_num_load++;
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_memory_addr_reg_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]+mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_lwl (Mipc *mc, unsigned ins)
{
   signed int a1;
   unsigned s1;
                                                                                
   mc->_num_load++;
   mc->_remove_num_lwl++;
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_memory_addr_reg_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]+mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_lw (Mipc *mc, unsigned ins)
{
   mc->_num_load++;
   //printf("Encountered unimplemented instruction: lw.\n");
   //printf("You need to fill in func_lw in exec_helper.cc to proceed forward.\n");
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_memory_addr_reg_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]+mc->_decodedSRC2_p[ID_EX]);
   //exit(0);
}

void
Mipc::func_lwr (Mipc *mc, unsigned ins)
{
   unsigned ar1, s1;
                                                                                
   mc->_num_load++;
   mc->_remove_num_lwr++;
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_memory_addr_reg_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]+mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_lwc1 (Mipc *mc, unsigned ins)
{
   mc->_num_load++;
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_memory_addr_reg_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]+mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_swc1 (Mipc *mc, unsigned ins)
{
   mc->_num_store++;
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_memory_addr_reg_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]+mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_sb (Mipc *mc, unsigned ins)
{
   mc->_num_store++;
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_memory_addr_reg_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]+mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_sh (Mipc *mc, unsigned ins)
{
   mc->_num_store++;
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_memory_addr_reg_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]+mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_swl (Mipc *mc, unsigned ins)
{
   unsigned ar1, s1;
                                                                                
   mc->_num_store++;
   mc->_remove_num_swl++;
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_memory_addr_reg_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]+mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_sw (Mipc *mc, unsigned ins)
{
   mc->_num_store++;
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_memory_addr_reg_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]+mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_swr (Mipc *mc, unsigned ins)
{
   unsigned ar1, s1;
                                                                                
   mc->_num_store++;
   mc->_remove_num_swr++;
   SIGN_EXTEND_IMM(mc->_decodedSRC2_p[ID_EX]);
   mc->_memory_addr_reg_ex_mem_wr = (unsigned)(mc->_decodedSRC1_p[ID_EX]+mc->_decodedSRC2_p[ID_EX]);
}

void
Mipc::func_mtc1 (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = mc->_decodedSRC1_p[ID_EX];
}

void
Mipc::func_mfc1 (Mipc *mc, unsigned ins)
{
   mc->_opResultLo_ex_mem_wr = mc->_decodedSRC1_p[ID_EX];
}



void
Mipc::mem_lb (Mipc *mc)
{
   signed int a1;
   a1 = mc->_mem->BEGetByte(mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7));
   SIGN_EXTEND_BYTE(a1);
   mc->_opResultLo_mem_wb_wr = a1;
}

void
Mipc::mem_lbu (Mipc *mc)
{
   mc->_opResultLo_mem_wb_wr = mc->_mem->BEGetByte(mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7));
}

void
Mipc::mem_lh (Mipc *mc)
{
   signed int a1;

   a1 = mc->_mem->BEGetHalfWord(mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7));
   SIGN_EXTEND_IMM(a1);
   mc->_opResultLo_mem_wb_wr = a1;
}

void
Mipc::mem_lhu (Mipc *mc)
{
   mc->_opResultLo_mem_wb_wr = mc->_mem->BEGetHalfWord (mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7));
}

void
Mipc::mem_lwl (Mipc *mc)
{
   signed int a1;
   unsigned s1;

   a1 = mc->_mem->BEGetWord (mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7));
   s1 = (mc->_memory_addr_reg_mem_wb & 3) << 3;
   mc->_opResultLo_mem_wb_wr = (a1 << s1) | (mc->_subregOperand_mem_wb & ~(~0UL << s1));
}

void
Mipc::mem_lw (Mipc *mc)
{
   mc->_opResultLo_mem_wb_wr = mc->_mem->BEGetWord (mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7));
}

void
Mipc::mem_lwr (Mipc *mc)
{
   unsigned ar1, s1;

   ar1 = mc->_mem->BEGetWord (mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7));
   s1 = (~mc->_memory_addr_reg_mem_wb & 3) << 3;
   mc->_opResultLo_mem_wb_wr = (ar1 >> s1) | (mc->_subregOperand_mem_wb & ~(~(unsigned)0 >> s1));
}

void
Mipc::mem_lwc1 (Mipc *mc)
{
   mc->_opResultLo_mem_wb_wr = mc->_mem->BEGetWord (mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7));
}

void
Mipc::mem_swc1 (Mipc *mc)
{
   mc->_mem->Write(mc->_memory_addr_reg_mem_wb & ~(LL)0x7, mc->_mem->BESetWord (mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7), mc->_fpr[mc->_decodedDST_mem_wb>>1].l[FP_TWIDDLE^(mc->_decodedDST_mem_wb&1)]));
}

void
Mipc::mem_sb (Mipc *mc)
{
   mc->_mem->Write(mc->_memory_addr_reg_mem_wb & ~(LL)0x7, mc->_mem->BESetByte (mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7), mc->_gpr[mc->_decodedDST_mem_wb] & 0xff));
}

void
Mipc::mem_sh (Mipc *mc)
{
   mc->_mem->Write(mc->_memory_addr_reg_mem_wb & ~(LL)0x7, mc->_mem->BESetHalfWord (mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7), mc->_gpr[mc->_decodedDST_mem_wb] & 0xffff));
}

void
Mipc::mem_swl (Mipc *mc)
{
   unsigned ar1, s1;

   ar1 = mc->_mem->BEGetWord (mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7));
   s1 = (mc->_memory_addr_reg_mem_wb & 3) << 3;
   ar1 = (mc->_gpr[mc->_decodedDST_mem_wb] >> s1) | (ar1 & ~(~(unsigned)0 >> s1));
   mc->_mem->Write(mc->_memory_addr_reg_mem_wb & ~(LL)0x7, mc->_mem->BESetWord (mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7), ar1));
}

void
Mipc::mem_sw (Mipc *mc)
{
   mc->_mem->Write(mc->_memory_addr_reg_mem_wb & ~(LL)0x7, mc->_mem->BESetWord (mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7), mc->_gpr[mc->_decodedDST_mem_wb]));
}

void
Mipc::mem_swr (Mipc *mc)
{
   unsigned ar1, s1;

   ar1 = mc->_mem->BEGetWord (mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7));
   s1 = (~mc->_memory_addr_reg_mem_wb & 3) << 3;
   ar1 = (mc->_gpr[mc->_decodedDST_mem_wb] << s1) | (ar1 & ~(~0UL << s1));
   mc->_mem->Write(mc->_memory_addr_reg_mem_wb & ~(LL)0x7, mc->_mem->BESetWord (mc->_memory_addr_reg_mem_wb, mc->_mem->Read(mc->_memory_addr_reg_mem_wb & ~(LL)0x7), ar1));
}
