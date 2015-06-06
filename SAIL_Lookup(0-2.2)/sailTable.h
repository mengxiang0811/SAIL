#ifndef _sail_TABLE_H
#define _sail_TABLE_H

#define CHUNK							256				//# of elements in a chunk
#define CHUNK_MAX_24					256				//# of updating chunk in level 24 at most
#define CHUNK_MAX_32					256				//# of updating chunk in level 32 at most

#define PORT_LEN								1		//the number of bytes for next-hop storage
#define PORT_RANGE	((1 << (PORT_LEN << 3)) - 1)			//the max number of nexthops

#define	ERROR_BITS_VAL				257

#define LEVEL16_ELE_NUM			65536			//the size of table in level 16
#define LEVEL24_CHUNK_NUM		25000			//the initial number of chunks in level 24
#define LEVEL32_CHUNK_NUM		1000000			//the initial number of chunks in level 32

#define NB_16										0			//only create new nodes in level 17~24
#define NB_24										1			//only create new nodes in level 25~32
#define NB_BOTH										2			//both above

struct sailTable_16 {	//struct in level 16
	short element[LEVEL16_ELE_NUM];	// >0 --- offset; <0 --- nexthop;
};

struct sailTable_24 {	//struct in level 24
	unsigned char nexthop[LEVEL24_CHUNK_NUM * CHUNK];	// flag+next-hop array
	unsigned short offset[LEVEL24_CHUNK_NUM * CHUNK];	// offset array
};

struct sailTable_32 {	//struct in level 32
	unsigned char nexthop[LEVEL32_CHUNK_NUM * CHUNK];	// next-hop array in level 32
};

struct segInfo_16 {
	int chunk_id;
	short nexthop[CHUNK];
};

struct segInfo_24 {
	int chunk_id;
	unsigned char nexthop[CHUNK];	// the updating chunk id and the value of segment to update  in level 24
};

struct segInfo_32 {
	int chunk_id;
	unsigned char nexthop[CHUNK];	// the updating chunk id and the value of segment to update  in level 32
};

struct chunkInfo {
	int chunk_id;
	unsigned char nexthop[CHUNK];	// the updating chunk id and the value to update  in level 24 or 32
};

struct subLevelPushArg {
	int bornStatus;
	int chunkID_24;
	int chunkID_32;
};

struct subTrieUpdateArg {
	unsigned char segLevel;	//16¡¢24¡¢32
	unsigned char length;
};

#endif