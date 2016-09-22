#include "simulator.h"

int main(int argc,char **argv)
{

	unsigned long instruction;
	unsigned long mem_size = 0x1000000;
	int (*ops[])(void) = {HLT,JMP,CJMP,OJMP,CALL,RET,PUSH,POP,
                          LOADB,LOADW,STOREB,STOREW,LOADI,NOP,
                          IN,OUT,ADD,ADDI,SUB,SUBI,MUL,DIV,AND,
                          OR,NOR,NOTB,SAL,SAR,EQU,LT,LTE,NOTC};/*函数指针数组，用于指令对应函数的调用*/

	FILE *fin;
	int ret = 1;
	long length;

	if(argc<2)
	{
		printf("ERROR: no enough command line arguments!\n");
		exit(-1);
	}
    /*模拟内存*/
	if((MEM = (unsigned char *)malloc(mem_size)) == NULL)
	{
		printf("ERROR: fail to get memery!\n");
		exit(-1);
	}
	PC = (unsigned long *)MEM;/*使指令计数器指向模拟内存的顶端*/
	if((fin = fopen(argv[1],"r")) == NULL)
	{
		printf("ERROR: can't open file!\n");
		exit(-1);
	}
    CS = PC;

	while(!feof(fin))/*从文件中取出目标代码，加载到模拟内存*/
	{
		fscanf(fin,"%li",&instruction);
		memcpy(PC,&instruction,sizeof(instruction));
		PC++;
	}

	SS = (short *)PC;
	ES = (SAVEES *)(MEM + mem_size - sizeof(SAVEES));

	//读最后一行之后PC指向数据的后一行，故需减一
	PC--;
	length = *PC;
	if(length%4 == 0)
    {
        PC -= length/4;
        DS = PC;
    }
	else
    {
        PC -= length/4 + 1;
        DS = PC;
    }
    DS = DS - 3;

    fclose(fin);
	PC = (unsigned long *)MEM;/*使PC指向模拟内存顶端的第一条指令*/

	while(ret)/*模拟处理器执行指令*/
	{
		IR = *PC;/*取指：将PC指示的指令加载到指令寄存器IR*/
		PC++;/*PC指向下一条指令*/
		ret = (*ops[OPCODE])();/*解码并执行指令*/
	}
	free(MEM);
	return 1;
}

int HLT(void)
{
	return 0;
}

int JMP(void)
{
	PC = (unsigned long *)(MEM + ADDRESS);
	return 1;
}

int CJMP(void)
{
	if(PSW.compare_flag == 1)
	{
		PC = (unsigned long *)(MEM + ADDRESS);
	}
	return 1;
}

int OJMP(void)
{
	if(PSW.overflow_flag == 1)
	{
		PC = (unsigned long *)(MEM + ADDRESS);
	}
    return 1;
}
int CALL(void)
{
    int i;
    for(i=0; i<7; i++)
    {
        ES->reg[i] = GR[i+1];
    }
    ES->c_flag = PSW.compare_flag;
    ES->o_flag = PSW.overflow_flag;
    ES->paddress = PC;
    ES--;
    PC = (unsigned long *)(MEM + ADDRESS);
    return 1;
}

int RET(void)
{
    int i;
    ES++;
    for(i=0; i<7; i++)
    {
        GR[i+1] = ES->reg[i];
    }
    PSW.compare_flag = ES->c_flag;
    PSW.overflow_flag = ES->o_flag;
    PC = ES->paddress;
    return 1;
}
int PUSH(void)
{
    *SS = GR[REG0];
    SS++;
    return 1;
}

int POP(void)
{
    if(REG0==0)
    {
        printf("ERROR!\n");
        exit(-1);
    }
    SS--;
    GR[REG0] = *SS;
    return 1;
}
int LOADB(void)
{
    GR[REG0] = (short)(*(DS+ADDRESS+GR[7]-1));
    return 1;
}

int LOADW(void)
{
	GR[REG0] = (short)((*DS+ADDRESS+GR[7]-1)<<8)+*(DS+ADDRESS+GR[7]);
	return 1;
}

int STOREB(void)
{
	*(DS+ADDRESS+GR[7]-1) = GR[REG0];
	return 1;
}

int STOREW(void)
{
    *(DS+ADDRESS+GR[7]-1) = ((GR[REG0]&0XFF00)>>8);
    *(DS+ADDRESS+GR[7]) = GR[REG0]&0X00FF;
    return 1;
}

int LOADI(void)
{
	GR[REG0] = (short)(IMMEDIATE);
	return 1;
}

int NOP(void)
{
	return 1;
}

int ADD(void)
{
	GR[REG0] = GR[REG1] + GR[REG2];
	if(GR[REG2]>0)
	{
		if(GR[REG0]<GR[REG1])
		{
			PSW.overflow_flag = 1;
		}
		else
		{
			PSW.overflow_flag = 0;
		}
	}
	else if(GR[REG2]<0)
	{
		if(GR[REG0]>GR[REG1])
		{
			PSW.overflow_flag = 1;
		}
		else
		{
			PSW.overflow_flag = 0;
		}
	}
	else
	{
		PSW.overflow_flag = 0;
	}

	return 1;

}

int ADDI(void)
{
    int n = GR[REG0]+IMMEDIATE;
    if(IMMEDIATE>0)
    {
        if(n<GR[REG0])
        {
            PSW.overflow_flag = 1;
        }
        else
        {
            PSW.overflow_flag = 0;
        }
    }
    else if(IMMEDIATE<0)
    {
        if(n>GR[REG0])
        {
            PSW.overflow_flag = 1;
        }
        else
        {
            PSW.overflow_flag = 0;
        }
    }
    else
    {
        PSW.overflow_flag = 0;
    }
    GR[REG0] = n;

    return 1;
}
int SUB(void)
{
	GR[REG0] = GR[REG1] - GR[REG2];
	if(GR[REG2]>0)
	{
		if(GR[REG0]>GR[REG1])
		{
			PSW.overflow_flag = 1;
		}
		else
		{
			PSW.overflow_flag = 0;
		}
	}
	else if(GR[REG2]<0)
	{
		if(GR[REG0]<GR[REG1])
		{
			PSW.overflow_flag = 1;
		}
		else
		{
			PSW.overflow_flag = 0;
		}
	}
	else
	{
		PSW.overflow_flag = 0;
	}

	return 1;
}
int SUBI(void)
{
    int n = GR[REG0]-IMMEDIATE;
    if(IMMEDIATE<0)
    {
        if(n<GR[REG0])
        {
            PSW.overflow_flag = 1;
        }
        else
        {
            PSW.overflow_flag = 0;
        }
    }
    else if(IMMEDIATE>0)
    {
        if(n>GR[REG0])
        {
            PSW.overflow_flag = 1;
        }
        else
        {
            PSW.overflow_flag = 0;
        }
    }
    else
    {
        PSW.overflow_flag = 0;
    }
    GR[REG0] = n;

    return 1;
}
int IN(void)
{
	read(0,(int *)(GR + REG0),1);
	return 1;
}

int OUT(void)
{
	write(1,(int *)(GR + REG0),1);
	return 1;
}


int MUL(void)
{
    GR[REG0] = GR[REG1]*GR[REG2];

    if(abs(GR[REG2])>1)
    {
        if(abs(GR[REG0])>abs(GR[REG1]))
        {
            PSW.overflow_flag = 0;
        }
        else
        {
            PSW.overflow_flag = 1;
        }
    }
    else
    {
        PSW.overflow_flag = 0;
    }
    return 1;
}
int DIV(void)
{
    if(GR[REG2]==0)
    {
        printf("0 can't be divided!\n");
        exit(-1);
    }
    GR[REG0] = GR[REG1]/GR[REG2];

    return 1;
}

int AND(void)
{
    GR[REG0] = GR[REG1] & GR[REG2];
    return 1;
}

int OR(void)
{
    GR[REG0] = GR[REG1] | GR[REG2];
    return 1;
}
int NOR(void)
{
    GR[REG0] = GR[REG1] ^ GR[REG2];
    return 1;
}

int SAL(void)
{
    GR[REG0] = GR[REG1] << GR[REG2];
    return 1;
}

int SAR(void)
{
    GR[REG0] = GR[REG1] >> GR[REG2];
    return 1;
}
int NOTB(void)
{
    GR[REG0] = ~GR[REG1];
    return 1;
}

int NOTC(void)
{
    PSW.compare_flag = !PSW.compare_flag;
	return 1;
}
int EQU(void)
{
	PSW.compare_flag = (GR[REG0] == GR[REG1]);
	return 1;
}

int LT(void)
{

	PSW.compare_flag = (GR[REG0] < GR[REG1]);
	return 1;
}

int LTE(void)
{
	PSW.compare_flag = (GR[REG0] <= GR[REG1]);
	return 1;
}
