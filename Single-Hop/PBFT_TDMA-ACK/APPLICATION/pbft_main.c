#include "mw1268_app.h"
#include "mw1268_cfg.h"
#include "usart3.h"
#include "string.h"
#include "led.h"
#include "delay.h"
#include "pbft.h"
u8 N=13;
u8 f=30;
u16 DONE_THRES=200;
 
u8 id = 0;	//id = 0 => leader           1-2-3 => follower 
u8 NTX = 1;
u8 NTX_proposal = 1;

u16 slot = 800;

u8 Choice_Truth_Set_Number = 0;
u8 Choice_False_Set_Number = 0;

u8 Commit_Truth_Set_Number = 0;
u8 Commit_False_Set_Number = 0;


u8 Choice[13+1];
u8 Commit[13+1];

u8 Send_Data_buff[800]={0};//发送数据缓冲区

u8 PBFT_status = 0; //0->choice  1->commit  2->decided

u8 my_choice;
u8 my_commit;

//throughput
u8 Commands_Size;						//区块内指令数
u8 Proposal_Packet_Number;				// = Command_Size / 4 上取整
u8 Rece_Proposal[MAX_PROPOSAL_PACKETS];	//跟随者收到提案数据包
u8 Rece_Proposal_flag;					//标记是否集满所有数据包 0 没有  1 有
u8 Rece_Proposal_Number; 				//收到提案数据包总数
u8 Command_buff[800];					//指令缓冲区
u8 Proposal_hash[40];					//提案哈希
u8 start_flag;
u8 before_round;
u8 after_round;
u8 TIM_slot = 2;
u32 TIM5_Exceed_Times;
u32 TIM9_Exceed_Times;
u32 TIM10_Exceed_Times;
////////定向传输
u32 obj_addr;//记录用户输入目标地址
u8 obj_chn;//记录用户输入目标信道

u16 Dire_DateLen;
u16 Dire_Number;
u8 Dire_Rece_Data_buff[30][800];
u16 DireLen[30];

////////ACK
u8 ACK_Proposal[13+1];
u8 ACK_Choice[13+1];
u8 ACK_Commit[13+1];

u8 ACK_Proposal_Number;
u8 ACK_Choice_Number;
u8 ACK_Commit_Number;

u8 ACK_Proposal_Flag;
u8 ACK_Choice_Flag;
u8 ACK_Commit_Flag;

//u8 ACK_To_Send_Number;
//u8 ACK_To_Send[40][30];

u8 ACK_To_Send_Flag;
//u8 Rece_Proposal;

u32 TIM2_Exceed_Times;


const struct uECC_Curve_t * curves[5];

void Send_PREPARE(u8 choice);
void Send_COMMIT(u8 commit);
//void Go_ACK();
//void Start_TIMER(u8* ACK_number);
void Send_REPLAY(u32 time1, u32 time2);
void sign_data(u8* hash,u8* sig);
void New_Proposal(u8 command_size);
void Propose();
void Send_ACK();
void ACK_phase();
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
	
	for(u8 i = 0 ; i < 13+1 ; i++){
		Choice[i]=0xFF;
		Commit[i]=0xFF;
		
		ACK_Proposal[i] = 0xFF;
		ACK_Choice[i] = 0xFF;
		ACK_Commit[i] = 0xFF;
	}
	
	//throughput
	for(u8 m=0;m<MAX_PROPOSAL_PACKETS;m++) Rece_Proposal[m] = 0xFF;
	Proposal_Packet_Number = 0xFF;
	Rece_Proposal_flag = 0;
	Rece_Proposal_Number = 0;
	
	ACK_Proposal_Number = 0;
	ACK_Choice_Number = 0;
	ACK_Commit_Number = 0;
	ACK_To_Send_Flag = 0x00;
	start_flag=0;
//	Rece_Proposal = 0xFF;
}
void Start_main(){
	while(1){
		if(start_flag)break;
	}
}
void BeforeRound(){
	if(id==1){
		before_round = 0;
		after_round = 0xff;
	}
	while(before_round);
}
void AfterRound(){
	while(after_round);
}
u16 Packet_Internal_delay;

void DONE_OR_NOT(){

	if(RNG_Get_RandomRange(2,101) <= DONE_THRES){
		usart3_rx(0);
	}else{
		usart3_rx(1);
	}

}

void pbft(){
	btim_tim2_enable(ENABLE);

	///////小车参数设置，id////////
	id = 10;
	PBFT_status = 0xFF;
	u16 before_delay = (id)*slot; 
	u16 after_delay = (N-id)*slot;
	Packet_Internal_delay = 200;
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
	
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};

	/*****************************PREPREPARE**********************************/
	ACK_Proposal_Number = 1;
	ACK_Proposal[id] = 0x01;
	
	if(id == 1){
		//new proposal
		Commands_Size = 1;
		Proposal_Packet_Number = (Commands_Size + 3)/4;
		New_Proposal(Commands_Size);
	}
	
	Start_main();
	
	u8 Need_to_Wait = N;
	
	TIM2->CNT = 0;
	TIM2_Exceed_Times = 0;
	TIM9->CNT = 0;
	TIM9_Exceed_Times = 0;
	before_round = 0xff;
	after_round = 0;
	
//	TIM10_Exceed_Times = 0;
//	TIM10->CR1|=0x01;
//	DONE_OR_NOT();
	
	while(1){
		
		if(id == 1){
			if(ACK_Proposal_Number >= Need_to_Wait){	
				Rece_Proposal_flag = 1;
				break;
			}
		}
		else{
			while(1){
				
				if(Rece_Proposal_flag)break;
				
				BeforeRound();
				
				LED1(led1sta);led1sta = ~led1sta;
				
				if(Rece_Proposal_flag)break;
				
				AfterRound();
				
				
			}
			break;

		}
		BeforeRound();
		
		if(ACK_Proposal_Number >= Need_to_Wait){	
			Rece_Proposal_flag = 1;
			break;
		}
		
		Propose();
		
		AfterRound();
		
	}
	
	
	/*****************************PREPARE**********************************/
	ACK_Choice_Number = 1;
	Choice_Truth_Set_Number = Choice_Truth_Set_Number + 1;
	Choice[id] = 1;
	ACK_Choice[id] = 1;
	my_choice = 1;
	u8 prepare_flag = 0;
	while(1){
		
		if((ACK_Choice_Number >= Need_to_Wait) && (Choice_Truth_Set_Number >= 2*f+1))
			break;
		
		
		BeforeRound();
		
		if((ACK_Choice_Number >= Need_to_Wait) && (Choice_Truth_Set_Number >= 2*f+1)) break;
		
		//ACK_phase
		if(ACK_To_Send_Flag && prepare_flag){
			Send_ACK();
			ACK_To_Send_Flag = 0;
			AfterRound();
			continue;
		}
		
		if((ACK_Choice_Number >= Need_to_Wait) && (Choice_Truth_Set_Number >= 2*f+1)) break;
		
		if(ACK_Choice_Number >= Need_to_Wait){	
			AfterRound();
			continue;
		}
		
		if((ACK_Choice_Number >= Need_to_Wait) && (Choice_Truth_Set_Number >= 2*f+1)) break;
		
		Send_PREPARE(my_choice);
		prepare_flag = 1;
		
		if((ACK_Choice_Number >= Need_to_Wait) && (Choice_Truth_Set_Number >= 2*f+1)) break;
		
		AfterRound();
	}
	
	
	/*****************************COMMIT**********************************/
	ACK_Commit_Number = ACK_Commit_Number + 1;
	Commit_Truth_Set_Number = Commit_Truth_Set_Number + 1;
	Commit[id] = 1;
	ACK_Commit[id] = 1;
	my_commit = 1;
	u8 commit_flag = 0;
	while(1){
		if((ACK_Commit_Number >= Need_to_Wait) && (Commit_Truth_Set_Number >= 2*f+1))break;
		
		BeforeRound();
		
		if((ACK_Commit_Number >= Need_to_Wait) && (Commit_Truth_Set_Number >= 2*f+1))break;
		
		if(ACK_To_Send_Flag && commit_flag){
			Send_ACK();
			ACK_To_Send_Flag = 0;
			AfterRound();
			continue;
		}
			
		if((ACK_Commit_Number >= Need_to_Wait) && (Commit_Truth_Set_Number >= 2*f+1))break;
		
		if(ACK_Commit_Number >= Need_to_Wait){
			AfterRound();
			continue;
		} 
		
		if((ACK_Commit_Number >= Need_to_Wait) && (Commit_Truth_Set_Number >= 2*f+1))break;
		
		Send_COMMIT(my_commit);
		commit_flag = 1;
		
		if((ACK_Commit_Number >= Need_to_Wait) && (Commit_Truth_Set_Number >= 2*f+1))break;
		
		AfterRound();
		
	}
	
	u32 Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
	
	u8 Temp_Times;
	Temp_Times=0;
	
	while(1){
		
		BeforeRound();
		
		if(ACK_To_Send_Flag){
			Send_ACK();
			ACK_To_Send_Flag = 0;
		}
		
		Temp_Times = Temp_Times + 1;
		if(Temp_Times >= 2){
			Temp_Times=0;
			
			Send_REPLAY(Total_time,TIM2_Exceed_Times);
					
		}

		AfterRound();
		
	}
	
	
}

//module

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
void Propose(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};	
	for(u8 packet_id=1;packet_id<=Proposal_Packet_Number;packet_id++){
		Send_Data_buff[0] = id;
		Send_Data_buff[1] = PROPOSAL;
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
	Send_Data_buff[2] = 1+N+32;
	Send_Data_buff[3] = choice;
	for(u8 m=1;m<=N;m++)Send_Data_buff[3+m] = ACK_Choice[m];
	
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
	Send_Data_buff[2] = 1 + N + 32;
	Send_Data_buff[3] = commit;
	for(u8 m=1;m<=N;m++)Send_Data_buff[3+m] = ACK_Commit[m];
	
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

void ACK_phase(){

}

void Send_ACK(){
	
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = ACK;
	Send_Data_buff[2] = 2*N + 1 + 32;
	Send_Data_buff[3] = Rece_Proposal_flag;
	for(u8 m=1;m<=N;m++)Send_Data_buff[3+m] = Choice[m];
	for(u8 m=1;m<=N;m++)Send_Data_buff[3+m+N] = Commit[m];
	
	u8 Data_Size = Send_Data_buff[2];
	
	for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Proposal_hash[m];
	
	sha2(Send_Data_buff,Data_Size+3,hash,0);
	
	//sign
	sign_data(hash,sig);

	for(u8 m=0;m<Sig_Size;m++)	Send_Data_buff[m+Data_Size+3] = sig[m];
	
	
	Send_Data_buff[Data_Size + 3 + Sig_Size] = 0x00;
			
	Lora_Device_Sta = LORA_TX_STA;
	LoRa_SendData(Send_Data_buff);// id 0 "Hello LoRa"
	
}

void Send_REPLAY(u32 time1, u32 time2){
	LoRa_CFG.chn = Msg_channel;
	LoRa_Set();
	
	Send_Data_buff[0] = id + '0';
	sprintf((char*)Send_Data_buff+ sizeof(u8),"I am done!%d||%d",time1,time2);
	
	Lora_Device_Sta = LORA_TX_STA;
	LoRa_SendData(Send_Data_buff);
	
	LoRa_CFG.chn = Data_channel;
	LoRa_Set();
}


