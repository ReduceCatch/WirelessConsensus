#ifndef _PBFT_H_
#define _PBFT_H_

#include "sys.h"
#include "uECC.h"
#include "sha2.h"

//#define N 7
//#define f 2

extern u8 N;
extern u8 f;
extern u16 DONE_THRES;

#define PROPOSAL 1
#define CHOICE 2
#define COMMIT 3
#define ACK 4
#define ACK_VC 5
#define VIEW_CHANGE 6
#define RECE_ACK_VC 7
#define NEXT_ROUND 8
#define Sig_Size 48
#define DECIDE 9
#define CATCHING 10
#define MAX_PROPOSAL_PACKETS 30
#define SINGLE_COMMAND_SIZE 32  //单位byte，目前是用hash代替的command

#define Data_channel 0
#define Msg_channel 27

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
extern u32 TIM5_Exceed_Times;
extern u32 TIM9_Exceed_Times;
extern u32 TIM10_Exceed_Times;
extern u32 Total_time;
extern u32 Total_time1;

extern u8 Need_to_Wait;
extern u8 id;	//id = 0 => leader           1-2-3 => follower 
extern u8 PBFT_status;
extern u8 Choice_Truth_Set_Number;
extern u8 Choice_False_Set_Number;

extern u8 Commit_Truth_Set_Number;
extern u8 Commit_False_Set_Number;

extern u8 Choice[13+1];
extern u8 Commit[13+1];


extern u8 Send_Data_buff[800];//发送数据缓冲区

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

//extern u8 ACK_To_Send_Number;
//extern u8 ACK_To_Send[40][30];
extern u8 ACK_To_Send_Flag;
//extern u8 Rece_Proposal;
extern u8 ACK_To_Send_VC_Flag;

//crypto
extern uint8_t public_key[20][64];
extern uint8_t private_key[20][32];
extern const struct uECC_Curve_t * curves[5];


//viewchange
extern u8 Rece_next;
extern u8 ACK_viewchange;
extern u8 Viewchange_Number;
//存目标节点的2f+1个签名
extern u8 Rece_Viewchange[20];//目标节点目前收到了几个签名
extern u8 Viewchange_id[20];
extern u8 Viewchange_Sig[20][64];

extern u8 Need_Proposal_Flag;
extern u8 Need_Choice_Flag;
extern u8 Need_Commit_Flag;
extern u8 Need_VIEWCHANGE;
extern u8 Need_next;
//////////////////////////////


//
extern u32 TIM2_Exceed_Times;

void tendermint2();

#endif
