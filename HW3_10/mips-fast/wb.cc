#include "wb.h"

Writeback::Writeback (Mipc *mc)
{
   _mc = mc;
}

Writeback::~Writeback (void) {}

void
Writeback::MainLoop (void)
{
   unsigned int ins;
   Bool writeReg;
   Bool writeFReg;
   Bool loWPort;
   Bool hiWPort;
   Bool isSyscall;
   Bool isIllegalOp;
   Bool memControl;
   unsigned decodedDST,decodedSRC1,decodedSRC2,subregOperand,decodedShiftAmt,memory_addr_reg;
   unsigned opResultLo, opResultHi;
   unsigned int pc;
   signed int branchOffset;
   void (*opControl)(Mipc*, unsigned);
   void (*memOp)(Mipc*);
   while (1) {
      AWAIT_P_PHI0;	// @posedge
      // Sample the important signals
      if (_mc->_memValid) {
        ins = _mc->_ins[MEM_WB];
        pc = _mc->_pc[MEM_WB];
        isSyscall = _mc->_isSyscall[MEM_WB];
        decodedSRC1 = _mc->_decodedSRC1_p[MEM_WB];
        decodedSRC2 = _mc->_decodedSRC2_p[MEM_WB];
        decodedDST = _mc->_decodedDST_p[MEM_WB];
        subregOperand = _mc->_subregOperand_p[MEM_WB];
        memControl = _mc->_memControl_p[MEM_WB];
        writeReg = _mc->_writeREG_p[MEM_WB];
        writeFReg = _mc->_writeFREG_p[MEM_WB];
        branchOffset = _mc->_branchOffset_p[MEM_WB];
        hiWPort = _mc->_hiWPort_p[MEM_WB];
        loWPort = _mc->_loWPort_p[MEM_WB];
        decodedShiftAmt = _mc->_decodedShiftAmt_p[MEM_WB];
        isIllegalOp = _mc->_isIllegalOp_p[MEM_WB];
        opControl = _mc->_opControl_p[MEM_WB];
        memOp = _mc->_memOp_p[MEM_WB];
        opResultHi = _mc->_opResultHi_p[MEM_WB];
        opResultLo = _mc->_opResultLo_p[MEM_WB];
        memory_addr_reg = _mc->_memory_addr_reg_p[MEM_WB];

         //AWAIT_P_PHI1;       // @negedge
         if (isSyscall) {
#ifdef MIPC_DEBUG
            //fprintf(_mc->_debugLog, "<%llu> SYSCALL! Trapping to emulation layer at PC %#x\n", SIM_TIME, _mc->_pc);
            fprintf(_mc->_debugLog, "<%llu> SYSCALL! Trapping to emulation layer at PC %#x\n", SIM_TIME, pc);
#endif      
            _mc->_opControl_p[MEM_WB](_mc, ins);
            _mc->_pc_fetch = pc + 4;
            _mc->_stall_syscall = FALSE;
         }
         else if (isIllegalOp) {
            //printf("Illegal ins %#x at PC %#x. Terminating simulation!\n", ins, _mc->_pc);
            printf("Illegal ins %#x at PC %#x. Terminating simulation!\n", ins, pc);
#ifdef MIPC_DEBUG
            fclose(_mc->_debugLog);
#endif
            printf("Register state on termination:\n\n");
            _mc->dumpregs();
            exit(0);
         }
         else {
            if (writeReg) {
               _mc->_gpr[decodedDST] = opResultLo;
#ifdef MIPC_DEBUG
               fprintf(_mc->_debugLog, "<%llu> Writing to reg %u, value: %#x\n", SIM_TIME, decodedDST, opResultLo);
#endif
            }
            else if (writeFReg) {
               _mc->_fpr[(decodedDST)>>1].l[FP_TWIDDLE^((decodedDST)&1)] = opResultLo;
#ifdef MIPC_DEBUG
               fprintf(_mc->_debugLog, "<%llu> Writing to freg %u, value: %#x\n", SIM_TIME, decodedDST>>1, opResultLo);
#endif
            }
            else if (loWPort || hiWPort) {
               if (loWPort) {
                  _mc->_lo = opResultLo;
#ifdef MIPC_DEBUG
                  fprintf(_mc->_debugLog, "<%llu> Writing to Lo, value: %#x\n", SIM_TIME, opResultLo);
#endif
               }
               if (hiWPort) {
                  _mc->_hi = opResultHi;
#ifdef MIPC_DEBUG
                  fprintf(_mc->_debugLog, "<%llu> Writing to Hi, value: %#x\n", SIM_TIME, opResultHi);
#endif
               }
            }
         }
         _mc->_gpr[0] = 0;
         //_mc->_memValid = FALSE;
         _mc->_insDone = TRUE;
         AWAIT_P_PHI1;       // @negedge
	
      }
      else {
         PAUSE(1);
      }
   }
}
