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

u8 Choice[16+1];
u8 Commit[16+1];

u8 Send_Data_buff[800]={0};//�������ݻ�����

u8 my_choice;
u8 my_commit;

//Global 
u8 Group_Number;
u8 Group_Leader_ID;

u8 Global_NTX = 1;
u8 Global_NTX_proposal = 3;
u8 Global_NTX_proposal_flag = 0xff;

//u8 Global_Ready = 0;
u8 Global_Commands_Size[TOTAL_GROUPS];							//������ָ����
u8 Global_Proposal_Packet_Number[TOTAL_GROUPS];					// = Command_Size / 4 ��ȡ��
u8 Global_Rece_Proposal[TOTAL_GROUPS][MAX_PROPOSAL_PACKETS];	//�������յ��᰸���ݰ�
//u8 Global_Rece_Proposal_flag[TOTAL_GROUPS];						//����Ƿ����������ݰ� 0 û��  1 ��
u8 Global_Rece_Proposal_Number[TOTAL_GROUPS]; 					//�յ��᰸���ݰ�����
u8 Global_Command_buff[TOTAL_GROUPS][800];						//ָ�����
u8 Global_Proposal_hash[TOTAL_GROUPS][40];						//�᰸��ϣ

u8 Global_Sig_Number[TOTAL_GROUPS];
u8 Global_IDS[TOTAL_GROUPS][MAX_GROUP_NUMBER];					//��ڼ��� ���ļ����ڵ��signature
u8 Global_Sig[TOTAL_GROUPS][MAX_GROUP_NUMBER][Sig_Size];			//��ڼ���ڵ�����signatures

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

u8 Global_Proposal_1_Number;			//�Լ��յ��˼���
u8 Rece_Global_Proposal_1[30];
u8 Global_Proposal_1_ACK_Number;
u8 Global_Proposal_1_ACK[TOTAL_GROUPS];	//˭�յ����ҵ�
u8 Global_Ready_Number;
u8 Global_Ready[TOTAL_GROUPS];//˭Ready��

u8 Proposal_2[40];
u8 Proposal_2_hash[40];
u8 Rece_Proposal_2_flag;
u8 Proposal_Queue[10];
u8 start_flag2 = 0;

u8 Global_Consensus_Leader = 1;

//NACK
u8 Global_Need_Choice_Flag;
u8 Global_Need_Commit_Flag;
u8 Global_Need_Proposal_Flag;

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
//void Global_NACK_phase();
void Global_Catching_PREPARE();
void Global_Catching_COMMIT();
void Global_Catching_PREPREPARE();
void Sleep_Random(){
	u32 random = RNG_Get_RandomRange(1,N);
	random = random * slot;
	delay_ms(random);
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
//throughput
u8 Commands_Size;						//������ָ����
u8 Proposal_Packet_Number;				// = Command_Size / 4 ��ȡ��
u8 Rece_Proposal[MAX_PROPOSAL_PACKETS];	//�������յ��᰸���ݰ�
u8 Rece_Proposal_flag;					//����Ƿ����������ݰ� 0 û��  1 ��
u8 Rece_Proposal_Number; 				//�յ��᰸���ݰ�����
u8 Command_buff[800];					//ָ�����
u8 Proposal_hash[40];					//�᰸��ϣ
u8 before_round;
u8 after_round;
u8 TIM_slot = 2;
u32 TIM9_Exceed_Times;
u32 TIM10_Exceed_Times;

////////������
u32 obj_addr;//��¼�û�����Ŀ���ַ
u8 obj_chn;//��¼�û�����Ŀ���ŵ�


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
void Propose(u8 idx);
void Catching_PREPREPARE();
void Catching_PREPARE();
void Catching_COMMIT();
void Start_TIMER();
void test_finish();

void init(){
	while(LoRa_Init())//��ʼ��LORAģ��
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
	
	//Global
	
	Global_Choice_Truth_Set_Number = Global_Choice_Truth_Set_Number + 1;
	Global_Commit_Truth_Set_Number = Global_Commit_Truth_Set_Number + 1;
	
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
	
}



u8 start_flag = 0;
void Start_main(){
	while(1){
		if(start_flag)break;
	}
}
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
u16 Packet_Internal_delay;

void DONE_OR_NOT(){

//	if(RNG_Get_RandomRange(2,101) <= DONE_THRES){
//		usart3_rx(0);
//	}else{
//		usart3_rx(1);
//	}

}

void pbft(){
  
	btim_tim2_enable(ENABLE);
	
	///////С���������ã�id////////
	id = 13;
	
	if(id >= 1 && id <= 4){Group_Number = 1;Group_Leader_ID = 1;}
	else if(id >= 5 && id <= 8){Group_Number = 2;Group_Leader_ID = 5;}
	else if(id >= 9 && id <= 12){Group_Number = 3;Group_Leader_ID = 9;}
	else if(id >= 13 && id <= 16){Group_Number = 4;Group_Leader_ID = 13;}
	
	Packet_Internal_delay = 200;
	//////////////

	init();
	init_public_key();
	
	//��������
	//0 : id 	1 2 3 4
	//1 : data_type => 1->propose 2->choice 3->commit
	//2 : data size 
	//-29 : data_body
	//30-93 sig
	
	LED0(1);
	LED1(1);
	LED2(1); 
/*****************************Local_Consensus**************************************************************************************************************************************************************************/	
	/*****************************PREPREPARE**********************************/
	if(id == Group_Leader_ID){
		Commands_Size = 1;
		Proposal_Packet_Number = (Commands_Size + 3)/4;
		New_Proposal(Commands_Size);
	}
	
	Start_main();
	Change_channel(10*Group_Number);
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
		
		if(id==Group_Leader_ID)
			Propose(0);
		else{

		}
			
	}
	if(id==Group_Leader_ID)Rece_Proposal_flag = 1;
	
	while(TIM9_Exceed_Times != 2 * NTX_proposal);
	
	TIM9_Exceed_Times = 0;
	before_round = 0xff;
	after_round = 0;
	
	/*****************************PREPARE**********************************/

	u8 id_idx = id - 4 * (Group_Number - 1);
	
	if(Rece_Proposal_flag){
		my_choice = 1;
		Choice[id_idx] = 1;
	}
	
	for(u8 i = 0 ; i < NTX ; i++){
		
		BeforeRound();
		
		if(Rece_Proposal_flag){
			for(u8 m=0;m<phase_NTX;m++){
				Send_PREPARE(1);
				delay_ms(Packet_Internal_delay);
			}	
		}
		
		AfterRound();
	}
	
	if(Choice_Truth_Set_Number >= 2*f+1 ){
		my_commit = 1;
		Commit[id_idx] = 1;
	}

	
	/*****************************COMMIT**********************************/	
	for(u8 i = 0 ; i < NTX ; i++){
		
		if(id == Group_Leader_ID && Global_Sig_Number[Group_Number] >= 2*f)break;
		
		BeforeRound();//random

		if(Choice_Truth_Set_Number >= 2*f+1 && Rece_Proposal_flag){
			for(u8 m=0;m<phase_NTX;m++){
				Send_COMMIT(1);
				delay_ms(Packet_Internal_delay);
			}	
		}
	
		AfterRound();
	}
		
	/*****************************DECIDED**********************************/
	
//	if(Commit_Truth_Set_Number >= 2*f+1 && Total_time==0 && Rece_Proposal_flag){
//		Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
//		
//		Send_REPLAY(Total_time,TIM2_Exceed_Times);
//		
//		decided=1;
//	}
	
	/*****************************CATCHING**********************************/
	my_choice = 1;
	my_commit = 1;
	
	while(1){
		if(id == Group_Leader_ID && Global_Sig_Number[Group_Number] >= 2*f)break;
		
		Start_TIMER();
		
		if(id == Group_Leader_ID && Global_Sig_Number[Group_Number] >= 2*f)break;
		
		if(id == Group_Leader_ID && Need_Proposal_Flag){			
			Propose(0);
			Need_Proposal_Flag = 0;
			delay_ms(Packet_Internal_delay);
		}
		
		
		if(!Rece_Proposal_flag){
			
			Catching_PREPREPARE();
			
			delay_ms(Packet_Internal_delay);
		}
		
		
		if(Need_Choice_Flag && Rece_Proposal_flag){
			
			Send_PREPARE(1);
			
			Need_Choice_Flag = 0;
			
			delay_ms(Packet_Internal_delay);
		}
		
		
		if(Need_Commit_Flag && (Choice_Truth_Set_Number >= 2*f+1) && Rece_Proposal_flag){
			
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
		
//		if(Commit_Truth_Set_Number >= 2*f+1  && Rece_Proposal_flag){
//			
//			if(!decided){
//				Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
//				decided = 1;
//			}
////			tmp_times = tmp_times + 1;
////			if(tmp_times == 5){
//				Send_REPLAY(Total_time,TIM2_Exceed_Times);
////				tmp_times=0;
////			}
//		
//			delay_ms(Packet_Internal_delay);
//		}
		
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
		
		if(start_flag2)break;
		
		Global_Send_Ready();
		
		if(start_flag2)break;
		
		if(Global_Ready_Number >= GROUPS_NUMBER && id == Global_Consensus_Leader)break;
		
		Global_TIMER();
		if(start_flag2)break;
	}
	if(id == Global_Consensus_Leader)delay_ms(5000);
	if(id == Global_Consensus_Leader){
		Global_Start();
		delay_ms(100);
		Global_Start();
//		delay_ms(100);
		
		start_flag2 = 1;
		TIM9_Exceed_Times = 0;
	}
	
/*****************************PREPREPARE**********************************/
	TIM9_Exceed_Times = 0;
	if(id==Global_Consensus_Leader){
		Global_New_Proposal();
	}
	
	before_round = 0x00;
	after_round = 0xFF;
	
	for(u8 i = 0 ; i < Global_NTX_proposal ; i++){
		
		while(1){
			if(TIM9_Exceed_Times==2 * i)break;
		}
		
		if(id==Global_Consensus_Leader)
			Global_Propose();
		else{

		}
			
	}
	if(id==Global_Consensus_Leader)Rece_Proposal_2_flag = 1;
	
	while(TIM9_Exceed_Times != 2 * Global_NTX_proposal);
	
	TIM9_Exceed_Times = 0;
	before_round = 0xff;
	after_round = 0;
	
	/*****************************PREPARE**********************************/

	id_idx = Group_Number;
	
	if(Rece_Proposal_2_flag){
		my_choice = 1;
		Global_Choice[id_idx] = 1;
	}
	
	for(u8 i = 0 ; i < Global_NTX ; i++){
		
		BeforeRound();
		
		if(Rece_Proposal_2_flag){
			for(u8 m=0;m<phase_NTX;m++){
				Global_Send_PREPARE(1);
				delay_ms(Packet_Internal_delay);
			}	
		}
		
		AfterRound();
	}
	
	if(Global_Choice_Truth_Set_Number >= 2*Global_f+1 ){
		my_commit = 1;
		Global_Commit[id_idx] = 1;
	}

	
	/*****************************COMMIT**********************************/	
	for(u8 i = 0 ; i < Global_NTX ; i++){
		
		BeforeRound();//random

		if(Global_Choice_Truth_Set_Number >= 2*Global_f+1 && Rece_Proposal_2_flag){
			for(u8 m=0;m<phase_NTX;m++){
				Global_Send_COMMIT(1);
				delay_ms(Packet_Internal_delay);
			}	
		}
	
		AfterRound();
	}
		
	/*****************************DECIDED**********************************/
	
	if(Global_Commit_Truth_Set_Number >= 2*Global_f+1 && Total_time==0 && Rece_Proposal_2_flag){
		Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
		
		Send_REPLAY(Total_time,TIM2_Exceed_Times);
		
		decided=1;
	}
	
	/*****************************CATCHING**********************************/
	my_choice = 1;
	my_commit = 1;
	
	while(1){
		test_finish();
		
		Start_TIMER();
		
		test_finish();
		
		if(id == Global_Consensus_Leader && Global_Need_Proposal_Flag){			
			Global_Propose();
			Global_Need_Proposal_Flag = 0;
			delay_ms(Packet_Internal_delay);
		}
		
		
		if(!Rece_Proposal_2_flag){
			test_finish();
			Global_Catching_PREPREPARE();
			
			delay_ms(Packet_Internal_delay);
		}
		
		
		if(Global_Need_Choice_Flag && Rece_Proposal_2_flag){
			test_finish();
			Global_Send_PREPARE(1);
			
			Global_Need_Choice_Flag = 0;
			
			delay_ms(Packet_Internal_delay);
		}
		
		
		if(Global_Need_Commit_Flag && (Global_Choice_Truth_Set_Number >= 2*Global_f+1) && Rece_Proposal_2_flag){
			test_finish();
			Global_Send_COMMIT(1);		
			
			Global_Need_Commit_Flag = 0;
			
			delay_ms(Packet_Internal_delay);
		}
		
		if(Global_Choice_Truth_Set_Number < 2*Global_f+1 && Rece_Proposal_2_flag){
			test_finish();
			Global_Catching_PREPARE();
			
			delay_ms(Packet_Internal_delay);
		}
		
		if((Global_Choice_Truth_Set_Number >= 2*Global_f+1) && (Global_Commit_Truth_Set_Number < 2*f+1)  && Rece_Proposal_2_flag){
			test_finish();
			Global_Catching_COMMIT();
			
			delay_ms(Packet_Internal_delay);
		}
		
		if(Global_Commit_Truth_Set_Number >= 2*Global_f+1  && Rece_Proposal_2_flag){
			
			test_finish();

			Send_REPLAY(Total_time,TIM2_Exceed_Times);
		
			delay_ms(Packet_Internal_delay);
		}
		
	}
	
}

//module
void test_finish(){
	if(Global_Commit_Truth_Set_Number >= 2*Global_f+1 && Total_time==0 && Rece_Proposal_2_flag){
		Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
		
		Send_REPLAY(Total_time,TIM2_Exceed_Times);
		
		decided=1;
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
		
		delay_ms(Packet_Internal_delay-20);
	}
	
}

void Start_TIMER(){
	//�򿪶�ʱ��
	Timer_Flag_5 = 0xFF;
	TIM5->CNT = 0;
	TIM5->CR1|=0x01;
	while(Timer_Flag_5){
//		if(id==1 && Need_Proposal_Flag) break;
//		if(Need_Choice_Flag || Need_Commit_Flag) break;
//		if(Commit_Truth_Set_Number >= 2*f+1)break;
	
	}
	//�رն�ʱ��
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
	//����2f��ǩ��,��Ϊ��2f��/3��
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
	//�򿪶�ʱ��
	Timer_Flag_5 = 0xFF;
	TIM5->CNT = 0;
	TIM5->CR1|=0x01;
	while(Timer_Flag_5){
		if(start_flag2)break;
	}
	//�رն�ʱ��
	TIM5->CR1|=0x00;

}

void Global_New_Proposal(){
	// ˳�� 1-2-3-4�ⶨ
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

//void Global_NACK_phase(){
//	
//	if(id == Global_Consensus_Leader && Global_Need_Proposal_Flag){
//		Global_Propose();
//		Global_Need_Proposal_Flag = 0;
//		delay_ms(Packet_Internal_delay);
//	}
//	
//	if(!Rece_Proposal_2_flag){
//		
//		Global_NACK_PREPREPARE();
//		
//		delay_ms(Packet_Internal_delay);
//	}
//	
//	
//	if(Global_Need_Choice_Flag && Rece_Proposal_2_flag){
//		
////		test_finish();
//		
//		Global_Send_PREPARE(1);
//		
//		Global_Need_Choice_Flag = 0;
//		
//		delay_ms(Packet_Internal_delay);
//	}
//	
//	
//	if(Global_Need_Commit_Flag && (Global_Choice_Truth_Set_Number >= 2*f+1) && Rece_Proposal_2_flag){
//		
//		test_finish();
//		
//		Global_Send_COMMIT(1);		
//		
//		Global_Need_Commit_Flag = 0;
//		
//		delay_ms(Packet_Internal_delay);
//	}
//	
//	if(Global_Choice_Truth_Set_Number < 2*Global_f+1 && Rece_Proposal_2_flag){
//		
//		Global_NACK_PREPARE();
//		
//		delay_ms(Packet_Internal_delay);
//	}
//	
//	if((Global_Choice_Truth_Set_Number >= 2*Global_f+1) && (Global_Commit_Truth_Set_Number < 2*Global_f+1)){
//		
//		Global_NACK_COMMIT();
//		
//		delay_ms(Packet_Internal_delay);
//	}
//	
//	if(Global_Commit_Truth_Set_Number >= 2*Global_f+1 && Rece_Proposal_2_flag){
//		
//		if(Total_time==0){
//			Total_time = TIM2->CNT + TIM2_Exceed_Times * 9000;
//		}
//		
//		Send_REPLAY(Total_time,TIM2_Exceed_Times);
//		
//		delay_ms(Packet_Internal_delay);
//	}
//}
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


