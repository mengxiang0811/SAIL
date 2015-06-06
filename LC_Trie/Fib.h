/*
 * Fib.h
 *
 */

#ifndef FIB_H_
#define FIB_H_


#define		FIBLEN			sizeof(struct TrieNode)		
#define		NEXTHOPLEN		sizeof(struct NextHop)		
#define		ZeroStr(a)		memset(a,0,sizeof(a))

#define _NOT_DELETE	0
#define _INSERT 1
#define _DELETE	2
#define _CHANGE	3

#define MAX_BLIND_COUNT	100000				

#define HIGHTBIT				2147483648				



#include	<string>
#include	<fstream>
using namespace std;


struct TrieNode
{
	TrieNode*			parent;			//		
	TrieNode*			lchild;					
	TrieNode*			rchild;					
	int					newPort;				
	int					oldPort;		//		
	bool				changedByPath;			
	bool				changedByMultibit;		
	int					skip;			
	char				segment[32];		
	int					maxLevel;			

	TrieNode **			MultiPoint;			
	bool				ifblind;		//	
	int					blindDepth;		//
};



class CFib
{
public:
	TrieNode* m_pTrie;							

#ifdef TCAM_COMPUTE
	int changeNum;
	int leafnum;
#endif

	int allNodeCount;		
	int solidNodeCount;		
	int nonRouteNum;		
	int oldNodeCount;		


	int BSCount;			
	int BZdepth[MAX_BLIND_COUNT];			
	float BZAve_depth;
	int   Max_depth;
	int BZSize;

	long MemoryVisitSum;		
	long MemoryAccessLookup;	
	long MemoryAccessLookup_BS;	



	bool ifthislookupBlind;

	CFib(void);
	~CFib(void);


	void CFib::ClearTrie(TrieNode* pTrie);


	void CreateNewNode(TrieNode* &pTrie);


	void ytGetNodeCounts();

	void Pretraversal(TrieNode* pTrie);

	void OutputTrie(TrieNode* pTrie,string sFileName,string oldPortfile);


	bool superDelete(TrieNode *deleteNode);

	void UpdateTreeNodeCount(TrieNode *insertNode);

	bool IsLeaf(TrieNode * pNode);
private:


	unsigned int GetAncestorHop(TrieNode* pTrie);


	void GetTrieHops(TrieNode* pTrie,unsigned int iVal,int iBitLen,ofstream* fout,bool ifnewPort);



public:

	unsigned int ConvertBinToIP(string sBinFile,string sIpFile);

	unsigned int ConvertIpToBin(string sIpFile,string sBinFile);

	void IpToBinary(string sIP,char sBin[32]);

	unsigned int BuildFibFromFile(string sFileName);

	void AddNode(unsigned long lPrefix,unsigned int iPrefixLen,unsigned int iNextHop);



	int CFib::lookup(TrieNode* pNode, char *ip, int cur, int port);
	int CFib::lookupNoSkip(TrieNode* pNode, char *ip, int cur, int port);
public:

	int  Iflame(TrieNode *pNode);			
	void PathCompression(TrieNode* pTrie);	

	void MarkMaxLevel(TrieNode* pTrie);		
	int  GetMaxLevel(TrieNode* pTrie);			
	void LevelTraverse(TrieNode* pTrie);	
	int  smaller(int a, int b);				

	void LevelCompression(TrieNode* pNode);	
	void CFib::LevelOrder(TrieNode *b);
	void CFib::printTest(TrieNode* pNode);


	bool CFib::pathCompare(char segment[],char insert_C[],int len);



	bool CFib::BS_Update(int insertport, char *insert_C, int operation_type);

	int CFib::GetBZSize(TrieNode* pTrie);
	int CFib::GetBZdepth(TrieNode* pTrie);

	bool CFib::ifFindBlind(TrieNode * m_trie,char * insert_C);

	bool CFib::ifSecondVisit(TrieNode * m_trie,char * insert_C);
};	


#endif /* FIB_H_ */