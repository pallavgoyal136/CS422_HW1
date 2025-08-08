#include "decode.h"

Decode::Decode (Mipc *mc)
{
   _mc = mc;
}

Decode::~Decode (void) {}

void
Decode::MainLoop (void)
{
   unsigned int ins;
   unsigned int pc;
   Bypass_information bypass_info;
   while (1) {
      AWAIT_P_PHI0;	// @posedge
      if (_mc->_insValid) {
         ins = _mc->_ins[IF_ID];
	 pc = _mc->_pc[IF_ID];
	 _mc->_pc_if_id = pc;
    if(_mc->_stall_syscall) {
      ins = 0;
    }
    _mc->_stall_data = FALSE;
    bypass_info = _mc->getBypassInfo(ins);
    //if(bypass_info.SRC1Bypass == MEM_EX || bypass_info.SRC2Bypass == MEM_EX || bypass_info.HiBypass == MEM_EX || bypass_info.LoBypass == MEM_EX) bypass_info.isStall=TRUE;        
    //if(bypass_info.SRC1Bypass == EX_EX || bypass_info.SRC2Bypass == EX_EX ||  bypass_info.LoBypass == EX_EX || bypass_info.HiBypass == EX_EX ) bypass_info.isStall=TRUE;
    //if(bypass_info.SRC1Bypass == MEM_MEM || bypass_info.SRC2Bypass == MEM_MEM) bypass_info.isStall = TRUE;
    if(bypass_info.isStall) {
        bypass_info.SRC1Bypass = NO_BYPASS;
        bypass_info.SRC2Bypass = NO_BYPASS;
        bypass_info.SRC1Reg = NO_BYPASS;
        bypass_info.SRC2Reg = NO_BYPASS;
        bypass_info.HiBypass = NO_BYPASS;
        bypass_info.LoBypass = NO_BYPASS;
        bypass_info.isJR = FALSE;
        bypass_info.isSRC2STORE = FALSE;
        _mc->_stall_data = TRUE;
        ins = 0;
    }
         AWAIT_P_PHI1;	// @negedge
         _mc->Dec(ins);
#ifdef MIPC_DEBUG
         fprintf(_mc->_debugLog, "<%llu> Decoded ins %#x\n", SIM_TIME, ins);
#endif
         if(_mc->_isSyscall[ID_EX]) {
            _mc->_stall_syscall = TRUE;
         }
         //_mc->_insValid = FALSE;
         _mc->_decodeValid = TRUE;
	       _mc->_ins[ID_EX] = ins;
	      _mc->_pc[ID_EX] = pc;
         _mc->_bypass_p[ID_EX] = bypass_info;
      }
      else {
         PAUSE(1);
      }
   }
}
