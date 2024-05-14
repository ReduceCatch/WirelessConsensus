/**
  ****************************************************************************************************
  * @file       mw1268_uart.c
  * @author     ����ԭ���Ŷ�(ALIENTEK)
  * @version    V1.0
  * @date       2022-2-15
  * @brief      mw196ģ�鴮������
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
#include "mw1268_uart.h"
#include "usart3.h" 
#include "delay.h"
#include "string.h" 
#include "led.h"


/**
 * @brief       ���mw1268����Ӧ�����ݺ���
 * @param       str  �ڴ���Ӧ������
 * @retval      uint8_t * �����ڴ�Ӧ�����ĵ�ַλ��  
 */
uint8_t *lora_check_cmd(uint8_t *str)
{
	
    char *strx = 0;

    if (USART3_RX_STA & 0X8000)		//���յ�һ��������
    {
        uart3RxBuffer[USART3_RX_STA & 0X7FFF] = 0; //��ӽ�����
        strx = strstr((const char *)uart3RxBuffer, (const char *)str);
    }

    return (uint8_t *)strx;
}
/**
 * @brief       mw1268����ָ���
 * @param       uint8_t* cmd,���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���
                uint8_t* ack,�ڴ���Ӧ������,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
                uint16_t waittime,�ȴ�Ӧ��ʱ��(��λ:10ms)
                
 * @retval      uint8_t Ӧ����  0,���ͳɹ�  1,����ʧ��
 */
uint8_t lora_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime)
{
    uint8_t res = 0;
    USART3_RX_STA = 0;

    if ((uint32_t)cmd <= 0XFF)
    {
        while ((USART3->ISR & 0X40) == 0); //�ȴ���һ�����ݷ������
        USART3->TDR = (uint32_t)cmd;
    }
    else usart3_sendData("%s\r\n", cmd);//��������

    if (ack && waittime)		//��Ҫ�ȴ�Ӧ��
    {
        while (--waittime)	//�ȴ�����ʱ
        {
            delay_ms(10);
            if (USART3_RX_STA & 0X8000) //���յ��ڴ���Ӧ����
            {
				if (lora_check_cmd(ack))break; //�õ���Ч����

                USART3_RX_STA = 0;
            }
        }
        if (waittime == 0)res = 1;
    }

    return res;
}
