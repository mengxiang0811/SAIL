/*
* Fib.cpp
*
*  Created on: 2011-4-6
*      Author: root
*	Updated on: 2014-4-18
*		By Qiaobin Fu
*/

#include "Fib.h"
#include <iostream>


#define FIBLEN				sizeof(struct FibTrie)		//size of each Trie node
#define EMPTYHOP			0							//Trie node doesnt't has a next hop
#define HIGHTBIT			2147483648					//Binary: 10000000000000000000000000000000


char * result_compare="result_compare.txt";
char * hop_count="hop_count.txt";

CFib::CFib(void)
{
	//initial the root of the Trie tree
	CreateNewNode(m_pTrie);

	allNodeCount=0;			//to count all the nodes in Trie tree, including empty node
	solidNodeCount=0;		//to count all the solid nodes in Trie tree
	prefix32_num=0;
	lenofchain=0;

	memset(num_level, 0, sizeof(num_level));
	memset(LevelPort,0,sizeof(LevelPort));

	/*****************************CDF stat Init*************************/
	trueUpdateNum=0;

	invalid = 0;
	invalid0 = 0;
	invalid1 = 0;
	invalid2 = 0;

	memory_access = 0; // 2014-05-02; Qiaobin Fu

	m_pTrie->oldPort = PORT_MAX;		// Add by Qiaobin Fu, 2014-4-19
	m_pTrie->newPort = PORT_MAX;

	/********************************End******************************/


	/*********************************Trie Table*************************************/
	sailTableInit();	// 2014-05-09


	struct subLevelPushArg arg;
	arg.bornStatus = -1;
	arg.chunkID_24 = -1;
	arg.chunkID_32 = -1;

	subTrieLevelPushing(m_pTrie, PORT_MAX, 0, &arg);
	/************************************End****************************************/
}

void CFib::updateInfoInit()
{
	unsigned int i = 0;

	chunkUpdateNum24 = 0;
	chunkUpdateNum32 = 0;

	memset(&segmentUpdate16, 0, sizeof(segmentUpdate16));
	memset(&segmentUpdate24, 0, sizeof(segmentUpdate24));
	memset(&segmentUpdate32, 0, sizeof(segmentUpdate32));

	segmentUpdate16.chunk_id = -1;
	segmentUpdate24.chunk_id = -1;
	segmentUpdate32.chunk_id = -1;

	/*cout << sizeof(segmentUpdate16) << endl;
	cout << sizeof(segmentUpdate24) << endl;
	cout << sizeof(segmentUpdate32) << endl;*/

	memset(&chunkUpdate24, 0, sizeof(chunkUpdate24));
	memset(&chunkUpdate32, 0, sizeof(chunkUpdate32));

	for (i = 0; i < CHUNK_MAX_24; i++) {
		chunkUpdate24[i].chunk_id = -1;
	}

	for (i = 0; i < CHUNK_MAX_32; i++) {
		chunkUpdate32[i].chunk_id = -1;
	}

	/*cout << sizeof(chunkUpdate24) << endl;
	cout << sizeof(chunkUpdate32) << endl;*/
}

void CFib::sailTableInit()
{
	//currentOffsetNum16  = 0;
	currentChunkNum24 = 0;
	currentChunkNum32 = 0;

	levelTable16 = (struct sailTable_16 *)calloc(1, sizeof(*levelTable16));
	levelTable24 = (struct sailTable_24 *)calloc(1, sizeof(*levelTable24));
	levelTable32 = (struct sailTable_32 *)calloc(1, sizeof(*levelTable32));

	if (!levelTable16 || !levelTable24 || !levelTable32) {
		cout << "Level tables init error! Exit..." << endl;
		system("pause");
		exit(-1);
	}

	/*memset(levelTable16, 0, sizeof(*levelTable16));
	memset(levelTable24, 0, sizeof(*levelTable24));
	memset(levelTable32, 0, sizeof(*levelTable32));*/

	/*cout << sizeof(*levelTable16) << endl;
	cout << sizeof(*levelTable24) << endl;
	cout << sizeof(*levelTable32) << endl;*/

	updateInfoInit();
}


unsigned int CFib::getBitsValue(unsigned int prefix, unsigned char prefixLen, unsigned char start, unsigned char len)
{
	if ((len > 8) || (start + len > prefixLen)) {
		return ERROR_BITS_VAL;
	}

	unsigned int val = (1 << len) - 1;
	return (val << (prefixLen - start - len));
}

void CFib::trieDestroy(FibTrie *root)
{
	if (root == NULL)
		return;
	
	trieDestroy(root->lchild);
	trieDestroy(root->rchild);

	free(root);
	root = NULL;
}

CFib::~CFib(void)
{
	if (levelTable16) {
		free(levelTable16);
		levelTable16 = NULL;
	}

	if (levelTable24) {
		free(levelTable24);
		levelTable24 = NULL;
	}

	if (levelTable32) {
		free(levelTable32);
		levelTable32 = NULL;
	}

	cout << "\tDestroying the trie......" << endl;
	trieDestroy(m_pTrie);
	cout << "\tThe trie has been destroyed!" << endl;
	printf("\nMission complete, Press any key to exit...");
}

//creat new node 
void CFib::CreateNewNode(FibTrie* &pTrie)
{
	pTrie= (struct FibTrie*)malloc(FIBLEN);

	//initial
	pTrie->parent = NULL;
	pTrie->lchild = NULL;
	pTrie->rchild = NULL;
	pTrie->newPort = EMPTYHOP;
	pTrie->oldPort = EMPTYHOP;
	pTrie->nodeLevel = 0;
	pTrie->chunkID = -1;
	pTrie->ifpushed= false;
}



unsigned int CFib::btod(char *bstr)
{
	unsigned int d = 0;
	unsigned int len = (unsigned int)strlen(bstr);
	if (len > 32)
	{
		printf("too long\n");
		return -1; 
	}
	len--;

	unsigned int i = 0;
	for (i = 0; i <= len; i++)
	{
		d += (bstr[i] - '0') * (1 << (len - i));
	}

	return d;
}


bool CFib::IsLeaf(FibTrie * pNode)
{
	if (pNode->lchild==NULL && pNode->rchild==NULL) return true;
	else return false;	
}


void CFib::Pretraversal(FibTrie* pTrie)
{
	if (NULL==pTrie) return;

	allNodeCount++;
	if (pTrie->newPort != 0) {
		solidNodeCount++;
	}


	Pretraversal(pTrie->lchild);
	Pretraversal(pTrie->rchild);
}
void CFib::ytGetNodeCounts()
{
	allNodeCount = 0;
	solidNodeCount = 0;

	Pretraversal(m_pTrie);
}

void CFib::OutputTrie(FibTrie* pTrie,string sFileName,string oldPortfile)
{
	ofstream fout(sFileName.c_str());
	GetTrieHops(pTrie,0,0,&fout,true);
	fout<<flush;
	fout.close();

	ofstream fout1(oldPortfile.c_str());
	GetTrieHops(pTrie,0,0,&fout1,false);
	fout1<<flush;
	fout1.close();
}

void CFib::OutputTrie_32(FibTrie* pTrie)
{
	ofstream fout("Prefixes_32.txt");
	GetTrieHops_32(pTrie,0,0,&fout,true);
	fout<<flush;
	fout.close();
}


void CFib::GetTrieHops_32(FibTrie* pTrie,unsigned int iVal,int iBitLen,ofstream* fout,bool ifnewPort)
{
	unsigned short portOut=PORT_MAX;

	if (-1!=pTrie->newPort)
	{
		portOut=pTrie->newPort;
	}

	if(portOut!=EMPTYHOP  && 32==iBitLen )
	{
		*fout<<iVal<<"\t"<<portOut<<endl;
	}

	iBitLen++;

	//try to handle the left sub-tree
	if(pTrie->lchild!=NULL)
	{
		GetTrieHops_32(pTrie->lchild,iVal,iBitLen,fout,ifnewPort);
	}
	//try to handle the right sub-tree
	if(pTrie->rchild!=NULL)
	{
		iVal += 1<<(32-iBitLen);
		GetTrieHops_32(pTrie->rchild,iVal,iBitLen,fout,ifnewPort);
	}
}
//get and output all the nexthop in Trie
void CFib::GetTrieHops(FibTrie* pTrie,unsigned int iVal,int iBitLen,ofstream* fout,bool ifnewPort)
{

	int portOut=-1;
	if (true==ifnewPort)
		portOut=pTrie->newPort;
	else				
		portOut=pTrie->oldPort;

	// Add by Qiaobin Fu, 2014-4-21. port mapping
	// if the mapping is on, the port should be transformed


	//1 00000000  00010000   00000000
	if(portOut!=EMPTYHOP)
	{
		char strVal[50];
		memset(strVal,0,sizeof(strVal));

		sprintf(strVal,"%d.%d.%d.%d/%d\t%d\n",(iVal>>24),(iVal<<8)>>24,(iVal<<16)>>24,(iVal<<24)>>24,iBitLen,portOut);
		*fout<<strVal;
	}

	iBitLen++;

	//try to handle the left sub-tree
	if(pTrie->lchild!=NULL)
	{
		GetTrieHops(pTrie->lchild,iVal,iBitLen,fout,ifnewPort);
	}
	//try to handle the right sub-tree
	if(pTrie->rchild!=NULL)
	{
		iVal += 1<<(32-iBitLen);
		GetTrieHops(pTrie->rchild,iVal,iBitLen,fout,ifnewPort);
	}
}


/*
*PURPOSE: construct RIB tree from file
*RETURN VALUES: number of items in rib file
*/
unsigned int CFib::BuildFibFromFile(string sFileName)
{
	unsigned int	iEntryCount=0;		//the number of items from file

	char			sPrefix[100];		//prefix from rib file
	unsigned long	lPrefix;			//the value of Prefix
	unsigned int	iPrefixLen;			//the length of PREFIX
	unsigned int	iNextHop;			//to store NEXTHOP in RIB file


	ifstream fin(sFileName.c_str());

	if (!fin) {
		fprintf(stderr, "Open file %s error!\n", sFileName.c_str());
		return 0;
	}

	while (!fin.eof()) {			// Add by Qiaobin Fu 2014-4-22

		lPrefix = 0;
		iPrefixLen = 0;
		iNextHop = EMPTYHOP;

		memset(sPrefix,0,sizeof(sPrefix));

		fin >> sPrefix>> iNextHop;

		int iStart=0;				//the start point of PREFIX
		int iEnd=0;					//the start point of PREFIX
		int iFieldIndex = 3;		
		int iLen=(int)strlen(sPrefix);	//The length of PREFIX

		if (iLen>20)
		{
			continue;//maybe IPv6 address
		}

		if(iLen > 0) {
			iEntryCount++;
			for ( int i=0; i<iLen; i++ ) {
				//get the first three sub-items
				if ( sPrefix[i] == '.' ) {
					iEnd = i;
					string strVal(sPrefix+iStart,iEnd-iStart);
					lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex);
					iFieldIndex--;
					iStart = i+1;
					i++;
				}
				if ( sPrefix[i] == '/' ) {
					//get the prefix length
					iEnd = i;
					string strVal(sPrefix+iStart,iEnd-iStart);
					lPrefix += atol(strVal.c_str());
					iStart = i+1;

					i++;
					strVal= string(sPrefix+iStart,iLen-1);
					iPrefixLen=atoi(strVal.c_str());
				}
			}

			char insert_B[50];
			memset(insert_B,0,sizeof(insert_B));
			//insert the current node into Trie tree
			for (unsigned int yi = 0; yi < iPrefixLen; yi++)
			{
				//turn right
				if(((lPrefix << yi) & HIGHTBIT) == HIGHTBIT) insert_B[yi]='1';
				else insert_B[yi]='0';
			}
			//printf("%s\/%d\t%d\n",insert_C,iPrefixLen,iNextHop);

			if (iPrefixLen < 8) {
				//printf("%d-%d; ", iPrefixLen, iNextHop);
			}
			else
			{
				Update(iNextHop, insert_B, _INSERT, sPrefix);
			}
		}
	}

	fin.close();

	return iEntryCount;
}

void CFib::subTrieChunkUpdate(FibTrie* root, unsigned int prefix, struct subTrieUpdateArg *arg)
{
	int level = root->nodeLevel;
	if (arg->segLevel == 16) {
		if (level == 16) {
			segmentUpdate16.chunk_id = root->chunkID;
			if (IsLeaf(root)) {
				segmentUpdate16.nexthop[prefix & 255] = (0 - root->newPort);
			}
			else {
				segmentUpdate16.nexthop[prefix & 255] = levelTable16->element[prefix];
			}
		}
		else if (level == 24) {
			if (chunkUpdate24[chunkUpdateNum24].chunk_id != root->chunkID) {
				chunkUpdateNum24++;

				if (chunkUpdateNum24 > CHUNK_MAX_24) {
					printf("The number of updated chunks in level 24 is too large!!!\n");
					system("pause");
				}
				chunkUpdate24[chunkUpdateNum24].chunk_id = root->chunkID;
			}
			chunkUpdate24[chunkUpdateNum24].nexthop[(prefix & 255)] = root->newPort;
		}
		else if (level == 32){
			if (chunkUpdate32[chunkUpdateNum32].chunk_id != root->chunkID) {
				chunkUpdateNum32++;

				if (chunkUpdateNum32 > CHUNK_MAX_32) {
					printf("The number of updated chunks in level 32 is too large!!!\n");
					system("pause");
				}

				chunkUpdate32[chunkUpdateNum32].chunk_id = root->chunkID;
			}
			chunkUpdate32[chunkUpdateNum32].nexthop[(prefix & 255)] = root->newPort;
		}
	}
	else if (arg->segLevel == 24) {
		if (level == 24) {
			segmentUpdate24.chunk_id = root->chunkID;
			segmentUpdate24.nexthop[prefix & 255] = root->newPort;
		}
		else if (level == 32) {
			if (chunkUpdate32[chunkUpdateNum32].chunk_id != root->chunkID) {
				chunkUpdateNum32++;

				if (chunkUpdateNum32 > CHUNK_MAX_32) {
					printf("The number of updated chunks in level 32 is too large!!!\n");
					system("pause");
				}

				chunkUpdate32[chunkUpdateNum32].chunk_id = root->chunkID;
			}
			chunkUpdate32[chunkUpdateNum32].nexthop[(prefix & 255)] = root->newPort;
		}
	}
	else if (arg->segLevel == 32){
		if (level == 32) {
			segmentUpdate32.chunk_id = root->chunkID;
			segmentUpdate32.nexthop[prefix & 255] = root->newPort;
		}
	}
}

unsigned int CFib::subTrieUpdate(FibTrie* root, unsigned int default_port, unsigned int prefix, bool ifUpdate, struct subTrieUpdateArg *arg) {
	int level = root->nodeLevel;
	unsigned int updateNum = 0;

	unsigned int lPrefix = prefix << 1;
	unsigned int rPrefix = (prefix<< 1) + 1;

	if(ifUpdate) {	//update
		if (level == 16 || level == 24 || level == 32) {	
			if (IsLeaf(root)) {
				subTrieChunkUpdate(root, prefix, arg);
				return 1;
			}
			else {
				updateNum = 1;
				root->newPort = 0;
				subTrieChunkUpdate(root, prefix, arg);
			}
		}

		root->newPort = 0;

		if (root->lchild != NULL) {	
			if (root->lchild->oldPort == 0) {
				root->lchild->newPort = default_port;		
				updateNum += subTrieUpdate(root->lchild, default_port, lPrefix, true, arg); 
			}
			else {
				updateNum += subTrieUpdate(root->lchild, root->newPort, lPrefix, false, arg);	
			}
		}

		if (root->rchild != NULL) {	
			if (root->rchild->oldPort == 0) {
				root->rchild->newPort = default_port;		
				updateNum += subTrieUpdate(root->rchild, default_port, rPrefix, true, arg);
			}
			else {
				updateNum += subTrieUpdate(root->rchild, root->newPort, rPrefix, false, arg);
			}
		}
	}
	else {
		if (level == 16 || level == 24 || level == 32) {
			subTrieChunkUpdate(root, prefix, arg);
			return 1;
		}

		if (root->lchild != NULL) {
			updateNum += subTrieUpdate(root->lchild, root->newPort, lPrefix, false, arg);
		}

		if (root->rchild != NULL) {
			updateNum += subTrieUpdate(root->rchild, root->newPort, rPrefix, false, arg);
		}
	}

	return updateNum;
}

void CFib::subTrieLevelPushing(FibTrie* pTrie, unsigned int default_port, unsigned int prefix, struct subLevelPushArg* arg) {

	int level = 0;

	if(NULL == pTrie) return;

	memory_access ++;

	level = pTrie->nodeLevel;

	unsigned int lPrefix = prefix << 1;
	unsigned int rPrefix = (prefix << 1) + 1;

	if (level == 16) {	//level 16
		if (pTrie->chunkID == -1) {	//get the chunk id
			pTrie->chunkID = prefix >> 8;
		}

		if (IsLeaf(pTrie)) {	//leaf node
			levelTable16->element[prefix] = (0 - pTrie->newPort);
			return;
		}
	}
	else if (level == 24) {
		if (arg->bornStatus == NB_16 || arg->bornStatus == NB_BOTH) {
			if (pTrie->chunkID != -1) {
				printf("Error chunk id in level 24!!!\n");
			}
			pTrie->chunkID = arg->chunkID_24;

	
			if (IsLeaf(pTrie)) {
				chunkUpdate24[0].nexthop[prefix & 255] = pTrie->newPort;
				return;
			}
		}
	}
	else if (level == 32) {
		if (pTrie->chunkID != -1) {
			printf("Error chunk id in level 32!!!\n");
		}

		pTrie->chunkID = arg->chunkID_32;
		chunkUpdate32[0].nexthop[prefix & 255] = pTrie->newPort;
		return;
	}

	if((level == 16 || level == 24 || level == 32) && IsLeaf(pTrie)) {
		return;
	}

	if (pTrie->newPort > 0) {
		default_port = pTrie->newPort;
	}

	memory_access += 2;
	//left child
	if (NULL== pTrie->lchild)
	{
		FibTrie* pTChild;
		CreateNewNode(pTChild);

		if (NULL==pTChild)
		{
			printf("malloc failed");
		}
		pTChild->parent = pTrie;
		pTChild->lchild = NULL;
		pTChild->rchild = NULL;
		pTChild->oldPort = 0;
		pTChild->newPort = pTrie->newPort;
		pTChild->nodeLevel = pTrie->nodeLevel + 1;
		pTChild->chunkID = -1;
		pTChild->ifpushed = true;
		pTrie->lchild = pTChild;
	}
	else if (0 == pTrie->lchild->newPort) {
		pTrie->lchild->newPort = default_port;
	}

	//right child
	if (NULL == pTrie->rchild)
	{
		FibTrie* pTChild;
		CreateNewNode(pTChild);

		if (NULL==pTChild)
		{
			printf("malloc failed");
		}
		pTChild->parent = pTrie;
		pTChild->lchild = NULL;
		pTChild->rchild = NULL;
		pTChild->oldPort = 0;
		pTChild->newPort = pTrie->newPort;
		pTChild->nodeLevel = pTrie->nodeLevel + 1;
		pTChild->chunkID = -1;
		pTChild->ifpushed = true;
		pTrie->rchild = pTChild;
	}
	else if (0 == pTrie->rchild->newPort) {
		pTrie->rchild->newPort = default_port;
	}

	pTrie->newPort = 0;

	subTrieLevelPushing(pTrie->lchild, default_port, lPrefix, arg);
	subTrieLevelPushing(pTrie->rchild, default_port, rPrefix, arg);
}

bool CFib::isTheRange(unsigned int prefixLen, unsigned int visitedNodeNum)
{
	unsigned int basicNum = 0;

	if (prefixLen <= 16) {
		basicNum = (1 << (16 - prefixLen));

		if (visitedNodeNum < basicNum) {
			return false;
		}

		if (visitedNodeNum == basicNum) {
			return true;
		}
		else if ((visitedNodeNum - basicNum) % (1 << 8) == 0) {
			return true;
		}
		else {
			return false;
		}
	}
	else if (prefixLen <= 24) {
		basicNum = (1 << (24 - prefixLen));

		if (visitedNodeNum < basicNum) {
			return false;
		}

		if (visitedNodeNum == basicNum) {
			return true;
		}
		else if ((visitedNodeNum - basicNum) % (1 << 8) == 0) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		basicNum = (1 << (32 - prefixLen));

		if (visitedNodeNum == basicNum) {
			return true;
		}
		else {
			return false;
		}
	}
}

void CFib::Update(int insertport, char *insert_C, int operation_type, char* sprefix)
{
	//indicating to insertion, changing or deletion.
	int operation = -9;


	FibTrie *insertNode= m_pTrie;
	memory_access ++;

	FibTrie *levelNode16 = NULL;
	FibTrie *levelNode24 = NULL;

	int default_oldport = 0;

	int prefixLen = (int)strlen(insert_C);

	if (prefixLen < 8) {	
		return;
	}

	if (insertport > PORT_RANGE) {
	//if (insertport > 32767) {
		return;
	}

	int bornStatus = -1;
	bool IfNewBornNode = false;
	bool ifSubTrieUpdate = false;
	//look up the location of the current node
	for (int i=0;i< prefixLen; i++)	//0.0.0.0/0£»if len is 0, it is root node.
	{
		memory_access ++;

		if ('0' == insert_C[i])
		{
			if (NULL== insertNode->lchild)
			{//turn left, if left child is empty, create new node
				if(_DELETE == operation_type)
				{
					invalid++;
					invalid0++;
					return;
				}

				IfNewBornNode = true;
				FibTrie* pNewNode;
				CreateNewNode(pNewNode);
				pNewNode->parent = insertNode;
				pNewNode->nodeLevel = insertNode->nodeLevel + 1;		// By Qiaobin Fu

				if (pNewNode->nodeLevel == 17) {
					bornStatus = NB_16;
				}
				else if (pNewNode->nodeLevel == 25) {
					if (bornStatus == NB_16) {
						bornStatus = NB_BOTH;
					}
					else {
						bornStatus = NB_24;
					}
				}

				insertNode->lchild = pNewNode;
			}

			if (insertNode->oldPort != 0) default_oldport = insertNode->oldPort;
			insertNode = insertNode->lchild;
		}
		else
		{
			//turn right, if left child is empty, create new node
			if (NULL== insertNode->rchild)
			{
				if(_DELETE == operation_type)	
				{
					invalid++;
					invalid0++;
					return;
				}

				IfNewBornNode = true;
				FibTrie* pNewNode;
				CreateNewNode(pNewNode);
				pNewNode->parent = insertNode;
				pNewNode->nodeLevel = insertNode->nodeLevel + 1;		// 2014-4-18

				if (pNewNode->nodeLevel == 17) {
					bornStatus = NB_16;
				}
				else if (pNewNode->nodeLevel == 25) {
					if (bornStatus == NB_16) {
						bornStatus = NB_BOTH;
					}
					else {
						bornStatus = NB_24;
					}
				}

				insertNode->rchild = pNewNode;
			}

			if (insertNode->oldPort !=0 ) default_oldport = insertNode->oldPort;
			insertNode = insertNode->rchild;
		}

		if (insertNode->nodeLevel == 16) {
			levelNode16 = insertNode;
		}

		if (insertNode->nodeLevel == 24) {
			levelNode24 = insertNode;
		}
	}

	if(_DELETE != operation_type)		// it is not deletion
	{
		if (0 == insertNode->oldPort) {		//insertion
			operation = _INSERT;
		}
		else if (insertNode->oldPort == insertport)		// insertion port is the same as the original.
		{
			invalid++;
			invalid1++;	// TODO nothing
			return;
		}
		else {	//changing
			operation = _CHANGE;
		}
	}
	else if (0 == insertNode->oldPort)	{	//Withdraw a prefix which is absent
		invalid++;
		invalid2++;	// TODO nothing
		return;
	}
	else	{	//deletion
		operation = _DELETE;
	}


	int updateNumber = 0; 
	unsigned int origPrefix = btod(insert_C);

	unsigned int prefixLevel16 = 0;
	unsigned int prefixLevel24 = 0;
	unsigned int offset24 = 0;

	if (prefixLen >= 16) {
		prefixLevel16 = origPrefix >> (prefixLen - 16);
	}

	if (prefixLen >= 24) {
		prefixLevel24 = origPrefix >> (prefixLen - 24);
		offset24 = prefixLevel24 & 255;
	}

	struct subTrieUpdateArg stuArg;

	if (prefixLen <= 16) {
		stuArg.segLevel = 16;
	}
	else if (prefixLen <= 24) {
		stuArg.segLevel = 24;
	}
	else {
		stuArg.segLevel = 32;
	}

	stuArg.length = stuArg.segLevel - prefixLen;

	//printf("%s:\t%d\t%d\n", sprefix, insertport, stuArg.length);

	trueUpdateNum++;

	updateInfoInit();


	if (operation == _INSERT) {

		insertNode->oldPort = insertport;
		insertNode->newPort = insertport;

		if (IfNewBornNode) {		//beyond the outline
			struct subLevelPushArg sArg;
			sArg.bornStatus = bornStatus;
			sArg.chunkID_24 = -1;
			sArg.chunkID_32 = -1;

			if (levelTable16->element[prefixLevel16] < 0) {	

				if (bornStatus == NB_24 || bornStatus == -1) {
					printf("born status error!!!\n");
				}

				levelTable16->element[prefixLevel16] = currentChunkNum24;	
				currentChunkNum24++;	

				if (currentChunkNum24 > LEVEL24_CHUNK_NUM) {
					printf("Too large currentChunkNum24!!!\n");
					system("pause");
				}
			}

			sArg.chunkID_24 = levelTable16->element[prefixLevel16];	
			chunkUpdate24[0].chunk_id = sArg.chunkID_24;

			if (bornStatus == NB_24 || bornStatus == NB_BOTH) {	//if in level 25-32, then creat new chunk
				levelTable24->offset[(sArg.chunkID_24 << 8) + offset24] = currentChunkNum32;
				levelTable24->nexthop[(sArg.chunkID_24 << 8) + offset24] = 0;
				sArg.chunkID_32 = currentChunkNum32;
				chunkUpdate32[0].chunk_id = sArg.chunkID_32;
				currentChunkNum32++;

				if (currentChunkNum32 > LEVEL32_CHUNK_NUM) {
					printf("Too large currentChunkNum32!!!\n");
					system("pause");
				}
			}

			if (levelNode16 && levelNode16->newPort != 0) {
				subTrieLevelPushing(levelNode16, levelNode16->newPort, prefixLevel16, &sArg);
			}
			else if (levelNode24 && levelNode24->newPort != 0) {
				subTrieLevelPushing(levelNode24, levelNode24->newPort, prefixLevel24, &sArg);
			}
			else {//do level-pushing
				subTrieLevelPushing(insertNode, insertNode->oldPort, origPrefix, &sArg);
				printf("newBorneNode Error!!!\n");
			}

			if (bornStatus == NB_16 || bornStatus == NB_BOTH) {
				memcpy(&levelTable24->nexthop[chunkUpdate24[0].chunk_id << 8], chunkUpdate24[0].nexthop, sizeof(chunkUpdate24[0].nexthop));
			}

			if (bornStatus == NB_24 || bornStatus == NB_BOTH) {
				memcpy(&levelTable32->nexthop[chunkUpdate32[0].chunk_id << 8], chunkUpdate32[0].nexthop, sizeof(chunkUpdate32[0].nexthop));
			}
		}
		else if (IsLeaf(insertNode) && (insertNode->nodeLevel == 16 || insertNode->nodeLevel == 24 || insertNode->nodeLevel == 32))  {//Ò¶×Ó½Úµã
			updateNumber = 1;			// leaf node

			if(insertNode->nodeLevel == 16) {
				levelTable16->element[prefixLevel16] = (0 - insertport);
			}
			else if (insertNode->nodeLevel == 24) {
				levelTable24->nexthop[(levelTable16->element[prefixLevel16] << 8) + offset24] = insertport;
			}
			else {
				levelTable32->nexthop[(levelTable24->offset[(levelTable16->element[prefixLevel16] << 8) + offset24] << 8) + (origPrefix & 255)] = insertport;
			}
		}
		else if (!IsLeaf(insertNode)) {		// internal node
			ifSubTrieUpdate = true;
			updateNumber = subTrieUpdate(insertNode, insertport, origPrefix, true, &stuArg);		//updating the sub-trie
		}
		else {
			fprintf(stderr, "Error!!!\n");
		}
	}
	else if (operation == _CHANGE) {			//	changing operation
		insertNode->oldPort = insertport;
		insertNode->newPort = insertport;
		ifSubTrieUpdate = true;
		updateNumber = subTrieUpdate(insertNode, insertport, origPrefix, true, &stuArg);		//updating the sub-trie
	}
	else if (operation == _DELETE) {			// deletion
		insertNode->oldPort = 0;
		insertNode->newPort = default_oldport;
		ifSubTrieUpdate = true;
		//printf("nodeLevel = %d\t default_newport = %d\n", insertNode->nodeLevel, default_newport);
		updateNumber = subTrieUpdate(insertNode, default_oldport, origPrefix, true, &stuArg);		//updating the sub-trie
	}

	if (ifSubTrieUpdate) {
		if (stuArg.segLevel == 16) {
			memcpy(&levelTable16->element[origPrefix << stuArg.length], &segmentUpdate16.nexthop[(origPrefix << stuArg.length) & 255], (sizeof(short) << stuArg.length));

			for (unsigned int i = 1; i <= chunkUpdateNum24; i++) {
				memcpy(&levelTable24->nexthop[chunkUpdate24[i].chunk_id << 8], chunkUpdate24[i].nexthop, sizeof(chunkUpdate24[i].nexthop));
			}

			for (unsigned int i = 1; i <= chunkUpdateNum32; i++) {
				memcpy(&levelTable32->nexthop[chunkUpdate32[i].chunk_id << 8], chunkUpdate32[i].nexthop, sizeof(chunkUpdate32[i].nexthop));
			}
		}
		else if (stuArg.segLevel == 24) {
			memcpy(&levelTable24->nexthop[(segmentUpdate24.chunk_id << 8) + ((origPrefix << stuArg.length) & 255)], &segmentUpdate24.nexthop[(origPrefix << stuArg.length) & 255], (PORT_LEN << stuArg.length));

			for (unsigned int i = 1; i <= chunkUpdateNum32; i++) {
				memcpy(&levelTable32->nexthop[chunkUpdate32[i].chunk_id << 8], chunkUpdate32[i].nexthop, sizeof(chunkUpdate32[i].nexthop));
			}
		}
		else {
			memcpy(&levelTable32->nexthop[(segmentUpdate32.chunk_id << 8) + ((origPrefix << stuArg.length) & 255)], &segmentUpdate32.nexthop[(origPrefix << stuArg.length) & 255], (PORT_LEN << stuArg.length));
		}
	}

	if (!IfNewBornNode && (updateNumber != ((1 << stuArg.length) + (chunkUpdateNum24 << 8) + (chunkUpdateNum32 << 8)))) {
		cout << "Update Error!\tType:" << operation_type << "\t" << sprefix << "\t" << insertport<<"\t" << updateNumber << ":\t" << (1 << stuArg.length) << "\t"<< (chunkUpdateNum24 << 8) << "\t" << (chunkUpdateNum32 << 8)<< endl;
	}

	if (!IfNewBornNode && !isTheRange(prefixLen, updateNumber)) {	//judging whether the number of updating is correct
		cout << sprefix << ":	"<< updateNumber << endl;
	}
}

unsigned int CFib::sailLookup(unsigned int ip)
{
	register unsigned short offset16 = ip >> 16;
	register unsigned int LPMPort=0;

	if (levelTable16->element[offset16] < 0)	LPMPort = (0 - levelTable16->element[offset16]);
	else if(LPMPort = levelTable24->nexthop[(levelTable16->element[offset16] << 8) + (ip<<16>>24)]);
	else LPMPort = levelTable32->nexthop[(levelTable24->offset[(levelTable16->element[offset16] << 8) + (ip<<16>>24)] << 8) + (ip & 255)];

	return LPMPort;
}

unsigned int * CFib::TrafficRead(char *traffic_file)
{
	unsigned int *traffic=new unsigned int[TRACE_READ];
	int return_value=-1;
	unsigned int traceNum=0;

	//first read the trace...
	ifstream fin(traffic_file);
	if (!fin)return 0;
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
	printf("    trace read complete...\n");

	if (TraceLine<TRACE_READ)
	{
		printf("not enough\n",TraceLine);
	}

	return traffic;
}


void CFib::ytTriePortTest(FibTrie* pTrie)
{
	int level = pTrie->nodeLevel;

	// internal node, newport != 0
	if (!IsLeaf(pTrie) && pTrie->newPort != 0) {
		printf("Error:\tThe newport of an Internal node is %d\n", pTrie->newPort);
	}

	// leaf node, newport == 0
	if((level == 16  || level == 24 || level == 32) && IsLeaf(pTrie) && pTrie->newPort == 0) {
		printf("The leaf node with newport = 0\n");
	}

	// oldport!=newport
	if (pTrie->oldPort != 0 && pTrie->newPort != 0 && pTrie->oldPort != pTrie->newPort) {
		printf("Error:\toldport = %d\t newport = %d\n", pTrie->oldPort, pTrie->newPort);
	}

	if (pTrie->lchild) {
		ytTriePortTest(pTrie->lchild);
	}

	if (pTrie->rchild) {
		ytTriePortTest(pTrie->rchild);
	}
}

bool CFib::isCorrectTrie(FibTrie *pTrie)
{
	if (pTrie == NULL)
		return false;

	int level = pTrie->nodeLevel;

	if (IsLeaf(pTrie)) {	//if is leaf, must in level 16, 24, or 32
		if (level == 16 || level == 24 || level == 32) {
			return true;
		}
		else {
			return false;
		}
	}
	else {	//non-leaf
		if (!pTrie->rchild || !pTrie->lchild) {	// missing a child node
			return false;
		}
		else if (isCorrectTrie(pTrie->rchild) && isCorrectTrie(pTrie->lchild)) {	
			return true;
		}
		else
			return false;
	}
}


void CFib::LevelStatistic(FibTrie* pTrie, unsigned int level)
{
	if(NULL == pTrie)return;
	if(pTrie->newPort != 0)num_level[level]++;

	LevelStatistic(pTrie->lchild, level+1);
	LevelStatistic(pTrie->rchild, level+1);
}

void CFib::checkTable(FibTrie *root, unsigned int prefix)
{
	unsigned int lPrefix = prefix << 1;
	unsigned int rPrefix = (prefix << 1) + 1;

	if (root->nodeLevel == 16) {	//level 16
		
		if (root->chunkID != (prefix >> 8)) {	//chunk id
			printf("level 16 node ChunkID Error!!!\n");
			return;
		}

		if (IsLeaf(root)) {	//leaf node
			if (levelTable16->element[(root->chunkID << 8) + (prefix & 255)] != (0 - root->newPort)) {	//nexthop information
				printf("LevelTable16 nexthop Error!!!%d\t%d\n", levelTable16->element[(root->chunkID << 8) + (prefix & 255)], root->newPort);
				return;
			}
		}
		else if (levelTable16->element[(root->chunkID << 8) + (prefix & 255)] < 0) { //offset
			printf("LevelTable16 offset Error!!!\n");
			return;
		}
	}
	else if (root->nodeLevel == 24) {	//level 24
		if (root->chunkID != levelTable16->element[prefix >> 8]) {	// chunk id
			printf("level 24 node ChunkID Error!!!%d\t%d\t%d\n", prefix, root->chunkID, levelTable16->element[prefix >> 8]);
			return;
		}

		if (IsLeaf(root)) {	//leaf node
			if (levelTable24->nexthop[(root->chunkID << 8) + (prefix & 255)] != root->newPort || levelTable24->offset[(root->chunkID << 8) + (prefix & 255)] != 0) {
				printf("LevelTable24 nexthop or offset Error!!!\n");
				return;
			}
		}
		else if (levelTable24->nexthop[(root->chunkID << 8) + (prefix & 255)] != 0) {
			printf("LevelTable24 nexthop Error!!!\n");
			return;
		}
	}
	else if (root->nodeLevel == 32) {	//level 32
		if (root->chunkID != levelTable24->offset[(levelTable16->element[prefix >> 16] << 8) + (prefix << 16 >> 24)]) {
			printf("level 32 node ChunkID Error!!!\n");
			return;
		}

		if (levelTable32->nexthop[(root->chunkID << 8) + (prefix & 255)] != root->newPort) {
			printf("LevelTable32 nexthop Error!!!\n");
			return;
		}
	}
	
	if (root->lchild) {
		checkTable(root->lchild, lPrefix);
	}

	if (root->rchild) {
		checkTable(root->rchild, rPrefix);
	}
}