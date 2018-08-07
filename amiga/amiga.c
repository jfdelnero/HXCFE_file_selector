/*
//
// Copyright (C) 2009-2018 Jean-François DEL NERO
//
// This file is part of the HxCFloppyEmulator file selector.
//
// HxCFloppyEmulator file selector may be used and distributed without restriction
// provided that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator file selector is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator file selector is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator file selector; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

#include <intuition/intuitionbase.h>

#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <graphics/gfxbase.h>
#include <graphics/videocontrol.h>

#include <devices/trackdisk.h>

#include <exec/interrupts.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <exec/execbase.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#include "conf.h"

#include "fast_char.h"

#include "keysfunc_defs.h"
#include "keys_defs.h"
#include "keymap.h"

#include "cfg_file.h"
#include "ui_context.h"
#include "gui_utils.h"

#include "../graphx/font_list.h"

#include "../hal.h"

#include "amiga_regs.h"

#include "reboot.h"

#include "crc.h"

#include "color_table.h"
#include "mfm_table.h"

#include "errors_def.h"

#define DEPTH    2 /* 1 BitPlanes should be used, gives eight colours. */
#define COLOURS  2 /* 2^1 = 2                                          */

#define BLACK 0x002           /*  RGB values for the four colors used.   */
#define RED   0xFFF
#define GREEN 0x0f0
#define BLUE  0x00f

volatile unsigned short io_floppy_timeout;

unsigned char * screen_buffer;

static unsigned char CIABPRB_DSKSEL;

static unsigned char * mfmtobinLUT_L;
static unsigned char * mfmtobinLUT_H;

#define MFMTOBIN(W) ( mfmtobinLUT_H[W>>8] | mfmtobinLUT_L[W&0xFF] )

#define RD_TRACK_BUFFER_SIZE 10*1024
#define WR_TRACK_BUFFER_SIZE 600

static unsigned short * track_buffer_rd;
static unsigned short * track_buffer_wr;

static unsigned char validcache;

#define MAX_CACHE_SECTOR 16
unsigned short sector_pos[MAX_CACHE_SECTOR];

unsigned char keyup;

unsigned long timercnt;

unsigned long __commandline;
unsigned long __commandlen;

struct Interrupt *rbfint, *priorint;

extern struct Library * DOSBase;
struct Library * libptr;
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBaseptr;
struct View * my_old_view;
struct View view;
struct ViewPort viewPort = { 0 };
struct RasInfo rasInfo;
struct BitMap my_bit_map;
struct RastPort my_rast_port;
struct Screen *screen;

static unsigned short bytes_per_line;

UWORD  *pointer;
struct ColorMap *cm=NULL;

extern ui_context g_ui_ctx;

struct TextAttr MyFont =
{
		(STRPTR)"topaz.font", // Font Name
		TOPAZ_SIXTY, // Font Height
		FS_NORMAL, // Style
		FPF_ROMFONT, // Preferences
};

struct NewScreen screen_cfg =
{
		(WORD)0,   /* the LeftEdge should be equal to zero */
		(WORD)0,   /* TopEdge */
		(WORD)640, /* Width (low-resolution) */
		(WORD)256, /* Height (non-interlace) */
		(WORD)1,   /* Depth (4 colors will be available) */
		(UBYTE)0, (UBYTE)1, /* the DetailPen and BlockPen specifications */
		(UWORD)0,  /* no special display modes */
		CUSTOMSCREEN, /* the screen type */
		&MyFont, /* use my own font */
		(UBYTE *)"HxC Floppy Emulator file selector", /* this declaration is compiled as a text pointer */
		(struct Gadget *)NULL, /* no special screen gadgets */
		(struct BitMap *)NULL  /* no special CustomBitMap */
};

struct TagItem vcTags[] =
{
	{VTAG_ATTACH_CM_SET, (ULONG)NULL },
	{VTAG_VIEWPORTEXTRA_SET, (ULONG)NULL },
	{VTAG_NORMAL_DISP_SET, (ULONG)NULL },
	{VTAG_END_CM, (ULONG)NULL }
};

/********************************************************************************
*                     amiga.lib missing functions
*********************************************************************************/
extern struct ExecBase *SysBase;

void NewList(struct List *lh)
{
	lh->lh_Head = (struct Node *)(&lh->lh_Tail);
	lh->lh_Tail = NULL;
	lh->lh_TailPred = (struct Node *)(&lh->lh_Head);
}

struct MsgPort *CreatePort(UBYTE *name, LONG pri)
{
	LONG sigBit;
	struct MsgPort *mp;

	if ((sigBit = AllocSignal(-1L)) == -1)
		return(NULL);

	mp = (struct MsgPort *) AllocMem((ULONG)sizeof(struct MsgPort),
			(ULONG)MEMF_PUBLIC | MEMF_CLEAR);

	if (!mp) {
			FreeSignal(sigBit);
			return(NULL);
	}

	mp->mp_Node.ln_Name = (char*)name;
	mp->mp_Node.ln_Pri  = pri;
	mp->mp_Node.ln_Type = NT_MSGPORT;
	mp->mp_Flags        = PA_SIGNAL;
	mp->mp_SigBit       = sigBit;
	mp->mp_SigTask      = (struct Task *)FindTask(0L);  /* Find THIS task.   */

	if (name)
		AddPort(mp);
	else
		NewList(&(mp->mp_MsgList));                     /* init message list */

	return(mp);
}

struct IOStdReq * LCreateIORequest(struct MsgPort * replyPort,long  size)
{
	struct IOStdReq *io = NULL;

	if (replyPort)
	{
		io = AllocMem(size, MEMF_PUBLIC | MEMF_CLEAR);
		if ( io )
		{
			io->io_Message.mn_ReplyPort = replyPort;
			io->io_Message.mn_Length = size;
			io->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
		}
	}
	return(io);
}

int FlushDevice(unsigned char *name)
{
	struct Device *devpoint;

	Forbid();

	devpoint = (struct Device *)FindName(&SysBase->DeviceList,name);

	if ( devpoint )
	{
		RemDevice(devpoint);

		Permit();
		return 1;
	}

	Permit();

	return 0;
}

int FlushResource(unsigned char *name)
{
	APTR resource;

	Forbid();

	resource = (APTR)FindName(&SysBase->ResourceList,name);

	if ( resource )
	{
		RemResource(resource);

		Permit();
		return 1;
	}

	Permit();

	return 0;
}

#ifdef DEBUG

void push_serial_char(unsigned char byte)
{
	WRITEREG_W(0xDFF032,0x1E);              //SERPER - 115200 baud/s

	while(!(READREG_W(0xDFF018) & 0x2000)); //SERDATR

	WRITEREG_W(0xDFF030,byte | 0x100);      //SERDAT
}

void dbg_printf(char * chaine, ...)
{
	unsigned char txt_buffer[1024];
	int i;

	va_list marker;
	va_start( marker, chaine );

	vsnprintf(txt_buffer,sizeof(txt_buffer),chaine,marker);

	i = 0;
	while(txt_buffer[i])
	{
		if(txt_buffer[i] == '\n')
		{
			push_serial_char('\r');
			push_serial_char('\n');
		}
		else
			push_serial_char(txt_buffer[i]);

		i++;
	}

	va_end( marker );
}

#endif

void waitus(int centus)
{
	unsigned short time;

	WRITEREG_B(CIAB_CRA, (READREG_B(CIAB_CRA)&0xC0) | 0x08 );
	WRITEREG_B(CIAB_ICR, 0x7F );

	time = 0x48 * centus;
	WRITEREG_B(CIAB_TALO, time&0xFF );
	WRITEREG_B(CIAB_TAHI, time>>8 );

	WRITEREG_B(CIAB_CRA, READREG_B(CIAB_CRA) | 0x01 );

	do
	{
	}while(!(READREG_B(CIAB_ICR)&1));
}

void waitms(int ms)
{
	int cnt;

	WRITEREG_B(CIAB_CRA, (READREG_B(CIAB_CRA)&0xC0) | 0x08 );
	WRITEREG_B(CIAB_ICR, 0x7F );

	WRITEREG_B(CIAB_TALO, 0xCC );
	WRITEREG_B(CIAB_TAHI, 0x02 );

	WRITEREG_B(CIAB_CRA, READREG_B(CIAB_CRA) | 0x01 );
	for(cnt=0;cnt<ms;cnt++)
	{
		do
		{
		}while(!(READREG_B(CIAB_ICR)&1));

		WRITEREG_B(CIAB_CRA, READREG_B(CIAB_CRA) | 0x01 );
	}
}

void testblink()
{
	for(;;)
	{
		waitms(500);

		WRITEREG_B(CIAAPRA, READREG_B(CIAAPRA) ^  0x02 );
	}
}

void lockup()
{
	#ifdef DEBUG
	dbg_printf("lockup : Sofware halted...\n");
	#endif

	for(;;)
	{
		waitsec(100);
	}
}

/********************************************************************************
*                              FDC I/O
*********************************************************************************/

/*
* Returns the unit number of the underlying device of a filesystem lock.
* Returns -1 on failure.
*/
LONG GetUnitNumFromLock(BPTR lock) {
	LONG unitNum = -1;
	if(lock != 0) {
		struct InfoData *infoData = AllocMem(sizeof(struct InfoData), MEMF_ANY);
		if(NULL != infoData) {
			if(Info(lock, infoData)) {
				unitNum = infoData->id_UnitNumber;
			}
			FreeMem(infoData, sizeof(struct InfoData));
		}
	}

	#ifdef DEBUG
	dbg_printf("GetUnitNumFromLock : %d\n", unitNum);
	#endif
	return unitNum;
}

/*
* Returns the unit number of the underlying device of a filesystem path.
* Returns -1 on failure.
*/
LONG GetUnitNumFromPath(char *path) {
	LONG unitNum = -1;

	if(path)
	{
		#ifdef DEBUG
		dbg_printf("GetUnitNumFromPath : %s\n",path);
		#endif

		BPTR lock = Lock((CONST_STRPTR)path, ACCESS_READ);
		if(lock != 0) {
			unitNum = GetUnitNumFromLock(lock);
			UnLock(lock);
		}
		else
		{
			#ifdef DEBUG
			dbg_printf("GetUnitNumFromPath : Lock failed !\n");
			#endif
		}
	}
	else
	{
		#ifdef DEBUG
		dbg_printf("GetUnitNumFromPath : NULL path !\n");
		#endif
	}
	return unitNum;
}

UWORD GetLibraryVersion(struct Library *library)
{
	#ifdef DEBUG
	dbg_printf("GetLibraryVersion : %d\n",library->lib_Version);
	#endif

	return library->lib_Version;
}

int test_drive(int drive)
{
	int t,j,c;
	Forbid();
	WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3)) ));

	waitms(100);

	// Jump to Track 0 ("Slow")
	t = 0;
	while((READREG_B(CIAAPRA) & CIAAPRA_DSKTRACK0) && (t<260))
	{
		WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3))  | CIABPRB_DSKSTEP));
		waitus(10);
		WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3)) ) );
		waitus(80);

		t++;
	}

	if(t<260)
	{
		c = 0;
		do
		{
			// Jump to Track 30 (Fast)
			for(j=0;j<40;j++)
			{
				WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3)) | CIABPRB_DSKDIREC |CIABPRB_DSKSTEP) );
				waitus(8);
				WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3)) | CIABPRB_DSKDIREC ) );
				waitus(8);
			}

			waitus(200);

			// And go back to Track 30 (Slow)
			t = 0;
			while((READREG_B(CIAAPRA) & CIAAPRA_DSKTRACK0) && (t<40))
			{
				WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3))  | CIABPRB_DSKSTEP));
				waitus(10);
				WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3)) ) );
				waitus(80);

				t++;
			}

			c++;
		}while( (t != 40) && c < 2 );

		if(t == 40)
		{
			WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3)) ) );

			Permit();

			return ERR_NO_ERROR;
		}
	}

	WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3)) ) );

	Permit();

	return -ERR_DRIVE_NOT_FOUND;
}

int get_start_unit(char * path)
{
	int i;
	LONG startedFromUnitNum;

	#ifdef DEBUG
	dbg_printf("get_start_unit\n");
	#endif

	if( DOSBase )
	{
		if( GetLibraryVersion((struct Library *) DOSBase) >= 36 )
		{
			startedFromUnitNum = GetUnitNumFromLock( GetProgramDir() );
		}
		else
		{
			startedFromUnitNum = GetUnitNumFromPath( path );
		}
	}
	else
	{
		// No DOS Library ? We are probably trackloaded.
		startedFromUnitNum = 0;
	}

	if( startedFromUnitNum < 0 )
		startedFromUnitNum = 0;

	for( i = 0; i < 4; i++ )
	{
		if(test_drive((startedFromUnitNum + i) & 0x3) == ERR_NO_ERROR)
		{
			#ifdef DEBUG
			dbg_printf("get_start_unit : drive %d\n",(startedFromUnitNum + i) & 0x3);
			#endif
			return (startedFromUnitNum + i) & 0x3;
		}
	}

	#ifdef DEBUG
	dbg_printf("get_start_unit : drive not found !\n");
	#endif

	return -ERR_DRIVE_NOT_FOUND;
}

int jumptotrack(unsigned char t)
{
	unsigned short j,k;

	WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | CIABPRB_DSKSEL ));

	#ifdef DEBUG
	dbg_printf("jumptotrack : %d\n",t);
	#endif

	waitms(100);

	#ifdef DEBUG
	dbg_printf("jumptotrack %d - seek track 0...\n",t);
	#endif

	k = 0;
	while((READREG_B(CIAAPRA) & CIAAPRA_DSKTRACK0) && k<1024)
	{
		WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | CIABPRB_DSKSEL  | CIABPRB_DSKSTEP));
		waitms(1);
		WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | CIABPRB_DSKSEL ) );
		waitms(1);

		k++;
	}

	if(k < 1024)
	{
		#ifdef DEBUG
		dbg_printf("jumptotrack %d - track 0 found\n",t);
		#endif

		for(j=0;j<t;j++)
		{
			WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | CIABPRB_DSKSEL | CIABPRB_DSKDIREC |CIABPRB_DSKSTEP) );
			waitms(1);
			WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | CIABPRB_DSKSEL | CIABPRB_DSKDIREC ) );
			waitms(1);
		}

		WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | CIABPRB_DSKSEL ) );

		#ifdef DEBUG
		dbg_printf("jumptotrack %d - jump done\n",t);
		#endif

		return ERR_NO_ERROR;
	}

	#ifdef DEBUG
	dbg_printf("jumptotrack %d - track 0 not found!!\n",t);
	#endif

	return -ERR_TRACK0_SEEK;
};

int waitindex()
{
	io_floppy_timeout = 0;

	do{
		asm("nop");
	}while( (!(READREG_B(CIAB_ICR)&0x10)) && ( io_floppy_timeout < 0x200 ) );

	do
	{
		asm("nop");
	}while( (READREG_B(CIAB_ICR)&0x10) && ( io_floppy_timeout < 0x200 ) );

	do{
		asm("nop");
	}while((!(READREG_B(CIAB_ICR)&0x10)) && ( io_floppy_timeout < 0x200 ) );

	if(!(io_floppy_timeout < 0x200 ))
	{
		return -ERR_TIMEOUT;
	}

	return ERR_NO_ERROR;
}

int readtrack(unsigned short * track,unsigned short size,unsigned char wait_index)
{
	WRITEREG_B(CIABPRB,~(CIABPRB_DSKMOTOR | CIABPRB_DSKSEL));
	WRITEREG_W( DMACON,0x8210);

	WRITEREG_W(INTREQ,0x0002);

	// set dsklen to 0x4000
	WRITEREG_W( DSKLEN ,0x4000);

	WRITEREG_L( DSKPTH ,track);

	WRITEREG_W( ADKCON, 0x7F00);
	WRITEREG_W( ADKCON, 0x9500); //9500
	WRITEREG_W( DMACON, 0x8210);
	WRITEREG_W( DSKSYNC,0x4489);
	WRITEREG_W( INTREQ, 0x0002);

	if(wait_index)
	{
		if(waitindex() != ERR_NO_ERROR)
		{
			return -ERR_MEDIA_READ_NO_INDEX;
		}
	}

	//Put the value you want into the DSKLEN register
	WRITEREG_W( DSKLEN ,size | 0x8000);
	//Write this value again into the DSKLEN register. This actually starts the DMA.
	WRITEREG_W( DSKLEN ,size | 0x8000);

	while(!(READREG_W(INTREQR)&0x0002));
	WRITEREG_W( DSKLEN ,0x4000);
	WRITEREG_W(INTREQ,0x0002);

	validcache=1;

	return ERR_NO_ERROR;
}

int writetrack(unsigned short * track,unsigned short size,unsigned char wait_index)
{

//	while(!(READREG_W(INTREQR)&0x0002));

	WRITEREG_B(CIABPRB,~(CIABPRB_DSKMOTOR | CIABPRB_DSKSEL));
	WRITEREG_W( DMACON,0x8210);

	WRITEREG_W(INTREQ,0x0002);

	// set dsklen to 0x4000
	WRITEREG_W( DSKLEN ,0x4000);

	WRITEREG_L( DSKPTH ,track);

	WRITEREG_W( ADKCON, 0x7F00);
	WRITEREG_W( ADKCON, 0xB100); //9500
	WRITEREG_W( DMACON, 0x8210);
	WRITEREG_W( DSKSYNC,0x4489);
	WRITEREG_W( INTREQ, 0x0002);

	if(wait_index)
	{
		io_floppy_timeout = 0;
		while( READREG_B(CIAB_ICR)&0x10 && ( io_floppy_timeout < 0x200 ) );
		while( !(READREG_B(CIAB_ICR)&0x10) && ( io_floppy_timeout < 0x200 ) );
		if(!( io_floppy_timeout < 0x200 ))
		{
			return -ERR_MEDIA_WRITE_NO_INDEX;
		}
	}

	//Put the value you want into the DSKLEN register
	WRITEREG_W( DSKLEN ,size | 0x8000 | 0x4000 );
	//Write this value again into the DSKLEN register. This actually starts the DMA.
	WRITEREG_W( DSKLEN ,size | 0x8000 | 0x4000 );

	while(!(READREG_W(INTREQR)&0x0002));

	WRITEREG_W( DSKLEN ,0x4000);
	WRITEREG_W(INTREQ,0x0002);

	validcache=0;

	return ERR_NO_ERROR;
}

// Fast Bin to MFM converter
int BuildCylinder(unsigned char * mfm_buffer,int mfm_size,unsigned char * track_data,int track_size,unsigned short lastbit,unsigned short * retlastbit)
{
	int i,l;
	unsigned char byte;
	unsigned short mfm_code;

	if(track_size*2>mfm_size)
	{
		track_size=mfm_size/2;
	}

	// MFM Encoding
	i=0;
	for(l=0;l<track_size;l++)
	{
		byte =track_data[l];

		mfm_code = MFM_tab[byte] & lastbit;

		mfm_buffer[i++]=mfm_code>>8;
		mfm_buffer[i++]=mfm_code&0xFF;

		lastbit=~(MFM_tab[byte]<<15);
	}

	if(retlastbit)
		*retlastbit = lastbit;

	return track_size;
}

int writesector(unsigned char sectornum,unsigned char * data)
{
	int ret;
	unsigned short i,j,len,retry,retry2,lastbit;
	unsigned char sectorfound;
	unsigned char c;
	unsigned char CRC16_High,CRC16_Low,byte;
	unsigned char sector_header[4];
	unsigned short crc16v;
	#ifdef DEBUG
	dbg_printf("writesector : %d\n",sectornum);
	#endif

	Forbid();

	retry2=2;

	i=0;
	validcache=0;

	// Preparing the buffer...
	crc16v = 0xFFFF;

	for(j=0;j<3;j++)
	{
		crc16v = crc16( 0xA1 , crc16v );
	}

	crc16v = crc16( 0xFB, crc16v );
	crc16v = crc16_buf(data, 512, crc16v);

	for(j=0;j<12;j++)
		track_buffer_wr[i++]=0xAAAA;

	track_buffer_wr[i++]=0x4489;
	track_buffer_wr[i++]=0x4489;
	track_buffer_wr[i++]=0x4489;
	lastbit = 0x7FFF;
	byte = 0xFB;
	BuildCylinder((unsigned char*)&track_buffer_wr[i++],1*2,&byte,1,lastbit,&lastbit);
	BuildCylinder((unsigned char*)&track_buffer_wr[i],512*2,data,512,lastbit,&lastbit);
	i += 512;

	CRC16_Low = (crc16v >> 8);
	CRC16_High = (crc16v & 0xFF);

	BuildCylinder((unsigned char*)&track_buffer_wr[i++],1*2,&CRC16_High,1,lastbit,&lastbit);
	BuildCylinder((unsigned char*)&track_buffer_wr[i++],1*2,&CRC16_Low,1,lastbit,&lastbit);
	byte = 0x4E;
	for(j=0;j<4;j++)
	{
		BuildCylinder((unsigned char*)&track_buffer_wr[i++],1*2,&byte,1,lastbit,&lastbit);
	}

	len = i;

	// Looking for/waiting the sector to write...

	sector_header[0]=0xFF;
	sector_header[1]=0x00;
	sector_header[2]=sectornum;

	sectorfound = 0;
	retry = 30;

	if(sectornum)
	{
		do
		{
			do
			{
				i=0;

				retry--;

				ret = readtrack(track_buffer_rd,16,0);
				if( ret != ERR_NO_ERROR )
				{
					Permit();
					return ret;
				}

				while(track_buffer_rd[i]==0x4489 && (i<16))
				{
					i++;
				}

				if(MFMTOBIN(track_buffer_rd[i])==0xFE && (i<(16-3)))
				{
					crc16v = 0xFFFF;

					for(j=0;j<3;j++)
						crc16v = crc16( 0xA1 , crc16v );

					for(j=0;j<(1+4+2);j++)
					{
						c = MFMTOBIN(track_buffer_rd[i+j]);
						crc16v = crc16( c , crc16v );
					}

					if(!crc16v)
					{
						i++;

						j = 0;
						while(j<3 && ( MFMTOBIN(track_buffer_rd[i]) == sector_header[j] ) ) // track,side,sector
						{
							j++;
							i++;
						}

						if(j == 3)
						{
							sectorfound=1;
							ret = writetrack(track_buffer_wr,len,0);
							if( ret != ERR_NO_ERROR )
							{
								Permit();
								return ret;
							}
						}
					}
				}
			}while(!sectorfound  && retry);

			if(!sectorfound)
			{
				ret = jumptotrack(255);
				if( ret != ERR_NO_ERROR)
				{
					return ret;
				}
				retry=30;
			}
			retry2--;

		}while(!sectorfound && retry2);

	}
	else
	{
		sectorfound=1;

		ret = writetrack(track_buffer_wr,len,1);
		if(ret != ERR_NO_ERROR)
		{
			Permit();
			return ret;
		}

	}

	Permit();

	if(sectorfound)
		return ERR_NO_ERROR;
	else
		return -ERR_MEDIA_WRITE_SECTOR_NOT_FOUND;
}


int readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache)
{
	int ret;
	unsigned short i,j;
	unsigned char sectorfound,tc;
	unsigned char c,retry,badcrc,retry2;
	unsigned char sector_header[8];
	unsigned char sect_num;
	unsigned short crc16v;

	#ifdef DEBUG
	dbg_printf("readsector : %d - %d\n",sectornum,invalidate_cache);
	#endif

	if(!(sectornum<MAX_CACHE_SECTOR))
		return -ERR_INVALID_PARAMETER;

	retry2 = 2;
	retry = 5;

	sector_header[0] = 0xFE; // IDAM
	sector_header[1] = 0xFF; // Track
	sector_header[2] = 0x00; // Side
	sector_header[3] = sectornum; // Sector
	sector_header[4] = 0x02;      // Size

	crc16v = 0xFFFF;
	for( j = 0; j < 3; j++ )
	{
		crc16v = crc16( 0xA1 , crc16v );
	}

	for(j=0;j< 5;j++)
	{
		crc16v = crc16( sector_header[j] , crc16v );
	}

	sector_header[5] = crc16v&0xFF; // CRC L
	sector_header[6] = crc16v>>8;   // CRC H

	do
	{
		do
		{
			sectorfound=0;
			i=0;
			badcrc=0;
			if(!validcache || invalidate_cache)
			{
				Forbid();
				ret = readtrack(track_buffer_rd,RD_TRACK_BUFFER_SIZE,0);
				if( ret != ERR_NO_ERROR )
				{
					Permit();
					return ret;
				}
				Permit();

				i=1;
				for(j=0;j<MAX_CACHE_SECTOR;j++)
				{
					sector_pos[j]=0xFFFF;
				}

				for(j=0;j<9;j++)
				{
					while(i < RD_TRACK_BUFFER_SIZE && ( track_buffer_rd[i]!=0x4489 ))
						i++;

					if( i == RD_TRACK_BUFFER_SIZE )
						break;

					while(i < RD_TRACK_BUFFER_SIZE && (track_buffer_rd[i]==0x4489 ))
						i++;

					if( i == RD_TRACK_BUFFER_SIZE)
						break;

					if(MFMTOBIN(track_buffer_rd[i])==0xFE)
					{
						#ifdef DEBUG
						dbg_printf("pre-cache sector : index mark at %d sector %d\n",i,MFMTOBIN(track_buffer_rd[i+3]));
						#endif

						sect_num = MFMTOBIN(track_buffer_rd[i+3]);
						if( sect_num < MAX_CACHE_SECTOR )
						{
							if(sector_pos[sect_num] == 0xFFFF)
							{
								if( i < (RD_TRACK_BUFFER_SIZE - 1088))
								{
									sector_pos[sect_num] = i;
									#ifdef DEBUG
									dbg_printf("pre-cache sector : %d - %d\n",sect_num,i);
									#endif
								}

							}
							else
							{
								#ifdef DEBUG
								dbg_printf("pre-cache sector : sector already found : %d sector %d\n",i,sector_pos[sect_num]);
								#endif
							}
						}

						i += ( 512 + 2 );
					}
					else
					{
						i++;
					}
				}
			}

			i = sector_pos[sectornum];

			#ifdef DEBUG
			dbg_printf("sector %d offset %d\n",sectornum,i);
			#endif

			if( i < (RD_TRACK_BUFFER_SIZE - 1088))
			{
				// Check if we have a valid sector header
				j = 0;
				while(j<7 && ( MFMTOBIN(track_buffer_rd[i+j]) == sector_header[j] ) ) // track,side,sector
				{
					j++;
				}

				if(j == 7) // yes
				{
					#ifdef DEBUG
					dbg_printf("Valid header found\n");
					#endif

					i += 35;

					j = 0;
					while(j<30 && ( MFMTOBIN(track_buffer_rd[i]) != 0xFB ) ) // Data mark
					{
						i++;
						j++;
					}

					if(j != 30)
					{
						#ifdef DEBUG
						dbg_printf("Data mark found (%d)\n",j);
						#endif

						// 0xA1 * 3
						crc16v = 0xFFFF;
						for(j=0;j<3;j++)
							crc16v = crc16( 0xA1 , crc16v );

						// Data Mark
						crc16v = crc16( MFMTOBIN(track_buffer_rd[i]) , crc16v );
						i++;

						// Data
						for(j=0;j<512;j++)
						{
							tc = MFMTOBIN(track_buffer_rd[i]);
							i++;
							data[j] = tc;
						}

						crc16v = crc16_buf(data, 512, crc16v);

						for(j=0;j<2;j++)
						{
							c = MFMTOBIN( track_buffer_rd[i] );
							crc16v = crc16( c, crc16v );
							i++;
						}

						if(!crc16v)
						{
							sectorfound=1;
						}
						else
						{
							badcrc=1;
						}
					}
				}
			}

			retry--;
			if(!sectorfound && retry)
			{
				validcache=0;
			}

		}while(!sectorfound && retry);


		if(!sectorfound)
		{
			ret = jumptotrack(255);
			if( ret != ERR_NO_ERROR)
			{
				return ret;
			}

			retry2--;
			retry=5;
		}

	}while(!sectorfound && retry2);

	if(!sectorfound)
	{
		validcache=0;
	}

	if(sectorfound)
		return -ERR_NO_ERROR;
	else
		return -ERR_MEDIA_READ_SECTOR_NOT_FOUND;
}

static void setnoclick(ULONG unitnum, ULONG onoff)
{
	struct MsgPort *port;

	port = CreatePort(0,0);
	if (port)
	{
		struct IOStdReq *ioreq;
		ioreq = LCreateIORequest(port, sizeof(*ioreq));
		if (ioreq)
		{
			if (OpenDevice((CONST_STRPTR)TD_NAME, unitnum, (APTR) ioreq, 0) == 0)
			{
				struct TDU_PublicUnit *unit = (APTR) ioreq->io_Unit;

				Forbid();

				if (onoff)
					unit->tdu_PubFlags |= TDPF_NOCLICK;
				else
					unit->tdu_PubFlags &= ~TDPF_NOCLICK;

				// Hack to fully disable the click...
				unit->tdu_SettleDelay = 0x7FFFFFFF;
				unit->tdu_StepDelay = 0x7FFFFFFF;
				unit->tdu_CalibrateDelay = 0x7FFFFFFF;

				Permit();
				CloseDevice((APTR) ioreq);
			}

			FreeMem(ioreq,sizeof(*ioreq));
		}

		FreeMem(port,sizeof(struct MsgPort));
	}
}

int init_fdc(int drive)
{
	int ret;
	unsigned short i;

	#ifdef DEBUG
	dbg_printf("init_fdc\n");
	#endif

	for(i=0;i<4;i++)
		setnoclick(i,0);

	CIABPRB_DSKSEL = CIABPRB_DSKSEL0 << (drive&3);

	if( test_drive(drive) != ERR_NO_ERROR )
		return -ERR_DRIVE_NOT_FOUND;

	validcache=0;

	mfmtobinLUT_L = (unsigned char*)AllocMem(256,MEMF_CHIP);
	mfmtobinLUT_H = (unsigned char*)AllocMem(256,MEMF_CHIP);
	if(mfmtobinLUT_L && mfmtobinLUT_H)
	{
		for(i=0;i<256;i++)
		{
			mfmtobinLUT_L[i] =   ((i&0x40)>>3) | ((i&0x10)>>2) | ((i&0x04)>>1) | (i&0x01);
			mfmtobinLUT_H[i] =   mfmtobinLUT_L[i] << 4;
		}
	}
	else
	{
		if(mfmtobinLUT_L)
			FreeMem(mfmtobinLUT_L,256);

		if(mfmtobinLUT_H)
			FreeMem(mfmtobinLUT_H,256);

		return -ERR_MEM_ALLOC;
	}

	track_buffer_rd = (unsigned short*)AllocMem( sizeof(unsigned short) * RD_TRACK_BUFFER_SIZE, MEMF_CHIP);
	if(track_buffer_rd)
	{
		memset(track_buffer_rd,0,sizeof(unsigned short) * RD_TRACK_BUFFER_SIZE);
	}
	else
	{
		FreeMem(mfmtobinLUT_L,256);
		FreeMem(mfmtobinLUT_H,256);

		return -ERR_MEM_ALLOC;
	}

	track_buffer_wr=(unsigned short*)AllocMem( sizeof(unsigned short) * WR_TRACK_BUFFER_SIZE,MEMF_CHIP);
	if(track_buffer_wr)
	{
		memset(track_buffer_wr,0, sizeof(unsigned short) * WR_TRACK_BUFFER_SIZE);
	}
	else
	{
		FreeMem(mfmtobinLUT_L,256);
		FreeMem(mfmtobinLUT_H,256);
		FreeMem(track_buffer_rd, sizeof(unsigned short) * RD_TRACK_BUFFER_SIZE);
		return -ERR_MEM_ALLOC;
	}

	Forbid();

	WRITEREG_B(CIABPRB,~(CIABPRB_DSKMOTOR | CIABPRB_DSKSEL));
	WRITEREG_W( DMACON,0x8210);

	ret = jumptotrack(255);
	if( ret != ERR_NO_ERROR )
	{
		Permit();
		return ret;
	}

	Delay(12);
	WRITEREG_W(INTREQ,0x0002);

	Permit();

	return ERR_NO_ERROR;
}

void deinit_fdc()
{
	jumptotrack(40);

	if(mfmtobinLUT_L)
	{
		FreeMem(mfmtobinLUT_L,256);
		mfmtobinLUT_L = 0;
	}

	if(mfmtobinLUT_H)
	{
		FreeMem(mfmtobinLUT_H,256);
		mfmtobinLUT_H = 0;
	}

	if(track_buffer_rd)
	{
		FreeMem(track_buffer_rd,sizeof(unsigned short) * RD_TRACK_BUFFER_SIZE);
		track_buffer_rd = 0;
	}

	if(track_buffer_wr)
	{
		FreeMem(track_buffer_wr, sizeof(unsigned short) * WR_TRACK_BUFFER_SIZE);
		track_buffer_wr = 0;
	}
}

/********************************************************************************
*                          Joystick / Keyboard I/O
*********************************************************************************/

unsigned char Joystick()
{
	unsigned short code;
	unsigned char bcode;
	unsigned char ret;

	/* Get a copy of the SDR value and invert it: */
	code = READREG_W(0xDFF00C);
	bcode = READREG_B(CIAAPRA);

	ret=0;
	if( (code&0x100) ^ ((code&0x200)>>1) ) // Forward
	{
		ret=ret| 0x1;
	}
	if( ((code&0x200)) )  // Left
	{
		ret=ret| 0x8;
	}

	if( (code&0x1) ^ ((code&0x2)>>1) ) // Back
	{
		ret=ret| 0x2;
	}

	if( ((code&0x002)) )  // Right
	{
		ret=ret| 0x4;
	}

	if(!(bcode&0x80))
	{
		ret=ret| 0x10;
	}

	return( ret );
}

unsigned char Keyboard()
{
	unsigned char code;

	/* Get a copy of the SDR value and invert it: */
	code = READREG_B(0xBFEC01) ^ 0xFF;

	/* Shift all bits one step to the right, and put the bit that is */
	/* pushed out last: 76543210 -> 07654321                         */
	code = code & 0x01 ? (code>>1)+0x80 : code>>1;

	/* Return the Raw Key Code Value: */
	return( code );
}

unsigned char get_char()
{
	unsigned char key,i,c;
	unsigned char function_code,key_code;

	function_code=FCT_NO_FUNCTION;
	while(!(Keyboard()&0x80));

	do
	{
		c=1;
		do
		{
			do
			{
				key=Keyboard();
				if(key&0x80)
				{
					c=1;
				}
			}while(key&0x80);
			waitms(55);
			c--;

		}while(c);

		i=0;
		do
		{
			function_code=char_keysmap[i].function_code;
			key_code=char_keysmap[i].keyboard_code;
			i++;
		}while((key_code!=key) && (function_code!=FCT_NO_FUNCTION) );

	}while(function_code==FCT_NO_FUNCTION);

	return function_code;
}

unsigned char wait_function_key()
{
	unsigned char key,joy,i,c;
	unsigned char function_code,key_code;

	function_code=FCT_NO_FUNCTION;

	if( keyup == 1 )
	{
		waitms(250);
	}

	do
	{
		c=1;
		do
		{
			do
			{
				key=Keyboard();
				joy=Joystick();
				if(key&0x80 && !joy)
				{
					c=1;

					keyup = 2;
				}
			}while(key&0x80 && !joy);

			waitms(55);

			c--;

		}while(c);

		if(keyup)
			keyup--;

		if(joy)
		{
			if(joy&0x10)
			{
				while(Joystick()&0x10);

				return FCT_SELECT_FILE_DRIVEA;
			}
			if(joy&2)
				return FCT_DOWN_KEY;
			if(joy&1)
				return FCT_UP_KEY;
			if(joy&4)
				return FCT_RIGHT_KEY;
			if(joy&8)
				return FCT_LEFT_KEY;
		}

		i=0;
		do
		{
			function_code=keysmap[i].function_code;
			key_code=keysmap[i].keyboard_code;
			i++;
		}while((key_code!=key) && (function_code!=FCT_NO_FUNCTION) );

	}while(function_code==FCT_NO_FUNCTION);

	return function_code;
}

void ithandler(void)
{
	timercnt++;

	io_floppy_timeout++;

	if( ( Keyboard() & 0x80 )  && !Joystick())
	{
		keyup  = 2;
	}
}

void init_timer()
{
	rbfint = AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC|MEMF_CLEAR);
	rbfint->is_Node.ln_Type = NT_INTERRUPT;      /* Init interrupt node. */
	rbfint->is_Node.ln_Name = "HxCFESelectorTimerInt";
	rbfint->is_Data = 0;//(APTR)rbfdata;
	rbfint->is_Code = ithandler;

	AddIntServer(5,rbfint);
}

/********************************************************************************
*                              Display Output
*********************************************************************************/

int init_display(ui_context * ctx)
{
	unsigned short loop;
	font_type * font;

	ctx->SCREEN_XRESOL = 640;

	memset(&view,0,sizeof(struct View));
	memset(&viewPort,0,sizeof(struct ViewPort));
	memset(&rasInfo,0,sizeof(struct RasInfo));
	memset(&my_bit_map,0,sizeof(struct BitMap));
	memset(&my_rast_port,0,sizeof(struct RastPort));

	IntuitionBase = (struct IntuitionBase *) OpenLibrary( (CONST_STRPTR)"intuition.library", 0 );
	screen = (struct Screen *)OpenScreen(&screen_cfg);

	/* Open the Graphics library: */
	GfxBaseptr = (struct GfxBase *) OpenLibrary( (CONST_STRPTR)"graphics.library", 0 );
	if( !GfxBaseptr )
		return -ERR_SYSLIB_LOAD;

	/* Save the current View, so we can restore it later: */
	my_old_view = GfxBaseptr->ActiView;

	/* 1. Prepare the View structure, and give it a pointer to */
	/*    the first ViewPort:                                  */
	InitView( &view );
	view.Modes |= HIRES;//LACE;

	/* 4. Prepare the BitMap: */
	InitBitMap( &my_bit_map, DEPTH, ctx->SCREEN_XRESOL, 256 );

	/* Allocate memory for the Raster: */
	for( loop = 0; loop < DEPTH; loop++ )
	{
		my_bit_map.Planes[ loop ] = (PLANEPTR) AllocRaster( ctx->SCREEN_XRESOL, 256 );
		BltClear( my_bit_map.Planes[ loop ], RASSIZE( ctx->SCREEN_XRESOL, 256 ), 0 );
	}

	/* 5. Prepare the RasInfo structure: */
	rasInfo.BitMap = &my_bit_map; /* Pointer to the BitMap structure.  */
	rasInfo.RxOffset = 0;         /* The top left corner of the Raster */
	rasInfo.RyOffset = 0;         /* should be at the top left corner  */
	/* of the display.                   */
	rasInfo.Next = NULL;          /* Single playfield - only one       */
	/* RasInfo structure is necessary.   */

	InitVPort(&viewPort);           /*  Initialize the ViewPort.  */
	view.ViewPort = &viewPort;      /*  Link the ViewPort into the View.  */
	viewPort.RasInfo = &rasInfo;
	viewPort.DWidth = ctx->SCREEN_XRESOL;
	viewPort.DHeight = 256;

	/* Set the display mode the old-fashioned way */
	viewPort.Modes=HIRES;// | LACE;

	cm =(struct ColorMap *) GetColorMap(COLOURS);

	/* Attach the ColorMap, old 1.3-style */
	viewPort.ColorMap = cm;

	LoadRGB4(&viewPort, colortable, 4);

	/* 6. Create the display: */
	MakeVPort( &view, &viewPort );
	MrgCop( &view );
	LoadView( &view );
	WaitTOF();
	WaitTOF();

	/* 7. Prepare the RastPort, and give it a pointer to the BitMap. */
	InitRastPort( &my_rast_port );
	my_rast_port.BitMap = &my_bit_map;
	SetAPen( &my_rast_port,   1 );
	screen_buffer = my_bit_map.Planes[ 0 ];

	ctx->SCREEN_YRESOL = (GfxBaseptr->DisplayFlags & PAL) ? 256 : 200;

	font = font_list[ctx->font_id];

	ctx->screen_txt_xsize = ctx->SCREEN_XRESOL / font->char_x_size;
	ctx->screen_txt_ysize = (ctx->SCREEN_YRESOL / font->char_y_size);

	disablemousepointer();
	init_timer();

	return ERR_NO_ERROR;
}

void patch_char_func(int numberoflines)
{
	volatile unsigned short * func_ptr;
	int i;
	if( numberoflines > 4 && numberoflines <= 16)
	{
		///////////////////////////////////////////

		func_ptr = (unsigned short*)&print_char;

		for(i = 0; i < numberoflines;i++)
		{
			func_ptr[5 + (i * 2) + 0] = 0x1298;    // move.b  (a0)+,(a1)
			func_ptr[5 + (i * 2) + 1] = 0xD3C0;    // add.l   d0,a1
		}

		func_ptr[5 + (i * 2) + 0] = 0x4E75;        // rts

		///////////////////////////////////////////

		func_ptr = (unsigned short*)&print_inv_char;

		for(i = 0; i < numberoflines;i++)
		{
			func_ptr[5 + (i * 4) + 0] = 0x1218;    // move.b  (a0)+,d1
			func_ptr[5 + (i * 4) + 1] = 0x4601;    // not.b   d1
			func_ptr[5 + (i * 4) + 2] = 0x1281;    // move.b  d1,(a1)
			func_ptr[5 + (i * 4) + 3] = 0xD3C0;    // add.l   d0,a1
		}

		func_ptr[5 + (i * 4) + 0] = 0x4E75;        // rts
	}

	// Invalidate instruction cache.
	if( GetLibraryVersion((struct Library *) SysBase) >= 37 )
	{
		CacheClearU();
	}
}

void chg_video_conf(ui_context * ctx)
{
	font_type * font;

	font = font_list[ctx->font_id];

	patch_char_func(font->char_y_size);

	bytes_per_line = font->char_y_size * 80;
}

void DestroyScrn ()
{
	WORD Cntr;

	if (view.LOFCprList)
		FreeCprList(view.LOFCprList);

	if (view.SHFCprList)
		FreeCprList(view.SHFCprList);

	FreeVPortCopLists(&viewPort);

	if (cm)
		FreeColorMap(cm);

	for (Cntr = 0; Cntr < 8; Cntr++)
	{
		if (my_bit_map.Planes[Cntr])
			FreeRaster (my_bit_map.Planes[Cntr], 640, 480);
	}
}

void disablemousepointer()
{
	WRITEREG_W( DMACON ,0x20);
}

void initpal()
{
	volatile unsigned short * ptr;

	ptr=(unsigned short *)0xFF8240;
	*ptr=0x0000;
	ptr=(unsigned short *)0xFF8242;
	*ptr=0x0070;
	ptr=(unsigned short *)0xFF8244;
	*ptr=0x0700;
	ptr=(unsigned short *)0xFF8246;
	*ptr=0x0777;

}

unsigned char set_color_scheme(unsigned char color)
{
	LoadRGB4(&viewPort, &colortable[(color&0x1F)*4], 4);
	return color;
}

void print_char8x8(ui_context * ctx,int col, int line, unsigned char c, int mode)
{
	unsigned char *ptr_dst;
	const unsigned char * char_data;
	font_type * font;

	font = font_list[ctx->font_id];

	if(col < ctx->screen_txt_xsize && line < ctx->screen_txt_ysize)
	{
		ptr_dst  = screen_buffer + (( line * bytes_per_line ) + col);
		char_data = font->font_data + (c * font->char_size);

		if(mode & INVERTED)
		{
			print_inv_char((void*)ptr_dst, (void*)char_data);
		}
		else
		{
			print_char((void*)ptr_dst, (void*)char_data);
		}
	}
}

void clear_line(ui_context * ctx,int line,int mode)
{
	unsigned short *ptr_dst;
	font_type * font;

	font = font_list[ctx->font_id];

	if(line < ctx->screen_txt_ysize)
	{
		ptr_dst  = (unsigned short *)(screen_buffer + ( line * bytes_per_line ));

		if(mode & INVERTED)
		{
			fast_clear_line( ptr_dst, 0xFFFFFFFF, font->char_y_size );
		}
		else
		{
			fast_clear_line( ptr_dst, 0x00000000, font->char_y_size );
		}
	}
}

void invert_line(ui_context * ctx,int line)
{
	unsigned short *ptr_dst;
	font_type * font;

	font = font_list[ctx->font_id];

	if(line < ctx->screen_txt_ysize)
	{
		ptr_dst  = (unsigned short *)(screen_buffer + ( line * bytes_per_line ));

		fast_inverse_line( ptr_dst, font->char_y_size);
	}
}

void reboot()
{
	_reboot();
	lockup();
}

int process_command_line(int argc, char* argv[])
{
	return 0;
}
