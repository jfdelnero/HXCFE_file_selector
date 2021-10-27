/*
//
// Copyright (C) 2009-2021 Jean-Fran�ois DEL NERO
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <mint/osbind.h>
#include <mint/ostruct.h>
#include <time.h>
#include <vt52.h>


#ifdef __VBCC__
# include <tos.h>
#else
# include <mint/osbind.h>
# include <mint/linea.h>
# include "libc/snprintf/snprintf.h"
#endif

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

#include "cache.h"

#include "atari_hw.h"

#include "errors_def.h"

# define _hz_200  ((unsigned long *) 0x4baL)

static unsigned char floppydrive;
static unsigned char datacache[512*9];
static unsigned char valid_cache;
unsigned char g_color;
volatile unsigned long g_hz200;
unsigned char g_trackpos;

volatile unsigned char g_joydata[3];

unsigned long old_physical_adr;

volatile unsigned short io_floppy_timeout;

unsigned char * screen_buffer;

static short  _oldrez = 0xffff;

unsigned short LINE_BYTES;					/* number of bytes per line     */
unsigned short LINE_WORDS;					/* number of words per line     */
unsigned short NB_PLANES;					/* number of planes (1:2 colors */
											/*  4:16 colors, 8: 256 colors) */
unsigned short CHUNK_WORDS;					/* number of words for a 16-    */
											/* pixel chunk =2*NB_PLANES     */
unsigned short PLANES_ALIGNDEC;				/* number of left shifts to
											   transform nbChucks to Bytes  */
unsigned short NB_WORDS_PER_TXT_LINE;
static unsigned short bytes_per_line;
static unsigned char * chars_LUT[256];

static unsigned short backup_pal[16];

__LINEA *__aline;
__FONT  **__fonts;
short  (**__funcs) (void);

unsigned char keyup;

unsigned long timercnt;

WORD fdcDmaMode = 0;

extern ui_context g_ui_ctx;
_KEYTAB * keyboard_map;

#ifdef DEBUG

void push_serial_char(unsigned char byte)
{
	while(!Bcostat(1));

	Bconout(1,byte);
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
}

/*
-------+-----+-----------------------------------------------------+----------
       |     |                                BIT 11111198 76543210|
       |     |                                    543210           |
       |     |                     ST color value .....RRr .GGr.BBb|
       |     |                    STe color value ....rRRR gGGGbBBB|
$FF8240|word |Video palette register 0              Lowercase = LSB|R/W
    :  |  :  |  :      :       :     :                             | :
$FF825E|word |Video palette register 15                            |R/W
-------+-----+-----------------------------------------------------+----------
*/

static unsigned short colortable[] = {
	0x002, 0xeee, 0x226, 0x567, // b ok blanc sur bleu nuit (nice)
	0x300, 0xEEE, 0x00f, 0xee4, // b ok blanc sur rouge fonc� (nice)
	0x777, 0x300, 0x00f, 0x5f5, // w noir sur blanc, select vert (nice)
	0xFFF, 0x343, 0x00f, 0x0f0, // w ok vert sombre sur blanc, select vert
	0x000, 0x00F, 0x222, 0xdd1, // b ok bleu sur noir
	0x000, 0xFFF, 0x00f, 0x3f3, // b ok blanc sur noir, select vert
	0x303, 0xFFF, 0x00f, 0xee4, // w ok blanc sur mauve
	0x030, 0xFFF, 0x00f, 0x0f0, // b ok vert
	0x999, 0x000, 0x999, 0x333, // w ok gris sombre
	0x330, 0xFFF, 0x77f, 0xcc0, // b ok caca d'oie
	0xF33, 0xFFF, 0x777, 0xe11, // w ok blanc sur rose et rouge
	0x000, 0xF00, 0x003, 0xd00, // b ok rouge sur noir
	0xF0F, 0xFFF, 0x000, 0x44f, // w ok violet vif
	0x000, 0x0E0, 0x00f, 0x0f0, // b ok vert sur noir
	0xFFF, 0x0F0, 0x4c4, 0x0f0, // w ok vert sur blanc
	0x004, 0xFFF, 0x00e, 0x5f5, // b ok blanc sur bleu marine

	0x036, 0xFFF, 0x00f, 0x0f0, // b
	0x444, 0x037, 0x00f, 0x0f0, // b
	0x000, 0xFF0, 0x00f, 0x0f0, // b
	0x404, 0x743, 0x00f, 0x0f0, // b
	0xFFF, 0x700, 0x00f, 0x0f0, // w
	0x000, 0x222, 0x00f, 0x0c0, // b
	0x000, 0x333, 0x00f, 0x0d0, // b
	0x000, 0x444, 0x00f, 0x0e0, // b
	0x000, 0x555, 0x00f, 0x0f0, // b
	0x000, 0x666, 0x00f, 0x0f0, // b
	0x000, 0x777, 0x00f, 0x0f0, // b
	0x222, 0x000, 0x00f, 0x0c0, // b
	0x333, 0x000, 0x00f, 0x0d0, // w
	0x444, 0x000, 0x00f, 0x0e0, // b
	0x555, 0x000, 0x00f, 0x0f0, // w
	0x666, 0x000, 0x00f, 0x0f0  // b
};

#if 0
#define _cookie_base   (0x5a0L)

static unsigned long read_cookie(unsigned long needed_cookie)
{
	unsigned long *cookie_ptr;
	unsigned long cookie_value;

	cookie_ptr = (unsigned long*)( Setexc( _cookie_base >> 2, (const void (*)(void))-1) ); // cookiejar pointer
	cookie_value = NULL;

	if(cookie_ptr != 0)
	{
		while( (*cookie_ptr != needed_cookie) && (*cookie_ptr != 0) )
		{
			cookie_ptr += 2;
		}

		if(*cookie_ptr == needed_cookie)
		{
			cookie_value = *(cookie_ptr + 1);
			return cookie_value;    // Cookie found.
		}
	}

	return cookie_value;
}
#endif

void su_get_hz200(void)
{
	g_hz200 = *_hz_200;
	return;
}

void waitms(int  ms)
{
	if(ms < 5)
		ms = 5;

	ms = ms / 5;

	Supexec(su_get_hz200);
	ms = g_hz200 + ms;
	while(g_hz200 < ms)
	{
		Supexec(su_get_hz200);
	}
}

void waitsec(int secs)
{
	int i;

	for(i=0;i<secs;i++)
	{
		waitms(1000);
	}
}

void lockup()
{
	int i;

	#ifdef DEBUG
	dbg_printf("lockup : Sofware halted...\n");
	#endif

	i = 0;
	for(;;)
	{
		if( i >= 4 && Fgetdta ())
		{
			// We are not trackloaded ! return to the system after some seconds.
			return_to_system(&g_ui_ctx);
		}

		waitsec(1);

		i++;
	}
}

/********************************************************************************
*                              FDC I/O
*********************************************************************************/

int get_start_unit(char * path)
{
	return 0;
}

#ifdef __VBCC__
void asm_nop(void) = "\tnop\n";
#else
#define asm_nop(void) __asm("nop");
#endif

#ifndef FDC_TOSAPI

void su_fdcRegSet(WORD reg, WORD data)
{
	DMA->control = reg | fdcDmaMode;

	asm_nop();
	asm_nop();
	DMA->data    = data;
	asm_nop();
	asm_nop();
}

void su_fdcSendCommandWait(WORD command)
{
	MFP *mfp = MFP_BASE;
	su_fdcRegSet(0x80, command);

	while (0x20 == (mfp->gpip & 0x20));       /* wait till the next interrupt */
}

WORD su_fdcRegGet(WORD reg)
{
	DMA->control = reg | fdcDmaMode;
	asm_nop();
	asm_nop();
	return DMA->data;
}

void su_fdcWait(void)
{
	DMA->control = 0x80 | fdcDmaMode;

	while (FDC_BUSY == (DMA->data & FDC_BUSY));
}

void su_fdcSelectDriveASide0()
{
	UBYTE data;
#ifdef __VBCC__
	__asm("\tmove.w sr,-(a7)\n");
	__asm("\tor.w #$700,sr\n");
#else
	__asm("\tmove.w sr,-(a7)\n"
		  "\tor.w #0x700,sr\n"
		   :::"%sp"
		 );
#endif

	PSG->regdata = 14;      /* select register 14 */
	data = PSG->regdata;
	data = data & 0xf8;     /* clear bits */
	if (0 == floppydrive)
	{
		data = data | 5;        /* select drive A, side 0 */
	} else {
		data = data | 3;        /* select drive B, side 0 */
	}

	PSG->write = data;

#ifdef __VBCC__
	__asm("\tmove.w (a7)+,sr\n");
#else
	__asm("\tmove.w (a7)+,sr\n" ::: "%sp");
#endif

}

void su_fdcLock(void)
{
/*
Set floppy lock, so the system VBL won't interfere
inhibit MFP interrupt
set the bit 5 of fffa01 as input (0)
enable changing of bit 5 of fffa01 (gpip) to poll the interrupt bit of the WDC
see the steem source at
https://github.com/btuduri/Steem-Engine/blob/629d8b98df7245c8645b0ad41f90ed395d427531/steem/code/iow.cpp
*/

#ifdef __VBCC__
	__asm("\tst $43e.w\n");
	__asm("\tbclr #7,$fffffa09.w\n");
	__asm("\tbclr #5, $fffffa05.w\n");
#else
	__asm("\tst 0x43e.w\n");
	__asm("\tbclr #7,0xfffffa09.w\n");
	__asm("\tbclr #5, 0xfffffa05.w\n");
#endif

	su_fdcWait();
}
void su_fdcUnlock(void)
{
	su_fdcWait();

#ifdef __VBCC__
	__asm("\tsf $43e.w\n");
#else
	__asm("\tsf 0x43e.w\n");
#endif
}

void su_headinit(void)
{
	su_fdcLock();
	su_fdcSelectDriveASide0();
	su_fdcRegSet(0x86, 255);        /* data : track number */
	su_fdcSendCommandWait(0x13);    /* SEEK, no verify, 3ms */
/*    su_fdcUnlock(); */
/*    Crawcin(); */
}

void su_jumptotrack(void)
{
	su_fdcLock();
	su_fdcRegSet(0x86, g_trackpos);          /* data : track number */
	su_fdcSendCommandWait(0x13);    /* SEEK, no verify, 3ms */
	su_fdcUnlock();
}

void su_fdcDmaAdrSet(unsigned char *adr)
{
	DMA->addr_low  = ((unsigned long) adr) & 0xff;
	DMA->addr_med  = ((unsigned long) adr>>8) & 0xff;
	DMA->addr_high = ((unsigned long) adr>>16) & 0xff;
}
void su_fdcDmaReadMode(void)
{
	DMA->control = 0x90;
	DMA->control = 0x190;
	DMA->control = 0x90;
	fdcDmaMode = 0x0;
}
void su_fdcDmaWriteMode(void)
{
	DMA->control = 0x190;
	DMA->control = 0x90;
	DMA->control = 0x190;
	fdcDmaMode = 0x100;
}

unsigned char read9sectors(unsigned char *adr)
{
	void * old_ssp;
	WORD sectorNumber;
	unsigned char status;
	int retry;

	old_ssp = (void *) Super(0L);

	retry = 10;

	do
	{
		su_fdcDmaReadMode();
		su_fdcDmaAdrSet(adr);
		su_fdcRegSet(0x90, 9);                   /* sector count : 9 sectors */

		for (sectorNumber = 0; sectorNumber<=8; sectorNumber++)
		{
			su_fdcRegSet(0x84, sectorNumber);
			su_fdcSendCommandWait(0x88);         /* READ SECTOR, no spinup */

			status = su_fdcRegGet(0x80); // Status register - Check CRC & DRQ issue flags;

			if(status & 0x1C)
				break;                   // CRC and/or DMA error
		}

		retry--;

	} while( retry && (status & 0x1C) );

	Super(old_ssp);

	return status & 0x1C;
}

void write1sector(WORD sectorNumber, unsigned char *adr)
{
	void *old_ssp;

	old_ssp = (void *) Super(0L);

	su_fdcDmaWriteMode();
	su_fdcDmaAdrSet(adr);
	su_fdcRegSet(0x90, 1);              /* sector count : 1 sector */

	su_fdcRegSet(0x84, sectorNumber);
	su_fdcSendCommandWait(0xa8);        /* WRITE SECTOR, no spinup */

	Super(old_ssp);
}

int writesector(unsigned char sectornum,unsigned char * data)
{
	valid_cache=0;

	write1sector(sectornum, data);

	return ERR_NO_ERROR;
}

int readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache)
{
	if(!valid_cache || invalidate_cache)
	{
		if(read9sectors(datacache))
		{
			return ERR_MEDIA_READ;
		}

		valid_cache=0xFF;
	}

	memcpy((void*)data,&datacache[sectornum*512],512);

	return ERR_NO_ERROR;
}

int jumptotrack(unsigned char t)
{
	g_trackpos = t;
	Supexec((LONG *) su_jumptotrack);

	return ERR_NO_ERROR;
};

int init_fdc(int drive)
{
	#ifdef DEBUG
	dbg_printf("init_fdc\n");
	#endif

	valid_cache = 0;
	floppydrive = drive;
	Supexec((LONG *) su_headinit);

	return ERR_NO_ERROR;
}

void deinit_fdc()
{
	jumptotrack(0);
}

#else

int jumptotrack(unsigned char t)
{
	unsigned char data[512];

	#ifdef DEBUG
	dbg_printf("jumptotrack : %d\n",t);
	#endif

	Floprd( &data, 0, floppydrive, 1, t, 0, 1 );

	return ERR_NO_ERROR;
};

int writesector(unsigned char sectornum,unsigned char * data)
{
	int ret,retry;

	#ifdef DEBUG
	dbg_printf("writesector : %d\n",sectornum);
	#endif

	valid_cache =0;
	retry=3;

	ret=1;
	while(retry && ret)
	{
		ret = Flopwr( data, 0, floppydrive, sectornum, 255, 0, 1 );
		retry--;
	}

	if(!ret)
		return ERR_NO_ERROR;
	else
		return -ERR_MEDIA_WRITE;
}

int readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache)
{
	int ret,retry;

	#ifdef DEBUG
	dbg_printf("readsector : %d - %d\n",sectornum,invalidate_cache);
	#endif

	retry=3;
	ret=0;
	if(!valid_cache || invalidate_cache)
	{
		if(sectornum<10)
		{
			ret=1;
			while(retry && ret)
			{
				ret=Floprd( datacache, 0, floppydrive, 0, 255, 0, 9 );
				retry--;
			}

			memcpy((void*)data,&datacache[sectornum*512],512);
			valid_cache=0xFF;
		}
	}
	else
	{
		memcpy((void*)data,&datacache[sectornum*512],512);
	}

	if(!ret)
		return ERR_NO_ERROR;
	else
		return -ERR_MEDIA_READ;
}

int init_fdc(int drive)
{
	valid_cache = 0;
	floppydrive = drive;
	Floprate( floppydrive, 2);

	Floprd( &datacache, 0, floppydrive, 0, 255, 0, 1 );

	return ERR_NO_ERROR;
}

#endif

/********************************************************************************
*                          Joystick / Keyboard I/O
*********************************************************************************/

void su_toggleConterm()
{
	#define CONTERM *((unsigned char *) 0x484)

	static unsigned char oldconterm = 0xff;
	if (0xff == oldconterm) {
		oldconterm = CONTERM;
		CONTERM &= 0xFA;				/* disable key sound and bell */
		CONTERM |= 8;					/* enable keyboard function to return shift/alt/ctrl status */
	} else {
		CONTERM = oldconterm;
	}
}

unsigned char Joystick()
{
	unsigned char joystick;

	joystick = 0;
	if( (g_joydata[2]&0x80) ) // Fire
		joystick |= 0x10;

	if((g_joydata[2]&0x02) )  // Down
		joystick |= 0x02;

	if( (g_joydata[2]&0x01) ) // Up
		joystick |= 0x01;

	if( (g_joydata[2]&0x08) ) // Right
		joystick |= 0x04;

	if( (g_joydata[2]&0x04) ) // Left
		joystick |= 0x08;

	return joystick;
}

unsigned char Keyboard()
{
	unsigned char c;
	if ( Cconis() < 0 )
	{
		do
		{
			c = Cnecin()>>16;
		}while( Cconis() < 0 ); // Flush the buffer...
		return c;
	}

	return 0x80;
}

unsigned char get_char()
{
	unsigned char key,c;
	unsigned char function_code;
	unsigned char shiftkey;

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

		if( (key == 28) || (key == 114) ) // Return
			return '\n';

		if( key == 14 ) // Backspace -> del
			return 127;

		shiftkey = Kbshift(-1);

		if( shiftkey & 0x03)
			function_code = keyboard_map->shift[key];
		else
			function_code = keyboard_map->unshift[key];

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

/********************************************************************************
*                              Display Output
*********************************************************************************/

unsigned char set_color_scheme(unsigned char color)
{
	unsigned short * palette;
	int i,j;
	int nbcols;

	palette = &colortable[(color<<2)&0x1F];
	nbcols = 2<<(NB_PLANES-1);

	g_color = color & 0x1F;

	for (i=0; i<4 && i<nbcols; i++) {
		j = i;
		if (i>=2) {
			// the first two colors are always pal[0] and pal[1]
			// the last two colors may be pal[2] and pal[3] in 2 planes, or pal[14] and pal[15] in 4 planes
			j = nbcols - 4 + i;
		}

		Setcolor(j, palette[i]);
	}

	return g_color;
}

void joystick_reader(char *packet)
{
	// Get the Joystick packet
	g_joydata[0] = *packet++; // 0xFF / 0xFE
	g_joydata[1] = *packet++; // Joy 0 / Mouse
	g_joydata[2] = *packet;   // Joy 1
}

int install_joy_vector()
{
	_KBDVECS *table_addr;

	g_joydata[0] = 0;
	g_joydata[1] = 0;
	g_joydata[2] = 0;

	table_addr = Kbdvbase();

	table_addr->joyvec = (void*)joystick_reader;

	Ikbdws(1, "\024");

	return 0;
}

int  init_display(ui_context * ctx)
{
	unsigned long k,i;
	font_type * font;

	linea0();

	// Save the current palette
	for(i=0;i<16;i++)
	{
		backup_pal[i] = Setcolor( i , -1 );
	}

	// Line-A : Hidemouse
	// do not do : __asm__("dc.w 0xa00a"); (it clobbers registry)
	lineaa();

	if (V_X_MAX < 640) {
		/*Blitmode(1) */;
		_oldrez = Getrez();
		Setscreen((unsigned char *) -1, (unsigned char *) -1, 1 );
	}

	ctx->SCREEN_XRESOL = V_X_MAX;
	ctx->SCREEN_YRESOL = V_Y_MAX;

	LINE_BYTES    = V_BYTES_LIN;
	LINE_WORDS    = V_BYTES_LIN/2;
	NB_PLANES     = __aline->_VPLANES;
	CHUNK_WORDS   = NB_PLANES<<1;

	for (i=NB_PLANES, k=0; i!=0; i>>=1, k++);

	PLANES_ALIGNDEC = k;

	screen_buffer = (unsigned char*)Physbase();
	memset(screen_buffer, 0, ctx->SCREEN_YRESOL * LINE_BYTES);

	set_color_scheme(0);

	disablemousepointer();

	Supexec(su_toggleConterm);

	// Get the current key map.
	keyboard_map = Keytbl(-1,-1,-1);

	install_joy_vector();

	font = font_list[ctx->font_id];

	ctx->screen_txt_xsize = ctx->SCREEN_XRESOL / font->char_x_size;
	ctx->screen_txt_ysize = (ctx->SCREEN_YRESOL / font->char_y_size);

	return ERR_NO_ERROR;
}

void su_invalidate_cache()
{
	invalidate_icache();
}

void patch_char_func(int numberoflines)
{
	volatile unsigned short * func_ptr;
	int i;
	if( numberoflines > 4 && numberoflines <= 16)
	{
		///////////////////////////////////////////

		func_ptr = (unsigned short*)&fast_print_char;

		func_ptr[5] = 0x0000;
		func_ptr[6] = LINE_BYTES;                  // move.l  #80, d0

		for(i = 0; i < numberoflines;i++)
		{
			func_ptr[7 + (i * 2) + 0] = 0x1298;    // move.b  (a0)+,(a1)
			func_ptr[7 + (i * 2) + 1] = 0xD3C0;    // add.l   d0,a1
		}

		func_ptr[7 + (i * 2) + 0] = 0x4E75;        // rts

		///////////////////////////////////////////

		func_ptr = (unsigned short*)&fast_print_inv_char;

		func_ptr[5] = 0x0000;
		func_ptr[6] = LINE_BYTES;                  // move.l  #80, d0

		for(i = 0; i < numberoflines;i++)
		{
			func_ptr[7 + (i * 4) + 0] = 0x1218;    // move.b  (a0)+,d1
			func_ptr[7 + (i * 4) + 1] = 0x4601;    // not.b   d1
			func_ptr[7 + (i * 4) + 2] = 0x1281;    // move.b  d1,(a1)
			func_ptr[7 + (i * 4) + 3] = 0xD3C0;    // add.l   d0,a1
		}

		func_ptr[7 + (i * 4) + 0] = 0x4E75;        // rts
	}

	if (is_68020())
	{
		Supexec(su_invalidate_cache);
	}
}

void chg_video_conf(ui_context * ctx)
{
	font_type * font;
	int i;

	font = font_list[ctx->font_id];

	patch_char_func(font->char_y_size);

	bytes_per_line = font->char_y_size * LINE_BYTES;
	NB_WORDS_PER_TXT_LINE = ( LINE_WORDS / NB_PLANES ) * font->char_y_size;

	for(i=0;i<256;i++)
	{
		chars_LUT[i] = (unsigned char*)font->font_data;
	}

	for(i=0;i<font->nb_of_chars;i++)
	{
		chars_LUT[i] = (unsigned char*)(font->font_data + (i * font->char_size));
	}
}

void disablemousepointer()
{

}

void set_char_pos(ui_context * ctx,int col, int line)
{
	if(col < ctx->screen_txt_xsize && line < ctx->screen_txt_ysize)
	{
		ctx->char_ptr_col = col;
		ctx->char_ptr_line = line;
	}
	else
	{
		ctx->char_ptr_col = ctx->screen_txt_xsize - 1;
		ctx->char_ptr_line = ctx->screen_txt_ysize - 1;
	}

	ctx->vid_mem_ptr = screen_buffer + (ctx->char_ptr_line * bytes_per_line) + ((ctx->char_ptr_col>>1)<<PLANES_ALIGNDEC) + (ctx->char_ptr_col&1);
}

void print_char(ui_context * ctx, unsigned char c, int mode)
{
	if(mode & INVERTED)
	{
		fast_print_inv_char((void*)ctx->vid_mem_ptr, (void*)chars_LUT[c]);
	}
	else
	{
		fast_print_char((void*)ctx->vid_mem_ptr, (void*)chars_LUT[c]);
	}

	ctx->char_ptr_col++;

	if( ctx->char_ptr_col == ctx->screen_txt_xsize)
	{
		if(ctx->char_ptr_line < (ctx->screen_txt_ysize - 1))
		{
			ctx->char_ptr_col = 0;
			ctx->char_ptr_line++;
			ctx->vid_mem_ptr = screen_buffer + (ctx->char_ptr_line * bytes_per_line);
		}
	}
	else
	{
		if(ctx->char_ptr_col&1)
			ctx->vid_mem_ptr ++;
		else
			ctx->vid_mem_ptr += ((1<<PLANES_ALIGNDEC) - 1);
	}
}

void clear_line(ui_context * ctx,int line,int mode)
{
	void *ptr_dst;

	if(line < ctx->screen_txt_ysize)
	{
		ptr_dst  = (void *)(screen_buffer + ( line * bytes_per_line ));

		if(mode & INVERTED)
		{
			fast_clear_line( ptr_dst,0xFFFF,NB_WORDS_PER_TXT_LINE,NB_PLANES);
		}
		else
		{
			fast_clear_line( ptr_dst,0x0000,NB_WORDS_PER_TXT_LINE,NB_PLANES);
		}
	}
}

void invert_line(ui_context * ctx,int line)
{
	void *ptr_dst;

	ptr_dst  = (void *)(screen_buffer + ( line * bytes_per_line ));

	fast_inverse_line( ptr_dst, NB_WORDS_PER_TXT_LINE, NB_PLANES );
}

void su_reboot()
{
	if( Fgetdta () )
	{
		// We are not trackloaded ! just return to the system.

		return_to_system(&g_ui_ctx);

		lockup();
	}
	else
	{
		asm("move.l #4,A6");
		asm("move.l (A6),A0");
		asm("move.l A0,-(SP)");
		asm("rts");
	}

	lockup();
}

void reboot()
{
	jumptotrack(0);
	Supexec(su_reboot);
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

int process_command_line(int argc, char* argv[])
{
	return 0;
}

int return_to_system(ui_context * ctx)
{
	int i;

	su_fdcUnlock();

	// Clear sceen
	memset(screen_buffer, 0, ctx->SCREEN_YRESOL * LINE_BYTES);

	// Restore video mode
	if ( _oldrez != 0xffff )
	{
		Setscreen((unsigned char *) -1, (unsigned char *) -1, _oldrez );
	}

	// Retore the palette
	for(i=0;i<16;i++)
	{
		 Setcolor( i, backup_pal[i] );
	}

	// Enable mouse pointer
	linea9();

	exit(0);

	return 0;
}
