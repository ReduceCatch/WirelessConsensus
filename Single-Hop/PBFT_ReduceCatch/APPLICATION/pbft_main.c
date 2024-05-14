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

u8 id = 0;	//id = 1 => leader           2-3-4 => follower 
u8 NTX = 1;
u8 NTX_proposal = 3;
u8 NTX_proposal_flag = 0xff;
u8 phase_NTX = 3;

u16 slot = 800;

u8 Choice_Truth_Set_Number = 0;
u8 Choice_False_Set_Number = 0;

u8 Commit_Truth_Set_Number = 0;
u8 Commit_False_Set_Number = 0;

u8 Choice[13+1];
u8 Commit[13+1];

u8 Send_Data_buff[800]={0};//发送数据缓冲区

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
u8 before_round;
u8 after_round;
u8 TIM_slot = 2;
u32 TIM9_Exceed_Times;
u32 TIM10_Exceed_Times;

////////定向传输
u32 obj_addr;//记录用户输入目标地址
u8 obj_chn;//记录用户输入目标信道


u8 Timer_Flag_5;

const struct uECC_Curve_t * curves[5];

u32 TIM2_Exceed_Times;
u32 TIM5_Exceed_Times;
//Catching up
u8 Need_Choice_Flag;
u8 Need_Commit_Flag;
u8 Need_Proposal_Flag;

u8 decided;
u32 Total_time;

void Send_PREPARE(u8 choice);
void Send_COMMIT(u8 commit);
//void Go_ACK();
//void Start_TIMER(u8* ACK_number);
void Send_REPLAY(u32 time1, u32 time2);
void sign_data(u8* hash,u8* sig);
void New_Proposal(u8 command_size);
void Propose();
void Catching_PREPREPARE();
void Catching_PREPARE();
void Catching_COMMIT();
void Start_TIMER();
void test_finish();

void init(){
	while(LoRa_Init())//初始化LORA模块
	{
		delay_ms(300);
	}
	LoRa_CFG.chn = Data_channel;
	LoRa_CFG.tmode = LORA_TMODE_PT;
	LoRa_CFG.wlrate = LORA_RATE_62K5;
	LoRa_CFG.lbt = LORA_LBT_ENABLE;
	LoRa_CFG.addr = 0;
	LoRa_Set();
	
	for(u8 i = 0 ; i < 13 + 1 ; i++){
		Choice[i]=0xFF;
		Commit[i]=0xFF;
	}
	
	//throughput
	for(u8 m=0;m<MAX_PROPOSAL_PACKETS;m++) Rece_Proposal[m] = 0xFF;
	Proposal_Packet_Number = 0xFF;
	Rece_Proposal_flag = 0;
	Rece_Proposal_Number = 0;
	
	TIM5_Exceed_Times = 0;
	//Catching up
	Need_Choice_Flag = 0;
	Need_Commit_Flag = 0;
	Need_Proposal_Flag = 0;
	
	decided = 0;
	Choice_Truth_Set_Number = Choice_Truth_Set_Number + 1;
	Commit_Truth_Set_Number = Commit_Truth_Set_Number + 1;
}



u8 start_flag = 0;
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
	u16 before_delay = (id)*slot; 
	u16 after_delay = (N-id)*slot;
	Packet_Internal_delay = 200;
	//////////////

	init();
	init_public_key();
	
	//数据类型
	//0 : id 	1 2 3 4
	//1 : data_type => 1->propose 2->choice 3->commit
	//2 : data size 
	//-29 : data_body
	//30-93 sig
	
	LED0(1);
	LED1(1);
	LED2(1); 
	
	/*****************************PREPREPARE**********************************/
	if(id == 1){
		
		Commands_Size = 1;
		Proposal_Packet_Number = (Commands_Size + 3)/4;
		New_Proposal(Commands_Size);
	}
	
	Start_main();
	TIM2->CNT = 0;
	TIM2_Exceed_Times = 0;
	TIM9->CNT = 0;
	TIM9_Exceed_Times = 0;
	before_round = 0xff;
	after_round = 0;
//	TIM10_Exceed_Times = 0;
//	TIM10->CR1|=0x01;
//	DONE_OR_NOT();
	
	for(u8 i = 0 ; i < NTX_proposal ; i++){
		
		while(1){
			if(TIM9_Exceed_Times==2 * i)break;
		}
		
		if(id==1)
			Propose();
		else{

		}
			
	}
	if(id==1)Rece_Proposal_flag = 1;
	
	while(TIM9_Exceed_Times != 2 * NTX_proposal);
	
	TIM9_Exceed_Times = 0;
	before_round = 0xff;
	after_round = 0;
	
	/*****************************PREPARE**********************************/

	if(Rece_Proposal_flag){
		my_choice = 1;
		Choice[id] = 1;
//		Choice_Truth_Set_Number = Choice_Truth_Set_Number + 1;
	}
	
	for(u8 i = 0 ; i < NTX ; i++){
//		if(Choice_Truth_Set_Number >= 2*f+1)break;
		
		BeforeRound();
		
//		if(Choice_Truth_Set_Number >= 2*f+1)break;
		
		if(Rece_Proposal_flag){
			for(u8 m=0;m<phase_NTX;m++){
				Send_PREPARE(1);
				delay_ms(Packet_Internal_delay);
			}	
		}
				
		
//		if(Choice_Truth_Set_Number >= 2*f+1)break;
		
		AfterRound();
	}
	
	if(Choice_Truth_Set_Number >= 2*f+1 ){
		my_commit = 1;
		Commit[id] = 1;
	}

	
	/*****************************COMMIT**********************************/	
	for(u8 i = 0 ; i < NTX ; i++){
		test_finish();
		BeforeRound();//random
		test_finish();
		
		if(Choice_Truth_Set_Number >= 2*f+1 && Rece_Proposal_flag){
			for(u8 m=0;m<phase_NTX;m++){
				Send_COMMIT(1);
				delay_ms(Packet_Internal_delay);
			}	
		}
			
		
		test_finish();
		AfterRound();
	}
		
	/*****************************DECIDED**********************************/
	
	if(Commit_Truth_Set_Number >= 2*f+1 && Total_time==0 && Rece_Proposal_flag){
		Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
		
		Send_REPLAY(Total_time,TIM2_Exceed_Times);
		
		decided=1;
	}
	
	/*****************************CATCHING**********************************/
	my_choice = 1;
	my_commit = 1;
	u8 tmp_times = 0;
	while(1){
		
		Start_TIMER();
		
		if(id==1 && Need_Proposal_Flag){
			
			test_finish();
			
			Propose();
			Need_Proposal_Flag = 0;
			delay_ms(Packet_Internal_delay);
		}
		
		
		if(!Rece_Proposal_flag){
			
			Catching_PREPREPARE();
			
			delay_ms(Packet_Internal_delay);
		}
		
		
		if(Need_Choice_Flag && Rece_Proposal_flag){
			
			test_finish();
			
			Send_PREPARE(1);
			
			Need_Choice_Flag = 0;
			
			delay_ms(Packet_Internal_delay);
		}
		
		
		if(Need_Commit_Flag && (Choice_Truth_Set_Number >= 2*f+1) && Rece_Proposal_flag){
			
			test_finish();
			
			Send_COMMIT(1);		
			
			Need_Commit_Flag = 0;
			
			delay_ms(Packet_Internal_delay);
		}
		
		if(Choice_Truth_Set_Number < 2*f+1 && Rece_Proposal_flag){
			
			Catching_PREPARE();
			
			delay_ms(Packet_Internal_delay);
		}
		
		if((Choice_Truth_Set_Number >= 2*f+1) && (Commit_Truth_Set_Number < 2*f+1)  && Rece_Proposal_flag){
			
			Catching_COMMIT();
			
			delay_ms(Packet_Internal_delay);
		}
		
		if(Commit_Truth_Set_Number >= 2*f+1  && Rece_Proposal_flag){
			
			if(!decided){
				Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
				decided = 1;
			}
//			tmp_times = tmp_times + 1;
//			if(tmp_times == 5){
				Send_REPLAY(Total_time,TIM2_Exceed_Times);
//				tmp_times=0;
//			}
		
			delay_ms(Packet_Internal_delay);
		}
		
	}
	
}

//module
void test_finish(){
	if(Commit_Truth_Set_Number >= 2*f+1 && !decided && Total_time==0 && Rece_Proposal_flag){
		Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
		decided = 1;
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
		
		delay_ms(Packet_Internal_delay-20);
	}
	
}

void Start_TIMER(){
	//打开定时器
	Timer_Flag_5 = 0xFF;
	TIM5->CNT = 0;
	TIM5->CR1|=0x01;
	while(Timer_Flag_5){
//		if(id==1 && Need_Proposal_Flag) break;
//		if(Need_Choice_Flag || Need_Commit_Flag) break;
//		if(Commit_Truth_Set_Number >= 2*f+1)break;
	
	}
	//关闭定时器
	TIM5->CR1|=0x00;
}

void Send_PREPARE(u8 choice){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};	
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = CHOICE;
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

void Send_COMMIT(u8 commit){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	Send_Data_buff[0] = id;
	Send_Data_buff[1] = COMMIT;
	Send_Data_buff[2] = 1 + 32;
	Send_Data_buff[3] = commit;
	
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
