#include "mw1268_app.h"
#include "mw1268_cfg.h"
#include "usart3.h"
#include "string.h"
#include "led.h"
#include "delay.h"
#include "tendermint.h"

u8 N=13;
u8 f=30;
u16 DONE_THRES=200;
 
u8 id = 0;	//id = 0 => leader           1-2-3 => follower 
u8 NTX = 3;
u8 NTX_proposal = 1;

u16 slot = 800;

u8 Choice_Truth_Set_Number = 0;
u8 Choice_False_Set_Number = 0;

u8 Commit_Truth_Set_Number = 0;
u8 Commit_False_Set_Number = 0;

u8 Choice[13+1];
u8 Commit[13+1];

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
u32 Total_time;
u32 Total_time1;
u8 start_flag;
u8 before_round;
u8 after_round;
u8 TIM_slot = 2;
u32 TIM9_Exceed_Times;
u32 TIM10_Exceed_Times;

u8 Tendermint_status = 0; //0->choice  1->commit  2->decided

u8 my_choice;
u8 my_commit;
u8 my_precommit;
u8 my_decide;



////////ACK
u8 ACK_Proposal[13+1];
u8 ACK_Choice[13+1];
u8 ACK_Commit[13+1];


u8 ACK_Proposal_Number;
u8 ACK_Choice_Number;
u8 ACK_Commit_Number;

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
//u8 Commit_Number;
u8 Viewchange_Number;

u8 ACK_prepare;
u8 ACK_precommit;
u8 ACK_commit;
u8 ACK_viewchange;

u8 TIM_Prepare_Flag;
u8 TIM_Precommit_Flag;
//u8 TIM_Commit_Flag;
u8 TIM_Decide_Flag;

u8 Rece_precommit;
//u8 Rece_commit;
u8 Rece_decide;
u8 Rece_next;
//id = 1
u8 Rece_Prepare[20];
u8 Prepare_id[20];
u8 Prepare_Sig[20][64];

u8 Rece_PreCommit[20];
u8 PreCommit_id[20];
u8 PreCommit_Sig[20][64];

//u8 Rece_Commit[20];
//u8 Commit_id[20];
//u8 Commit_Sig[20][64];

u8 Rece_Viewchange[20];
u8 Viewchange_id[20];
u8 Viewchange_Sig[20][64];
u8 precommit_verify_number;
u16 TEMP_LEN;
uint8_t hash[32] = {0};
uint8_t sig[64] = {0};
u32 delta = 5000;
u8 Need_to_Wait;
void Send_REPLAY(u32 time1, u32 time2, u32 time3);
void sign_data(u8* hash,u8* sig);
void New_Proposal(u8 command_size);
void Propose();
void Go_ACK();
void Start_TIMER();
void Sleep_Random();
void ACK_phase();
void Follower_Send(u8 PHASE, u8 choice);
void Leader_Send(u8 PHASE, u8* ID_list, u8 Sig_list[20][64]);
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
	
//tendermint
	for(u8 i=0;i<20;i++){
		Rece_Prepare[i] = 0xFF;
		Rece_PreCommit[i] = 0xFF;
//		Rece_Commit[i] = 0xFF;
		Rece_Viewchange[i] = 0xFF;
	}
	
	ACK_prepare = 0xFF;
	ACK_precommit = 0xFF;
	ACK_commit = 0xFF;
	ACK_viewchange = 0xFF;
	
	Prepare_Number = 1;
	Precommit_Number = 1;
//	Commit_Number = 1;
	Viewchange_Number = 1;
	
	TIM_Prepare_Flag = 0xFF;
	TIM_Precommit_Flag = 0xFF;
//	TIM_Commit_Flag = 0xFF;
	
	Rece_precommit = 0;
//	Rece_commit = 0;
	Rece_decide = 0;
	Rece_next = 0;
	precommit_verify_number=0;
	
	//throughput
	for(u8 m=0;m<MAX_PROPOSAL_PACKETS;m++) Rece_Proposal[m] = 0xFF;
	Proposal_Packet_Number = 0xFF;
	Rece_Proposal_flag = 0;
	Rece_Proposal_Number = 0;
}
void Start_main(){
	while(1){
		if(start_flag)break;
	}
}
u16 Packet_Internal_delay;

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

void DONE_OR_NOT(){

	if(RNG_Get_RandomRange(2,101) <= DONE_THRES){
		usart3_rx(0);
	}else{
		usart3_rx(1);
	}

}

void tendermint(){
	btim_tim2_enable(ENABLE);
	///////小车参数设置，id////////
	id = 10;
	u16 before_delay = (id)*slot; 
	u16 after_delay = (N-id)*slot;
	Packet_Internal_delay = 210;
	
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
	
	
	
	/**************************PREPREPARE**************************/
	Tendermint_status = PREPREPARE;
	
	if(id == 1){
		ACK_Proposal_Number = 1;
		ACK_Proposal[id] = 0x01;
		
		Commands_Size = 1;
		Proposal_Packet_Number = (Commands_Size + 3)/4;
		
		New_Proposal(Commands_Size);
	}
	Start_main();
	
	Need_to_Wait = N;
	
	TIM2->CNT = 0;
	TIM2_Exceed_Times = 0;
	TIM5_Exceed_Times = 0;
	
	TIM9->CNT = 0;
	TIM9_Exceed_Times = 0;
	before_round = 0xff;
	after_round = 0;
	
//	TIM10_Exceed_Times = 0;
//	TIM10->CR1|=0x01;
//	DONE_OR_NOT();
	
	decided = 0;
	u8 tmp_flag = 1;
	while(1){
		if(Prepare_Number >= N){
			Rece_Proposal_flag = 1;
			break;
		}
		if(Rece_Proposal_flag==1)break;
		
		BeforeRound();
		
		if(Prepare_Number >= N){
			Rece_Proposal_flag = 1;
			break;
		}
		
		if(id==1 && tmp_flag!=2)
			Propose();
		else{
			if(Rece_Proposal_flag==1)break;
		}
			
		
		tmp_flag = tmp_flag + 1;
		
		AfterRound();
	}
	
	
//	//run
//	run_slow();

	/**************************PREVOTE**************************/
	Tendermint_status = PREVOTE;
	my_choice = 1;

	while(1){
		
		//leader
		if(id==1){
			while(1){
				if(Prepare_Number >= (2 * f + 1)) break;
				BeforeRound();
				
				ACK_phase();
				
				AfterRound();
			
			}
		}
		if(id==1)break;
		if((ACK_prepare == 1) || (Rece_precommit == 1) || (Rece_decide == 1) || (Rece_next == 1)) break;
		
		BeforeRound();
		
		if((ACK_prepare == 1) || (Rece_precommit == 1) || (Rece_decide == 1) || (Rece_next == 1)) break;
		
		Follower_Send(PREVOTE,my_choice);
		
		if((ACK_prepare == 1) || (Rece_precommit == 1) || (Rece_decide == 1) || (Rece_next == 1)) break;
		
		AfterRound();
		
	}
	

	
/**************************PRECOMMIT**************************/
	Tendermint_status = PRECOMMIT;
	my_precommit = 1;
	u8 precommit_flag = 0;
	while(1){
		
		if((id != 1) && ((ACK_precommit == 1) || (Rece_decide == 1) || (Rece_next == 1)))break;
		if(id == 1 && Precommit_Number >= (2*f+1))	break;
		
		
		BeforeRound();

		if(id != 1){
			if(!(Rece_precommit == 1)){
				AfterRound();
				continue;
			}
		}
		
		if(id==1 && precommit_flag){
			u8 tmp_flag = 0;
			while(ACK_TO_Send_Number){
				Go_ACK();
				delay_ms(50);
				tmp_flag = 1;
			}
			if(tmp_flag){
				AfterRound();
				continue;
			}
		}
		
		if((id != 1) && ((ACK_precommit == 1) || (Rece_decide == 1) || (Rece_next == 1)))break;
		if(id == 1 && Precommit_Number >= (2*f+1))	break;
		
		precommit_flag = 1;
		
		if(id == 1){
			Leader_Send(PRECOMMIT,Prepare_id,Prepare_Sig);
		}
		if(id != 1){        
			Follower_Send(PRECOMMIT,my_precommit);
		}
		
		if((id != 1) && ((ACK_precommit == 1) || (Rece_decide == 1) || (Rece_next == 1)))break;
		if(id == 1 && Precommit_Number >= (2*f+1))	break;
		
		AfterRound();
		
	}
	
///**************************Commit**************************/

//	my_commit = 1;
//	Retrans_num = 0;
//	while(1){
//		
//		if((id != 1) && ((ACK_commit == 1) || (Rece_decide==1) || (Rece_next == 1)))break;
//		if(id == 1){
//			if(Commit_Number >= (2*f+1))
//			break;
//		}
//		
//		BeforeRound();

//		if(id != 1){
//			if(!(Rece_commit == 1)){
//				AfterRound();
//				continue;
//			}
//		}
//		
//		if(id==1){
//			u8 tmp_flag = 0;
//			while(ACK_TO_Send_Number){
//				Go_ACK();
//				delay_ms(50);
//				tmp_flag = 1;
//			}
//			if(tmp_flag){
//				AfterRound();
//				continue;
//			}
//		}
//		
//		if(id == 1){
//			u8 send_number = (2*f+2)/3;
//			for(u8 Packet_id=0;Packet_id<send_number;Packet_id++){
//				Send_Data_buff[0] = id;
//				Send_Data_buff[1] = COMMIT;
//				if(Packet_id != send_number-1 )Send_Data_buff[2] = 3;
//				else Send_Data_buff[2] = (2*f)-Packet_id*3;
//				u8 Data_Size = Send_Data_buff[2];
//				for(u8 i=0;i<Data_Size;i++){
//					Send_Data_buff[3+i] = PreCommit_id[i+Packet_id*3];
//				}
//				memset(hash,0x00,32);
//				for(u8 i=0; i < Data_Size+3;i++) hash[i] = Send_Data_buff[i];
//				
//				if (!uECC_sign(private_key[id], hash, Data_Size + 3, sig, curves[0])) {
//					sprintf((char*)Send_Data_buff,"Sign Fail2");
//					Lora_Device_Sta = LORA_TX_STA;
//					LoRa_SendData(Send_Data_buff);
//					while(1);
//				}
//				for(u8 m = 0 ; m < Sig_Size ; m++)Send_Data_buff[3 + Data_Size + m] = sig[m];
//				
//				for(u8 x=0;x<Data_Size;x++){
//					for(u8 y=0;y<Sig_Size;y++)
//					Send_Data_buff[3 + Data_Size + Sig_Size + Sig_Size*x + y]=PreCommit_Sig[x+Packet_id*3][y];
//				}
//				Send_Data_buff[3 + Data_Size+Sig_Size*(Data_Size+1)] = 0x00;
//				Lora_Device_Sta = LORA_TX_STA;
//				LoRa_SendData(Send_Data_buff);
//				
//				delay_ms(200);
//			}

//		}
//		if(id != 1){                 
//			Send_Data_buff[0] = id;
//			Send_Data_buff[1] = COMMIT;
//			Send_Data_buff[2] = 1;
//			Send_Data_buff[3] = my_commit;
//			u8 Data_Size = Send_Data_buff[2];
//			for(u8 m = 0 ; m < Data_Size + 3 ; m++)hash[m] = Send_Data_buff[m];
//			//sign
//			if (!uECC_sign(private_key[id], hash, Data_Size + 3, sig, curves[0])) {
//				sprintf((char*)Send_Data_buff,"Sign Fail2");
//				Lora_Device_Sta = LORA_TX_STA;
//				LoRa_SendData(Send_Data_buff);
//				while(1);
//			}
//			for(u8 m = 0 ; m < 64 ; m++)Send_Data_buff[Data_Size + 3 + m] = sig[m];
//			Send_Data_buff[Data_Size + 3 + 64] = 0x00;
//			Lora_Device_Sta = LORA_TX_STA;
//			LoRa_SendData(Send_Data_buff);
//		}
//		
//		AfterRound();
//		
//	}	
	
/**************************Decide**************************/
	Tendermint_status = DECIDE;
	my_decide = 1;
	u8 decide_flag = 1;
	u8 DECIDE_flag = 0;
	while(1){
		
		if((id != 1) && (Rece_next==1))break;
		if(id == 1 && Viewchange_Number >= Need_to_Wait) break;
		
		BeforeRound();

		if(id != 1 && !(Rece_decide == 1)){
			AfterRound();
			continue;
		}
		
		if(id==1 && DECIDE_flag){
			u8 tmp_flag = 0;
			while(ACK_TO_Send_Number){
				Go_ACK();
				delay_ms(50);
				tmp_flag = 1;
			}
			if(tmp_flag){
				AfterRound();
				continue;
			}
		}	
		DECIDE_flag = 1;
		if(Total_time1==0)
			Total_time1 = TIM2->CNT + TIM2_Exceed_Times * 9000;
		
		if((id != 1) && (Rece_next==1))break;
		if(id == 1 && Viewchange_Number >= Need_to_Wait) break;
		
		if(id == 1){
			Leader_Send(DECIDE,PreCommit_id,PreCommit_Sig);			
		}
		if(id != 1){ 
			Follower_Send(VIEW_CHANGE,my_decide);
		}
		
		if((id != 1) && (Rece_next==1))break;
		if(id == 1 && Viewchange_Number >= Need_to_Wait) break;
		
		AfterRound();
		
	}	
	if(Total_time==0)
		Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
	
	if(id==1){
		delay_ms((N-Need_to_Wait)*delta);
	}
/**************************Next_Round**************************/
	
	if(id == 1){
		while(1){
			BeforeRound();
			
			Leader_Send(NEXT_ROUND,Viewchange_id,Viewchange_Sig);
			
			Send_REPLAY(Total_time1,Total_time,TIM2_Exceed_Times);
			
			AfterRound();
			
		}
	}
	if(id != 1){
		while(1){
			BeforeRound();
			
			Send_REPLAY(Total_time1,Total_time,TIM2_Exceed_Times);
			
			AfterRound();
		}
	}
}
//module
void test_finish(){
	if(Viewchange_Number >= Need_to_Wait && !decided && Total_time==0){
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

void ACK_phase(){
	while(ACK_TO_Send_Number){
		taskENTER_CRITICAL();
		Go_ACK();
		taskEXIT_CRITICAL();
		Sleep_Random();
	}
}

void Go_ACK(){
	uint8_t hash[32] = {0};
    uint8_t sig[64] = {0};
	
	if(ACK_TO_Send_Number >= 100 ){
		ACK_TO_Send_Number = 10;
		return;
	}
	
	if(ACK_TO_Send[ACK_TO_Send_Number-1][2] == PROPOSAL){
		Send_Data_buff[0] = ACK_TO_Send[ACK_TO_Send_Number-1][0];
		Send_Data_buff[1] = ACK_TO_Send[ACK_TO_Send_Number-1][1];//ACK
		Send_Data_buff[2] = 1+32;
		Send_Data_buff[3] = ACK_TO_Send[ACK_TO_Send_Number-1][2];
		
		u8 Data_Size = Send_Data_buff[2];
		for(u8 m=0;m<32;m++)Send_Data_buff[Data_Size-32+3+m]=Proposal_hash[m];
		
		sha2(Send_Data_buff,Data_Size+3,hash,0);
		
		//sign
		sign_data(hash,sig);

		for(u8 m = 0 ; m < Sig_Size ; m++)Send_Data_buff[Data_Size + 3 + m] = sig[m];

		Send_Data_buff[Data_Size + 3 + Sig_Size] = 0x00;
		
		Lora_Device_Sta = LORA_TX_STA;
		LoRa_SendData(Send_Data_buff);// id 0 "Hello LoRa"1
		
	}else{
		Send_Data_buff[0] = ACK_TO_Send[ACK_TO_Send_Number-1][0];
		Send_Data_buff[1] = ACK_TO_Send[ACK_TO_Send_Number-1][1];
		Send_Data_buff[2] = 2+32;
		Send_Data_buff[3] = ACK_TO_Send[ACK_TO_Send_Number-1][2];
		Send_Data_buff[4] = ACK_TO_Send[ACK_TO_Send_Number-1][3];
		
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
	
	ACK_TO_Send_Number = ACK_TO_Send_Number - 1;
}

void Start_TIMER(){
	//打开定时器
	TIM5->CNT = 0;
	TIM5->CR1|=0x01;
	TIM_Prepare_Flag = 0xFF;
	while(TIM_Prepare_Flag){
		if(Tendermint_status == PREPREPARE){
			
			if(id == 1 && ACK_Proposal_Number == N)break;
			else if(id != 1 && Rece_Proposal_flag)break;
			
		}else if(Tendermint_status == PREVOTE){
			
			if(id == 1 && Prepare_Number >= (2 * f + 1)) break;
			else if((ACK_prepare == 1) || (Rece_precommit == 1) || (Rece_decide == 1) || (Rece_next == 1)) break;
		
		}else if(Tendermint_status == PRECOMMIT){
			
			if((id != 1) && ((ACK_precommit == 1) || (Rece_decide == 1) || (Rece_next == 1)))break;
			if(id == 1 && Precommit_Number >= (2*f+1))	break;
		
		}
//		else if(HotStuff_status == COMMIT){
//			
//			if((id != 1) && ((ACK_commit == 1) || (Rece_decide==1) || (Rece_next == 1)))break;
//			if(id == 1 && Commit_Number >= (2*f+1))	break;
//		
//		}
		else if(Tendermint_status == DECIDE){
			
			if((id != 1) && (Rece_next==1))break;
			if(id == 1 && Viewchange_Number >= (2*f+1))	break;
		
		}else if(Tendermint_status == NEXT_ROUND){
		
		}
		
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
	if(PHASE == NEXT_ROUND)send_number = ((Need_to_Wait-1)+2)/3;
	for(u8 Packet_id=0;Packet_id<send_number;Packet_id++){
		Send_Data_buff[0] = id;
		Send_Data_buff[1] = PHASE;
		
		if(Packet_id != send_number-1 )Send_Data_buff[3] = 3;
		else {
			if(PHASE == NEXT_ROUND)Send_Data_buff[3] = (Need_to_Wait-1)-Packet_id*3;
			else Send_Data_buff[3] = (2*f)-Packet_id*3;
		}
		
		
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
