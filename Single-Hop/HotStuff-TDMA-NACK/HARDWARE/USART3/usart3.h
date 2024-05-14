/**
  ****************************************************************************************************
  * @file    	usart3.h
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

#ifndef _USART3_H_
#define _USART3_H_


#include "sys.h"

#define UART3_BUFFER_SIZE	1024

extern UART_HandleTypeDef uart3_handler;

extern uint16_t USART3_RX_STA;

extern uint8_t uart3RxBuffer[UART3_BUFFER_SIZE];

void usart3_init(uint32_t bound);
void usart3_bpsset(uint8_t bps, uint8_t parity);

void usart3_sendData(char *fmt,...);


#endif /* _USART2_H_ */

/*******************************END OF FILE************************************/


