#ifndef Simulator_h
#define Simulator_h
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>

/*定义宏来模拟指令的解码*/
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
    short reg[7];               /*保存通用寄存器A~G*/
    unsigned short o_flag;
    unsigned short c_flag;
    unsigned short reserve;
    unsigned long *paddress;    /*保存程序计数器*/
}SAVEES;

unsigned char *MEM;     /*用动态存储区模拟内存*/
unsigned long *CS;      /*代码段*/
unsigned long *DS;      /*数据段*/
SAVEES *ES;             /*附加段*/
short *SS;              /*堆栈段*/
unsigned long *PC;      /*指令计数器，用来存放下条指令的内存地址*/
short GR[8];            /*8个通用寄存器*/
_PSW PSW;               /*程序状态字*/
unsigned long IR;       /*指令寄存器*/
/*用32个函数实现32条指令的相应功能*/
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
