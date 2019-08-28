#ifndef _LIDAR_H
#define _LIDAR_H

#include <stdint.h>

/********************************************************************************************************************/
//ͨѶ��ز�������
/********************************************************************************************************************/
#define FRAME_PARAM_MAX_RX_LEN		            (2000 + 10)	                                	//�������ݲ�������󳤶�
#define FRAME_PARAM_MAX_TX_LEN		            100		                                			//�������ݲ�������󳤶�

#define FRAME_HEAD		                        	0xAA                                        //֡ͷ
#define FRAME_PROTOCAL_VERSION	  	            0x00                                        //Э��汾                                     
#define FRAME_TYPE						               		0x61                                        //֡����

#define FRAME_MEASURE_INFO											0xAD
#define FRAME_DEVICE_HEALTH_INFO								0xAE

//�״�ɨ��״̬
enum SCANSTATE
{
	GRAB_SCAN_FIRST = 0,
	GRAB_SCAN_ELSE_DATA
};	
enum SCANRESULT
{
	LIDAR_GRAB_ING = 0,
	LIDAR_GRAB_SUCESS,
	LIDAR_GRAB_ERRO,
	LIDAR_GRAB_ELSE
};

//�����״���ָ���
#define  TOOTH_NUM																	16

/********************************************************************************************************************/
//ͨѶ֡����
/********************************************************************************************************************/
#pragma pack (1)
typedef struct
{
	uint8_t Header;
	uint16_t Len;
	uint8_t Addr;
	uint8_t CmdType;
	uint8_t CmdId;
	uint16_t ParamLen;
	uint8_t *Data;
}T_PROTOCOL;

//����Ϣ֡����
typedef struct 
{
	float Angle;
	float Distance;
}T_POINT;
//����֡����
typedef struct 
{
	float RotateSpeed;
	float ZeroOffset;
	float FrameStartAngle;
	uint8_t PointNum;//һ֡�ĵ���
	T_POINT Point[100];//һ֡ ����Ϣ
}T_FRAME_MEAS_INFO;

//�豸������Ϣ֡
typedef struct 
{
	uint8_t ErrCode;
}T_DEVICE_HEALTH_INFO;

//�״�ɨ����Ϣ����
typedef struct
{
	uint8_t State;
	uint8_t Result;
	float LastScanAngle;
	uint8_t ToothCount;
	uint16_t OneCriclePointNum;//һȦ����
	T_FRAME_MEAS_INFO FrameMeasInfo;//һ֡������Ϣ
	T_POINT OneCriclePoint[1000];//һȦ��������Ϣ������㿪ʼ����16֡����Ϣ���
}T_LIDARSCANINFO;
#pragma pack ()

extern T_LIDARSCANINFO lidarscaninfo;

void Lidarscaninfo_Init(void);
uint8_t P_Cmd_Process(void);


#endif
