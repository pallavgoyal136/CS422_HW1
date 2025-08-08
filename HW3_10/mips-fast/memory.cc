#include "memory.h"

Memory::Memory (Mipc *mc)
{
   _mc = mc;
}

Memory::~Memory (void) {}

void
Memory::MainLoop (void)
{
   Bool memControl;
   Bool isSyscall;
   unsigned int ins;
   unsigned int pc;
   Bool isIllegalOp;
   signed int   _decodedSRC1, _decodedSRC2;     // Reg fetch output (source values)
   unsigned     _decodedDST;                    // Decoder output (dest reg no)
   unsigned     _subregOperand;                 // Needed for lwl and lwr
   Bool         _writeREG, _writeFREG;          // WB control
   signed int   _branchOffset;
   Bool         _hiWPort, _loWPort;             // WB control
   unsigned     _decodedShiftAmt;               // Shift amount
   void (*_opControl)(Mipc*, unsigned);
   void (*_memOp)(Mipc*);
   unsigned     _opResultHi, _opResultLo;       // Result of operation
   unsigned     _memory_addr_reg;
   while (1) {
      AWAIT_P_PHI0;	// @posedge
      if (_mc->_execValid) {
        memControl = _mc->_memControl_p[EX_MEM];
        ins = _mc->_ins[EX_MEM];
        pc = _mc->_pc[EX_MEM];
        isSyscall = _mc->_isSyscall[EX_MEM];
        _decodedSRC1 = _mc->_decodedSRC1_p[EX_MEM];
        _decodedSRC2 = _mc->_decodedSRC2_p[EX_MEM];
        _decodedDST = _mc->_decodedDST_p[EX_MEM];
        _subregOperand = _mc->_subregOperand_p[EX_MEM];
        _writeREG = _mc->_writeREG_p[EX_MEM];
        _writeFREG = _mc->_writeFREG_p[EX_MEM];
        _branchOffset = _mc->_branchOffset_p[EX_MEM];
        _hiWPort = _mc->_hiWPort_p[EX_MEM];
        _loWPort = _mc->_loWPort_p[EX_MEM];
        _decodedShiftAmt = _mc->_decodedShiftAmt_p[EX_MEM];
        isIllegalOp = _mc->_isIllegalOp_p[EX_MEM];
        _opControl = _mc->_opControl_p[EX_MEM];
        _memOp = _mc->_memOp_p[EX_MEM];
        _opResultHi = _mc->_opResultHi_p[EX_MEM];
        _opResultLo = _mc->_opResultLo_p[EX_MEM];
        _memory_addr_reg = _mc->_memory_addr_reg_p[EX_MEM];
        _mc->_opResultLo_mem_wb_wr = _opResultLo;
        _mc->_memory_addr_reg_mem_wb = _memory_addr_reg;
         _mc->_subregOperand_mem_wb = _subregOperand;
         _mc->_decodedDST_mem_wb = _decodedDST;
         AWAIT_P_PHI1;       // @negedge
         if (memControl) {
            _memOp(_mc);
#ifdef MIPC_DEBUG
            //fprintf(_mc->_debugLog, "<%llu> Accessing memory at address %#x for ins %#x\n", SIM_TIME, _mc->_memory_addr_reg, _mc->_ins);
            fprintf(_mc->_debugLog, "<%llu> Accessing memory at address %#x for ins %#x\n", SIM_TIME, _memory_addr_reg, ins);
#endif
         }
         else {
#ifdef MIPC_DEBUG
            //fprintf(_mc->_debugLog, "<%llu> Memory has nothing to do for ins %#x\n", SIM_TIME, _mc->_ins);
            fprintf(_mc->_debugLog, "<%llu> Memory has nothing to do for ins %#x\n", SIM_TIME, ins);
#endif
         }
         //AWAIT_P_PHI1;       // @negedge
         //_mc->_execValid = FALSE;
         _mc->_memValid = TRUE;
	 _mc->_ins[MEM_WB] = ins;
	 _mc->_pc[MEM_WB] = pc;
        _mc->_isSyscall[MEM_WB] = isSyscall;
        _mc->_decodedSRC1_p[MEM_WB] = _decodedSRC1;
        _mc->_decodedSRC2_p[MEM_WB] = _decodedSRC2;
        _mc->_decodedDST_p[MEM_WB] = _decodedDST;
        _mc->_subregOperand_p[MEM_WB] = _subregOperand;
        _mc->_memControl_p[MEM_WB] = memControl;
        _mc->_writeREG_p[MEM_WB] = _writeREG;
        _mc->_writeFREG_p[MEM_WB] = _writeFREG;
        _mc->_branchOffset_p[MEM_WB] = _branchOffset;
        _mc->_hiWPort_p[MEM_WB] = _hiWPort;
        _mc->_loWPort_p[MEM_WB] = _loWPort;
        _mc->_decodedShiftAmt_p[MEM_WB] = _decodedShiftAmt;
        _mc->_isIllegalOp_p[MEM_WB] = isIllegalOp;
        _mc->_opControl_p[MEM_WB] = _opControl;
        _mc->_memOp_p[MEM_WB] = _memOp;
        _mc->_opResultHi_p[MEM_WB] = _opResultHi;
        _mc->_opResultLo_p[MEM_WB] = _mc->_opResultLo_mem_wb_wr;
        _mc->_memory_addr_reg_p[MEM_WB] = _memory_addr_reg;


      }
      else {
         PAUSE(1);
      }
   }
}
