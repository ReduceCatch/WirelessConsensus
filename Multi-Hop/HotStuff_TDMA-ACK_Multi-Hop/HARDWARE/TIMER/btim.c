/**
 ****************************************************************************************************
 * @file        btim.c
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

#include "btim.h"
#include "usart3.h"


TIM_HandleTypeDef timx_handler; /* 定时器参数句柄 */
TIM_HandleTypeDef tim5_handler; /* 定时器参数句柄 */
TIM_HandleTypeDef tim2_handler; /* 定时器参数句柄 */
TIM_HandleTypeDef tim9_handler; /* 定时器参数句柄 */
TIM_HandleTypeDef tim10_handler; /* 定时器参数句柄 */
/**
 * @brief       定时器底册驱动，开启时钟，设置中断优先级
                此函数会被HAL_TIM_Base_Init()函数调用
 * @param       无
 * @retval      无
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == BTIM_TIMX_INT)
    {
        BTIM_TIMX_INT_CLK_ENABLE();                     /* 使能TIM时钟 */
        HAL_NVIC_SetPriority(BTIM_TIMX_INT_IRQn, 1, 3); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(BTIM_TIMX_INT_IRQn);         /* 开启ITMx中断 */
    }
	if (htim->Instance == BTIM_TIM5_INT)
    {
        BTIM_TIM5_INT_CLK_ENABLE();                     /* 使能TIM时钟 */
        HAL_NVIC_SetPriority(BTIM_TIM5_INT_IRQn, 1, 3); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(BTIM_TIM5_INT_IRQn);         /* 开启ITMx中断 */
    }
	if (htim->Instance == BTIM_TIM2_INT)
    {
        BTIM_TIM2_INT_CLK_ENABLE();                     /* 使能TIM时钟 */
        HAL_NVIC_SetPriority(BTIM_TIM2_INT_IRQn, 1, 3); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(BTIM_TIM2_INT_IRQn);         /* 开启ITMx中断 */
    }
	if (htim->Instance == BTIM_TIM9_INT)
    {
        BTIM_TIM9_INT_CLK_ENABLE();                     /* 使能TIM时钟 */
        HAL_NVIC_SetPriority(BTIM_TIM9_INT_IRQn, 1, 3); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(BTIM_TIM9_INT_IRQn);         /* 开启ITMx中断 */
    }
	if (htim->Instance == BTIM_TIM10_INT)
    {
        BTIM_TIM10_INT_CLK_ENABLE();                     /* 使能TIM时钟 */
        HAL_NVIC_SetPriority(BTIM_TIM10_INT_IRQn, 1, 3); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(BTIM_TIM10_INT_IRQn);         /* 开启ITMx中断 */
    }
}
/**
 * @brief       回调函数，定时器中断服务函数调用
 * @param       无
 * @retval      无
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == (&timx_handler))
    {
        USART3_RX_STA |= 1 << 15; //标记接收完成
        btim_timx_enable(DISABLE); //关闭TIM7
    }
	if (htim == (&tim5_handler))
    {
		TIM5_Exceed_Times = TIM5_Exceed_Times + 1;
		if(TIM5_Exceed_Times >= 5){
			
			TIM_Prepare_Flag = 0;
			TIM_Precommit_Flag = 0;
			TIM_Commit_Flag = 0;
			TIM_Decide_Flag = 0;
			Timer_Flag_5 = 0;
			TIM5_Exceed_Times = 0;
		}
        
    }
	if (htim == (&tim2_handler))
    {
        TIM2_Exceed_Times = TIM2_Exceed_Times + 1;
    }
	if (htim == (&tim9_handler))
    {
        if(!start_flag2){
			u8 id_idx = id - 4 * (Group_Number - 1);
			TIM9_Exceed_Times = TIM9_Exceed_Times + 1;
			if(before_round && !after_round){
				if(TIM9_Exceed_Times == (id_idx-1)*TIM_slot){
					before_round = 0;
					after_round = 0xff;
				}
			}else if(!before_round && after_round){
				if(TIM9_Exceed_Times == N*TIM_slot){
					before_round = 0xff;
					after_round = 0;
					TIM9_Exceed_Times=0;
				}
			}
		}else{
			u8 id_idx = Group_Number;
			TIM9_Exceed_Times = TIM9_Exceed_Times + 1;
			if(before_round && !after_round){
				if(TIM9_Exceed_Times == (id_idx-1)*TIM_slot){
					before_round = 0;
					after_round = 0xff;
				}
			}else if(!before_round && after_round){
				if(TIM9_Exceed_Times >= GROUPS_NUMBER*TIM_slot){
					before_round = 0xff;
					after_round = 0;
					TIM9_Exceed_Times=0;
				}
			}
		}
    }
	if (htim == (&tim10_handler))
    {
//		//random kill
//		TIM10_Exceed_Times = TIM10_Exceed_Times + 1;
//		if(TIM10_Exceed_Times == 2){
//			
//			DONE_OR_NOT();
//			
//			TIM10_Exceed_Times = 0;
//		}
    }
}
/**********************TIM7*********************************************************/
/**
 * @brief       基本定时器TIMX中断服务函数
 * @param       无
 * @retval      无
 */
void BTIM_TIMX_INT_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&timx_handler); /* 定时器回调函数 */
}


/**
 * @brief       设置预装载周期值
 * @param       无
 * @retval      无
 */
void btim_timx_arrset(uint16_t period)
{
   __HAL_TIM_SET_COUNTER(&timx_handler,0);
    __HAL_TIM_SET_AUTORELOAD(&timx_handler,period);    
}

/**
 * @brief       设置计数值
 * @param       无
 * @retval      无
 */
void btim_timx_counterset(uint16_t count)
{
    __HAL_TIM_SET_COUNTER(&timx_handler,count); 
}



/**
 * @brief       使能基本定时器
 * @param       无
 * @retval      无
 */
void btim_timx_enable(uint8_t enable)
{
    static TIM_HandleTypeDef *p = &timx_handler;
    
    if(enable) p->Instance->CR1|=(TIM_CR1_CEN);
    else       p->Instance->CR1 &= ~(TIM_CR1_CEN);
}


/**
 * @brief       基本定时器TIMX定时中断初始化函数
 * @note
 *              基本定时器的时钟来自APB1,当PPRE1 ≥ 2分频的时候
 *              基本定时器的时钟为APB1时钟的2倍, 而APB1为36M, 所以定时器时钟 = 72Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值。
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void btim_timx_int_init(uint16_t arr, uint16_t psc)
{
    timx_handler.Instance = BTIM_TIMX_INT;                      /* 通用定时器X */
    timx_handler.Init.Prescaler = psc;                          /* 设置预分频器  */
    timx_handler.Init.CounterMode = TIM_COUNTERMODE_UP;         /* 向上计数器 */
    timx_handler.Init.Period = arr;                             /* 自动装载值 */
    timx_handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;   /* 时钟分频因子 */
    HAL_TIM_Base_Init(&timx_handler);

    HAL_TIM_Base_Start_IT(&timx_handler);                       /* 使能通用定时器x和及其更新中断：TIM_IT_UPDATE */
    
}
/*******************************************************************************/



/**********************TIM5*********************************************************/
/**
 * @brief       基本定时器TIM5定时中断初始化函数
 * @note
 *              基本定时器的时钟来自APB1,当PPRE1 ≥ 2分频的时候
 *              基本定时器的时钟为APB1时钟的2倍, 而APB1为36M, 所以定时器时钟 = 72Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值。
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void btim_tim5_int_init(uint16_t arr, uint16_t psc)
{
    tim5_handler.Instance = BTIM_TIM5_INT;                      /* 通用定时器5 */
    tim5_handler.Init.Prescaler = psc;                          /* 设置预分频器  */
    tim5_handler.Init.CounterMode = TIM_COUNTERMODE_UP;         /* 向上计数器 */
    tim5_handler.Init.Period = arr;                             /* 自动装载值 */
    tim5_handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;   /* 时钟分频因子 */
    HAL_TIM_Base_Init(&tim5_handler);
	
    HAL_TIM_Base_Start_IT(&tim5_handler);                       /* 使能通用定时器x和及其更新中断：TIM_IT_UPDATE */
    
}


/**
 * @brief       设置预装载周期值
 * @param       无
 * @retval      无
 */
void btim_tim5_arrset(uint16_t period)
{
   __HAL_TIM_SET_COUNTER(&tim5_handler,0);
    __HAL_TIM_SET_AUTORELOAD(&tim5_handler,period);    
}

/**
 * @brief       设置计数值
 * @param       无
 * @retval      无
 */
void btim_tim5_counterset(uint16_t count)
{
    __HAL_TIM_SET_COUNTER(&tim5_handler,count); 
}

/**
 * @brief       使能基本定时器
 * @param       无
 * @retval      无
 */
void btim_tim5_enable(uint8_t enable)
{
    static TIM_HandleTypeDef *p = &tim5_handler;
    
    if(enable) p->Instance->CR1|=(TIM_CR1_CEN);
    else       p->Instance->CR1 &= ~(TIM_CR1_CEN);
}

/**
 * @brief       基本定时器TIM5中断服务函数
 * @param       无
 * @retval      无
 */
void BTIM_TIM5_INT_IRQHandler(void)
{		
    HAL_TIM_IRQHandler(&tim5_handler); /* 定时器回调函数 */
}
/*******************************************************************************/

/**********************TIM2*********************************************************/
/**
 * @brief       基本定时器TIM2定时中断初始化函数
 * @note
 *              基本定时器的时钟来自APB1,当PPRE1 ≥ 2分频的时候
 *              基本定时器的时钟为APB1时钟的2倍, 而APB1为36M, 所以定时器时钟 = 72Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值。
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void btim_tim2_int_init(uint16_t arr, uint16_t psc)
{
    tim2_handler.Instance = BTIM_TIM2_INT;                      /* 通用定时器5 */
    tim2_handler.Init.Prescaler = psc;                          /* 设置预分频器  */
    tim2_handler.Init.CounterMode = TIM_COUNTERMODE_UP;         /* 向上计数器 */
    tim2_handler.Init.Period = arr;                             /* 自动装载值 */
    tim2_handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;   /* 时钟分频因子 */
    HAL_TIM_Base_Init(&tim2_handler);
	
    HAL_TIM_Base_Start_IT(&tim2_handler);                       /* 使能通用定时器x和及其更新中断：TIM_IT_UPDATE */
    
}


/**
 * @brief       设置预装载周期值
 * @param       无
 * @retval      无
 */
void btim_tim2_arrset(uint16_t period)
{
   __HAL_TIM_SET_COUNTER(&tim2_handler,0);
    __HAL_TIM_SET_AUTORELOAD(&tim2_handler,period);    
}

/**
 * @brief       设置计数值
 * @param       无
 * @retval      无
 */
void btim_tim2_counterset(uint16_t count)
{
    __HAL_TIM_SET_COUNTER(&tim2_handler,count); 
}

/**
 * @brief       使能基本定时器
 * @param       无
 * @retval      无
 */
void btim_tim2_enable(uint8_t enable)
{
    static TIM_HandleTypeDef *p = &tim2_handler;
    
    if(enable) p->Instance->CR1|=(TIM_CR1_CEN);
    else       p->Instance->CR1 &= ~(TIM_CR1_CEN);
}

/**
 * @brief       基本定时器TIM2中断服务函数
 * @param       无
 * @retval      无
 */
void BTIM_TIM2_INT_IRQHandler(void)
{		
    HAL_TIM_IRQHandler(&tim2_handler); /* 定时器回调函数 */
}
/*******************************************************************************/

/**********************TIM9*********************************************************/
/**
 * @brief       基本定时器TIM9定时中断初始化函数
 * @note
 *              基本定时器的时钟来自APB1,当PPRE1 ≥ 2分频的时候
 *              基本定时器的时钟为APB1时钟的2倍, 而APB1为36M, 所以定时器时钟 = 72Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值。
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void btim_tim9_int_init(uint16_t arr, uint16_t psc)
{
    tim9_handler.Instance = BTIM_TIM9_INT;                      /* 9 */
    tim9_handler.Init.Prescaler = psc;                          /* 设置预分频器  */
    tim9_handler.Init.CounterMode = TIM_COUNTERMODE_UP;         /* 向上计数器 */
    tim9_handler.Init.Period = arr;                             /* 自动装载值 */
    tim9_handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;   /* 时钟分频因子 */
    HAL_TIM_Base_Init(&tim9_handler);
	
    HAL_TIM_Base_Start_IT(&tim9_handler);                       /* 使能通用定时器x和及其更新中断：TIM_IT_UPDATE */
    
}


/**
 * @brief       设置预装载周期值
 * @param       无
 * @retval      无
 */
void btim_tim9_arrset(uint16_t period)
{
   __HAL_TIM_SET_COUNTER(&tim9_handler,0);
    __HAL_TIM_SET_AUTORELOAD(&tim9_handler,period);    
}

/**
 * @brief       设置计数值
 * @param       无
 * @retval      无
 */
void btim_tim9_counterset(uint16_t count)
{
    __HAL_TIM_SET_COUNTER(&tim9_handler,count); 
}

/**
 * @brief       使能基本定时器
 * @param       无
 * @retval      无
 */
void btim_tim9_enable(uint8_t enable)
{
    static TIM_HandleTypeDef *p = &tim9_handler;
    
    if(enable) p->Instance->CR1|=(TIM_CR1_CEN);
    else       p->Instance->CR1 &= ~(TIM_CR1_CEN);
}

/**
 * @brief       基本定时器TIM9中断服务函数
 * @param       无
 * @retval      无
 */
void BTIM_TIM9_INT_IRQHandler(void)
{		
    HAL_TIM_IRQHandler(&tim9_handler); /* 定时器回调函数 */
}
/*******************************************************************************/

/**********************TIM10*********************************************************/
/**
 * @brief       基本定时器TIM10定时中断初始化函数
 * @note
 *              基本定时器的时钟来自APB1,当PPRE1 ≥ 2分频的时候
 *              基本定时器的时钟为APB1时钟的2倍, 而APB1为36M, 所以定时器时钟 = 72Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值。
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void btim_tim10_int_init(uint16_t arr, uint16_t psc)
{
    tim10_handler.Instance = BTIM_TIM10_INT;                      /* 9 */
    tim10_handler.Init.Prescaler = psc;                          /* 设置预分频器  */
    tim10_handler.Init.CounterMode = TIM_COUNTERMODE_UP;         /* 向上计数器 */
    tim10_handler.Init.Period = arr;                             /* 自动装载值 */
    tim10_handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;   /* 时钟分频因子 */
    HAL_TIM_Base_Init(&tim10_handler);
	
    HAL_TIM_Base_Start_IT(&tim10_handler);                       /* 使能通用定时器x和及其更新中断：TIM_IT_UPDATE */
    
}


/**
 * @brief       设置预装载周期值
 * @param       无
 * @retval      无
 */
void btim_tim10_arrset(uint16_t period)
{
   __HAL_TIM_SET_COUNTER(&tim10_handler,0);
    __HAL_TIM_SET_AUTORELOAD(&tim10_handler,period);    
}

/**
 * @brief       设置计数值
 * @param       无
 * @retval      无
 */
void btim_tim10_counterset(uint16_t count)
{
    __HAL_TIM_SET_COUNTER(&tim10_handler,count); 
}

/**
 * @brief       使能基本定时器
 * @param       无
 * @retval      无
 */
void btim_tim10_enable(uint8_t enable)
{
    static TIM_HandleTypeDef *p = &tim10_handler;
    
    if(enable) p->Instance->CR1|=(TIM_CR1_CEN);
    else       p->Instance->CR1 &= ~(TIM_CR1_CEN);
}

/**
 * @brief       基本定时器TIM5中断服务函数
 * @param       无
 * @retval      无
 */
void BTIM_TIM10_INT_IRQHandler(void)
{		
    HAL_TIM_IRQHandler(&tim10_handler); /* 定时器回调函数 */
}
/*******************************************************************************/

//TIM4 PWM部分初始化 
//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数
void TIM4_PWM_Init(u32 arr,u32 psc)
{		 					 
	//此部分需手动修改IO口设置
	RCC->APB1ENR|=1<<2;		//TIM4时钟使能    
	RCC->AHB1ENR|=1<<3;   	//使能PORTD时钟	
	GPIO_Set(GPIOD,PIN14|PIN15,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);//复用功能,上拉输出
	GPIO_AF_Set(GPIOD,14,2);	//PD14,AF2 
	GPIO_AF_Set(GPIOD,15,2);	//PD15,AF2
	
	TIM4->ARR=arr;			//设定计数器自动重装值 
	TIM4->PSC=psc;			//预分频器不分频 
	
	TIM4->CCMR2|=6<<4;  	//CH3 PWM1模式		 
	TIM4->CCMR2|=1<<3; 		//CH3 预装载使能	   
	TIM4->CCER|=1<<9;   	//OC3 输出使能	
	TIM4->CCER|=1<<8;   	//OC3 低电平有效	   
	
	TIM4->CCMR2|=6<<12;  	//CH4 PWM1模式		 
	TIM4->CCMR2|=1<<11; 	//CH4 预装载使能	   
	TIM4->CCER|=1<<12;   	//OC4 输出使能	
	TIM4->CCER|=1<<13;   	//OC4 低电平有效
	
	TIM4->CR1|=1<<7;   		//ARPE使能 
	TIM4->CR1|=1<<0;    	//使能定时器3											  
}  

