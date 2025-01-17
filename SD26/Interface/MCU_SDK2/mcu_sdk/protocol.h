/****************************************Copyright (c)*************************
**                               版权所有 (C), 2018-0704, 涂鸦科技
**
**                                 http://www.tuya.com
**
**--------------文件信息-------------------------------------------------------
**文   件   名: protocol.h
**描        述: 下发/上报数据处理函数
**使 用 说 明 :

                  *******非常重要，一定要看哦！！！********

** 1、用户在此文件中实现数据下发/上报功能
** 2、DP的ID/TYPE及数据处理函数都需要用户按照实际定义实现
** 3、当开始某些宏定义后需要用户实现代码的函数内部有#err提示,完成函数后请删除该#err
**
**--------------当前版本修订---------------------------------------------------
** 版  本: v1.0.0
** 日　期: 2018年7月4日
** 描　述: 1:协议初版
**
**-----------------------------------------------------------------------------
******************************************************************************/
#ifndef __PROTOCOL_H_
#define __PROTOCOL_H_


#ifndef bool
#define bool unsigned char
#endif
/******************************************************************************
                            用户相关信息配置
******************************************************************************/
/******************************************************************************
                            1:修改产品信息                
******************************************************************************/
#define PRODUCT_KEY "o9kfm6ks"    //开发平台创建产品后生成的16位字符产品唯一标识


#define MCU_VER "1.0.0"           //用户的软件版本,用于MCU固件升级,MCU升级版本需修改

/******************************************************************************
                         2:定义收发缓存:
                    如当前使用MCU的RAM不够,可修改
******************************************************************************/
#define ZIGBEE_UART_QUEUE_LMT             255//16         //数据接收队列大小,如MCU的RAM不够,可缩小
#define ZIGBEE_UART_RECV_BUF_LMT          128              //根据用户DP数据大小量定,必须大于24
#define ZIGBEE_UART_SEND_BUF_LMT          128              //根据用户DP数据大小量定,必须大于24

//============================================================================= 
typedef struct
{
    unsigned int  year;     /*年， 取值范围为实际年份减去1970*/
    unsigned char month;    /*月， 取值范围为0-11*/
    unsigned char day;      /*日， 取值范围为1-31*/
    unsigned char hour;     /*时， 取值范围为0-23*/
    unsigned char minute;   /*分， 取值范围为0-59*/
    unsigned int second;   /*秒， 取值范围为0-59*/
}time_t;

/******************************************************************************
                      3:MCU是否需要支持zigbee功能测试                     
如需要请开启该宏,并且mcu在需要zigbee功能测试处调用mcu_api.c文件内mcu_start_zigbee_test
并在protocol.c文件zigbee_test_result函数内查看测试结果,
zigbee_test_result内部有#err提示,完成函数后请删除该#err
******************************************************************************/
#define         ZIGBEE_TEST_ENABLE                //开启zigbee模块产测功能

/******************************************************************************
                          4:MCU是否需要支固件升级                  
如需要支持MCU固件升级,请开启该宏

******************************************************************************/
//#define         SUPPORT_MCU_FIRM_UPDATE                 //开启MCU固件升级功能(默认关闭)
#ifdef SUPPORT_MCU_FIRM_UPDATE
  #define NOW_MCU_VER 0x41 //01.00.0000  ->1.0.0
  #define UPDATE_MCU_VER 0x42
#endif
/******************************************************************************
                        1:dp数据点序列号重新定义
          **此为自动生成代码,如在开发平台有相关修改请重新下载MCU_SDK**         
******************************************************************************/
//开关1(可下发可上报)
//备注:
#define DPID_SWITCH_LED_1 1
//亮度值1(可下发可上报)
//备注:
#define DPID_BRIGHT_VALUE_1 2
//最小亮度1(可下发可上报)
//备注:
#define DPID_BRIGHTNESS_MIN_1 3
//光源类型1(可下发可上报)
//备注:
#define DPID_LED_TYPE_1 4



/*****************************************************************************
函数名称 : all_data_update
功能描述 : 系统所有dp点信息上传
输入参数 : 无
返回参数 : 无
使用说明 : MCU必须实现该函数内数据上报功能
*****************************************************************************/
void all_data_update(void);

/*****************************************************************************
函数名称 : mcu_write_rtctime
功能描述 : MCU校对本地RTC时钟
输入参数 : local_or_utc: true是local time， false是UTCtime
返回参数 : 无
使用说明 : MCU需要自行实现该功能
*****************************************************************************/
void mcu_write_rtctime(bool local_or_utc, time_t* time);

/*****************************************************************************
函数名称 : zigbee_test_result
功能描述 : zigbee RF功能测试反馈
输入参数 : result:zigbee功能测试结果;0:失败/1:成功
           rssi:测试成功表示zigbee信号强度/测试失败表示错误类型
返回参数 : 无
使用说明 : MCU需要自行实现该功能
*****************************************************************************/
void zigbee_test_result(bool result,unsigned char rssi);

/*****************************************************************************
函数名称 : dp_download_handle
功能描述 : dp下发处理函数
输入参数 : dpid:DP序号
value:dp数据缓冲区地址
length:dp数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERRO
使用说明 : 该函数用户不能修改
*****************************************************************************/
unsigned char dp_download_handle(unsigned char dpid,const unsigned char value[], unsigned short length);

/*****************************************************************************
函数名称 : get_download_cmd_total
功能描述 : 获取所有dp命令总和
输入参数 : 无
返回参数 : 下发命令总和
使用说明 : 该函数用户不能修改
*****************************************************************************/
unsigned char get_download_cmd_total(void);

#endif

