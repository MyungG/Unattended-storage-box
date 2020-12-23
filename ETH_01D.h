#ifndef _ETH_01D_H
#define _ETH_01D_H_

#include <math.h>
#include <mega128.h>
#include <delay.h>
#include <stdio.h>

#define ExtDev_ERR_MAX_CNT 2000
#define TWSR_TWI_START   0x08
#define MT_SLA_ACK 0x18
#define MR_SLA_ACK 0x40
#define MR_DATA_ACK 0x50
#define TWSR_TWI_RESTART 0x10
#define MR_DATA_NACK 0x58

unsigned short int T_humidity=0, T_temperature=0; // ���������� ��ȯ�� �������� �µ����� �����ϱ� ���� ����
unsigned char ETH_addr = 0x88;      // �����ϰ��� �ϴ� ��ġ �� 


// I2C Ŭ�����ļ� ����
void I2C_Init(void)
{
    TWBR = 0x0c; // 400kHz I2C clock frequency
}

//���� �ڵ� Ȯ���Լ�
unsigned char TWI_TransCheck_ACK(unsigned char Stat)
{ 
    char str[10];
    unsigned int ExtDev_ErrCnt = 0;
    while(!(TWCR & (1<<TWINT)))         // ��Ŷ ���� �Ϸ�� �� ����  wait
    { 
        if(ExtDev_ErrCnt++ > ExtDev_ERR_MAX_CNT){ return 1; }
    }      
    
    
    if((TWSR & 0xf8) != Stat)
    {                    

        LCD_Pos(0,0);

        sprintf(str, "%02x, %02x", TWSR & 0xf8, Stat);     
        LCD_Str(str);
        delay_ms(1000);
                    
        return 2;  // ���� �˻�(ACK) : error�� 2 ��ȯ  
    } 
        
    else return 0;
               
}

// START ���� 
unsigned char TWI_Start()
{
    TWCR = ((1<<TWINT) | (1<<TWSTA) | (1<<TWEN));   // START ��ȣ ����
    while(!(TWCR & (1<<TWINT)));                    // START ��ȣ ���� �Ϸ�� �� ���� wait
    return TWI_TransCheck_ACK(TWSR_TWI_START);    
}

// STOP ���� 
void TWI_Stop()
{
    TWCR = ((1<<TWINT) | (1<<TWSTO) | (1<<TWEN));   // STOP ��ȣ ������
}       

// SLA+W ��Ŷ ����
unsigned char TWI_Write_SLAW(unsigned char Addr)
{
    unsigned char ret_err=0;
    TWDR = Addr;                        // SLA + W ��Ŷ(�����̺� �ּ�+Wirte bit(Low))
    TWCR = (1<<TWINT) | (1<<TWEN);      // SLA + W ��Ŷ ������      
    return TWI_TransCheck_ACK(MT_SLA_ACK); 
}

// SLA_R ��Ŷ ����
unsigned char TWI_Write_SLAR(unsigned char Addr)
{
    unsigned char ret_err=0;
    TWDR = Addr+1;                  // SLA + R ��Ŷ(�����̺� �ּ�+Read bit(High))
    TWCR = (1<<TWINT) | (1<<TWEN);  // SLA + R ��Ŷ ������ 
    return TWI_TransCheck_ACK(MR_SLA_ACK);     
}    

// write ��Ŷ ����
unsigned char ETH_01D_I2C_Write(unsigned char address)
{ 
    unsigned char ret_err=0;     
    
    ret_err = TWI_Start();              // I2C ���� ��Ʈ ����  
            
    ret_err = TWI_Write_SLAW(address);  // SLAW ��Ŷ ����  
    if(ret_err != 0) return ret_err;    // error�� ����   
    TWI_Stop();                         // I2C �����Ʈ ���� 
     
    return 0;                           // ���� ����                                
}

void startRanging(char addr)
{
    // �½��� ���� ��û
    ETH_01D_I2C_Write(addr);
}

// ������ 1����Ʈ ������ Ack��ȣ ����
unsigned char TWI_Read_Data_Aak(unsigned char* Data)
{   
    unsigned char ret_err=0;
    
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);   
    ret_err = TWI_TransCheck_ACK(MR_DATA_ACK);
    if(ret_err != 0) 
        return ret_err;
    *Data = TWDR;           // no error, return ���� ������(�����ͷ�)        
    return 0;               // ���� ���� 
}

// ������ 1����Ʈ ������ Nack��ȣ ����
unsigned char TWI_Read_Data_Nack(unsigned char* Data)
{   
    unsigned char ret_err=0;
    
    TWCR = (1<<TWINT)|(1<<TWEN);   
    ret_err = TWI_TransCheck_ACK(MR_DATA_NACK);
    if(ret_err != 0) 
        return ret_err;
    *Data = TWDR;           // no error, return ���� ������(�����ͷ�)        
    return 0;               // ���� ���� 
}

// ���� �����Ͱ� ���������� ��ȯ
void Trans_Data(unsigned short int Humidity, unsigned short int Temperature)
{

    T_humidity = Humidity/(pow(2,14)-1)*100;
    
    T_temperature = ((Temperature/4)/(pow(2,14)-1))*165-40;
                     
}

// RESTART ���� 
unsigned char TWI_Restart()
{
    unsigned char ret_err=0;
    TWCR = ((1<<TWINT) | (1<<TWSTA) | (1<<TWEN));   // Restart ��ȣ ����
    return TWI_TransCheck_ACK(TWSR_TWI_RESTART);                 
}  

unsigned int ETH_01D_I2C_Read(char address)
{
    unsigned short int Humidity=0, Temperature=0;                    
    unsigned char ret_err=0;   
    char str[10];
    unsigned int ETH_Data=0;
    unsigned char read_data = 0;
    
    ret_err = TWI_Start();              // Start ����
    
    ret_err = TWI_Write_SLAR(ETH_addr);     // SLAR ��Ŷ ����
    if(ret_err != 0) return ret_err;    // error�� ����
       
    ret_err = TWI_Read_Data_Aak(&read_data); // �������� ������ ����
    if(ret_err != 0) return ret_err;        // error�� ����
    Humidity = read_data<<8;               // Humidity ���� ����Ʈ ����
        
    ret_err = TWI_Read_Data_Aak(&read_data); // �������� ������ ����
    if(ret_err != 0) return ret_err;        // error�� ����
    Humidity |= read_data;                  // Humidity ���� ����Ʈ ����
        
    ret_err = TWI_Read_Data_Aak(&read_data); // �������� ������ ����
    if(ret_err != 0) return ret_err;        // error�� ����
    Temperature = read_data<<8;             // Temperature ���� ����Ʈ ����
        
    ret_err = TWI_Read_Data_Nack(&read_data); // �������� ������ ����
    if(ret_err != 0) return ret_err;          // error�� ����
    Temperature |= read_data;                 // Temperature ���� ����Ʈ ����
           
    TWI_Stop();                               // STOP ��ȣ ����
    
    Humidity = (Humidity&0x3fff);       // Humidity ���� ����Ʈ �ֻ��� 2bit ���ɾ� ó��
    Temperature = (Temperature&0xfffc); // Temperature ���� ����Ʈ ������ 2bit ���ɾ� ó��
    
    Trans_Data(Humidity,Temperature);   // ���������� ��ȯ
    
    return ETH_Data;                           // ���� ���� 
}

unsigned char ti_Cnt_1ms;     // 1ms ���� �ð� ��� ���� ���� ��������   
unsigned char LCD_DelCnt_1ms; 

void Timer2_Init(){
    TCCR2 = (1<<WGM21)|(1<<CS20)|(0<<CS21)|(1<<CS22); //CTC모드, 1024분주
    TCNT2 = 0x00;
    OCR2  = 15; // 16Mhz / 1024분주 / 15단계 = 1.041kHz 
    TIMSK = (1<<OCIE2);// 비교일치 인터럽트 허가   
}

interrupt[TIM2_COMP] void timer2_comp(void)
{                
    ti_Cnt_1ms++;      
    LCD_DelCnt_1ms++;
}

#endif