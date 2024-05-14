/**
  ****************************************************************************************************
  * @file    	usart3.h
  * @author		正点原子团队(ALIENTEK)
  * @version    V1.0
  * @date		2020-04-17
  * @brief   	串口2驱动代码
  * @license   	Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
  ****************************************************************************************************
  * @attention
  *
  * 实验平台:正点原子 STM32开发板
  * 在线视频:www.yuanzige.com
  * 技术论坛:www.openedv.com
  * 公司网址:www.alientek.com
  * 购买地址:openedv.taobao.com
  *
  * 修改说明
  * V1.0 2020-04-17
  * 第一次发布
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


