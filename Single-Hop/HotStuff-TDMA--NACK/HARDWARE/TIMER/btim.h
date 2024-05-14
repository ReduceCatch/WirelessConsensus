/**
 ****************************************************************************************************
 * @file        btim.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2020-04-20
 * @brief       基本定时器 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F103开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20200422
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __BTIM_H
#define __BTIM_H

#include "sys.h"
#include "hotstuff.h"
/******************************************************************************************/
/* 基本定时器 定义 */

/* TIMX 中断定义 
 * 默认是针对TIM6/TIM7
 * 注意: 通过修改这4个宏定义,可以支持TIM1~TIM8任意一个定时器.
 */
 
#define BTIM_TIMX_INT                       TIM7
#define BTIM_TIMX_INT_IRQn                  TIM7_IRQn
#define BTIM_TIMX_INT_IRQHandler            TIM7_IRQHandler
#define BTIM_TIMX_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM7_CLK_ENABLE(); }while(0)   /* TIM7 时钟使能 */

#define BTIM_TIM5_INT                       TIM5
#define BTIM_TIM5_INT_IRQn                  TIM5_IRQn
#define BTIM_TIM5_INT_IRQHandler            TIM5_IRQHandler
#define BTIM_TIM5_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM5_CLK_ENABLE(); }while(0)   /* TIM5 时钟使能 */

#define BTIM_TIM2_INT                       TIM2
#define BTIM_TIM2_INT_IRQn                  TIM2_IRQn
#define BTIM_TIM2_INT_IRQHandler            TIM2_IRQHandler
#define BTIM_TIM2_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM2_CLK_ENABLE(); }while(0)   /* TIM2 时钟使能 */

#define BTIM_TIM9_INT                       TIM9
#define BTIM_TIM9_INT_IRQn                  TIM1_BRK_TIM9_IRQn
#define BTIM_TIM9_INT_IRQHandler            TIM1_BRK_TIM9_IRQHandler
#define BTIM_TIM9_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM9_CLK_ENABLE(); }while(0)   /* TIM9 时钟使能 */

#define BTIM_TIM10_INT                       TIM10
#define BTIM_TIM10_INT_IRQn                  TIM1_UP_TIM10_IRQn
#define BTIM_TIM10_INT_IRQHandler            TIM1_UP_TIM10_IRQHandler
#define BTIM_TIM10_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM10_CLK_ENABLE(); }while(0)   /* TIM10 时钟使能 */


/******************************************************************************************/

void btim_timx_int_init(uint16_t arr, uint16_t psc);    /* 基本定时器 定时中断初始化函数 */
void btim_timx_arrset(uint16_t period);  /*基本定时器 定时器预装载值函数*/
void btim_timx_counterset(uint16_t count);
void btim_timx_enable(uint8_t enable);  /*基本定时器 定时器使能函数*/

void btim_tim5_int_init(uint16_t arr, uint16_t psc);    /* 基本定时器 定时中断初始化函数 */
void btim_tim5_arrset(uint16_t period);  /*基本定时器 定时器预装载值函数*/
void btim_tim5_counterset(uint16_t count);
void btim_tim5_enable(uint8_t enable);  /*基本定时器 定时器使能函数*/


void btim_tim2_int_init(uint16_t arr, uint16_t psc);    /* 基本定时器 定时中断初始化函数 */
void btim_tim2_arrset(uint16_t period);  /*基本定时器 定时器预装载值函数*/
void btim_tim2_counterset(uint16_t count);
void btim_tim2_enable(uint8_t enable);  /*基本定时器 定时器使能函数*/

void btim_tim9_int_init(uint16_t arr, uint16_t psc);    /* 基本定时器 定时中断初始化函数 */
void btim_tim9_arrset(uint16_t period);  /*基本定时器 定时器预装载值函数*/
void btim_tim9_counterset(uint16_t count);
void btim_tim9_enable(uint8_t enable);  /*基本定时器 定时器使能函数*/

void btim_tim10_int_init(uint16_t arr, uint16_t psc);    /* 基本定时器 定时中断初始化函数 */
void btim_tim10_arrset(uint16_t period);  /*基本定时器 定时器预装载值函数*/
void btim_tim10_counterset(uint16_t count);
void btim_tim10_enable(uint8_t enable);  /*基本定时器 定时器使能函数*/

#endif

















