/**
  ****************************************************************************************************
  * @file    	usart3.c
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


#include "sys.h"
#include "usart3.h"
#include "btim.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

#include "mw1268_cfg.h"


/*串口收发缓冲区*/
uint8_t uart3RxBuffer[UART3_BUFFER_SIZE];
uint8_t uart3TxBuffer[UART3_BUFFER_SIZE];

UART_HandleTypeDef uart3_handler; 		/*UART句柄*/


/*
    通过判断接收连续2个字符之间的时间差不大于10ms来决定是不是一次连续的数据.
    如果2个字符接收间隔超过timer,则认为不是1次连续数据.也就是超过timer没有接收到
    任何数据,则表示此次接收完毕.
    接收到的数据状态
    [15]:0,没有接收到数据;1,接收到了一批数据.
    [14:0]:接收到的数据长度
*/
uint16_t USART3_RX_STA=0;

/**
 * @brief       串口X引脚初始化
 * @param       无
 * @retval      无
 */
static void usart3_gpioinit(void)
{
    GPIO_InitTypeDef gpio_initure;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();

    gpio_initure.Pin = GPIO_PIN_10;
    gpio_initure.Mode = GPIO_MODE_AF_PP;
    gpio_initure.Pull = GPIO_PULLUP;
    gpio_initure.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio_initure.Alternate=GPIO_AF7_USART3;	//复用为USART3
    HAL_GPIO_Init(GPIOB, &gpio_initure);

    gpio_initure.Pin = GPIO_PIN_11;
    ;
    HAL_GPIO_Init(GPIOB, &gpio_initure);
}

/**
 * @brief       串口X初始化
 * @param       无
 * @retval      无
 */
void usart3_init(uint32_t bound)
{
    
    usart3_gpioinit();

    /* USART 初始化设置 */
    uart3_handler.Instance = USART3;
    uart3_handler.Init.BaudRate = bound;                                     /*波特率*/
    uart3_handler.Init.WordLength = UART_WORDLENGTH_8B;                      /*字长为8位数据格式*/
    uart3_handler.Init.StopBits = UART_STOPBITS_1;                           /*一个停止位*/
    uart3_handler.Init.Parity = UART_PARITY_NONE;                            /*无奇偶校验位*/
    uart3_handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;                      /*无硬件流控*/
    uart3_handler.Init.Mode = UART_MODE_TX_RX;                               /*收发模式*/
    HAL_UART_Init(&uart3_handler);                                           /*HAL_UART_Init()会使能UART2*/

    __HAL_UART_ENABLE_IT(&uart3_handler, UART_IT_RXNE);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
    HAL_NVIC_SetPriority(USART3_IRQn, 3, 3);
    
    btim_timx_int_init(99, 8399); //10ms中断
    USART3_RX_STA = 0; //清零
    btim_timx_enable(DISABLE); //关闭定时器

  
}

/**
 * @brief       串口X参数设置
 * @param       bps    波特率
 * @param       parity 校验位               
 * @retval      无
 */
void usart3_bpsset(uint8_t bps, uint8_t parity)
{
    static uint32_t bound = 0;

    switch (bps)
    {
        case LORA_BRD_1200: bound = 1200; break;
        case LORA_BRD_2400: bound = 2400; break;
        case LORA_BRD_4800: bound = 4800; break;
        case LORA_BRD_9600: bound = 9600; break;
        case LORA_BRD_19200: bound = 19200;break;
        case LORA_BRD_38400: bound = 38400; break;
        case LORA_BRD_57600: bound = 57600; break;
        case LORA_BRD_115200:bound = 115200;break;
    }
    
    __HAL_UART_DISABLE(&uart3_handler);
    uart3_handler.Init.BaudRate = bound;  
    uart3_handler.Init.StopBits = UART_STOPBITS_1;
    
    if(parity == LORA_BRDVER_8N1) //无校验
    {
        uart3_handler.Init.WordLength = UART_WORDLENGTH_8B;
        uart3_handler.Init.Parity = UART_PARITY_NONE;       
    }
    else if(parity == LORA_BRDVER_8E1) //偶校验
    {
        uart3_handler.Init.WordLength = UART_WORDLENGTH_9B;
        uart3_handler.Init.Parity = UART_PARITY_EVEN; 
    }
    else if(parity == LORA_BRDVER_8O1) //奇校验
    {
        uart3_handler.Init.WordLength = UART_WORDLENGTH_9B;
        uart3_handler.Init.Parity = UART_PARITY_ODD; 
    }
    
     HAL_UART_Init(&uart3_handler);
     
}  

/**
  * @brief  串口3发送
  * @note  确保一次发送数据不超过UART3_BUFFER_SIZE字节
  * @retval 无
  */
void usart3_sendData(char *fmt,...)
{
    uint16_t len=0;
    va_list ap;
    va_start(ap, fmt);
    vsprintf((char *)uart3TxBuffer, fmt, ap);
    va_end(ap);
    len = strlen((const char *)uart3TxBuffer); //此次发送数据的长度

    HAL_UART_Transmit(&uart3_handler, (uint8_t *)uart3TxBuffer, len, 500);
}


/**
 * @brief       串口X中断服务函数
 * @param       无
 * @retval      无
 */
void USART3_IRQHandler(void)
{
    HAL_UART_IRQHandler(&uart3_handler);

    if (__HAL_UART_GET_FLAG(&uart3_handler, UART_FLAG_RXNE) != RESET) /*!< 接收非空中断 */
    {
        uint8_t res = uart3_handler.Instance->RDR;
        
        if ((USART3_RX_STA & (1 << 15)) == 0)       //接收完的一批数据,还没有被处理,则不再接收其他数据
        {
            if (USART3_RX_STA < UART3_BUFFER_SIZE) //还可以接收数据
            {
                 if (!Lora_Device_Sta) //配置功能下(启动定时器超时)
                 {
                    btim_timx_counterset(0);            //计数器清空

                    if (USART3_RX_STA == 0)  //使能定时器7的中断
                    {
                        btim_timx_enable(ENABLE);          //使能定时器7
                    } 
                 }

                uart3RxBuffer[USART3_RX_STA++] = res; //记录接收到的值
            }
            else
            {
                USART3_RX_STA |= 1 << 15; //强制标记接收完成
            }
        }
        
        
        
    }
}




/*******************************END OF FILE************************************/

//串口接收使能控制
//enable:0.关闭 1,打开
void usart3_rx(u8 enable)
{
	 USART3->CR1 &= ~(1<<0);//关闭串口
	
	 if(enable)
	 {
		 USART3->CR1|=(1<<3)|(1<<2);//发送和接收
	 }else
	 { 
		 USART3->CR1&=~(1<<2);//关闭接收
		 USART3->CR1|=(1<<3);//打开发送
	 }
	 
	 USART3->CR1 |= (1<<0);//使能串口 
}

