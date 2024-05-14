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
 
u8 id = 0;	//id = 0 => leader           1-2-3 => follower 
u8 NTX = 1;
u8 NTX_proposal = 5;
u8 phase_NTX = 3;
u8 phase_NTX2 = 1;
u16 slot = 800;

u8 Choice_Truth_Set_Number = 0;
u8 Choice_False_Set_Number = 0;

u8 Commit_Truth_Set_Number = 0;
u8 Commit_False_Set_Number = 0;

u8 Choice[13+1];
u8 Commit[13+1];

u8 Send_Data_buff[800]={0};//�������ݻ�����

////////throughput
u8 Commands_Size;						//������ָ����
u8 Proposal_Packet_Number;				// = Command_Size / 4 ��ȡ��
u8 Rece_Proposal[MAX_PROPOSAL_PACKETS];	//�������յ��᰸���ݰ�
u8 Rece_Proposal_flag;					//����Ƿ����������ݰ� 0 û��  1 ��
u8 Rece_Proposal_Number; 				//�յ��᰸���ݰ�����
u8 Command_buff[800];					//ָ�����
u8 Proposal_hash[40];					//�᰸��ϣ
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
void Propose();
void Start_TIMER();
void Sleep_Random();
void Follower_Send(u8 PHASE, u8 choice);
void Leader_Send(u8 PHASE, u8* ID_list, u8 Sig_list[20][64]);
void Leader_Catching(u8 PHASE, u8* ACK_list);
void Follower_Catching(u8 PHASE);


void init(){
	while(LoRa_Init())//��ʼ��LORAģ��
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
	
}


void Start_main(){
	while(1){
		if(start_flag)break;
	}
}

u32 PREPREPARE_delay = 2000;//��proposal�������������������֮����500ms
u32 Packet_Internal_delay = 200;
u32 Phase_Internal_delay = 0;

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

void hotstuff(){
	btim_tim2_enable(ENABLE);
	///////С���������ã�id////////
	id = 10;
	
	u16 before_delay = (id)*slot; 
	u16 after_delay = (N-id)*slot;
	
	//////////////
	led0sta = 0;
	led1sta = 0;
	led2sta = 0;
	LED0(1);LED1(1);LED2(1);
	init();
	init_public_key();
//	u8 Need_to_Wait = N;
	//��������
	//0 : id 	1 2 3 4
	//1 : data_type => 1->PROPOSAL 2->CHOICE 3->COMMIT 4->ACK
	//2 : data size 
	//2-x : data_body  ��ACK �� 2��ʾ���ĸ����͵�����1->propose 2->choice 3->commit��//���ֹ�ʶ������Ҫ�޸�
	//x+1-x+64 sig
	
	//leader : proposal
	
	
/**************************PREPREPARE**************************/
	if(id == 1){
		ACK_Proposal_Number = 1;
		ACK_Proposal[id] = 0x01;
		
		Commands_Size = 1;
		Proposal_Packet_Number = (Commands_Size + 3)/4;
		
		New_Proposal(Commands_Size);
		Rece_Proposal_flag = 1;
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
	
	for(u8 ntx=0;ntx<NTX_proposal;ntx++){
		
		while(1){
			if(TIM9_Exceed_Times==2 * ntx)break;
		}
		
		if(id==1)
			Propose();
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
		if(id==1){
			for(u8 ntx=0;ntx<NTX;ntx++){
				if(Prepare_Number >= (2 * f + 1)) break;
				BeforeRound();
				
				//nop
				
				AfterRound();
			
			}
		}
		if(id==1)break;
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
		
		if((id != 1) && ((Rece_commit==1) || (Rece_decide == 1) || (Rece_next == 1)))break;
		if(id == 1 && Precommit_Number >= (2*f+1)) break;
		
		BeforeRound();

		if(id != 1){
			if(!(Rece_precommit == 1)){
				AfterRound();
				continue;
			}
		}
		
		if(id==1 && Prepare_Number < ( 2 * f + 1)){
				AfterRound();
				continue;
		}
		
		if((id != 1) && ((Rece_commit==1) || (Rece_decide == 1) || (Rece_next == 1)))break;
		if(id == 1 && Precommit_Number >= (2*f+1)) break;
		

		if(id == 1){
			Leader_Send(PRECOMMIT,Prepare_id,Prepare_Sig);
			for(u8 m=0;m<Extera_slot;m++)Leader_Send(PRECOMMIT,Prepare_id,Prepare_Sig);
		}
		if(id != 1){
			
			for(u8 m=0;m<phase_NTX;m++){
				Follower_Send(PRECOMMIT,my_precommit);
				delay_ms(Packet_Internal_delay);
			}
			
		}	
		
		if((id != 1) && ((Rece_commit==1) || (Rece_decide == 1) || (Rece_next == 1)))break;
		if(id == 1 && Precommit_Number >= (2*f+1)) break;
		
		AfterRound();
		
	}
	delay_ms(Phase_Internal_delay);
	/**************************COMMIT**************************/
	HotStuff_status = COMMIT;
	my_commit = 1;
	
	for(u8 ntx=0;ntx<NTX;ntx++){
		
		if((id != 1) && ((Rece_decide==1) || (Rece_next == 1)))break;
		if(id == 1 && Commit_Number >= (2*f+1))	break;
		
		BeforeRound();

		if(id != 1){
			if(!(Rece_commit == 1)){
				AfterRound();
				continue;
			}
		}
		
		if(id==1 && Precommit_Number < ( 2 * f + 1)){
				AfterRound();
				continue;
		}
		
		if((id != 1) && ((Rece_decide==1) || (Rece_next == 1)))break;
		if(id == 1 && Commit_Number >= (2*f+1))	break;
		
		if(id == 1){
			Leader_Send(COMMIT,PreCommit_id,PreCommit_Sig);
			for(u8 m=0;m<Extera_slot;m++)Leader_Send(COMMIT,PreCommit_id,PreCommit_Sig);
		}
		if(id != 1){     
			for(u8 m=0;m<phase_NTX;m++){
				Follower_Send(COMMIT,my_commit);
				delay_ms(Packet_Internal_delay);
			}
		}
		
		if((id != 1) && ((Rece_decide==1) || (Rece_next == 1)))break;
		if(id == 1 && Commit_Number >= (2*f+1))	break;
		AfterRound();
		
	}	
	
	delay_ms(Phase_Internal_delay);
/**************************DECIDE**************************/
	HotStuff_status = DECIDE;
	my_decide = 1;

	for(u8 ntx=0;ntx<NTX;ntx++){
		
		if((id != 1) && (Rece_next==1))break;
		if(id == 1 && Viewchange_Number >= (2*f+1))break;
		
		BeforeRound();

		if(id != 1){
			if(!(Rece_decide == 1)){
				AfterRound();
				continue;
			}
		}
		
		if(id==1 && Commit_Number < ( 2 * f + 1)){
				AfterRound();
				continue;
		}	
		if(Total_time1 == 0)
			Total_time1 = TIM2->CNT + TIM2_Exceed_Times * 9000;
		
		if((id != 1) && (Rece_next==1))break;
		if(id == 1 && Viewchange_Number >= (2*f+1))break;
		
		if(id == 1){
			Leader_Send(DECIDE,Commit_id,Commit_Sig);			
			for(u8 m=0;m<Extera_slot;m++)Leader_Send(DECIDE,Commit_id,Commit_Sig);
		}
		if(id != 1){ 
			for(u8 m=0;m<phase_NTX;m++){
				Follower_Send(VIEW_CHANGE,my_decide);
				delay_ms(Packet_Internal_delay);
			}
			
		}
		
		if((id != 1) && (Rece_next==1))break;
		if(id == 1 && Viewchange_Number >= (2*f+1))break;
		
		AfterRound();
		
	}	
	if((Rece_next==1 || Viewchange_Number >= (2*f+1)) && Total_time==0)
		Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
	delay_ms(Phase_Internal_delay);
/**************************Next_Round**************************/
	
	if(id == 1 && Viewchange_Number >= (2*f+1)){
		for(u8 ntx=0;ntx<NTX;ntx++){
			BeforeRound();
			
			Leader_Send(NEXT_ROUND,Viewchange_id,Viewchange_Sig);
			for(u8 m=0;m<Extera_slot;m++)Leader_Send(NEXT_ROUND,Viewchange_id,Viewchange_Sig);
			
			Send_REPLAY(Total_time1,Total_time,TIM2_Exceed_Times);
			
			AfterRound();
			
		}
	}else if(id!=1){
		BeforeRound();
		
		
		if((Rece_next==1 || Viewchange_Number >= (2*f+1)) && Total_time==0)
			Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
		
		AfterRound();
	}
	
	
	
	if(id != 1 && Rece_next ==1){
		
		u8 nnn_flag = 1;
		while(1){
			BeforeRound();
			
			Send_REPLAY(Total_time1,Total_time,TIM2_Exceed_Times);
			
			AfterRound();
		}
	}
	
	
	
/**************************Catching Up**************************/

	if(id==1){
		while(1){
			
			if(Need_proposal){
				Need_proposal = 0;
				Propose();
				delay_ms(Packet_Internal_delay);
			}
			
			if(Need_precommit && (Prepare_Number >= (2*f+1))){
				Need_precommit = 0;
				
				for(u8 m=0;m<phase_NTX2;m++){
					Leader_Send(PRECOMMIT,Prepare_id,Prepare_Sig);
					delay_ms(Packet_Internal_delay);
				}
//				Leader_Send(PRECOMMIT,Prepare_id,Prepare_Sig);
				
//				delay_ms(Packet_Internal_delay);
				
			}
			if(Need_commit && (Precommit_Number >= (2*f+1))){
				Need_commit = 0;
				for(u8 m=0;m<phase_NTX2;m++){
					Leader_Send(COMMIT,PreCommit_id,PreCommit_Sig);
					delay_ms(Packet_Internal_delay);
				}
//				Leader_Send(COMMIT,PreCommit_id,PreCommit_Sig);
				
//				delay_ms(Packet_Internal_delay);
			}
			if(Need_decide && (Commit_Number >= (2*f+1))){
				Need_decide = 0;
				for(u8 m=0;m<phase_NTX2;m++){
					Leader_Send(DECIDE,Commit_id,Commit_Sig);
					delay_ms(Packet_Internal_delay);
				}
//				Leader_Send(DECIDE,Commit_id,Commit_Sig);
//				
//				delay_ms(Packet_Internal_delay);
			}
			if(Need_next && (Viewchange_Number >= (2*f+1))){
				Need_next = 0;
				for(u8 m=0;m<phase_NTX2;m++){
					Leader_Send(NEXT_ROUND,Viewchange_id,Viewchange_Sig);
					delay_ms(Packet_Internal_delay);
				}
//				Leader_Send(NEXT_ROUND,Viewchange_id,Viewchange_Sig);
//				
//				delay_ms(Packet_Internal_delay);
			}
			
			if(Prepare_Number < (2*f+1)){
				for(u8 m=0;m<phase_NTX;m++){
					Leader_Catching(PREPARE,Rece_Prepare);
					delay_ms(Packet_Internal_delay);
				}
//				Leader_Catching(PREPARE,Rece_Prepare);
//				
//				delay_ms(Packet_Internal_delay);
//				continue;
			}else if(Precommit_Number < (2*f+1)){
				for(u8 m=0;m<phase_NTX;m++){
					Leader_Catching(PRECOMMIT,Rece_PreCommit);
					delay_ms(Packet_Internal_delay);
				}
//				Leader_Catching(PRECOMMIT,Rece_PreCommit);
//				
//				delay_ms(Packet_Internal_delay);
					
			}else if(Commit_Number < (2*f+1)){
				for(u8 m=0;m<phase_NTX;m++){
					Leader_Catching(COMMIT,Rece_Commit);
					delay_ms(Packet_Internal_delay);
				}
//				Leader_Catching(COMMIT,Rece_Commit);
//				
//				delay_ms(Packet_Internal_delay);
					
			}else if(Viewchange_Number < (2*f+1)){
				
				if(Total_time1 == 0) Total_time1 = TIM2->CNT + TIM2_Exceed_Times * 9000;
				for(u8 m=0;m<phase_NTX;m++){
					Leader_Catching(DECIDE,Rece_Viewchange);
					delay_ms(Packet_Internal_delay);
				}
//				Leader_Catching(DECIDE,Rece_Viewchange);
//	
//				delay_ms(Packet_Internal_delay);
					
			}else if(Viewchange_Number >= (2*f+1)){
				
				if(Total_time == 0) Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
				
				Send_REPLAY(Total_time1,Total_time,TIM2_Exceed_Times);
				
				delay_ms(Packet_Internal_delay);
					
			}
			Start_TIMER();
		}
	}
	else if (id != 1){
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
				
//				delay_ms(Packet_Internal_delay);
//				continue;
			}
			else if(Rece_precommit != 1){
				for(u8 m=0;m<phase_NTX;m++){
					Follower_Catching(PRECOMMIT);
					delay_ms(Packet_Internal_delay);
				}
				
//				delay_ms(Packet_Internal_delay);
//				continue;
				
			}else if(Rece_commit != 1){
				for(u8 m=0;m<phase_NTX;m++){
					Follower_Catching(COMMIT);
					delay_ms(Packet_Internal_delay);
				}
				
//				delay_ms(Packet_Internal_delay);
//				continue;
			}else if(Rece_decide != 1){
				for(u8 m=0;m<phase_NTX;m++){
					Follower_Catching(DECIDE);
					delay_ms(Packet_Internal_delay);
				}
//								
//				delay_ms(Packet_Internal_delay);
//				continue;
			}else if(Rece_next != 1){
				
				if(Total_time1 == 0) Total_time1 = TIM2->CNT + TIM2_Exceed_Times * 9000;
				for(u8 m=0;m<phase_NTX;m++){
					Follower_Catching(NEXT_ROUND);
					delay_ms(Packet_Internal_delay);
				}
//				Follower_Catching(NEXT_ROUND);				
				
//				delay_ms(Packet_Internal_delay);
//				continue;
			}else if(Rece_next == 1){
				if(Total_time == 0) Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
				Send_REPLAY(Total_time1,Total_time,TIM2_Exceed_Times);
				delay_ms(Packet_Internal_delay);
			}
			Start_TIMER();
		}
	}
}


//module
void test_finish(){
	if(Viewchange_Number >= 2*f+1 && !decided && Total_time==0){
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
	//�򿪶�ʱ��
	TIM5->CNT = 0;
	TIM5->CR1|=0x01;
	Timer_Flag_5 = 0xFF;
	while(Timer_Flag_5){
		
	}
	//�رն�ʱ��
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
	//����2f��ǩ��,��Ϊ��2f��/3��
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





