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
#define Global_f 1
#define MAX_ACK_NUMBER 5

#define PROPOSAL 1
#define CHOICE 2
#define COMMIT 3
#define COMMIT2 5
#define ACK 4

#define Sig_Size 48
#define MAX_PROPOSAL_PACKETS 30
#define SINGLE_COMMAND_SIZE 32  //��λbyte��Ŀǰ����hash�����command

#define Data_channel 66
#define Msg_channel 12



//Global
#define GROUPS_NUMBER 4
#define TOTAL_GROUPS 5
#define MAX_GROUP_NUMBER 20

#define GLOBAL_PROPOSAL_1 6
#define GLOBAL_SIG 7
#define GLOBAL_PROPOSAL_2 8
#define GLOBAL_CHOICE 9
#define GLOBAL_COMMIT 10

#define GLOBAL_READY 11
#define GLOBAL_ACK 12
#define GLOBAL_CONSENSUS_START 13

extern u8 Group_Number;
extern u8 Group_Leader_ID;
//extern u8 Global_Ready;

extern u8 Global_Commands_Size[TOTAL_GROUPS];							//������ָ����
extern u8 Global_Proposal_Packet_Number[TOTAL_GROUPS];					// = Command_Size / 4 ��ȡ��
extern u8 Global_Rece_Proposal[TOTAL_GROUPS][MAX_PROPOSAL_PACKETS];	//�������յ��᰸���ݰ�
//extern u8 Global_Rece_Proposal_flag[TOTAL_GROUPS];						//����Ƿ����������ݰ� 0 û��  1 ��
extern u8 Global_Rece_Proposal_Number[TOTAL_GROUPS]; 					//�յ��᰸���ݰ�����
extern u8 Global_Command_buff[TOTAL_GROUPS][800];						//ָ�����
extern u8 Global_Proposal_hash[TOTAL_GROUPS][40];						//�᰸��ϣ

extern u8 Global_Sig_Number[TOTAL_GROUPS];
extern u8 Global_IDS[TOTAL_GROUPS][MAX_GROUP_NUMBER];					//��ڼ��� ���ļ����ڵ��signature
extern u8 Global_Sig[TOTAL_GROUPS][MAX_GROUP_NUMBER][Sig_Size];			//��ڼ���ڵ�����signatures

extern u8 Global_Rece_Sig[TOTAL_GROUPS][MAX_GROUP_NUMBER];
extern u8 Global_Rece_Sig_Number[TOTAL_GROUPS];

extern u8 Global_Choice[16+1];
extern u8 Global_Commit[16+1];
extern u8 Global_ACK_Proposal[16+1];
extern u8 Global_ACK_Choice[16+1];
extern u8 Global_ACK_Commit[16+1];
extern u8 Global_ACK_Proposal_Number;
extern u8 Global_ACK_Choice_Number;
extern u8 Global_ACK_Commit_Number;
extern u8 Global_Choice_Truth_Set_Number;
extern u8 Global_Choice_False_Set_Number;
extern u8 Global_Commit_Truth_Set_Number;
extern u8 Global_Commit_False_Set_Number;

extern u8 Global_ACK_TO_Send_Number;
extern u8 Global_ACK_TO_Send[100][30];
extern u8 Global_Proposal_1_Number;			//�Լ��յ��˼���
extern u8 Global_Proposal_1_ACK_Number;
extern u8 Global_Proposal_1_ACK[TOTAL_GROUPS];	//˭�յ����ҵ�
extern u8 Global_Ready[TOTAL_GROUPS]; //˭Ready��
extern u8 start_flag2;
extern u8 Global_Ready_Number;
extern u8 Global_Consensus_Leader;
extern u8 Proposal_2[40];
extern u8 Proposal_2_hash[40];
extern u8 Rece_Proposal_2_flag;
extern u8 Rece_Global_Proposal_1[30];

extern u16 slot;
extern u8 id;	//id = 0 => leader           1-2-3 => follower 
extern u8 PBFT_status;
extern u8 Choice_Truth_Set_Number;
extern u8 Choice_False_Set_Number;

extern u8 Commit_Truth_Set_Number;
extern u8 Commit_False_Set_Number;

extern u8 Choice[16+1];
extern u8 Commit[16+1];

extern u8 Send_Data_buff[800];//�������ݻ�����



extern u8 Commands_Size;
extern u8 Proposal_Packet_Number;// = Command_Size / 4 ��ȡ��
extern u8 Rece_Proposal[MAX_PROPOSAL_PACKETS];
extern u8 Rece_Proposal_flag;
extern u8 Rece_Proposal_Number;
extern u8 Command_buff[800];
extern u8 Proposal_hash[40];
extern u8 start_flag;
extern u8 Timer_Flag_5;
extern u8 start_flag;

////////ACK
extern u8 ACK_Proposal[16+1];
extern u8 ACK_Choice[16+1];
extern u8 ACK_Commit[16+1];

extern u8 ACK_Proposal_Number;
extern u8 ACK_Choice_Number;
extern u8 ACK_Commit_Number;




extern u8 ACK_TO_Send_Number;
extern u8 ACK_TO_Send[100][30];

//crypto
extern uint8_t public_key[16+1][64];
extern uint8_t private_key[16+1][32];
extern const struct uECC_Curve_t * curves[5];

//
extern u32 TIM2_Exceed_Times;
extern u32 TIM5_Exceed_Times;
extern u32 TIM10_Exceed_Times;
extern u8 decided;


void pbft();




#endif