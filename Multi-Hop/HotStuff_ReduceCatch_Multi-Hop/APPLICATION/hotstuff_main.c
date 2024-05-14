#include "mw1268_app.h"
#include "mw1268_cfg.h"
#include "usart3.h"
#include "string.h"
#include "led.h"
#include "delay.h"
#include "hotstuff.h"
u8 N=13;
u8 f=30;
u16 DONE_THRES=200;
u8 Extera_slot = 3;
u8 id = 0;	//id = 0 => leader           1-2-3 => follower 
u8 NTX = 1;
u8 NTX_proposal = 5;
u8 phase_NTX = 3;
u8 phase_NTX2 = 1;
u16 slot = 800;

//Global 
u8 Global_NTX_proposal = 5;
u8 Global_NTX = 1;
u8 Group_Number;
u8 Group_Leader_ID;

//u8 Global_Ready = 0;
u8 Global_Commands_Size[TOTAL_GROUPS];							//区块内指令数
u8 Global_Proposal_Packet_Number[TOTAL_GROUPS];					// = Command_Size / 4 上取整
u8 Global_Rece_Proposal[TOTAL_GROUPS][MAX_PROPOSAL_PACKETS];	//跟随者收到提案数据包
//u8 Global_Rece_Proposal_flag[TOTAL_GROUPS];						//标记是否集满所有数据包 0 没有  1 有
u8 Global_Rece_Proposal_Number[TOTAL_GROUPS]; 					//收到提案数据包总数
u8 Global_Command_buff[TOTAL_GROUPS][800];						//指令缓冲区
u8 Global_Proposal_hash[TOTAL_GROUPS][40];						//提案哈希

u8 Global_Sig_Number[TOTAL_GROUPS];
u8 Global_IDS[TOTAL_GROUPS][MAX_GROUP_NUMBER];					//存第几组 有哪几个节点的signature
u8 Global_Sig[TOTAL_GROUPS][MAX_GROUP_NUMBER][Sig_Size];			//存第几组节点具体的signatures

u8 Global_Rece_Sig[TOTAL_GROUPS][MAX_GROUP_NUMBER];
u8 Global_Rece_Sig_Number[TOTAL_GROUPS];


//u8 Global_Choice[16+1];
//u8 Global_Commit[16+1];
u8 Global_ACK_Proposal[16+1];
//u8 Global_ACK_Choice[16+1];
//u8 Global_ACK_Commit[16+1];
u8 Global_ACK_Proposal_Number;
//u8 Global_ACK_Choice_Number;
//u8 Global_ACK_Commit_Number;
//u8 Global_Choice_Truth_Set_Number = 0;
//u8 Global_Choice_False_Set_Number = 0;
//u8 Global_Commit_Truth_Set_Number = 0;
//u8 Global_Commit_False_Set_Number = 0;
u8 Global_ACK_TO_Send_Number;
u8 Global_ACK_TO_Send[100][30];

u8 Global_Proposal_1_Number;			//自己收到了几个
u8 Rece_Global_Proposal_1[30];
u8 Global_Proposal_1_ACK_Number;
u8 Global_Proposal_1_ACK[TOTAL_GROUPS];	//谁收到了我的
u8 Global_Ready_Number;
u8 Global_Ready[TOTAL_GROUPS];//谁Ready了

u8 Proposal_2[40];
u8 Proposal_2_hash[40];
u8 Rece_Proposal_2_flag;
u8 Proposal_Queue[10];
u8 start_flag2 = 0;

u8 Global_Consensus_Leader = 1;
//Global_hotstuff
u8 Global_Prepare_Number;
u8 Global_Precommit_Number;
u8 Global_Commit_Number;
u8 Global_Viewchange_Number;

u8 Global_ACK_prepare;
u8 Global_ACK_precommit;
u8 Global_ACK_commit;
u8 Global_ACK_viewchange;

u8 Global_TIM_Prepare_Flag;
u8 Global_TIM_Precommit_Flag;
u8 Global_TIM_Commit_Flag;
u8 Global_TIM_Decide_Flag;

u8 Global_Rece_precommit;
u8 Global_Rece_commit;
u8 Global_Rece_decide;
u8 Global_Rece_next;
//id = 1
u8 Global_Rece_Prepare[20];
u8 Global_Prepare_id[20];
u8 Global_Prepare_Sig[20][64];

u8 Global_Rece_PreCommit[20];
u8 Global_PreCommit_id[20];
u8 Global_PreCommit_Sig[20][64];

u8 Global_Rece_Commit[20];
u8 Global_Commit_id[20];
u8 Global_Commit_Sig[20][64];

u8 Global_Rece_Viewchange[20];
u8 Global_Viewchange_id[20];
u8 Global_Viewchange_Sig[20][64];

u8 Timer_Flag_5;

//leader
u8 Need_proposal_2 = 0;
u8 Global_Need_precommit = 0;
u8 Global_Need_commit = 0;
u8 Global_Need_decide = 0;
u8 Global_Need_next = 0;
//viewchange
u8 Global_Need_PREPARE = 0;
u8 Global_Need_PRECOMMIT = 0;
u8 Global_Need_COMMIT = 0;
u8 Global_Need_VIEWCHANGE = 0;

void Send_Sig();
void Global_Follower_Send(u8 PHASE, u8 choice);
void Global_Leader_Send(u8 PHASE, u8* ID_list, u8 Sig_list[20][64]);
void Global_Send_Ready();
void Global_Start();
void Global_ACK_phase();
void Global_Go_ACK();
void Global_Start_TIMER();
void Global_TIMER();
void Global_New_Proposal();
void Global_Propose();
//void Global_NACK_phase();
void Global_Leader_Catching(u8 PHASE, u8* ACK_list);
void Global_Follower_Catching(u8 PHASE);

u8 NTX_proposal_flag = 0xff;
u8 Global_NTX_proposal_flag = 0xff;

//u8 Choice_Truth_Set_Number = 0;
//u8 Choice_False_Set_Number = 0;

//u8 Commit_Truth_Set_Number = 0;
//u8 Commit_False_Set_Number = 0;

//u8 Choice[13+1];
//u8 Commit[13+1];

u8 Send_Data_buff[800]={0};//发送数据缓冲区

////////throughput
u8 Commands_Size;						//区块内指令数
u8 Proposal_Packet_Number;				// = Command_Size / 4 上取整
u8 Rece_Proposal[MAX_PROPOSAL_PACKETS];	//跟随者收到提案数据包
u8 Rece_Proposal_flag;					//标记是否集满所有数据包 0 没有  1 有
u8 Rece_Proposal_Number; 				//收到提案数据包总数
u8 Command_buff[800];					//指令缓冲区
u8 Proposal_hash[40];					//提案哈希
u8 start_flag;
u8 Timer_Flag_5;
u8 before_round;
u8 after_round;
u8 TIM_slot = 2;
u32 TIM9_Exceed_Times;
u32 TIM10_Exceed_Times;


u8 HotStuff_status = 0; //0->choice  1->commit  2->decided

u8 my_choice=1;
u8 my_commit=1;
u8 my_precommit=1;
u8 my_decide=1;

////////ACK
//u8 ACK_Proposal[13+1];
//u8 ACK_Choice[13+1];
//u8 ACK_Commit[13+1];

//u8 ACK_Proposal_Number;
//u8 ACK_Choice_Number;
//u8 ACK_Commit_Number;

//u8 ACK_Proposal_Flag;
//u8 ACK_Choice_Flag;
//u8 ACK_Commit_Flag;

u8 ACK_TO_Send_Number;
u8 ACK_TO_Send[100][30];
const struct uECC_Curve_t * curves[5];

u32 TIM2_Exceed_Times;
u32 TIM5_Exceed_Times;

u8 decided;

//hotstuff
u8 Prepare_Number;
u8 Precommit_Number;
u8 Commit_Number;
u8 Viewchange_Number;

u8 Rece_precommit;
u8 Rece_commit;
u8 Rece_decide;
u8 Rece_next;

//leader
u8 Need_proposal;
u8 Need_precommit;
u8 Need_commit;
u8 Need_decide;
u8 Need_next;
//viewchange
u8 Need_PREPREPARE;
u8 Need_PREPARE;
u8 Need_PRECOMMIT;
u8 Need_COMMIT;
u8 Need_VIEWCHANGE;

//id = 1
u8 Rece_Prepare[20];
u8 Prepare_id[20];
u8 Prepare_Sig[20][64];

u8 Rece_PreCommit[20];
u8 PreCommit_id[20];
u8 PreCommit_Sig[20][64];

u8 Rece_Commit[20];
u8 Commit_id[20];
u8 Commit_Sig[20][64];

u8 Rece_Viewchange[20];
u8 Viewchange_id[20];
u8 Viewchange_Sig[20][64];
u8 precommit_verify_number;
u16 TEMP_LEN;
uint8_t hash[32] = {0};
uint8_t sig[64] = {0};

u32 Total_time;
u32 Total_time1;

void Send_REPLAY(u32 time1, u32 time2, u32 time3);
void sign_data(u8* hash,u8* sig);
void New_Proposal(u8 command_size);
void Propose(u8 idx);
void Start_TIMER();
void Sleep_Random();
void Follower_Send(u8 PHASE, u8 choice);
void Leader_Send(u8 PHASE, u8* ID_list, u8 Sig_list[20][64]);
void Leader_Catching(u8 PHASE, u8* ACK_list);
void Follower_Catching(u8 PHASE);
void test_finish();

void init(){
	while(LoRa_Init())//初始化LORA模块
	{
		delay_ms(300);
	}
	LoRa_CFG.chn = Data_channel;
	LoRa_CFG.tmode = LORA_TMODE_PT;
	LoRa_CFG.addr = 0;
	LoRa_CFG.power = LORA_TPW_9DBM;
	LoRa_CFG.wlrate = LORA_RATE_62K5;
	LoRa_CFG.lbt = LORA_LBT_ENABLE;
	
	LoRa_Set();
	
	Total_time=0;
	Total_time1=0;
	
//hotstuff
	for(u8 i=0;i<20;i++){
		Rece_Prepare[i] = 0xFF;
		Rece_PreCommit[i] = 0xFF;
		Rece_Commit[i] = 0xFF;
		Rece_Viewchange[i] = 0xFF;
	}
	
	Prepare_Number = 1;
	Precommit_Number = 1;
	Commit_Number = 1;
	Viewchange_Number = 1;
	
	Rece_precommit = 0;
	Rece_commit = 0;
	Rece_decide = 0;
	Rece_next = 0;
	precommit_verify_number=0;
	
	//throughput
	for(u8 m=0;m<MAX_PROPOSAL_PACKETS;m++) Rece_Proposal[m] = 0xFF;
	Proposal_Packet_Number = 0xFF;
	Rece_Proposal_flag = 0;
	Rece_Proposal_Number = 0;
	
	Need_proposal=0;
	Need_precommit=0;
	Need_commit=0;
	Need_decide=0;
	Need_next=0;
	//viewchange
	Need_PREPREPARE=0;
	Need_PREPARE=0;
	Need_PRECOMMIT=0;
	Need_COMMIT=0;
	Need_VIEWCHANGE=0;
	
	//Global
	Global_Proposal_1_Number = 1;
	Global_Ready_Number = 0;
	for(u8 m=0;m<TOTAL_GROUPS;m++){
		for(u8 n=0;n<MAX_PROPOSAL_PACKETS;n++) Global_Rece_Proposal[m][n] = 0xFF;
		for(u8 n=0;n<MAX_GROUP_NUMBER;n++){
			Global_IDS[m][n] = 0xFF;
			Global_Rece_Sig[m][n] = 0xFF;
		}
		
		Global_Proposal_Packet_Number[m] = 0xFF;
		Global_Rece_Proposal_Number[m] = 0;
		Global_Sig_Number[m] = 0;
		Global_Rece_Sig_Number[m] = 0;
		Global_Proposal_1_ACK[m] = 0xFF;
		Rece_Proposal_2_flag = 0;
		Global_Ready[m] = 0xFF;
	}
	for(u8 m=0;m<20;m++){
		Rece_Global_Proposal_1[m] = 0xFF;
	}
	
	for(u8 i=0;i<20;i++){
		Global_Rece_Prepare[i] = 0xFF;
		Global_Rece_PreCommit[i] = 0xFF;
		Global_Rece_Commit[i] = 0xFF;
		Global_Rece_Viewchange[i] = 0xFF;
	}
	
	Global_ACK_prepare = 0xFF;
	Global_ACK_precommit = 0xFF;
	Global_ACK_commit = 0xFF;
	Global_ACK_viewchange = 0xFF;
	
	Global_Prepare_Number = 1;
	Global_Precommit_Number = 1;
	Global_Commit_Number = 1;
	Global_Viewchange_Number = 1;
	
	Global_TIM_Prepare_Flag = 0xFF;
	Global_TIM_Precommit_Flag = 0xFF;
	Global_TIM_Commit_Flag = 0xFF;
	
	Global_Rece_precommit = 0;
	Global_Rece_commit = 0;
	Global_Rece_decide = 0;
	Global_Rece_next = 0;
}


void Start_main(){
	while(1){
		if(start_flag)break;
	}
}

u32 PREPREPARE_delay = 2000;//和proposal区块关联，区块两个包之间间隔500ms
u32 Packet_Internal_delay = 200;
u32 Phase_Internal_delay = 0;

void BeforeRound(){
	if(id==Group_Leader_ID && !start_flag2){
		before_round = 0;
		after_round = 0xff;
	}
	if(id == Global_Consensus_Leader && start_flag2){
		before_round = 0;
		after_round = 0xff;
	}
	while(before_round);
}
void AfterRound(){
	while(after_round){
//		if(id==2)
	
	}
}

void DONE_OR_NOT(){

	if(RNG_Get_RandomRange(2,101) <= DONE_THRES){
		usart3_rx(0);
	}else{
		usart3_rx(1);
	}

}

void Change_channel(u8 channel){
	LoRa_CFG.chn = channel;
//	LoRa_CFG.tmode = LORA_TMODE_PT;
//	LoRa_CFG.addr = 0;
//	LoRa_CFG.power = LORA_TPW_9DBM;
//	LoRa_CFG.wlrate = LORA_RATE_62K5;
//	LoRa_CFG.lbt = LORA_LBT_ENABLE;
	LoRa_Set();
}

void hotstuff(){
	btim_tim2_enable(ENABLE);
	///////小车参数设置，id////////
	id = 13;
	
	if(id >= 1 && id <= 4){Group_Number = 1;Group_Leader_ID = 1;}
	else if(id >= 5 && id <= 8){Group_Number = 2;Group_Leader_ID = 5;}
	else if(id >= 9 && id <= 12){Group_Number = 3;Group_Leader_ID = 9;}
	else if(id >= 13 && id <= 16){Group_Number = 4;Group_Leader_ID = 13;}
	
	//////////////
	led0sta = 0;
	led1sta = 0;
	led2sta = 0;
	LED0(1);LED1(1);LED2(1);
	init();
	init_public_key();
//	u8 Need_to_Wait = N;
	//数据类型
	//0 : id 	1 2 3 4
	//1 : data_type => 1->PROPOSAL 2->CHOICE 3->COMMIT 4->ACK
	//2 : data size 
	//2-x : data_body  （ACK 则 2表示是哪个类型的数据1->propose 2->choice 3->commit）//多轮共识这里需要修改
	//x+1-x+64 sig
	
	//leader : proposal
	
/*****************************Local_Consensus**************************************************************************************************************************************************************************/	

/**************************PREPREPARE**************************/
	if(id == Group_Leader_ID){
		Commands_Size = 1;
		Proposal_Packet_Number = (Commands_Size + 3)/4;
		
		New_Proposal(Commands_Size);
		Rece_Proposal_flag = 1;
	}
	
	Start_main();
	Change_channel(Group_Number * 10);
	TIM2->CNT = 0;
	TIM2_Exceed_Times = 0;
	
	TIM9->CNT = 0;
	TIM9_Exceed_Times = 0;
	before_round = 0xff;
	after_round = 0;
	
//	TIM10_Exceed_Times = 0;
//	TIM10->CR1|=0x01;
//	DONE_OR_NOT();
	
	for(u8 ntx=0;ntx<NTX_proposal;ntx++){
		
		while(1){
			if(TIM9_Exceed_Times==2 * ntx)break;
		}
		
		if(id==Group_Leader_ID)
			Propose(0);
		else{
			if(Rece_Proposal_flag)break;
		}
			
	}
	
	while(TIM9_Exceed_Times != 2 * NTX_proposal);
	
	TIM9_Exceed_Times = 0;
	before_round = 0xff;
	after_round = 0;
	
//	//run
//	run_slow();
	
	/**************************PREPARE**************************/
	
	my_choice = 1;

	for(u8 ntx=0;ntx<NTX;ntx++){

		//leader
		if(id==Group_Leader_ID){
			for(u8 ntx=0;ntx<NTX;ntx++){
				if(Prepare_Number >= (2 * f + 1)) break;
				BeforeRound();
				
				//nop
				
				AfterRound();
			
			}
		}
		if(id==Group_Leader_ID)break;
		if((Rece_precommit == 1) || (Rece_commit == 1) || (Rece_decide == 1) || (Rece_next == 1)) break;
		
		BeforeRound();
		
		if((Rece_precommit == 1) || (Rece_commit == 1) || (Rece_decide == 1) || (Rece_next == 1)) break;
		
		if(Rece_Proposal_flag){
			for(u8 m=0;m<phase_NTX;m++){
				Follower_Send(PREPARE,my_choice);
				delay_ms(Packet_Internal_delay);
			}
		}
			
		
		if((Rece_precommit == 1) || (Rece_commit == 1) || (Rece_decide == 1) || (Rece_next == 1)) break;
		
		AfterRound();
		
	}
	

	delay_ms(Phase_Internal_delay);
/**************************PRECOMMIT**************************/
	HotStuff_status = PRECOMMIT;
	my_precommit = 1;
	
	for(u8 ntx=0;ntx<NTX;ntx++){
		
		if((id != Group_Leader_ID) && ((Rece_commit==1) || (Rece_decide == 1) || (Rece_next == 1)))break;
		if(id == Group_Leader_ID && Precommit_Number >= (2*f+1)) break;
		
		BeforeRound();

		if(id != Group_Leader_ID){
			if(!(Rece_precommit == 1)){
				AfterRound();
				continue;
			}
		}
		
		if(id==Group_Leader_ID && Prepare_Number < ( 2 * f + 1)){
				AfterRound();
				continue;
		}
		
		if((id != Group_Leader_ID) && ((Rece_commit==1) || (Rece_decide == 1) || (Rece_next == 1)))break;
		if(id == Group_Leader_ID && Precommit_Number >= (2*f+1)) break;
		

		if(id == Group_Leader_ID){
			Leader_Send(PRECOMMIT,Prepare_id,Prepare_Sig);
			for(u8 m=0;m<Extera_slot;m++)Leader_Send(PRECOMMIT,Prepare_id,Prepare_Sig);
		}
		if(id != Group_Leader_ID){
			
			for(u8 m=0;m<phase_NTX;m++){
				Follower_Send(PRECOMMIT,my_precommit);
				delay_ms(Packet_Internal_delay);
			}
			
		}	
		
		if((id != Group_Leader_ID) && ((Rece_commit==1) || (Rece_decide == 1) || (Rece_next == 1)))break;
		if(id == Group_Leader_ID && Precommit_Number >= (2*f+1)) break;
		
		AfterRound();
		
	}
	delay_ms(Phase_Internal_delay);
	/**************************COMMIT**************************/
	HotStuff_status = COMMIT;
	my_commit = 1;
	
	for(u8 ntx=0;ntx<NTX;ntx++){
		
		if((id != Group_Leader_ID) && ((Rece_decide==1) || (Rece_next == 1)))break;
		if(id == Group_Leader_ID && Commit_Number >= (2*f+1))	break;
		
		BeforeRound();

		if(id != Group_Leader_ID){
			if(!(Rece_commit == 1)){
				AfterRound();
				continue;
			}
		}
		
		if(id==Group_Leader_ID && Precommit_Number < ( 2 * f + 1)){
				AfterRound();
				continue;
		}
		
		if((id != Group_Leader_ID) && ((Rece_decide==1) || (Rece_next == 1)))break;
		if(id == Group_Leader_ID && Commit_Number >= (2*f+1))	break;
		
		if(id == Group_Leader_ID){
			Leader_Send(COMMIT,PreCommit_id,PreCommit_Sig);
			for(u8 m=0;m<Extera_slot;m++)Leader_Send(COMMIT,PreCommit_id,PreCommit_Sig);
		}
		if(id != Group_Leader_ID){     
			for(u8 m=0;m<phase_NTX;m++){
				Follower_Send(COMMIT,my_commit);
				delay_ms(Packet_Internal_delay);
			}
		}
		
		if((id != Group_Leader_ID) && ((Rece_decide==1) || (Rece_next == 1)))break;
		if(id == Group_Leader_ID && Commit_Number >= (2*f+1))	break;
		AfterRound();
		
	}	
	
	delay_ms(Phase_Internal_delay);
/**************************DECIDE**************************/
	HotStuff_status = DECIDE;
	my_decide = 1;

	for(u8 ntx=0;ntx<NTX;ntx++){
		
		if((id != Group_Leader_ID) && (Rece_next==1))break;
		if(id == Group_Leader_ID && Viewchange_Number >= (2*f+1))break;
		
		BeforeRound();

		if(id != Group_Leader_ID){
			if(!(Rece_decide == 1)){
				AfterRound();
				continue;
			}
		}
		
		if(id==Group_Leader_ID && Commit_Number < ( 2 * f + 1)){
				AfterRound();
				continue;
		}	
		
		if((id != Group_Leader_ID) && (Rece_next==1))break;
		if(id == Group_Leader_ID && Viewchange_Number >= (2*f+1))break;
		
		if(id == Group_Leader_ID){
			Leader_Send(DECIDE,Commit_id,Commit_Sig);			
			for(u8 m=0;m<Extera_slot;m++)Leader_Send(DECIDE,Commit_id,Commit_Sig);
		}
		if(id != Group_Leader_ID){ 
			for(u8 m=0;m<phase_NTX;m++){
				Follower_Send(VIEW_CHANGE,my_decide);
				delay_ms(Packet_Internal_delay);
			}
			
		}
		
		if((id != Group_Leader_ID) && (Rece_next==1))break;
		if(id == Group_Leader_ID && Viewchange_Number >= (2*f+1))break;
		
		AfterRound();
		
	}	

	
	
/**************************Next_Round**************************/
	
	if(id == Group_Leader_ID && Viewchange_Number >= (2*f+1)){
		for(u8 ntx=0;ntx<NTX;ntx++){
			BeforeRound();
			
			Leader_Send(NEXT_ROUND,Viewchange_id,Viewchange_Sig);

			

			break;
			AfterRound();
			
		}
	}else if(id != Group_Leader_ID){
		BeforeRound();
		
		


		
		AfterRound();
	}
	
	
	
	if(id != Group_Leader_ID && Rece_next ==1){
		
		u8 nnn_flag = 1;
		while(1){
			BeforeRound();
			
			
			
			AfterRound();
		}
	}
	
	
	
/**************************Catching Up**************************/

	if(id==Group_Leader_ID){
		while(1){
			
			if(Need_proposal){
				Need_proposal = 0;
				Propose(0);
				delay_ms(Packet_Internal_delay);
			}
			
			if(Need_precommit && (Prepare_Number >= (2*f+1))){
				Need_precommit = 0;
				
				for(u8 m=0;m<phase_NTX2;m++){
					Leader_Send(PRECOMMIT,Prepare_id,Prepare_Sig);
					delay_ms(Packet_Internal_delay);
				}

				
			}
			if(Need_commit && (Precommit_Number >= (2*f+1))){
				Need_commit = 0;
				for(u8 m=0;m<phase_NTX2;m++){
					Leader_Send(COMMIT,PreCommit_id,PreCommit_Sig);
					delay_ms(Packet_Internal_delay);
				}

			}
			if(Need_decide && (Commit_Number >= (2*f+1))){
				Need_decide = 0;
				for(u8 m=0;m<phase_NTX2;m++){
					Leader_Send(DECIDE,Commit_id,Commit_Sig);
					delay_ms(Packet_Internal_delay);
				}

			}
			if(Need_next && (Viewchange_Number >= (2*f+1))){
				Need_next = 0;
				for(u8 m=0;m<phase_NTX2;m++){
					Leader_Send(NEXT_ROUND,Viewchange_id,Viewchange_Sig);
					delay_ms(Packet_Internal_delay);
				}

			}
			
			if(Prepare_Number < (2*f+1)){
				for(u8 m=0;m<phase_NTX;m++){
					Leader_Catching(PREPARE,Rece_Prepare);
					delay_ms(Packet_Internal_delay);
				}

			}else if(Precommit_Number < (2*f+1)){
				for(u8 m=0;m<phase_NTX;m++){
					Leader_Catching(PRECOMMIT,Rece_PreCommit);
					delay_ms(Packet_Internal_delay);
				}

					
			}else if(Commit_Number < (2*f+1)){
				for(u8 m=0;m<phase_NTX;m++){
					Leader_Catching(COMMIT,Rece_Commit);
					delay_ms(Packet_Internal_delay);
				}

					
			}else if(Viewchange_Number < (2*f+1)){
				

				for(u8 m=0;m<phase_NTX;m++){
					Leader_Catching(DECIDE,Rece_Viewchange);
					delay_ms(Packet_Internal_delay);
				}

					
			}else if(Viewchange_Number >= (2*f+1)){
				break;

//				

//				

					
			}
			Start_TIMER();
		}
	}
	else if (id != Group_Leader_ID){
		while(1){

			if(Need_PREPARE && Rece_Proposal_flag){
				Need_PREPARE = 0;
				
				for(u8 m=0;m<phase_NTX;m++){
					Follower_Send(PREPARE,my_choice);
					delay_ms(Packet_Internal_delay);
				}
				
			}
			if(Need_PRECOMMIT && ((Rece_precommit == 1)||(Rece_commit == 1)||(Rece_decide == 1)||(Rece_next==1))){
				Need_PRECOMMIT = 0;
				
				for(u8 m=0;m<phase_NTX;m++){
					Follower_Send(PRECOMMIT,my_precommit);
					delay_ms(Packet_Internal_delay);
				}
				
			}
			if(Need_COMMIT && ((Rece_commit == 1)||(Rece_decide == 1)||(Rece_next==1))){
				Need_COMMIT = 0;
				
				for(u8 m=0;m<phase_NTX;m++){
					Follower_Send(COMMIT,my_commit);
					delay_ms(Packet_Internal_delay);
				}
				
			}
			if(Need_VIEWCHANGE && ((Rece_decide == 1)||(Rece_next==1))){
				Need_VIEWCHANGE = 0;
				for(u8 m=0;m<phase_NTX;m++){
					Follower_Send(VIEW_CHANGE,my_decide);
					delay_ms(Packet_Internal_delay);
				}
				
			}
			
			if(Rece_Proposal_flag != 1){
				for(u8 m=0;m<phase_NTX;m++){
					Follower_Catching(PREPREPARE);
					delay_ms(Packet_Internal_delay);
				}
				
			}
			else if(Rece_precommit != 1){
				for(u8 m=0;m<phase_NTX;m++){
					Follower_Catching(PRECOMMIT);
					delay_ms(Packet_Internal_delay);
				}
				
			}else if(Rece_commit != 1){
				for(u8 m=0;m<phase_NTX;m++){
					Follower_Catching(COMMIT);
					delay_ms(Packet_Internal_delay);
				}
				
			}else if(Rece_decide != 1){
				for(u8 m=0;m<phase_NTX;m++){
					Follower_Catching(DECIDE);
					delay_ms(Packet_Internal_delay);
				}

			}else if(Rece_next != 1){
				

				for(u8 m=0;m<phase_NTX;m++){
					Follower_Catching(NEXT_ROUND);
					delay_ms(Packet_Internal_delay);
				}

			}else if(Rece_next == 1){

				
				
			}
			Start_TIMER();
		}
	}
/*****************************Global_Consensus**************************************************************************************************************************************************************************/	
	//broadcast Proposal&Sig
	Change_channel(Data_channel);
	for(u8 m=0;m<32;m++)
		Global_Proposal_hash[Group_Number][m] = Proposal_hash[m];
	
	Global_Proposal_1_Number = 1;
	Global_Proposal_1_ACK_Number = 1;
	Global_Ready_Number = 1;
	while(1){
		
		Global_ACK_phase();
		
		if(Global_Proposal_1_Number >= GROUPS_NUMBER && Global_Proposal_1_ACK_Number >= GROUPS_NUMBER)break;
		
		if(Global_Proposal_1_ACK_Number >= GROUPS_NUMBER)continue;
		
		Propose(1);
		Send_Sig();
		
		Global_TIMER();
	}
	//Ready
	while(1){
		Global_ACK_phase();
		
		Global_Send_Ready();
		if(start_flag2)break;
		if(Global_Ready_Number >= GROUPS_NUMBER && id == Global_Consensus_Leader)break;
		u8 tmp_ACK=0;
		Global_TIMER();
	}
	if(id == 1)delay_ms(5000);
	if(id == Global_Consensus_Leader){
		Global_Start();
		delay_ms(200);
		Global_Start();
		delay_ms(200);
		
		start_flag2 = 1;
	}
	while(!start_flag2){
	
	}
	
	/**************************PREPREPARE**************************/
	TIM9_Exceed_Times = 0;
	before_round = 0x00;
	after_round = 0xFF;
	
	if(id == Global_Consensus_Leader){
		Global_New_Proposal();
	}
	
	for(u8 ntx=0;ntx<Global_NTX_proposal;ntx++){
		
		while(1){
			if(TIM9_Exceed_Times==2 * ntx)break;
		}
		
		if(id==Global_Consensus_Leader)
			Global_Propose();
		else{
			if(Rece_Proposal_2_flag)break;
		}
			
	}
	
	while(TIM9_Exceed_Times != 2 * Global_NTX_proposal);
	
	TIM9_Exceed_Times = 0;
	before_round = 0xff;
	after_round = 0;
	
//	//run
//	run_slow();
	
	/**************************PREPARE**************************/
	
	my_choice = 1;

	for(u8 ntx=0;ntx<Global_NTX;ntx++){

		//leader
		if(id==Global_Consensus_Leader){
			for(u8 ntx=0;ntx<NTX;ntx++){
				if(Global_Prepare_Number >= (2 * Global_f + 1)) break;
				BeforeRound();
				
				//nop
				
				AfterRound();
			
			}
		}
		if(id==Global_Consensus_Leader)break;
		if((Global_Rece_precommit == 1) || (Global_Rece_commit == 1) || (Global_Rece_decide == 1) || (Global_Rece_next == 1)) break;
		
		BeforeRound();
		
		if((Global_Rece_precommit == 1) || (Global_Rece_commit == 1) || (Global_Rece_decide == 1) || (Global_Rece_next == 1)) break;
		
		if(Rece_Proposal_2_flag){
			for(u8 m=0;m<phase_NTX;m++){
				Global_Follower_Send(GLOBAL_PREPARE,1);
				delay_ms(Packet_Internal_delay);
			}
		}
			
		
		if((Global_Rece_precommit == 1) || (Global_Rece_commit == 1) || (Global_Rece_decide == 1) || (Global_Rece_next == 1)) break;
		
		AfterRound();
		
	}
	

	delay_ms(Phase_Internal_delay);
/**************************PRECOMMIT**************************/
	HotStuff_status = GLOBAL_PRECOMMIT;
	my_precommit = 1;
	
	for(u8 ntx=0;ntx<Global_NTX;ntx++){
		
		if((id != Global_Consensus_Leader) && ((Global_Rece_commit==1) || (Global_Rece_decide == 1) || (Global_Rece_next == 1)))break;
		if(id == Global_Consensus_Leader && Global_Precommit_Number >= (2*Global_f+1)) break;
		
		BeforeRound();

		if(id != Global_Consensus_Leader){
			if(!(Global_Rece_precommit == 1)){
				AfterRound();
				continue;
			}
		}
		
		if(id == Global_Consensus_Leader && Global_Prepare_Number < ( 2 * Global_f + 1)){
			AfterRound();
			continue;
		}
		
		if((id != Global_Consensus_Leader) && ((Global_Rece_commit==1) || (Global_Rece_decide == 1) || (Global_Rece_next == 1)))break;
		if(id == Global_Consensus_Leader && Global_Precommit_Number >= (2*Global_f+1)) break;
		

		if(id == Global_Consensus_Leader){
			Global_Leader_Send(GLOBAL_PRECOMMIT,Global_Prepare_id,Global_Prepare_Sig);
			for(u8 m=0;m<Extera_slot;m++)Global_Leader_Send(GLOBAL_PRECOMMIT,Global_Prepare_id,Global_Prepare_Sig);
		}
		if(id != Global_Consensus_Leader){
			
			for(u8 m=0;m<phase_NTX;m++){
				Global_Follower_Send(GLOBAL_PRECOMMIT,1);
				delay_ms(Packet_Internal_delay);
			}
			
		}	
		
		if((id != Global_Consensus_Leader) && ((Global_Rece_commit==1) || (Global_Rece_decide == 1) || (Global_Rece_next == 1)))break;
		if(id == Global_Consensus_Leader && Global_Precommit_Number >= (2*Global_f+1)) break;
		
		AfterRound();
		
	}
	delay_ms(Phase_Internal_delay);
	/**************************COMMIT**************************/
	HotStuff_status = GLOBAL_COMMIT;
	my_commit = 1;
	
	for(u8 ntx=0;ntx<Global_NTX;ntx++){
		
		if((id != Global_Consensus_Leader) && ((Global_Rece_decide==1) || (Global_Rece_next == 1)))break;
		if(id == Global_Consensus_Leader && Global_Commit_Number >= (2*Global_f+1))	break;
		
		BeforeRound();

		if(id != Global_Consensus_Leader){
			if(!(Global_Rece_commit == 1)){
				AfterRound();
				continue;
			}
		}
		
		if(id == Global_Consensus_Leader && Global_Precommit_Number < ( 2 * Global_f + 1)){
			AfterRound();
			continue;
		}
		
		if((id != Global_Consensus_Leader) && ((Global_Rece_decide==1) || (Global_Rece_next == 1)))break;
		if(id == Global_Consensus_Leader && Global_Commit_Number >= (2*Global_f+1))	break;
		
		if(id == Global_Consensus_Leader){
			Global_Leader_Send(GLOBAL_COMMIT,Global_PreCommit_id,Global_PreCommit_Sig);
			for(u8 m=0;m<Extera_slot;m++)Global_Leader_Send(GLOBAL_COMMIT,Global_PreCommit_id,Global_PreCommit_Sig);
		}
		if(id != Global_Consensus_Leader){     
			for(u8 m=0;m<phase_NTX;m++){
				Global_Follower_Send(GLOBAL_COMMIT,1);
				delay_ms(Packet_Internal_delay);
			}
		}
		
		if((id != Global_Consensus_Leader) && ((Global_Rece_decide==1) || (Global_Rece_next == 1)))break;
		if(id == Global_Consensus_Leader && Global_Commit_Number >= (2*Global_f+1))	break;
		AfterRound();
		
	}	
	
	delay_ms(Phase_Internal_delay);
/**************************DECIDE**************************/
	HotStuff_status = GLOBAL_DECIDE;
	my_decide = 1;

	for(u8 ntx=0;ntx<Global_NTX;ntx++){
		
		if((id != Global_Consensus_Leader) && (Global_Rece_next==1))break;
		if(id == Global_Consensus_Leader && Global_Viewchange_Number >= (2*Global_f+1))break;
		
		BeforeRound();

		if(id != Global_Consensus_Leader){
			if(!(Global_Rece_decide == 1)){
				AfterRound();
				continue;
			}
		}
		
		if(id == Global_Consensus_Leader && Global_Commit_Number < ( 2 * Global_f + 1)){
				AfterRound();
				continue;
		}	
		
		if((id != Global_Consensus_Leader) && (Global_Rece_next==1))break;
		if(id == Global_Consensus_Leader && Global_Viewchange_Number >= (2*Global_f+1))break;
		
		if(id == Global_Consensus_Leader){
			Global_Leader_Send(GLOBAL_DECIDE,Global_Commit_id,Global_Commit_Sig);			
			for(u8 m=0;m<Extera_slot;m++)Global_Leader_Send(GLOBAL_DECIDE,Global_Commit_id,Global_Commit_Sig);
		}
		if(id != Global_Consensus_Leader){ 
			for(u8 m=0;m<phase_NTX;m++){
				Global_Follower_Send(GLOBAL_VIEW_CHANGE,1);
				delay_ms(Packet_Internal_delay);
			}
			
		}
		
		if((id != Global_Consensus_Leader) && (Global_Rece_next==1))break;
		if(id == Global_Consensus_Leader && Global_Viewchange_Number >= (2*Global_f+1))break;
		
		AfterRound();
		
	}	
	if((Global_Rece_next==1 || Global_Viewchange_Number >= (2*Global_f+1)) && Total_time==0)
		Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
	delay_ms(Phase_Internal_delay);
/**************************Next_Round**************************/
	
	if(id == Global_Consensus_Leader && Global_Viewchange_Number >= (2*Global_f+1)){
		for(u8 ntx=0;ntx<Global_NTX;ntx++){
			BeforeRound();
			
			Global_Leader_Send(GLOBAL_NEXT_ROUND,Global_Viewchange_id,Global_Viewchange_Sig);
			for(u8 m=0;m<Extera_slot;m++)Global_Leader_Send(GLOBAL_NEXT_ROUND,Global_Viewchange_id,Global_Viewchange_Sig);
			
			Send_REPLAY(Total_time1,Total_time,TIM2_Exceed_Times);
			
			AfterRound();
			
		}
	}else if(id != Global_Consensus_Leader){
		BeforeRound();
		
		if((Global_Rece_next==1 || Global_Viewchange_Number >= (2*Global_f+1)) && Total_time==0)
			Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
		
		AfterRound();
	}
	
	
	
	if(id != Global_Consensus_Leader && Global_Rece_next ==1){
		
		u8 nnn_flag = 1;
		while(1){
			BeforeRound();
			
			Send_REPLAY(Total_time1,Total_time,TIM2_Exceed_Times);
			
			AfterRound();
		}
	}
	
	
	
/**************************Catching Up**************************/

	if(id==Global_Consensus_Leader){
		while(1){
			
			if(Need_proposal_2){
				Need_proposal_2 = 0;
				Global_Propose();
				delay_ms(Packet_Internal_delay);
			}
			test_finish();
			if(Global_Need_precommit && (Global_Prepare_Number >= (2*Global_f+1))){
				Global_Need_precommit = 0;
				
				for(u8 m=0;m<phase_NTX2;m++){
					Global_Leader_Send(GLOBAL_PRECOMMIT,Global_Prepare_id,Global_Prepare_Sig);
					delay_ms(Packet_Internal_delay);
				}

			}
			test_finish();
			if(Global_Need_commit && (Global_Precommit_Number >= (2*Global_f+1))){
				Global_Need_commit = 0;
				for(u8 m=0;m<phase_NTX2;m++){
					Global_Leader_Send(GLOBAL_COMMIT,Global_PreCommit_id,Global_PreCommit_Sig);
					delay_ms(Packet_Internal_delay);
				}

			}
			test_finish();
			if(Global_Need_decide && (Global_Commit_Number >= (2*Global_f+1))){
				Global_Need_decide = 0;
				for(u8 m=0;m<phase_NTX2;m++){
					Global_Leader_Send(GLOBAL_DECIDE,Global_Commit_id,Global_Commit_Sig);
					delay_ms(Packet_Internal_delay);
				}

			}
			test_finish();
			if(Global_Need_next && (Global_Viewchange_Number >= (2*Global_f+1))){
				Global_Need_next = 0;
				for(u8 m=0;m<phase_NTX2;m++){
					Global_Leader_Send(GLOBAL_NEXT_ROUND,Global_Viewchange_id,Global_Viewchange_Sig);
					delay_ms(Packet_Internal_delay);
				}

			}
			test_finish();
			if(Global_Prepare_Number < (2*Global_f+1)){
				for(u8 m=0;m<phase_NTX;m++){
					Global_Leader_Catching(GLOBAL_PREPARE,Global_Rece_Prepare);
					delay_ms(Packet_Internal_delay);
				}

			}else if(Global_Precommit_Number < (2*Global_f+1)){
				for(u8 m=0;m<phase_NTX;m++){
					Global_Leader_Catching(GLOBAL_PRECOMMIT,Global_Rece_PreCommit);
					delay_ms(Packet_Internal_delay);
				}

					
			}else if(Global_Commit_Number < (2*Global_f+1)){
				for(u8 m=0;m<phase_NTX;m++){
					Global_Leader_Catching(GLOBAL_COMMIT,Global_Rece_Commit);
					delay_ms(Packet_Internal_delay);
				}

					
			}else if(Global_Viewchange_Number < (2*Global_f+1)){
				
				if(Total_time1 == 0) Total_time1 = TIM2->CNT + TIM2_Exceed_Times * 9000;
				for(u8 m=0;m<phase_NTX;m++){
					Global_Leader_Catching(GLOBAL_DECIDE,Global_Rece_Viewchange);
					delay_ms(Packet_Internal_delay);
				}

					
			}else if(Global_Viewchange_Number >= (2*Global_f+1)){
				
				if(Total_time == 0) Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
				
				Send_REPLAY(Total_time1,Total_time,TIM2_Exceed_Times);
				
				delay_ms(Packet_Internal_delay);
					
			}
			Start_TIMER();
		}
	}
	else if (id != Global_Consensus_Leader){
		while(1){

			if(Global_Need_PREPARE && Rece_Proposal_2_flag){
				Global_Need_PREPARE = 0;
				
				for(u8 m=0;m<phase_NTX;m++){
					Global_Follower_Send(GLOBAL_PREPARE,1);
					delay_ms(Packet_Internal_delay);
				}
				
			}
			
			if(Global_Need_PRECOMMIT && ((Global_Rece_precommit == 1)||(Global_Rece_commit == 1)||(Global_Rece_decide == 1)||(Global_Rece_next==1))){
				Global_Need_PRECOMMIT = 0;
				
				for(u8 m=0;m<phase_NTX;m++){
					Global_Follower_Send(GLOBAL_PRECOMMIT,1);
					delay_ms(Packet_Internal_delay);
				}
				
			}
			if(Global_Need_COMMIT && ((Global_Rece_commit == 1)||(Global_Rece_decide == 1)||(Global_Rece_next==1))){
				Global_Need_COMMIT = 0;
				
				for(u8 m=0;m<phase_NTX;m++){
					Global_Follower_Send(GLOBAL_COMMIT,1);
					delay_ms(Packet_Internal_delay);
				}
				
			}
			if(Global_Need_VIEWCHANGE && ((Global_Rece_decide == 1)||(Global_Rece_next==1))){
				Global_Need_VIEWCHANGE = 0;
				for(u8 m=0;m<phase_NTX;m++){
					Global_Follower_Send(GLOBAL_VIEW_CHANGE,1);
					delay_ms(Packet_Internal_delay);
				}
				
			}
			
			if(Rece_Proposal_2_flag != 1){
				for(u8 m=0;m<phase_NTX;m++){
					Global_Follower_Catching(GLOBAL_PREPREPARE);
					delay_ms(Packet_Internal_delay);
				}
				
			}
			else if(Global_Rece_precommit != 1){
				for(u8 m=0;m<phase_NTX;m++){
					Global_Follower_Catching(GLOBAL_PRECOMMIT);
					delay_ms(Packet_Internal_delay);
				}
				
			}else if(Global_Rece_commit != 1){
				for(u8 m=0;m<phase_NTX;m++){
					Global_Follower_Catching(GLOBAL_COMMIT);
					delay_ms(Packet_Internal_delay);
				}
				
			}else if(Global_Rece_decide != 1){
				for(u8 m=0;m<phase_NTX;m++){
					Global_Follower_Catching(GLOBAL_DECIDE);
					delay_ms(Packet_Internal_delay);
				}

			}else if(Global_Rece_next != 1){
				
				if(Total_time1 == 0) Total_time1 = TIM2->CNT + TIM2_Exceed_Times * 9000;
				for(u8 m=0;m<phase_NTX;m++){
					Global_Follower_Catching(GLOBAL_NEXT_ROUND);
					delay_ms(Packet_Internal_delay);
				}

			}else if(Global_Rece_next == 1){
				if(Total_time == 0) Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
				Send_REPLAY(Total_time1,Total_time,TIM2_Exceed_Times);
				delay_ms(Packet_Internal_delay);
			}
			Global_Start_TIMER();
		}
	}
	
}


//module
void test_finish(){
	if(Global_Viewchange_Number >= 2*Global_f+1 && Total_time==0){
		Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
	}
}

void sign_data(u8* hash,u8* sig){
	while(1){
			u8 sign_flag=1;
			
			while(1){
				if (!uECC_sign(private_key[id], hash, 32, sig, uECC_secp192r1())) {
					sprintf((char*)Send_Data_buff,"Sign Fail2");
					Lora_Device_Sta = LORA_TX_STA;
					LoRa_SendData(Send_Data_buff);// id 0 "Hello LoRa"
				}else{
					break;
				}
			}
			
			
			for(u8 p=0;p<Sig_Size;p++){
				if(sig[p]==0){
					sign_flag=0;
					break;
				}
			}
			if(sign_flag)break;
		}
}
void New_Proposal(u8 command_size){
	//command_size
	u32 seed;
	u8 output[32];
	u8 seed_u8[4];
	for(u16 i=0;i<command_size;i++){
		seed = RNG_Get_RandomNum();
		
		seed_u8[0]=seed & 0xFF;
		seed_u8[1]=(seed >> 8) & 0xFF;
		seed_u8[2]=(seed >> 8*2) & 0xFF;
		seed_u8[3]=(seed >> 8*3) & 0xFF;
		
		sha2(seed_u8,4,output,0);
		
		u8 _flag = 1;
		for(u8 m=0;m<32;m++){
			if(output[m]==0x00){
				_flag = 0;
				break;
			}
		}
		if(!_flag){
			i = i - 1;
			continue;
		}
			
		
		for(u8 m=0;m<32;m++)
			Command_buff[m+i*32] = output[m];
		
		_flag = 1;
		if(i==command_size-1){
			sha2(Command_buff,command_size*SINGLE_COMMAND_SIZE,Proposal_hash,0);
			for(u32 m=0;m<32;m++){
				if(Proposal_hash[m]==0x00){
					_flag = 0;
					break;
				}
			}
		}
		
		if(!_flag){
			i = i - 1;
			continue;
		}
	
	}
	
}
void Propose(u8 idx){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};	
	for(u8 packet_id=1;packet_id<=Proposal_Packet_Number;packet_id++){
		Send_Data_buff[0] = id;
		
		if(idx==0)
			Send_Data_buff[1] = PROPOSAL;
		else if(idx == 1)
			Send_Data_buff[1] = GLOBAL_PROPOSAL_1;
		
		Send_Data_buff[2] = Proposal_Packet_Number;
		Send_Data_buff[3] = packet_id;
		
		if(packet_id!=Proposal_Packet_Number) Send_Data_buff[4] = 4;
		else Send_Data_buff[4] = Commands_Size - 4 * ( Proposal_Packet_Number - 1);
		
		Send_Data_buff[4] = SINGLE_COMMAND_SIZE * Send_Data_buff[4];
		u8 Data_Size = Send_Data_buff[4];
		
		for(u8 m=0;m<Data_Size;m++)
			Send_Data_buff[5 + m] = Command_buff[(packet_id-1)*4*SINGLE_COMMAND_SIZE + m];
		
		sha2(Send_Data_buff,Send_Data_buff[4]+5,hash,0);
		
		//sign
		sign_data(hash,sig);
		
		for(u8 m=0;m<64;m++){
			Send_Data_buff[m+Data_Size+5] = sig[m];
		}
		
		Send_Data_buff[Data_Size+5+Sig_Size] = 0x00;
		
		Lora_Device_Sta = LORA_TX_STA;
		LoRa_SendData(Send_Data_buff);// id 0 "Hello LoRa"
		
		delay_ms(Packet_Internal_delay);
	}
	
}

void Send_REPLAY(u32 time1, u32 time2, u32 time3){
	LoRa_CFG.chn = Msg_channel;
	LoRa_Set();
	
	Send_Data_buff[0] = id + '0';
	sprintf((char*)Send_Data_buff+ sizeof(u8),"I am done!%d,,%d||%d",time1,time2,time3);
	
	Lora_Device_Sta = LORA_TX_STA;
	LoRa_SendData(Send_Data_buff);
	
	LoRa_CFG.chn = Data_channel;
	LoRa_Set();
}
void Sleep_Random(){
	u32 random = RNG_Get_RandomRange(1,N);
	random = random * slot;
	delay_ms(random);
}


void Start_TIMER(){
	//打开定时器
	TIM5->CNT = 0;
	TIM5->CR1|=0x01;
	Timer_Flag_5 = 0xFF;
	while(Timer_Flag_5){
		
	}
	//关闭定时器
	TIM5->CR1|=0x00;
}

void Follower_Send(u8 PHASE, u8 choice){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};	
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = PHASE;
	Send_Data_buff[2] = 1+32;
	Send_Data_buff[3] = choice;
	
	u8 Data_Size = Send_Data_buff[2];
	
	for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Proposal_hash[m];
		
	sha2(Send_Data_buff,Data_Size+3,hash,0);
	
	//sign
	sign_data(hash,sig);

	for(u8 m = 0 ; m < Sig_Size ; m++)Send_Data_buff[Data_Size + 3 + m] = sig[m];

	Send_Data_buff[Data_Size + 3 + Sig_Size] = 0x00;
	
	Lora_Device_Sta = LORA_TX_STA;
	LoRa_SendData(Send_Data_buff);// id 0 "Hello LoRa"1
}

void Leader_Send(u8 PHASE, u8* ID_list, u8 Sig_list[20][64]){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	//发送2f个签名,分为（2f）/3次
	u8 send_number = (2*f+2)/3;
	for(u8 Packet_id=0;Packet_id<send_number;Packet_id++){
		Send_Data_buff[0] = id;
		Send_Data_buff[1] = PHASE;
		if(Packet_id != send_number-1 )Send_Data_buff[3] = 3;
		else Send_Data_buff[3] = (2*f)-Packet_id*3;
		u8 Total_id = Send_Data_buff[3];
		Send_Data_buff[2] = 1+Total_id+Total_id*Sig_Size+32;
		u8 Data_Size = Send_Data_buff[2];
		
		for(u8 i=0;i<Total_id;i++){
			Send_Data_buff[4+i] = ID_list[i+Packet_id*3];
		}
		
		for(u8 x=0;x<Total_id;x++){
			for(u8 y=0;y<Sig_Size;y++)
			Send_Data_buff[3 + 1 + Total_id + Sig_Size*x + y]=Sig_list[x+Packet_id*3][y];
		}
		
		for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Proposal_hash[m];
		
		sha2(Send_Data_buff,Data_Size+3,hash,0);
	
		//sign
		sign_data(hash,sig);
		
		for(u8 m = 0 ; m < Sig_Size ; m++)Send_Data_buff[Data_Size + 3 + m] = sig[m];

		Send_Data_buff[Data_Size + 3 + Sig_Size] = 0x00;
		
		Lora_Device_Sta = LORA_TX_STA;
		LoRa_SendData(Send_Data_buff);// id 0 "Hello LoRa"1
				
		delay_ms(Packet_Internal_delay);
	}
}


void Leader_Catching(u8 PHASE, u8* ACK_list){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = CATCHING;
	Send_Data_buff[2] = 1+N+32;
	Send_Data_buff[3] = PHASE;
	for(u8 m=1;m<=N;m++) Send_Data_buff[3+m]=ACK_list[m + 4 * (Group_Number - 1)];
	
	u8 Data_Size = Send_Data_buff[2];
	
	for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Proposal_hash[m];
	
	sha2(Send_Data_buff,Data_Size+3,hash,0);
	//sign
	sign_data(hash,sig);
	
	for(u8 m=0;m<Sig_Size;m++)	Send_Data_buff[m+Data_Size+3] = sig[m];
	
	Send_Data_buff[Data_Size+3+Sig_Size] = 0x00;
	
	Lora_Device_Sta = LORA_TX_STA;
	LoRa_SendData(Send_Data_buff);
}

void Follower_Catching(u8 PHASE){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = CATCHING;
	if(PHASE == PREPREPARE)
		Send_Data_buff[2] = 1;
	else 
		Send_Data_buff[2] = 1+32;
	Send_Data_buff[3] = PHASE;
//	for(u8 m=1;m<=N;m++) Send_Data_buff[3+m]=ACK_list[m];
	
	u8 Data_Size = Send_Data_buff[2];
	
	if(PHASE != PREPREPARE)
		for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Proposal_hash[m];
	
	sha2(Send_Data_buff,Data_Size+3,hash,0);
	//sign
	sign_data(hash,sig);
	
	for(u8 m=0;m<Sig_Size;m++)	Send_Data_buff[m+Data_Size+3] = sig[m];
	
	Send_Data_buff[Data_Size+3+Sig_Size] = 0x00;
	
	Lora_Device_Sta = LORA_TX_STA;
	LoRa_SendData(Send_Data_buff);
}

//Global

void Global_Send_Ready(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = GLOBAL_READY;
	Send_Data_buff[2] = 1 + 32;
	Send_Data_buff[3] = 1;
	
	u8 Data_Size = Send_Data_buff[2];
	
	for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Global_Proposal_hash[Group_Number][m];
		
	sha2(Send_Data_buff,Data_Size+3,hash,0);
	
	//sign
	sign_data(hash,sig);

	for(u8 m = 0 ; m < Sig_Size ; m++)Send_Data_buff[Data_Size + 3 + m] = sig[m];

	Send_Data_buff[Data_Size + 3 + Sig_Size] = 0x00;

	Lora_Device_Sta = LORA_TX_STA;
	LoRa_SendData(Send_Data_buff);// id 0 "Hello LoRa"1	

}

void Global_Start(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = GLOBAL_CONSENSUS_START;
	Send_Data_buff[2] = 1 + 32;
	Send_Data_buff[3] = 1;
	
	u8 Data_Size = Send_Data_buff[2];
	
	for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Global_Proposal_hash[Group_Number][m];
		
	sha2(Send_Data_buff,Data_Size+3,hash,0);
	
	//sign
	sign_data(hash,sig);

	for(u8 m = 0 ; m < Sig_Size ; m++)Send_Data_buff[Data_Size + 3 + m] = sig[m];

	Send_Data_buff[Data_Size + 3 + Sig_Size] = 0x00;

	Lora_Device_Sta = LORA_TX_STA;
	LoRa_SendData(Send_Data_buff);// id 0 "Hello LoRa"1	

}
void Send_Sig(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	//发送2f个签名,分为（2f）/3次
	u8 send_number = (2*f+2)/3;
	for(u8 Packet_id=0;Packet_id<send_number;Packet_id++){
		Send_Data_buff[0] = id;
		Send_Data_buff[1] = GLOBAL_SIG;
		if(Packet_id != send_number-1 )Send_Data_buff[3] = 3;
		else Send_Data_buff[3] = (2*f)-Packet_id*3;
		u8 Total_id = Send_Data_buff[3];
		Send_Data_buff[2] = 1+Total_id+Total_id*Sig_Size+32;
		u8 Data_Size = Send_Data_buff[2];
		
		for(u8 i=0;i<Total_id;i++){
			Send_Data_buff[4+i] = Global_IDS[Group_Number][i+Packet_id*3];
		}
		
		for(u8 x=0;x<Total_id;x++){
			for(u8 y=0;y<Sig_Size;y++)
			Send_Data_buff[3 + 1 + Total_id + Sig_Size*x + y]=Global_Sig[Group_Number][x+Packet_id*3][y];
		}
		
		for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Global_Proposal_hash[Group_Number][m];
		
		sha2(Send_Data_buff,Data_Size+3,hash,0);
	
		//sign
		sign_data(hash,sig);
		
		for(u8 m = 0 ; m < Sig_Size ; m++)Send_Data_buff[Data_Size + 3 + m] = sig[m];

		Send_Data_buff[Data_Size + 3 + Sig_Size] = 0x00;
		
		Lora_Device_Sta = LORA_TX_STA;
		LoRa_SendData(Send_Data_buff);// id 0 "Hello LoRa"1
				
		delay_ms(Packet_Internal_delay);
	}
	
}

void Global_ACK_phase(){
	while(Global_ACK_TO_Send_Number){
		taskENTER_CRITICAL();
		Global_Go_ACK();
		taskEXIT_CRITICAL();
		Sleep_Random();
	}
}

void Global_Go_ACK(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	if(Global_ACK_TO_Send_Number >= 100 ){
		Global_ACK_TO_Send_Number = 10;
		return;
	}
	
	if(Global_ACK_TO_Send[Global_ACK_TO_Send_Number-1][2] == GLOBAL_PROPOSAL_1){
		Send_Data_buff[0] = Global_ACK_TO_Send[Global_ACK_TO_Send_Number-1][0];
		Send_Data_buff[1] = Global_ACK_TO_Send[Global_ACK_TO_Send_Number-1][1];//ACK
		Send_Data_buff[2] = 2+32;
		Send_Data_buff[3] = Global_ACK_TO_Send[Global_ACK_TO_Send_Number-1][2];
		Send_Data_buff[4] = Global_ACK_TO_Send[Global_ACK_TO_Send_Number-1][3];
		
		u8 Data_Size = Send_Data_buff[2];
		for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Global_Proposal_hash[Group_Number][m];
		
		sha2(Send_Data_buff,Data_Size+3,hash,0);
		
		//sign
		sign_data(hash,sig);

		for(u8 m = 0 ; m < Sig_Size ; m++)Send_Data_buff[Data_Size + 3 + m] = sig[m];

		Send_Data_buff[Data_Size + 3 + Sig_Size] = 0x00;
		
		Lora_Device_Sta = LORA_TX_STA;
		LoRa_SendData(Send_Data_buff);// id 0 "Hello LoRa"1
		
	}else{
		Send_Data_buff[0] = Global_ACK_TO_Send[Global_ACK_TO_Send_Number-1][0];
		Send_Data_buff[1] = Global_ACK_TO_Send[Global_ACK_TO_Send_Number-1][1];
		Send_Data_buff[2] = 2+32;
		Send_Data_buff[3] = Global_ACK_TO_Send[Global_ACK_TO_Send_Number-1][2];
		Send_Data_buff[4] = Global_ACK_TO_Send[Global_ACK_TO_Send_Number-1][3];
		
		u8 Data_Size = Send_Data_buff[2];
		
		for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m] = Proposal_2_hash[m];
		
		sha2(Send_Data_buff,Data_Size+3,hash,0);
		
		//sign
		sign_data(hash,sig);

		for(u8 m = 0 ; m < Sig_Size ; m++)Send_Data_buff[Data_Size + 3 + m] = sig[m];

		Send_Data_buff[Data_Size + 3 + Sig_Size] = 0x00;
		
		Lora_Device_Sta = LORA_TX_STA;
		LoRa_SendData(Send_Data_buff);// id 0 "Hello LoRa"1
	}
	
	Global_ACK_TO_Send_Number = Global_ACK_TO_Send_Number - 1;
}


void Global_TIMER(){
	//打开定时器
	Timer_Flag_5 = 0xFF;
	TIM5->CNT = 0;
	TIM5->CR1|=0x01;
	while(Timer_Flag_5){
		if(start_flag2)break;
	}
	//关闭定时器
	TIM5->CR1|=0x00;

}

void Global_Start_TIMER(){
		//打开定时器
	TIM5->CNT = 0;
	TIM5->CR1|=0x01;
	Timer_Flag_5 = 0xFF;
	while(Timer_Flag_5){
		
	}
	
	//关闭定时器
	TIM5->CR1|=0x00;

}

void Global_New_Proposal(){
	// 顺序 1-2-3-4拟定
	u8 tmp_buff[100];
	Proposal_Queue[0] = 1;Proposal_Queue[1] = 2;
	Proposal_Queue[2] = 3;Proposal_Queue[3] = 4;
	tmp_buff[0] = 1;
	tmp_buff[1] = 2;
	tmp_buff[2] = 3;
	tmp_buff[3] = 4;
	u32 seed;
	u8 output[32];
	u8 seed_u8[4];
	while(1){
		seed = RNG_Get_RandomNum();
		tmp_buff[4]=seed & 0xFF;
		tmp_buff[5]=(seed >> 8) & 0xFF;
		tmp_buff[6]=(seed >> 8*2) & 0xFF;
		tmp_buff[7]=(seed >> 8*3) & 0xFF;
		if(tmp_buff[4]==0x00 || tmp_buff[5]==0x00 || tmp_buff[6]==0x00 || tmp_buff[7]==0x00) continue;
		
		sha2(tmp_buff,8,output,0);
		
		u8 _flag = 1;
		for(u8 m=0;m<32;m++){
			if(output[m]==0x00){
				_flag = 0;
				break;
			}
		}
		if(_flag == 1)break;
	}
	
	for(u8 m=0;m<8;m++)
		Proposal_2[m] = tmp_buff[m];
	for(u8 m=0;m<32;m++)
		Proposal_2_hash[m] = output[m];
}

void Global_Propose(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = GLOBAL_PROPOSAL_2;
	Send_Data_buff[2] = 8 + 32;
	for(u8 m=0;m<8;m++)
		Send_Data_buff[3+m] = Proposal_2[m];
//	Send_Data_buff[3] = Proposal_Queue[0];
//	Send_Data_buff[4] = Proposal_Queue[1];
//	Send_Data_buff[5] = Proposal_Queue[2];
//	Send_Data_buff[6] = Proposal_Queue[3];
	
	u8 Data_Size = Send_Data_buff[2];
	
	for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Proposal_2_hash[m];
		
	sha2(Send_Data_buff,Data_Size+3,hash,0);
	
	//sign
	sign_data(hash,sig);

	for(u8 m = 0 ; m < Sig_Size ; m++)Send_Data_buff[Data_Size + 3 + m] = sig[m];

	Send_Data_buff[Data_Size + 3 + Sig_Size] = 0x00;

	Lora_Device_Sta = LORA_TX_STA;
	LoRa_SendData(Send_Data_buff);// id 0 "Hello LoRa"1

}
void Global_Follower_Send(u8 PHASE, u8 choice){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};	
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = PHASE;
	Send_Data_buff[2] = 1+32;
	Send_Data_buff[3] = choice;
	
	u8 Data_Size = Send_Data_buff[2];
	
	for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Proposal_2_hash[m];
		
	sha2(Send_Data_buff,Data_Size+3,hash,0);
	
	//sign
	sign_data(hash,sig);

	for(u8 m = 0 ; m < Sig_Size ; m++)Send_Data_buff[Data_Size + 3 + m] = sig[m];

	Send_Data_buff[Data_Size + 3 + Sig_Size] = 0x00;
	
	Lora_Device_Sta = LORA_TX_STA;
	LoRa_SendData(Send_Data_buff);// id 0 "Hello LoRa"1
}

void Global_Leader_Send(u8 PHASE, u8* ID_list, u8 Sig_list[20][64]){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	//发送2f个签名,分为（2f）/3次
	u8 send_number = (2*f+2)/3;
	for(u8 Packet_id=0;Packet_id<send_number;Packet_id++){
		Send_Data_buff[0] = id;
		Send_Data_buff[1] = PHASE;
		if(Packet_id != send_number-1 )Send_Data_buff[3] = 3;
		else Send_Data_buff[3] = (2*f)-Packet_id*3;
		u8 Total_id = Send_Data_buff[3];
		Send_Data_buff[2] = 1+Total_id+Total_id*Sig_Size+32;
		u8 Data_Size = Send_Data_buff[2];
		
		for(u8 i=0;i<Total_id;i++){
			Send_Data_buff[4+i] = ID_list[i+Packet_id*3];
		}
		
		for(u8 x=0;x<Total_id;x++){
			for(u8 y=0;y<Sig_Size;y++)
			Send_Data_buff[3 + 1 + Total_id + Sig_Size*x + y]=Sig_list[x+Packet_id*3][y];
		}
		
		for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Proposal_2_hash[m];
		
		sha2(Send_Data_buff,Data_Size+3,hash,0);
	
		//sign
		sign_data(hash,sig);
		
		for(u8 m = 0 ; m < Sig_Size ; m++)Send_Data_buff[Data_Size + 3 + m] = sig[m];

		Send_Data_buff[Data_Size + 3 + Sig_Size] = 0x00;
		
		Lora_Device_Sta = LORA_TX_STA;
		LoRa_SendData(Send_Data_buff);// id 0 "Hello LoRa"1
				
		delay_ms(Packet_Internal_delay);
	}
}
void Global_Leader_Catching(u8 PHASE, u8* ACK_list){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = GLOBAL_CATCHING;
	Send_Data_buff[2] = 1+N+32;
	Send_Data_buff[3] = PHASE;
//	for(u8 m=1;m<=N;m++) Send_Data_buff[3+m]=ACK_list[m];
	Send_Data_buff[4]=ACK_list[1];
	Send_Data_buff[5]=ACK_list[5];
	Send_Data_buff[6]=ACK_list[9];
	Send_Data_buff[7]=ACK_list[13];
	
	u8 Data_Size = Send_Data_buff[2];
	
	for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Proposal_2_hash[m];
	
	sha2(Send_Data_buff,Data_Size+3,hash,0);
	//sign
	sign_data(hash,sig);
	
	for(u8 m=0;m<Sig_Size;m++)	Send_Data_buff[m+Data_Size+3] = sig[m];
	
	Send_Data_buff[Data_Size+3+Sig_Size] = 0x00;
	
	Lora_Device_Sta = LORA_TX_STA;
	LoRa_SendData(Send_Data_buff);
}

void Global_Follower_Catching(u8 PHASE){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = GLOBAL_CATCHING;
	if(PHASE != GLOBAL_PREPREPARE)
		Send_Data_buff[2] = 1+32;
	else Send_Data_buff[2] = 1;
	Send_Data_buff[3] = PHASE;
//	for(u8 m=1;m<=N;m++) Send_Data_buff[3+m]=ACK_list[m];
	
	u8 Data_Size = Send_Data_buff[2];
	
	if(PHASE != GLOBAL_PREPREPARE)
		for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Proposal_2_hash[m];
	
	sha2(Send_Data_buff,Data_Size+3,hash,0);
	//sign
	sign_data(hash,sig);
	
	for(u8 m=0;m<Sig_Size;m++)	Send_Data_buff[m+Data_Size+3] = sig[m];
	
	Send_Data_buff[Data_Size+3+Sig_Size] = 0x00;
	
	Lora_Device_Sta = LORA_TX_STA;
	LoRa_SendData(Send_Data_buff);
}

