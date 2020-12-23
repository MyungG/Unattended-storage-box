#ifndef _HC_SR04_H
#define _HC_SR04_H

#include <mega128.h>
#include <stdio.h>
#include <delay.h>
//#include "lcd.h"

#define Trigger         PORTE.0 //������ Ʈ����
#define Echo            PINE.1 //������ ����

unsigned int range; // �Ÿ� ����(cm)
unsigned int TCNT3; // Ÿ�̸�/ī����3 16��Ʈ �� ���� ��������
int close_count = 0; 
//unsigned char Sonic_Decimal[3];

void getEcho(void){
    while(!Echo);     // �������� High Edge�� �� ������ ���
    TCNT3H = 0;       // TCNT3 ���� ����Ʈ
    TCNT3L = 0;       // TCNT3 ���� ����Ʈ
    TCCR3B=2;         // Ÿ�̸�/ī����3 8���ֺ�
    while(Echo);      // ���� ���� Low Edge�� �� ������ ���
    TCNT3 = TCNT3L | (TCNT3H << 8); // TCNT3�� ��������Ʈ, ��������Ʈ ������ �Է� 
    TCCR3B=8;                       // Ÿ�̸�/ī����3 ���� ���� (CTC ���)
    range = TCNT3/116;              // 116clk = 58us, �����ļ����� 1cm�� 58us�� ����
                                    // Ÿ�̸� Ŭ���� 8����, 0.5us���� TCNT�� 1�� ����
    if(range<10)
    {
      close_count++;  
    } 
}

void Init_HC_SR04()
{
	DDRE = 0x01; // PC0 ouput Trigger, PC1 input Echo
	TCCR3A=0; TCCR3B=0x08; // TCNT �������� �ʱ�ȭ, ���� CTC���� ���� ����
	delay_ms(100);
		
}

#endif
