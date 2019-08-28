/**
  ******************************************************************************
  * @file    bsp_LIDAR_usart.c
  * @author  
  * @version V1.0
  * @date    
  * @brief   ����LIDAR��λ��Ϣ����
  ****************************************************************************
**/
  
#include "bsp_uart.h"
#include "lidar.h"

T_TX_BUFF TxBuffer;
T_RX_BUFF RxBuffer;


 /**
  * @brief  USART GPIO ����,����ģʽ���á�115200 8-N-1
  * @param  ��
  * @retval �� 
  */
void USART1_Init(int baud)
{
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef  USART_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);//ʹ��USART1ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
		
 	USART_DeInit(USART1);  //��λ����1

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = baud;            //���ò�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;      //һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;         //����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(USART1, &USART_InitStructure); //��ʼ������

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  //���������ж�
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);  //�������ܿ����ж�
	USART_Cmd(USART1, ENABLE);                      //ʹ�ܴ���

	USART_ClearITPendingBit(USART1, USART_IT_RXNE);   //��������жϱ�־
	USART_ClearITPendingBit(USART1, USART_IT_IDLE);   //������տ����жϱ�־

	/* Enable USART1 DMA Rxrequest */
//	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);    //����USART1��DMA����ʹ��
//	USART_ClearFlag(USART1, USART_FLAG_TC);           //���������ɱ�־
	
	  //USART1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00 ; //��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			    //IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure); //��ʼ��NVIC�Ĵ���
}

//�ض���c�⺯��printf������
int fputc(int ch, FILE *f) 
{	
		USART_SendData(USART1, (uint8_t) ch);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);		
		return (ch);
}
/********************************************************************************************************************/
//��λ֡������ɱ�־
//����:    ��
//����ֵ:  ��
/********************************************************************************************************************/
 
void RxDataComplete(void)
{
	if((RxBuffer.Rdy == 0)&&(RxBuffer.Len > 0))
	{
		if(RxBuffer.Len > 3)
			RxBuffer.Rdy = 1;
		else
			RxBuffer.Len = 0;
	}
}

/********************************************************************************************************************/
//д���ݵ����ջ���
//����:
//         data:    ���յ�������
//����ֵ:  ��
/********************************************************************************************************************/
void UartReceive(u8 data)
{
	if(RxBuffer.Rdy == 0)
	{
		RxBuffer.Buff[RxBuffer.Len++] = data;
		if(RxBuffer.Len >= sizeof(RxBuffer.Buff))
			RxBuffer.Rdy = 1;
	}
}

/***********************************************************************************************************************
* @brief  This function handles USART1 Handler.
* @param  None
* @retval None
***********************************************************************************************************************/
void USART1_IRQHandler(void)
{
    volatile u8 Temp;
    if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE) != RESET)
    {
        UartReceive(USART1->DR);
        USART_ClearFlag(USART1,USART_FLAG_RXNE);
    }
    if(USART_GetFlagStatus(USART1,USART_FLAG_IDLE) != RESET)
    {
        RxDataComplete();
        USART_ClearFlag(USART1,USART_FLAG_IDLE);
    }

    Temp = USART1->SR;
    Temp = USART1->DR;
}

/********************************************************************************************************************/
//�������ݴ�����
//����:    ��
//����ֵ:  ��
/********************************************************************************************************************/
void ProcessUartRxData(void)
{
	if(RxBuffer.Rdy == 0)
		return;

	if(RxBuffer.Rdy > 0)
		P_Cmd_Process();
	RxBuffer.Len = 0;
	RxBuffer.Rdy = 0;
}


/*********************************************END OF FILE**********************/
