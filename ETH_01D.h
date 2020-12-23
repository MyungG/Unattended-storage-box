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

unsigned short int T_humidity=0, T_temperature=0; // ½ÇÁ¦°ªÀ¸·Î º¯È¯µÈ ½Àµµ°ª°ú ¿Âµµ°ªÀ» ÀúÀåÇÏ±â À§ÇÑ º¯¼ö
unsigned char ETH_addr = 0x88;      // ÃøÁ¤ÇÏ°íÀÚ ÇÏ´Â ÀåÄ¡ ÁÖ 


// I2C Å¬·°ÁÖÆÄ¼ö ¼³Á¤
void I2C_Init(void)
{
    TWBR = 0x0c; // 400kHz I2C clock frequency
}

//»óÅÂ ÄÚµå È®ÀÎÇÔ¼ö
unsigned char TWI_TransCheck_ACK(unsigned char Stat)
{ 
    char str[10];
    unsigned int ExtDev_ErrCnt = 0;
    while(!(TWCR & (1<<TWINT)))         // ÆÐÅ¶ Àü¼Û ¿Ï·áµÉ ¶§ ±îÁö  wait
    { 
        if(ExtDev_ErrCnt++ > ExtDev_ERR_MAX_CNT){ return 1; }
    }      
    
    
    if((TWSR & 0xf8) != Stat)
    {                    

        LCD_Pos(0,0);

        sprintf(str, "%02x, %02x", TWSR & 0xf8, Stat);     
        LCD_Str(str);
        delay_ms(1000);
                    
        return 2;  // Àü¼Û °Ë»ç(ACK) : error½Ã 2 ¹ÝÈ¯  
    } 
        
    else return 0;
               
}

// START Àü¼Û 
unsigned char TWI_Start()
{
    TWCR = ((1<<TWINT) | (1<<TWSTA) | (1<<TWEN));   // START ½ÅÈ£ Àü¼Û
    while(!(TWCR & (1<<TWINT)));                    // START ½ÅÈ£ Àü¼Û ¿Ï·áµÉ ¶§ ±îÁö wait
    return TWI_TransCheck_ACK(TWSR_TWI_START);    
}

// STOP Àü¼Û 
void TWI_Stop()
{
    TWCR = ((1<<TWINT) | (1<<TWSTO) | (1<<TWEN));   // STOP ½ÅÈ£ º¸³»±â
}       

// SLA+W ÆÐÅ¶ Àü¼Û
unsigned char TWI_Write_SLAW(unsigned char Addr)
{
    unsigned char ret_err=0;
    TWDR = Addr;                        // SLA + W ÆÐÅ¶(½½·¹ÀÌºê ÁÖ¼Ò+Wirte bit(Low))
    TWCR = (1<<TWINT) | (1<<TWEN);      // SLA + W ÆÐÅ¶ º¸³»±â      
    return TWI_TransCheck_ACK(MT_SLA_ACK); 
}

// SLA_R ÆÐÅ¶ Àü¼Û
unsigned char TWI_Write_SLAR(unsigned char Addr)
{
    unsigned char ret_err=0;
    TWDR = Addr+1;                  // SLA + R ÆÐÅ¶(½½·¹ÀÌºê ÁÖ¼Ò+Read bit(High))
    TWCR = (1<<TWINT) | (1<<TWEN);  // SLA + R ÆÐÅ¶ º¸³»±â 
    return TWI_TransCheck_ACK(MR_SLA_ACK);     
}    

// write ÆÐÅ¶ Àü¼Û
unsigned char ETH_01D_I2C_Write(unsigned char address)
{ 
    unsigned char ret_err=0;     
    
    ret_err = TWI_Start();              // I2C ½ÃÀÛ ºñÆ® Àü¼Û  
            
    ret_err = TWI_Write_SLAW(address);  // SLAW ÆÐÅ¶ Àü¼Û  
    if(ret_err != 0) return ret_err;    // error½Ã Á¾·á   
    TWI_Stop();                         // I2C Á¾·áºñÆ® Àü¼Û 
     
    return 0;                           // Á¤»ó Á¾·á                                
}

void startRanging(char addr)
{
    // ¿Â½Àµµ ÃøÁ¤ ¿äÃ»
    ETH_01D_I2C_Write(addr);
}

// µ¥ÀÌÅÍ 1¹ÙÀÌÆ® ¼ö½ÅÈÄ Ack½ÅÈ£ Àü¼Û
unsigned char TWI_Read_Data_Aak(unsigned char* Data)
{   
    unsigned char ret_err=0;
    
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);   
    ret_err = TWI_TransCheck_ACK(MR_DATA_ACK);
    if(ret_err != 0) 
        return ret_err;
    *Data = TWDR;           // no error, return ¼ö½Å µ¥ÀÌÅÍ(Æ÷ÀÎÅÍ·Î)        
    return 0;               // Á¤»ó Á¾·á 
}

// µ¥ÀÌÅÍ 1¹ÙÀÌÆ® ¼ö½ÅÈÄ Nack½ÅÈ£ Àü¼Û
unsigned char TWI_Read_Data_Nack(unsigned char* Data)
{   
    unsigned char ret_err=0;
    
    TWCR = (1<<TWINT)|(1<<TWEN);   
    ret_err = TWI_TransCheck_ACK(MR_DATA_NACK);
    if(ret_err != 0) 
        return ret_err;
    *Data = TWDR;           // no error, return ¼ö½Å µ¥ÀÌÅÍ(Æ÷ÀÎÅÍ·Î)        
    return 0;               // Á¤»ó Á¾·á 
}

// ¹ÞÀº µ¥ÀÌÅÍ°ª ½ÇÁ¦°ªÀ¸·Î º¯È¯
void Trans_Data(unsigned short int Humidity, unsigned short int Temperature)
{

    T_humidity = Humidity/(pow(2,14)-1)*100;
    
    T_temperature = ((Temperature/4)/(pow(2,14)-1))*165-40;
                     
}

// RESTART Àü¼Û 
unsigned char TWI_Restart()
{
    unsigned char ret_err=0;
    TWCR = ((1<<TWINT) | (1<<TWSTA) | (1<<TWEN));   // Restart ½ÅÈ£ Àü¼Û
    return TWI_TransCheck_ACK(TWSR_TWI_RESTART);                 
}  

unsigned int ETH_01D_I2C_Read(char address)
{
    unsigned short int Humidity=0, Temperature=0;                    
    unsigned char ret_err=0;   
    char str[10];
    unsigned int ETH_Data=0;
    unsigned char read_data = 0;
    
    ret_err = TWI_Start();              // Start Àü¼Û
    
    ret_err = TWI_Write_SLAR(ETH_addr);     // SLAR ÆÐÅ¶ Àü¼Û
    if(ret_err != 0) return ret_err;    // error½Ã Á¾·á
       
    ret_err = TWI_Read_Data_Aak(&read_data); // ·¹Áö½ºÅÍ µ¥ÀÌÅÍ ¼ö½Å
    if(ret_err != 0) return ret_err;        // error½Ã Á¾·á
    Humidity = read_data<<8;               // Humidity »óÀ§ ¹ÙÀÌÆ® ÀúÀå
        
    ret_err = TWI_Read_Data_Aak(&read_data); // ·¹Áö½ºÅÍ µ¥ÀÌÅÍ ¼ö½Å
    if(ret_err != 0) return ret_err;        // error½Ã Á¾·á
    Humidity |= read_data;                  // Humidity ÇÏÀ§ ¹ÙÀÌÆ® ÀúÀå
        
    ret_err = TWI_Read_Data_Aak(&read_data); // ·¹Áö½ºÅÍ µ¥ÀÌÅÍ ¼ö½Å
    if(ret_err != 0) return ret_err;        // error½Ã Á¾·á
    Temperature = read_data<<8;             // Temperature »óÀ§ ¹ÙÀÌÆ® ÀúÀå
        
    ret_err = TWI_Read_Data_Nack(&read_data); // ·¹Áö½ºÅÍ µ¥ÀÌÅÍ ¼ö½Å
    if(ret_err != 0) return ret_err;          // error½Ã Á¾·á
    Temperature |= read_data;                 // Temperature ÇÏÀ§ ¹ÙÀÌÆ® ÀúÀå
           
    TWI_Stop();                               // STOP ½ÅÈ£ Àü¼Û
    
    Humidity = (Humidity&0x3fff);       // Humidity »óÀ§ ¹ÙÀÌÆ® ÃÖ»óÀ§ 2bit µ·ÄÉ¾î Ã³¸®
    Temperature = (Temperature&0xfffc); // Temperature ÇÏÀ§ ¹ÙÀÌÆ® ÃÖÇÏÀ§ 2bit µ·ÄÉ¾î Ã³¸®
    
    Trans_Data(Humidity,Temperature);   // ½ÇÁ¦°ªÀ¸·Î º¯È¯
    
    return ETH_Data;                           // Á¤»ó Á¾·á 
}

unsigned char ti_Cnt_1ms;     // 1ms ´ÜÀ§ ½Ã°£ °è¼ö À§ÇÑ Àü¿ª º¯¼ö¼±¾ð   
unsigned char LCD_DelCnt_1ms; 

void Timer2_Init(){
    TCCR2 = (1<<WGM21)|(1<<CS20)|(0<<CS21)|(1<<CS22); //CTCëª¨ë“œ, 1024ë¶„ì£¼
    TCNT2 = 0x00;
    OCR2  = 15; // 16Mhz / 1024ë¶„ì£¼ / 15ë‹¨ê³„ = 1.041kHz 
    TIMSK = (1<<OCIE2);// ë¹„êµì¼ì¹˜ ì¸í„°ëŸ½íŠ¸ í—ˆê°€   
}

interrupt[TIM2_COMP] void timer2_comp(void)
{                
    ti_Cnt_1ms++;      
    LCD_DelCnt_1ms++;
}

#endif