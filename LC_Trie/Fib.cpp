#include "Fib.h"
#include <iostream>
#include <math.h>

#define NONROUTE_PORT -1

#define FIBLEN				sizeof(struct TrieNode)	
#define EMPTYHOP			0					
#define HIGHTBIT			2147483648			


#define LEFT_ONLY	1	
#define RIGHT_ONLY	2	

char * result_compare="result_compare.txt";
char * hop_count="hop_count.txt";
CFib::CFib(void)
{

	CreateNewNode(m_pTrie);

#ifdef TCAM_COMPUTE
	changeNum=0;
	leafnum=0;
	memorycount=0;
#endif
	allNodeCount=0;			
	solidNodeCount=0;		
	nonRouteNum=0;


	BSCount=0;
	memset(BZdepth,0,sizeof(BZdepth));
	BZAve_depth=0.0;
	Max_depth=0;
	BZSize=0;
	ifthislookupBlind=0;
	MemoryVisitSum=0;
	MemoryAccessLookup=0;
}

CFib::~CFib(void)
{
}


void CFib::ClearTrie(TrieNode* pTrie)
{

}

void CFib::CreateNewNode(TrieNode* &pNode)
{

	pNode= (struct TrieNode*)malloc(FIBLEN);

	pNode->parent = NULL;
	pNode->lchild = NULL;
	pNode->rchild = NULL;
	pNode->newPort = EMPTYHOP;
	pNode->oldPort = EMPTYHOP;
	pNode->changedByMultibit=false;
	pNode->changedByPath=false;
	pNode->skip=0;
	ZeroStr(pNode->segment);
	pNode->maxLevel=0;

	pNode->MultiPoint=NULL;
	pNode->ifblind=false;
	pNode->blindDepth=0;
}

bool CFib::IsLeaf(TrieNode * pNode)
{
	if (pNode->lchild==NULL && pNode->rchild==NULL)return true;
	else return false;	
}


unsigned int CFib::GetAncestorHop(TrieNode* pTrie)
{
	unsigned int iHop = EMPTYHOP;
	if(pTrie != NULL){
		pTrie=pTrie->parent;
		if(pTrie!=NULL){
			iHop = pTrie->newPort;
			if(iHop==EMPTYHOP){
				iHop=GetAncestorHop(pTrie);
			}
		}
	}
	return iHop;
}

void CFib::Pretraversal(TrieNode* pTrie)
{
	if (NULL==pTrie)return;

	allNodeCount++;
	if (pTrie->newPort!=0)solidNodeCount++;
	if (-1==pTrie->newPort)nonRouteNum++;
	if (pTrie->oldPort!=0)oldNodeCount++;	

	if (true==pTrie->ifblind)
	{
		BZdepth[BSCount]=GetBZdepth(pTrie);
		BSCount++;
		BZSize+=GetBZSize(pTrie)-1;
	}

	Pretraversal(pTrie->lchild);
	Pretraversal(pTrie->rchild);
}
void CFib::ytGetNodeCounts()
{
	allNodeCount=0;
	solidNodeCount=0;
	nonRouteNum=0;
	oldNodeCount=0;
	BSCount=0;
	BZSize=0;

	memset(BZdepth,0,sizeof(BZdepth));
	Pretraversal(m_pTrie);

	float depthSum=0.0;
	for (int i=0;i<BSCount;i++)
	{
		depthSum+=BZdepth[i];
		if (BZdepth[i]>Max_depth)Max_depth=BZdepth[i];
	}

	BZAve_depth=depthSum/BSCount;
}

int CFib::GetBZSize(TrieNode* pTrie)
{
	if (NULL==pTrie)return 0;
	int lsize=GetBZSize(pTrie->lchild);
	int rsize=GetBZSize(pTrie->rchild);

	return lsize+rsize + pTrie->skip +1;
}

int CFib::GetBZdepth(TrieNode* pTrie)
{
	if (NULL==pTrie)return 0;
	if (NULL==pTrie->lchild && NULL==pTrie->rchild)return 0;

	int lheight=GetBZdepth(pTrie->lchild);
	int rheight=GetBZdepth(pTrie->rchild);

	return (lheight+ pTrie->skip +1) > (rheight + pTrie->skip +1) ? (lheight+ pTrie->skip +1) : (rheight + pTrie->skip +1);
}

void CFib::OutputTrie(TrieNode* pTrie,string sFileName,string oldPortfile)
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

void CFib::GetTrieHops(TrieNode* pTrie,unsigned int iVal,int iBitLen,ofstream* fout,bool ifnewPort)
{
	int portOut=-1;
	if (true==ifnewPort)
		portOut=pTrie->newPort;
	else				
		portOut=pTrie->oldPort;

	if(portOut!=EMPTYHOP || !IsLeaf(pTrie))
	{
		char strVal[100];
		memset(strVal,0,sizeof(strVal));
		if (0!=pTrie->skip)sprintf(strVal,"%d.%d.%d.%d/%d\t%d\t%d\t%s\t%d\n",(iVal>>24),(iVal<<8)>>24,(iVal<<16)>>24,(iVal<<24)>>24,iBitLen,portOut,pTrie->skip,pTrie->segment,pTrie->maxLevel);
		else			   sprintf(strVal,"%d.%d.%d.%d/%d\t%d\t%d\t%s\t%d\n",(iVal>>24),(iVal<<8)>>24,(iVal<<16)>>24,(iVal<<24)>>24,iBitLen,portOut,pTrie->skip,"*",pTrie->maxLevel);

		*fout<<strVal;
	}

	if (-1==pTrie->newPort)
	{
		nonRouteNum++;
	}

	iBitLen++;

	if(pTrie->lchild!=NULL)
	{
		GetTrieHops(pTrie->lchild,iVal,iBitLen,fout,ifnewPort);
	}
	if(pTrie->rchild!=NULL)
	{
		iVal += 1<<(32-iBitLen);
		GetTrieHops(pTrie->rchild,iVal,iBitLen,fout,ifnewPort);
	}
}




bool CFib::superDelete(TrieNode *deleteNode)
{
	TrieNode *fatherNode=deleteNode->parent;
	if (	0==fatherNode->lchild->newPort && 0==fatherNode->lchild->oldPort
	  && NULL==fatherNode->lchild->lchild && NULL==fatherNode->lchild->rchild
		 && 0==fatherNode->rchild->newPort  && 0==fatherNode->rchild->oldPort
	  && NULL==fatherNode->rchild->lchild && NULL==fatherNode->rchild->rchild
	 )
	{
		free(fatherNode->lchild);
		free(fatherNode->rchild);
		fatherNode->lchild=fatherNode->rchild=NULL;
		superDelete(fatherNode);
		return true;
	}
	return false;
}

void CFib::UpdateTreeNodeCount(TrieNode *insertNode)
{
	if (NULL==insertNode)return;
	else
	{
		UpdateTreeNodeCount(insertNode->rchild);
		UpdateTreeNodeCount(insertNode->lchild);
	}
}


unsigned int CFib::ConvertBinToIP(string sBinFile,string sIpFile)
{

	char			sBinPrefix[32];		
	string			strIpPrefix;		
	unsigned int	iPrefixLen;		
	unsigned int	iNextHop;		
	unsigned int	iEntryCount=0;		

	ofstream fout(sIpFile.c_str());

	ifstream fin(sBinFile.c_str());
	while (!fin.eof()) {
		iNextHop = 0;

		memset(sBinPrefix,0,sizeof(sBinPrefix));
		fin >> sBinPrefix>> iNextHop;

		if(iNextHop != 0){
			string strBin(sBinPrefix);
			iPrefixLen=strBin.length();
			strBin.append(32-iPrefixLen,'0');

			strIpPrefix="";
			for(int i=0; i<32; i+=8){		
				int iVal=0;
				string strVal=strBin.substr(i,8);

				for(int j=7;j>=0;j--){
					if(strVal.substr(j,1)=="1"){
						iVal+=(1<<(7-j));
					}
				}

				char buffer[5];
				memset(buffer,0,sizeof(buffer));
				sprintf(buffer,"%d",iVal);
				strVal=string(buffer);

				strIpPrefix += strVal;
				if(i<24)
				{
					strIpPrefix += ".";
				}
				strVal="";
			}
			fout<<strIpPrefix<<"/"<<iPrefixLen<<" "<<iNextHop<<endl;
		}
	}

	fin.close();

	fout<<flush;
	fout.close();

	return iEntryCount;
}


unsigned int CFib::ConvertIpToBin(string sIpFile,string sBinFile)
{
	char			sBinPrefix[32];		
	string			strIpPrefix;	
	unsigned int	iPrefixLen;		
	unsigned int	iNextHop;			
	unsigned int	iEntryCount=0;	

	char			sPrefix[20];	

	int				skipRead;		
	char			segRead[50];
	int				levelRead;

	ofstream fout(sBinFile.c_str());

	ifstream fin(sIpFile.c_str());
	while (!fin.eof()) {

		iPrefixLen = 0;
		iNextHop = EMPTYHOP;

		memset(sPrefix,0,sizeof(sPrefix));
		ZeroStr(segRead);
		fin >> sPrefix>> iNextHop >> skipRead >> segRead >>levelRead;

		int iLen=strlen(sPrefix);

		if(iLen>0)
		{
			iEntryCount++;
			for ( int i=0; i<iLen; i++ )
			{
				if ( sPrefix[i] == '/' )
				{
					string strVal(sPrefix,i);
					strIpPrefix=strVal;

					strVal= string(sPrefix+i+1,iLen-1);
					iPrefixLen=atoi(strVal.c_str());
					break;
				}
			}

			memset(sBinPrefix,0,sizeof(sBinPrefix));
			IpToBinary(strIpPrefix,sBinPrefix);
			if(iPrefixLen>0)
			{
				strIpPrefix=string(sBinPrefix,iPrefixLen);
			}
			else
			{
				strIpPrefix="*";
			}
			fout<<strIpPrefix<<"\t"<<iNextHop<<"\t"<<skipRead<<"\t"<<segRead<<"\t"<<levelRead<<endl;
		}
	}

	fin.close();

	fout<<flush;
	fout.close();

	return iEntryCount;
}

void CFib::IpToBinary(string sIP,char saBin[32]){
	int iStart=0;			
	int iEnd=0;				
	int iFieldIndex = 3;	
	int iLen=sIP.length();	
	unsigned long	lPrefix=0;


	for ( int i=0; i<iLen; i++ ){

		if ( sIP.substr(i,1)== "." ){
			iEnd = i;
			string strVal=sIP.substr(iStart,iEnd-iStart);
			lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex);
			iFieldIndex--;
			iStart = i+1;
			i++;
		}
		if ( iFieldIndex == 0 ){
			iEnd = iLen;
			string strVal=sIP.substr(iStart,iEnd-iStart);
			lPrefix += atol(strVal.c_str());
			iStart = i+1;
		}
	}

	unsigned long	lVal=0x80000000;
	for(int i=0;i<32;i++){
		if(lPrefix&lVal){
			saBin[i]='1';
		}
		else{
			saBin[i]='0';
		}
		lVal=lVal>>1;
	}
}

void CFib::AddNode(unsigned long lPrefix,unsigned int iPrefixLen,unsigned int iNextHop)
{

	TrieNode* pTrie = m_pTrie;
	for (unsigned int i=0; i<iPrefixLen; i++){

		if(((lPrefix<<i) & HIGHTBIT)==HIGHTBIT){

			if(pTrie->rchild == NULL){

				TrieNode* pTChild;
				CreateNewNode(pTChild);

				pTChild->parent = pTrie;
				pTrie->rchild = pTChild;
			}
			pTrie = pTrie->rchild;

		}

		else{

			if(pTrie->lchild == NULL){
				TrieNode* pTChild;
				CreateNewNode(pTChild);

				pTChild->parent = pTrie;
				pTrie->lchild = pTChild;
			}

			pTrie = pTrie->lchild;
		}
	}

	pTrie->newPort = iNextHop;
	pTrie->oldPort = iNextHop;
}

unsigned int CFib::BuildFibFromFile(string sFileName)
{
	unsigned int	iEntryCount=0;	

	char			sPrefix[20];
	unsigned long	lPrefix;		
	unsigned int	iPrefixLen;	
	unsigned int	iNextHop;	

	ifstream fin(sFileName.c_str());
	while (!fin.eof()) {

		lPrefix = 0;
		iPrefixLen = 0;
		iNextHop = EMPTYHOP;

		memset(sPrefix,0,sizeof(sPrefix));
		fin >> sPrefix>> iNextHop;

		int iStart=0;			
		int iEnd=0;				
		int iFieldIndex = 3;	
		int iLen=strlen(sPrefix);	

		if(iLen>0){
			iEntryCount++;
			for ( int i=0; i<iLen; i++ ){

				if ( sPrefix[i] == '.' ){
					iEnd = i;
					string strVal(sPrefix+iStart,iEnd-iStart);
					lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex);
					iFieldIndex--;
					iStart = i+1;
					i++;
				}
				if ( sPrefix[i] == '/' ){
					iEnd = i;
					string strVal(sPrefix+iStart,iEnd-iStart);
					lPrefix += atol(strVal.c_str());
					iStart = i+1;

					i++;
					strVal= string(sPrefix+iStart,iLen-1);
					iPrefixLen=atoi(strVal.c_str());
				}
			}

			AddNode(lPrefix,iPrefixLen,iNextHop);
		}
	}
	fin.close();
	return iEntryCount;
}

int CFib::Iflame(TrieNode *pNode)
{
	if (NULL==pNode || NULL==pNode->parent)return -2;

	if (0!=pNode->newPort)return -1;

	if (NULL==pNode->lchild && NULL!=pNode->rchild)return RIGHT_ONLY;
	if (NULL==pNode->rchild && NULL!=pNode->lchild)return LEFT_ONLY;

	return -2;
}

void CFib::PathCompression(TrieNode* pNode)
{
	if (NULL==pNode)return;

	TrieNode *pPara;
	int skipTmp=0;
	char segmenTmp[32];
	ZeroStr(segmenTmp);

	int res=Iflame(pNode);

	while (res>0)
	{
		if (LEFT_ONLY==res)
		{
			segmenTmp[skipTmp++]='L';

			pPara=pNode->parent;
			pNode->lchild->parent=pPara;
			if (pPara->lchild==pNode)pPara->lchild=pNode->lchild;
			else					 pPara->rchild=pNode->lchild;

			pPara=pNode;

			pNode=pNode->lchild;
			pNode->skip=skipTmp;
			//printf("%d.",pNode->skip);
			strcpy(pNode->segment,segmenTmp);
			res=Iflame(pNode);

			free(pPara);
		}
		else if(RIGHT_ONLY==res)
		{
			segmenTmp[skipTmp++]='R';

			pPara=pNode->parent;
			pNode->rchild->parent=pPara;
			if (pPara->lchild==pNode)pPara->lchild=pNode->rchild;
			else					 pPara->rchild=pNode->rchild;

			pPara=pNode;

			pNode=pNode->rchild;
			pNode->skip=skipTmp;
			//printf("%d.",pNode->skip);
			strcpy(pNode->segment,segmenTmp);
			res=Iflame(pNode);

			free(pPara);
		}
	}

	//if (skipTmp!=0)
	//{
	//	printf("skipTmp=%d,segmenTmp=%s\n",skipTmp,segmenTmp);
	//}

	PathCompression(pNode->lchild);
	PathCompression(pNode->rchild);
}


void CFib::LevelCompression(TrieNode* pNode)
{
	if (NULL==pNode)return;
	if (0==pNode->maxLevel)
	{
		//printf("%d^^\n ",pNode->newPort);
		LevelCompression(pNode->lchild);
		LevelCompression(pNode->rchild);
	}
	else{
		LevelOrder(pNode);

		//printf("*****\n");

		for (int i=0;i< (1 << pNode->maxLevel);i++)
		{
			LevelCompression(pNode->MultiPoint[i]);
		}
	}
	
}

void CFib::printTest(TrieNode* pNode)
{
	if (NULL==pNode)return;
	if(pNode->maxLevel!=0)
	{
		printf("%d\t", pNode->newPort);
		if (NULL!=pNode->MultiPoint)
		{
			for(int i =0; i< (1 << pNode->maxLevel); i++)
			{

				printf("%d", pNode->MultiPoint[i]->newPort);
			}
		}
		printf("****\n");
	}
	else{
		printf("%d^^\n", pNode->newPort);
	}
	printTest(pNode->lchild);
	printTest(pNode->rchild);

}

void CFib::LevelOrder(TrieNode *pNode)
{
	if (NULL==pNode)return;


	int ilevel=pNode->maxLevel;
	int MaxSize = (1 << (ilevel+1));
	TrieNode ** pTmp= (TrieNode **)malloc((1 << ilevel)*sizeof(TrieNode*));
	int cursor = 0;

	if (NULL==pTmp){printf("malloc failed...\n");return;}

	int count=0;

	TrieNode *p;
	TrieNode **qu=(TrieNode **)malloc(MaxSize*sizeof(TrieNode *));;
	int front,rear;
	front=rear=-1;
	rear++;
	qu[rear]=pNode;
	while(front!=rear)
	{
		front=(front+1)%MaxSize;
		p=qu[front];
		//printf("%d-- ",p->newPort);
		count++;

		if(count >= (1 << (ilevel + 1)))break;
		if(count >= (1 << ilevel))
		{
			pTmp[cursor++] = p;
			continue;
		}

		if(p->lchild!=NULL)
		{
			rear=(rear+1)%MaxSize;
			qu[rear]=p->lchild;
		}   
		if(p->rchild!=NULL)
		{
			rear=(rear+1)%MaxSize;
			qu[rear]=p->rchild;
		}     
	}    
	pNode->MultiPoint = pTmp;
}


int CFib::smaller(int a, int b)
{
	return a>b?b:a;
}

int CFib::GetMaxLevel(TrieNode* pNode)
{
	if (NULL==pNode)return 0;

	GetMaxLevel(pNode->lchild);
	GetMaxLevel(pNode->rchild);

	if (NULL==pNode->lchild || NULL==pNode->rchild)pNode->maxLevel=0;
	else
	{		
		int lLevel = pNode->lchild->maxLevel;
		int rLevel = pNode->rchild->maxLevel;
		if(pNode->lchild->newPort > 0)lLevel=0;
		if(pNode->rchild->newPort > 0)rLevel=0;

		pNode->maxLevel=smaller(lLevel,rLevel)+1;
	}

	return pNode->maxLevel;
}

int CFib::lookupNoSkip(TrieNode* pNode, char *ip, int cur, int port)
{
	//if (true==pNode->ifblind)ifthislookupBlind=true;

	TrieNode * tmp = NULL;
	//if(pNode->newPort != 0) port = pNode->newPort;
	if(pNode->maxLevel == 0){
		if(ip[cur] == '0') tmp = pNode->lchild;
		else if(ip[cur] == '1') tmp = pNode->rchild;
		
		if(tmp == NULL) return port;
		else 
		{
			//MemoryAccessLookup++;
			//MemoryAccessLookup_BS++;
			//if (true==pNode->ifblind)MemoryAccessLookup_BS+=4;
			return lookup(tmp, ip, cur + 1, port);
		}
	}
	else{
		int loc = 0;
		for(int i = 0; i < pNode->maxLevel; i++){
			loc <<= 1;
			if(ip[cur + i] == '1')
				loc ++;
		}
		tmp = pNode->MultiPoint[loc];
		//MemoryAccessLookup++;
		//MemoryAccessLookup_BS++;
		//if (true==pNode->ifblind)MemoryAccessLookup_BS+=4;
		return lookup(tmp, ip, cur + pNode->maxLevel, port);
	}
}
//11000010.10000101.11010000.00011000

int CFib::lookup(TrieNode* pNode, char *ip, int cur, int port){
	//if(pNode->newPort != 0) port = pNode->newPort;
	//if (true==pNode->ifblind)ifthislookupBlind=true;

	if(cur > 31)
	{
		if(pNode->newPort != 0) port = pNode->newPort;
		return port;
	}
	if(pNode->skip == 0)
	{
		if(pNode->newPort != 0) port = pNode->newPort;
		return lookupNoSkip(pNode, ip, cur, port);	
	}
	else
	{
		bool flag = true;
		for(int i = 0; i < pNode->skip; i++)
		{
			if(!((ip[cur + i] == '0' && pNode->segment[i] == 'L')||(ip[cur + i] == '1' && pNode->segment[i] == 'R')))
			{
				flag = false;
				break;
			}
		}
		if(flag)
		{
			if(pNode->newPort != 0) port = pNode->newPort;
			return lookupNoSkip(pNode, ip, cur + pNode->skip, port);
		}
		else
		{
			return port;
		}
	}
}


bool CFib::pathCompare(char segment[],char insert_C[],int len)
{
	if (strlen(insert_C)<len)
		return false;

	for (int i=0;i<len;i++)
	{
		if (segment[i]=='L' && insert_C[i]=='0' || segment[i]=='R' && insert_C[i]=='1')continue;
		else return false;
	}
	return true;
}


bool CFib::BS_Update(int insertport, char *insert_C, int operation_type)
{
	int operation;
	int maxLevelAccum=0;
	int pathAccum=0;
	TrieNode *insertNode=m_pTrie;

	TrieNode *ret=NULL;
	bool IfNewBornNode=false;
	//look up the location of the current node
	for (int i=0;i<strlen(insert_C);)
	{
		if (insertNode->skip!=0)
		{
			pathAccum+=insertNode->skip;
			if (true == pathCompare(insertNode->segment,&(insert_C[i]),insertNode->skip))
			{
				i+=insertNode->skip;
			}
			else
			{
				if(_DELETE==operation_type)
				{
					return NULL;
				}
				else
				{
					insertNode->ifblind=true;

					//insertNode->blindDepth=GetDepthSizeofaNode(insertNode,blindDepth[blindCount],blindSize[blindCount]);
					//blindCount++;
				}

			}
		}

		//if(insertNode->maxLevel == 0)
		{
			if(insert_C[i] == '0')
			{
				if (NULL==insertNode->lchild)
				{//turn left, if left child is empty, create new node
					if(_DELETE==operation_type)	
					{
						return NULL;
					}
					IfNewBornNode=true;
					TrieNode* pNewNode;
					CreateNewNode(pNewNode);
					pNewNode->parent=insertNode;
					insertNode->lchild=pNewNode;
				}

				insertNode=insertNode->lchild;
			}
			else if(insert_C[i] == '1')			//turn right, if left child is empty, create new node
			{	
				if (NULL==insertNode->rchild)
				{
					if(_DELETE==operation_type)	
					{
						return NULL;
					}
					IfNewBornNode=true;
					TrieNode* pNewNode;
					CreateNewNode(pNewNode);
					pNewNode->parent=insertNode;
					insertNode->rchild=pNewNode;
				}
				insertNode=insertNode->rchild;
			}

			if (insertNode->maxLevel>1 && insertNode->MultiPoint!=NULL)
			{
				maxLevelAccum=maxLevelAccum+insertNode->maxLevel-1;
			}
			i++;
		}
	}

	if('A'==operation_type) 
	{
		if (insertNode->newPort==insertport)return NULL;

		insertNode->newPort=insertport;
		ret=insertNode;
	}
	else if (0==insertNode->newPort)
	{
		return NULL;
	}
	else 
	{
		if (IsLeaf(insertNode))
		{
			insertNode->newPort=0;
			ret=insertNode;
		}
		else
		{
			ret=insertNode;
		}
	}

	MemoryVisitSum = MemoryVisitSum+ (int)strlen(insert_C)-maxLevelAccum-pathAccum;

	return ret;
}


bool CFib::ifFindBlind(TrieNode * m_trie,char * insert_C)
{
	ifthislookupBlind=false;
	lookup(m_trie,insert_C,0,-1);

	return	ifthislookupBlind;
}

bool CFib::ifSecondVisit(TrieNode * m_trie,char * insert_C)
{
	TrieNode *insertNode=m_pTrie;

	for (int i=0;i<strlen(insert_C);)
	{
		if(true==insertNode->ifblind)return true;

		if (insertNode->skip!=0)
		{
			if (true == pathCompare(insertNode->segment,&(insert_C[i]),insertNode->skip))
			{
				i+=insertNode->skip;
			}
			else	break;
		}

		if ('0'==insert_C[i])
		{//if 0, turn left
			if (NULL!=insertNode->lchild)	insertNode=insertNode->lchild;
			else								break;		
		}
		else
		{//if 1, turn right
			if (NULL!=insertNode->rchild)	insertNode=insertNode->rchild;
			else								break;	
		}
		i++;
	}
	return false;	
}
