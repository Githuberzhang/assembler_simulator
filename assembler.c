#include "assembler.h"
int main(int argc, char **argv)
{
    char getline[MAX_LEN];              /**< 获取文件每行内容 */
    char *comment;                      /**< 指向注释的指针 */
    char instrword[8];                  /**< 指令助记符 */
    int op_num = 0;                     /**< 指令操作码1-32 */

    //文件指针
    FILE *ffrom;
    FILE *fto;
    int i;
    int byte;                           /**< 字节数判断是WORD还是BYTE型变量 */
    int n;                              /**< sscanf函数返回值 */
    int line_num = 0;                   /**< 存放代码当前行数 */
    char tab_temp[20];                  /**<´存放标号的临时名称 */
    char string[MAX_LEN];               /**< 存放变量name和size的字符串 */
    char var_size_str[20];              /**< 存放字符串变量的大小 */
    int var_size;                       /**< 存放整型变量的大小 */
    int exc_num = 0;

    TABNODE *hdtab = (TABNODE *)malloc(sizeof(TABNODE));
    hdtab->next = NULL;
    VARNODE *hdvar = (VARNODE *)malloc(sizeof(VARNODE));
    hdvar->next = NULL;

    if(argc<3)
    {
        printf("ERROR:lack of arguments!\n");
        return 0;
    }

    if((ffrom = fopen(argv[1],"r")) == NULL)
    {
        printf("ERROR:can't open source code file!\n");
        return 0;
    }

    if((fto = fopen(argv[2],"w")) == NULL)
    {
        printf("ERROR:can't open target code file!");
        return 0;
    }

    /**< 第一次扫描汇编代码文件 */
    while(!feof(ffrom))
    {
        fgets(getline,MAX_LEN,ffrom);

        if((comment = strchr(getline,'#')) != NULL)
        {
            *comment = '\0';/**< 去除注释 */
        }

        n = sscanf(getline,"%s",instrword);/**< 取指令助记符 */
        if(n<1)/**<  空行和注释行的处理*/
        {
            continue;
        }
        if ((comment = strchr(getline, ':'))!=NULL)
        {
            sscanf(getline, "%[^:]",tab_temp);
            SaveTab(tab_temp, line_num, hdtab);
        }
        else if(strstr(getline, "WORD")!=NULL)
        {
            sscanf(getline, "%*s %s",string);
            byte = 2;
            if((comment = strchr(string, '['))!=NULL)/**< 变量为数组形式 */
            {
                sscanf(string, "%*[^[][%[^]]",var_size_str);/**< 存变量大小 */
                var_size = atoi(var_size_str);
            }
            else
            {
                var_size = 1;
            }
            SaveVar(getline,exc_num,var_size,hdvar,byte);

            exc_num += 2*var_size;/**< 一个word占两个字节 */
            line_num--;
        }
        else if(strstr(getline, "BYTE")!=NULL)
        {
            sscanf(getline, "%*s %s",string);
            byte = 1;
            if((comment = strchr(string, '['))!=NULL)/**< 变量为数组形式 */
            {
                sscanf(string, "%*[^[][%[^]]",var_size_str);/**< 存变量大小，即若为数组，则记下数组大小 */
                var_size = atoi(var_size_str);
            }
            else
            {
                var_size = 1;/**< 单个变量则记下值为1 */
            }
            SaveVar(getline,exc_num,var_size,hdvar,byte);

            exc_num += var_size;/**< 一个byte占一个字节 */
            line_num--;
        }
        line_num++;
    }
    fclose(ffrom);
    /**< 第二次扫描汇编代码文件 */
    ffrom = fopen(argv[1], "r");
    while(!feof(ffrom))
    {
        fgets(getline,MAX_LEN,ffrom);

        if((comment = strchr(getline,'#')) != NULL)
        {
            *comment = '\0';
        }
        if((comment=strstr(getline, "WORD")) != NULL)
        {
            continue;
        }
        else if ((comment=strstr(getline, "BYTE")) != NULL)
        {
            continue;
        }
        else if((comment=strchr(getline, ':')) != NULL)/**< 去掉标号 */
        {
            for(i=0; getline[i]!=':'; i++)
            {
                getline[i] = ' ';
            }
            getline[i] = ' ';/**< 将：变成空格符 */
        }

        n = sscanf(getline, "%s",instrword);

        if(n<1)
        {
            continue;
        }
        op_num = GetInstrCode(instrword);/**< 由助记符得到指令操作码 */
        if(op_num>31)
        {
            printf("%s is a invalid instruction!\n",getline);
            exit(-1);
        }
        fprintf(fto,"0x%lx\n",ConvertInstrument(getline, op_num, hdtab, hdvar));
    }
    WriteVar(fto, hdvar);
    fclose(ffrom);
    fclose(fto);
    printf("Already converted to target code in %s\n",argv[2]);
    return 1;
}

/**< 由助记符得到指令代码 */
int GetInstrCode(const char *instrword)
{
    int i = 0;

    for(i=0; i<SUM; i++)
    {
        if(strcmp(instr_name[i],instrword) == 0)
        {
            break;
        }
    }

    return i;
}

/**< 指令的译码 */
unsigned long ConvertInstrument(char *getline,int op_num,TABNODE *hd1,VARNODE *hd2)
{
    unsigned long op_code;                      /**< 指令操作码 5bit,使用助记符表示*/
    unsigned long arg1,arg2,arg3;               /**< arg为对应的寄存器编号 */
    unsigned long returned_code = 0ul;          /**< 返回代码 */
    unsigned long address;                      /**< 地址 */

    int immenum,port;                           /**< 立即数与端口号 */
    int n;                                      /**< sscanf返回值 */
    char instrword[8],reg0[8],reg1[8],reg2[8];  /**< instrword为指令助记符，reg为寄存器名 */

    char labelstring[20];                       /**< 存放标号（在JMP等后面） */
    TABNODE *p1;
    VARNODE *p2;
    switch(instr_format[op_num])
    {
        /**< 类型 op(5bit) + padding(27bit) */
        case '1':
            op_code = op_num;
            returned_code = op_code << 27;
            break;
        /**< 类型 op(5bit) + padding(3bit) + address(24bit)*/
        case '2':
            n = sscanf(getline,"%s %s",instrword,labelstring);
            if(n<2)
            {
                printf("ERROR: bad instruction format!\n");
                exit(-1);
            }
            for (p1=hd1->next; p1 != NULL; p1=p1->next)
            {
                if(strcmp(p1->name, labelstring) == 0)
                {
                    break;
                }
            }
            if(p1 == NULL)
            {
                printf("ERROR: %s",getline);
                exit(-1);
            }
            address = (unsigned long)((p1->line_num)*4);
            op_code = GetInstrCode(instrword);
            returned_code = (op_code << 27) | (address & 0x00ffffff);
            break;
        /**< 类型 op(5bit) + reg0(3bit) + padding(24bit)*/
        case '3':
            n = sscanf(getline,"%s %s",instrword,reg0);
            if(n<2)
            {
                printf("ERROR: bad instruction format!\n");
                exit(-1);
            }

            op_code = GetInstrCode(instrword);
            arg1 = GetRegNum(getline,reg0);/**< 由寄存器名得到寄存器编号 */
            returned_code = (op_code << 27) | (arg1 << 24);
            break;
        /**< 类型 OP(5bit) + reg0(3bit) + address(24bit)*/
        case '4':
            n = sscanf(getline,"%s %s %s",instrword,reg0,labelstring);
            if(n<3)
            {
                printf("ERROR: bad instruction format!\n");
                exit(-1);
            }
            for(p2=hd2->next;p2 != NULL; p2=p2->next)
            {
                if(strcmp(p2->name,labelstring))
                {
                    break;
                }
            }
            if(p2 ==  NULL)
            {
                printf("ERROR: %s",getline);
                exit(-1);
            }
            address = (unsigned long)(p2->excursion);
            op_code = GetInstrCode(instrword);
            arg1 = GetRegNum(getline,reg0);
            returned_code = (op_code << 27) | (arg1 << 24) | (address & 0x00ffffff);
            break;
        /**< 类型 OP(5bit) + reg0(3bit) + padding(8bit) + immediate(16bit)*/
        case '5':
            n = sscanf(getline,"%s %s %i",instrword,reg0,&immenum);
            if(n<3)
            {
                printf("ERROR: bad instruction format!\n");
                exit(-1);
            }

            op_code = GetInstrCode(instrword);
            arg1 = GetRegNum(getline,reg0);
            returned_code = (op_code << 27) | (arg1 << 24) |(immenum & 0x0000ffff);
            break;
        /**< 类型 OP(5bit) + reg0(3bit) + padding(16bit) + port(8bit)*/
        case '6':
            n = sscanf(getline,"%s %s %i",instrword,reg0,&port);
            if(n<3)
            {
                printf("ERROR: bad instruction format!\n");
                exit(-1);
            }

            op_code = GetInstrCode(instrword);
            arg1 = GetRegNum(getline,reg0);
            returned_code = (op_code << 27) | (arg1 << 24) | (port & 0x0000ffff);
            break;
        /**< 类型 OP(5bit) + reg0(3bit) + + reg1(4bit) + reg2(4bit) + padding(16bit)*/
        case '7':
            n = sscanf(getline,"%s %s %s %s",instrword,reg0,reg1,reg2);
            if(n<4)
            {
                printf("ERROR: bad instruction format!\n");
                exit(-1);
            }

            op_code = GetInstrCode(instrword);
            arg1 = GetRegNum(getline,reg0);
            arg2 = GetRegNum(getline,reg1);
            arg3 = GetRegNum(getline,reg2);
            returned_code = (op_code << 27) | (arg1 << 24) | (arg2 << 20) | (arg3 << 16);
            break;
        /**< 类型 OP(5bit) + reg0(3bit) + + reg1(4bit) + padding(20bit)*/
        case '8':
            n = sscanf(getline, "%s %s %s",instrword,reg0,reg1);
            if(n<3)
            {
                printf("ERROR: bad instruction format!\n");
                exit(-1);
            }
            op_code = GetInstrCode(instrword);
            arg1 = GetRegNum(getline, reg0);
            arg2 = GetRegNum(getline, reg1);
            returned_code = (op_code << 27) | (arg1 << 24) | (arg2 << 20);
            break;
    }
    return returned_code;
}
/**< 由寄存器名得到寄存器编号 */
int GetRegNum(char *getline,char *reg_name)
{
    int reg_num;
    if(*reg_name == 'Z')
    {
        reg_num = 0;
    }
    else if(*reg_name <= 'G' && *reg_name >= 'A')
    {
        reg_num = *reg_name - 'A' + 1;
    }
    else
    {
        printf("error register name in %s!\n",getline);
        exit(-1);
    }

    return reg_num;
}
/**< 存放标号信息链表 */
int SaveTab(char *tab_name,int line_num,TABNODE *hd)
{
    TABNODE *p;
    for(p = hd;p->next != NULL;p=p->next)
    {
        ;
    }
    TABNODE *pnew = (TABNODE *)malloc(sizeof(TABNODE));

    p->next = pnew;
    pnew->line_num = line_num;
    strcpy(pnew->name, tab_name);
    pnew->next = NULL;

    return 1;
}
/**< 存放变量信息链表 */
int SaveVar(char *getline, int exc_num,int var_size,VARNODE *hdvar,int byte)
{
    VARNODE *p;
    short value1;
    char *pc = NULL;
    char value_str[20];
    int i,k;
    int flag;               /**< 判断特殊符号 */
    char var_temp[20];
    char string[MAX_LEN];
    sscanf(getline, "%*s %s",string);                       /**< * 号表示跳过该字符串不读 */
    sscanf(string, "%[^[]",var_temp);
    for (p=hdvar; p->next != NULL; p = p->next)
    {
        ;
    }

    VARNODE *pnew = (VARNODE *)malloc(sizeof(VARNODE));
    p->next = pnew;
    pnew->excursion = exc_num;
    pnew->size = var_size;
    pnew->byte = byte;

    strcpy(pnew->name, var_temp);
    pnew->next = NULL;

    /**< 处理变量的赋值 */
    for (i=0; i<var_size; i++)
    {
        pnew->value[i] = 0;
    }

    if((pc = strchr(getline, '=')))
    {
        if(var_size == 1)
        {
            sscanf(getline, "%*[^=]=%hd",&value1);/**< 取=号后的值 */
            pnew->value[0] = value1;
        }
        else
        {
            for(;pc != NULL;pc++)
            {
                if(*pc == '{')
                {
                    ++pc;
                    flag = 1;/**< 表示将会被赋值 */
                    k = 0;
                    while (*pc!='}')
                    {
                        for(i=0; *pc!=',' && *pc != '}'; pc++)
                        {
                            value_str[i++] = *pc;
                        }
                        value_str[i] = '\0';
                        pnew->value[k++] = atoi(value_str);
                        if(*pc == ',')
                        {
                            ++pc;
                        }
                    }
                }
                else if(*pc == '"')
                {
                    flag = 1;/**< 表示将会被赋值 */
                    ++pc;
                    k = 0;
                    for(i=0; *pc != '"'; pc++)
                    {
                        pnew->value[k++] = *pc;
                    }
                    pnew->value[k] = '\0';
                }
                if(flag == 1)
                {
                    break;
                }
            }
        }
    }
    return 0;
}
/**< 输出变量的值至目标文件 */
void WriteVar(FILE *fto,VARNODE *hd)
{
    VARNODE *p;
    int i;
    long k = 0;
    unsigned long address = 0ul;
    for(p = hd->next; p!=NULL; p=p->next)
    {
        for(i=0; i<p->size; i++)
        {
            if((k%4)==0)
            {
               if(p->byte == 1)
               {
                   address += p->value[i];
                   k += 1;
               }
               else if(p->byte == 2)
               {
                   address += (p->value[i])<<8;
                   k += 2;
               }
            }
            else if((k%4)==1)
            {
                if(p->byte == 1)
                {
                    address += (p->value[i])<<8;
                    k += 1;
                }
                else if((p->byte)==2)
                {
                    address += (p->value[i])<<16;
                    k += 2;
                }
            }
            else if((k%4)==2)
            {
                if(p->byte == 1)
                {
                    address += (p->value[i])<<16;
                    k += 1;
                }
                else if((p->byte)==2)
                {
                    address += (p->value[i])<<24;
                    k += 2;
                    fprintf(fto,"0x%08lx\n",address);
                    address = 0ul;
                }
            }
            else if((k%4)==3)
            {
                if(p->byte == 1)
                {
                    address += (p->value[i])<<24;
                    k += 1;
                    fprintf(fto,"0x%08lx\n",address);
                    address = 0ul;
                }
                else if((p->byte)==2)
                {
                    address += (p->value[i]&0x00ff)<<24;
                    k += 2;
                    fprintf(fto,"0x%08lx\n",address);
                    address = (p->value[i])>>8;
                }
            }
        }
    }
    if((k%4)!=0)
    {
        fprintf(fto,"0x%08lx\n",address);
    }
    fprintf(fto,"0x%08lx\n",k);

}
