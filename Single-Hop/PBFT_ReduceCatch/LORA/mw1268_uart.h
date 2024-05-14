/**
  ****************************************************************************************************
  * @file       mw1268_uart.c
  * @author     正点原子团队(ALIENTEK)
  * @version    V1.0
  * @date       2022-2-15
  * @brief      mw196模块串口驱动
  * @license   	Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
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
  * V1.0 2022-2-15
  * 第一次发布
  *
  ****************************************************************************************************
  */

#ifndef _MW1268_UART_H
#define _MW1268_UART_H

#include "sys.h"


uint8_t *lora_check_cmd(uint8_t *str);
uint8_t lora_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime);


#endif
