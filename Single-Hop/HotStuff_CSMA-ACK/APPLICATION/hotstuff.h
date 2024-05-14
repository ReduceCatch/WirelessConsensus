#ifndef _PBFT_H_
#define _PBFT_H_

#include "sys.h"
#include "uECC.h"
#include "sha2.h"

//#define N 4
//#define f 1
extern u8 N;
extern u8 f;
extern u16 DONE_THRES;

#define PROPOSAL 1

#define PREPREPARE 2
#define PREPARE 3
#define PRECOMMIT 4
#define COMMIT 5
#define DECIDE 6
#define VIEW_CHANGE 7
#define NEXT_ROUND 8
#define ACK 9
#define Sig_Size 48
#define MAX_PROPOSAL_PACKETS 30
#define SINGLE_COMMAND_SIZE 32  //单位byte，目前是用hash代替的command

#define Data_channel 0
#define Msg_channel 27

//throughput
extern u8 Commands_Size;
extern u8 Proposal_Packet_Number;// = Command_Size / 4 上取整
extern u8 Rece_Proposal[MAX_PROPOSAL_PACKETS];
extern u8 Rece_Proposal_flag;
extern u8 Rece_Proposal_Number;
extern u8 Command_buff[800];
extern u8 Proposal_hash[40];
extern u8 start_flag;
extern u32 Total_time;
extern u32 Total_time1;
/***********************************hotstuff***********************************/
extern u8 Prepare_Number;
extern u8 Precommit_Number;
extern u8 Commit_Number;
extern u8 Viewchange_Number;

extern u8 ACK_prepare;
extern u8 ACK_precommit;
extern u8 ACK_commit;
extern u8 ACK_viewchange;

extern u8 TIM_Prepare_Flag;
extern u8 TIM_Precommit_Flag;
extern u8 TIM_Commit_Flag;
extern u8 TIM_Decide_Flag;

extern u8 Rece_precommit;
extern u8 Rece_commit;
extern u8 Rece_decide;
extern u8 Rece_next;

extern u8 Rece_Prepare[20];
extern u8 Prepare_id[20];
extern u8 Prepare_Sig[20][64];

extern u8 Rece_PreCommit[20];
extern u8 PreCommit_id[20];
extern u8 PreCommit_Sig[20][64];

extern u8 Rece_Commit[20];
extern u8 Commit_id[20];
extern u8 Commit_Sig[20][64];

extern u8 Rece_Viewchange[20];
extern u8 Viewchange_id[20];
extern u8 Viewchange_Sig[20][64];

extern u8 precommit_verify_number;

/***********************************hotstuff***********************************/
extern u16 slot;

extern u8 id;	//id = 0 => leader           1-2-3 => follower 
extern u8 HotStuff_status;
extern u8 Choice_Truth_Set_Number;
extern u8 Choice_False_Set_Number;

extern u8 Commit_Truth_Set_Number;
extern u8 Commit_False_Set_Number;

extern u8 Choice[13+1];
extern u8 Commit[13+1];

extern u8 Send_Data_buff[800];//发送数据缓冲区

extern u8 Proposal[30];



////////ACK
extern u8 ACK_Proposal[13+1];
extern u8 ACK_Choice[13+1];
extern u8 ACK_Commit[13+1];

extern u8 ACK_Proposal_Number;
extern u8 ACK_Choice_Number;
extern u8 ACK_Commit_Number;

extern u8 ACK_Proposal_Flag;
extern u8 ACK_Choice_Flag;
extern u8 ACK_Commit_Flag;

extern u8 ACK_TO_Send_Number;
extern u8 ACK_TO_Send[100][30];

//crypto
extern uint8_t public_key[13+1][64];
extern uint8_t private_key[13+1][32];
extern const struct uECC_Curve_t * curves[5];

//
extern u32 TIM2_Exceed_Times;
extern u32 TIM5_Exceed_Times;
extern u32 TIM10_Exceed_Times;
extern u8 decided;



void hotstuff();

#endif