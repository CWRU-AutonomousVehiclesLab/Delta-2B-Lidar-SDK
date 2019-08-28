#include <stdlib.h>
#include "lidar.h"
#include "bsp_uart.h"
#include "check.h"
  
T_LIDARSCANINFO lidarscaninfo;
/********************************************************************************************/
//��ʼ���״�ɨ����Ϣ����
//����:     ��
//����ֵ:   ��
/*****************************************************************************************/
void Lidarscaninfo_Init(void)
{
	lidarscaninfo.State = GRAB_SCAN_FIRST;
	lidarscaninfo.ToothCount=0;
	lidarscaninfo.LastScanAngle=0.0;
	lidarscaninfo.Result = LIDAR_GRAB_ING;
	lidarscaninfo.OneCriclePointNum=0;
}

uint16_t Little2BigEndian_u16(uint16_t dat)
{
	return ((dat>>8)|(dat<<8));
}
/********************************************************************************************/
//2���ַ�ת����uint16_t����
//����:     ��
//����ֵ:   ��
/*****************************************************************************************/
uint16_t Strto_u16(uint8_t* str)
{
	return ((*str << 8) | *(str+1));
}
void LittleCopy_u16(uint8_t *dest,uint8_t *src,uint16_t len)
{
	while(len)
  {
		*dest = *(src+1);
		*(dest+1) = *src;
		dest+=2;src+=2;
		len--;
	}
}
/********************************************************************************************/
//һ�β�������Ϣ���뵽һȦ����Ϣ�����ݽṹ�� ����
//����:     ��
//����ֵ:   ��
/*****************************************************************************************/
void OneCriclePoint_insert(T_FRAME_MEAS_INFO one_meas_info)
{
	uint8_t i=0;
	for(i=0;i<one_meas_info.PointNum;i++)
	{
		lidarscaninfo.OneCriclePoint[lidarscaninfo.OneCriclePointNum].Angle=one_meas_info.Point[i].Angle;
		lidarscaninfo.OneCriclePoint[lidarscaninfo.OneCriclePointNum].Distance=one_meas_info.Point[i].Distance;
		lidarscaninfo.OneCriclePointNum++;
	}
}
/********************************************************************************************/
//�ж��ǲ���ɨ�赽��һ������ ����
//����:     ��
//����ֵ:   ��
/*****************************************************************************************/
uint8_t isFirstGratingScan(float angle)
{
	return angle<0.0001? 1:0;
}
/********************************************************************************************/
//�洢��һ�����ּ�ɨ��ĵ��� ����
//����:     ��
//����ֵ:   ��
/*****************************************************************************************/
void grabFirstGratingScan(T_FRAME_MEAS_INFO one_meas_info)
{
	lidarscaninfo.State = GRAB_SCAN_ELSE_DATA;
	lidarscaninfo.ToothCount=1;
	lidarscaninfo.LastScanAngle=one_meas_info.FrameStartAngle;
	OneCriclePoint_insert(one_meas_info);
}
/********************************************************************************************/
//ɨ��һȦ�Ĺ��� ����
//����:     ��
//����ֵ:   ��
/*****************************************************************************************/
void ScanOneCircle(T_FRAME_MEAS_INFO one_meas_info)
{
	switch(lidarscaninfo.State)
	{
		case GRAB_SCAN_FIRST:
				//first tooth scan come
				if(isFirstGratingScan(one_meas_info.FrameStartAngle))
				{
					Lidarscaninfo_Init();
					grabFirstGratingScan(one_meas_info);
				}
				else
				{
					printf("[Lidar] GRAB_SCAN_FIRST is not zero; scan angle %5.2f!\n",one_meas_info.FrameStartAngle);
				}
				break;
		
		case GRAB_SCAN_ELSE_DATA:
				/* Handle angle suddenly reduces */
				if(one_meas_info.FrameStartAngle < lidarscaninfo.LastScanAngle) 
				{    
						printf("[Lidar] Restart scan, FrameStartAngle: %5.2f, LastScanAngle: %5.2f!\n",one_meas_info.FrameStartAngle, lidarscaninfo.LastScanAngle); 
						//��νǶ�С����һ�νǶȣ��ҽǶ�Ϊ�㣺��ʾ��һȦ���ݲ��������պô����������һȦ(��Ȧ���ݶ���)
						if(isFirstGratingScan(one_meas_info.FrameStartAngle))
						{
							Lidarscaninfo_Init();
							grabFirstGratingScan(one_meas_info);
						}
						else //��νǶ�С����һ�νǶȣ��ҽǶȲ�Ϊ�㣺��ʾ��һȦ����Ȧ���ݶ������������½�������㿪ʼɨ�裬��һȦ����Ȧ���ݶ�����
						{
							lidarscaninfo.State = GRAB_SCAN_FIRST;
						}
						return;				
        }
				OneCriclePoint_insert(one_meas_info);
				lidarscaninfo.ToothCount++;
				lidarscaninfo.LastScanAngle = one_meas_info.FrameStartAngle;

				if(lidarscaninfo.ToothCount == TOOTH_NUM)//Scan One Circle
				{
					lidarscaninfo.ToothCount=0;
					lidarscaninfo.State = GRAB_SCAN_FIRST;
					lidarscaninfo.Result =  LIDAR_GRAB_SUCESS;
					return;
				}
				break;			
		default:
				printf("[Lidar] Uknow grab scan data state\n");
				break;
	}
}
/********************************************************************************************/
//������Ϣ֡��������
//����:     ��
//����ֵ:   ��
/*****************************************************************************************/
void AnalysisMeasureInfo(T_PROTOCOL* Preq)
{
	uint8_t data_head_offset=0;
	uint8_t i = 0;
	float per_angle_offset = 0;

	Preq->ParamLen = Little2BigEndian_u16(Preq->ParamLen);
	
	lidarscaninfo.FrameMeasInfo.RotateSpeed=(*(Preq->Data))*0.05f;
	data_head_offset +=1;
	lidarscaninfo.FrameMeasInfo.ZeroOffset=Strto_u16(Preq->Data+data_head_offset)*0.01f;
	data_head_offset +=2;   
	lidarscaninfo.FrameMeasInfo.FrameStartAngle=Strto_u16(Preq->Data+data_head_offset)*0.01f;
	data_head_offset +=2;
	lidarscaninfo.FrameMeasInfo.PointNum = (Preq->ParamLen-data_head_offset)/3;
	
	per_angle_offset=22.5f/lidarscaninfo.FrameMeasInfo.PointNum;
	for(i=0;i<lidarscaninfo.FrameMeasInfo.PointNum;i++)
	{
		lidarscaninfo.FrameMeasInfo.Point[i].Distance=Strto_u16(Preq->Data+data_head_offset+3*i+1)*0.25f;
		lidarscaninfo.FrameMeasInfo.Point[i].Angle=	lidarscaninfo.FrameMeasInfo.FrameStartAngle+i*per_angle_offset;
	}
	
	ScanOneCircle(lidarscaninfo.FrameMeasInfo);
}
/********************************************************************************************/
//�豸������Ϣ֡����֡
//����:     ��
//����ֵ:   ��
/*****************************************************************************************/
void AnalysisDeviceHealthInfo(T_PROTOCOL* Preq)
{
	float speed = 0;
	speed = *(Preq->Data)*0.05f;
	printf("[Lidar] Speed error!\n");
}
/********************************************************************************************/
//�ж�ͨѶ֡�Ƿ�����֡����
//����:     ��
//����ֵ:   ��
/*****************************************************************************************/
uint8_t FrameIsRight(T_PROTOCOL Preq)
{
	uint16_t Temp = 0,CalcCRC,Len;
	Len = Little2BigEndian_u16(Preq.Len);
	if(Preq.Header != 0xAA) 
	{	
		return 0;
	}
	else if(Len > RxBuffer.Len)
	{
		return 0;
	}
	else if(Preq.CmdType != FRAME_TYPE)
	{
		return 0;
	} 
	else 
	{	
		//1.crcУ��
		//CalcCRC = CRC16(RxBuffer.Buff,Len);
		//2.�ۼ�У���
		CalcCRC	= Calc_CheckSum(RxBuffer.Buff,Len);
		
		LittleCopy_u16((uint8_t *)&Temp,&RxBuffer.Buff[Len],1);
		if(CalcCRC != Temp)
		{
			printf("[Lidar] Frame is CRC error!\n");
			return 0;
		}
	}
	return 1;
}
/********************************************************************************************/
//ͨѶ֡��������
//����:     ��
//����ֵ:   ��
/*****************************************************************************************/
void FrameAnalysis(T_PROTOCOL Preq)
{
	switch(Preq.CmdId) 
	{
		case FRAME_MEASURE_INFO://�״������Ϣ֡
			AnalysisMeasureInfo(&Preq);
			break;
		case FRAME_DEVICE_HEALTH_INFO://�״��豸������Ϣ֡
			AnalysisDeviceHealthInfo(&Preq);
			break;
		default:
			return;
	}
}

/********************************************************************************************/
//ͨѶ֡������
//����:     ��
//����ֵ:   ��
/*****************************************************************************************/
uint8_t P_Cmd_Process(void)
{
  T_PROTOCOL Preq;
    
	memcpy((uint8_t *)&Preq,(uint8_t * )RxBuffer.Buff,8);
	Preq.Data = &RxBuffer.Buff[8];
	
	if(FrameIsRight(Preq)==0)
	{
		return 0;
	}
	FrameAnalysis(Preq);
	return 1;
}

/********************************************************************************************************************/



