#define LCD_WDATA       PORTA    // LCD 데이터 포트 정의
#define LCD_WINST       PORTA

#define LCD_CTRL        PORTG    // LCD 제어포트 정의
#define LCD_EN          0
#define LCD_RW          1
#define LCD_RS          2

#define Byte unsigned char

#define On              1    //불리언 상수 정의
#define Off             0


void LCD_PORT_Init(void)
{
    DDRA = 0xFF;
    DDRG = 0x1F;
    //DDRF = 0x00;
}

#define RIGHT           1
#define LEFT            0

void LCD_Data(unsigned char ch)
{
    LCD_CTRL |= (1 << LCD_RS);
    LCD_CTRL &= ~(1 << LCD_RW);
    LCD_CTRL |= (1 << LCD_EN);
    delay_us(50);
    LCD_WDATA = ch;
    delay_us(50);
    LCD_CTRL &= ~(1 << LCD_EN);
}


void LCD_Comm(unsigned char ch)
{
    LCD_CTRL &= ~(1 << LCD_RS);
    LCD_CTRL &= ~(1 << LCD_RW);
    LCD_CTRL |= (1 << LCD_EN);
    delay_us(50);
    LCD_WINST = ch;
    delay_us(50);
    LCD_CTRL &= ~(1 << LCD_EN);
}

void LCD_delay(unsigned char ms)
{
    delay_ms(ms);
}

void LCD_Pos(unsigned char col, unsigned char row) // LCD 포지션 설정
{
    LCD_Comm(0x80|(col*0x40+row));
}

void LCD_Char(unsigned char c)
{
   LCD_Data(c);
   delay_ms(1); 
}

void LCD_Str(unsigned char *str)
{
    while(*str != 0) {
        LCD_Char(*str);
        str++;
    }
}

void LCD_Str_1Row_20L_Over(unsigned char *str)
{
    unsigned char loop_count=0;
    
    while(*str != 0) {
        
        if(loop_count == 20)
        {
            LCD_Pos(1,0);
            LCD_Str("                    ");
            LCD_Pos(1,0);   
            
            loop_count=0;
        }
        else
            loop_count++;
        
        LCD_Char(*str);
        str++; 
        
    }
}

void LCD_Clear(void)        // 화면 클리어 (1)
{
    LCD_Comm(0x01);
    LCD_delay(2);
}

void LCD_Init(void)
{
    LCD_PORT_Init();
    LCD_Comm(0x38);
    LCD_delay(4);
    LCD_Comm(0x38);
    LCD_delay(4);
    LCD_Comm(0x38);
    LCD_delay(4);
    LCD_Comm(0x0e);
    LCD_delay(4);
    LCD_Comm(0x06);
    LCD_delay(4);
    

    LCD_Clear();
} 

void Cursor_Home(void)
{
    LCD_Comm(0x02);
    LCD_delay(2);
}


void LCD_Shift(unsigned char p)
{
    if(p==RIGHT){
        LCD_Comm(0x1c);
        LCD_delay(1); // 시간지연
    }
    
    //표시 화면 전체를 왼쪽으로 이동
    else if(p == LEFT) {
        LCD_Comm(0x18);
        LCD_delay(1);
     }
}
void LCD_Cursor_Shift(unsigned char p)
{
    if(p==RIGHT)
    {
        LCD_Comm(0x14);
        LCD_delay(1);
    }
    else if(p==LEFT)
    {
        LCD_Comm(0x10);
        LCD_delay(1);
    }
}


              
