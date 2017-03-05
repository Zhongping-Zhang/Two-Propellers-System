#include <aduc7026.h>
#include <stdio.h>

#define CR     0x0D

//void Uart_Initiate();
void Timer_Initiate();	   //��ʱ���趨��ʱʱ��
void ADC_Initiate();	   //ADC�ϵ�
void PWMInit();            //PWM����
void UpMotorData();		//����ռ�ձ�   
float AngleData();      //������������ģ����ת���������и���ģ����
//int putchar(int ch);
//int getchar (void);
void MotorControl();    //���ݼ��������Ե�����п��ƣ��������ưڸ˵�ת��
void IRQ_Handler() __irq;	  //�жϺ���
//void float_to_char(float *data); 
void StartMotor();		  //�����������



int count,starttime; 
float YawRate1;
typedef signed   short   int16;      // �з���16λ���ͱ���
typedef unsigned int     uint32;     // �޷���32λ���ͱ���
	int16  MotorValue[4];      //��������
	uint32 MyIRQState;

void delay1(int time)
{
	while(time > 0)
	time--;
}

int main()
{		

	count     = 0;
	starttime = 0;
	//����IO�ܽţ�������LED�Ƴ�ʼ״̬-���Ա�ʹ��ģ��SPI
	GP4DAT    = 0x3F3F0000;   



	PWMInit();				  //ʹ��PWM0H��PWM1H��PWM2H������2.5ms
	Timer_Initiate();		  //��ʱ10us��ʹ�ܶ�ʱ����ʹ���ж�
	ADC_Initiate();			  //ADC�ϵ�
	Uart_Initiate();
		
	MotorValue[0] = 0;
	MotorValue[1] = 0;


	while(1)
	{

	if(count > 999)     //��10ms
	{
		count = 0;				  //�������ݼ���������
		starttime++;
		 
		if(starttime<=300)        //��������Ĺ��̣����ת�ٴ�0��ʼ������
		{	
			StartMotor();        //������StartMotor()����
			UpMotorData();       //������UpMotorData()����
		}
		else
		{
			starttime=301;			//�����
			YawRate1 = AngleData();   //����֮���ģ��ֵ��YawRate1 

		//	printf("%f\n",YawRate1);
			//�����ɼ��� 
			MotorControl();              //���ݼ���������ģ�����԰ڸ˽��п���
			UpMotorData();
		}
	}	
		
	
	} 
	return 0;		  //����Ҫ�з���
}


float AngleData()
{
	float AngleVoltage;
//	float Angle;

	   ADCCON=0x6A3;		 //����ģʽ�������������ת������ȡĬ�����������ʱ����8��ʱ�����ڣ�ʱ��Ϊ2��Ƶ
	   ADCCP=0x00;			 //��ADC0�ɼ�ģ����

	
		while(!ADCSTA){}   //�ȴ�ת������			   
		AngleVoltage = (float)((ADCDAT>>16)*0.61-1250);		//����������ADC��ģ��������ADת�������ADCDAT��16λ��27λ�12λADCת��������ο���ѹ��2.5V���ٳ�����Ӧ�ķֱ��ʽ���ת����ģ���������ڼ�����������0��2.5V������Ҫ��ȥ1250mv����������и���ģ�������������������
//		AngleVoltage = (ADCDAT>>16)*0.61;
//		Angle=3.45*AngleVoltage/33;

	return AngleVoltage;	
}

void Timer_Initiate()	//��ʱʱ��Ϊ10us
{
	T1CON  = 0xC0;		//����ģʽ�����������ݸ�ʽ��clk/1,ʹ��Timer1
	T1LD   = 0x1A2;						    
	IRQEN |= 0x08;       //��ʱ��1���������ж�
}

void IRQ_Handler() __irq
{
	MyIRQState = IRQSTA;	
	if((MyIRQState & 0x08)  != 0)    //�ж��Ƿ�ΪTimer1 IRQ
	{
		count++;
		
		T1CLRI = 0;              //ȡ��Timer1�ж�
	}

}



void ADC_Initiate()	  //ADC�ϵ�
{	
	int time;
	
	time = 2000;	
	ADCCON = 0x20;	 	//����ģʽ				
	while (time >=0)	  			
    time--;
	REFCON = 0x01;      //�ڲ�2.5V�ο�Դ�ӵ�Vref��������ɹ��ⲿ��·ʹ�á�
}

void MotorControl()
{		   
		//	MotorValue[0]      = 10445 + 250*YawRate1 + 8*(YawRate2-RollMean);
		//	MotorValue[1]      = 10445 - 250*YawRate1 - 8*(YawRate2-RollMean);
		//	MotorValue[0]      = 10445 + 250*YawRate1;
		//	MotorValue[1]      = 10445 - 250*YawRate1;
			MotorValue[0]      = 2001 + 1*YawRate1;     //��������������̣�������������ֵ�Ѵﵽ2001�����ת���ȶ����ڴ˻����Ͻ��п��ƣ�ʹ��������ת�ٲ���������ﵽ�ڸ���б�Ƕȵ�Ŀ��
			MotorValue[1]      = 2001 - 1*YawRate1;     //YawRate1Ϊ������PWM0��Ӧ���ת�ٿ죬��֮PWM1��Ӧ���ת�ٿ�

		//	MotorSendData[0] = ((MotorValue[3] >> 8)&0xFF);	//��MotorValue[3]�ֳ����ֽڣ��Ա㷢�͡�	  ���ֽ�
		//	MotorSendData[1] = (MotorValue[3]&0xFF);	   //���ֽ�

	

}
void StartMotor()
{
	MotorValue[0] = MotorValue[0]+6.67;     //��starttime<300ʱ�������������300��6.67����2001��300�μ���֮����������������ʼ���ݼ����������źŽ��п���
	MotorValue[1] = MotorValue[1]+6.67;	
}

void PWMInit()
{
	GP3CON  = 0x0010101;      //����3.0ΪPWM0H 3.2ΪPWM1H,3.4ΪPWM2H
	PWMCON  = 0x0001;         //ѡ�񵥴θ���ģʽ������ʹ��PWM
	PWMDAT0 = 0x6600;	    //����Ϊ2.5ms,Ƶ��Ϊ400HZ
    PWMDAT1 = 0x00;    		//Dead time����ʱ��Ϊ0
   	PWMCFG  = 0x00;     	//��ն��
	PWMCH0  = 0xF5CC; 		//%40ռ�ձȣ�0xF5CC����Ӧ1ms�ߵ�ƽ��80%ռ�ձȣ�0x1E99����Ӧ2ms�ߵ�ƽ���������仯��ΧΪ-2611(F5CC)��7833(1E99),������յ�ռ�ձȷ�Χ��40%��80%֮��
	PWMCH1  = 0xF5CC;
	PWMCH2  = 0xEB99;       //��ʵû�õ����
	PWMEN   = 0x2A;		   //ʹ��0H,1H,2H
}

void UpMotorData()
{
		PWMCH0         = -2611+MotorValue[0];		   //��ָ��ת������ѹ�ı��������Ϊ12/(15668-(-5223)��12/20891,12Ϊ��ѹֵ��20891����1ms��2ms�ı仯Ҳ����0V��ѹ����ߵ�ѹ�������Ҳ������
		PWMCH1         = -2611+MotorValue[1];           //��ռ�ձ���ʽ�����-2611���������ѹΪ0V��PWM����ֵ�󣬾Ϳ��������Ӧ��ռ�ձ���

		//PWMCH2         = -5223+MotorValue[2];
}