/**
 ****************************************************************************************************
 * @file        btim.c
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

#include "btim.h"
#include "usart3.h"


TIM_HandleTypeDef timx_handler; /* ��ʱ��������� */
TIM_HandleTypeDef tim5_handler; /* ��ʱ��������� */
TIM_HandleTypeDef tim2_handler; /* ��ʱ��������� */
TIM_HandleTypeDef tim9_handler; /* ��ʱ��������� */
TIM_HandleTypeDef tim10_handler; /* ��ʱ��������� */
/**
 * @brief       ��ʱ���ײ�����������ʱ�ӣ������ж����ȼ�
                �˺����ᱻHAL_TIM_Base_Init()��������
 * @param       ��
 * @retval      ��
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == BTIM_TIMX_INT)
    {
        BTIM_TIMX_INT_CLK_ENABLE();                     /* ʹ��TIMʱ�� */
        HAL_NVIC_SetPriority(BTIM_TIMX_INT_IRQn, 1, 3); /* ��ռ1�������ȼ�3����2 */
        HAL_NVIC_EnableIRQ(BTIM_TIMX_INT_IRQn);         /* ����ITMx�ж� */
    }
	if (htim->Instance == BTIM_TIM5_INT)
    {
        BTIM_TIM5_INT_CLK_ENABLE();                     /* ʹ��TIMʱ�� */
        HAL_NVIC_SetPriority(BTIM_TIM5_INT_IRQn, 1, 3); /* ��ռ1�������ȼ�3����2 */
        HAL_NVIC_EnableIRQ(BTIM_TIM5_INT_IRQn);         /* ����ITMx�ж� */
    }
	if (htim->Instance == BTIM_TIM2_INT)
    {
        BTIM_TIM2_INT_CLK_ENABLE();                     /* ʹ��TIMʱ�� */
        HAL_NVIC_SetPriority(BTIM_TIM2_INT_IRQn, 1, 3); /* ��ռ1�������ȼ�3����2 */
        HAL_NVIC_EnableIRQ(BTIM_TIM2_INT_IRQn);         /* ����ITMx�ж� */
    }
	if (htim->Instance == BTIM_TIM9_INT)
    {
        BTIM_TIM9_INT_CLK_ENABLE();                     /* ʹ��TIMʱ�� */
        HAL_NVIC_SetPriority(BTIM_TIM9_INT_IRQn, 1, 3); /* ��ռ1�������ȼ�3����2 */
        HAL_NVIC_EnableIRQ(BTIM_TIM9_INT_IRQn);         /* ����ITMx�ж� */
    }
	if (htim->Instance == BTIM_TIM10_INT)
    {
        BTIM_TIM10_INT_CLK_ENABLE();                     /* ʹ��TIMʱ�� */
        HAL_NVIC_SetPriority(BTIM_TIM10_INT_IRQn, 1, 3); /* ��ռ1�������ȼ�3����2 */
        HAL_NVIC_EnableIRQ(BTIM_TIM10_INT_IRQn);         /* ����ITMx�ж� */
    }
}
/**
 * @brief       �ص���������ʱ���жϷ���������
 * @param       ��
 * @retval      ��
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == (&timx_handler))
    {
        USART3_RX_STA |= 1 << 15; //��ǽ������
        btim_timx_enable(DISABLE); //�ر�TIM7
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
 * @brief       ������ʱ��TIMX�жϷ�����
 * @param       ��
 * @retval      ��
 */
void BTIM_TIMX_INT_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&timx_handler); /* ��ʱ���ص����� */
}


/**
 * @brief       ����Ԥװ������ֵ
 * @param       ��
 * @retval      ��
 */
void btim_timx_arrset(uint16_t period)
{
   __HAL_TIM_SET_COUNTER(&timx_handler,0);
    __HAL_TIM_SET_AUTORELOAD(&timx_handler,period);    
}

/**
 * @brief       ���ü���ֵ
 * @param       ��
 * @retval      ��
 */
void btim_timx_counterset(uint16_t count)
{
    __HAL_TIM_SET_COUNTER(&timx_handler,count); 
}



/**
 * @brief       ʹ�ܻ�����ʱ��
 * @param       ��
 * @retval      ��
 */
void btim_timx_enable(uint8_t enable)
{
    static TIM_HandleTypeDef *p = &timx_handler;
    
    if(enable) p->Instance->CR1|=(TIM_CR1_CEN);
    else       p->Instance->CR1 &= ~(TIM_CR1_CEN);
}


/**
 * @brief       ������ʱ��TIMX��ʱ�жϳ�ʼ������
 * @note
 *              ������ʱ����ʱ������APB1,��PPRE1 �� 2��Ƶ��ʱ��
 *              ������ʱ����ʱ��ΪAPB1ʱ�ӵ�2��, ��APB1Ϊ36M, ���Զ�ʱ��ʱ�� = 72Mhz
 *              ��ʱ�����ʱ����㷽��: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=��ʱ������Ƶ��,��λ:Mhz
 *
 * @param       arr: �Զ���װֵ��
 * @param       psc: ʱ��Ԥ��Ƶ��
 * @retval      ��
 */
void btim_timx_int_init(uint16_t arr, uint16_t psc)
{
    timx_handler.Instance = BTIM_TIMX_INT;                      /* ͨ�ö�ʱ��X */
    timx_handler.Init.Prescaler = psc;                          /* ����Ԥ��Ƶ��  */
    timx_handler.Init.CounterMode = TIM_COUNTERMODE_UP;         /* ���ϼ����� */
    timx_handler.Init.Period = arr;                             /* �Զ�װ��ֵ */
    timx_handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;   /* ʱ�ӷ�Ƶ���� */
    HAL_TIM_Base_Init(&timx_handler);

    HAL_TIM_Base_Start_IT(&timx_handler);                       /* ʹ��ͨ�ö�ʱ��x�ͼ�������жϣ�TIM_IT_UPDATE */
    
}
/*******************************************************************************/



/**********************TIM5*********************************************************/
/**
 * @brief       ������ʱ��TIM5��ʱ�жϳ�ʼ������
 * @note
 *              ������ʱ����ʱ������APB1,��PPRE1 �� 2��Ƶ��ʱ��
 *              ������ʱ����ʱ��ΪAPB1ʱ�ӵ�2��, ��APB1Ϊ36M, ���Զ�ʱ��ʱ�� = 72Mhz
 *              ��ʱ�����ʱ����㷽��: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=��ʱ������Ƶ��,��λ:Mhz
 *
 * @param       arr: �Զ���װֵ��
 * @param       psc: ʱ��Ԥ��Ƶ��
 * @retval      ��
 */
void btim_tim5_int_init(uint16_t arr, uint16_t psc)
{
    tim5_handler.Instance = BTIM_TIM5_INT;                      /* ͨ�ö�ʱ��5 */
    tim5_handler.Init.Prescaler = psc;                          /* ����Ԥ��Ƶ��  */
    tim5_handler.Init.CounterMode = TIM_COUNTERMODE_UP;         /* ���ϼ����� */
    tim5_handler.Init.Period = arr;                             /* �Զ�װ��ֵ */
    tim5_handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;   /* ʱ�ӷ�Ƶ���� */
    HAL_TIM_Base_Init(&tim5_handler);
	
    HAL_TIM_Base_Start_IT(&tim5_handler);                       /* ʹ��ͨ�ö�ʱ��x�ͼ�������жϣ�TIM_IT_UPDATE */
    
}


/**
 * @brief       ����Ԥװ������ֵ
 * @param       ��
 * @retval      ��
 */
void btim_tim5_arrset(uint16_t period)
{
   __HAL_TIM_SET_COUNTER(&tim5_handler,0);
    __HAL_TIM_SET_AUTORELOAD(&tim5_handler,period);    
}

/**
 * @brief       ���ü���ֵ
 * @param       ��
 * @retval      ��
 */
void btim_tim5_counterset(uint16_t count)
{
    __HAL_TIM_SET_COUNTER(&tim5_handler,count); 
}

/**
 * @brief       ʹ�ܻ�����ʱ��
 * @param       ��
 * @retval      ��
 */
void btim_tim5_enable(uint8_t enable)
{
    static TIM_HandleTypeDef *p = &tim5_handler;
    
    if(enable) p->Instance->CR1|=(TIM_CR1_CEN);
    else       p->Instance->CR1 &= ~(TIM_CR1_CEN);
}

/**
 * @brief       ������ʱ��TIM5�жϷ�����
 * @param       ��
 * @retval      ��
 */
void BTIM_TIM5_INT_IRQHandler(void)
{		
    HAL_TIM_IRQHandler(&tim5_handler); /* ��ʱ���ص����� */
}
/*******************************************************************************/

/**********************TIM2*********************************************************/
/**
 * @brief       ������ʱ��TIM2��ʱ�жϳ�ʼ������
 * @note
 *              ������ʱ����ʱ������APB1,��PPRE1 �� 2��Ƶ��ʱ��
 *              ������ʱ����ʱ��ΪAPB1ʱ�ӵ�2��, ��APB1Ϊ36M, ���Զ�ʱ��ʱ�� = 72Mhz
 *              ��ʱ�����ʱ����㷽��: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=��ʱ������Ƶ��,��λ:Mhz
 *
 * @param       arr: �Զ���װֵ��
 * @param       psc: ʱ��Ԥ��Ƶ��
 * @retval      ��
 */
void btim_tim2_int_init(uint16_t arr, uint16_t psc)
{
    tim2_handler.Instance = BTIM_TIM2_INT;                      /* ͨ�ö�ʱ��5 */
    tim2_handler.Init.Prescaler = psc;                          /* ����Ԥ��Ƶ��  */
    tim2_handler.Init.CounterMode = TIM_COUNTERMODE_UP;         /* ���ϼ����� */
    tim2_handler.Init.Period = arr;                             /* �Զ�װ��ֵ */
    tim2_handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;   /* ʱ�ӷ�Ƶ���� */
    HAL_TIM_Base_Init(&tim2_handler);
	
    HAL_TIM_Base_Start_IT(&tim2_handler);                       /* ʹ��ͨ�ö�ʱ��x�ͼ�������жϣ�TIM_IT_UPDATE */
    
}


/**
 * @brief       ����Ԥװ������ֵ
 * @param       ��
 * @retval      ��
 */
void btim_tim2_arrset(uint16_t period)
{
   __HAL_TIM_SET_COUNTER(&tim2_handler,0);
    __HAL_TIM_SET_AUTORELOAD(&tim2_handler,period);    
}

/**
 * @brief       ���ü���ֵ
 * @param       ��
 * @retval      ��
 */
void btim_tim2_counterset(uint16_t count)
{
    __HAL_TIM_SET_COUNTER(&tim2_handler,count); 
}

/**
 * @brief       ʹ�ܻ�����ʱ��
 * @param       ��
 * @retval      ��
 */
void btim_tim2_enable(uint8_t enable)
{
    static TIM_HandleTypeDef *p = &tim2_handler;
    
    if(enable) p->Instance->CR1|=(TIM_CR1_CEN);
    else       p->Instance->CR1 &= ~(TIM_CR1_CEN);
}

/**
 * @brief       ������ʱ��TIM2�жϷ�����
 * @param       ��
 * @retval      ��
 */
void BTIM_TIM2_INT_IRQHandler(void)
{		
    HAL_TIM_IRQHandler(&tim2_handler); /* ��ʱ���ص����� */
}
/*******************************************************************************/

/**********************TIM9*********************************************************/
/**
 * @brief       ������ʱ��TIM9��ʱ�жϳ�ʼ������
 * @note
 *              ������ʱ����ʱ������APB1,��PPRE1 �� 2��Ƶ��ʱ��
 *              ������ʱ����ʱ��ΪAPB1ʱ�ӵ�2��, ��APB1Ϊ36M, ���Զ�ʱ��ʱ�� = 72Mhz
 *              ��ʱ�����ʱ����㷽��: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=��ʱ������Ƶ��,��λ:Mhz
 *
 * @param       arr: �Զ���װֵ��
 * @param       psc: ʱ��Ԥ��Ƶ��
 * @retval      ��
 */
void btim_tim9_int_init(uint16_t arr, uint16_t psc)
{
    tim9_handler.Instance = BTIM_TIM9_INT;                      /* 9 */
    tim9_handler.Init.Prescaler = psc;                          /* ����Ԥ��Ƶ��  */
    tim9_handler.Init.CounterMode = TIM_COUNTERMODE_UP;         /* ���ϼ����� */
    tim9_handler.Init.Period = arr;                             /* �Զ�װ��ֵ */
    tim9_handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;   /* ʱ�ӷ�Ƶ���� */
    HAL_TIM_Base_Init(&tim9_handler);
	
    HAL_TIM_Base_Start_IT(&tim9_handler);                       /* ʹ��ͨ�ö�ʱ��x�ͼ�������жϣ�TIM_IT_UPDATE */
    
}


/**
 * @brief       ����Ԥװ������ֵ
 * @param       ��
 * @retval      ��
 */
void btim_tim9_arrset(uint16_t period)
{
   __HAL_TIM_SET_COUNTER(&tim9_handler,0);
    __HAL_TIM_SET_AUTORELOAD(&tim9_handler,period);    
}

/**
 * @brief       ���ü���ֵ
 * @param       ��
 * @retval      ��
 */
void btim_tim9_counterset(uint16_t count)
{
    __HAL_TIM_SET_COUNTER(&tim9_handler,count); 
}

/**
 * @brief       ʹ�ܻ�����ʱ��
 * @param       ��
 * @retval      ��
 */
void btim_tim9_enable(uint8_t enable)
{
    static TIM_HandleTypeDef *p = &tim9_handler;
    
    if(enable) p->Instance->CR1|=(TIM_CR1_CEN);
    else       p->Instance->CR1 &= ~(TIM_CR1_CEN);
}

/**
 * @brief       ������ʱ��TIM9�жϷ�����
 * @param       ��
 * @retval      ��
 */
void BTIM_TIM9_INT_IRQHandler(void)
{		
    HAL_TIM_IRQHandler(&tim9_handler); /* ��ʱ���ص����� */
}
/*******************************************************************************/

/**********************TIM10*********************************************************/
/**
 * @brief       ������ʱ��TIM10��ʱ�жϳ�ʼ������
 * @note
 *              ������ʱ����ʱ������APB1,��PPRE1 �� 2��Ƶ��ʱ��
 *              ������ʱ����ʱ��ΪAPB1ʱ�ӵ�2��, ��APB1Ϊ36M, ���Զ�ʱ��ʱ�� = 72Mhz
 *              ��ʱ�����ʱ����㷽��: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=��ʱ������Ƶ��,��λ:Mhz
 *
 * @param       arr: �Զ���װֵ��
 * @param       psc: ʱ��Ԥ��Ƶ��
 * @retval      ��
 */
void btim_tim10_int_init(uint16_t arr, uint16_t psc)
{
    tim10_handler.Instance = BTIM_TIM10_INT;                      /* 9 */
    tim10_handler.Init.Prescaler = psc;                          /* ����Ԥ��Ƶ��  */
    tim10_handler.Init.CounterMode = TIM_COUNTERMODE_UP;         /* ���ϼ����� */
    tim10_handler.Init.Period = arr;                             /* �Զ�װ��ֵ */
    tim10_handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;   /* ʱ�ӷ�Ƶ���� */
    HAL_TIM_Base_Init(&tim10_handler);
	
    HAL_TIM_Base_Start_IT(&tim10_handler);                       /* ʹ��ͨ�ö�ʱ��x�ͼ�������жϣ�TIM_IT_UPDATE */
    
}


/**
 * @brief       ����Ԥװ������ֵ
 * @param       ��
 * @retval      ��
 */
void btim_tim10_arrset(uint16_t period)
{
   __HAL_TIM_SET_COUNTER(&tim10_handler,0);
    __HAL_TIM_SET_AUTORELOAD(&tim10_handler,period);    
}

/**
 * @brief       ���ü���ֵ
 * @param       ��
 * @retval      ��
 */
void btim_tim10_counterset(uint16_t count)
{
    __HAL_TIM_SET_COUNTER(&tim10_handler,count); 
}

/**
 * @brief       ʹ�ܻ�����ʱ��
 * @param       ��
 * @retval      ��
 */
void btim_tim10_enable(uint8_t enable)
{
    static TIM_HandleTypeDef *p = &tim10_handler;
    
    if(enable) p->Instance->CR1|=(TIM_CR1_CEN);
    else       p->Instance->CR1 &= ~(TIM_CR1_CEN);
}

/**
 * @brief       ������ʱ��TIM5�жϷ�����
 * @param       ��
 * @retval      ��
 */
void BTIM_TIM10_INT_IRQHandler(void)
{		
    HAL_TIM_IRQHandler(&tim10_handler); /* ��ʱ���ص����� */
}
/*******************************************************************************/

//TIM4 PWM���ֳ�ʼ�� 
//PWM�����ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��
void TIM4_PWM_Init(u32 arr,u32 psc)
{		 					 
	//�˲������ֶ��޸�IO������
	RCC->APB1ENR|=1<<2;		//TIM4ʱ��ʹ��    
	RCC->AHB1ENR|=1<<3;   	//ʹ��PORTDʱ��	
	GPIO_Set(GPIOD,PIN14|PIN15,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);//���ù���,�������
	GPIO_AF_Set(GPIOD,14,2);	//PD14,AF2 
	GPIO_AF_Set(GPIOD,15,2);	//PD15,AF2
	
	TIM4->ARR=arr;			//�趨�������Զ���װֵ 
	TIM4->PSC=psc;			//Ԥ��Ƶ������Ƶ 
	
	TIM4->CCMR2|=6<<4;  	//CH3 PWM1ģʽ		 
	TIM4->CCMR2|=1<<3; 		//CH3 Ԥװ��ʹ��	   
	TIM4->CCER|=1<<9;   	//OC3 ���ʹ��	
	TIM4->CCER|=1<<8;   	//OC3 �͵�ƽ��Ч	   
	
	TIM4->CCMR2|=6<<12;  	//CH4 PWM1ģʽ		 
	TIM4->CCMR2|=1<<11; 	//CH4 Ԥװ��ʹ��	   
	TIM4->CCER|=1<<12;   	//OC4 ���ʹ��	
	TIM4->CCER|=1<<13;   	//OC4 �͵�ƽ��Ч
	
	TIM4->CR1|=1<<7;   		//ARPEʹ�� 
	TIM4->CR1|=1<<0;    	//ʹ�ܶ�ʱ��3											  
}  

