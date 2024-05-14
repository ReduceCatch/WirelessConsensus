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

#ifndef _MW1268_UART_H
#define _MW1268_UART_H

#include "sys.h"


uint8_t *lora_check_cmd(uint8_t *str);
uint8_t lora_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime);


#endif
