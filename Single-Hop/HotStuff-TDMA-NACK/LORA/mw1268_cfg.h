/**
  ****************************************************************************************************
  * @file       mw1268_cfg.h
  * @author     ����ԭ���Ŷ�(ALIENTEK)
  * @version    V1.0
  * @date       2022-2-15
  * @brief      mw1268ģ���������
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

#ifndef _MW1268_CFG_H
#define _MW1268_CFG_H

#include "sys.h"


/*�豸��������*/
typedef struct
{
   uint16_t addr;    /*�豸��ַ*/
   uint8_t chn;      /*�ŵ�*/
   uint8_t netid;    /*�����ַ*/
   uint8_t power;    /*���书��*/
   uint8_t wlrate;   /*��������*/
   uint8_t wltime;   /*����ʱ��*/
   uint8_t wmode;    /*����ģʽ*/
   uint8_t tmode;    /*����ģʽ*/
   uint8_t packsize; /*���ݰ���С*/
   uint8_t bps;      /*���ڲ�����*/
   uint8_t parity;   /*У��λ*/
   uint8_t lbt;      /*�ŵ����*/
    
}_LoRa_CFG;

/*��������(��λ:Kbps)*/
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


/*����ʱ��(��λ:��)*/
typedef enum
{
    LORA_WLTIME_1S=0, /*1��*/
    LORA_WLTIME_2S    /*2��*/ 
       
}_LORA_WLTIME;


/*����ģʽ*/
typedef enum
{
    LORA_WMODE_GEN=0,      /*һ��ģʽ*/
    LORA_WMODE_WAKE,       /*����ģʽ*/
    LORA_WMODE_SAVEPOWER,  /*ʡ��ģʽ*/
    LORA_WMODE_RSSI,       /*�ź�ǿ��ģʽ*/
    LORA_WMODE_DEEPSLEEP,  /*˯��ģʽ*/
    LORA_WMODE_RELAY       /*�м�ģʽ*/
    
}_LORA_WMODE;


/*���书��*/
typedef enum
{
    LORA_TPW_9DBM=0,   /*9dBm*/
    LORA_TPW_11DBM,    /*11dBm*/
    LORA_TPW_14DBM,    /*14dBm*/
    LORA_TPW_17DBM,    /*17dBm*/
    LORA_TPW_20DBM,    /*20dBm*/
    LORA_TPW_22DBM     /*22dBm*/
    
}_LORA_TPOWER;


/*����ģʽ*/
typedef enum
{
    LORA_TMODE_PT=0, /*͸������*/
    LORA_TMODE_FP,   /*������*/   
    
}_LORA_TMODE;


/*���ڲ�����(��λ:bps)*/
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


/*��������У��*/
typedef enum
{
    LORA_BRDVER_8N1=0, /*��У��*/
    LORA_BRDVER_8E1,   /*żУ��*/
    LORA_BRDVER_8O1    /*��У��*/
    
}_LORA_BRDVERIFT;

/*�ŵ����*/
typedef enum
{
    LORA_LBT_DISABLE=0, /*�ر�*/
    LORA_LBT_ENABLE     /*ʹ��*/
    
}_LORA_LBT;


/*���ݰ���С*/
typedef enum
{
    LORA_PACKSIZE_32=0,  /*32�ֽ�*/
    LORA_PACKSIZE_64,    /*64�ֽ�*/
    LORA_PACKSIZE_128,   /*128�ֽ�*/
    LORA_PACKSIZE_240    /*240�ֽ�*/
    
}_LORA_PACKSIZE;


#define  RF_CHN_MAX    83   /*�ŵ����ֵ*/
#define  RF_NETID_MAX  255  /*����ID���ֵ*/


/*�豸����Ĭ�ϲ���*/
#define LORA_ADDR      0                  /*�豸��ַ*/
#define LORA_CHN       23                 /*ͨ���ŵ�*/
#define LORA_NETID     0                  /*�����ַ*/
#define LORA_TPOWER    LORA_TPW_20DBM     /*���书��*/
#define LORA_RATE      LORA_RATE_19K2     /*��������*/
#define LORA_WLTIME    LORA_WLTIME_1S     /*����ʱ��*/
#define LORA_WMODE     LORA_WMODE_GEN     /*����ģʽ*/
#define LORA_TMODE     LORA_TMODE_PT      /*����ģʽ*/
#define LORA_PACKSIZE  LORA_PACKSIZE_240  /*���ݰ���С*/
#define LORA_TTLBPS    LORA_BRD_115200    /*������*/
#define LORA_TTLPAR    LORA_BRDVER_8N1    /*У��λ*/  
#define LORA_LBT       LORA_LBT_ENABLE   /*�ŵ����*/


/*�豸����״̬*/
typedef enum
{
    LORA_CONFG_STA=0, /*����״̬*/
    LORA_RX_STA,      /*����״̬*/
    LORA_TX_STA       /*����״̬*/
    
}_LORA_DEVICE_STA;

extern _LORA_DEVICE_STA  Lora_Device_Sta;


/*AUX�ж�״̬*/
typedef enum
{
    LORA_INT_OFF=0, /*�ر�*/        
    LORA_INT_REDGE, /*������*/
    LORA_INT_FEDGE  /*�½���*/  
    
}_LORA_INT_STA;


#endif


