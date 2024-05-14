#include "sys.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mw1268_app.h"
#include "tendermint.h"
#include "uECC.h"
#include "rng.h"

#define USE_STM32F7XX_NUCLEO_144

static void MPU_Config(void);
static void CPU_CACHE_Enable(void);
static void SystemClock_Config(void);
void init_public_key();
void setup();
void run_slow();
void spin();
void stop();
/************************************************
 ALIENTEK 阿波罗STM32F767开发板 FreeRTOS实验2-1
 FreeRTOS移植实验-HAL库版本
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define CSMACA_PBFT_TASK_PRIO		2
//任务堆栈大小	
#define CSMACA_PBFT_STK_SIZE 		512  
//任务句柄
TaskHandle_t CSMACA_PBFT_Task_Handler;
//任务函数
void CSMACA_PBFT(void *pvParameters);


//crypto
char public_key_string[16][192] = {
	"9f a5 ef bd 99 5a bb cd d1 f8 e0 8f 5d ae bf 24 0c d8 91 46 5d 89 91 55 9c 36 92 2e b4 23 3d fd a1 88 60 be a6 4b c6 41 7c 4c bc ba e0 8d 53 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"3d b7 07 05 3b b6 95 5b 9b 40 16 4f 90 0f c2 bb b1 ea 57 19 b7 c8 25 ca 4f 82 6f 6e 78 54 71 14 91 85 16 8e 09 30 aa 41 4c 79 93 b6 f6 91 78 7a 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"66 28 e7 aa 37 d1 2a 94 28 c0 03 0a b6 89 48 17 ba e3 bd 45 b2 a1 9e 6f f8 fc 9b e1 06 a3 32 c5 63 b3 5a 35 16 30 bb 8c e0 d3 5c 27 8f 28 d0 b0 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"2e f0 f1 33 58 2b 22 0c 80 8f ce 64 fe 23 a1 78 1c 40 4d 3a ef a3 de fd df 04 2a 6d f0 fa bc 15 a1 28 6c 36 4e c5 bf 8a 98 26 f9 68 3a e7 03 dc 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"97 ab 36 69 c6 7a 83 e3 d9 49 a1 13 7d d8 07 50 bd 44 bd 13 61 10 0b 09 55 e8 6d 9e 74 c4 71 f8 77 78 42 01 ba ea 22 c7 2a 74 c1 b1 44 c0 10 ea 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"dc 6c b0 7c a4 97 4c 5a 0c 4f ff 54 c9 0e 64 a9 5d e4 ac dd a4 8a 8c bf 75 57 ff d1 7a 68 9a 2d 0b a2 28 0d 56 be aa 2c b4 2c b8 4d 64 f2 04 9d 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"14 0b be 2a c5 86 79 58 df 2c ac 71 7c 3e 9f 55 c4 0a cd 42 00 0d d6 a2 ea 3b ca 53 39 e8 1e 5e 85 38 be 0d 00 39 45 67 a8 cc 32 48 a9 01 7d 55 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"8f c6 c6 f2 62 2d 89 a0 03 b3 14 50 1f 00 b5 12 a4 77 9f 79 35 a5 f4 c4 13 13 d3 90 c9 7d 39 17 e9 35 ba 4b a7 67 c6 c0 01 c3 1a 12 82 7f 24 40 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"0e c6 1e 74 1f 82 d8 e6 d8 52 87 14 f7 de 5a 28 3d a7 d0 e5 33 d8 b4 b2 45 80 2e 1e 31 c0 c7 86 6b 6d 6d fd 4a 46 d4 15 37 e1 13 4f b7 39 0a 86 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"09 1e 0e 9b ad cc cd c7 5c 50 83 14 c3 29 51 43 2b 07 85 4c 0e 33 df 32 91 8e ad 10 bd 0a bb 99 3e 77 60 c4 ea 77 09 d4 db cf d1 6c c3 55 ca 38 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"75 b0 ce ca 13 74 26 47 4d 84 0e bd c4 f7 e4 03 08 d3 55 d4 0b 1b 1a 8a d3 89 a6 0a f4 86 6f 25 55 85 16 5b be 86 4d 34 45 15 8f 1d 76 e9 80 4b 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"77 d2 ea ca e0 39 1e 2b 97 4c dd 79 2c 6c 82 67 6f a5 60 ca 67 16 62 32 b0 5e eb 93 74 2a 51 cd 36 e7 ed 35 e7 29 31 5a 2d bb 96 f3 75 af be bf 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"df 67 30 1c 55 20 97 62 39 76 7b 37 2c 1e 08 0a df 68 f7 1d e9 61 07 10 cc c3 19 c0 ef 1d 68 1c 31 6c 4b b2 e9 f9 21 3d 9d f7 f5 a0 37 eb ca b8 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"c9 50 bd ca 99 47 35 a4 83 9e 05 68 d7 08 d9 18 48 7c 0b 8f 5b a6 9a a2 f0 d0 ab f7 ca c5 ac e0 8e 3d d0 59 8e 80 2d 4a ab c5 e8 0e 94 51 44 ca 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"72 27 74 b1 1b 0e 33 fc 38 86 7a f1 11 d9 0e 1a cc 8e bf 93 4b 99 df cb 17 76 be bf 76 ee 3c d0 2e a2 80 c3 78 3f d8 a3 cc fd 0e f6 7e 0f bc 92 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
	"c7 7e a9 d7 4f 3d a4 06 39 65 0b c9 7f 80 35 49 d9 a9 33 27 e1 bb 36 0b 85 c0 04 98 37 db 58 f1 91 56 c3 72 f6 fa 2b 60 9c 6c c9 3a 76 d2 de 85 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
//	"4f 86 5e ff 66 45 39 4a 81 a7 ee 69 ca b3 74 59 49 1d bd 66 1f ec 1c 8d a5 be 75 a9 5a 19 cc 6a 25 30 72 ff 68 10 f8 e6 b8 43 fa d6 19 b6 19 f9 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
//	"a7 e5 78 a6 1a 75 34 c8 0d c9 f9 53 e8 7c 4c dc b2 75 cc cb 4a f9 4a b1 3f 59 29 a7 7e 98 7c 19 7b 29 1c c2 86 06 51 db 26 ab 82 28 6e 6b cf a8 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00",
//	"11 ae a2 cc dc 3a 82 07 ff 9a 38 7c 5e 53 af 99 82 31 0c 86 ca 7e 14 c3 bd 70 70 3f 76 41 6d ce 38 28 c0 d6 fa e2 14 47 49 ed 87 76 a9 3e 83 d7 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"
};
char private_key_string[16][96] = {
	"8c 1b c1 e1 34 66 30 8e 0f 07 8a b5 57 0a 10 02 55 80 98 73 a3 8c 52 dd 00 00 00 00 00 00 00 00",
	"c8 43 21 2e 41 b3 01 bc 6b 83 3d 3b a1 38 15 2f b7 d2 d6 71 8e 36 f8 13 00 00 00 00 00 00 00 00",
	"76 71 30 f1 b3 55 b2 58 52 1c fd 17 bf e7 86 58 8a 2c 1c d5 f1 dd 42 19 00 00 00 00 00 00 00 00",
	"2f c4 44 27 8e 26 b5 21 6a ac 2d fd 5b 2e 7a 24 31 b6 76 da 27 0a c6 51 00 00 00 00 00 00 00 00",
	"1e 9e 36 3d 8d d4 cd ec 00 05 2b c0 aa 18 a6 8e ba 4a 0d 62 e4 aa ad 4d 00 00 00 00 00 00 00 00",
	"97 a1 75 3a d9 ed 3d 35 20 40 bd 35 41 28 34 61 d5 92 a7 80 bc 3a dc 2f 00 00 00 00 00 00 00 00",
	"4a 27 34 f7 47 1d e5 e4 d4 72 61 6d 18 ac 43 27 9f ba bb dc 7e 4b c3 88 00 00 00 00 00 00 00 00",
	"ba 91 b4 7e e1 da 98 37 6a 03 07 7e 81 21 95 97 e5 50 c2 12 6a 7f 30 c9 00 00 00 00 00 00 00 00",
	"b0 9f 0e 0c 56 ae 26 75 7f a4 3b aa 26 56 ab 10 34 7d f5 2c 4c 2a 73 c9 00 00 00 00 00 00 00 00",
	"29 44 3d 84 03 b3 f4 6c 41 e5 5e c6 95 02 ab b3 c1 3e c3 f0 14 2b 81 1c 00 00 00 00 00 00 00 00",
	"3a e1 52 fe c5 d6 6f cd 94 73 7c be 8f fb c5 b8 97 94 8d 12 fa 28 86 0e 00 00 00 00 00 00 00 00",
	"cb 6b 28 61 8b 40 ef 62 1d 3a 9e e6 e4 b6 b6 68 4e 5f bb db da 79 a4 37 00 00 00 00 00 00 00 00",
	"23 40 a2 ca 4f 10 7a c4 81 16 b1 4a 08 5e 73 79 49 5c 31 04 4c a0 d6 1d 00 00 00 00 00 00 00 00",
	"74 6f 42 64 4d e3 25 0e ce 22 ce 87 54 80 15 10 b7 2e 9b 4a 4e 83 b1 22 00 00 00 00 00 00 00 00",
	"3b e5 c3 2a 0f c3 38 84 7f 69 73 77 1d a2 e4 a4 96 37 47 bd f7 13 16 4e 00 00 00 00 00 00 00 00",
	"b4 58 b3 48 48 4f 4f 84 ad 02 23 5f 54 65 02 2b b9 15 59 81 2b 3a b4 db 00 00 00 00 00 00 00 00",
//	"69 13 75 6e 2a 56 68 1c 74 cd e5 74 eb 81 9e c9 fc a5 2f e2 43 90 bc 3f 00 00 00 00 00 00 00 00",
//	"cc 34 c6 f3 db 21 77 ef 77 47 d6 36 5e e7 07 2d f8 45 0f 37 e9 bf 84 58 00 00 00 00 00 00 00 00",
//	"23 b1 57 56 a1 f0 cb a7 75 50 e3 20 c1 7b 88 d8 bd b1 fb d1 8d 10 e8 bb 00 00 00 00 00 00 00 00"
};

uint8_t private_key[16+1][32] = {0};
uint8_t public_key[16+1][64] = {0};

u8 Sixteen2Ten(char ch){
	if( (ch >= '0') && ( ch <= '9' ) )	return ch - '0';
	return 10 + (ch - 'a');
}

static int my_RNG(uint8_t *dest, unsigned size){
	u32 random;
	for(int i=0;i<size;i++){
		u8 Random_idx = i % 4;
		if( Random_idx % 4 == 0) random = RNG_Get_RandomNum();
		dest[i] = (random >> (Random_idx * 8) ) & 0xFF;
	}
	return 1;
}

int main(void)
{
    /* Configure the MPU attributes */
	MPU_Config();
	CPU_CACHE_Enable();                 //打开L1-Cache
    HAL_Init();				        //初始化HAL库
	SystemClock_Config();

    delay_init(216);                //延时初始化
    LED_Init();                     //初始化LED

	
    //创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
    //创建LED0任务
    xTaskCreate((TaskFunction_t )CSMACA_PBFT,     	
                (const char*    )"csmaca_pbft",   	
                (uint16_t       )CSMACA_PBFT_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )CSMACA_PBFT_TASK_PRIO,	
                (TaskHandle_t*  )&CSMACA_PBFT_Task_Handler);   
//    //创建LED1任务
//    xTaskCreate((TaskFunction_t )led1_task,     
//                (const char*    )"led1_task",   
//                (uint16_t       )LED1_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )LED1_TASK_PRIO,
//                (TaskHandle_t*  )&LED1Task_Handler);        
	
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}

//LED0任务函数 
void CSMACA_PBFT(void *pvParameters)
{
	btim_tim5_int_init(9000-1,10800-1);//108Mhz/10800 = 10000hz  900ms
    btim_tim2_int_init(9000-1,10800-1);
	btim_tim9_int_init(5000-1,21600-1);//216Mhz/21600 = 10000hz  900ms
//	btim_tim10_int_init(5000-1,21600-1);
//	TIM10->CR1 &= 0xFE;
//	setup();
//	TIM4_PWM_Init(600-1,10800-1);
//	stop();
	//consensus main
	tendermint();
	
	while(1);
}   

void setup(){
	//左轮
	RCC->AHB1ENR|=1<<5;	//使能PORTF时钟 
	GPIO_Set(GPIOF,PIN12,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PF12设置
	RCC->AHB1ENR|=1<<3;	//使能PORTD时钟 
	GPIO_Set(GPIOD,PIN15,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PD15设置
	//右轮
	//	RCC->AHB1ENR|=1<<3;	//使能PORTD时钟 
	GPIO_Set(GPIOD,PIN14,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PD14设置
	RCC->AHB1ENR|=1<<0;	//使能PORTA时钟 
	GPIO_Set(GPIOA,PIN7,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PA7设置

	//寻迹
	//right
//	RCC->AHB1ENR|=1<<5;	//使能PORTF时钟 
	GPIO_Set(GPIOF,PIN14,GPIO_MODE_IN,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PF14设置
	//left
	RCC->AHB1ENR|=1<<4;	//使能PORTE时钟 
	GPIO_Set(GPIOE,PIN13,GPIO_MODE_IN,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PE13设置
}

void run_slow(){
//	//左轮前进                  // PF12 0 前进 1后退
								//0 数越小约快   1数约大约快
	GPIO_Pin_Set(GPIOF,PIN12,0);
	TIM4->CCR4 =200;	//PD15    TIM4-CH4
//	GPIO_Pin_Set(GPIOD,PIN15,1);
//	//右轮前进				// PA12 0 前进 1后退
							//0 数越小约快   1数约大约快
//	GPIO_Pin_Set(GPIOD,PIN14,1);
	GPIO_Pin_Set(GPIOA,PIN7,0);
	TIM4->CCR3 = 350;//PD14    TIM4-CH3
}

void spin(){
//	//左轮前进                  // PF12 0 前进 1后退
								//0 数越小约快   1数约大约快
	GPIO_Pin_Set(GPIOF,PIN12,0);
	TIM4->CCR4 =600;	//PD15    TIM4-CH4
//	GPIO_Pin_Set(GPIOD,PIN15,1);
//	//右轮前进				// PA12 0 前进 1后退
							//0 数越小约快   1数约大约快
//	GPIO_Pin_Set(GPIOD,PIN14,1);
	GPIO_Pin_Set(GPIOA,PIN7,0);
	TIM4->CCR3 = 0;//PD14    TIM4-CH3
}

void stop(){
//	//左轮前进                  // PF12 0 前进 1后退
								//0 数越小约快   1数约大约快
	GPIO_Pin_Set(GPIOF,PIN12,0);
	TIM4->CCR4 =600;	//PD15    TIM4-CH4
//	GPIO_Pin_Set(GPIOD,PIN15,1);
//	//右轮前进				// PA12 0 前进 1后退
							//0 数越小约快   1数约大约快
//	GPIO_Pin_Set(GPIOD,PIN14,1);
	GPIO_Pin_Set(GPIOA,PIN7,0);
	TIM4->CCR3 = 600;//PD14    TIM4-CH3

}
void init_public_key(){
	while(RNG_Init())	 		    //初始化随机数发生器
	{ 
		delay_ms(200); 
	} 
	u8 string_idx=0;
	u8 key_idx = 0;
	for(u8 l=1;l<=16;l++){
		key_idx = 0;
		for(string_idx = 0; string_idx < sizeof(public_key_string[l-1]);){
			public_key[l][key_idx] = Sixteen2Ten(public_key_string[l-1][string_idx])*16+Sixteen2Ten(public_key_string[l-1][string_idx + 1]);
			key_idx = key_idx + 1;
			string_idx = string_idx + 3;
		}

		key_idx = 0;
		for(string_idx = 0; string_idx < sizeof(private_key_string[l-1]);){
			private_key[l][key_idx] = Sixteen2Ten(private_key_string[l-1][string_idx])*16 + Sixteen2Ten(private_key_string[l-1][string_idx + 1]);
			key_idx = key_idx + 1;
			string_idx = string_idx + 3;
		}
		
	}
	

    int num_curves = 0;
//#if uECC_SUPPORTS_secp160r1
//    curves[num_curves++] = uECC_secp160r1();
//#endif
#if uECC_SUPPORTS_secp192r1
    curves[num_curves++] = uECC_secp192r1();
#endif
//#if uECC_SUPPORTS_secp224r1
//    curves[num_curves++] = uECC_secp224r1();
//#endif
//#if uECC_SUPPORTS_secp256r1
//    curves[num_curves++] = uECC_secp256r1();
//#endif
//#if uECC_SUPPORTS_secp256k1
//    curves[num_curves++] = uECC_secp256k1();
//#endif
    uECC_set_rng(my_RNG);
}




/////////////////System Configure

static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    while(1) {};
  }

  /* Activate the OverDrive to reach the 216 Mhz Frequency */
  if(HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    while(1) {};
  }


  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    while(1) {};
  }
}

/**
  * @brief  Configure the MPU attributes
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU as Strongly ordered for not defined regions */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x00;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
SCB->CACR|=1<<2;   												/* 强制D-Cache透写,如不开启,实际使用中可能遇到各种问题 */

}

