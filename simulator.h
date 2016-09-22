#ifndef Simulator_h
#define Simulator_h
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>

/*�������ģ��ָ��Ľ���*/
#define REG0 ((IR>>24) & 0x7)
#define REG1 ((IR>>20) & 0xF)
#define REG2 ((IR>>16) & 0xF)
#define IMMEDIATE (IR & 0XFFFF)
#define ADDRESS (IR & 0XFFFFFF)
#define PORT (IR & 0XFF)
#define OPCODE ((IR >> 27) & 0X1F)


typedef struct _PSW_
{
	unsigned short overflow_flag: 1;
	unsigned short compare_flag: 1;
	unsigned short reserve: 14;
}_PSW;


typedef struct _savees
{
    short reg[7];               /*����ͨ�üĴ���A~G*/
    unsigned short o_flag;
    unsigned short c_flag;
    unsigned short reserve;
    unsigned long *paddress;    /*������������*/
}SAVEES;

unsigned char *MEM;     /*�ö�̬�洢��ģ���ڴ�*/
unsigned long *CS;      /*�����*/
unsigned long *DS;      /*���ݶ�*/
SAVEES *ES;             /*���Ӷ�*/
short *SS;              /*��ջ��*/
unsigned long *PC;      /*ָ��������������������ָ����ڴ��ַ*/
short GR[8];            /*8��ͨ�üĴ���*/
_PSW PSW;               /*����״̬��*/
unsigned long IR;       /*ָ��Ĵ���*/
/*��32������ʵ��32��ָ�����Ӧ����*/
int HLT(void);

int JMP(void);
int CJMP(void);
int OJMP(void);

int CALL(void);
int RET(void);

int PUSH(void);
int POP(void);

int LOADB(void);
int LOADW(void);
int LOADI(void);

int STOREB(void);
int STOREW(void);

int NOP(void);

int IN(void);
int OUT(void);

int ADD(void);
int ADDI(void);
int SUB(void);
int SUBI(void);
int MUL(void);
int DIV(void);

int AND(void);
int OR(void);
int NOR(void);
int SAL(void);
int SAR(void);

int EQU(void);
int LT(void);
int LTE(void);
int NOTB(void);
int NOTC(void);
#endif // Simulator_h
