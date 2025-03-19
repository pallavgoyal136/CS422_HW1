using namespace std;
#include <iostream>
#include <fstream>
#include "pin.H"
ofstream OutFile;
#define bimodal_pht_height 512
#define sag_bht_height 1024
#define sag_pht_height 512
#define gag_pht_height 512
#define gshare_pht_height 512
#define sag_gag_hybrid_height 512
#define gshare_sag_hybrid_height 512
#define gshare_gag_hybrid_height 512
#define mask512 0x1FF
#define mask1024 0x3FF
bool FNBT;
bool bimodal;
bool sag;
bool gag;
bool gshare;
bool sag_gag_hybrid;
bool sag_gag_gshare_hybrid_majority;
bool sag_gag_gshare_hybrid_tournament;
bool analyze=false;
INT8 bimodal_pht[bimodal_pht_height];
INT16 sag_bht[sag_bht_height];
INT8 sag_pht[sag_pht_height];
INT16 ghr;
INT8 gag_pht[gag_pht_height];
INT8 gshare_pht[gshare_pht_height];
INT8 sag_gag_hybrid_pht[sag_gag_hybrid_height];
INT8 gshare_sag_hybrid_pht[gshare_sag_hybrid_height];
INT8 gshare_gag_hybrid_pht[gshare_gag_hybrid_height];
UINT64 icount;
VOID docount(UINT32 c) { icount += c;}
ADDRINT Terminate(void){
    return (icount >= fast_forward_count + 1000000000);
}
ADDRINT CheckFastForward (void) {
    return analyze=((icount >= fast_forward_count) && (icount < fast_forward_count + 1000000000));
}
ADDRINT FastForward (void) {
    return analyze;
}
VOID predict_unconditional_branch(ADDRINT pc){
    bimodal=(bimodal_pht[pc&mask512]>=0);
    sag=(sag_pht[sag_bht[pc&mask1024]]>=0);
    gag=(gag_pht[ghr]>=0);
    gshare=(gshare_pht[(pc&mask512)^ghr]);
    sag_gag_hybrid=((sag_gag_hybrid_pht[ghr]>=0) ? sag: gag);
    INT8 majority=0;
    majority+=(sag?1:0);majority+=(gag?1:0);majority+=(gshare?1:0);
    sag_gag_gshare_hybrid_majority=(majority>=2);
    if(sag_gag_hybrid_pht[ghr]>=0){
        sag_gag_gshare_hybrid_tournament=((gshare_sag_hybrid_pht[ghr]>=0)?gshare:sag);
    }
    else{
        sag_gag_gshare_hybrid_tournament=((gshare_gag_hybrid_pht[ghr]>=0)?gshare:gag);
    }
    return; 
}
void MyExitRoutine() {

}
VOID Trace(TRACE trace, VOID *v){
    for( BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl) ){
        BBL_InsertIfCall(bbl, IPOINT_BEFORE, (AFUNPTR)Terminate, IARG_END);
        BBL_InsertThenCall(bbl, IPOINT_BEFORE, (AFUNPTR)MyExitRoutine, IARG_END);
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)CheckFastForward, IARG_END);
        for( INS ins= BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins) ){
            if(INS_Category(ins) == XED_CATEGORY_COND_BR){
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertTnenCall(ins,IPOINT_BEFORE,(AFUNPTR)predict_unconditional_branch,IARG_ADDRINT,(ADDRINT)INS_Address(ins),IARG_END);
            }
        }
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)docount, IARG_UINT32, BBL_NumIns(bbl), IARG_END);
    }
}
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "sampler.out", "specify output file name");
KNOB<UINT64> KnobForwardCount(KNOB_MODE_WRITEONCE, "pintool",
    "f", "100000000", "specify fast forward count");
VOID Fini(INT32 code, VOID *v){

}
int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());
    fast_forward_count = KnobForwardCount.Value();
    fast_forward_count = fast_forward_count * 1000000000ULL;
    startTime = std::chrono::system_clock::now();
    // Register Instruction to be called to instrument instructions
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
