/**
  ****************************************************************************************************
  * @file       mw1268_app.h
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

#ifndef _MW1268_APP_H
#define _MW1268_APP_H

#include "sys.h"
#include "tendermint2.h"
#include "mw1268_cfg.h"
#include "FreeRTOS.h"
#include "task.h"
/******************************************************/
/*���Ŷ���*/

/*LORA MD0*/
#define MD0_GPIO_PORT                  GPIOG
#define MD0_GPIO_PIN                   GPIO_PIN_0
#define MD0_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOG_CLK_ENABLE(); }while(0)   /* PA��ʱ��ʹ�� */

#define LORA_MD0(x)                    do{ x ? \
                                           HAL_GPIO_WritePin(MD0_GPIO_PORT, MD0_GPIO_PIN, GPIO_PIN_SET) : \
                                           HAL_GPIO_WritePin(MD0_GPIO_PORT, MD0_GPIO_PIN, GPIO_PIN_RESET); \
                                         }while(0)   

/*LORA AUX*/
#define AUX_GPIO_PORT                  GPIOE
#define AUX_GPIO_PIN                   GPIO_PIN_15
#define AUX_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)   /* PA��ʱ��ʹ�� */

#define LORA_AUX                       HAL_GPIO_ReadPin(AUX_GPIO_PORT, AUX_GPIO_PIN)     /* ��ȡAUX���� */

#define AUX_INT_IRQn                   EXTI15_10_IRQn
#define AUX_INT_IRQHandler             EXTI15_10_IRQHandler

extern _LoRa_CFG LoRa_CFG;
extern _LORA_DEVICE_STA  Lora_Device_Sta;
extern _LORA_INT_STA     Lora_Int_Sta;
void MW1268_Test(void);


#endif

