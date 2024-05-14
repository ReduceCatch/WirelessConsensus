/**
  ****************************************************************************************************
  * @file       mw1268_app.c
  * @author     ����ԭ���Ŷ�(ALIENTEK)
  * @version    V1.0
  * @date       2022-2-15
  * @brief      mw1268ģ��Ӧ������
  * @license   	Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
  ****************************************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:����ԭ�� STM32������
  * ������Ƶ:www.yuanzige.com
  * ������̳:www.openedv.com
  * ��˾��ַ:www.alientek.com
  * �����ַ:openedv.taobao.com
  *
  * �޸�˵��
  * V1.0 2022-2-15
  * ��һ�η���
  *
  ****************************************************************************************************
  */

#include "mw1268_app.h"
#include "mw1268_cfg.h"
#include "mw1268_uart.h"
#include "usart3.h"
#include "led.h"
#include "delay.h"
#include "usart.h"

#include "string.h"


/*�豸������ʼ��(�����豸������lora_cfg.h����*/
_LoRa_CFG LoRa_CFG =
{
    .addr     =   LORA_ADDR,       /*�豸��ַ*/
    .chn      =   LORA_CHN,        /*�ŵ�*/
    .netid    =   LORA_NETID,      /*�����ַ*/
    .power    =   LORA_TPOWER,     /*���书��*/
    .wlrate   =   LORA_RATE,       /*��������*/
    .wltime   =   LORA_WLTIME,     /*˯��ʱ��*/
    .wmode    =   LORA_WMODE,      /*����ģʽ*/
    .tmode    =   LORA_TMODE,      /*����״̬*/
    .packsize =   LORA_PACKSIZE,   /*���ݰ���С*/
    .bps      =   LORA_TTLBPS,     /*����������*/
    .parity   =   LORA_TTLPAR,     /*У��λ����*/
    .lbt      =   LORA_LBT         /*�ŵ��������*/
};

/*��¼�豸����״̬*/
_LORA_DEVICE_STA  Lora_Device_Sta;

/*��¼AUX�ж�״̬*/
_LORA_INT_STA     Lora_Int_Sta;

static GPIO_InitTypeDef gpio_init_struct;

/**
 * @brief       ģ�����ų�ʼ��
 * @param       ��
 * @retval      ��
 */
void User_GpioInit(void)
{

    MD0_GPIO_CLK_ENABLE();
    AUX_GPIO_CLK_ENABLE();


    gpio_init_struct.Pin = MD0_GPIO_PIN;                    /* LORA MD0���� */
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;            /* ������� */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* ���� */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;          /* ���� */
    HAL_GPIO_Init(MD0_GPIO_PORT, &gpio_init_struct);        /* AUX����ģʽ���� */

    gpio_init_struct.Pin = AUX_GPIO_PIN;                    /* LORA AUX���� */
    gpio_init_struct.Mode = GPIO_MODE_INPUT;             /* ���� */
    gpio_init_struct.Pull = GPIO_PULLDOWN;                  /*���� */
    HAL_GPIO_Init(AUX_GPIO_PORT, &gpio_init_struct);        /* AUX����ģʽ���� */

    HAL_NVIC_SetPriority(AUX_INT_IRQn, 0, 2);               /* ��ռ0�������ȼ�2 */

}

/**
 * @brief       Aux�����ж�����
 * @param       mode   ģʽ
 * @retval      ��
 */
void Aux_Int(_LORA_INT_STA mode)
{
	
    HAL_NVIC_DisableIRQ(AUX_INT_IRQn);

    if (mode != LORA_INT_OFF) /*�ر��ж�*/
    {
        if (mode == LORA_INT_REDGE) /* �����ش��� */
            gpio_init_struct.Mode = GPIO_MODE_IT_RISING;
        else if (mode == LORA_INT_FEDGE) /*�½��ش���*/
            gpio_init_struct.Mode = GPIO_MODE_IT_FALLING;

        HAL_GPIO_Init(AUX_GPIO_PORT, &gpio_init_struct);
        HAL_NVIC_EnableIRQ(AUX_INT_IRQn);
    }

    Lora_Int_Sta = mode;

}

/**
 * @brief       Aux�����жϷ�����
 * @param       ��
 * @retval      ��
 */
void AUX_INT_IRQHandler(void)
{

    if (__HAL_GPIO_EXTI_GET_IT(AUX_GPIO_PIN))
    {

        if (Lora_Int_Sta == LORA_INT_REDGE) /*������(����:��ʼ��������  ����:���ݿ�ʼ���)*/
        {
            if (Lora_Device_Sta == LORA_RX_STA) /*����ģʽ*/
            {
                USART3_RX_STA = 0; /*���ݼ�����0*/
            }

            Lora_Int_Sta = LORA_INT_FEDGE; /*�����½��ش���*/
            LED0(0); /*DS0��*/

        }
        else if (Lora_Int_Sta == LORA_INT_FEDGE) /*�½���(����:�����ѷ����� ����:�����������)*/
        {

            if (Lora_Device_Sta == LORA_RX_STA) /*����ģʽ*/
            {
				usart3_rx(0);
                u16 len;
				USART3_RX_STA |= 1 << 15; /*���ݼ���������*/
				len = USART3_RX_STA&0X7FFF;
				
				
				
				if(len <= 2){
					USART3_RX_STA = 0;
					Lora_Int_Sta = LORA_INT_REDGE; /*���������ش���*/
					Aux_Int(Lora_Int_Sta);/*���������жϱ���*/
					__HAL_GPIO_EXTI_CLEAR_IT(AUX_GPIO_PIN);					
					usart3_rx(1);
					return;
				}
				uart3RxBuffer[len-2]=0x00;//��ӽ�����
				USART3_RX_STA=0;
				
///////////////////////////////////////////////////////////////////////////////////////////////////
			if(LoRa_CFG.tmode == LORA_TMODE_PT){
				taskENTER_CRITICAL();
				
				if(!start_flag && uart3RxBuffer[0]==0xff && uart3RxBuffer[1]==0xff && uart3RxBuffer[2]==0xff){
					start_flag = 1;					
					N = uart3RxBuffer[3];
					f = uart3RxBuffer[4];
					DONE_THRES = uart3RxBuffer[5];
				}
				
				if(RNG_Get_RandomRange(2,101) <= DONE_THRES){
					USART3_RX_STA = 0;
					Lora_Int_Sta = LORA_INT_REDGE; /*���������ش���*/
					Aux_Int(Lora_Int_Sta);/*���������жϱ���*/
					__HAL_GPIO_EXTI_CLEAR_IT(AUX_GPIO_PIN);					
					taskEXIT_CRITICAL();
					usart3_rx(1);
					return;
				}
				
				LED1(led1sta);led1sta = ~led1sta;
				Commit_False_Set_Number = Commit_False_Set_Number + 1;
				
				
				uint8_t hash[32] = {0};
				uint8_t sig[64] = {0};
				
				u8 temp_id = uart3RxBuffer[0];
				u8 temp_type = uart3RxBuffer[1];
				u8 temp_datasize;
				u8 prefix_proposal;
				u8 temp_proposal_hash[32];
				u8 verify_flag = 1;//signature
				u8 verify_flag2 = 1;//proposal has  h
				if(temp_type == PROPOSAL){
					temp_datasize = uart3RxBuffer[4];
					prefix_proposal = 5;
				}
				else{
					temp_datasize = uart3RxBuffer[2];
					prefix_proposal = 3;
					for(u8 m=0;m<32;m++)
						temp_proposal_hash[31-m] = uart3RxBuffer[2+temp_datasize-m];
				
					for(u8 m=0;m<32;m++){
						if(Proposal_hash[m] != temp_proposal_hash[m]){
							verify_flag2 = 0;
							break;
						}
					}
				}
				
				if(temp_datasize + 5 > 240){
					verify_flag = 0;
				}
				LED1(0);
				LED2(1);

				
				if(verify_flag){
					
					sha2(uart3RxBuffer,temp_datasize+prefix_proposal,hash,0);
					for(u8 m=0;m<Sig_Size;m++){
						if(uart3RxBuffer[m+temp_datasize+prefix_proposal] == 0x00)break;
						sig[m] = uart3RxBuffer[m+temp_datasize+prefix_proposal];
					}
					
					if (!uECC_verify(public_key[temp_id], hash, 32, sig, uECC_secp192r1())) {
						verify_flag = 0;
					}
				
				}
				LED1(1);
				LED2(0);

//				//Relaying
//				if(verify_flag && RELAY_FLAG){
//					//len
//					if(Relay_Number > MAX_RELAY_PACKETS) Relay_Number = MAX_RELAY_PACKETS;
//					
//					for(u16 m=0; m<len; m++)
//						Relay_buff[Relay_Number][m] = uart3RxBuffer[m];
//					
//					Relay_Number = Relay_Number + 1;
//				}
				
				if(verify_flag && (temp_type == PROPOSAL)){
					//Preprepare   Proposal
					Proposal_Packet_Number = uart3RxBuffer[2];
					u8 packet_id = uart3RxBuffer[3];
					if(Rece_Proposal[packet_id] == 0xFF){
						
						for(u8 m=0;m<temp_datasize;m++)
							Command_buff[(packet_id-1)*4*32 + m] = uart3RxBuffer[5+m];
						
						Rece_Proposal[packet_id] = 1;
						
						Rece_Proposal_Number = Rece_Proposal_Number + 1;
						
						if(packet_id == Proposal_Packet_Number){
							//temp_datasize
							Commands_Size = (Proposal_Packet_Number-1)*4 + temp_datasize/32;
						}
						
						if(Rece_Proposal_Number == Proposal_Packet_Number){
							sha2(Command_buff,Commands_Size*SINGLE_COMMAND_SIZE,Proposal_hash,0);
						}
					}
					
					if(Rece_Proposal_Number == Proposal_Packet_Number){
						Rece_Proposal_flag = 1;
						//�ظ�ACK
						if(ACK_TO_Send_Number >= 20)ACK_TO_Send_Number=20;
						
						ACK_TO_Send[ACK_TO_Send_Number][0] = id;
						ACK_TO_Send[ACK_TO_Send_Number][1] = ACK;
						ACK_TO_Send[ACK_TO_Send_Number][2] = PROPOSAL;
						ACK_TO_Send[ACK_TO_Send_Number][3] = 0x00;
						ACK_TO_Send_Number = ACK_TO_Send_Number + 1;
					}
					
					
				} 
				else if(verify_flag2 && verify_flag && (temp_type == CHOICE)){
					//Prepare	Choice
					u8 temp_value = uart3RxBuffer[3];

					if(Choice[temp_id] == 0xFF){
						if(temp_value == 0x01){
							Choice_Truth_Set_Number = Choice_Truth_Set_Number + 1;
							Choice[temp_id] = 1;
							
						}
					}
					
					//�ظ�ACK��
					//�ж��Ƿ���Ҫ�ظ�
					u8 My_id = id+3;
					if(uart3RxBuffer[My_id] == 0xFF){
						if(ACK_TO_Send_Number >= 20)ACK_TO_Send_Number=20;
						ACK_TO_Send[ACK_TO_Send_Number][0] = id;
						ACK_TO_Send[ACK_TO_Send_Number][1] = ACK;
						ACK_TO_Send[ACK_TO_Send_Number][2] = CHOICE;
						ACK_TO_Send[ACK_TO_Send_Number][3] = temp_id;
						ACK_TO_Send[ACK_TO_Send_Number][4] = 0x00;
						ACK_TO_Send_Number = ACK_TO_Send_Number + 1;
					}
				}
				else if(verify_flag2 && verify_flag && (temp_type == COMMIT)){
					//Commit
					u8 temp_value = uart3RxBuffer[3];
					
					if(Commit[temp_id]/*old_value*/ == 0xFF){
						if(temp_value == 0x01){
							Commit_Truth_Set_Number = Commit_Truth_Set_Number + 1;
							Commit[temp_id] = 1;
							
						}
					}
					
					//�ظ�ACK
					//�ж��Ƿ���Ҫ�ظ�
					u8 My_id = id+3;
					if(uart3RxBuffer[My_id] == 0xFF){
						if(ACK_TO_Send_Number >= 20)ACK_TO_Send_Number=20;
						ACK_TO_Send[ACK_TO_Send_Number][0] = id;
						ACK_TO_Send[ACK_TO_Send_Number][1] = ACK;
						ACK_TO_Send[ACK_TO_Send_Number][2] = COMMIT;
						ACK_TO_Send[ACK_TO_Send_Number][3] = temp_id;
						ACK_TO_Send[ACK_TO_Send_Number][4] = 0x00;
						ACK_TO_Send_Number = ACK_TO_Send_Number + 1;
					}
				}
				
				else if(verify_flag2 && verify_flag && (temp_type == ACK)){
					//ACK
					u8 ACK_type;
					ACK_type = uart3RxBuffer[3];
					
					if(ACK_type == PROPOSAL){
						if(ACK_Proposal[temp_id]/*old_value*/ == 0xFF){
							ACK_Proposal_Number = ACK_Proposal_Number + 1;
							ACK_Proposal[temp_id] = 1;
						}
					}
					else if(ACK_type == CHOICE){
						u8 temp2_id = uart3RxBuffer[4];
						
						if(temp2_id == id){
							if(ACK_Choice[temp_id]/*old_value1*/ == 0xFF){
								ACK_Choice_Number = ACK_Choice_Number + 1;
								ACK_Choice[temp_id] = 1;

							}
						}
					}
					else if(ACK_type == COMMIT){
						u8 temp2_id = uart3RxBuffer[4];
						
						if(temp2_id == id){
							if(ACK_Commit[temp_id]/*old_value2*/ == 0xFF){
								ACK_Commit_Number = ACK_Commit_Number + 1;
								ACK_Commit[temp_id] = 1;								
								
							}
						}						
					}
				}
//			
			taskEXIT_CRITICAL();
			
			}
			else if(LoRa_CFG.tmode==LORA_TMODE_FP){
//				LED2(led2sta); led2sta = ~led2sta;
//				DireLen[Dire_Number] = len;
//				for(u16 i=0;i<len;i++){
//					Dire_Rece_Data_buff[Dire_Number][i] = uart3RxBuffer[i];
//				}
//				Dire_Number++;
			}
			
			 /////////////////////////////////////////////////////////////////////////////////////////////////// 
			usart3_rx(1);	
            }
            else if (Lora_Device_Sta == LORA_TX_STA) /*����ģʽ(�������ݷ������)*/
            {
                Lora_Device_Sta = LORA_RX_STA; /*�������ģʽ*/
            }

            Lora_Int_Sta = LORA_INT_REDGE; /*���������ش���*/
            LED0(1); /*DS0��*/

        }

        Aux_Int(Lora_Int_Sta);/*���������жϱ���*/
        __HAL_GPIO_EXTI_CLEAR_IT(AUX_GPIO_PIN);

    }
}


static uint8_t Lora_Cfgbuff[20] = {0}; /*ָ��ͻ���*/

/**
 * @brief       Loraģ���ʼ��
 * @param       ��
 * @retval      ��
 */
uint8_t LoRa_Init(void)
{
	uint8_t temp;
    uint8_t retry = 0;
    uint8_t lora_addrh, lora_addrl = 0;

    User_GpioInit();/*�������ų�ʼ��*/

	usart3_init(115200);/*���ڳ�ʼ��*/
	
	
    LORA_MD0(1); /*��������ģʽ*/

    Lora_Device_Sta = LORA_CONFG_STA; /*���"����ģʽ"*/

    retry = 10;
	
	
    while (retry)
    {
        if (!lora_send_cmd("AT", "OK", 50))
        {
            break;
        }

        retry--;
    }


    if (!retry)
    {
        /*ģ���쳣*/
		temp = 1;
		return temp;
    }

    lora_addrh =  (LoRa_CFG.addr >> 8) & 0xff;
    lora_addrl = LoRa_CFG.addr & 0xff;

    /*�豸��ַ����*/
    sprintf((char *)Lora_Cfgbuff, "AT+ADDR=%02x,%02x", lora_addrh, lora_addrl);

    if (lora_send_cmd(Lora_Cfgbuff, "OK", 50)){
		temp = 0;
		return temp;
	}

    /*�����ַ����*/
    sprintf((char *)Lora_Cfgbuff, "AT+NETID=%d", LoRa_CFG.netid);

    if (lora_send_cmd(Lora_Cfgbuff, "OK", 50)){
		temp = 0;
		return temp;
	}

    /*�źźͿ�����������*/
    sprintf((char *)Lora_Cfgbuff, "AT+WLRATE=%d,%d", LoRa_CFG.chn, LoRa_CFG.wlrate);

    if (lora_send_cmd(Lora_Cfgbuff, "OK", 50)){
		temp = 0;
		return temp;
	}

    /*���书������*/
    sprintf((char *)Lora_Cfgbuff, "AT+TPOWER=%d", LoRa_CFG.power);
    if (lora_send_cmd(Lora_Cfgbuff, "OK", 50)){
		temp = 0;
		return temp;
	}

    /*����ģʽ����*/
    sprintf((char *)Lora_Cfgbuff, "AT+CWMODE=%d", LoRa_CFG.wmode);
    if (lora_send_cmd(Lora_Cfgbuff, "OK", 50)){
		temp = 0;
		return temp;
	}

    /*����ģʽ����*/
    sprintf((char *)Lora_Cfgbuff, "AT+TMODE=%d", LoRa_CFG.tmode);
    if (lora_send_cmd(Lora_Cfgbuff, "OK", 50)){
		temp = 0;
		return temp;
	}

    /*���ݰ���С����*/
    sprintf((char *)Lora_Cfgbuff, "AT+PACKSIZE=%d", LoRa_CFG.packsize);
    if (lora_send_cmd(Lora_Cfgbuff, "OK", 50)){
		temp = 0;
		return temp;
	}

    /*˯��ʱ������*/
    sprintf((char *)Lora_Cfgbuff, "AT+WLTIME=%d", LoRa_CFG.wltime);
    if (lora_send_cmd(Lora_Cfgbuff, "OK", 50)){
		temp = 0;
		return temp;
	}

    /*��������*/
    sprintf((char *)Lora_Cfgbuff, "AT+UART=%d,%d", LoRa_CFG.bps, LoRa_CFG.parity);
    if (lora_send_cmd(Lora_Cfgbuff, "OK", 50)){
		temp = 0;
		return temp;
	}
    
     /*�ŵ��������*/
    sprintf((char *)Lora_Cfgbuff, "AT+LBT=%d", LoRa_CFG.lbt);
    if (lora_send_cmd(Lora_Cfgbuff, "OK", 50)){
		temp = 0;
		return temp;
	}


    LORA_MD0(0);  /*�˳�����ģʽ*/
    delay_ms(50);

    while (LORA_AUX); /*�ж�ģ���Ƿ����*/

    USART3_RX_STA = 0;
    Lora_Device_Sta = LORA_RX_STA; /*���"����ģʽ"*/
    usart3_bpsset(LoRa_CFG.bps, LoRa_CFG.parity); /*����ͨ��,����ͨ�Ŵ�������(�����ʡ�����У��λ)*/

    Aux_Int(LORA_INT_REDGE);/*����LORA_AUX�������ж�*/
	
	temp = 0;
	return temp;
}



//LoRaģ�����ò���
void LoRa_Set(void)
{
	delay_ms(200);
	u8 sendbuf[20];
	u8 lora_addrh,lora_addrl=0;
	
	usart3_bpsset(LORA_BRD_115200,LORA_BRDVER_8N1);//��������ģʽǰ����ͨ�Ų����ʺ�У��λ(115200 8λ���� 1λֹͣ ������У�飩
//	usart3_rx(1);//��������3����
	
	while(LORA_AUX);//�ȴ�ģ�����
	LORA_MD0(1); //��������ģʽ
	delay_ms(40);
	Lora_Device_Sta=0;//���"����ģʽ"
	
	lora_addrh =  (LoRa_CFG.addr>>8)&0xff;
	lora_addrl = LoRa_CFG.addr&0xff;
	sprintf((char*)sendbuf,"AT+ADDR=%02x,%02x",lora_addrh,lora_addrl);//�����豸��ַ
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+NETID=%d",LoRa_CFG.netid);//���������ַ
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+WLRATE=%d,%d",LoRa_CFG.chn,LoRa_CFG.wlrate);//�����ŵ��Ϳ�������
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+TPOWER=%d",LoRa_CFG.power);//���÷��书��
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+CWMODE=%d",LoRa_CFG.wmode);//���ù���ģʽ
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+TMODE=%d",LoRa_CFG.tmode);//���÷���״̬
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+PACKSIZE=%d",LoRa_CFG.packsize);//�������ݰ���С
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+WLTIME=%d",LoRa_CFG.wltime);//����˯��ʱ��
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+UART=%d,%d",LoRa_CFG.bps,LoRa_CFG.parity);//���ô��ڲ����ʡ�����У��λ
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+LBT=%d",LoRa_CFG.lbt);//�ŵ��������
	lora_send_cmd(sendbuf,"OK",50);

	LORA_MD0(0);//�˳�����,����ͨ��
	delay_ms(40);
	while(LORA_AUX);//�ж��Ƿ����(ģ����������ò���)
	USART3_RX_STA=0;
	Lora_Device_Sta=1;//���"����ģʽ"
	
	usart3_bpsset(LoRa_CFG.bps,LoRa_CFG.parity);//����ͨ��,����ͨ�Ŵ�������(�����ʡ�����У��λ)
	Aux_Int(LORA_INT_REDGE);//����LORA_AUX�������ж�

}

uint8_t Dire_Date[] = {0x11, 0x22, 0x33, 0x44, 0x55}; /*�����������*/
uint8_t date[30] = {0}; /*����������*/
uint8_t Tran_Data[800] = {0}; /*͸����������*/

#define Dire_DateLen sizeof(Dire_Date)/sizeof(Dire_Date[0]) /*�������鳤��*/

/**
 * @brief       Loraģ�����ݷ��ʹ���
 * @param       ��
 * @retval      ��
 */
void LoRa_SendData(u8* data)
{
    static uint8_t num = 0;
    uint16_t addr;
    uint8_t chn;
    uint16_t i = 0;

    if (LoRa_CFG.tmode == LORA_TMODE_PT) /*͸������*/
    {
		
//		u32 random = RNG_Get_RandomRange(1,N) * 2;
//		random = random * slot;
//		delay_ms(random);
//		sprintf((char*)Tran_Data,data);
		usart3_sendData("%s\r\n",data);
		
	}
    else if (LoRa_CFG.tmode == LORA_TMODE_FP)/*������*/
    {

//        addr = LORA_ADDR;/*Ŀ���ַ*/
//        chn = LORA_CHN;  /*Ŀ���ŵ�*/

//        date[i++] = (addr >> 8) & 0xff; /*��λ��ַ*/
//        date[i++] = addr & 0xff;        /*��λ��ַ*/
//        date[i] = chn;                  /*�ŵ�*/

//        for (i = 0; i < Dire_DateLen; i++) /*����д������BUFF*/
//        {
//            date[3 + i] = Dire_Date[i];
//        }

//        for (i = 0; i < (Dire_DateLen + 3); i++)
//        {
//            while ((USART3->ISR & 0X40) == 0); /*ѭ������,ֱ���������*/

//            USART3->TDR = date[i];
//        }
//        printf("tx:%02X %02X %02X %02X %02X %02X %02X %02X\r\n", date[0], date[1], date[2], date[3], date[4], date[5], date[6], date[7]); /*��ӡ���͵�����*/
//        Dire_Date[4]++;
    }

}

/**
 * @brief       Loraģ�����ݽ��մ���
 * @param       ��
 * @retval      ��
 */
void LoRa_ReceData(void)
{
    uint16_t i = 0;
    uint16_t len = 0;

    /*����������*/
    if (USART3_RX_STA & 0x8000)
    {
        len = USART3_RX_STA & 0X7FFF;
        uart3RxBuffer[len] = 0; /*��ӽ�����*/
        USART3_RX_STA = 0;

        printf("rx:");

        for (i = 0; i < len; i++)
        {
            while ((USART1->ISR & 0X40) == 0); /*ѭ������,ֱ���������*/

            USART1->TDR = uart3RxBuffer[i];
        }

        memset((char *)uart3RxBuffer, 0x00, len); /*���ڽ��ջ�������0*/

    }

}

/**
 * @brief       mw1268ģ�����
 * @param       ��
 * @retval      ��
 */
void MW1268_Test(void)
{
//nothing

}



