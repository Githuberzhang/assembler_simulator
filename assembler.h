#ifndef assembler_h
#define assembler_h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_LEN 200
#define SUM 32
/**< ָ���� */
const char *instr_name[] = {"HLT","JMP","CJMP","OJMP","CALL","RET",
    "PUSH","POP","LOADB","LOADW","STOREB","STOREW",
    "LOADI","NOP","IN","OUT","ADD","ADDI","SUB",
    "SUBI","MUL","DIV","AND","OR","NOR","NOTB",
    "SAL","SAR","EQU","LT","LTE","NOTC"};

const char instr_format[33] = "12222133444451667575777778778881";

/**< �����Ϣ���� */
typedef struct tabnode
{
    char name[30];
    int line_num;
    struct tabnode *next;
}TABNODE;

/**< ������Ϣ���� */
typedef struct varnode
{
    char name[30];
    int size;               /**< �������� */
    int excursion;          /**< ƫ����*/
    short value[256];       /**< ������ֵ */
    int byte;               /**< �ֽ����������ж���WORD�ͱ�������BYTE�ͱ����� */
    struct varnode *next;
}VARNODE;

int GetInstrCode(const char *instrword);/**< �����Ƿ��õ�ָ����� */

/**< ָ������� */
unsigned long ConvertInstrument(char *getline,int op_num,TABNODE *hd1,VARNODE *hd2);/**< ָ������ */
int GetRegNum(char *getline,char *reg_name);/**< �ɼĴ�������Ӧ������ */

int SaveTab(char *tab_name,int line_num,TABNODE *hd);/**< ��������Ϣ������ */
int SaveVar(char *getline, int exc_num,int var_size,VARNODE *hdvar,int byte);/**< ���������Ϣ������ */

void WriteVar(FILE *fto,VARNODE *hd);/**< ��Ŀ���ļ����������ֵ */

#endif // assembler_h
