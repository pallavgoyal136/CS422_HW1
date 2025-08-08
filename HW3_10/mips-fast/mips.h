#ifndef __MIPS_H__
#define __MIPS_H__

#include "sim.h"

class Mipc;
class MipcSysCall;
class SysCall;

typedef unsigned Bool;
#define TRUE 1
#define FALSE 0

#if BYTE_ORDER == LITTLE_ENDIAN

#define FP_TWIDDLE 0

#else

#define FP_TWIDDLE 1

#endif

#include "mem.h"
#include "../../common/syscall.h"
#include "queue.h"

#define MIPC_DEBUG 1
#define IF_ID 0
#define ID_EX 1
#define EX_MEM 2
#define MEM_WB 3
#define NO_BYPASS 0
#define MEM_EX 1
#define EX_EX 2
#define MEM_MEM 3
struct Bypass_information{
   unsigned SRC1Bypass;
   unsigned SRC2Bypass;
   unsigned SRC1Reg;
   unsigned SRC2Reg;
   unsigned HiBypass;
   unsigned LoBypass;
   Bool isSRC1FP;
   Bool isSRC2FP;
   Bool isStall;
   Bool isJR;
   Bool isSRC2STORE;
}; 
class Mipc : public SimObject {
public:
   Mipc (Mem *m);
   ~Mipc ();
  
   FAKE_SIM_TEMPLATE;

   MipcSysCall *_sys;		// Emulated system call layer

   void dumpregs (void);	// Dumps current register state

   void Reboot (char *image = NULL);
				// Restart processor.
				// "image" = file name for new memory
				// image if any.

   void MipcDumpstats();			// Prints simulation statistics
   void Dec (unsigned int ins);			// Decoder function
   void fake_syscall (unsigned int ins);	// System call interface
   Bool checkDataHazard (unsigned int ins); //Check for potential data dependencies
   Bypass_information getBypassInfo (unsigned int ins); //Check for potential data dependencies
   /* processor state */
   //unsigned int _ins;   // instruction register
   unsigned int _ins[4];   // instruction register
   Bool _stall[4]; //pipeline stage to be stalled
   Bool         _insValid;      // Needed for unpipelined design
   Bool         _decodeValid;   // Needed for unpipelined design
   Bool		_execValid;	// Needed for unpipelined design
   Bool		_memValid;	// Needed for unpipelined design
   Bool         _insDone;       // Needed for unpipelined design
   Bool _stall_branch, _stall_data, _stall_syscall;
/*
   signed int	_decodedSRC1, _decodedSRC2;	// Reg fetch output (source values)
   unsigned	_decodedDST;			// Decoder output (dest reg no)
   unsigned 	_subregOperand;			// Needed for lwl and lwr
   unsigned	_memory_addr_reg;				// Memory address register
   unsigned	_opResultHi, _opResultLo;	// Result of operation
   Bool 	_memControl;			// Memory instruction?
   Bool		_writeREG, _writeFREG;		// WB control
   signed int	_branchOffset;
   Bool 	_hiWPort, _loWPort;		// WB control
   unsigned	_decodedShiftAmt;		// Shift amount
  */ 

   signed int   _decodedSRC1_p[4], _decodedSRC2_p[4];     // Reg fetch output (source values)
   unsigned     _decodedDST_p[4];                    // Decoder output (dest reg no)
   unsigned     _subregOperand_p[4];                 // Needed for lwl and lwr
   unsigned     _memory_addr_reg_p[4];                               // Memory address register
   unsigned     _opResultHi_p[4], _opResultLo_p[4];       // Result of operation
   unsigned     _memory_addr_reg_ex_mem_wr;                               // this is for the functions which will be executewd during positive half of the ex stage and we would like to save the results in a temporary type of register and ensure that the pipeline registers are actually written only in the negative half to prevent race condition
   unsigned     _opResultHi_ex_mem_wr, _opResultLo_ex_mem_wr,_opResultLo_mem_wb_wr;       // Result of operation
   unsigned     _memory_addr_reg_mem_wb, _subregOperand_mem_wb, _decodedDST_mem_wb;
   Bool         _memControl_p[4];                    // Memory instruction?
   Bool         _writeREG_p[4], _writeFREG_p[4];          // WB control
   signed int   _branchOffset_p[4];
   Bool         _hiWPort_p[4], _loWPort_p[4];             // WB control
   unsigned     _decodedShiftAmt_p[4]; 

   unsigned int 	_gpr[32];		// general-purpose integer registers

   union {
      unsigned int l[2];
      float f[2];
      double d;
   } _fpr[16];					// floating-point registers (paired)

   //unsigned int _hi, _lo; 			// mult, div destination
   unsigned int _hi, _lo; 			// mult, div destination
   unsigned int _hi_EX, _lo_EX; 			// to be used for the EX stage
   unsigned int	_pc_fetch;				// Program counter
   unsigned int	_pc[4];				// Program counter
   unsigned int _pc_if_id;			//BECUASE WE CAN'T READ FROM PIPELINE REG IN NEG OF DECODE , SO WE STORE THE PC IN POSITIVE HALF
   unsigned int _lastbdslot_p[4];			// branch delay state
   unsigned int _boot;				// boot code loaded?

   int 		_btaken_p[4]; 			// taken branch (1 if taken, 0 if fall-through)
   int 		_btaken_ex_mem_wr; 			// taken branch (1 if taken, 0 if fall-through)
   int 		_bdslot_p[4];				// 1 if the next ins is delay slot
   unsigned int	_btgt_p[4];				// branch target

   //Bool		_isSyscall;			// 1 if system call
   Bool		_isSyscall[4];			// 1 if system call
   Bool		_isIllegalOp_p[4];			// 1 if illegal opcode

   //Information regarding bypass stored in ID/RF phase and used in futher pipelinesatges to resolve bypass if any
   //Passing information about the source register numbers decoded in ID/RF used further
   Bypass_information _bypass_p[4];

   // Simulation statistics counters

   LL	_nfetched;
   LL	_num_cond_br;
   LL	_num_jal;
   LL	_num_jr;
   LL   _num_load;
   LL   _num_store;
   LL   _fpinst;
   LL   _remove_num_lwl;
   LL   _remove_num_lwr;
   LL   _remove_num_swl;
   LL   _remove_num_swr;

   Mem	*_mem;	// attached memory (not a cache)

   Log	_l;
   int  _sim_exit;		// 1 on normal termination

   //void (*_opControl)(Mipc*, unsigned);
   void (*_opControl_p[4])(Mipc*, unsigned);
   //void (*_memOp)(Mipc*);
   void (*_memOp_p[4])(Mipc*);

   FILE *_debugLog;

   // EXE stage definitions

   static void func_add_addu (Mipc*, unsigned);
   static void func_and (Mipc*, unsigned);
   static void func_nor (Mipc*, unsigned);
   static void func_or (Mipc*, unsigned);
   static void func_sll (Mipc*, unsigned);
   static void func_sllv (Mipc*, unsigned);
   static void func_slt (Mipc*, unsigned);
   static void func_sltu (Mipc*, unsigned);
   static void func_sra (Mipc*, unsigned);
   static void func_srav (Mipc*, unsigned);
   static void func_srl (Mipc*, unsigned);
   static void func_srlv (Mipc*, unsigned);
   static void func_sub_subu (Mipc*, unsigned);
   static void func_xor (Mipc*, unsigned);
   static void func_div (Mipc*, unsigned);
   static void func_divu (Mipc*, unsigned);
   static void func_mfhi (Mipc*, unsigned);
   static void func_mflo (Mipc*, unsigned);
   static void func_mthi (Mipc*, unsigned);
   static void func_mtlo (Mipc*, unsigned);
   static void func_mult (Mipc*, unsigned);
   static void func_multu (Mipc*, unsigned);
   static void func_jalr (Mipc*, unsigned);
   static void func_jr (Mipc*, unsigned);
   static void func_await_break (Mipc*, unsigned);
   static void func_syscall (Mipc*, unsigned);
   static void func_addi_addiu (Mipc*, unsigned);
   static void func_andi (Mipc*, unsigned);
   static void func_lui (Mipc*, unsigned);
   static void func_ori (Mipc*, unsigned);
   static void func_slti (Mipc*, unsigned);
   static void func_sltiu (Mipc*, unsigned);
   static void func_xori (Mipc*, unsigned);
   static void func_beq (Mipc*, unsigned);
   static void func_bgez (Mipc*, unsigned);
   static void func_bgezal (Mipc*, unsigned);
   static void func_bltzal (Mipc*, unsigned);
   static void func_bltz (Mipc*, unsigned);
   static void func_bgtz (Mipc*, unsigned);
   static void func_blez (Mipc*, unsigned);
   static void func_bne (Mipc*, unsigned);
   static void func_j (Mipc*, unsigned);
   static void func_jal (Mipc*, unsigned);
   static void func_lb (Mipc*, unsigned);
   static void func_lbu (Mipc*, unsigned);
   static void func_lh (Mipc*, unsigned);
   static void func_lhu (Mipc*, unsigned);
   static void func_lwl (Mipc*, unsigned);
   static void func_lw (Mipc*, unsigned);
   static void func_lwr (Mipc*, unsigned);
   static void func_lwc1 (Mipc*, unsigned);
   static void func_swc1 (Mipc*, unsigned);
   static void func_sb (Mipc*, unsigned);
   static void func_sh (Mipc*, unsigned);
   static void func_swl (Mipc*, unsigned);
   static void func_sw (Mipc*, unsigned);
   static void func_swr (Mipc*, unsigned);
   static void func_mtc1 (Mipc*, unsigned);
   static void func_mfc1 (Mipc*, unsigned);

   // MEM stage definitions

   static void mem_lb (Mipc*);
   static void mem_lbu (Mipc*);
   static void mem_lh (Mipc*);
   static void mem_lhu (Mipc*);
   static void mem_lwl (Mipc*);
   static void mem_lw (Mipc*);
   static void mem_lwr (Mipc*);
   static void mem_lwc1 (Mipc*);
   static void mem_swc1 (Mipc*);
   static void mem_sb (Mipc*);
   static void mem_sh (Mipc*);
   static void mem_swl (Mipc*);
   static void mem_sw (Mipc*);
   static void mem_swr (Mipc*);
};


// Emulated system call interface

class MipcSysCall : public SysCall {
public:

   MipcSysCall (Mipc *ms) {

      char buf[1024];
      m = ms->_mem;
      _ms = ms;
      _num_load = 0;
      _num_store = 0;
   };

   ~MipcSysCall () { };

   LL GetDWord (LL addr);
   void SetDWord (LL addr, LL data);

   Word GetWord (LL addr);
   void SetWord (LL addr, Word data);
  
   void SetReg (int reg, LL val);
   LL GetReg (int reg);
   LL GetTime (void);

private:

   Mipc *_ms;
};
#endif /* __MIPS_H__ */
