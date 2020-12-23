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

    EICRA |= (0<<ISC21) | (0<<ISC20); // Low상태 일때 인터럽트 요청
    SREG |= 0x80;
    
}

// 서보모터동작을 위한 타이머카운터1
void Init_Timer1()
{
    TCCR1A = 0x82; // 비반전 출력모드, 고속 PWM
    TCCR1B = 0x1a; // 고속 PWM, 8분주비 사용
    OCR1A = 4710; // 0.5us X 3000 = 1.5ms -> 0
    ICR1 = 40000; // 20ms 주기 설정
}

interrupt [TIM1_COMPA] void timer1_out_comp()
{
    #asm("nop");
}

void Servo(int direction)
{
    // 닫혔을때 -90
    if(direction)
    {
        delay_ms(500);
        OCR1A = 1290;
    }
    // 열렸을때 90
    else
    {
        delay_ms(500);
        OCR1A = 4710;    
    }
}

// 환풍팬 동작시간을 위한 타이머카운터0
void Init_Timer0()
{
    TIMSK |= (1<<OCIE0); //출력 비교 인터럽트 0 허용
    TCCR0 |= (1<<WGM01) | (1 << CS01); // CTC모드, 8분주
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

// 눌린키의 위치를 스캔하기 위한 함수 
unsigned char KeyScan(void)
{
    unsigned int Key_Scan_Line_Sel = 0xfe;
    unsigned char Key_Scan_sel=0, key_scan_num=0;
    unsigned char Get_Key_Data=0;

    for(Key_Scan_sel=0; Key_Scan_sel<4; Key_Scan_sel++)
    {
        PORTC = Key_Scan_Line_Sel; // output
        delay_us(10);

        //수신 부분
        Get_Key_Data = (PINC & 0xf0); // 74LS14의 데이터 취득, C포트 내용의 상위 니블(4bit)만 받음

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
        Key_Scan_Line_Sel = (Key_Scan_Line_Sel<<1) | 0x01; //H/W 연결 특징을 고려하여 추가 비트 연산
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
            Servo(0); // 문열림 
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

// 눌린키의값을 실제값으로 변환하기 위한 함수
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
    char Message1[40];           // LCD 화면에 문자열 출력을 위한 문자열 변수
    char Message2[40];
           
    int readCnt = 0;             // LCD 화면에 동작중임을 나타내기 위한 변수   
    
    DDRD |= 0x03;
    
    init_reg();
    Init_Timer1();
    Interrupt_init();
    
    LCD_PORT_Init();                // LCD 포트 설정 
    LCD_Init();                     // LCD 초기화   
    
    Timer2_Init();                  // 1ms 계수 위한 타이머 초기화 
    I2C_Init();                      // I2C 통신 초기화( baudrate 설정)
    
    Init_HC_SR04();    

    delay_ms(1000);                // 온습도센서 전원 안정화 시간 대기     

    SREG|=0x80;                   // 타이머 인터럽트 활성화 위한 전역 인터럽트 활성화
         
    ti_Cnt_1ms = 0;               //  측정시간 대기를 위한 변수 초기화     
    LCD_DelCnt_1ms = 0;            //LCD 표시 주기 설정 카운트
    
    LCD_Clear();  

    startRanging(ETH_addr);
          
    while (1)
    {
        New_key_data = KeyScan();
        Key_data_trans(New_key_data);
        
        // 눌렀던 비밀번호 초기화
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
        
        
        // 대기시간이 35ms 이상일 경우 
        if(ti_Cnt_1ms > 35)
        {                               
            // 온습도센서 측정 데이터 얻기 
            delay_ms(15); //write 후 최소 50ms 이후에 Read
            Get_Data = ETH_01D_I2C_Read(ETH_addr);
            
            // lcd 출력 업데이트 주기 설정            
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
            
            // 대기시간 초기화  
            ti_Cnt_1ms = 0; 
            readCnt = (readCnt+1)%10;
        }
           
    }
}