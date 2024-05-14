/**
  ****************************************************************************************************
  * @file    	usart3.c
  * @author		����ԭ���Ŷ�(ALIENTEK)
  * @version    V1.0
  * @date		2020-04-17
  * @brief   	����2��������
  * @license   	Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
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
  * V1.0 2020-04-17
  * ��һ�η���
  *
  ****************************************************************************************************
  */


#include "sys.h"
#include "usart3.h"
#include "btim.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

#include "mw1268_cfg.h"


/*�����շ�������*/
uint8_t uart3RxBuffer[UART3_BUFFER_SIZE];
uint8_t uart3TxBuffer[UART3_BUFFER_SIZE];

UART_HandleTypeDef uart3_handler; 		/*UART���*/


/*
    ͨ���жϽ�������2���ַ�֮���ʱ������10ms�������ǲ���һ������������.
    ���2���ַ����ռ������timer,����Ϊ����1����������.Ҳ���ǳ���timerû�н��յ�
    �κ�����,���ʾ�˴ν������.
    ���յ�������״̬
    [15]:0,û�н��յ�����;1,���յ���һ������.
    [14:0]:���յ������ݳ���
*/
uint16_t USART3_RX_STA=0;

/**
 * @brief       ����X���ų�ʼ��
 * @param       ��
 * @retval      ��
 */
static void usart3_gpioinit(void)
{
    GPIO_InitTypeDef gpio_initure;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();

    gpio_initure.Pin = GPIO_PIN_10;
    gpio_initure.Mode = GPIO_MODE_AF_PP;
    gpio_initure.Pull = GPIO_PULLUP;
    gpio_initure.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio_initure.Alternate=GPIO_AF7_USART3;	//����ΪUSART3
    HAL_GPIO_Init(GPIOB, &gpio_initure);

    gpio_initure.Pin = GPIO_PIN_11;
    ;
    HAL_GPIO_Init(GPIOB, &gpio_initure);
}

/**
 * @brief       ����X��ʼ��
 * @param       ��
 * @retval      ��
 */
void usart3_init(uint32_t bound)
{
    
    usart3_gpioinit();

    /* USART ��ʼ������ */
    uart3_handler.Instance = USART3;
    uart3_handler.Init.BaudRate = bound;                                     /*������*/
    uart3_handler.Init.WordLength = UART_WORDLENGTH_8B;                      /*�ֳ�Ϊ8λ���ݸ�ʽ*/
    uart3_handler.Init.StopBits = UART_STOPBITS_1;                           /*һ��ֹͣλ*/
    uart3_handler.Init.Parity = UART_PARITY_NONE;                            /*����żУ��λ*/
    uart3_handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;                      /*��Ӳ������*/
    uart3_handler.Init.Mode = UART_MODE_TX_RX;                               /*�շ�ģʽ*/
    HAL_UART_Init(&uart3_handler);                                           /*HAL_UART_Init()��ʹ��UART2*/

    __HAL_UART_ENABLE_IT(&uart3_handler, UART_IT_RXNE);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
    HAL_NVIC_SetPriority(USART3_IRQn, 3, 3);
    
    btim_timx_int_init(99, 8399); //10ms�ж�
    USART3_RX_STA = 0; //����
    btim_timx_enable(DISABLE); //�رն�ʱ��

  
}

/**
 * @brief       ����X��������
 * @param       bps    ������
 * @param       parity У��λ               
 * @retval      ��
 */
void usart3_bpsset(uint8_t bps, uint8_t parity)
{
    static uint32_t bound = 0;

    switch (bps)
    {
        case LORA_BRD_1200: bound = 1200; break;
        case LORA_BRD_2400: bound = 2400; break;
        case LORA_BRD_4800: bound = 4800; break;
        case LORA_BRD_9600: bound = 9600; break;
        case LORA_BRD_19200: bound = 19200;break;
        case LORA_BRD_38400: bound = 38400; break;
        case LORA_BRD_57600: bound = 57600; break;
        case LORA_BRD_115200:bound = 115200;break;
    }
    
    __HAL_UART_DISABLE(&uart3_handler);
    uart3_handler.Init.BaudRate = bound;  
    uart3_handler.Init.StopBits = UART_STOPBITS_1;
    
    if(parity == LORA_BRDVER_8N1) //��У��
    {
        uart3_handler.Init.WordLength = UART_WORDLENGTH_8B;
        uart3_handler.Init.Parity = UART_PARITY_NONE;       
    }
    else if(parity == LORA_BRDVER_8E1) //żУ��
    {
        uart3_handler.Init.WordLength = UART_WORDLENGTH_9B;
        uart3_handler.Init.Parity = UART_PARITY_EVEN; 
    }
    else if(parity == LORA_BRDVER_8O1) //��У��
    {
        uart3_handler.Init.WordLength = UART_WORDLENGTH_9B;
        uart3_handler.Init.Parity = UART_PARITY_ODD; 
    }
    
     HAL_UART_Init(&uart3_handler);
     
}  

/**
  * @brief  ����3����
  * @note  ȷ��һ�η������ݲ�����UART3_BUFFER_SIZE�ֽ�
  * @retval ��
  */
void usart3_sendData(char *fmt,...)
{
    uint16_t len=0;
    va_list ap;
    va_start(ap, fmt);
    vsprintf((char *)uart3TxBuffer, fmt, ap);
    va_end(ap);
    len = strlen((const char *)uart3TxBuffer); //�˴η������ݵĳ���

    HAL_UART_Transmit(&uart3_handler, (uint8_t *)uart3TxBuffer, len, 500);
}


/**
 * @brief       ����X�жϷ�����
 * @param       ��
 * @retval      ��
 */
void USART3_IRQHandler(void)
{
    HAL_UART_IRQHandler(&uart3_handler);

    if (__HAL_UART_GET_FLAG(&uart3_handler, UART_FLAG_RXNE) != RESET) /*!< ���շǿ��ж� */
    {
        uint8_t res = uart3_handler.Instance->RDR;
        
        if ((USART3_RX_STA & (1 << 15)) == 0)       //�������һ������,��û�б�����,���ٽ�����������
        {
            if (USART3_RX_STA < UART3_BUFFER_SIZE) //�����Խ�������
            {
                 if (!Lora_Device_Sta) //���ù�����(������ʱ����ʱ)
                 {
                    btim_timx_counterset(0);            //���������

                    if (USART3_RX_STA == 0)  //ʹ�ܶ�ʱ��7���ж�
                    {
                        btim_timx_enable(ENABLE);          //ʹ�ܶ�ʱ��7
                    } 
                 }

                uart3RxBuffer[USART3_RX_STA++] = res; //��¼���յ���ֵ
            }
            else
            {
                USART3_RX_STA |= 1 << 15; //ǿ�Ʊ�ǽ������
            }
        }
        
        
        
    }
}




/*******************************END OF FILE************************************/

//���ڽ���ʹ�ܿ���
//enable:0.�ر� 1,��
void usart3_rx(u8 enable)
{
	 USART3->CR1 &= ~(1<<0);//�رմ���
	
	 if(enable)
	 {
		 USART3->CR1|=(1<<3)|(1<<2);//���ͺͽ���
	 }else
	 { 
		 USART3->CR1&=~(1<<2);//�رս���
		 USART3->CR1|=(1<<3);//�򿪷���
	 }
	 
	 USART3->CR1 |= (1<<0);//ʹ�ܴ��� 
}

