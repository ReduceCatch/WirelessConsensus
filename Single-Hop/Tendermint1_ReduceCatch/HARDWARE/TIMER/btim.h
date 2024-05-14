/**
 ****************************************************************************************************
 * @file        btim.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2020-04-20
 * @brief       ������ʱ�� ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32F103������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20200422
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef __BTIM_H
#define __BTIM_H

#include "sys.h"
#include "tendermint2.h"
/******************************************************************************************/
/* ������ʱ�� ���� */

/* TIMX �ж϶��� 
 * Ĭ�������TIM6/TIM7
 * ע��: ͨ���޸���4���궨��,����֧��TIM1~TIM8����һ����ʱ��.
 */
 
#define BTIM_TIMX_INT                       TIM7
#define BTIM_TIMX_INT_IRQn                  TIM7_IRQn
#define BTIM_TIMX_INT_IRQHandler            TIM7_IRQHandler
#define BTIM_TIMX_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM7_CLK_ENABLE(); }while(0)   /* TIM7 ʱ��ʹ�� */

#define BTIM_TIM5_INT                       TIM5
#define BTIM_TIM5_INT_IRQn                  TIM5_IRQn
#define BTIM_TIM5_INT_IRQHandler            TIM5_IRQHandler
#define BTIM_TIM5_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM5_CLK_ENABLE(); }while(0)   /* TIM5 ʱ��ʹ�� */

#define BTIM_TIM2_INT                       TIM2
#define BTIM_TIM2_INT_IRQn                  TIM2_IRQn
#define BTIM_TIM2_INT_IRQHandler            TIM2_IRQHandler
#define BTIM_TIM2_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM2_CLK_ENABLE(); }while(0)   /* TIM2 ʱ��ʹ�� */

#define BTIM_TIM9_INT                       TIM9
#define BTIM_TIM9_INT_IRQn                  TIM1_BRK_TIM9_IRQn
#define BTIM_TIM9_INT_IRQHandler            TIM1_BRK_TIM9_IRQHandler
#define BTIM_TIM9_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM9_CLK_ENABLE(); }while(0)   /* TIM9 ʱ��ʹ�� */

#define BTIM_TIM10_INT                       TIM10
#define BTIM_TIM10_INT_IRQn                  TIM1_UP_TIM10_IRQn
#define BTIM_TIM10_INT_IRQHandler            TIM1_UP_TIM10_IRQHandler
#define BTIM_TIM10_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM10_CLK_ENABLE(); }while(0)   /* TIM10 ʱ��ʹ�� */


/******************************************************************************************/

void btim_timx_int_init(uint16_t arr, uint16_t psc);    /* ������ʱ�� ��ʱ�жϳ�ʼ������ */
void btim_timx_arrset(uint16_t period);  /*������ʱ�� ��ʱ��Ԥװ��ֵ����*/
void btim_timx_counterset(uint16_t count);
void btim_timx_enable(uint8_t enable);  /*������ʱ�� ��ʱ��ʹ�ܺ���*/

void btim_tim5_int_init(uint16_t arr, uint16_t psc);    /* ������ʱ�� ��ʱ�жϳ�ʼ������ */
void btim_tim5_arrset(uint16_t period);  /*������ʱ�� ��ʱ��Ԥװ��ֵ����*/
void btim_tim5_counterset(uint16_t count);
void btim_tim5_enable(uint8_t enable);  /*������ʱ�� ��ʱ��ʹ�ܺ���*/


void btim_tim2_int_init(uint16_t arr, uint16_t psc);    /* ������ʱ�� ��ʱ�жϳ�ʼ������ */
void btim_tim2_arrset(uint16_t period);  /*������ʱ�� ��ʱ��Ԥװ��ֵ����*/
void btim_tim2_counterset(uint16_t count);
void btim_tim2_enable(uint8_t enable);  /*������ʱ�� ��ʱ��ʹ�ܺ���*/

void btim_tim9_int_init(uint16_t arr, uint16_t psc);    /* ������ʱ�� ��ʱ�жϳ�ʼ������ */
void btim_tim9_arrset(uint16_t period);  /*������ʱ�� ��ʱ��Ԥװ��ֵ����*/
void btim_tim9_counterset(uint16_t count);
void btim_tim9_enable(uint8_t enable);  /*������ʱ�� ��ʱ��ʹ�ܺ���*/

void btim_tim10_int_init(uint16_t arr, uint16_t psc);    /* ������ʱ�� ��ʱ�жϳ�ʼ������ */
void btim_tim10_arrset(uint16_t period);  /*������ʱ�� ��ʱ��Ԥװ��ֵ����*/
void btim_tim10_counterset(uint16_t count);
void btim_tim10_enable(uint8_t enable);  /*������ʱ�� ��ʱ��ʹ�ܺ���*/

#endif

















