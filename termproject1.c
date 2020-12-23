#include <mega128.h>
#include <delay.h>
#include <stdio.h>
#include "lcd.h"
#include "ETH_01D.h"
#include "HC_SR04.h"
#include <math.h>

#define Null 17
#define Star 13
#define Sharp 15
#define M1 10

unsigned char New_key_data=0, key_Num=0;
unsigned char Key_off_flag=0;
unsigned char Pan_flag=0;

unsigned int Password[4] ={1,2,3,4};
unsigned int Input_password[4] = {0,};
unsigned char Close_flag =0;

int p_count=0;
int count=0;
unsigned int cnt0;
unsigned char fivesecond;

void init_reg(void)
{
    DDRB = 0xff;
    
    // Keypad
    DDRC = 0x0f;
    PORTC = 0x0f;
    
    // Switch
    DDRD &= ~0x04;
}

void Interrupt_init()
{
    EIMSK |= (1<<INT2);         

    EICRA |= (0<<ISC21) | (0<<ISC20); // Low���� �϶� ���ͷ�Ʈ ��û
    SREG |= 0x80;
    
}

// �������͵����� ���� Ÿ�̸�ī����1
void Init_Timer1()
{
    TCCR1A = 0x82; // ����� ��¸��, ��� PWM
    TCCR1B = 0x1a; // ��� PWM, 8���ֺ� ���
    OCR1A = 4710; // 0.5us X 3000 = 1.5ms -> 0
    ICR1 = 40000; // 20ms �ֱ� ����
}

interrupt [TIM1_COMPA] void timer1_out_comp()
{
    #asm("nop");
}

void Servo(int direction)
{
    // �������� -90
    if(direction)
    {
        delay_ms(500);
        OCR1A = 1290;
    }
    // �������� 90
    else
    {
        delay_ms(500);
        OCR1A = 4710;    
    }
}

// ȯǳ�� ���۽ð��� ���� Ÿ�̸�ī����0
void Init_Timer0()
{
    TIMSK |= (1<<OCIE0); //��� �� ���ͷ�Ʈ 0 ���
    TCCR0 |= (1<<WGM01) | (1 << CS01); // CTC���, 8����
    OCR0 = 100; // 50us    
}

interrupt [EXT_INT2] void INT2_interrupt(void)
{
    delay_ms(200);
    Pan_flag = 1;
    Init_Timer0();
}

interrupt [TIM0_COMP] void timer0_out_comp(void)
{
    
    if(Pan_flag)
    {
        cnt0++;
        if(cnt0 == 20000) // 50us * 20000 = 1sec
        {
            cnt0 = 0;
            fivesecond++;
            if(fivesecond == 5)
            {
                fivesecond = 0;
                PORTB.7 = 0;
                PORTB.0 = 1;
                PORTB.1 = 0;
                delay_ms(100);
                Pan_flag = 0;
            }
            else
            {
                PORTB.7 = 1;
                PORTB.0 = 1;
                PORTB.1 = 0;
            }
        }
    }
        
}

// ����Ű�� ��ġ�� ��ĵ�ϱ� ���� �Լ� 
unsigned char KeyScan(void)
{
    unsigned int Key_Scan_Line_Sel = 0xfe;
    unsigned char Key_Scan_sel=0, key_scan_num=0;
    unsigned char Get_Key_Data=0;

    for(Key_Scan_sel=0; Key_Scan_sel<4; Key_Scan_sel++)
    {
        PORTC = Key_Scan_Line_Sel; // output
        delay_us(10);

        //���� �κ�
        Get_Key_Data = (PINC & 0xf0); // 74LS14�� ������ ���, C��Ʈ ������ ���� �Ϻ�(4bit)�� ����

        if(Get_Key_Data != 0x00)
        {
            switch(Get_Key_Data)
            {
                case 0x10: key_scan_num = Key_Scan_sel*4 + 1;
                    break;
                case 0x20: key_scan_num = Key_Scan_sel*4 + 2;
                    break;
                case 0x40: key_scan_num = Key_Scan_sel*4 + 3;
                    break;
                case 0x80: key_scan_num = Key_Scan_sel*4 + 4;
                    break;
                default :
                    key_scan_num = Null;
                    break;
            }
            return key_scan_num;
        }
        Key_Scan_Line_Sel = (Key_Scan_Line_Sel<<1) | 0x01; //H/W ���� Ư¡�� ����Ͽ� �߰� ��Ʈ ����
    }
    return key_scan_num;
}

void Inpassword(unsigned char key_Num)
{
    int i;
    delay_ms(150);
    Input_password[count] = key_Num;
    count++;
    
    if(count == 4)
    {   
        for(i=0;i<4;i++)
        {
            if(Input_password[i] == Password[i])
                p_count++;
            else;
        }
        if(p_count == 4)
        {
            Servo(0); // ������ 
            p_count=0;
            Close_flag = 1;
            delay_ms(3000);
            Trigger=0x01;
            delay_us(10);
            Trigger=0;
            getEcho();    
            while(close_count<10)
            {
              Trigger=0x01;
              delay_us(10);
              Trigger=0;
              getEcho();
            }
            delay_ms(3000);
            Servo(1);
            close_count = 0;
            Close_flag = 0;
        }
        else
        {
            p_count=0;
        }
        count = 0;
    }
    else;        
}

// ����Ű�ǰ��� ���������� ��ȯ�ϱ� ���� �Լ�
unsigned char Key_data_trans(unsigned char New_key_data)
{
    if(New_key_data)
    {
        if(New_key_data%4 != 0)
        {
            key_Num = (New_key_data/4)*3 +(New_key_data%4);
            delay_ms(100);
            Inpassword(key_Num);
            if(key_Num >=10)
            {
                switch(key_Num)
                {
                    case 10 :
                        key_Num = Star;
                        break;
                    case 11 :
                        key_Num = 0;
                        break;
                    case 12 :
                        key_Num = Sharp;
                        break;
                    default :
                        break;
                }
            }
        }
        else
            key_Num = (New_key_data/4)+9;
    }
    else
        Key_off_flag=1;
    
    return key_Num; 
}


                    
void main(void)
{
    int i;  
    unsigned int Get_Data=0;
    char Message1[40];           // LCD ȭ�鿡 ���ڿ� ����� ���� ���ڿ� ����
    char Message2[40];
           
    int readCnt = 0;             // LCD ȭ�鿡 ���������� ��Ÿ���� ���� ����   
    
    DDRD |= 0x03;
    
    init_reg();
    Init_Timer1();
    Interrupt_init();
    
    LCD_PORT_Init();                // LCD ��Ʈ ���� 
    LCD_Init();                     // LCD �ʱ�ȭ   
    
    Timer2_Init();                  // 1ms ��� ���� Ÿ�̸� �ʱ�ȭ 
    I2C_Init();                      // I2C ��� �ʱ�ȭ( baudrate ����)
    
    Init_HC_SR04();    

    delay_ms(1000);                // �½������� ���� ����ȭ �ð� ���     

    SREG|=0x80;                   // Ÿ�̸� ���ͷ�Ʈ Ȱ��ȭ ���� ���� ���ͷ�Ʈ Ȱ��ȭ
         
    ti_Cnt_1ms = 0;               //  �����ð� ��⸦ ���� ���� �ʱ�ȭ     
    LCD_DelCnt_1ms = 0;            //LCD ǥ�� �ֱ� ���� ī��Ʈ
    
    LCD_Clear();  

    startRanging(ETH_addr);
          
    while (1)
    {
        New_key_data = KeyScan();
        Key_data_trans(New_key_data);
        
        // ������ ��й�ȣ �ʱ�ȭ
        if(key_Num == M1)
        {
            delay_ms(100);
            for(i=0;i<4;i++)
            {
                Input_password[i] = 0;
            }
            count = 0;
        }
        else;
        
        
        // ���ð��� 35ms �̻��� ��� 
        if(ti_Cnt_1ms > 35)
        {                               
            // �½������� ���� ������ ��� 
            delay_ms(15); //write �� �ּ� 50ms ���Ŀ� Read
            Get_Data = ETH_01D_I2C_Read(ETH_addr);
            
            // lcd ��� ������Ʈ �ֱ� ����            
            if(LCD_DelCnt_1ms > 100)
            {    
                sprintf(Message1, "Humidity = %dRH",T_humidity);
                LCD_Pos(0,0);
                LCD_Str(Message1);
                
                sprintf(Message2, "Temperaute %dC",T_temperature);
                LCD_Pos(1,0);
                LCD_Str(Message2);
                
                LCD_DelCnt_1ms = 0;      
            }
                    
            startRanging(ETH_addr);
            
            // ���ð� �ʱ�ȭ  
            ti_Cnt_1ms = 0; 
            readCnt = (readCnt+1)%10;
        }
           
    }
}