#ifndef _PBFT_H_
#define _PBFT_H_

#include "sys.h"
#include "uECC.h"
#include "sha2.h"

//#define N 4
//#define f 1
//#define DONE_THRES 0
extern u8 N;
extern u8 f;
extern u16 DONE_THRES;

#define PROPOSAL 1
#define CHOICE 2
#define COMMIT 3
#define ACK 4
#define NACK 5
#define Sig_Size 48
#define MAX_PROPOSAL_PACKETS 30
#define SINGLE_COMMAND_SIZE 32  //��λbyte��Ŀǰ����hash�����command


#define Data_channel 0
#define Msg_channel 27

extern u16 slot;

extern u8 id;	//id = 0 => leader           1-2-3 => follower 
extern u8 PBFT_status;
extern u8 Choice_Truth_Set_Number;
extern u8 Choice_False_Set_Number;

extern u8 Commit_Truth_Set_Number;
extern u8 Commit_False_Set_Number;

extern u8 Choice[13+1];
extern u8 Commit[13+1];

extern u8 Send_Data_buff[800];//�������ݻ�����



extern u8 Commands_Size;
extern u8 Proposal_Packet_Number;// = Command_Size / 4 ��ȡ��
extern u8 Rece_Proposal[MAX_PROPOSAL_PACKETS];
extern u8 Rece_Proposal_flag;
extern u8 Rece_Proposal_Number;
extern u8 Command_buff[800];
extern u8 Proposal_hash[40];
extern u8 before_round;
extern u8 after_round;
extern u8 TIM_slot;
extern u32 TIM9_Exceed_Times;
extern u32 TIM10_Exceed_Times;
//NACK
extern u8 Need_Choice_Flag;
extern u8 Need_Commit_Flag;
extern u8 Need_Proposal_Flag;
extern u8 start_flag;

extern u8 Timer_Flag_5;

//crypto
extern uint8_t public_key[13+1][64];
extern uint8_t private_key[13+1][32];
extern const struct uECC_Curve_t * curves[5];

//
extern u32 TIM2_Exceed_Times;
extern u32 TIM5_Exceed_Times;
extern u8 decided;


void pbft();




#endif