#ifndef assembler_h
#define assembler_h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_LEN 200
#define SUM 32
/**< 指令名 */
const char *instr_name[] = {"HLT","JMP","CJMP","OJMP","CALL","RET",
    "PUSH","POP","LOADB","LOADW","STOREB","STOREW",
    "LOADI","NOP","IN","OUT","ADD","ADDI","SUB",
    "SUBI","MUL","DIV","AND","OR","NOR","NOTB",
    "SAL","SAR","EQU","LT","LTE","NOTC"};

const char instr_format[33] = "12222133444451667575777778778881";

/**< 标号信息链表 */
typedef struct tabnode
{
    char name[30];
    int line_num;
    struct tabnode *next;
}TABNODE;

/**< 变量信息链表 */
typedef struct varnode
{
    char name[30];
    int size;               /**< 变量个数 */
    int excursion;          /**< 偏移量*/
    short value[256];       /**< 变量的值 */
    int byte;               /**< 字节数（用于判断是WORD型变量还是BYTE型变量） */
    struct varnode *next;
}VARNODE;

int GetInstrCode(const char *instrword);/**< 由助记符得到指令代码 */

/**< 指令的译码 */
unsigned long ConvertInstrument(char *getline,int op_num,TABNODE *hd1,VARNODE *hd2);/**< 指令译码 */
int GetRegNum(char *getline,char *reg_name);/**< 由寄存器名对应到编码 */

int SaveTab(char *tab_name,int line_num,TABNODE *hd);/**< 保存标号信息到链表 */
int SaveVar(char *getline, int exc_num,int var_size,VARNODE *hdvar,int byte);/**< 保存变量信息到链表 */

void WriteVar(FILE *fto,VARNODE *hd);/**< 在目标文件中输出变量值 */

#endif // assembler_h
