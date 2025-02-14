using namespace std;
#include <iostream>
#include <fstream>
#include "pin.H"

ofstream OutFile;

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 icount = 0;static bool analyze=false;
static UINT64 fast_forward_count=0;
static UINT64 loads=0;
static UINT64 stores=0;
static UINT64 nops=0;
static UINT64 direct_calls=0;
static UINT64 indirect_calls=0;
static UINT64 returns=0;
static UINT64 unconditional_branches=0;
static UINT64 conditional_branches=0;
static UINT64 logical_operations=0;
static UINT64 rotate_and_shift_operations=0;
static UINT64 flag_operations=0;
static UINT64 vector_instructions=0;
static UINT64 conditional_moves=0;
static UINT64 mmx_and_sse_instructions=0;
static UINT64 system_calls=0;
static UINT64 floating_point_instructions=0;
static UINT64 others=0;

// This function is called before every instruction is executed
ADDRINT Terminate(void)
{
    return (icount >= fast_forward_count + 1,000,000,000);
}
VOID CheckFastForward (void) {
	return analyze=((icount >= fast_forward_count) && (icount < fast_forward_count + 1,000,000,000));
}
VOID FastForward (void) {
	return analyze;
}

// This function is called before every block
VOID docount(UINT32 c) { icount += c;}
VOID countloads() { loads ++;}
VOID countstores() { stores ++;}
VOID countnops() { nops ++;}
VOID countdirectcalls() { direct_calls ++;}
VOID countindirectcalls() { indirect_calls ++;}
VOID countreturns() { returns ++;}
VOID countunconditionalbranches() { unconditional_branches ++;}
VOID countconditionalbranches() { conditional_branches ++;}
VOID countlogicaloperations() { logical_operations ++;}
VOID countrotateandshiftoperations() { rotate_and_shift_operations ++;}
VOID countflagoperations() { flag_operations ++;}
VOID countvectorinstructions() { vector_instructions ++;}
VOID countconditionalmoves() { conditional_moves ++;}
VOID countmmxandsseinstructions() { mmx_and_sse_instructions ++;}
VOID countsystemcalls() { system_calls ++;}
VOID countfloatingpointinstructions() { floating_point_instructions ++;}
VOID countothers() { others ++;}
VOID Analysis(){

}
VOID PredicatedAnalysis(){
    
}
void MyExitRoutine() {
    OutFile << "===============================================\n";
    OutFile << "Instruction Type Results: \n";
    OutFile << "Loads: " << loads << endl;
    OutFile << "Stores: " << stores << endl;
    OutFile << "Nops: " << nops << endl;
    OutFile << "Direct Calls: " << direct_calls << endl;
    OutFile << "Indirect Calls: " << indirect_calls << endl;
    OutFile << "Returns: " << returns << endl;
    OutFile << "Unconditional Branches: " << unconditional_branches << endl;
    OutFile << "Conditional Branches: " << conditional_branches << endl;
    OutFile << "Logical Operations: " << logical_operations << endl;
    OutFile << "Rotate and Shift Operations: " << rotate_and_shift_operations << endl;
    OutFile << "Flag Operations: " << flag_operations << endl;
    OutFile << "Vector Instructions: " << vector_instructions << endl;
    OutFile << "Conditional Moves: " << conditional_moves << endl;
    OutFile << "MMX and SSE Instructions: " << mmx_and_sse_instructions << endl;
    OutFile << "System Calls: " << system_calls << endl;
    OutFile << "Floating Point Instructions: " << floating_point_instructions << endl;
    OutFile << "Others: " << others << endl;
    OutFile.close();
	exit(0);
}    
// Pin calls this function every time a new instruction is encountered
VOID Trace(TRACE trace, VOID *v)
{
    // Insert a call to docount before every instruction, no arguments are passed
    for( BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl) ){
        BBL_InsertIfCall(bbl, IPOINT_BEFORE, Terminate, IARG_END);
        BBL_InsertThenCall(bbl, IPOINT_BEFORE, MyExitRoutine, ..., IARG_END);
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)CheckFastForward, IARG_END);
        for( INS ins= BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins) ){
            UINT32 memOperands = INS_MemoryOperandCount(ins);
            for (UINT32 memOp = 0; memOp < memOperands; memOp++){
                if(INS_MemoryOperandIsRead(ins, memOp)){
                    INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                    INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countloads,IARG_END);
                }
                if(INS_MemoryOperandIsWritten(ins, memOp)){
                    INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                    INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countstores,IARG_END);
                }
            }
            if (INS_Category(ins) == XED_CATEGORY_NOP) {
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countnops,IARG_END);
            }
            else if(INS_category(ins) == XED_CATEGORY_CALL){
                if(INS_isDirectCall(ins)){
                    INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                    INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countdirectcalls,IARG_END);
                }
                else{
                    INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                    INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countindirectcalls,IARG_END);
                }
            }
            else if(INS_category(ins) == XED_CATEGORY_RET){
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countreturns,IARG_END);
            }
            else if(INS_category(ins) == XED_CATEGORY_UNCOND_BR){
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countunconditionalbranches,IARG_END);
            }
            else if(INS_category(ins) == XED_CATEGORY_COND_BR){
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countconditionalbranches,IARG_END);
            }
            else if(INS_category(ins) == XED_CATEGORY_LOGICAL){
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countlogicaloperations,IARG_END);
            }
            else if((INS_Category(ins) == XED_CATEGORY_ROTATE) || (INS_Category(ins) == XED_CATEGORY_SHIFT)){
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countrotateandshiftoperations,IARG_END);
            }
            else if(INS_category(ins) == XED_CATEGORY_FLAGOP){
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countflagoperations,IARG_END);
            }
            else if((INS_category(ins) == XED_CATEGORY_AVX)||(INS_category(ins) == XED_CATEGORY_AVX2) || (INS_category(ins) == XED_CATEGORY_AVX2GATHER) || (INS_category(ins) == XED_CATEGORY_AVX512)){
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countvectorinstructions,IARG_END);
            }
            else if(INS_category(ins) == XED_CATEGORY_CMOV){
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countconditionalmoves,IARG_END);
            }
            else if((INS_category(ins) == XED_CATEGORY_MMX) || (INS_category(ins) == XED_CATEGORY_SSE)){
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countmmxandsseinstructions,IARG_END);
            }
            else if(INS_category(ins) == XED_CATEGORY_SYSCALL){
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countsystemcalls,IARG_END);
            }
            else if(INS_category(ins) == XED_CATEGORY_X87_ALU){
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countfloatingpointinstructions,IARG_END);
            }
            else{
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)countothers,IARG_END);
            }
            INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
            INS_InsertThenCall(ins,IPOINT_BEFORE, (AFUNPTR)Analysis,IARG_ADDRINT,INS_Address(ins),IARG_END);
            INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
            INS_InsertThenPredicatedCall(ins,IPOINT_BEFORE, (AFUNPTR)PredicatedAnalysis, IARG_ADDRINT,INS_Address(ins),IARG_END);
        }

        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)docount, IARG_UINT32, BBL_NumIns(bbl), IARG_END);
    }


}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "sampler.out", "specify output file name");
KNOB<UINT64> KnobForwardCount(KNOB_MODE_WRITEONCE, "pintool",
    "f", "100000000", "specify fast forward count");
// Set static variable from KNOB

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << "Count " << icount << endl;
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());
    fast_forward_count = KnobForwardCount.Value();
    // Register Instruction to be called to instrument instructions
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
