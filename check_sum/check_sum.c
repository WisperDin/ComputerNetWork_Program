// check_sum.cpp : 定义控制台应用程序的入口点。
//

#include "stdio.h"

#define BUFSIZE 255

//读取文件,返回读取到的字节数据
unsigned char * readFile(char* tarPath,int *ref_readCount) {
	if (tarPath == NULL)
	{
		return;
	}
	FILE *fp;
	if ((fp = fopen(tarPath, "r")) == NULL) {
		printf("Cannot open file");
		exit(1);
	}
	unsigned char buf[BUFSIZE];
	int readCount = fread(buf, sizeof(unsigned char), BUFSIZE, fp);

	//处理奇数个数据问题
	if (readCount %2 != 0)
	{
		buf[readCount++] = 0;
	}
	*ref_readCount = readCount; 
	unsigned char *dst = (unsigned char*)malloc(sizeof(unsigned char)*readCount);
	int i;
	for (i = 0; i < readCount; i++)
	{
		dst[i] = buf[i];
	}
	return dst;
}

//计算检验和
int sum(unsigned char* src,int length) {
	if (src==NULL||length<=0)
	{
		return;
	}
	unsigned int SumData = 0;

	int i;
	//两个一组作为一个16位整数求和
	for (i = 0; i < length; i+=2)
	{
		//两个字节
		short int tmpOper = 0;
		//加运算比左移运算优先级高
		tmpOper = ( src[i] << 8 ) + src[i+1];
		//printf("%x\n", tmpOper);
		SumData += tmpOper;
	}
	//加上进位
	SumData = (SumData & 0X0000FFFF) + ((SumData & 0XFFFF0000) >> 16);
	//printf("%x\n", SumData);
	return SumData;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("请输入参数\n");
		return;
	}

	char* tarPath = argv[1];
	printf("读入文件:%s\n", tarPath);
	int readCount;
	unsigned char* dst = readFile(tarPath,&readCount);

	int SumData = sum(dst, readCount);

	printf("检验和:%x", SumData);
    return 0;
}

