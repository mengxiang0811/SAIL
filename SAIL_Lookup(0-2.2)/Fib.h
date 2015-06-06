
#pragma once

#include <stdint.h>
#include <windows.h>
#include <fstream>
#include "sailTable.h"

using namespace std;

#define TRACE_READ 100000
#define _NOT_DELETE	0
#define _INSERT 1											// add by Qiaobin Fu. 2014-04-17
#define _DELETE	2
#define _CHANGE	3

#define PORT_MAX		255
#define PORT_NUM_MAX    255	// Used for port mapping
#define HIGHTBIT			2147483648				//a 32 bit binary number, whose highest bit is 1,and other bits are all 0.


struct FibTrie
{
	FibTrie*			parent;				//point to father node
	FibTrie*			lchild;					//point to the left child(0)
	FibTrie*			rchild;					//point to the right child(1)
	int					newPort;					
	int					oldPort;
	int					nodeLevel;			// Add by Qiaobin Fu
	int					chunkID;				//the chunk of the node. Add by Qiaobin Fu
	bool				ifpushed;				//if the missing node.
};

class CFib
{
public:
	FibTrie* m_pTrie;				//root node of FibTrie
	int allNodeCount;				//to count all the nodes in Trie tree, including empty node
	int solidNodeCount;				//to count all the solid nodes in Trie tree
	int LevelPort[40][200];

	unsigned int prefix32_num;		
	unsigned int lenofchain;
	
	/*********************************Trie Table*************************************/
	unsigned int currentChunkNum24;	//# of chunks in level 24
	unsigned int currentChunkNum32;	//# of chunks in level 32

	struct sailTable_16	*levelTable16;	//table in level 16
	struct sailTable_24	*levelTable24;	//table in level 24
	struct sailTable_32	*levelTable32;	//table in level 32

	struct segInfo_16		segmentUpdate16;	//the updating segment in level 16
	struct segInfo_24		segmentUpdate24;	//the updating segment in level 24
	struct segInfo_32		segmentUpdate32;	//the updating segment in level 32

	unsigned int				chunkUpdateNum24;		//# of the updating chunks in level 24
	unsigned int				chunkUpdateNum32;		//# of the updating chunks in level 32

	struct chunkInfo			chunkUpdate24[CHUNK_MAX_24];	//the updating chunks in level 24
	struct chunkInfo			chunkUpdate32[CHUNK_MAX_32];	//the updating chunks in level 32
	/************************************End****************************************/


	/********************************CDF stat************************************/
	//unsigned int CBFInsertNum;
	//unsigned int CBFDelNum;
	unsigned int trueUpdateNum;						// true total update item
	unsigned int invalid;
	unsigned int invalid0;
	unsigned int invalid1;
	unsigned int invalid2;

	unsigned int deepest;

	uint64_t memory_access;

	//unsigned int CBFInsertArray[25000];
	//unsigned int CBFDeleteArray[25000];
	/********************************End************************************/


	CFib(void);
	~CFib(void);

	// creat a new FIBTRIE ndoe 
	void CreateNewNode(FibTrie* &pTrie);
	//get the total number of nodes in RibTrie  
	void ytGetNodeCounts();
	void Pretraversal(FibTrie* pTrie);
	//output the result
	void OutputTrie(FibTrie* pTrie,string sFileName,string oldPortfile);
	void OutputTrie_32(FibTrie* pTrie);

	bool IsLeaf(FibTrie * pNode);
private:
	//get and output all the nexthop in Trie
	void CFib::GetTrieHops(FibTrie* pTrie,unsigned int iVal,int iBitLen,ofstream* fout,bool ifnewPort);
	void CFib::GetTrieHops_32(FibTrie* pTrie,unsigned int iVal,int iBitLen,ofstream* fout,bool ifnewPort);

public:
	unsigned int BuildFibFromFile(string sFileName);
    //void Update(int insertport, char *insert_C, int operation_type);
	void Update(int insertport, char *insert_C, int operation_type, char *sprefix);	// used for test. By Qiaobin Fu

	int num_level[50];

	void CFib::LevelStatistic(FibTrie* pTrie, unsigned int level);

	unsigned int CFib::btod(char *bstr);

	/*********************************Trie Table*************************************/
	void sailTableInit();
	void updateInfoInit();
	void subTrieChunkUpdate(FibTrie* root, unsigned int prefix, struct subTrieUpdateArg *arg);
	unsigned int sailLookup(unsigned int ip);
	unsigned int * TrafficRead(char *traffic_file);
	//unsigned int sailPerformanceTest(char *traffi_file, char* fib_file);
	unsigned int getBitsValue(unsigned int prefix, unsigned char prefixLen, unsigned char start, unsigned char len);

	void checkTable(FibTrie *root, unsigned int prefix);
	void trieDestroy(FibTrie *root);
	/************************************End****************************************/


	/*******************************BFstat*******************************************/
	unsigned int GetAncestorHop(FibTrie* pTrie);
	//void subTrieUpdate(FibTrie* root, unsigned int default_port, int operation);
	unsigned int subTrieUpdate(FibTrie* root, unsigned int default_port, unsigned int prefix, bool ifUpdate, struct subTrieUpdateArg *arg);	//2014-05-07, Qiaobin Fu
	void subTrieLevelPushing(FibTrie* root, unsigned int default_port, unsigned int prefix, struct subLevelPushArg* arg);
	bool isTheRange(unsigned int prefixLen, unsigned int visitedNodeNum);	// 2014-05-07, Qiaobin Fu
	void ytTriePortTest(FibTrie* pTrie);
	bool isCorrectTrie(FibTrie* pTrie);
	/********************************End*********************************************/
};


