/**
  ****************************************************************************************************
  * @file       mw1268_cfg.h
  * @author     正点原子团队(ALIENTEK)
  * @version    V1.0
  * @date       2022-2-15
  * @brief      mw1268模块参数定义
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

#ifndef _MW1268_CFG_H
#define _MW1268_CFG_H

#include "sys.h"


/*设备参数定义*/
typedef struct
{
   uint16_t addr;    /*设备地址*/
   uint8_t chn;      /*信道*/
   uint8_t netid;    /*网络地址*/
   uint8_t power;    /*发射功率*/
   uint8_t wlrate;   /*空中速率*/
   uint8_t wltime;   /*休眠时间*/
   uint8_t wmode;    /*工作模式*/
   uint8_t tmode;    /*发送模式*/
   uint8_t packsize; /*数据包大小*/
   uint8_t bps;      /*串口波特率*/
   uint8_t parity;   /*校验位*/
   uint8_t lbt;      /*信道检测*/
    
}_LoRa_CFG;

/*空中速率(单位:Kbps)*/
typedef enum
{
    LORA_RATE_0K3=0,   /*0.3kbps*/
    LORA_RATE_1K2,     /*1.2kbps*/
    LORA_RATE_2K4,     /*2.4kbps*/
    LORA_RATE_4K8,     /*4.8kbps*/  
    LORA_RATE_9K6,     /*9.6kbps*/ 
    LORA_RATE_19K2,    /*19.2kbps*/ 
    LORA_RATE_38K4,    /*38.4kbps*/ 
    LORA_RATE_62K5     /*62.5kbps*/
    
}_LORA_RATE;


/*休眠时间(单位:秒)*/
typedef enum
{
    LORA_WLTIME_1S=0, /*1秒*/
    LORA_WLTIME_2S    /*2秒*/ 
       
}_LORA_WLTIME;


/*工作模式*/
typedef enum
{
    LORA_WMODE_GEN=0,      /*一般模式*/
    LORA_WMODE_WAKE,       /*唤醒模式*/
    LORA_WMODE_SAVEPOWER,  /*省电模式*/
    LORA_WMODE_RSSI,       /*信号强度模式*/
    LORA_WMODE_DEEPSLEEP,  /*睡眠模式*/
    LORA_WMODE_RELAY       /*中继模式*/
    
}_LORA_WMODE;


/*发射功率*/
typedef enum
{
    LORA_TPW_9DBM=0,   /*9dBm*/
    LORA_TPW_11DBM,    /*11dBm*/
    LORA_TPW_14DBM,    /*14dBm*/
    LORA_TPW_17DBM,    /*17dBm*/
    LORA_TPW_20DBM,    /*20dBm*/
    LORA_TPW_22DBM     /*22dBm*/
    
}_LORA_TPOWER;


/*发送模式*/
typedef enum
{
    LORA_TMODE_PT=0, /*透明传输*/
    LORA_TMODE_FP,   /*定向传输*/   
    
}_LORA_TMODE;


/*串口波特率(单位:bps)*/
typedef enum
{
    LORA_BRD_1200=0, /*1200bps*/
    LORA_BRD_2400,   /*2400bps*/
    LORA_BRD_4800,   /*4800bps*/
    LORA_BRD_9600,   /*9600bps*/
    LORA_BRD_19200,  /*19200bps*/
    LORA_BRD_38400,  /*38400bps*/
    LORA_BRD_57600,  /*57600bps*/ 
    LORA_BRD_115200  /*115200bps*/ 
    
}_LORA_BRDRATE;


/*串口数据校验*/
typedef enum
{
    LORA_BRDVER_8N1=0, /*无校验*/
    LORA_BRDVER_8E1,   /*偶校验*/
    LORA_BRDVER_8O1    /*奇校验*/
    
}_LORA_BRDVERIFT;

/*信道检测*/
typedef enum
{
    LORA_LBT_DISABLE=0, /*关闭*/
    LORA_LBT_ENABLE     /*使能*/
    
}_LORA_LBT;


/*数据包大小*/
typedef enum
{
    LORA_PACKSIZE_32=0,  /*32字节*/
    LORA_PACKSIZE_64,    /*64字节*/
    LORA_PACKSIZE_128,   /*128字节*/
    LORA_PACKSIZE_240    /*240字节*/
    
}_LORA_PACKSIZE;


#define  RF_CHN_MAX    83   /*信道最大值*/
#define  RF_NETID_MAX  255  /*网络ID最大值*/


/*设备出厂默认参数*/
#define LORA_ADDR      0                  /*设备地址*/
#define LORA_CHN       23                 /*通信信道*/
#define LORA_NETID     0                  /*网络地址*/
#define LORA_TPOWER    LORA_TPW_20DBM     /*发射功率*/
#define LORA_RATE      LORA_RATE_19K2     /*空中速率*/
#define LORA_WLTIME    LORA_WLTIME_1S     /*休眠时间*/
#define LORA_WMODE     LORA_WMODE_GEN     /*工作模式*/
#define LORA_TMODE     LORA_TMODE_PT      /*发送模式*/
#define LORA_PACKSIZE  LORA_PACKSIZE_240  /*数据包大小*/
#define LORA_TTLBPS    LORA_BRD_115200    /*波特率*/
#define LORA_TTLPAR    LORA_BRDVER_8N1    /*校验位*/  
#define LORA_LBT       LORA_LBT_DISABLE   /*信道检测*/


/*设备工作状态*/
typedef enum
{
    LORA_CONFG_STA=0, /*配置状态*/
    LORA_RX_STA,      /*接收状态*/
    LORA_TX_STA       /*发送状态*/
    
}_LORA_DEVICE_STA;

extern _LORA_DEVICE_STA  Lora_Device_Sta;


/*AUX中断状态*/
typedef enum
{
    LORA_INT_OFF=0, /*关闭*/        
    LORA_INT_REDGE, /*上升沿*/
    LORA_INT_FEDGE  /*下降沿*/  
    
}_LORA_INT_STA;


#endif


