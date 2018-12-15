#include<windows.h>
#include<winnt.h>
#include<stdio.h>
#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>
#include <malloc.h>
#include<math.h>
//换底公式，破c里没有自定义底数的log函数
double loga(double n, double base){
	return double(log(n)/log(base));
}
//获取指定区段的熵值
float get_entropy(char *ptr,int start,int end){
	if((end-start)==0){return 0;}
	double entropy=0;
	int length=end-start;
	for(int a=0;a<256;a++){
		int len=length;
		//一个byte一个byte得统计，0x00到0xFF
		BYTE *p=(BYTE*)(ptr+start);
		int count=0;
		while(len--){
			if(*p++==a){
				count++;
			}
		}
		double p_c=double(double(count)/double(length));
		if(p_c>0)entropy += -double(p_c*loga(p_c,2));
	}
	return entropy;
}

int file_size(char* filename)  
{  
    FILE *fp=fopen(filename,"r");  
    if(!fp) return -1;  
    fseek(fp,0L,SEEK_END);  
    int size=ftell(fp);  
    fclose(fp);
    return size;  
}
int main(int argc,char *argv[],char* envp[]){
	if(argc!=2){
		printf("usage:");
		return 0;
	}
	printf("%s",argv[1]);
	int filesize=file_size(argv[1]);
	int fd=open(argv[1],O_RDONLY|O_BINARY); 
	void *p=malloc(filesize);
	BYTE *ptr=(BYTE*)p;
	read(fd, p, filesize);
	
	//判断e_magic位置处的值是不是MZ
	PIMAGE_DOS_HEADER pDos=(PIMAGE_DOS_HEADER)ptr;
	if(pDos->e_magic!=IMAGE_DOS_SIGNATURE){
		printf("没有MZ标志位，不是一个可执行文件");
		return -1;
	}
	
	//通过dos头里的e_lfanew成员找到nt头
	PIMAGE_NT_HEADERS pNtH = (PIMAGE_NT_HEADERS) (ptr + pDos->e_lfanew);
	
	//通过IMAGE_FILE_HEADER里的成员变量NumberOfSections获取到区段的数目
	IMAGE_FILE_HEADER pFileH=(IMAGE_FILE_HEADER)(pNtH->FileHeader);
	int numOfSections=pFileH.NumberOfSections;
	printf("\r\n一共%d个节\r\n",numOfSections);
	
	//通过IMAGE_FIRST_SECTION宏获取到区段表的第一个头部指针
	PIMAGE_SECTION_HEADER pSec= (PIMAGE_SECTION_HEADER)IMAGE_FIRST_SECTION(pNtH);
	
	//遍历节表 Name是区段名称，PointerToRawData指向区段，SizeOfRawData是区段所占空间
	while(numOfSections--){
		printf("\r\n%s",(char*)pSec->Name);
		int offset=pSec->PointerToRawData;
		int length=pSec->SizeOfRawData;
		float entropy=get_entropy((char*)ptr,offset,offset+length);
		printf("     %f    ",entropy);
		if(entropy>6.75)printf("加壳了");
		if(entropy<6.75 && entropy >6.5)printf("可能加壳了");
		if(entropy<6.5)printf("貌似没加壳");
		pSec++;
	}
	return 0;
}