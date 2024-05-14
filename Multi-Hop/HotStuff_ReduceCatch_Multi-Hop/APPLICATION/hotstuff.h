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
extern u8 NTX_proposal;

extern u8 Extera_slot;

#define PROPOSAL 1

#define PREPREPARE 2
#define PREPARE 3
#define PRECOMMIT 4
#define COMMIT 5
#define DECIDE 6
#define VIEW_CHANGE 7
#define NEXT_ROUND 8
//#define ACK 9
#define CATCHING 9

#define Sig_Size 48
#define MAX_PROPOSAL_PACKETS 30
#define SINGLE_COMMAND_SIZE 32  //单位byte，目前是用hash代替的command

#define Data_channel 66
#define Msg_channel 12

//Global
#define MAX_ACK_NUMBER 5
#define Global_f 1
#define GROUPS_NUMBER 4
#define TOTAL_GROUPS 5
#define MAX_GROUP_NUMBER 20

#define GLOBAL_PROPOSAL_1 10
#define GLOBAL_SIG 11
#define GLOBAL_PROPOSAL_2 12

//#define GLOBAL_CHOICE 9
//#define GLOBAL_COMMIT 10


#define GLOBAL_PREPREPARE 13
#define GLOBAL_PREPARE 14
#define GLOBAL_PRECOMMIT 15
#define GLOBAL_COMMIT 16
#define GLOBAL_DECIDE 17
#define GLOBAL_VIEW_CHANGE 18
#define GLOBAL_NEXT_ROUND 19
#define GLOBAL_READY 20
#define GLOBAL_ACK 21
#define GLOBAL_CONSENSUS_START 22
#define GLOBAL_CATCHING 23

extern u8 Group_Number;
extern u8 Group_Leader_ID;
//extern u8 Global_Ready;

extern u8 Global_Commands_Size[TOTAL_GROUPS];							//区块内指令数
extern u8 Global_Proposal_Packet_Number[TOTAL_GROUPS];					// = Command_Size / 4 上取整
extern u8 Global_Rece_Proposal[TOTAL_GROUPS][MAX_PROPOSAL_PACKETS];	//跟随者收到提案数据包
//extern u8 Global_Rece_Proposal_flag[TOTAL_GROUPS];						//标记是否集满所有数据包 0 没有  1 有
extern u8 Global_Rece_Proposal_Number[TOTAL_GROUPS]; 					//收到提案数据包总数
extern u8 Global_Command_buff[TOTAL_GROUPS][800];						//指令缓冲区
extern u8 Global_Proposal_hash[TOTAL_GROUPS][40];						//提案哈希

extern u8 Global_Sig_Number[TOTAL_GROUPS];
extern u8 Global_IDS[TOTAL_GROUPS][MAX_GROUP_NUMBER];					//存第几组 有哪几个节点的signature
extern u8 Global_Sig[TOTAL_GROUPS][MAX_GROUP_NUMBER][Sig_Size];			//存第几组节点具体的signatures

extern u8 Global_Rece_Sig[TOTAL_GROUPS][MAX_GROUP_NUMBER];
extern u8 Global_Rece_Sig_Number[TOTAL_GROUPS];

//extern u8 Global_Choice[16+1];
//extern u8 Global_Commit[16+1];
extern u8 Global_ACK_Proposal[16+1];
//extern u8 Global_ACK_Choice[16+1];
//extern u8 Global_ACK_Commit[16+1];
extern u8 Global_ACK_Proposal_Number;
//extern u8 Global_ACK_Choice_Number;
//extern u8 Global_ACK_Commit_Number;
//extern u8 Global_Choice_Truth_Set_Number;
//extern u8 Global_Choice_False_Set_Number;
//extern u8 Global_Commit_Truth_Set_Number;
//extern u8 Global_Commit_False_Set_Number;

extern u8 Global_ACK_TO_Send_Number;
extern u8 Global_ACK_TO_Send[100][30];
extern u8 Global_Proposal_1_Number;			//自己收到了几个
extern u8 Global_Proposal_1_ACK_Number;
extern u8 Global_Proposal_1_ACK[TOTAL_GROUPS];	//谁收到了我的
extern u8 Global_Ready[TOTAL_GROUPS]; //谁Ready了
extern u8 start_flag2;
extern u8 Global_Ready_Number;
extern u8 Global_Consensus_Leader;
extern u8 Proposal_2[40];
extern u8 Proposal_2_hash[40];
extern u8 Rece_Proposal_2_flag;
extern u8 Rece_Global_Proposal_1[30];
//Global_hotstuff
extern u8 Global_Prepare_Number;
extern u8 Global_Precommit_Number;
extern u8 Global_Commit_Number;
extern u8 Global_Viewchange_Number;

extern u8 Global_ACK_prepare;
extern u8 Global_ACK_precommit;
extern u8 Global_ACK_commit;
extern u8 Global_ACK_viewchange;

extern u8 Global_TIM_Prepare_Flag;
extern u8 Global_TIM_Precommit_Flag;
extern u8 Global_TIM_Commit_Flag;
extern u8 Global_TIM_Decide_Flag;

extern u8 Global_Rece_precommit;
extern u8 Global_Rece_commit;
extern u8 Global_Rece_decide;
extern u8 Global_Rece_next;
//id = 1
extern u8 Global_Rece_Prepare[20];
extern u8 Global_Prepare_id[20];
extern u8 Global_Prepare_Sig[20][64];

extern u8 Global_Rece_PreCommit[20];
extern u8 Global_PreCommit_id[20];
extern u8 Global_PreCommit_Sig[20][64];

extern u8 Global_Rece_Commit[20];
extern u8 Global_Commit_id[20];
extern u8 Global_Commit_Sig[20][64];

extern u8 Global_Rece_Viewchange[20];
extern u8 Global_Viewchange_id[20];
extern u8 Global_Viewchange_Sig[20][64];
extern u8 Timer_Flag_5;

//leader
extern u8 Need_proposal_2;
extern u8 Global_Need_precommit;
extern u8 Global_Need_commit;
extern u8 Global_Need_decide;
extern u8 Global_Need_next;
//viewchange
extern u8 Global_Need_PREPARE;
extern u8 Global_Need_PRECOMMIT;
extern u8 Global_Need_COMMIT;
extern u8 Global_Need_VIEWCHANGE;
extern u8 Global_NTX;
extern u8 Global_NTX_proposal;

extern u8 NTX_proposal_flag;
extern u8 Global_NTX_proposal_flag;
//throughput
extern u8 Commands_Size;
extern u8 Proposal_Packet_Number;// = Command_Size / 4 上取整
extern u8 Rece_Proposal[MAX_PROPOSAL_PACKETS];
extern u8 Rece_Proposal_flag;
extern u8 Rece_Proposal_Number;
extern u8 Command_buff[800];
extern u8 Proposal_hash[40];
extern u8 start_flag;
extern u8 Timer_Flag_5;
extern u8 before_round;
extern u8 after_round;
extern u8 TIM_slot;
extern u32 TIM9_Exceed_Times;
extern u32 TIM10_Exceed_Times;
extern u32 Total_time;
extern u32 Total_time1;

/***********************************hotstuff***********************************/
extern u8 Prepare_Number;
extern u8 Precommit_Number;
extern u8 Commit_Number;
extern u8 Viewchange_Number;


extern u8 Rece_precommit;
extern u8 Rece_commit;
extern u8 Rece_decide;
extern u8 Rece_next;

extern u8 Need_proposal;
extern u8 Need_precommit;
extern u8 Need_commit;
extern u8 Need_decide;
extern u8 Need_next;

extern u8 Need_PREPREPARE;
extern u8 Need_PREPARE;
extern u8 Need_PRECOMMIT;
extern u8 Need_COMMIT;
extern u8 Need_VIEWCHANGE;

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
extern u16 TEMP_LEN;
/***********************************hotstuff***********************************/
extern u16 slot;

extern u8 id;	//id = 0 => leader           1-2-3 => follower 
extern u8 PBFT_status;
//extern u8 Choice_Truth_Set_Number;
//extern u8 Choice_False_Set_Number;

//extern u8 Commit_Truth_Set_Number;
//extern u8 Commit_False_Set_Number;

//extern u8 Choice[13+1];
//extern u8 Commit[13+1];

extern u8 Send_Data_buff[800];//发送数据缓冲区

extern u8 Rece_Data_buff[800];//接受数据缓冲区
extern u8 Tmp_Rece_Data_buff[30][800];

extern u8 Proposal[30];


////////ACK
//extern u8 ACK_Proposal[13+1];
//extern u8 ACK_Choice[13+1];
//extern u8 ACK_Commit[13+1];

//extern u8 ACK_Proposal_Number;
//extern u8 ACK_Choice_Number;
//extern u8 ACK_Commit_Number;

//extern u8 ACK_Proposal_Flag;
//extern u8 ACK_Choice_Flag;
//extern u8 ACK_Commit_Flag;

extern u8 ACK_TO_Send_Number;
extern u8 ACK_TO_Send[100][30];

//crypto
extern uint8_t public_key[16+1][64];
extern uint8_t private_key[16+1][32];
extern const struct uECC_Curve_t * curves[5];

//
extern u32 TIM2_Exceed_Times;
extern u32 TIM5_Exceed_Times;
extern u8 decided;

void hotstuff();

#endif
