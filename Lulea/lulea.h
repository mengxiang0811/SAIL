/*
 * @# bitType.h 2011-10-19
 **/
#ifndef BITTYPE_H_
#define BITTYPE_H_
#define LENGTH 676
#define MAX_CHUNK 16
#define DEFAULT ((1 << 30) - 1)




typedef unsigned short ushort;
typedef unsigned int uint;


typedef struct{
	uint p:30;
	uint t:2 ;
}TrieNode;


typedef TrieNode Level16[1<<17];
typedef TrieNode Level8[1<<9];


typedef struct {
	ushort inx :10;
	ushort offset : 6;
}uword;



typedef uword codeword16[1<<12];
typedef uword codeword8[1<<4];


typedef ushort baseindex16[1<<10];
typedef ushort baseindex8[1<<2];


typedef unsigned long long ulong;
typedef ulong maptable[LENGTH];


typedef struct {
	uint ipnet;
	uint port;
	ushort length;
}RouterEntry;


typedef struct {
	uword * codeword;
	ushort * baseindex;
	TrieNode * lookup;
}LookupEntry;


typedef LookupEntry LookupTalbe[1 << MAX_CHUNK];

void start(char * file);
int lookup(uint ipaddr);
void freeMemory();

#endif /* BITTYPE_H_ */
