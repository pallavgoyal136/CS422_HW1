using namespace std;
#include <iostream>
#include <fstream>
#include <vector>
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
#define mask512 (UINT64)0x1FF
#define mask1024 (UINT64)0x3FF
#define mask128 (UINT64)0x7F
#define masktag (UINT64)0x1FFFFFF
#define shifttag 32
#define shiftvalid 59
#define shiftlru 57
#define masklru (UINT64)0x3
#define hmask (UINT64)0x1FFFFFFFFFFFFFF
#define masktarget (UINT64)0xFFFFFFFF
#define numways 4
#define numsets 128 
bool FNBT;
bool bimodal;
bool sag;
bool sag2;
bool sag3;
bool sag4;
bool gag;
bool gag2;
bool gag3;
bool gag4;
bool gshare;
bool gshare2;
bool gshare3;
bool gshare4;
bool sag_gag_hybrid;
bool sag_gag_hybrid2;
bool sag_gag_gshare_hybrid_majority;
bool sag_gag_gshare_hybrid_tournament;
bool analyze=false;
INT8 bimodal_pht[bimodal_pht_height];
INT16 sag_bht[sag_bht_height];
INT8 sag_pht[sag_pht_height];
INT16 sag_bht2[sag_bht_height];
INT8 sag_pht2[sag_pht_height];
INT16 sag_bht3[sag_bht_height];
INT8 sag_pht3[sag_pht_height];
INT16 sag_bht4[sag_bht_height];
INT8 sag_pht4[sag_pht_height];
INT16 ghr;
INT8 gag_pht[gag_pht_height];
INT8 gshare_pht[gshare_pht_height];
INT8 gag_pht2[gag_pht_height];
INT8 gshare_pht2[gshare_pht_height];
INT8 gag_pht3[gag_pht_height];
INT8 gshare_pht3[gshare_pht_height];
INT8 gag_pht4[gag_pht_height];
INT8 gshare_pht4[gshare_pht_height];
INT8 sag_gag_hybrid_pht[sag_gag_hybrid_height];
INT8 gshare_sag_hybrid_pht[gshare_sag_hybrid_height];
INT8 gshare_gag_hybrid_pht[gshare_gag_hybrid_height];
INT8 sag_gag_hybrid_pht2[sag_gag_hybrid_height];
INT8 gshare_sag_hybrid_pht2[gshare_sag_hybrid_height];
INT8 gshare_gag_hybrid_pht2[gshare_gag_hybrid_height];
UINT64 BTB_PC[numsets][numways];
UINT64 BTB_H1[numsets][numways];
UINT64 BTB_H2[numsets][numways];
UINT64 icount=0;
UINT64 funcount=0;
UINT64 fast_forward_count;
UINT64 forward_branches=0;
UINT64 forward_FNBT=0;
UINT64 forward_bimodal=0;
UINT64 forward_sag=0;
UINT64 forward_sag2=0;
UINT64 forward_sag3=0;
UINT64 forward_sag4=0;
UINT64 forward_gag=0;
UINT64 forward_gag2=0;
UINT64 forward_gag3=0;
UINT64 forward_gag4=0;
UINT64 forward_gshare=0;
UINT64 forward_gshare2=0;
UINT64 forward_gshare3=0;
UINT64 forward_gshare4=0;
UINT64 forward_sag_gag_hybrid=0;
UINT64 forward_sag_gag_hybrid2=0;
UINT64 forward_sag_gag_gshare_hybrid_majority=0;
UINT64 forward_sag_gag_gshare_hybrid_tournament=0;
UINT64 backward_branches=0;
UINT64 backward_FNBT=0;
UINT64 backward_bimodal=0;
UINT64 backward_sag=0;
UINT64 backward_sag2=0;
UINT64 backward_sag3=0;
UINT64 backward_sag4=0;
UINT64 backward_gag=0;
UINT64 backward_gag2=0;
UINT64 backward_gag3=0;
UINT64 backward_gag4=0;
UINT64 backward_gshare=0;
UINT64 backward_gshare2=0;
UINT64 backward_gshare3=0;
UINT64 backward_gshare4=0;
UINT64 backward_sag_gag_hybrid=0;
UINT64 backward_sag_gag_hybrid2=0;
UINT64 backward_sag_gag_gshare_hybrid_majority=0;
UINT64 backward_sag_gag_gshare_hybrid_tournament=0;
UINT64 miss_BTB_PC=0;
UINT64 mispred_BTB_PC=0; 
UINT64 miss_BTB_H=0;
UINT64 mispred_BTB_H=0; 
UINT64 control_flow=0;
UINT64 countnottaken=0;
std::chrono::time_point<std::chrono::system_clock> startTime;
std::chrono::time_point<std::chrono::system_clock> endTime;
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
VOID predict_control_flow_ins_fin(UINT32 taken, ADDRINT pc, ADDRINT nextpc, ADDRINT target)
{
    if(!taken)
     countnottaken++;
    UINT64 pc64 = (UINT64)(pc);
    UINT64 nextpc64 = (UINT64)(nextpc);
    UINT64 target64 = (UINT64)(target);

    UINT64 index = pc64 & mask128;
    UINT64 tag = (pc64 >> 7) & masktag;
    UINT64 pred;
    UINT64 hit = 0, way = 0;
    UINT64 curr;

   
    for (UINT64 i = 0; i < numways; i++)
    {
        UINT64 btb_entry = BTB_PC[index][i];
        UINT64 btb_tag = (btb_entry >> shifttag) & masktag;
        UINT64 valid = (btb_entry >> shiftvalid) & 1;

        if (btb_tag == tag && valid)
        {
            hit = 1;
            way = i;
            curr = (btb_entry >> shiftlru) & masklru;
            break;
        }
    }

    if (hit == 0)
    {
        pred = nextpc64;
        miss_BTB_PC++;
    }
    else
    {
        pred = BTB_PC[index][way] & masktarget;
    }

    if (pred != (taken?target:nextpc))
    {
        mispred_BTB_PC++;
    }

    if (hit == 0)
    {
       
        for (UINT64 i = 0; i < numways; i++)
        {
            UINT64 btb_entry = BTB_PC[index][i];
            UINT64 valid = (btb_entry >> shiftvalid) & 1;
            UINT64 lru = (btb_entry >> shiftlru) & masklru;

            if (valid == 0 || lru == 3)
            {
                BTB_PC[index][i] = (tag << shifttag) | (1ULL << shiftvalid) | (target64 & masktarget);
                way = i;
                break;
            }
        }

        
        for (UINT64 i = 0; i < numways; i++)
        {
            if (i != way)
            {
                UINT64 btb_entry = BTB_PC[index][i];
                UINT64 valid = (btb_entry >> shiftvalid) & 1;

                if (valid)
                {
                    UINT64 lru = (btb_entry >> shiftlru) & masklru;
                    lru = (lru + 1) & masklru;
                    BTB_PC[index][i] = (btb_entry & ~(masklru << shiftlru)) | (lru << shiftlru);
                }
            }
        }
    }
    else
    {
        
        for (UINT64 i = 0; i < numways; i++)
        {
            UINT64 btb_entry = BTB_PC[index][i];
            UINT64 valid = (btb_entry >> shiftvalid) & 1;

            if (i == way)
            {
                BTB_PC[index][i] = (tag << shifttag) | (1ULL << shiftvalid) | (target64 & masktarget);
            }
            else if (valid)
            {
                UINT64 lru = (btb_entry >> shiftlru) & masklru;
                if (lru < curr)
                {
                    lru = (lru + 1) & masklru;
                    BTB_PC[index][i] = (btb_entry & ~(masklru << shiftlru)) | (lru << shiftlru);
                }
            }
        }
    }

    control_flow++;
}
VOID predict_control_flow_ins2(UINT32 taken, ADDRINT pc, ADDRINT nextpc, ADDRINT target)
{
    UINT64 index =  (pc&mask128)^(ghr&mask128);
    UINT64 tag = pc;
    UINT64 pred;
    UINT64 hit=0, way=0;
    UINT64 curr;
    for(UINT64 i=0;i<4;i++)
    {
        if(((BTB_H2[index][i])&masktarget) == tag && BTB_H2[index][i]>>34==1)
        {
            hit=1;
            way=i;
            curr=(BTB_H2[index][i]>>32)&masklru;
        }
    }
    if(hit==0){
        pred=nextpc;
        miss_BTB_H++;
    }
    else
        pred=BTB_H1[index][way]&masktarget;
    if(pred!=(taken?target:nextpc))
        mispred_BTB_H++;
    if(hit==0)
    {
        for(UINT64 i=0;i<4;i++)
        {
            if(BTB_H2[index][i]>>34==0 || (((BTB_H2[index][i]>>32)&masklru)==3))
            {
                BTB_H2[index][i]=tag;
                BTB_H2[index][i]=BTB_H2[index][i]|(1ULL<<34);
                BTB_H1[index][i]=target;
                way=i;
                break;
            }
        }
        for(UINT64 i=0;i<4;i++)
        {
            if(i!=way && BTB_H2[index][i]>>34==1)
            {
                BTB_H2[index][i]=(((BTB_H2[index][i]>>32)+1)<<32)|(BTB_H2[index][i]&masktarget);
            }
        }
    }
    else
    {
        for(UINT32 i=0;i<4;i++)
        {
            if(i!=way && ((BTB_H2[index][i]>>32)&masklru)<curr && BTB_H2[index][i]>>34==1)
            {
                BTB_H2[index][i]=(((BTB_H2[index][i]>>32)+1)<<32)|(BTB_H2[index][i]&masktarget);
            }
            else if(i==way)
            {
                BTB_H2[index][i]=tag;
                BTB_H2[index][i]=BTB_H2[index][i]|(1ULL<<34);
                BTB_H1[index][i]=target;   
            }
        }
    }
 //   control_flow++;
}
VOID predict_unconditional_branch(ADDRINT pc, ADDRINT target){
    FNBT=(pc>target);
    bimodal=(bimodal_pht[pc&mask512]>=1);
    sag=(sag_pht[sag_bht[pc&mask1024]]>=1);
    sag2=(sag_pht2[sag_bht2[pc&mask1024]]>=1);
    sag3=(sag_pht3[sag_bht3[pc&mask1024]]>=1);
    sag4=(sag_pht4[sag_bht4[pc&mask1024]]>=1);
    gag=(gag_pht[ghr]>=4);
    gag2=(gag_pht2[ghr]>=4);
    gag3=(gag_pht3[ghr]>=4);
    gag4=(gag_pht4[ghr]>=4);
    gshare=(gshare_pht[(pc&mask512)^ghr]>=4);
    gshare2=(gshare_pht2[(pc&mask512)^ghr]>=4);
    gshare3=(gshare_pht3[(pc&mask512)^ghr]>=4);
    gshare4=(gshare_pht4[(pc&mask512)^ghr]>=4);
    sag_gag_hybrid=((sag_gag_hybrid_pht[ghr]>=1) ? sag2: gag2);
    sag_gag_hybrid2=((sag_gag_hybrid_pht2[ghr]>=1) ? sag4: gag4);
    INT8 majority=0;
    majority+=(sag3?1:0);majority+=(gag3?1:0);majority+=(gshare3?1:0);
    sag_gag_gshare_hybrid_majority=(majority>=2);
    if(sag_gag_hybrid_pht2[ghr]>=1){
        sag_gag_gshare_hybrid_tournament=((gshare_sag_hybrid_pht2[ghr]>=1)?gshare4:sag4);
    }
    else{
        sag_gag_gshare_hybrid_tournament=((gshare_gag_hybrid_pht2[ghr]>=1)?gshare4:gag4);
    }
    if(FNBT){
        backward_branches++;
    }
    else{
        forward_branches++;
    }
    return; 
}
VOID update_fall_through(ADDRINT pc){
    if(FNBT){
        backward_FNBT++;
        backward_bimodal+=bimodal;
        backward_sag+=sag;
        backward_sag2+=sag2;
        backward_sag3+=sag3;
        backward_sag4+=sag4;
        backward_gag+=gag;
        backward_gag2+=gag2;
        backward_gag3+=gag3;
        backward_gag4+=gag4;
        backward_gshare+=gshare;
        backward_gshare2+=gshare2;
        backward_gshare3+=gshare3;
        backward_gshare4+=gshare4;
        backward_sag_gag_hybrid+=sag_gag_hybrid;
        backward_sag_gag_hybrid2+=sag_gag_hybrid2;
        backward_sag_gag_gshare_hybrid_majority+=sag_gag_gshare_hybrid_majority;
        backward_sag_gag_gshare_hybrid_tournament+=sag_gag_gshare_hybrid_tournament;
    }
    else{
        forward_bimodal+=bimodal;
        forward_sag += sag;
        forward_sag2 += sag2;
        forward_sag3 += sag3;
        forward_sag4 += sag4;
        forward_gag += gag;
        forward_gag2 += gag2;
        forward_gag3 += gag3;
        forward_gag4 += gag4;
        forward_gshare += gshare;
        forward_gshare2 += gshare2;
        forward_gshare3 += gshare3;
        forward_gshare4 += gshare4;
        forward_sag_gag_hybrid += sag_gag_hybrid;
        forward_sag_gag_hybrid2 += sag_gag_hybrid2;
        forward_sag_gag_gshare_hybrid_majority += sag_gag_gshare_hybrid_majority;
        forward_sag_gag_gshare_hybrid_tournament += sag_gag_gshare_hybrid_tournament;
    }
    bimodal_pht[pc&mask512]--;
    sag_pht[sag_bht[pc&mask1024]]--;
    sag_pht2[sag_bht2[pc&mask1024]]--;
    sag_pht3[sag_bht3[pc&mask1024]]--;
    sag_pht4[sag_bht4[pc&mask1024]]--;
    gag_pht[ghr]--;
    gag_pht2[ghr]--;
    gag_pht3[ghr]--;
    gag_pht4[ghr]--;
    gshare_pht[(pc&mask512)^ghr]--;
    gshare_pht2[(pc&mask512)^ghr]--;
    gshare_pht3[(pc&mask512)^ghr]--;
    gshare_pht4[(pc&mask512)^ghr]--;
    if(sag2&&(!gag2)) sag_gag_hybrid_pht[ghr]--;
    else if((!sag2)&&gag2) sag_gag_hybrid_pht[ghr]++;
    if(sag4&&(!gag4)) sag_gag_hybrid_pht2[ghr]--;
    else if((!sag4)&&gag4) sag_gag_hybrid_pht2[ghr]++;
    if(sag2&&(!gshare2)) gshare_sag_hybrid_pht[ghr]++;
    else if((!sag2)&&gshare2) gshare_sag_hybrid_pht[ghr]--;
    if(sag4&&(!gshare4)) gshare_sag_hybrid_pht2[ghr]++;
    else if((!sag4)&&gshare4) gshare_sag_hybrid_pht2[ghr]--;
    if(gag2&&(!gshare2)) gshare_gag_hybrid_pht[ghr]++;
    else if((!gag2)&&gshare2) gshare_gag_hybrid_pht[ghr]--;
    if(gag4&&(!gshare4)) gshare_gag_hybrid_pht2[ghr]++;
    else if((!gag4)&&gshare4) gshare_gag_hybrid_pht2[ghr]--;
    bimodal_pht[pc&mask512]=(bimodal_pht[pc&mask512]<(0))?(0):bimodal_pht[pc&mask512];
    sag_pht[sag_bht[pc&mask1024]]=(sag_pht[sag_bht[pc&mask1024]]<(0))?(0):sag_pht[sag_bht[pc&mask1024]];
    sag_pht2[sag_bht2[pc&mask1024]]=(sag_pht2[sag_bht2[pc&mask1024]]<(0))?(0):sag_pht2[sag_bht2[pc&mask1024]];
    sag_pht3[sag_bht3[pc&mask1024]]=(sag_pht3[sag_bht3[pc&mask1024]]<(0))?(0):sag_pht3[sag_bht3[pc&mask1024]];
    sag_pht4[sag_bht4[pc&mask1024]]=(sag_pht4[sag_bht4[pc&mask1024]]<(0))?(0):sag_pht4[sag_bht4[pc&mask1024]];
    gag_pht[ghr]=(gag_pht[ghr]<(0))?(0):gag_pht[ghr];
    gag_pht2[ghr]=(gag_pht2[ghr]<(0))?(0):gag_pht2[ghr];
    gag_pht3[ghr]=(gag_pht3[ghr]<(0))?(0):gag_pht3[ghr];
    gag_pht4[ghr]=(gag_pht4[ghr]<(0))?(0):gag_pht4[ghr];
    gshare_pht[(pc&mask512)^ghr]=(gshare_pht[(pc&mask512)^ghr]<(0))?(0):gshare_pht[(pc&mask512)^ghr];
    gshare_pht2[(pc&mask512)^ghr]=(gshare_pht2[(pc&mask512)^ghr]<(0))?(0):gshare_pht2[(pc&mask512)^ghr];
    gshare_pht3[(pc&mask512)^ghr]=(gshare_pht3[(pc&mask512)^ghr]<(0))?(0):gshare_pht3[(pc&mask512)^ghr];
    gshare_pht4[(pc&mask512)^ghr]=(gshare_pht4[(pc&mask512)^ghr]<(0))?(0):gshare_pht4[(pc&mask512)^ghr];
    sag_gag_hybrid_pht[ghr]=(sag_gag_hybrid_pht[ghr]<(0))?(0):sag_gag_hybrid_pht[ghr];
    sag_gag_hybrid_pht2[ghr]=(sag_gag_hybrid_pht2[ghr]<(0))?(0):sag_gag_hybrid_pht2[ghr];
    gshare_sag_hybrid_pht[ghr]=(gshare_sag_hybrid_pht[ghr]<(0))?(0):gshare_sag_hybrid_pht[ghr];
    gshare_sag_hybrid_pht2[ghr]=(gshare_sag_hybrid_pht2[ghr]<(0))?(0):gshare_sag_hybrid_pht2[ghr];
    gshare_gag_hybrid_pht[ghr]=(gshare_gag_hybrid_pht[ghr]<(0))?(0):gshare_gag_hybrid_pht[ghr];
    gshare_gag_hybrid_pht2[ghr]=(gshare_gag_hybrid_pht2[ghr]<(0))?(0):gshare_gag_hybrid_pht2[ghr];
    sag_gag_hybrid_pht[ghr]=(sag_gag_hybrid_pht[ghr]>3)?3:sag_gag_hybrid_pht[ghr];
    sag_gag_hybrid_pht2[ghr]=(sag_gag_hybrid_pht2[ghr]>3)?3:sag_gag_hybrid_pht2[ghr];
    gshare_sag_hybrid_pht[ghr]=(gshare_sag_hybrid_pht[ghr]>3)?3:gshare_sag_hybrid_pht[ghr];
    gshare_sag_hybrid_pht2[ghr]=(gshare_sag_hybrid_pht2[ghr]>3)?3:gshare_sag_hybrid_pht2[ghr];
    gshare_gag_hybrid_pht[ghr]=(gshare_gag_hybrid_pht[ghr]>3)?3:gshare_gag_hybrid_pht[ghr];
    gshare_gag_hybrid_pht2[ghr]=(gshare_gag_hybrid_pht2[ghr]>3)?3:gshare_gag_hybrid_pht2[ghr];
    sag_bht[pc&mask1024]=(sag_bht[pc&mask1024]<<1);
    sag_bht2[pc&mask1024]=(sag_bht2[pc&mask1024]<<1);
    sag_bht3[pc&mask1024]=(sag_bht3[pc&mask1024]<<1);
    sag_bht4[pc&mask1024]=(sag_bht4[pc&mask1024]<<1);
    sag_bht[pc&mask1024]&=mask512;
    sag_bht2[pc&mask1024]&=mask512;
    sag_bht3[pc&mask1024]&=mask512;
    sag_bht4[pc&mask1024]&=mask512;
    ghr=(ghr<<1);
    ghr&=mask512;
    return;
}
VOID update_taken_branch(ADDRINT pc){
    if(FNBT){
        backward_bimodal+=(bimodal==false);
        backward_sag += (sag == false);
        backward_sag2 += (sag2 == false);
        backward_sag3 += (sag3 == false);
        backward_sag4 += (sag4 == false);
        backward_gag += (gag == false);
        backward_gag2 += (gag2 == false);
        backward_gag3 += (gag3 == false);
        backward_gag4 += (gag4 == false);
        backward_gshare += (gshare == false);
        backward_gshare2 += (gshare2 == false);
        backward_gshare3 += (gshare3 == false);
        backward_gshare4 += (gshare4 == false);
        backward_sag_gag_hybrid += (sag_gag_hybrid == false);
        backward_sag_gag_hybrid2 += (sag_gag_hybrid2 == false);
        backward_sag_gag_gshare_hybrid_majority += (sag_gag_gshare_hybrid_majority == false);
        backward_sag_gag_gshare_hybrid_tournament += (sag_gag_gshare_hybrid_tournament == false);
    }
    else{
        forward_FNBT++;
        forward_bimodal += (bimodal == false);
        forward_sag += (sag == false);
        forward_sag2 += (sag2 == false);
        forward_sag3 += (sag3 == false);
        forward_sag4 += (sag4 == false);
        forward_gag += (gag == false);
        forward_gag2 += (gag2 == false);
        forward_gag3 += (gag3 == false);
        forward_gag4 += (gag4 == false);
        forward_gshare += (gshare == false);
        forward_gshare2 += (gshare2 == false);
        forward_gshare3 += (gshare3 == false);
        forward_gshare4 += (gshare4 == false);
        forward_sag_gag_hybrid += (sag_gag_hybrid == false);
        forward_sag_gag_hybrid2 += (sag_gag_hybrid2 == false);
        forward_sag_gag_gshare_hybrid_majority += (sag_gag_gshare_hybrid_majority == false);
        forward_sag_gag_gshare_hybrid_tournament += (sag_gag_gshare_hybrid_tournament == false);
    }
    bimodal_pht[pc&mask512]++;
    sag_pht[sag_bht[pc&mask1024]]++;
    sag_pht2[sag_bht2[pc&mask1024]]++;
    sag_pht3[sag_bht3[pc&mask1024]]++;
    sag_pht4[sag_bht4[pc&mask1024]]++;
    gag_pht[ghr]++;
    gag_pht2[ghr]++;
    gag_pht3[ghr]++;
    gag_pht4[ghr]++;
    gshare_pht[(pc&mask512)^ghr]++;
    gshare_pht2[(pc&mask512)^ghr]++;
    gshare_pht3[(pc&mask512)^ghr]++;
    gshare_pht4[(pc&mask512)^ghr]++;
    if(sag2&&(!gag2)) sag_gag_hybrid_pht[ghr]++;
    else if((!sag2)&&gag2) sag_gag_hybrid_pht[ghr]--;
    if(sag4&&(!gag4)) sag_gag_hybrid_pht2[ghr]++;
    else if((!sag4)&&gag4) sag_gag_hybrid_pht2[ghr]--;
    if(sag2&&(!gshare2)) gshare_sag_hybrid_pht[ghr]--;
    else if((!sag2)&&gshare2) gshare_sag_hybrid_pht[ghr]++;
    if(sag4&&(!gshare4)) gshare_sag_hybrid_pht2[ghr]--;
    else if((!sag4)&&gshare4) gshare_sag_hybrid_pht2[ghr]++;
    if(gag2&&(!gshare2)) gshare_gag_hybrid_pht[ghr]--;
    else if((!gag2)&&gshare2) gshare_gag_hybrid_pht[ghr]++;
    if(gag4&&(!gshare4)) gshare_gag_hybrid_pht2[ghr]--;
    else if((!gag4)&&gshare4) gshare_gag_hybrid_pht2[ghr]++;
    bimodal_pht[pc&mask512]=(bimodal_pht[pc&mask512]>3)?3:bimodal_pht[pc&mask512];
    sag_pht[sag_bht[pc&mask1024]]=(sag_pht[sag_bht[pc&mask1024]]>3)?3:sag_pht[sag_bht[pc&mask1024]];
    sag_pht2[sag_bht2[pc&mask1024]]=(sag_pht2[sag_bht2[pc&mask1024]]>3)?3:sag_pht2[sag_bht2[pc&mask1024]];
    sag_pht3[sag_bht3[pc&mask1024]]=(sag_pht3[sag_bht3[pc&mask1024]]>3)?3:sag_pht3[sag_bht3[pc&mask1024]];
    sag_pht4[sag_bht4[pc&mask1024]]=(sag_pht4[sag_bht4[pc&mask1024]]>3)?3:sag_pht4[sag_bht4[pc&mask1024]];
    gag_pht[ghr]=(gag_pht[ghr]>7)?7:gag_pht[ghr];
    gag_pht2[ghr]=(gag_pht2[ghr]>7)?7:gag_pht2[ghr];
    gag_pht3[ghr]=(gag_pht3[ghr]>7)?7:gag_pht3[ghr];
    gag_pht4[ghr]=(gag_pht4[ghr]>7)?7:gag_pht4[ghr];
    gshare_pht[(pc&mask512)^ghr]=(gshare_pht[(pc&mask512)^ghr]>7)?7:gshare_pht[(pc&mask512)^ghr];
    gshare_pht2[(pc&mask512)^ghr]=(gshare_pht2[(pc&mask512)^ghr]>7)?7:gshare_pht2[(pc&mask512)^ghr];
    gshare_pht3[(pc&mask512)^ghr]=(gshare_pht3[(pc&mask512)^ghr]>7)?7:gshare_pht3[(pc&mask512)^ghr];
    gshare_pht4[(pc&mask512)^ghr]=(gshare_pht4[(pc&mask512)^ghr]>7)?7:gshare_pht4[(pc&mask512)^ghr];
    sag_gag_hybrid_pht[ghr]=(sag_gag_hybrid_pht[ghr]<(0))?(0):sag_gag_hybrid_pht[ghr];
    sag_gag_hybrid_pht2[ghr]=(sag_gag_hybrid_pht2[ghr]<(0))?(0):sag_gag_hybrid_pht2[ghr];
    gshare_sag_hybrid_pht[ghr]=(gshare_sag_hybrid_pht[ghr]<(0))?(0):gshare_sag_hybrid_pht[ghr];
    gshare_sag_hybrid_pht2[ghr]=(gshare_sag_hybrid_pht2[ghr]<(0))?(0):gshare_sag_hybrid_pht2[ghr];
    gshare_gag_hybrid_pht[ghr]=(gshare_gag_hybrid_pht[ghr]<(0))?(0):gshare_gag_hybrid_pht[ghr];
    gshare_gag_hybrid_pht2[ghr]=(gshare_gag_hybrid_pht2[ghr]<(0))?(0):gshare_gag_hybrid_pht2[ghr];
    sag_gag_hybrid_pht[ghr]=(sag_gag_hybrid_pht[ghr]>3)?3:sag_gag_hybrid_pht[ghr];
    sag_gag_hybrid_pht2[ghr]=(sag_gag_hybrid_pht2[ghr]>3)?3:sag_gag_hybrid_pht2[ghr];
    gshare_sag_hybrid_pht[ghr]=(gshare_sag_hybrid_pht[ghr]>3)?3:gshare_sag_hybrid_pht[ghr];
    gshare_sag_hybrid_pht2[ghr]=(gshare_sag_hybrid_pht2[ghr]>3)?3:gshare_sag_hybrid_pht2[ghr];
    gshare_gag_hybrid_pht[ghr]=(gshare_gag_hybrid_pht[ghr]>3)?3:gshare_gag_hybrid_pht[ghr];
    gshare_gag_hybrid_pht2[ghr]=(gshare_gag_hybrid_pht2[ghr]>3)?3:gshare_gag_hybrid_pht2[ghr];
    sag_bht[pc&mask1024]=(sag_bht[pc&mask1024]<<1);
    sag_bht2[pc&mask1024]=(sag_bht2[pc&mask1024]<<1);
    sag_bht3[pc&mask1024]=(sag_bht3[pc&mask1024]<<1);
    sag_bht4[pc&mask1024]=(sag_bht4[pc&mask1024]<<1);
    sag_bht[pc&mask1024]|=1;
    sag_bht2[pc&mask1024]|=1;
    sag_bht3[pc&mask1024]|=1;
    sag_bht4[pc&mask1024]|=1;
    sag_bht[pc&mask1024]&=mask512;
    sag_bht2[pc&mask1024]&=mask512;
    sag_bht3[pc&mask1024]&=mask512;
    sag_bht4[pc&mask1024]&=mask512;
    ghr=(ghr<<1);
    ghr|=1;
    ghr&=mask512;
    return;
}

void MyExitRoutine() {
    UINT64 total_branches = forward_branches + backward_branches;
    OutFile<<"===============================================\n";
    OutFile<<"Direction Predictors\n";
    OutFile<<"FNBT : Accesses "<<total_branches<<", Mispredictions "<<forward_FNBT+backward_FNBT<<" ("<<(double)(forward_FNBT+backward_FNBT)/total_branches<<"), Forward branches "<<forward_branches<<", Forward mispredictions "<<forward_FNBT<<" ("<<(double)forward_FNBT/forward_branches<<"), Backward branches "<<backward_branches<<", Backward mispredictions "<<backward_FNBT<<" ("<<(double)backward_FNBT/backward_branches<<")\n";
    OutFile<<"Bimodal : Accesses "<<total_branches<<", Mispredictions "<<forward_bimodal+backward_bimodal<<" ("<<(double)(forward_bimodal+backward_bimodal)/total_branches<<"), Forward branches "<<forward_branches<<", Forward mispredictions "<<forward_bimodal<<" ("<<(double)forward_bimodal/forward_branches<<"), Backward branches "<<backward_branches<<", Backward mispredictions "<<backward_bimodal<<" ("<<(double)backward_bimodal/backward_branches<<")\n";
    OutFile<<"SAg : Accesses "<<total_branches<<", Mispredictions "<<forward_sag+backward_sag<<" ("<<(double)(forward_sag+backward_sag)/total_branches<<"), Forward branches "<<forward_branches<<", Forward mispredictions "<<forward_sag<<" ("<<(double)forward_sag/forward_branches<<"), Backward branches "<<backward_branches<<", Backward mispredictions "<<backward_sag<<" ("<<(double)backward_sag/backward_branches<<")\n";
    OutFile<<"GAg : Accesses "<<total_branches<<", Mispredictions "<<forward_gag+backward_gag<<" ("<<(double)(forward_gag+backward_gag)/total_branches<<"), Forward branches "<<forward_branches<<", Forward mispredictions "<<forward_gag<<" ("<<(double)forward_gag/forward_branches<<"), Backward branches "<<backward_branches<<", Backward mispredictions "<<backward_gag<<" ("<<(double)backward_gag/backward_branches<<")\n";
    OutFile<<"GShare : Accesses "<<total_branches<<", Mispredictions "<<forward_gshare+backward_gshare<<" ("<<(double)(forward_gshare+backward_gshare)/total_branches<<"), Forward branches "<<forward_branches<<", Forward mispredictions "<<forward_gshare<<" ("<<(double)forward_gshare/forward_branches<<"), Backward branches "<<backward_branches<<", Backward mispredictions "<<backward_gshare<<" ("<<(double)backward_gshare/backward_branches<<")\n";
    OutFile<<"SAg-GAg Hybrid : Accesses "<<total_branches<<", Mispredictions "<<forward_sag_gag_hybrid+backward_sag_gag_hybrid<<" ("<<(double)(forward_sag_gag_hybrid+backward_sag_gag_hybrid)/total_branches<<"), Forward branches "<<forward_branches<<", Forward mispredictions "<<forward_sag_gag_hybrid<<" ("<<(double)forward_sag_gag_hybrid/forward_branches<<"), Backward branches "<<backward_branches<<", Backward mispredictions "<<backward_sag_gag_hybrid<<" ("<<(double)backward_sag_gag_hybrid/backward_branches<<")\n";
    OutFile<<"SAg-GAg-GShare Hybrid Majority : Accesses "<<total_branches<<", Mispredictions "<<forward_sag_gag_gshare_hybrid_majority+backward_sag_gag_gshare_hybrid_majority<<" ("<<(double)(forward_sag_gag_gshare_hybrid_majority+backward_sag_gag_gshare_hybrid_majority)/total_branches<<"), Forward branches "<<forward_branches<<", Forward mispredictions "<<forward_sag_gag_gshare_hybrid_majority<<" ("<<(double)forward_sag_gag_gshare_hybrid_majority/forward_branches<<"), Backward branches "<<backward_branches<<", Backward mispredictions "<<backward_sag_gag_gshare_hybrid_majority<<" ("<<(double)backward_sag_gag_gshare_hybrid_majority/backward_branches<<")\n";
    OutFile<<"SAg-GAg-GShare Hybrid Tournament : Accesses "<<total_branches<<", Mispredictions "<<forward_sag_gag_gshare_hybrid_tournament+backward_sag_gag_gshare_hybrid_tournament<<" ("<<(double)(forward_sag_gag_gshare_hybrid_tournament+backward_sag_gag_gshare_hybrid_tournament)/total_branches<<"), Forward branches "<<forward_branches<<", Forward mispredictions "<<forward_sag_gag_gshare_hybrid_tournament<<" ("<<(double)forward_sag_gag_gshare_hybrid_tournament/forward_branches<<"), Backward branches "<<backward_branches<<", Backward mispredictions "<<backward_sag_gag_gshare_hybrid_tournament<<" ("<<(double)backward_sag_gag_gshare_hybrid_tournament/backward_branches<<")\n";
    OutFile<<"\n";
    OutFile<<"Branch Target Predictors\n";
    OutFile<<"BTB1 : Accesses "<<control_flow<<", Mispredictions "<<mispred_BTB_PC<<" ("<<(double)mispred_BTB_PC/control_flow<<"), Misses "<<miss_BTB_PC<<" ("<<(double)miss_BTB_PC/control_flow<<")\n";
    OutFile<<"BTB2 : Accesses "<<control_flow<<", Mispredictions "<<mispred_BTB_H<<" ("<<(double)mispred_BTB_H/control_flow<<"), Misses "<<miss_BTB_H<<" ("<<(double)miss_BTB_H/control_flow<<")\n";
    OutFile<<"not taken btb:"<<countnottaken<<endl;
    OutFile<<"number of valid fall through:"<<funcount<<endl;
    OutFile<<"===============================================\n";
    endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = endTime - startTime;
    OutFile << "Time in minutes: " << elapsed_seconds.count()/60 << "m\n";
    OutFile.close();
    exit(0);
}
VOID Trace(TRACE trace, VOID *v){
    for( BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl) ){
        BBL_InsertIfCall(bbl, IPOINT_BEFORE, (AFUNPTR)Terminate, IARG_END);
        BBL_InsertThenCall(bbl, IPOINT_BEFORE, (AFUNPTR)MyExitRoutine, IARG_END);
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)CheckFastForward, IARG_END);
        for( INS ins= BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins) ){
            if(INS_Category(ins) == XED_CATEGORY_COND_BR){
                INS_InsertIfCall(ins,IPOINT_BEFORE, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenCall(ins,IPOINT_BEFORE,(AFUNPTR)predict_unconditional_branch,IARG_ADDRINT,(ADDRINT)INS_Address(ins),IARG_BRANCH_TARGET_ADDR,IARG_END);
                INS_InsertIfCall(ins,IPOINT_AFTER, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenCall(ins,IPOINT_AFTER,(AFUNPTR)update_fall_through,IARG_ADDRINT,(ADDRINT)INS_Address(ins),IARG_END);
                INS_InsertIfCall(ins,IPOINT_TAKEN_BRANCH, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenCall(ins,IPOINT_TAKEN_BRANCH,(AFUNPTR)update_taken_branch,IARG_ADDRINT,(ADDRINT)INS_Address(ins),IARG_END);
            }
            if(INS_IsIndirectControlFlow(ins))
            {
  	      if(INS_Valid(INS_Next(ins))){
		funcount++;
                INS_InsertIfCall(ins,IPOINT_AFTER, (AFUNPTR)FastForward,IARG_END);
                INS_InsertThenCall(ins,IPOINT_AFTER, (AFUNPTR)predict_control_flow_ins_fin, IARG_UINT32, 0,IARG_ADDRINT,(ADDRINT)INS_Address(ins),IARG_ADDRINT, (ADDRINT)INS_NextAddress(ins),IARG_BRANCH_TARGET_ADDR,IARG_END);
                INS_InsertIfCall(ins,IPOINT_AFTER, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenCall(ins,IPOINT_AFTER,(AFUNPTR)predict_control_flow_ins2,IARG_UINT32, 0, IARG_ADDRINT,(ADDRINT)INS_Address(ins),IARG_ADDRINT, (ADDRINT)INS_NextAddress(ins),IARG_BRANCH_TARGET_ADDR,IARG_END);
		}
		INS_InsertIfCall(ins,IPOINT_TAKEN_BRANCH, (AFUNPTR)FastForward,IARG_END);
                INS_InsertThenCall(ins,IPOINT_TAKEN_BRANCH, (AFUNPTR)predict_control_flow_ins_fin, IARG_UINT32, 1,IARG_ADDRINT,(ADDRINT)INS_Address(ins),IARG_ADDRINT, (ADDRINT)INS_NextAddress(ins),IARG_BRANCH_TARGET_ADDR,IARG_END);
                INS_InsertIfCall(ins,IPOINT_TAKEN_BRANCH, (AFUNPTR)FastForward, IARG_END);
                INS_InsertThenCall(ins,IPOINT_TAKEN_BRANCH,  (AFUNPTR)predict_control_flow_ins2, IARG_UINT32, 1,IARG_ADDRINT,(ADDRINT)INS_Address(ins),IARG_ADDRINT, (ADDRINT)INS_NextAddress(ins),IARG_BRANCH_TARGET_ADDR,IARG_END);
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
    OutFile.setf(ios::showbase);
    OutFile << "Count " << icount << endl;
    OutFile.close();
}
INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}
int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();
    for(int i=0;i<bimodal_pht_height;i++) bimodal_pht[i]=0;
    for(int i=0;i<sag_bht_height;i++) sag_bht[i]=0;
    for(int i=0;i<sag_pht_height;i++) sag_pht[i]=0;
    for(int i=0;i<gag_pht_height;i++) gag_pht[i]=0;
    for(int i=0;i<gshare_pht_height;i++) gshare_pht[i]=0;
    for(int i=0;i<sag_gag_hybrid_height;i++) sag_gag_hybrid_pht[i]=0;
    for(int i=0;i<gshare_sag_hybrid_height;i++) gshare_sag_hybrid_pht[i]=0;
    for(int i=0;i<gshare_gag_hybrid_height;i++) gshare_gag_hybrid_pht[i]=0;
    for(int i=0;i<numsets;i++){
        for(int j=0;j<numways;j++){
            BTB_PC[i][j]=0;
            BTB_H1[i][j]=0;
            BTB_H2[i][j]=0;
        }
    }
    ghr=0;
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
