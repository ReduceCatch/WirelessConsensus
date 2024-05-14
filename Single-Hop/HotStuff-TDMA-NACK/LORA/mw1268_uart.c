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
#include "mw1268_uart.h"
#include "usart3.h" 
#include "delay.h"
#include "string.h" 
#include "led.h"


/**
 * @brief       检测mw1268接收应答数据函数
 * @param       str  期待的应答数据
 * @retval      uint8_t * 返回期待应答结果的地址位置  
 */
uint8_t *lora_check_cmd(uint8_t *str)
{
	
    char *strx = 0;

    if (USART3_RX_STA & 0X8000)		//接收到一次数据了
    {
        uart3RxBuffer[USART3_RX_STA & 0X7FFF] = 0; //添加结束符
        strx = strstr((const char *)uart3RxBuffer, (const char *)str);
    }

    return (uint8_t *)strx;
}
/**
 * @brief       mw1268发送指令函数
 * @param       uint8_t* cmd,发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串
                uint8_t* ack,期待的应答数据,如果为控,则表示不需要等待应答
                uint16_t waittime,等待应答时间(单位:10ms)
                
 * @retval      uint8_t 应答结果  0,发送成功  1,发送失败
 */
uint8_t lora_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime)
{
    uint8_t res = 0;
    USART3_RX_STA = 0;

    if ((uint32_t)cmd <= 0XFF)
    {
        while ((USART3->ISR & 0X40) == 0); //等待上一次数据发送完成
        USART3->TDR = (uint32_t)cmd;
    }
    else usart3_sendData("%s\r\n", cmd);//发送命令

    if (ack && waittime)		//需要等待应答
    {
        while (--waittime)	//等待倒计时
        {
            delay_ms(10);
            if (USART3_RX_STA & 0X8000) //接收到期待的应答结果
            {
				if (lora_check_cmd(ack))break; //得到有效数据

                USART3_RX_STA = 0;
            }
        }
        if (waittime == 0)res = 1;
    }

    return res;
}
