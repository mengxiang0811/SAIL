#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fstream>
#include "lulea.h"
#include <windows.h>
#include <conio.h>

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


inline unsigned long long GetCycleCount()
{
	__asm
	{
		RDTSC
	}
}

void main(int argc, char** argv)
{
	if (argc > 1)
	{
		read_trace(argv[1]);

		start(argv[2]);

		int LPMPort=0;
		register unsigned int taffic_Integer=0;
		register int structInteger=0;
		LARGE_INTEGER frequence,privious,privious1;
		if(!QueryPerformanceFrequency(&frequence))return;
		QueryPerformanceCounter(&privious);

		

		//for (register int j=0;j<10000;j++)
		{
			for (register int i=0;i<TRACE_READ;i++)
			{
				unsigned long long counter=GetCycleCount();
				LPMPort=lookup(traffic[i]);
				unsigned long long counter1=GetCycleCount();
				printf("%u\n",counter1-counter);
			}
		}

		QueryPerformanceCounter(&privious1);
		unsigned int Lookuptime=1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart;

		printf("LPMPort=%d,",LPMPort);
		printf("Lookup time=\t%u\n",Lookuptime);
	} 
}

