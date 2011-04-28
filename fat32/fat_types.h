#ifndef __FAT_TYPES_H__
#define __FAT_TYPES_H__

//-------------------------------------------------------------
// System specific types
//-------------------------------------------------------------
#ifndef DEF_BYTE
#define DEF_BYTE
	//typedef unsigned char BYTE;
#endif

#ifndef DEF_UINT16
#define DEF_UINT16
	typedef unsigned short UINT16;
#endif

#ifndef DEF_UINT32
#define DEF_UINT32
	typedef unsigned long UINT32;
#endif

#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef NULL
	#define NULL 0
#endif

//-------------------------------------------------------------
// Structure Packing Compile Options
//-------------------------------------------------------------
#define STRUCT_PACK	
#define STRUCT_PACK_BEGIN	
#define STRUCT_PACK_END		
#define STRUCT_PACK_PREEND	__attribute__((__packed__))	

#endif
