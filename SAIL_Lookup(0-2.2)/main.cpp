#include "Fib.h"
#include <iostream>

#include <stdio.h>
#include <fstream>
#include <math.h>
#include <string.h>
#include <conio.h>


#define IP_LEN		32

char * ribFile					= "rib.txt";				//original Rib file
char * updateFile				= "updates.txt"; 			//update file in IP format

char * oldPortfile1				= "oldport1.txt";
char * oldPortfile_bin1			= "oldport_bin1.txt";
char * oldPortfile2				= "oldport2.txt";
char * oldPortfile_bin2			= "oldport_bin2.txt";
char * oldPortfile3				= "oldport3.txt";
char * oldPortfile_bin3			= "oldport_bin3.txt";

char * newPortfile1				= "newport1.txt";
char * newPortfile_bin1			= "newport_bin1.txt";
char * newPortfile2				= "newport2.txt";
char * newPortfile_bin2			= "newport_bin2.txt";
char * newPortfile3				= "newport3.txt";
char * newPortfile_bin3			= "newport_bin3.txt";

char * trace_path				= "trace(100000).integer";
char * ribfileName				= "rib.txt.port";
char ret[IP_LEN+1];

#define UpdateFileCount		6
#define UPDATE_ALG	_MINI_REDUANDANCY_TWOTRAS


int FindFibTrieNextHop(FibTrie * m_trie, char * insert_C)
{
	int nextHop = -1;//init the return value
	FibTrie *insertNode = m_trie;

	if (insertNode->oldPort != 0) {
		nextHop = insertNode->oldPort;
	}

	int len=(int) strlen(insert_C);

	for (int i=0; i < (len + 1);i++)
	{		
		if ('0' == insert_C[i])
		{//if 0, turn left
			if (NULL != insertNode->lchild)	
			{
				insertNode = insertNode->lchild;
			}
			else {
				break;
			}
		}
		else
		{//if 1, turn right
			if (NULL != insertNode->rchild) {
				insertNode = insertNode->rchild;
			}
			else {
				break;
			}
		}

		if (insertNode->newPort != 0)	{
			nextHop = insertNode->newPort;
		}
	}

	return	nextHop;
}



//given a ip in binary---str and its length---len, return the next ip in binary
char * GetStringIP(char *str, int len)
{
	memset(ret,0,sizeof(ret));
	memcpy(ret,str,IP_LEN);
	int i;
	for (i=0;i<len;i++)
	{
		if ('0'==ret[i])
		{
			ret[i]='1';
			break;
		}
		else if ('1'==ret[i])
		{
			ret[i]='0';
		}
	}
	//printf("%s*\n",ret);
	return ret;
}

unsigned int btod(char *bstr)
{
	unsigned int d = 0;
	unsigned int len = (unsigned int)strlen(bstr);
	if (len > 32)
	{
		printf("too long\n");
		return -1; 
	}
	len--;
	for (unsigned int i = 0; i <= len; i++)
	{
		d += (bstr[i] - '0') * (1 << (len - i));
	}
	return d;
}

void sailDetectForFullIp(CFib *tFib) {
	int nonRouteStatic=0;

	int hop1=0;
	int hop2=0;

	char strIP00[IP_LEN + 1];
	memset(strIP00, 0, sizeof(strIP00));
	
	for (int tmp=0; tmp < IP_LEN; tmp++)
	{
		strIP00[tmp]='0';
	}

	int len88 = (int)strlen(strIP00);

	char new_tmp[IP_LEN + 1];
	char old_tmp[IP_LEN + 1];

	memset(new_tmp, 0, sizeof(new_tmp));
	memset(new_tmp, 0, sizeof(new_tmp));
	memcpy(new_tmp, strIP00, IP_LEN);

	double zhishuI = pow((double)2,(double)IP_LEN);

	bool ifhalved = false;
	printf("\t\ttotal\t%.0f\t\n", zhishuI);
	printf("\t\tlength\tcycles\t\tpercent\tnexthop\n");

	for (long long k=0; k < zhishuI; k++)
	{
		memcpy(old_tmp, new_tmp, IP_LEN);
		memcpy(new_tmp, GetStringIP(old_tmp, IP_LEN), IP_LEN);
		
		hop1 = FindFibTrieNextHop(tFib->m_pTrie, new_tmp);
	
		unsigned int IPInteger = btod(new_tmp);
		hop2 = tFib->sailLookup(IPInteger);

		if (hop1== -1 && hop2 != hop1)
		{
			nonRouteStatic++;
			continue;
		}

		double ratio=0;
		
		if (hop2 != hop1)
		{
			printf("%d:%d", hop1, hop2);
			printf("\n\n\t\tNot Equal!!!\n");
			_getch();
		}
		else 
		{
			if (k%100000 == 0)
			{
				ratio=k/(double)(zhishuI/100);
				printf("\r\t\t%d\t%lld\t%.2f%%\t%d             ", IP_LEN, k, ratio, hop1);
			}
		}
	}

	printf("\n\t\tTotal number of garbage roaming route£º%d",nonRouteStatic);
	printf("\n\t\tEqual!!!!\n");
}

void help () {
	printf ("#######################################\n");
	printf ("##  *-*-*-*-OH algorithm-*-*-*-*-*   ##\n");
	printf ("#   {para} = [trace name] [rib name]  #\n");
	printf ("##       trace_path   ribfileName    ##\n");
	printf ("#######################################\n");
	system ("pause");
}

// Levelpushing Trie Update
unsigned int BFLevelPushingTrieUpdate(string sFileName,CFib *tFib)
{
	unsigned int		iEntryCount = 0;					//the number of items from file
	char				sPrefix[20];						//prefix from rib file
	unsigned long		lPrefix;							//the value of Prefix
	unsigned int		iPrefixLen;							//the length of PREFIX
	int					iNextHop;							//to store NEXTHOP in RIB file

	char				operate_type_read;
	int 				operate_type;
	int					readlines = 0;
	unsigned long long	updatetimeused = 0;

	long				yearmonthday=0;						//an integer to record year, month, day
	long				hourminsec=0;						//an integer to record hour, minute, second
	long				yearmonthday_old=0;					//an integer to record year, month, day
	long				hourminsec_old=0;					//an integer to record hour, minute, second
	
	long				outputCount=0;
	long				insertNum_old=0;
	long				DelNum_old=0;
	long				readlines_old=0;

	LARGE_INTEGER frequence,privious,privious1;
	if(!QueryPerformanceFrequency(&frequence)) return 0;
	
	for (int jjj = 1; jjj <= UpdateFileCount; jjj++)
	{
		char strName[20];
		memset(strName, 0, sizeof(strName));
		sprintf(strName, "updates%d.txt", jjj);

		ifstream fin(strName);
		if (!fin)
		{
			//printf("!!!error!!!!  no file named:%s\n",strName);
			continue;
		}

		printf("\n    Parsing %s\n", strName);

		while (!fin.eof() ) {			// Add by Qiaobin Fu 2014-4-22
		
			lPrefix = 0;
			iPrefixLen = 0;
			iNextHop = -9;

			memset(sPrefix,0,sizeof(sPrefix));
			
			//read data from rib file, iNextHop attention !!!
			fin >> yearmonthday >> hourminsec >> operate_type_read >> sPrefix;	//>> iNextHop;

			if('W' == operate_type_read) {
				operate_type = _DELETE;
			}
			else if ('A' == operate_type_read)
			{
				fin >> iNextHop;
				operate_type = _NOT_DELETE;
			}
			else
			{
				printf("\tFormat of update file Error, quit....\n");
				getchar();
				return 0;
			}

			int iStart = 0;								//the end point of IP
			int iEnd = 0;								//the end point of IP
			int iFieldIndex = 3;		
			int iLen = (int)strlen(sPrefix);			//the length of Prefix
		
			if(iLen > 0)
			{
				readlines++;

				for ( int i=0; i<iLen; i++ )
				{
					//extract the first 3 sub-part
					if ( sPrefix[i] == '.' )
					{
						iEnd = i;
						string strVal(sPrefix + iStart, iEnd - iStart);
						lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex); 
						iFieldIndex--;
						iStart = i + 1;
						i++;
					}

					if ( sPrefix[i] == '/' ) {
						//extract the 4th sub-part
						iEnd = i;
						string strVal(sPrefix + iStart, iEnd - iStart);
						lPrefix += atol(strVal.c_str());
						iStart = i + 1;

						//extract the length of prefix
						i++;
						strVal = string(sPrefix + iStart, iLen - 1);
						iPrefixLen = atoi(strVal.c_str());
					}
				}

				char insert_C[50];
				memset(insert_C,0,sizeof(insert_C));
				//insert the current node into Trie tree
				for (unsigned int yi = 0; yi < iPrefixLen; yi++)
				{
					//turn right
					if(((lPrefix << yi) & HIGHTBIT) == HIGHTBIT) insert_C[yi]='1';
					else insert_C[yi]='0';
				}
				//printf("%s\/%d\t%d\n",insert_C,iPrefixLen,iNextHop);

				if (iPrefixLen < 8) {
					//printf("%d-%d; ", iPrefixLen, iNextHop);
				}
				else
				{
					QueryPerformanceCounter(&privious); 
					tFib->Update(iNextHop, insert_C, operate_type, sPrefix);

					QueryPerformanceCounter(&privious1);
					updatetimeused += 1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart;
				}
			}
		}
		fin.close();
	}

	printf("\tupdate performance: readline=%u\ttime=%uus\n\tspeed=%.7f Mups\n",readlines,updatetimeused, readlines/(updatetimeused+0.0));
	return readlines;
}

void amination()
{
	//system("color 1D");
	system("mode con cols=75 lines=40 &color 3f");
	int sleeptime=200;
	printf("\t\t    |-*-*-*-*-*-*-     -*-*-*-*-*-*-*-|\n");
	printf("\t\t    |-*-*-*-                 -*-*-*-*-|\n");
	printf("\t\t    |          S                      |");
	Sleep(sleeptime);
	printf("\r\t\t    |          SA                     |");
	Sleep(sleeptime);
	printf("\r\t\t    |          SAI                    |");
	Sleep(sleeptime);
	printf("\r\t\t    |          SAIL                  |");
	Sleep(sleeptime);
	printf("\r\t\t    |          SAIL A                |");
	Sleep(sleeptime);
	printf("\r\t\t    |          SAIL Al               |");
	Sleep(sleeptime);
	printf("\r\t\t    |          SAIL Alg              |");
	Sleep(sleeptime);
	printf("\r\t\t    |          SAIL Algo             |");
	Sleep(sleeptime);
	printf("\r\t\t    |          SAIL Algor            |");
	Sleep(sleeptime);
	printf("\r\t\t    |          SAIL Algori           |");
	Sleep(sleeptime);
	printf("\r\t\t    |          SAIL Algorit          |");
	Sleep(sleeptime);
	printf("\r\t\t    |          SAIL Algoritm         |\n");


	printf("\t\t    |by Tong Yang & Qiaobin Fu        |");
	Sleep(sleeptime);
	printf("\r\t\t    |by Tong Yang & Qiaobin Fu        |");
	Sleep(sleeptime);
	printf("\r\t\t    | by Tong Yang & Qiaobin Fu       |");
	Sleep(sleeptime);
	printf("\r\t\t    |   by Tong Yang & Qiaobin Fu     |");
	Sleep(sleeptime);
	printf("\r\t\t    |    by Tong Yang & Qiaobin Fu    |");

	Sleep(sleeptime/2);
	printf("\r\t\t    |                                 |");
	Sleep(sleeptime/2);
	printf("\r\t\t    |    by Tong Yang & Qiaobin Fu    |");
	Sleep(sleeptime/2);
	printf("\r\t\t    |                                 |");
	Sleep(sleeptime/2);
	printf("\r\t\t    |    by Tong Yang & Qiaobin Fu    |");
	Sleep(sleeptime/2);
	printf("\r\t\t    |                                 |");
	Sleep(sleeptime/2);
	printf("\r\t\t    |    by Tong Yang & Qiaobin Fu    |");
	Sleep(sleeptime/2);
	printf("\r\t\t    |                                 |");
	Sleep(sleeptime/2);
	printf("\r\t\t    |    by Tong Yang & Qiaobin Fu    |");
	Sleep(sleeptime/2);
	printf("\r\t\t    |                                 |");
	Sleep(sleeptime/2);
	printf("\r\t\t    |    by Tong Yang & Qiaobin Fu    |\n");
	Sleep(sleeptime/2);
	printf("\r\t\t    |  Directed by Prof. Gaogang Xie  |\n");
	Sleep(sleeptime);
	printf("\r\t\t    |         In Sigcomm 2014         |\n");
	Sleep(sleeptime);
	printf("\r\t\t    |          ICT,CAS,China          |\n");
	Sleep(sleeptime);
	printf("\t\t    |-*-*-*-                 -*-*-*-*-|\n");
	Sleep(sleeptime);
	printf("\t\t    |-*-*-*-*-*-*-     -*-*-*-*-*-*-*-|\n");
}
void sailPerformanceTest(char *traffic_file, char* fib_file)
{
	//printf("\t\t\t********************************************\n");
	//printf("\t\t\t*-*-*         sail algorithm        *-*-*-*\n");
	//printf("\t\t\t*-*-*            ICT, CAS            *-*-*-*\n");
	//printf("\t\t\t********************************************\n");

	amination();

	printf("\n\nsail algorithm starts...\n\n");
	CFib tFib = CFib();
	tFib.BuildFibFromFile(fib_file);
	unsigned int *traffic=tFib.TrafficRead(traffic_file);

	register unsigned char LPMPort=0;

	LARGE_INTEGER frequence,privious,privious1;
	if(!QueryPerformanceFrequency(&frequence))return;
	QueryPerformanceCounter(&privious);
	printf("\tfrequency=%u\n",frequence.QuadPart);//2825683

	for (register int j=0;j<10000;j++)
	{
		for (register int i=0;i<TRACE_READ;i++)
		{
			LPMPort=tFib.sailLookup(traffic[i]);
		}
	}

	QueryPerformanceCounter(&privious1);
	long long Lookuptime=1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart;

	printf("\tLMPport=%d\n\tLookup time=%u\n\tThroughput is:\t %.3f Mpps\n",LPMPort,Lookuptime, 10000.0*TRACE_READ/Lookuptime);

	int updateEntryCount = BFLevelPushingTrieUpdate(updateFile, &tFib);
}




void test(int argc, char** argv)
{
	/******************************************Stage 1******************************************/
	printf("\t\tStage One: The Initial Trie Construction\n");
	//build FibTrie
	CFib tFib = CFib();

	tFib.ytGetNodeCounts();
	printf("\nThe total number of Trie node is :\t%u.\n", tFib.allNodeCount);
	printf("The total number of solid Trie node is :\t%d.\n", tFib.solidNodeCount);

	/*tFib.outputPortMapping(portMapFile1);
	tFib.OutputTrie(tFib.m_pTrie, newPortfile1, oldPortfile1);*/
	
	printf("\n***********************Trie Correct Test***********************\n");
	
	if (!tFib.isCorrectTrie(tFib.m_pTrie)) {
		printf("The trie structure is incorrect!!!\n");
	}
	else {
		printf("The trie structure is correct!\n");
	}

	tFib.ytTriePortTest(tFib.m_pTrie);
	printf("******************************End******************************\n");

	//system("pause");

	/******************************************Stage 2******************************************/
	printf("\n\n\t\tStage Two: The First Round Update\n");
	unsigned int iEntryCount = 0;
	unsigned int updateEntryCount = 0;

	iEntryCount = tFib.BuildFibFromFile(ribFile);



	tFib.ytGetNodeCounts();
	printf("\nThe total number of Trie node is :\t%u.\n",tFib.allNodeCount);
	printf("The total number of solid Trie node is :\t%d.\n", tFib.solidNodeCount);
	printf("The total number of routing items in FRib file is :\t%u.\n", iEntryCount);

	/*tFib.outputPortMapping(portMapFile2);
	tFib.OutputTrie(tFib.m_pTrie, newPortfile2, oldPortfile2);*/

	printf("\n***********************Trie Correct Test***********************\n");
	
	if (!tFib.isCorrectTrie(tFib.m_pTrie)) {
		printf("The trie structure is incorrect!!!\n");
	}
	else {
		printf("The trie structure is correct!\n");
	}

	tFib.ytTriePortTest(tFib.m_pTrie);
	printf("******************************End******************************\n");

	//tFib.checkTable(tFib.m_pTrie, 0);

	/*printf("\n************************sail Lookup Correct Test************************\n");
	sailDetectForFullIp(&tFib);
	printf("***********************************End***********************************\n");*/

	//system("pause");

	/******************************************Stage 3******************************************/
	printf("\n\n\t\tStage Three: The Second Round Update\n");
	//update FibTrie stage
	updateEntryCount = BFLevelPushingTrieUpdate(updateFile, &tFib);
	tFib.ytGetNodeCounts();
	printf("\nThe total memory access is :\t%llu.\n", tFib.memory_access);
	printf("The total number of Trie node is :\t%d.\n", tFib.allNodeCount);
	printf("The total number of solid Trie node is :\t%u.\n", tFib.solidNodeCount);
	printf("The total number of updated routing items is :\t%u.\n", updateEntryCount);
	
	/*tFib.outputPortMapping(portMapFile3);
	tFib.OutputTrie(tFib.m_pTrie, newPortfile3, oldPortfile3);*/

	printf("\n************************Trie Correct Test************************\n");
	
	if (!tFib.isCorrectTrie(tFib.m_pTrie)) {
		printf("The trie structure is incorrect!!!\n");
	}
	else {
		printf("The trie structure is correct!\n");
	}

	tFib.ytTriePortTest(tFib.m_pTrie);
	printf("*******************************End*******************************\n");

	printf("\n\n\t\tUpdate Statistics\n");
	printf("\nThe total number of true update items is :\t%u.\n", tFib.trueUpdateNum);
	printf("The total number of invalid update items is :\t%u.\n", tFib.invalid);
	printf("The detailed invalid items:\n\tinvalid0 = %u\tinvalid1 = %u\tinvalid2 = %u\n", tFib.invalid0, tFib.invalid1, tFib.invalid2);

	//system("pause");
	//tFib.checkTable(tFib.m_pTrie, 0);

	printf("\n\n************************sail Lookup Correct Test************************\n");
	sailDetectForFullIp(&tFib);
	printf("***********************************End***********************************\n");

	printf("\nMission Complete, Press any key to continue...\n");
	system("pause");
}



void main (int argc, char** argv) {

	if (argc>1)
	{
		sailPerformanceTest(argv[1], argv[2]);
	}
}