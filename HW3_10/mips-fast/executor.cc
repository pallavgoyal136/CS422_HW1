#include "executor.h"

Exe::Exe (Mipc *mc)
{
   _mc = mc;
}

Exe::~Exe (void) {}

void
Exe::MainLoop (void)
{
   unsigned int ins;
   unsigned int pc;
   Bool isSyscall, isIllegalOp;
   signed int   _decodedSRC1, _decodedSRC2;     // Reg fetch output (source values)
   unsigned     _decodedDST;                    // Decoder output (dest reg no)
   unsigned     _subregOperand;                 // Needed for lwl and lwr
   Bool         _memControl;                    // Memory instruction?
   Bool         _writeREG, _writeFREG;          // WB control
   signed int   _branchOffset;
   Bool         _hiWPort, _loWPort;             // WB control
   unsigned     _decodedShiftAmt;               // Shift amount
   void (*_opControl)(Mipc*, unsigned);
   void (*_memOp)(Mipc*);
   int _bdslot;                           // 1 if the next ins is delay slot
   unsigned int _btgt;
   Bypass_information bypass_info; 
   while (1) {
      AWAIT_P_PHI0;	// @posedge
      if (_mc->_decodeValid) {        
        bypass_info = _mc->_bypass_p[ID_EX];
        _mc->_hi_EX = _mc->_hi;
         _mc->_lo_EX = _mc->_lo;
        if(bypass_info.SRC1Bypass == MEM_EX){
            if(bypass_info.isSRC1FP){
               if(!_memControl){
                  _mc->_decodedSRC1_p[ID_EX] = _mc->_opResultLo_p[MEM_WB]; //this is for mfc1 getting bypass from lwc1 or mtc1 and is the src value fpr

               }
               // whem mem control is on instruction is swc1 and in the implementation the instruction only passes the register number and when it is trying to read the value in mem stage already earlier instructions would have complete their WB thus swc1 would get its correct value
            }
            else{
               _mc->_decodedSRC1_p[ID_EX] = _mc->_opResultLo_p[MEM_WB];
               if(bypass_info.isJR){
                  _mc->_btgt_p[ID_EX] = _mc->_decodedSRC1_p[ID_EX];
               }  
            }

        }
        if(bypass_info.SRC1Bypass == EX_EX){
            if(bypass_info.isSRC1FP){
               if(!_memControl){
                  _mc->_decodedSRC1_p[ID_EX] = _mc->_opResultLo_p[EX_MEM]; //this is for mfc1 getting bypass from mtc1

               } 
               // whem mem control is on instruction is swc1 and in the implementation the instruction only passes the register number and when it is trying to read the value in mem stage already earlier instructions would have complete their WB thus swc1 would get its correct value         
            }
            else{
               _mc->_decodedSRC1_p[ID_EX] = _mc->_opResultLo_p[EX_MEM];
               if(bypass_info.isJR){
                  _mc->_btgt_p[ID_EX] = _mc->_decodedSRC1_p[ID_EX];
               } 
            }
        }
        if(bypass_info.SRC2Bypass == MEM_EX){
            if(bypass_info.isSRC2FP){
            }
            else{
               if(!bypass_info.isSRC2STORE) _mc->_decodedSRC2_p[ID_EX] = _mc->_opResultLo_p[MEM_WB]; 
            }
        }
        if(bypass_info.SRC2Bypass == EX_EX){
            if(bypass_info.isSRC2FP){
            }
            else{
               if(!bypass_info.isSRC2STORE) _mc->_decodedSRC2_p[ID_EX] = _mc->_opResultLo_p[EX_MEM]; 
            }
        }
        
        if(bypass_info.HiBypass == MEM_EX){
            _mc->_hi_EX = _mc->_opResultHi_p[MEM_WB];
        }
        if(bypass_info.HiBypass == EX_EX){
            _mc->_hi_EX = _mc->_opResultHi_p[EX_MEM];
        }
        if(bypass_info.LoBypass == MEM_EX){
            _mc->_lo_EX = _mc->_opResultLo_p[MEM_WB];
        }
         if(bypass_info.LoBypass == EX_EX){
            _mc->_lo_EX = _mc->_opResultLo_p[EX_MEM];
         }
         ins = _mc->_ins[ID_EX];
        pc = _mc->_pc[ID_EX];
        isSyscall = _mc->_isSyscall[ID_EX];
        isIllegalOp = _mc->_isIllegalOp_p[ID_EX];
        _decodedSRC1 = _mc->_decodedSRC1_p[ID_EX];
        _decodedSRC2 = _mc->_decodedSRC2_p[ID_EX];
        _decodedDST = _mc->_decodedDST_p[ID_EX];
        _subregOperand = _mc->_subregOperand_p[ID_EX];
        _memControl = _mc->_memControl_p[ID_EX];
        _writeREG = _mc->_writeREG_p[ID_EX];
        _writeFREG = _mc->_writeFREG_p[ID_EX];
        _branchOffset = _mc->_branchOffset_p[ID_EX];
        _hiWPort = _mc->_hiWPort_p[ID_EX];
        _loWPort = _mc->_loWPort_p[ID_EX];
        _decodedShiftAmt = _mc->_decodedShiftAmt_p[ID_EX];
        _opControl = _mc->_opControl_p[ID_EX];
        _memOp = _mc->_memOp_p[ID_EX];
        _btgt = _mc->_btgt_p[ID_EX];
        _bdslot = _mc->_bdslot_p[ID_EX];



         //AWAIT_P_PHI1;	// @negedge
         if (!isSyscall && !isIllegalOp) {
            //_mc->_opControl(_mc,ins);
            _mc->_opControl_p[ID_EX](_mc,ins);
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Executed ins %#x\n", SIM_TIME, ins);
#endif
         }
         else if (isSyscall) {
	    _mc->_stall[IF_ID] = TRUE;
	    _mc->_stall[ID_EX] = TRUE;
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Deferring execution of syscall ins %#x\n", SIM_TIME, ins);
#endif
         }
         else {
#ifdef MIPC_DEBUG
            //fprintf(_mc->_debugLog, "<%llu> Illegal ins %#x in execution stage at PC %#x\n", SIM_TIME, ins, _mc->_pc);
            fprintf(_mc->_debugLog, "<%llu> Illegal ins %#x in execution stage at PC %#x\n", SIM_TIME, ins, pc);
#endif
         }
         //_mc->_decodeValid = FALSE;
         _mc->_execValid = TRUE;

         if (!isIllegalOp && !isSyscall) {
            //if (_mc->_lastbdslot && _mc->_btaken)
            if (_bdslot && _mc->_btaken_ex_mem_wr)
            {
               //_mc->_pc = _mc->_btgt;
               _mc->_pc_fetch = _btgt;
            }
            else
            {
               //_mc->_pc = _mc->_pc + 4;
            }
            //_mc->_lastbdslot = _mc->_bdslot;
         }
         //if(_bdslot) _mc->_stall_branch=TRUE;
         AWAIT_P_PHI1;	// @negedge
        _mc->_ins[EX_MEM] = ins;
        _mc->_pc[EX_MEM] = pc;
        _mc->_isSyscall[EX_MEM] = isSyscall;
        _mc->_decodedSRC1_p[EX_MEM] = _decodedSRC1;
        _mc->_decodedSRC2_p[EX_MEM] = _decodedSRC2;
        _mc->_decodedDST_p[EX_MEM] = _decodedDST;
        _mc->_subregOperand_p[EX_MEM] = _subregOperand;
        _mc->_memControl_p[EX_MEM] = _memControl;
        _mc->_writeREG_p[EX_MEM] = _writeREG;
        _mc->_writeFREG_p[EX_MEM] = _writeFREG;
        _mc->_branchOffset_p[EX_MEM] = _branchOffset;
        _mc->_hiWPort_p[EX_MEM] = _hiWPort;
        _mc->_loWPort_p[EX_MEM] = _loWPort;
        _mc->_decodedShiftAmt_p[EX_MEM] = _decodedShiftAmt;
        _mc->_isIllegalOp_p[EX_MEM] = isIllegalOp;
        _mc->_opControl_p[EX_MEM] = _opControl;
        _mc->_memOp_p[EX_MEM] = _memOp;
        _mc->_opResultHi_p[EX_MEM] = _mc->_opResultHi_ex_mem_wr;
        _mc->_opResultLo_p[EX_MEM] = _mc->_opResultLo_ex_mem_wr;
        _mc->_memory_addr_reg_p[EX_MEM] = _mc->_memory_addr_reg_ex_mem_wr;
	_mc->_bdslot_p[EX_MEM] = _bdslot;
	_mc->_btgt_p[EX_MEM] = _btgt;
	_mc->_btaken_p[EX_MEM] = _mc->_btaken_ex_mem_wr;
      }
      else {
         PAUSE(1);
      }
   }
}
