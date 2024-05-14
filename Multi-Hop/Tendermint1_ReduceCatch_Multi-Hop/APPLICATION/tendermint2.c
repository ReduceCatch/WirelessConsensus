#include "mw1268_app.h"
#include "mw1268_cfg.h"
#include "usart3.h"
#include "string.h"
#include "led.h"
#include "delay.h"
#include "tendermint2.h"

u8 N=13;
u8 f=20;
u16 DONE_THRES=200;
 
u8 id = 0;	//id = 0 => leader           1-2-3 => follower 
u8 NTX = 1;
u8 NTX_proposal = 5;

u8 phase_NTX = 3;
u8 phase_NTX2 = 2;

u16 slot = 800;

u8 Choice_Truth_Set_Number = 0;
u8 Choice_False_Set_Number = 0;

u8 Commit_Truth_Set_Number = 0;
u8 Commit_False_Set_Number = 0;


u8 Choice[16+1];
u8 Commit[16+1];

//Global 
u8 Global_NTX_proposal = 3;
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


u8 Global_Choice[16+1];
u8 Global_Commit[16+1];
u8 Global_ACK_Proposal[16+1];
u8 Global_ACK_Choice[16+1];
u8 Global_ACK_Commit[16+1];
u8 Global_ACK_Proposal_Number;
u8 Global_ACK_Choice_Number;
u8 Global_ACK_Commit_Number;
u8 Global_Choice_Truth_Set_Number = 0;
u8 Global_Choice_False_Set_Number = 0;
u8 Global_Commit_Truth_Set_Number = 0;
u8 Global_Commit_False_Set_Number = 0;
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

u8 Global_Rece_next;
u8 Global_ACK_viewchange;
u8 Global_Viewchange_Number;
u8 Global_Rece_Viewchange[20];
u8 Global_Viewchange_id[20];
u8 Global_Viewchange_Sig[20][64];

u8 Global_Need_Proposal_Flag=0;
u8 Global_Need_Choice_Flag=0;
u8 Global_Need_Commit_Flag=0;
u8 Global_Need_VIEWCHANGE=0;
u8 Global_Need_next=0;

u8 NTX_proposal_flag = 0xff;
u8 Global_NTX_proposal_flag = 0xff;

void Send_Sig();
void Global_Send_PREPARE(u8 choice);
void Global_Send_COMMIT(u8 commit);
void Global_Send_Ready();
void Global_Start();
void Global_ACK_phase();
void Global_Go_ACK();
void Global_TIMER();
void Global_New_Proposal();
void Global_Propose();
void Global_Leader_Send(u8 PHASE, u8* ID_list, u8 Sig_list[20][64]);
void Global_Send_VIEW_CHANGE();
//void Global_NACK_phase();
void Global_Catching_PREPREPARE();
void Global_Catching_PREPARE();
void Global_Catching_COMMIT();
void Global_Leader_Catching(u8 PHASE, u8* ACK_list);
void Global_Follower_Catching(u8 PHASE);


/////////viewchange/////////
u8 Rece_next;
u8 Viewchange_Number;
//存目标节点的2f+1个签名
u8 Rece_Viewchange[20];//目标节点目前收到了几个签名
u8 Viewchange_id[20];
u8 Viewchange_Sig[20][64];
u8 ACK_viewchange;

u8 Need_Proposal_Flag;
u8 Need_Choice_Flag;
u8 Need_Commit_Flag;
u8 Need_VIEWCHANGE;
u8 Need_next;
///////////

u8 Send_Data_buff[800]={0};//发送数据缓冲区

u8 PBFT_status = 0; //0->choice  1->commit  2->decided

u8 my_choice;
u8 my_commit;

////////定向传输
u32 obj_addr;//记录用户输入目标地址
u8 obj_chn;//记录用户输入目标信道

//throughput
u8 Commands_Size;						//区块内指令数
u8 Proposal_Packet_Number;				// = Command_Size / 4 上取整
u8 Rece_Proposal[MAX_PROPOSAL_PACKETS];	//跟随者收到提案数据包
u8 Rece_Proposal_flag;					//标记是否集满所有数据包 0 没有  1 有
u8 Rece_Proposal_Number; 				//收到提案数据包总数
u8 Command_buff[800];					//指令缓冲区
u8 Proposal_hash[40];					//提案哈希
u8 start_flag;
u8 Timer_Flag_5;
u32 Total_time=0;
u32 Total_time1=0;
u32 TIM2_Exceed_Times;
u8 before_round;
u8 after_round;
u8 TIM_slot = 2;
u32 TIM5_Exceed_Times;
u32 TIM9_Exceed_Times;
u32 TIM10_Exceed_Times;

////////ACK
u8 ACK_Proposal[16+1];
u8 ACK_Choice[16+1];
u8 ACK_Commit[16+1];

u8 ACK_Proposal_Number;
u8 ACK_Choice_Number;
u8 ACK_Commit_Number;

u8 ACK_Proposal_Flag;
u8 ACK_Choice_Flag;
u8 ACK_Commit_Flag;

//u8 ACK_To_Send_Number;
//u8 ACK_To_Send[40][30];

u8 ACK_To_Send_Flag;
u8 ACK_To_Send_VC_Flag;

u8 Need_to_Wait;
u32 delta=5000;

const struct uECC_Curve_t * curves[5];

void Send_PREPARE(u8 choice);
void Send_COMMIT(u8 commit);
void Go_ACK();
void Start_TIMER();
void Send_REPLAY(u32 time1, u32 time2, u32 time3);
void sign_data(u8* hash,u8* sig);
void New_Proposal(u8 command_size);
void Propose(u8 idx);
void Send_VIEW_CHANGE();
//void Leader_Send(u8 PHASE, u8* ID_list, u8 Sig_list[20][64]);
void Sleep_Random();
void ACK_phase();
void Catching_PREPREPARE();
void Catching_PREPARE();
void Catching_COMMIT();
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
	for(u8 i = 0 ; i < 13+1 ; i++){
		Choice[i]=0xFF;
		Commit[i]=0xFF;

		ACK_Proposal[i] = 0xFF;
		ACK_Choice[i] = 0xFF;
		ACK_Commit[i] = 0xFF;
	}
	
	//viewchange
	for(u8 i=0;i<20;i++){
//		Rece_Commit[i] = 0xFF;
		Rece_Viewchange[i] = 0xFF;
	}
	Viewchange_Number=1;
	Rece_next = 0;

	ACK_To_Send_VC_Flag = 0;
	ACK_viewchange = 0;
	//////////////////////////////////////////////////////
	
	ACK_Proposal_Number = 0;
	ACK_Choice_Number = 0;
	ACK_Commit_Number = 0;
	ACK_To_Send_Flag = 0x00;
//	Rece_Proposal = 0xFF;
	
	//throughput
	for(u8 m=0;m<MAX_PROPOSAL_PACKETS;m++) Rece_Proposal[m] = 0xFF;
	Proposal_Packet_Number = 0xFF;
	Rece_Proposal_flag = 0;
	Rece_Proposal_Number = 0;
	
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
//		Global_Rece_Proposal_flag[m] = 0;
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
	for(u8 i = 0 ; i < 16+1 ; i++){
		Global_Choice[i]=0xFF;
		Global_Commit[i]=0xFF;
		
		Global_ACK_Proposal[i] = 0xFF;
		Global_ACK_Choice[i] = 0xFF;
		Global_ACK_Commit[i] = 0xFF;
	}
	
	for(u8 i=0;i<20;i++){
		Global_Rece_Viewchange[i] = 0xFF;
	}
	Global_ACK_viewchange = 0x00;
	Global_Viewchange_Number = 1;
	Global_Rece_next = 0;
}





void Start_main(){
	while(1){
		if(start_flag)break;
	}
}
u16 Packet_Internal_delay;
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
	while(after_round);
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

void tendermint2(){
	btim_tim2_enable(ENABLE);
	
	///////小车参数设置，id////////
	id = 13;
	if(id >= 1 && id <= 4){Group_Number = 1;Group_Leader_ID = 1;}
	else if(id >= 5 && id <= 8){Group_Number = 2;Group_Leader_ID = 5;}
	else if(id >= 9 && id <= 12){Group_Number = 3;Group_Leader_ID = 9;}
	else if(id >= 13 && id <= 16){Group_Number = 4;Group_Leader_ID = 13;}
	
	Packet_Internal_delay = 210;
	//////////////
	led0sta = 0;
	led1sta = 0;
	led2sta = 0;
	LED0(1);LED1(1);LED2(1);
	init();
	init_public_key(); 
	//数据类型
	//0 : id 	1 2 3 4
	//1 : data_type => 1->PROPOSAL 2->CHOICE 3->COMMIT 4->ACK
	//2-29 : data_body  （ACK 则 2表示是哪个类型的数据1->propose 2->choice 3->commit）//多轮共识这里需要修改
	
	//leader : proposal
/*****************************Local_Consensus**************************************************************************************************************************************************************************/

	/********************Preprepare**********************************/
	u8 id_idx = id - 4 * (Group_Number - 1);
	if(id==Group_Leader_ID){	
		Commands_Size = 1;
		Proposal_Packet_Number = (Commands_Size + 3)/4;
		
		New_Proposal(Commands_Size);
	}
	if(id == Group_Leader_ID)Rece_Proposal_flag=1;

	Start_main();
	Change_channel(Group_Number * 10);
	Need_to_Wait = N;
	
	TIM2->CNT = 0;
	TIM2_Exceed_Times = 0;
	
	TIM5_Exceed_Times = 0;
	
	TIM9->CNT = 0;
	TIM9_Exceed_Times = 0;
	before_round = 0xff;
	after_round = 0;
	
	for(u8 ntx=0;ntx<NTX_proposal;ntx++){
		
		while(1){
			if(TIM9_Exceed_Times==2 * ntx)break;
		}
		
		if(id==Group_Leader_ID)
			Propose(0);
		else{
			if(Rece_Proposal_flag)	break;
		}

	}
	
	while(TIM9_Exceed_Times != 2 * NTX_proposal);
	
	TIM9_Exceed_Times = 0;
	before_round = 0xff;
	after_round = 0;
	
	/************************Prepare*****************************/
	ACK_Choice_Number = ACK_Choice_Number + 1;
	Choice_Truth_Set_Number = Choice_Truth_Set_Number + 1;
	Choice[id_idx] = 1;
	ACK_Choice[id_idx] = 1;
	my_choice = 1;
	for(u8 ntx=0;ntx<NTX;ntx++){
		
		BeforeRound();
		
		if(Rece_Proposal_flag){
			for(u8 m=0;m<phase_NTX;m++){
				Send_PREPARE(my_choice);
				delay_ms(Packet_Internal_delay);
			}
		}
			
		AfterRound();

	}
	
	/******************Commit***************************/
	ACK_Commit_Number = ACK_Commit_Number + 1;
	Commit_Truth_Set_Number = Commit_Truth_Set_Number + 1;
	Commit[id_idx] = 1;
	ACK_Commit[id_idx] = 1;
	my_commit=1;
	for(u8 ntx=0;ntx<NTX;ntx++){
		
		BeforeRound();
		
		if(Choice_Truth_Set_Number >= 2*f+1 && Rece_Proposal_flag){
			for(u8 m=0;m<phase_NTX;m++){
				Send_COMMIT(1);
				delay_ms(Packet_Internal_delay);
			}
		}
		
		AfterRound();
		
	}
	
	if(Commit_Truth_Set_Number >= 2*f+1 && Total_time1==0 && Rece_Proposal_flag)
		Total_time1 = TIM2->CNT + TIM2_Exceed_Times * 9000;
	/**********************view_change*********************************/
	
	for(u8 ntx=0;ntx<NTX;ntx++){
		
		BeforeRound();
		
		if(id != Group_Leader_ID && Commit_Truth_Set_Number >= 2*f+1){
			//vc msg
			for(u8 m=0;m<phase_NTX;m++){
				Send_VIEW_CHANGE();
				delay_ms(Packet_Internal_delay);
			}
			
		}
		
		if(id==Group_Leader_ID && Viewchange_Number == Need_to_Wait  && Rece_Proposal_flag){

			Leader_Send(NEXT_ROUND,Viewchange_id,Viewchange_Sig);
		
		}
		
		AfterRound();
	}
	
	if(id==Group_Leader_ID){
		delay_ms((N-Need_to_Wait)*delta);
	}
	
	u8 my_flag=0;
	for(u8 ntx=0;ntx<NTX;ntx++){

		if(Rece_next==1)break;
		
		if(id==Group_Leader_ID && Viewchange_Number == Need_to_Wait){

			Leader_Send(NEXT_ROUND,Viewchange_id,Viewchange_Sig);
			
		}else{
			delay_ms(1500);
		}
		
	}
	
	/**********************CATCHING*********************************/
	
	my_choice = 1;
	my_commit = 1;
	u8 my_decide = 1;
	while(1){
		
		if(id == Group_Leader_ID && Viewchange_Number >= Need_to_Wait && Rece_Proposal_flag)break;
		
		if(id==Group_Leader_ID && Need_Proposal_Flag){
			
			Propose(0);
			Need_Proposal_Flag = 0;
			delay_ms(Packet_Internal_delay);
		}
		
		if(!Rece_Proposal_flag){
			for(u8 m=0;m<phase_NTX2;m++){
				Catching_PREPREPARE();
				delay_ms(Packet_Internal_delay);
			}
		}
		
		if(Need_Choice_Flag && Rece_Proposal_flag){
			for(u8 m=0;m<phase_NTX2;m++){
				Send_PREPARE(my_choice);
				delay_ms(Packet_Internal_delay);
			}
			Need_Choice_Flag = 0;
		}

		if(Need_Commit_Flag && (Choice_Truth_Set_Number >= 2*f+1)){
			for(u8 m=0;m<phase_NTX2;m++){
				Send_COMMIT(my_commit);	
				delay_ms(Packet_Internal_delay);
			}
			
			Need_Commit_Flag = 0;
		}
		
		if(Choice_Truth_Set_Number < 2*f+1 && Rece_Proposal_flag){
			for(u8 m=0;m<phase_NTX2;m++){
				Catching_PREPARE();
				delay_ms(Packet_Internal_delay);
			}
		}
		
		if((Choice_Truth_Set_Number >= 2*f+1) && (Commit_Truth_Set_Number < 2*f+1)){
			for(u8 m=0;m<phase_NTX2;m++){
				Catching_COMMIT();
				delay_ms(Packet_Internal_delay);
			}
		}
		
		if(id == Group_Leader_ID && Commit_Truth_Set_Number >= 2*f+1){
			
			if(Need_next && (Viewchange_Number >= Need_to_Wait)){
				Need_next = 0;
				
				Leader_Send(NEXT_ROUND,Viewchange_id,Viewchange_Sig);
			}
			
			if(Viewchange_Number < Need_to_Wait){
				if(Total_time1 == 0) Total_time1 = TIM2->CNT + TIM2_Exceed_Times * 9000;
				
				for(u8 m=0;m<phase_NTX2;m++){
					Leader_Catching(DECIDE,Rece_Viewchange);
					delay_ms(Packet_Internal_delay);
				}
			
			}
			else if(Viewchange_Number >= Need_to_Wait && Rece_Proposal_flag){
				break;
			}
			
		}
		if(id != Group_Leader_ID && Commit_Truth_Set_Number >= 2*f+1){
			if(Need_VIEWCHANGE){
				Need_VIEWCHANGE = 0;
				for(u8 m=0;m<phase_NTX2;m++){
					Send_VIEW_CHANGE();
					delay_ms(Packet_Internal_delay);
				}
			}
			if(Rece_next != 1){
				if(Total_time1 == 0) Total_time1 = TIM2->CNT + TIM2_Exceed_Times * 9000;
				for(u8 m=0;m<phase_NTX2;m++){
					Follower_Catching(NEXT_ROUND);
					delay_ms(Packet_Internal_delay);
				}
			
			}else if(Rece_next == 1  && Rece_Proposal_flag){
				
			}
			
		}
		u8 ACK_tmp=0;
		Start_TIMER(&ACK_tmp,40);
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
		
		u8 tmp_ACK=0;
		Start_TIMER(&tmp_ACK,Need_to_Wait);
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
	/********************Preprepare**********************************/
	id_idx = Group_Number;
	TIM9_Exceed_Times = 0;
	if(id == Global_Consensus_Leader){
		Global_New_Proposal();
		Global_Propose();
		Rece_Proposal_2_flag=1;
	}

	before_round = 0x00;
	after_round = 0xFF;
	
	for(u8 ntx=0;ntx<Global_NTX_proposal;ntx++){
		
		while(1){
			if(TIM9_Exceed_Times==2 * ntx)break;
		}
		
		if(id==Global_Consensus_Leader)
			Global_Propose();
		else{
			if(Rece_Proposal_2_flag)	break;
		}

	}
	
	while(TIM9_Exceed_Times != 2 * Global_NTX_proposal);
	
	TIM9_Exceed_Times = 0;
	before_round = 0xff;
	after_round = 0;
	
	/************************Prepare*****************************/
	Global_ACK_Choice_Number = Global_ACK_Choice_Number + 1;
	Global_Choice_Truth_Set_Number = Global_Choice_Truth_Set_Number + 1;
	Global_Choice[id_idx] = 1;
	Global_ACK_Choice[id_idx] = 1;
	my_choice = 1;
	for(u8 ntx=0;ntx<Global_NTX;ntx++){
		
		BeforeRound();
		
		if(Rece_Proposal_2_flag){
			for(u8 m=0;m<phase_NTX;m++){
				Global_Send_PREPARE(1);
				delay_ms(Packet_Internal_delay);
			}
		}
			
		AfterRound();

	}
	
	/******************Commit***************************/
	Global_ACK_Commit_Number = Global_ACK_Commit_Number + 1;
	Global_Commit_Truth_Set_Number = Global_Commit_Truth_Set_Number + 1;
	Global_Commit[id_idx] = 1;
	Global_ACK_Commit[id_idx] = 1;
	my_commit=1;
	for(u8 ntx=0;ntx<Global_NTX;ntx++){
		
		BeforeRound();
		
		if(Global_Choice_Truth_Set_Number >= 2*Global_f+1 && Rece_Proposal_2_flag){
			for(u8 m=0;m<phase_NTX;m++){
				Global_Send_COMMIT(1);
				delay_ms(Packet_Internal_delay);
			}
		}
		
		AfterRound();
		
	}

	/**********************view_change*********************************/
	
	for(u8 ntx=0;ntx<Global_NTX;ntx++){
		
		BeforeRound();
		
		if(id != Global_Consensus_Leader && Global_Commit_Truth_Set_Number >= 2*Global_f+1){
			//vc msg
			for(u8 m=0;m<phase_NTX;m++){
				Global_Send_VIEW_CHANGE();
				delay_ms(Packet_Internal_delay);
			}
			
		}
		
		if(id==Global_Consensus_Leader && Global_Viewchange_Number == GROUPS_NUMBER){
			if(Total_time == 0 ){
				Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
			}
			Global_Leader_Send(GLOBAL_NEXT_ROUND,Global_Viewchange_id,Global_Viewchange_Sig);
		
		}
		
		AfterRound();
	}
	
	if(id==Global_Consensus_Leader && Total_time == 0 && Viewchange_Number == Need_to_Wait){
		Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
	}
	
	if(id==Global_Consensus_Leader){
		delay_ms((N-Need_to_Wait)*delta);
	}
	
	my_flag=0;
	for(u8 ntx=0;ntx<NTX;ntx++){

		if(Global_Rece_next==1)break;
		
		if(id==Global_Consensus_Leader && Global_Viewchange_Number == GROUPS_NUMBER){
			if(Total_time == 0) Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
			Global_Leader_Send(GLOBAL_NEXT_ROUND,Global_Viewchange_id,Global_Viewchange_Sig);
			
		}else{
			delay_ms(1500);
		}
		
	}
	
	if(id != Global_Consensus_Leader && Global_Rece_next==1 && Total_time==0  && Rece_Proposal_2_flag){
		Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
	}
	
	/**********************CATCHING*********************************/
	
	my_choice = 1;
	my_commit = 1;
	my_decide = 1;
	while(1){
		test_finish();
		if(id==Global_Consensus_Leader && Global_Need_Proposal_Flag){
			
			Global_Propose();
			Global_Need_Proposal_Flag = 0;
			delay_ms(Packet_Internal_delay);
		}
		test_finish();
		if(!Rece_Proposal_2_flag){
			for(u8 m=0;m<phase_NTX2;m++){
				Global_Catching_PREPREPARE();
				delay_ms(Packet_Internal_delay);
			}
		}
		test_finish();
		if(Global_Need_Choice_Flag && Rece_Proposal_2_flag){
			for(u8 m=0;m<phase_NTX2;m++){
				Global_Send_PREPARE(1);
				delay_ms(Packet_Internal_delay);
			}
			Global_Need_Choice_Flag = 0;
		}
		test_finish();
		if(Global_Need_Commit_Flag && (Global_Choice_Truth_Set_Number >= 2*Global_f+1)){
			for(u8 m=0;m<phase_NTX2;m++){
				Global_Send_COMMIT(1);	
				delay_ms(Packet_Internal_delay);
			}
			Global_Need_Commit_Flag = 0;
		}
		test_finish();
		if(Global_Choice_Truth_Set_Number < 2*Global_f+1 && Rece_Proposal_2_flag){
			for(u8 m=0;m<phase_NTX2;m++){
				Global_Catching_PREPARE();
				delay_ms(Packet_Internal_delay);
			}
		}
		test_finish();
		if((Global_Choice_Truth_Set_Number >= 2*Global_f+1) && (Global_Commit_Truth_Set_Number < 2*Global_f+1)){
			for(u8 m=0;m<phase_NTX2;m++){
				Global_Catching_COMMIT();
				delay_ms(Packet_Internal_delay);
			}
		}
		test_finish();
		if(id == Global_Consensus_Leader && Global_Commit_Truth_Set_Number >= 2*Global_f+1){
			
			if(Global_Need_next && (Global_Viewchange_Number >= GROUPS_NUMBER)){
				Global_Need_next = 0;
				if(Total_time == 0) Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
				Global_Leader_Send(GLOBAL_NEXT_ROUND,Global_Viewchange_id,Global_Viewchange_Sig);
			}
			
			if(Global_Viewchange_Number < GROUPS_NUMBER){
				if(Total_time1 == 0) Total_time1 = TIM2->CNT + TIM2_Exceed_Times * 9000;
				
				for(u8 m=0;m<phase_NTX2;m++){
					Global_Leader_Catching(GLOBAL_DECIDE,Global_Rece_Viewchange);
					delay_ms(Packet_Internal_delay);
				}
			
			}
			else if(Global_Viewchange_Number >= GROUPS_NUMBER && Rece_Proposal_2_flag){
				if(Total_time == 0) Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
				
				Send_REPLAY(Total_time1,Total_time,TIM2_Exceed_Times);
				
				delay_ms(Packet_Internal_delay);
			
			}
			
		}
		if(id != Global_Consensus_Leader && Global_Commit_Truth_Set_Number >= 2*Global_f+1){
			if(Global_Need_VIEWCHANGE){
				Global_Need_VIEWCHANGE = 0;
				for(u8 m=0;m<phase_NTX2;m++){
					Global_Send_VIEW_CHANGE();
					delay_ms(Packet_Internal_delay);
				}
			}
			if(Global_Rece_next != 1){
				if(Total_time1 == 0) Total_time1 = TIM2->CNT + TIM2_Exceed_Times * 9000;
				for(u8 m=0;m<phase_NTX2;m++){
					Global_Follower_Catching(GLOBAL_NEXT_ROUND);
					delay_ms(Packet_Internal_delay);
				}
			
			}else if(Global_Rece_next == 1  && Rece_Proposal_2_flag){
				if(Total_time == 0) Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
				
				Send_REPLAY(Total_time1,Total_time,TIM2_Exceed_Times);
				
				delay_ms(Packet_Internal_delay);
			}
			
		}
		u8 ACK_tmp=0;
		Start_TIMER(&ACK_tmp,40);
	}
	
}


//module
void Sleep_Random(){
	u32 random = RNG_Get_RandomRange(1,N);
	random = random * slot;
	delay_ms(random);
}
void test_finish(){
	if(Global_Viewchange_Number >= GROUPS_NUMBER && Total_time == 0) 
		Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
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

void Send_PREPARE(u8 choice){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};	
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = CHOICE;
	Send_Data_buff[2] = 1+32;
	Send_Data_buff[3] = choice;
//	for(u8 m=1;m<=N;m++)Send_Data_buff[3+m] = ACK_Choice[m];
	
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

void Send_COMMIT(u8 commit){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = COMMIT;
	Send_Data_buff[2] = 1+32;
	Send_Data_buff[3] = commit;
//	for(u8 m=1;m<=N;m++)Send_Data_buff[3+m] = ACK_Commit[m];
	
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
	//发送N-1个签名,分为（N-1）/3次
	u8 send_number = ((Need_to_Wait-1)+2)/3;
	for(u8 Packet_id=0;Packet_id<send_number;Packet_id++){
		Send_Data_buff[0] = id;
		Send_Data_buff[1] = PHASE;
		if(Packet_id != send_number-1 )Send_Data_buff[3] = 3;
		else Send_Data_buff[3] = (Need_to_Wait-1)-Packet_id*3;
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

void Send_VIEW_CHANGE(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = VIEW_CHANGE;
	Send_Data_buff[2] = 1 + 32;
	Send_Data_buff[3] = 1;
//	for(u8 m=1;m<=N;m++)Send_Data_buff[3+m] = ACK_Commit[m];
	
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

void Start_TIMER(u8* ACK_number,u8 Thres_number){
	//打开定时器
	Timer_Flag_5 = 0xFF;
	TIM5->CNT = 0;
	TIM5->CR1|=0x01;
	while(Timer_Flag_5){
		if(*ACK_number >= Thres_number)break;
	}
	//关闭定时器
	TIM5->CR1|=0x00;
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


void Catching_PREPREPARE(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = CATCHING;
	Send_Data_buff[2] = 1;
	Send_Data_buff[3] = PROPOSAL;
			
	u8 Data_Size = Send_Data_buff[2];
	
//	for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Proposal_hash[m];
	
	sha2(Send_Data_buff,Data_Size+3,hash,0);
	//sign
	sign_data(hash,sig);
	
	for(u8 m=0;m<Sig_Size;m++)	Send_Data_buff[m+Data_Size+3] = sig[m];
	
	Send_Data_buff[Data_Size+3+Sig_Size] = 0x00;
	
	Lora_Device_Sta = LORA_TX_STA;
	LoRa_SendData(Send_Data_buff);
}

void Catching_PREPARE(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = CATCHING;
	Send_Data_buff[2] = 1+N+32;
	Send_Data_buff[3] = CHOICE;
	for(u8 m=1;m<=N;m++) Send_Data_buff[3+m]=Choice[m];
			
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

void Catching_COMMIT(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = CATCHING;
	Send_Data_buff[2] = 1+N+32;
	Send_Data_buff[3] = COMMIT;
	for(u8 m=1;m<=N;m++) Send_Data_buff[3+m]=Commit[m];
			
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



void Leader_Catching(u8 PHASE, u8* ACK_list){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = CATCHING;
	Send_Data_buff[2] = 1+N+32;
	Send_Data_buff[3] = PHASE;
	for(u8 m=1;m<=N;m++) Send_Data_buff[3+m]=ACK_list[m];
	
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
	Send_Data_buff[2] = 1+32;
	Send_Data_buff[3] = PHASE;
//	for(u8 m=1;m<=N;m++) Send_Data_buff[3+m]=ACK_list[m];
	
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
	u8 send_number = ((Need_to_Wait-1)+2)/3;
	for(u8 Packet_id=0;Packet_id<send_number;Packet_id++){
		Send_Data_buff[0] = id;
		Send_Data_buff[1] = GLOBAL_SIG;
		if(Packet_id != send_number-1 )Send_Data_buff[3] = 3;
		else Send_Data_buff[3] = (Need_to_Wait-1)-Packet_id*3;
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

void Global_Send_PREPARE(u8 choice){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};	
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = GLOBAL_CHOICE;
	Send_Data_buff[2] = 1+32;
	Send_Data_buff[3] = choice;
//	for(u8 m=1;m<=N;m++)Send_Data_buff[3+m] = Global_ACK_Choice[m];
	
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


void Global_Send_COMMIT(u8 commit){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = GLOBAL_COMMIT;
	Send_Data_buff[2] = 1 + 32;
	Send_Data_buff[3] = commit;
//	for(u8 m=1;m<=N;m++)Send_Data_buff[3+m] = Global_ACK_Commit[m];
	
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

void Global_Send_VIEW_CHANGE(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = GLOBAL_VIEW_CHANGE;
	Send_Data_buff[2] = 1 + 32;
	Send_Data_buff[3] = 1;
//	for(u8 m=1;m<=N;m++)Send_Data_buff[3+m] = ACK_Commit[m];
	
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
	//发送N-1个签名,分为（N-1）/3次
	u8 send_number = ((GROUPS_NUMBER-1)+2)/3;
	for(u8 Packet_id=0;Packet_id<send_number;Packet_id++){
		Send_Data_buff[0] = id;
		Send_Data_buff[1] = PHASE;
		if(Packet_id != send_number-1 )Send_Data_buff[3] = 3;
		else Send_Data_buff[3] = (GROUPS_NUMBER-1)-Packet_id*3;
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

void Global_Catching_PREPREPARE(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = GLOBAL_CATCHING;
	Send_Data_buff[2] = 1;
	Send_Data_buff[3] = GLOBAL_PROPOSAL_2;
			
	u8 Data_Size = Send_Data_buff[2];
	
//	for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Proposal_hash[m];
	
	sha2(Send_Data_buff,Data_Size+3,hash,0);
	//sign
	sign_data(hash,sig);
	
	for(u8 m=0;m<Sig_Size;m++)	Send_Data_buff[m+Data_Size+3] = sig[m];
	
	Send_Data_buff[Data_Size+3+Sig_Size] = 0x00;
	
	Lora_Device_Sta = LORA_TX_STA;
	LoRa_SendData(Send_Data_buff);
}

void Global_Catching_PREPARE(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = GLOBAL_CATCHING;
	Send_Data_buff[2] = 1+GROUPS_NUMBER+32;
	Send_Data_buff[3] = GLOBAL_CHOICE;
	for(u8 m=1;m<=N;m++) Send_Data_buff[3+m]=Global_Choice[m];
			
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

void Global_Catching_COMMIT(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = GLOBAL_CATCHING;
	Send_Data_buff[2] = 1+GROUPS_NUMBER+32;
	Send_Data_buff[3] = GLOBAL_COMMIT;
	for(u8 m=1;m<=N;m++) Send_Data_buff[3+m]=Global_Commit[m];
			
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



void Global_Leader_Catching(u8 PHASE, u8* ACK_list){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = GLOBAL_CATCHING;
	Send_Data_buff[2] = 1+GROUPS_NUMBER+32;
	Send_Data_buff[3] = PHASE;
	for(u8 m=1;m<=N;m++) Send_Data_buff[3+m]=ACK_list[m];
	
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
	Send_Data_buff[2] = 1+32;
	Send_Data_buff[3] = PHASE;
	
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






//


