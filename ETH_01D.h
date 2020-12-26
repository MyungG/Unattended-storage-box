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

unsigned short int T_humidity=0, T_temperature=0; // 실제값으로 변환된 습도값과 온도값을 저장하기 위한 변수
unsigned char ETH_addr = 0x88;      // 측정하고자 하는 장치 주 


// I2C 클럭주파수 설정
void I2C_Init(void)
{
    TWBR = 0x0c; // 400kHz I2C clock frequency
}

//상태 코드 확인함수
unsigned char TWI_TransCheck_ACK(unsigned char Stat)
{ 
    char str[10];
    unsigned int ExtDev_ErrCnt = 0;
    while(!(TWCR & (1<<TWINT)))         // 패킷 전송 완료될 때 까지  wait
    { 
        if(ExtDev_ErrCnt++ > ExtDev_ERR_MAX_CNT){ return 1; }
    }      
    
    
    if((TWSR & 0xf8) != Stat)
    {                    

        LCD_Pos(0,0);

        sprintf(str, "%02x, %02x", TWSR & 0xf8, Stat);     
        LCD_Str(str);
        delay_ms(1000);
                    
        return 2;  // 전송 검사(ACK) : error시 2 반환  
    } 
        
    else return 0;
               
}

// START 전송 
unsigned char TWI_Start()
{
    TWCR = ((1<<TWINT) | (1<<TWSTA) | (1<<TWEN));   // START 신호 전송
    while(!(TWCR & (1<<TWINT)));                    // START 신호 전송 완료될 때 까지 wait
    return TWI_TransCheck_ACK(TWSR_TWI_START);    
}

// STOP 전송 
void TWI_Stop()
{
    TWCR = ((1<<TWINT) | (1<<TWSTO) | (1<<TWEN));   // STOP 신호 보내기
}       

// SLA+W 패킷 전송
unsigned char TWI_Write_SLAW(unsigned char Addr)
{
    unsigned char ret_err=0;
    TWDR = Addr;                        // SLA + W 패킷(슬레이브 주소+Wirte bit(Low))
    TWCR = (1<<TWINT) | (1<<TWEN);      // SLA + W 패킷 보내기      
    return TWI_TransCheck_ACK(MT_SLA_ACK); 
}

// SLA_R 패킷 전송
unsigned char TWI_Write_SLAR(unsigned char Addr)
{
    unsigned char ret_err=0;
    TWDR = Addr+1;                  // SLA + R 패킷(슬레이브 주소+Read bit(High))
    TWCR = (1<<TWINT) | (1<<TWEN);  // SLA + R 패킷 보내기 
    return TWI_TransCheck_ACK(MR_SLA_ACK);     
}    

// write 패킷 전송
unsigned char ETH_01D_I2C_Write(unsigned char address)
{ 
    unsigned char ret_err=0;     
    
    ret_err = TWI_Start();              // I2C 시작 비트 전송  
            
    ret_err = TWI_Write_SLAW(address);  // SLAW 패킷 전송  
    if(ret_err != 0) return ret_err;    // error시 종료   
    TWI_Stop();                         // I2C 종료비트 전송 
     
    return 0;                           // 정상 종료                                
}

void startRanging(char addr)
{
    // 온습도 측정 요청
    ETH_01D_I2C_Write(addr);
}

// 데이터 1바이트 수신후 Ack신호 전송
unsigned char TWI_Read_Data_Aak(unsigned char* Data)
{   
    unsigned char ret_err=0;
    
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);   
    ret_err = TWI_TransCheck_ACK(MR_DATA_ACK);
    if(ret_err != 0) 
        return ret_err;
    *Data = TWDR;           // no error, return 수신 데이터(포인터로)        
    return 0;               // 정상 종료 
}

// 데이터 1바이트 수신후 Nack신호 전송
unsigned char TWI_Read_Data_Nack(unsigned char* Data)
{   
    unsigned char ret_err=0;
    
    TWCR = (1<<TWINT)|(1<<TWEN);   
    ret_err = TWI_TransCheck_ACK(MR_DATA_NACK);
    if(ret_err != 0) 
        return ret_err;
    *Data = TWDR;           // no error, return 수신 데이터(포인터로)        
    return 0;               // 정상 종료 
}

// 받은 데이터값 실제값으로 변환
void Trans_Data(unsigned short int Humidity, unsigned short int Temperature)
{

    T_humidity = Humidity/(pow(2,14)-1)*100;
    
    T_temperature = ((Temperature/4)/(pow(2,14)-1))*165-40;
                     
}

// RESTART 전송 
unsigned char TWI_Restart()
{
    unsigned char ret_err=0;
    TWCR = ((1<<TWINT) | (1<<TWSTA) | (1<<TWEN));   // Restart 신호 전송
    return TWI_TransCheck_ACK(TWSR_TWI_RESTART);                 
}  

unsigned int ETH_01D_I2C_Read(char address)
{
    unsigned short int Humidity=0, Temperature=0;                    
    unsigned char ret_err=0;   
    char str[10];
    unsigned int ETH_Data=0;
    unsigned char read_data = 0;
    
    ret_err = TWI_Start();              // Start 전송
    
    ret_err = TWI_Write_SLAR(ETH_addr);     // SLAR 패킷 전송
    if(ret_err != 0) return ret_err;    // error시 종료
       
    ret_err = TWI_Read_Data_Aak(&read_data); // 레지스터 데이터 수신
    if(ret_err != 0) return ret_err;        // error시 종료
    Humidity = read_data<<8;               // Humidity 상위 바이트 저장
        
    ret_err = TWI_Read_Data_Aak(&read_data); // 레지스터 데이터 수신
    if(ret_err != 0) return ret_err;        // error시 종료
    Humidity |= read_data;                  // Humidity 하위 바이트 저장
        
    ret_err = TWI_Read_Data_Aak(&read_data); // 레지스터 데이터 수신
    if(ret_err != 0) return ret_err;        // error시 종료
    Temperature = read_data<<8;             // Temperature 상위 바이트 저장
        
    ret_err = TWI_Read_Data_Nack(&read_data); // 레지스터 데이터 수신
    if(ret_err != 0) return ret_err;          // error시 종료
    Temperature |= read_data;                 // Temperature 하위 바이트 저장
           
    TWI_Stop();                               // STOP 신호 전송
    
    Humidity = (Humidity&0x3fff);       // Humidity 상위 바이트 최상위 2bit 돈케어 처리
    Temperature = (Temperature&0xfffc); // Temperature 하위 바이트 최하위 2bit 돈케어 처리
    
    Trans_Data(Humidity,Temperature);   // 실제값으로 변환
    
    return ETH_Data;                           // 정상 종료 
}

unsigned char ti_Cnt_1ms;     // 1ms 단위 시간 계수 위한 전역 변수선언   
unsigned char LCD_DelCnt_1ms; 

void Timer2_Init(){
    TCCR2 = (1<<WGM21)|(1<<CS20)|(0<<CS21)|(1<<CS22); //CTC모드, 1024분주
    TCNT2 = 0x00;
    OCR2  = 15; // 16Mhz / 1024 / 15 = 1.041kHz 
    TIMSK = (1<<OCIE2);// 비교일치 인터럽트 허용  
}

interrupt[TIM2_COMP] void timer2_comp(void)
{                
    ti_Cnt_1ms++;      
    LCD_DelCnt_1ms++;
}

#endif
