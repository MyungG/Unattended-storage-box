#ifndef _HC_SR04_H
#define _HC_SR04_H

#include <mega128.h>
#include <stdio.h>
#include <delay.h>
//#include "lcd.h"

#define Trigger         PORTE.0 //초음파 트리거
#define Echo            PINE.1 //초음파 에코

unsigned int range; // 거리 변수(cm)
unsigned int TCNT3; // 타이머/카운터3 16비트 값 저장 레지스터
int close_count = 0; 
//unsigned char Sonic_Decimal[3];

void getEcho(void){
    while(!Echo);     // 에코핀이 High Edge가 될 때까지 대기
    TCNT3H = 0;       // TCNT3 상위 바이트
    TCNT3L = 0;       // TCNT3 하위 바이트
    TCCR3B=2;         // 타이머/카운터3 8분주비
    while(Echo);      // 에코 핀이 Low Edge가 될 때까지 대기
    TCNT3 = TCNT3L | (TCNT3H << 8); // TCNT3에 하위바이트, 상위바이트 순으로 입력 
    TCCR3B=8;                       // 타이머/카운터3 동작 정지 (CTC 모드)
    range = TCNT3/116;              // 116clk = 58us, 초음파센서는 1cm를 58us로 측정
                                    // 타이머 클럭은 8분주, 0.5us마다 TCNT값 1씩 증가
    if(range<10)
    {
      close_count++; // 10cm보다 작은 범위 감지시   
    } 
}

void Init_HC_SR04()
{
	DDRE = 0x01; // PC0 ouput Trigger, PC1 input Echo
	TCCR3A=0; TCCR3B=0x08; // TCNT 레지스터 초기화, 현재 CTC모드로 정지 상태
	delay_ms(100);
		
}

#endif
