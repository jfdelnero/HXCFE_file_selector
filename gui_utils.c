/*
//
// Copyright (C) 2009-2014 Jean-François DEL NERO
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <intuition/screens.h>
#include <intuition/preferences.h>
#include <exec/memory.h>


#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/copper.h>
#include <graphics/view.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxnodes.h>
#include <graphics/videocontrol.h>
#include <libraries/dos.h>
#include <utility/tagitem.h>

#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include "graphx/data_bmp_hxc2001logo_bmp.h"
#include "graphx/data_bmp_font_bmp.h"
#include "graphx/data_bmp_font8x8_bmp.h"
#include "graphx/data_bmp_sdhxcfelogo_bmp.h"

#include "gui_utils.h"

#include "conf.h"

static unsigned char * screen_buffer_aligned;
static unsigned char * screen_buffer_backup_aligned;
unsigned short SCREEN_YRESOL;
unsigned char NUMBER_OF_FILE_ON_DISPLAY;// 19-5 //19 -240

struct TextAttr MyFont =
{
		"topaz.font", // Font Name 
		TOPAZ_SIXTY, // Font Height
		FS_NORMAL, // Style
		FPF_ROMFONT, // Preferences
};

struct NewScreen screen_cfg =
{
		0, /* the LeftEdge should be equal to zero */
		0, /* TopEdge */
		SCREEN_XRESOL, /* Width (low-resolution) */
		256, /* Height (non-interlace) */
		1, /* Depth (4 colors will be available) */
		0, 1, /* the DetailPen and BlockPen specifications */
		NULL, /* no special display modes */
		CUSTOMSCREEN, /* the screen type */
		&MyFont, /* use my own font */
		"SD HxC Floppy Emulator Manager", /* this declaration is compiled as a text pointer */
		NULL, /* no special screen gadgets */
		NULL, /* no special CustomBitMap */
};

struct TagItem vcTags[] =
{
		{VTAG_ATTACH_CM_SET, NULL },
		{VTAG_VIEWPORTEXTRA_SET, NULL },
		{VTAG_NORMAL_DISP_SET, NULL },
		{VTAG_END_CM, NULL }
};

	#define BLACK 0x002           /*  RGB values for the four colors used.   */
	#define RED   0xFFF
	#define GREEN 0x0f0
	#define BLUE  0x00f


	#define WIDTH  SCREEN_XRESOL /* 640 pixels wide (high resolution)                */
	//#define HEIGHT SCREEN_YRESOL /* 200 lines high (non interlaced NTSC display)     */
	#define DEPTH    2 /* 1 BitPlanes should be used, gives eight colours. */
	#define COLOURS  2 /* 2^1 = 2                                          */

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
  UWORD  *pointer;
  struct ColorMap *cm=NULL;

	#define BLACK 0x002           /*  RGB values for the four colors used.   */
	#define RED   0xFFF
	#define GREEN 0x0f0
	#define BLUE  0x00f

static UWORD colortable[] = {
								0x002, 0xFFF, 0x0f0, 0x00f,
								0x000, 0xFFF, 0x0f0, 0x00f,
								0xFFF, 0x000, 0x0f0, 0x00f,
								0x030, 0xFFF, 0x0f0, 0x00f,
								0x300, 0xFFF, 0x0f0, 0x00f,
								0x303, 0xFFF, 0x0f0, 0x00f,
								0x999, 0x000, 0x0f0, 0x00f,
								0xFFF, 0x343, 0x0f0, 0x00f,
								0xF33, 0xFFF, 0x0f0, 0x00f,
								0xF0F, 0xFFF, 0x0f0, 0x00f,
								0xFFF, 0x0F0, 0x0f0, 0x00f,
								0x330, 0xFFF, 0x0f0, 0x00f,
								0x000, 0xF00, 0x0f0, 0x00f,
								0x000, 0x0F0, 0x0f0, 0x00f,
								0x000, 0x00F, 0x0f0, 0x00f,
								0x004, 0xFFF, 0x0f0, 0x00f,

								0x036, 0xFFF, 0x0f0, 0x00f,
								0x444, 0x037, 0x0f0, 0x00f,
								0x000, 0xFF0, 0x0f0, 0x00f,
								0x404, 0x743, 0x0f0, 0x00f,
								0xFFF, 0x700, 0x0f0, 0x00f,
								0x000, 0x222, 0x0f0, 0x00f,
								0x000, 0x333, 0x0f0, 0x00f,
								0x000, 0x444, 0x0f0, 0x00f,
								0x000, 0x555, 0x0f0, 0x00f,
								0x000, 0x666, 0x0f0, 0x00f,
								0x000, 0x777, 0x0f0, 0x00f,
								0x222, 0x000, 0x0f0, 0x00f,
								0x333, 0x000, 0x0f0, 0x00f,
								0x444, 0x000, 0x0f0, 0x00f,
								0x555, 0x000, 0x0f0, 0x00f,
								0x666, 0x000, 0x0f0, 0x00f,
								
};


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




void display_sprite(unsigned char * membuffer, bmaptype * sprite,unsigned short x, unsigned short y)
{
	unsigned short i,j,k,l,x_offset,base_offset;
	unsigned short *ptr_src;
	unsigned short *ptr_dst;
	
	ptr_dst=(unsigned short*)membuffer;
	ptr_src=(unsigned short*)&sprite->data[0];
	
	k=0;
	l=0;
	base_offset=((y*80)+ ((x>>3)))/2;
	for(j=0;j<(sprite->Ysize);j++)
	{
		l=base_offset +(40*j);
		for(i=0;i<(sprite->Xsize/16);i++)
		{
			ptr_dst[l]=ptr_src[k];
			l++;
			k++;
		}
	}
}

void print_char(unsigned char * membuffer, bmaptype * font,unsigned short x, unsigned short y,unsigned char c)
{
	unsigned short j,k,l,c1;
	unsigned short *ptr_src;
	unsigned short *ptr_dst;
	
	ptr_dst=(unsigned short*)membuffer;
	ptr_src=(unsigned short*)&font->data[0];
	x=(x>>3) & (~0x1);
	// x=((x&(~0x1))<<1)+(x&1);//  0 1   2 3
	
	l=(y*80)+ x;
	k=((c>>4)*(16*16))+(c&0xF);

	for(j=0;j<16;j++)
	{
		ptr_dst[l]=ptr_src[k];
		k=k+(16);
		l=l+(40);
	}
	
}

/*
void print_char8x8(unsigned char * membuffer, bmaptype * font,unsigned short x, unsigned short y,unsigned char c)
{
  unsigned short j,k,l,c1;
  unsigned char *ptr_src;
  unsigned char *ptr_dst;

  ptr_dst=(unsigned char*)membuffer;
  ptr_src=(unsigned char*)&font->data[0];

  x=x>>3;
  //x=((x&(~0x1))<<1)+(x&1);//  0 1   2 3
  l=(y*80)+ (x);
  k=((c>>4)*(8*8*2))+(c&0xF);
  for(j=0;j<8;j++)
  {
    ptr_dst[l]  =ptr_src[k];
    k=k+(16);
    l=l+(80);
  }

}
  */

void print_char8x8(unsigned char * membuffer, bmaptype * font,unsigned short x, unsigned short y,unsigned char c)
{
	unsigned short j,k,l,c1;
	unsigned char *ptr_src;
	unsigned char *ptr_dst;
	
	ptr_dst=(unsigned char*)membuffer;
	ptr_src=(unsigned char*)&font->data[0];
	
	x=x>>3;
	//x=((x&(~0x1))<<1)+(x&1);//  0 1   2 3
	ptr_dst=ptr_dst + ((y*80)+ x);
	ptr_src=ptr_src + (((c>>4)*(8*8*2))+(c&0xF));
	for(j=0;j<8;j++)
	{
		*ptr_dst=*ptr_src;
		ptr_src=ptr_src+16;
		ptr_dst=ptr_dst+80;
	}
	
}



void print_str(unsigned char * membuffer,char * buf,unsigned short x_pos,unsigned short y_pos,unsigned char font)
{
	unsigned short i;
	i=0;
	
	switch(font)
	{
	case 8:
		while(buf[i])
		{
			if(x_pos<=(SCREEN_XRESOL-8))
			{
				print_char8x8(membuffer,bitmap_font8x8_bmp,x_pos,y_pos,buf[i]);
				x_pos=x_pos+8;
			}
			i++;
		}
	break;
	case 16:
		while(buf[i])
		{
			if(x_pos<=(SCREEN_XRESOL-16))
			{
				print_char(membuffer,bitmap_font_bmp,x_pos,y_pos,buf[i]);
				x_pos=x_pos+16;
			}
			i++;
		}
	break;	
	}
}

int hxc_printf(unsigned char mode,unsigned short x_pos,unsigned short y_pos,char * chaine, ...)
{
      char temp_buffer[1024];

      va_list marker;
      va_start( marker, chaine );

      vsnprintf(temp_buffer,1024,chaine,marker);
      switch(mode)
      {
        case 0:
        print_str(screen_buffer_aligned,temp_buffer,x_pos,y_pos,8);
        break;
        case 1:
        print_str(screen_buffer_aligned,temp_buffer,(SCREEN_XRESOL-(strlen(temp_buffer)*8))/2,y_pos,8);
        break;
        case 2:
        print_str(screen_buffer_aligned,temp_buffer,(SCREEN_XRESOL-(strlen(temp_buffer)*8)),y_pos,8);
        break;
        case 4:
        print_str(screen_buffer_aligned,temp_buffer,x_pos,y_pos,16);
        break;
        case 5:
        print_str(screen_buffer_aligned,temp_buffer,(SCREEN_XRESOL-(strlen(temp_buffer)*16))/2,y_pos,16);
        break;
        case 6:
        print_str(screen_buffer_aligned,temp_buffer,(SCREEN_XRESOL-(strlen(temp_buffer)*16)),y_pos,16);
        break;
      }

      va_end( marker );

      return 0;
}

void h_line(unsigned short y_pos,unsigned short val)
{
  unsigned short *ptr_dst;
  unsigned short i,ptroffset;

  ptr_dst=(unsigned short*)screen_buffer_aligned;
  ptroffset=40* y_pos;

  for(i=0;i<40;i++)
  {
     ptr_dst[ptroffset+i]=val;
  }

}


void box(unsigned short x_p1,unsigned short y_p1,unsigned short x_p2,unsigned short y_p2,unsigned short fillval,unsigned char fill)
{
	unsigned short *ptr_dst;
	unsigned short i,j,ptroffset,x_size;
	
	
	ptr_dst=(unsigned short*)screen_buffer_aligned;
	
	x_size=((x_p2-x_p1)/16)*2;
	
	for(j=0;j<(y_p2-y_p1);j++)
	{
		for(i=0;i<x_size;i++)
		{
			ptr_dst[ptroffset+i]=fillval;
		}
		ptroffset=80* (y_p1+j);
	}
}

void clear_line(unsigned short y_pos,unsigned short val)
{
	unsigned char i;
	for(i=0;i<8;i++)
		h_line(y_pos+i,val);
}

void invert_line(unsigned short y_pos)
{
	unsigned char i,j;
	unsigned short *ptr_dst;
	unsigned short ptroffset;
	
	for(j=0;j<8;j++)
	{
		ptr_dst=(unsigned short*)screen_buffer_aligned;
		ptroffset=40* (y_pos+j);
		
		for(i=0;i<40;i++)
		{
			ptr_dst[ptroffset+i]=ptr_dst[ptroffset+i]^0xFFFF;
		}
	}
}

void restore_box()
{
	memcpy(&screen_buffer_aligned[160*70],screen_buffer_backup_aligned, (8*1000) + 256);
}

int hxc_printf_box(unsigned char mode,char * chaine, ...)
{
	char temp_buffer[1024];
	int str_size;
	unsigned short i;
	
	memcpy(screen_buffer_backup_aligned,&screen_buffer_aligned[160*70], (8*1000) + 256);
	
	va_list marker;
	va_start( marker, chaine );
	
	vsnprintf(temp_buffer,1024,chaine,marker);
	
	str_size=strlen(temp_buffer) * 8;
	str_size=str_size+(4*8);
	
	for(i=0;i< str_size;i=i+8)
	{
        print_char8x8(screen_buffer_aligned,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2)+i,80-8,8);
	}
	print_char8x8(screen_buffer_aligned,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2)+(i-8),80-8,3);
	print_char8x8(screen_buffer_aligned,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2),80-8,2);
	
	for(i=0;i< str_size;i=i+8)
	{
        print_char8x8(screen_buffer_aligned,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2)+i,80,' ');
	}
	
	print_str(screen_buffer_aligned,temp_buffer,((SCREEN_XRESOL-str_size)/2)+(2*8),80,8);
	print_char8x8(screen_buffer_aligned,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2)+(i-8),80,7);
	print_char8x8(screen_buffer_aligned,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2),80,6);
	
	for(i=0;i< str_size;i=i+8)
	{
        print_char8x8(screen_buffer_aligned,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2)+i,80+8,9);
	}
	print_char8x8(screen_buffer_aligned,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2)+(i-8),80+8,5);
	print_char8x8(screen_buffer_aligned,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2),80+8,4);
	
	va_end( marker );
}

void init_buffer()
{
	int i;
	display_sprite(screen_buffer_aligned, bitmap_hxc2001logo_bmp,(SCREEN_XRESOL-bitmap_hxc2001logo_bmp->Xsize), (SCREEN_YRESOL-bitmap_hxc2001logo_bmp->Ysize));
	display_sprite(screen_buffer_aligned, bitmap_sdhxcfelogo_bmp,(SCREEN_XRESOL-bitmap_sdhxcfelogo_bmp->Xsize)/2, (SCREEN_YRESOL-bitmap_sdhxcfelogo_bmp->Ysize));
	
	h_line(SCREEN_YRESOL-34,0xFFFF) ;
	h_line(SCREEN_YRESOL-((48+(3*8))+2),0xFFFF) ;
	h_line(8,0xFFFF) ;
	
	hxc_printf(0,0,SCREEN_YRESOL-(8*1),"Ver %s",VERSIONCODE);
	
	hxc_printf(1,0,0,"SDCard HxC Floppy Emulator Manager for Amiga");
	h_line(SCREEN_YRESOL-(48+20)+24-2,0xFFFF) ;
	hxc_printf(1,0,SCREEN_YRESOL-(48+20)+24,">>>Press HELP key for the function key list<<<");
	
	i=1;
	hxc_printf(1,0,HELP_Y_POS+(i*8), "SDCard HxC Floppy Emulator file selector for Amiga");
	i++;
	hxc_printf(1,0,HELP_Y_POS+(i*8), "(c) 2009-2014 HxC2001 / Jean-Francois DEL NERO");
	i++;
	hxc_printf(1,0,HELP_Y_POS+(i*8), "Check for updates on :");
	i++;
	hxc_printf(1,0,HELP_Y_POS+(i*8), "http://hxc2001.free.fr/floppy_drive_emulator/");
	i++;
	hxc_printf(1,0,HELP_Y_POS+(i*8), "Email : hxc2001@free.fr");
	i++;
	hxc_printf(1,0,HELP_Y_POS+(i*8), "V%s - %s",VERSIONCODE,DATECODE);
	
	
}

void DestroyScrn ()
{ 
	WORD Cntr;

	if (view.LOFCprList) FreeCprList(view.LOFCprList);
	if (view.SHFCprList) FreeCprList(view.SHFCprList);

	FreeVPortCopLists(&viewPort);

	if (cm) FreeColorMap(cm);
	//if (VpXtr) GfxFree (VpXtr);
	for (Cntr = 0; Cntr < 8; Cntr++)
	{
		if (my_bit_map.Planes[Cntr])
			FreeRaster (my_bit_map.Planes[Cntr], 640, 480);
	}

	/*if (VwXtr)
	{ if (VwXtr->Monitor)
	CloseMonitor (VwXtr->Monitor);
	GfxFree (VwXtr);
	}*/
}

void set_color_scheme(unsigned char color)
{
	LoadRGB4(&viewPort, &colortable[(color&0x1F)*4], 4);
}
int init_display()
{
	unsigned short loop,yr;

	memset(&view,0,sizeof(struct View));
	memset(&viewPort,0,sizeof(struct ViewPort));
	memset(&rasInfo,0,sizeof(struct RasInfo));
	memset(&my_bit_map,0,sizeof(struct BitMap));
	memset(&my_rast_port,0,sizeof(struct RastPort));
	screen_buffer_backup_aligned=(unsigned char*)malloc(16*1024);
	
	IntuitionBase=(struct IntuitionBase *)OpenLibrary( "intuition.library", 0 );
	screen=(struct Screen *)OpenScreen(&screen_cfg);
	
	/* Open the Graphics library: */
	GfxBaseptr = (struct GfxBase *) OpenLibrary( "graphics.library", 0 );
	if( !GfxBaseptr )  return -1;
	
	/* Save the current View, so we can restore it later: */
	my_old_view = GfxBaseptr->ActiView;
	
	/* 1. Prepare the View structure, and give it a pointer to */
	/*    the first ViewPort:                                  */
	InitView( &view );
	view.Modes |= HIRES;//LACE;
	
	/* 4. Prepare the BitMap: */
	InitBitMap( &my_bit_map, DEPTH, WIDTH, 256 );
	
	/* Allocate memory for the Raster: */
	for( loop = 0; loop < DEPTH; loop++ )
	{
		my_bit_map.Planes[ loop ] = (PLANEPTR) AllocRaster( WIDTH, 256 );
		BltClear( my_bit_map.Planes[ loop ], RASSIZE( WIDTH, 256 ), 0 );
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
	viewPort.DWidth = WIDTH;
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
	screen_buffer_aligned=my_bit_map.Planes[ 0 ];
	
	yr= get_vid_mode();
	if(yr>290)
	{
		SCREEN_YRESOL=256;
		NUMBER_OF_FILE_ON_DISPLAY=21;// 19-5 //19 -240
	}
	else
	{
		SCREEN_YRESOL=200;
		NUMBER_OF_FILE_ON_DISPLAY=19-5;// 19-5 //19 -240
	}
	
	init_buffer();

	//Delay(100);

	//LoadView(my_old_view);
	//WaitTOF();
	//DestroyScrn();
	//exit(0);
	//for(;;);
}



