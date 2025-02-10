using namespace std;
#include <iostream>
#include <fstream>
#include "pin.H"

ofstream OutFile;

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 icount = 0;static bool analyze=false;
static UINT64 fast_forward_count=0;

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
VOID Analysis(){

}
VOID PredicatedAnalysis(){
    
}
void MyExitRoutine(...) {
	// Do an exit system call to exit the application.
	// As we are calling the exit system call PIN would not be able to instrument application end.
	// Because of this, even if you are instrumenting the application end, the Fini function would not
	// be called. Thus you should report the statistics here, before doing the exit system call.

	// Results etc
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

    // Register Instruction to be called to instrument instructions
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
