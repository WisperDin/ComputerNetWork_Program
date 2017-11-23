#include "stdio.h"

#define MAXLENGTH 1024

//mac地址 6个字节  48位
#define MACBYTELEN 6
#define MACADDRLEN 48

//数据长度 字节
#define DATABYTELEN 5 

//站表大小
#define SWTABLESIZE 255

//从文件p中接收第n行字串，保存在str中 
//假设一行的文本内容不超过1000字符，如果估计超过，请自行修改函数中的1000
//如果打不开文件，返回NULL
//如果文件不足n行，返回NULL
//如果正获得正常数据，返回str的地址

char *getfileline(char *p, int n, char *str) {
	FILE *fp;
	int i;
	if ((fp = fopen(p, "r")) == NULL) {
		printf("打开文件错误\n");
		return NULL;
	}
	for (i = 1; i<n; i++)
		if ((fgets(str, MAXLENGTH, fp)) == NULL) {
			fclose(fp);
			return NULL;
		}
	fgets(str, MAXLENGTH, fp);
	fclose(fp);
	return str;
}

typedef struct 
{
	int SrcAddr[MACBYTELEN];
	int DstAddr[MACBYTELEN];
	int Data[DATABYTELEN];
}Frame;

typedef struct 
{
	//接口
	int InfId;
	//网段地址
	int *Addr;
}SwtichTable;

int switchTableCurSize = 0;
SwtichTable *switchTable[SWTABLESIZE];

//主要用来转一个字节二进制字符串->int
//二进制字符串转int
int binaryStringToInt(char* src, int start, int length) {
	if (src == NULL || strlen(src) < length) {
		printf("src invalid");
		return -1;
	}
	if (start<0)
	{
		printf("start invalid");
		return -1;
	}
	if (length<=0)
	{
		printf("length invalid");
		return -1;
	}
	int n = 0;
	for (int i = start; i<start + length; i++)
	{
		n = n * 2 + (src[i] - '0');
	}
	return n;
}

void testBinaryStringToInt() {
	int testInt = binaryStringToInt("111111010100011110111101101101000111100001010110", 8, 8);
	printf("MAC %x", testInt);
}

void parseToByte(char *rawStr,int *dst,int length) {
	if (rawStr == NULL)
	{
		printf("rawMac NULL");
		return;
	}

	for (unsigned int i = 0; i < length; i++)
		dst[i] = binaryStringToInt(rawStr, i * 8, 8);
}

void testParseMac() {
	int mac[MACBYTELEN];
	parseToByte("111111010100011110111101101101000111100001010110", mac, MACBYTELEN);
	for (unsigned int i = 0; i < MACBYTELEN; i++)
	{
		printf("%x\n", mac[i]);
	}
}

Frame* parseFrame(char* str) {
	Frame *f = (Frame*)malloc(sizeof(Frame));
	parseToByte(str, f->SrcAddr, MACBYTELEN);
	parseToByte(str + MACADDRLEN, f->DstAddr, MACBYTELEN);
	parseToByte(str + 2 * MACADDRLEN, f->Data, DATABYTELEN);
	return f;
}

void testParseFrame() {
	Frame *f = parseFrame("1111110101000111101111011011010001111000010101101011010010000101000111111010110000010100010100100100100001100101011011000110110001101111");
	for (unsigned int i = 0; i < MACBYTELEN; i++)
	{
		printf("%x\n", f->SrcAddr[i]);
	}
	printf("\n");
	for (unsigned int i = 0; i < MACBYTELEN; i++)
	{
		printf("%x\n", f->DstAddr[i]);
	}
	printf("\n");
	for (unsigned int i = 0; i < DATABYTELEN; i++)
	{
		printf("%c\n", f->Data[i]);
	}
	printf("\n");
	free(f);
}


//1表示相同 0表示不同
int compareMacAddr(int addr1[MACBYTELEN],int addr2[MACBYTELEN]) {
	for (unsigned int i = 0; i < MACBYTELEN; i++)
	{
		if (addr1[i] != addr2[i])
		{
			return 0;
		}
	}
	return 1;
}

void PrintMacAddr(char* header,int addr[MACBYTELEN]) {
	if (header == NULL || addr == NULL)
		return;
	printf("%s:%x-%x-%x-%x-%x-%x  \n", header, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

void PrintSwitchTable() {
	printf("\n站表:\n");
	for (unsigned i = 0; i < switchTableCurSize; i++)
	{
		printf("infid:%d ", switchTable[i]->InfId);
		PrintMacAddr("addr", switchTable[i]->Addr);
	}
}



//参数为收到的帧与来自的接口
void RecvFrame(Frame* f,int fromIntf) {
	if (fromIntf != 0 && fromIntf != 1)
		return;
	if (f->SrcAddr == NULL)
		return;
	if (f->DstAddr == NULL)
		return;

	//打印收到的帧情况
	printf("------------------------------------------");
	printf("\n从接口%d收到帧\n", fromIntf);

	PrintMacAddr("此帧源地址", f->SrcAddr);
	PrintMacAddr("此帧目的地址", f->DstAddr);

	printf("\n");

	//自学习 站表更新 根据源地址
	int addrExistFlag = 0;
	for (unsigned int i = 0; i < switchTableCurSize; i++)
	{
		//找到源地址已经存在在站表
		if (compareMacAddr(f->SrcAddr, switchTable[i]->Addr))
		{
			//todo...
			switchTable[i]->InfId = fromIntf;
			addrExistFlag = 1;
			printf("自学习:更新站表 interfaceid:%d", fromIntf);
		}
	}
	if (!addrExistFlag)
	{
		SwtichTable *swT = (SwtichTable*)malloc(sizeof(SwtichTable));
		swT->InfId = fromIntf;
		swT->Addr = f->SrcAddr;
		switchTable[switchTableCurSize++] = swT;
		printf("自学习:添加一条记录到站表 interfaceid:%d ", fromIntf);
		PrintMacAddr("addr", f->SrcAddr);
	}
	printf("\n");
	//转发机制 查站表 根据目的地址
	int hasSendFlag = 0;
	for (unsigned int i = 0; i < switchTableCurSize; i++)
	{
		//找到目的地址已经存在在站表 且目的地址所在接口不与此帧接收的接口在同一接口
		if (compareMacAddr(f->DstAddr, switchTable[i]->Addr))
		{
			if (fromIntf == switchTable[i]->InfId)
			{
				printf("转发表中的接口与接收到此帧的接口一样 : %d,所以不转发帧\n", fromIntf);
				return;
			}
			printf("转发: ");
			//转发
			printf("从接口%d 转发到 接口%d\n", fromIntf, switchTable[i]->InfId);

			//源地址:
			PrintMacAddr("源地址", f->SrcAddr);

			//目的地址
			PrintMacAddr("目的地址", f->DstAddr);

			//数据
			printf("数据:%c%c%c%c%c\n", f->Data[0], f->Data[1], f->Data[2], f->Data[3], f->Data[4]);

			hasSendFlag = 1;
		}
	}
	if (!hasSendFlag)
	{
		printf("于站表未找到匹配的记录,向其余网段转发\n");

		printf("转发: ");
		//转发
		printf("从接口%d 转发到 接口%d\n", fromIntf, !fromIntf);

		//源地址:
		PrintMacAddr("源地址", f->SrcAddr);

		//目的地址
		PrintMacAddr("目的地址", f->DstAddr);

		//数据
		printf("数据:%c%c%c%c%c\n", f->Data[0], f->Data[1], f->Data[2], f->Data[3], f->Data[4]);
	}
}


void testRecvFrame() {
	Frame *f1 = parseFrame("1111110101000111101111011011010001111000010101101011010010000101000111111010110000010100010100100100100001100101011011000110110001101111");

	//跟上面的帧,目的地址,源地址换了位置
	Frame *f2 = parseFrame("1011010010000101000111111010110000010100010100101111110101000111101111011011010001111000010101100100011101100101011101000010000100100001");
	
	Frame *f3 = parseFrame("1011010110000101000110111110111000010100010100101111110101000110001111011011010001111000010101100100011101100101011101000010000100100001");


	RecvFrame(f1, 0);
	RecvFrame(f2, 1);
	RecvFrame(f3, 0);

	
	PrintSwitchTable();
}




void main() {
	
	char str[MAXLENGTH];
	//读6次数据,交替分别从文件1,文件2读取3此
	int readCount = 6;
	for (unsigned i = 0; i < readCount; i++)
	{
		int readFlag = (i % 2 == 0);
		getfileline(readFlag ? "1.txt":"2.txt", i / 2 + 1, str);
		Frame *f = parseFrame(str);
		RecvFrame(f, readFlag == 0);
	}

	PrintSwitchTable();
	
}