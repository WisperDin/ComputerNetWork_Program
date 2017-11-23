// check_sum.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdio.h"

#define BUFSIZE 255

//��ȡ�ļ�,���ض�ȡ�����ֽ�����
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

	//������������������
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

//��������
int sum(unsigned char* src,int length) {
	if (src==NULL||length<=0)
	{
		return;
	}
	unsigned int SumData = 0;

	int i;
	//����һ����Ϊһ��16λ�������
	for (i = 0; i < length; i+=2)
	{
		//�����ֽ�
		short int tmpOper = 0;
		//������������������ȼ���
		tmpOper = ( src[i] << 8 ) + src[i+1];
		//printf("%x\n", tmpOper);
		SumData += tmpOper;
	}
	//���Ͻ�λ
	SumData = (SumData & 0X0000FFFF) + ((SumData & 0XFFFF0000) >> 16);
	//printf("%x\n", SumData);
	return SumData;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("���������\n");
		return;
	}

	char* tarPath = argv[1];
	printf("�����ļ�:%s\n", tarPath);
	int readCount;
	unsigned char* dst = readFile(tarPath,&readCount);

	int SumData = sum(dst, readCount);

	printf("�����:%x", SumData);
    return 0;
}

