#include "Fib.h"
#include <iostream>
#include <windows.h>
#include <conio.h>

char * ribFile		  ="rib.txt";				
char * ribFile_IP     ="rib_IP.txt";			

char * outputFile     ="rib.result";		
char * outputFile_bin ="rib_bin.result";		

char * updateFile     ="updates.txt"; 			
char * updateFile_IP  ="updates_IP.txt";	

char * oldPortfile    ="oldport.txt";			
char * oldPortfile_bin="oldport_bin.txt";		

char * traceFile	  ="trace.txt";				
char * stat_resultF	  ="stat_result.update";	

int	UpdateFileCount	  =6;						

bool	bIpFormat=true;								
#define LINESIZE	66

using namespace std;

#define TRACE_READ 100000
unsigned int traffic[TRACE_READ];

void read_trace(char *trace_path)
{
	// read traffic from file
	int return_value=-1;
	unsigned int traceNum=0;

	//first read the trace...
	ifstream fin(trace_path);
	if (!fin)return;
	fin>>traceNum;

	int TraceLine=0;
	int IPtmp=0;
	while (!fin.eof() && TraceLine<TRACE_READ )
	{
		fin>>IPtmp;
		traffic[TraceLine]=IPtmp;
		TraceLine++;
	}
	fin.close();
	printf("trace read complete...\n");

}

string DupString(char cElement,int iLen,bool bNewLine){
	string strRet;
	for (int i=1;i<=iLen;i++){
		if(i==iLen && bNewLine==true){
			strRet += "\n";
		}
		else{
			strRet += cElement;
		}
	}
	return strRet;
}
bool Exist(char * FileName)
{
	FILE*   fp;
	if((fp=fopen(FileName, "r "))==NULL)
	{
		return false;
	}
	else
	{
		fclose(fp);
		return true;
	}
}

bool ShowTipInfo()
{
	printf("                     LC-trie algorithm\n");
	printf("%s",DupString('=',LINESIZE,true).c_str());

	printf("please input the format of RIB file: binary (B) or IP (I)£º");

	char cResponse=getch();
	printf("%c",cResponse);
	while(cResponse!='B'||cResponse!='b'||cResponse!='I'||cResponse!='i')
	{
		if(cResponse=='B'||cResponse=='b')
		{
			if(Exist(ribFile)==false)
			{
				printf("\nthe rib file doesn't exist£º\t%s\n",ribFile);

				printf("%s",DupString('=',LINESIZE,true).c_str());
				printf("\t\t    Press any key to quit...");
				getch();
				return false;
			}
			else
			{
				bIpFormat=false;
				break;
			}
		}
		if(cResponse=='I'||cResponse=='i')
		{
			if(Exist(ribFile)==false)
			{
				printf("\nthe rib file doesn't exist£º\t%s\n",ribFile_IP);
				printf("%s",DupString('=',LINESIZE,true).c_str());
				printf("\t\t    Press any key to quit...");
				getch();
				return false;
			}
			else
			{
				break;
			}
		}
		printf("\nInput error! only B and I is legal£¬please input(B/I)£º");
		cResponse=getch();
		printf("%c",cResponse);
	}
	return true;
}

unsigned int ytReadandUpdate(string sFileName,CFib *tFib)
{
	unsigned int	iEntryCount=0;		//the number of items from file
	char			sPrefix[20];		//prefix from rib file
	unsigned long	lPrefix;			//the value of Prefix
	unsigned int	iPrefixLen;			//the length of PREFIX
	int				iNextHop;			//to store NEXTHOP in RIB file

	char			operate_type_read;
	int 			operate_type;
	int				readlines=0;
	long			updatetimeused=0;

	long			yearmonthday=0;		//an integer to record year, month, day
	long			hourminsec=0;		//an integer to record hour, minute, second
	long			yearmonthday_old=0;		//an integer to record year, month, day
	long			hourminsec_old=0;		//an integer to record hour, minute, second
	long			outputCount=0;

	long			validUpdateCount=0;
	long			validUpdateCount_old=0;
	long			MemoryVisitSum_old=0;


	LARGE_INTEGER frequence,privious,privious1;
	if(!QueryPerformanceFrequency(&frequence))return 0;
	FILE * fp_u;
	fp_u=fopen(stat_resultF, "w");
	//fprintf(fp_u,"Number of Update items\tNumber of Update items\tThe Number of rib nodes\tupdate time used\tBSCount\tBZdepth\tMAX_depth\tBZSize\n");
	fprintf(fp_u,"time		#items  #Update	#ofrib	time	BSCount	BZdepth	MAX_depth	BZSize\tupdateVisit#\n");

	for (int jjj=1;jjj<=UpdateFileCount;jjj++)
	{
		char strName[20];
		memset(strName,0,sizeof(strName));
		sprintf(strName,"updates%d.txt",jjj);

		ifstream fin(strName);
		if (!fin)
		{
			//printf("!!!error!!!!  no file named:%s\n",strName);
			continue;
		}
		printf("parsing %s\n",strName);

		while (!fin.eof()) 
		{
			lPrefix = 0;
			iPrefixLen = 0;
			iNextHop = -9;

			memset(sPrefix,0,sizeof(sPrefix));
			//read data from rib file, iNextHop attention !!!
			fin >> yearmonthday >> hourminsec >> operate_type_read >> sPrefix;//>> iNextHop;

			if('W'==operate_type_read)operate_type=_DELETE;
			else if ('A'==operate_type_read)
			{
				fin >> iNextHop;
				operate_type=_NOT_DELETE;
			}
			else
			{
				printf("Format of update file Error, quit....\n");
				getchar();
				return 0;
			}
			int iStart=0;				//the end point of IP
			int iEnd=0;					//the end point of IP
			int iFieldIndex = 3;		
			int iLen=strlen(sPrefix);	//the length of Prefix


			if(iLen>0)
			{
				if (yearmonthday-yearmonthday_old>0 || hourminsec-hourminsec_old>=100)
				{	

					yearmonthday_old=yearmonthday;
					hourminsec_old=hourminsec;
					//printf("%d%d\n",yearmonthday,hourminsec/100);

					int hour_format=hourminsec/100;
					char hour_string[20];
					memset(hour_string,0,sizeof(hour_string));
					if (0==hour_format)			sprintf(hour_string,"0000");
					if (hour_format<10)			sprintf(hour_string,"000%d",hour_format);
					else if (hour_format<100)	sprintf(hour_string,"00%d",hour_format);
					else if (hour_format<1000)	sprintf(hour_string,"0%d",hour_format);
					else						sprintf(hour_string,"%d",hour_format);
					
					tFib->ytGetNodeCounts();
					fprintf(fp_u,"%d%s\t%d\t%d\t%d\t%d\t%d\t%f\t%d\t%d\t%f\n",yearmonthday,hour_string,readlines,tFib->solidNodeCount,tFib->oldNodeCount,updatetimeused,tFib->BSCount,tFib->BZAve_depth,tFib->Max_depth,tFib->BZSize,1.0*(tFib->MemoryVisitSum-MemoryVisitSum_old)/(validUpdateCount-validUpdateCount_old));
					MemoryVisitSum_old=tFib->MemoryVisitSum;
					validUpdateCount_old=validUpdateCount;
				}

				readlines++;
				validUpdateCount++;
				for ( int i=0; i<iLen; i++ )
				{
					//extract the first 3 sub-part
					if ( sPrefix[i] == '.' )
					{
						iEnd = i;
						string strVal(sPrefix+iStart,iEnd-iStart);
						lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex); 
						iFieldIndex--;
						iStart = i+1;
						i++;
					}
					if ( sPrefix[i] == '/' ){
						//extract the 4th sub-part
						iEnd = i;
						string strVal(sPrefix+iStart,iEnd-iStart);
						lPrefix += atol(strVal.c_str());
						iStart = i+1;

						//extract the length of prefix
						i++;
						strVal= string(sPrefix+iStart,iLen-1);
						iPrefixLen=atoi(strVal.c_str());
					}
				}

				char insert_C[50];
				memset(insert_C,0,sizeof(insert_C));
				for (unsigned int yi=0; yi<iPrefixLen; yi++)
				{
					//turn right
					if(((lPrefix<<yi) & HIGHTBIT)==HIGHTBIT)insert_C[yi]='1';
					else insert_C[yi]='0';
				}

				if (iPrefixLen<8)printf("%d-%d; ",iPrefixLen,iNextHop);
				else
				{
					QueryPerformanceCounter(&privious);

					if(NULL==tFib->BS_Update(iNextHop,insert_C,operate_type))validUpdateCount--;

					QueryPerformanceCounter(&privious1);
					updatetimeused+=1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart;
				}
			}
		}
		fin.close();
	}

	fclose(fp_u);
	return readlines;
}

void handleTraffic(CFib *tFib)
{
	char trace_file[20];
	int  i=0;
	int  Sec_visi_count=0;
	ZeroStr(trace_file);
	for (int i=0;i<100;i++)
	{
		Sec_visi_count=0;
		sprintf(trace_file,"trace (%d).result",i);
		ifstream fin(trace_file);
		if (!fin)
		{
			printf("!!!error!!!!  no file named:%s\n",trace_file);
			continue;
		}


		char result_file[100];
		memset(result_file,0,sizeof(result_file));
		sprintf(result_file,"%s.result",trace_file);

		FILE * fp_f=fopen(result_file,"w");

		printf(      "yearmonthday\thourminsec\tSec_visi_count\t#oflookups\t#ofLookup_BS\n");
		fprintf(fp_f,"yearmonthday\thourminsec\tSec_visi_count\t#oflookups\t#ofLookup_BS\n");


		string	yearmonthday,hourminsec,sPrefix,hourminsec_old,line_read;
		int		linelength=0;
		int		mallocSize=0;
		hourminsec_old.clear();

		char sPrefix_bin[40];
		ZeroStr(sPrefix_bin);

		int MemoryAccessLookup_old=0;
		int MemoryAccessLookup_BS_old=0;
		int iClock_old=0;

		int iClock=0;
		while (!fin.eof()) 
		{
			iClock++;

			line_read.clear();
			yearmonthday.clear();
			sPrefix.clear();
			hourminsec.clear();

			getline(fin,line_read);
			linelength=line_read.length();
			if (linelength<25)
			{
				iClock--;
				continue;
			}

			yearmonthday=line_read.substr(0,10);
			hourminsec	=line_read.substr(11,8);
			sPrefix		=line_read.substr(21,linelength-21);

			// convert to binary format
			tFib->IpToBinary(sPrefix,sPrefix_bin);

			// search nexthop info from tow destination files
			//if(true==tFib->lookup(tFib->m_pTrie,sPrefix_bin,0,-1))Sec_visi_count++;
			if (true==tFib->ifSecondVisit(tFib->m_pTrie,sPrefix_bin))Sec_visi_count++;

			tFib->lookup(tFib->m_pTrie,sPrefix_bin,0,-1);

			if (hourminsec!=hourminsec_old)
			{
				double visitAverage=(tFib->MemoryAccessLookup-MemoryAccessLookup_old+0.0)/(iClock-iClock_old);
				double visitAverage_BS=(tFib->MemoryAccessLookup_BS-MemoryAccessLookup_BS_old+0.0)/(iClock-iClock_old);

				MemoryAccessLookup_old=tFib->MemoryAccessLookup;
				MemoryAccessLookup_BS_old=tFib->MemoryAccessLookup_BS;
				iClock_old=iClock;

				printf(      "%s\t%s\t%d\t%d\t%lf\t%lf\n",yearmonthday.data(),hourminsec.data(),Sec_visi_count,iClock,visitAverage,visitAverage_BS);
				fprintf(fp_f,"%s\t%s\t%d\t%d\t%lf\t%lf\n",yearmonthday.data(),hourminsec.data(),Sec_visi_count,iClock,visitAverage,visitAverage_BS);
				hourminsec_old.clear();
				hourminsec_old=hourminsec;
			}
		}

		fclose(fp_f);
		fin.close();
	}

}


//char * ConvertIntegerToBinary(unsigned int IPInteger)
//{
//	char prefix_bin[33];
//	memset(prefix_bin,'0',sizeof(prefix_bin));
//	prefix_bin[32]='\0';
//	itoa(IPInteger,prefix_bin,2);
//	
//	int len=strlen(prefix_bin);
//	char prefix_head[33];
//	memset(prefix_head,'0',sizeof(prefix_head));
//	prefix_head[32-len]='\0';
//
//	sprintf(prefix_head,"%s%s",prefix_head,prefix_bin);
//
//	return prefix_head;
//}


void main(int argc, char * argv[])
{
	CFib tFib= CFib();	
	//CFib tFib_Ori= CFib();	

	if (argc > 1)
	{
		unsigned int iEntryCount = 0;
		iEntryCount=tFib.BuildFibFromFile(argv[2]);
		//bIpFormat=true;
		//if(bIpFormat)
		//{
		//	iEntryCount=tFib.BuildFibFromFile(ribFile);
		//	//tFib_Ori.BuildFibFromFile(ribFile);
		//}
		//else
		//{	
		//	tFib.ConvertBinToIP(ribFile,ribFile_IP);
		//	iEntryCount=tFib.BuildFibFromFile(ribFile_IP);
		//	//tFib_Ori.BuildFibFromFile(ribFile);
		//}



		//LC-trie
		tFib.GetMaxLevel(tFib.m_pTrie);
		tFib.PathCompression(tFib.m_pTrie);
		tFib.LevelCompression(tFib.m_pTrie);
		read_trace(argv[1]);

		int Nodesize=20;
		tFib.ytGetNodeCounts();
		printf("\nthere are\t%u entries and\t%u solid Trie nodes£¬the tFib.allNodeCount is %d\n",iEntryCount,tFib.solidNodeCount,tFib.allNodeCount);


		int LPMPort=0;
		register unsigned int taffic_Integer=0;
		register int structInteger=0;
		LARGE_INTEGER frequence,privious,privious1,privious2;
		if(!QueryPerformanceFrequency(&frequence))return;
		QueryPerformanceCounter(&privious);
		
		for (register int j=0;j<100;j++)//
		{
			for (register int i=0;i<TRACE_READ;i++)
			{
				unsigned int iVal=traffic[i];
				char IP[40];
				ZeroStr(IP);
				sprintf(IP,"%d.%d.%d.%d",(iVal>>24),(iVal<<8)>>24,(iVal<<16)>>24,(iVal<<24)>>24);
				char sPrefix_bin[32];
				ZeroStr(sPrefix_bin);
				tFib.IpToBinary(IP,sPrefix_bin);

				LPMPort=tFib.lookup(tFib.m_pTrie,sPrefix_bin,0,-1);
			}
		}

		QueryPerformanceCounter(&privious1);
		unsigned int Lookuptime=1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart;

		for (register int j=0;j<100;j++)//
		{
			for (register int i=0;i<TRACE_READ;i++)
			{
				unsigned int iVal=traffic[i];
				char IP[40];
				ZeroStr(IP);
				sprintf(IP,"%d.%d.%d.%d",(iVal>>24),(iVal<<8)>>24,(iVal<<16)>>24,(iVal<<24)>>24);
				char sPrefix_bin[32];
				ZeroStr(sPrefix_bin);
				tFib.IpToBinary(IP,sPrefix_bin);

				//LPMPort=tFib.lookup(tFib.m_pTrie,sPrefix_bin,0,-1);
			}
		}
		QueryPerformanceCounter(&privious2);
		unsigned int Lookuptime2=1000000*(privious2.QuadPart-privious1.QuadPart)/frequence.QuadPart;

		printf("LPMPort=%d\n",LPMPort);
		printf("Lookup time:%u=%u-%u\n",Lookuptime-Lookuptime2,Lookuptime,Lookuptime2);
		
		FILE *fp = fopen("LC_trie_stat.txt", "at");
		fprintf(fp,"%s\t%s\t%u\t%u\n",argv[1],argv[2],Lookuptime-Lookuptime2,tFib.solidNodeCount*Nodesize+(tFib.allNodeCount-tFib.solidNodeCount)*8);
		fclose(fp);

		printf("OH lookup finished...\n");
		//tFib.printTest(tFib.m_pTrie);
		//ytReadandUpdate(updateFile,&tFib);


		//tFib.ytGetNodeCounts();
		//printf("\nafter update, there are\t%u solid Trie nodes£¬the allNodeCount is\t%d\n",tFib.solidNodeCount,tFib.allNodeCount);

		//printf("start handling the traffic....\n");
		//handleTraffic(&tFib);

		//printf("outputting the results...\n");

		//tFib.OutputTrie(tFib.m_pTrie,outputFile,oldPortfile);
		//printf("transforming the result into binary format...\n");

		//tFib.ConvertIpToBin(outputFile,outputFile_bin);
		////tFib.ConvertIpToBin(oldPortfile,oldPortfile_bin);

		//printf("%s",DupString('=',LINESIZE,true).c_str());
		//printf("\t\t    Press any key to quit...\n");
		//getch();
	}
}
