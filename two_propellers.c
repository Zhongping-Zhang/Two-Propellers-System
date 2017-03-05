#include <aduc7026.h>
#include <stdio.h>

#define CR     0x0D

//void Uart_Initiate();
void Timer_Initiate();	   //定时器设定定时时长
void ADC_Initiate();	   //ADC上电
void PWMInit();            //PWM配置
void UpMotorData();		//更新占空比   
float AngleData();      //将计算机输出的模拟量转换成有正有负的模拟量
//int putchar(int ch);
//int getchar (void);
void MotorControl();    //根据计算机输出对电机进行控制，进而控制摆杆的转动
void IRQ_Handler() __irq;	  //中断函数
//void float_to_char(float *data); 
void StartMotor();		  //电机启动程序



int count,starttime; 
float YawRate1;
typedef signed   short   int16;      // 有符号16位整型变量
typedef unsigned int     uint32;     // 无符号32位整型变量
	int16  MotorValue[4];      //定义数组
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
	//配置IO管脚（和设置LED灯初始状态-灭）以便使用模拟SPI
	GP4DAT    = 0x3F3F0000;   



	PWMInit();				  //使能PWM0H，PWM1H，PWM2H，周期2.5ms
	Timer_Initiate();		  //定时10us，使能定时器且使能中断
	ADC_Initiate();			  //ADC上电
	Uart_Initiate();
		
	MotorValue[0] = 0;
	MotorValue[1] = 0;


	while(1)
	{

	if(count > 999)     //满10ms
	{
		count = 0;				  //更新数据计数器清零
		starttime++;
		 
		if(starttime<=300)        //启动电机的过程，电机转速从0开始逐渐增加
		{	
			StartMotor();        //看下面StartMotor()函数
			UpMotorData();       //看下面UpMotorData()函数
		}
		else
		{
			starttime=301;			//防溢出
			YawRate1 = AngleData();   //换算之后的模拟值给YawRate1 

		//	printf("%f\n",YawRate1);
			//控制律计算 
			MotorControl();              //根据计算机输出的模拟量对摆杆进行控制
			UpMotorData();
		}
	}	
		
	
	} 
	return 0;		  //必须要有返回
}


float AngleData()
{
	float AngleVoltage;
//	float Angle;

	   ADCCON=0x6A3;		 //单端模式，单次软件触发转换，采取默认情况，采样时间是8个时钟周期，时钟为2分频
	   ADCCP=0x00;			 //从ADC0采集模拟量

	
		while(!ADCSTA){}   //等待转换结束			   
		AngleVoltage = (float)((ADCDAT>>16)*0.61-1250);		//计算机输出给ADC的模拟量经过AD转换后存在ADCDAT的16位到27位里，12位ADC转换结果，参考电压是2.5V，再乘以相应的分辨率将其转换成模拟量，由于计算机输出的是0到2.5V，所以要减去1250mv，变成有正有负的模拟量，方便下面的运算
//		AngleVoltage = (ADCDAT>>16)*0.61;
//		Angle=3.45*AngleVoltage/33;

	return AngleVoltage;	
}

void Timer_Initiate()	//定时时间为10us
{
	T1CON  = 0xC0;		//周期模式，二进制数据格式，clk/1,使能Timer1
	T1LD   = 0x1A2;						    
	IRQEN |= 0x08;       //定时器1可以请求中断
}

void IRQ_Handler() __irq
{
	MyIRQState = IRQSTA;	
	if((MyIRQState & 0x08)  != 0)    //判断是否为Timer1 IRQ
	{
		count++;
		
		T1CLRI = 0;              //取消Timer1中断
	}

}



void ADC_Initiate()	  //ADC上电
{	
	int time;
	
	time = 2000;	
	ADCCON = 0x20;	 	//正常模式				
	while (time >=0)	  			
    time--;
	REFCON = 0x01;      //内部2.5V参考源接到Vref上输出，可供外部电路使用。
}

void MotorControl()
{		   
		//	MotorValue[0]      = 10445 + 250*YawRate1 + 8*(YawRate2-RollMean);
		//	MotorValue[1]      = 10445 - 250*YawRate1 - 8*(YawRate2-RollMean);
		//	MotorValue[0]      = 10445 + 250*YawRate1;
		//	MotorValue[1]      = 10445 - 250*YawRate1;
			MotorValue[0]      = 2001 + 1*YawRate1;     //经过电机启动过程，这两个变量的值已达到2001，电机转速稳定，在此基础上进行控制，使两侧电机的转速产生差别，来达到摆杆倾斜角度的目的
			MotorValue[1]      = 2001 - 1*YawRate1;     //YawRate1为正，则PWM0对应电机转速快，反之PWM1对应电机转速快

		//	MotorSendData[0] = ((MotorValue[3] >> 8)&0xFF);	//将MotorValue[3]分成两字节，以便发送。	  高字节
		//	MotorSendData[1] = (MotorValue[3]&0xFF);	   //低字节

	

}
void StartMotor()
{
	MotorValue[0] = MotorValue[0]+6.67;     //当starttime<300时，逐渐启动电机，300乘6.67等于2001，300次加满之后电机启动结束，开始根据计算机输出的信号进行控制
	MotorValue[1] = MotorValue[1]+6.67;	
}

void PWMInit()
{
	GP3CON  = 0x0010101;      //配置3.0为PWM0H 3.2为PWM1H,3.4为PWM2H
	PWMCON  = 0x0001;         //选择单次更新模式，并且使能PWM
	PWMDAT0 = 0x6600;	    //周期为2.5ms,频率为400HZ
    PWMDAT1 = 0x00;    		//Dead time死区时间为0
   	PWMCFG  = 0x00;     	//无斩波
	PWMCH0  = 0xF5CC; 		//%40占空比（0xF5CC）对应1ms高电平，80%占空比（0x1E99）对应2ms高电平，即该数变化范围为-2611(F5CC)到7833(1E99),电调接收的占空比范围是40%到80%之间
	PWMCH1  = 0xF5CC;
	PWMCH2  = 0xEB99;       //其实没用到这个
	PWMEN   = 0x2A;		   //使能0H,1H,2H
}

void UpMotorData()
{
		PWMCH0         = -2611+MotorValue[0];		   //将指令转换到电压的标度因数变为12/(15668-(-5223)即12/20891,12为电压值，20891代表1ms到2ms的变化也代表0V电压到最高电压（这段我也不懂）
		PWMCH1         = -2611+MotorValue[1];           //以占空比形式输出，-2611代表输出电压为0V，PWM赋完值后，就可以输出相应的占空比了

		//PWMCH2         = -5223+MotorValue[2];
}