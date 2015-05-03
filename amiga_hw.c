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

#include <devices/trackdisk.h>

#include <dos/dos.h>

#include <exec/types.h>

#include <libraries/commodities.h>

#include <proto/commodities.h>
#include <proto/exec.h>

#include <intuition/screens.h>
#include <intuition/preferences.h>
#include <stdio.h>


#include "keysfunc_defs.h"
#include "keys_defs.h"
#include "amiga_hw.h"
#include "amiga_regs.h"

#include "crc.h"

extern unsigned short SCREEN_YRESOL;

static unsigned char CIABPRB_DSKSEL;

static const unsigned char MFM_short_tab[]=
{
	0xAA,0xA9,0xA4,0xA5,0x92,0x91,0x94,0x95,
	0x4A,0x49,0x44,0x45,0x52,0x51,0x54,0x55
};

static const unsigned short MFM_tab[]=
{
	0xAAAA,0xAAA9,0xAAA4,0xAAA5,0xAA92,0xAA91,0xAA94,0xAA95,
	0xAA4A,0xAA49,0xAA44,0xAA45,0xAA52,0xAA51,0xAA54,0xAA55,
	0xA92A,0xA929,0xA924,0xA925,0xA912,0xA911,0xA914,0xA915,
	0xA94A,0xA949,0xA944,0xA945,0xA952,0xA951,0xA954,0xA955,
	0xA4AA,0xA4A9,0xA4A4,0xA4A5,0xA492,0xA491,0xA494,0xA495,
	0xA44A,0xA449,0xA444,0xA445,0xA452,0xA451,0xA454,0xA455,
	0xA52A,0xA529,0xA524,0xA525,0xA512,0xA511,0xA514,0xA515,
	0xA54A,0xA549,0xA544,0xA545,0xA552,0xA551,0xA554,0xA555,
	0x92AA,0x92A9,0x92A4,0x92A5,0x9292,0x9291,0x9294,0x9295,
	0x924A,0x9249,0x9244,0x9245,0x9252,0x9251,0x9254,0x9255,
	0x912A,0x9129,0x9124,0x9125,0x9112,0x9111,0x9114,0x9115,
	0x914A,0x9149,0x9144,0x9145,0x9152,0x9151,0x9154,0x9155,
	0x94AA,0x94A9,0x94A4,0x94A5,0x9492,0x9491,0x9494,0x9495,
	0x944A,0x9449,0x9444,0x9445,0x9452,0x9451,0x9454,0x9455,
	0x952A,0x9529,0x9524,0x9525,0x9512,0x9511,0x9514,0x9515,
	0x954A,0x9549,0x9544,0x9545,0x9552,0x9551,0x9554,0x9555,
	0x4AAA,0x4AA9,0x4AA4,0x4AA5,0x4A92,0x4A91,0x4A94,0x4A95,
	0x4A4A,0x4A49,0x4A44,0x4A45,0x4A52,0x4A51,0x4A54,0x4A55,
	0x492A,0x4929,0x4924,0x4925,0x4912,0x4911,0x4914,0x4915,
	0x494A,0x4949,0x4944,0x4945,0x4952,0x4951,0x4954,0x4955,
	0x44AA,0x44A9,0x44A4,0x44A5,0x4492,0x4491,0x4494,0x4495,
	0x444A,0x4449,0x4444,0x4445,0x4452,0x4451,0x4454,0x4455,
	0x452A,0x4529,0x4524,0x4525,0x4512,0x4511,0x4514,0x4515,
	0x454A,0x4549,0x4544,0x4545,0x4552,0x4551,0x4554,0x4555,
	0x52AA,0x52A9,0x52A4,0x52A5,0x5292,0x5291,0x5294,0x5295,
	0x524A,0x5249,0x5244,0x5245,0x5252,0x5251,0x5254,0x5255,
	0x512A,0x5129,0x5124,0x5125,0x5112,0x5111,0x5114,0x5115,
	0x514A,0x5149,0x5144,0x5145,0x5152,0x5151,0x5154,0x5155,
	0x54AA,0x54A9,0x54A4,0x54A5,0x5492,0x5491,0x5494,0x5495,
	0x544A,0x5449,0x5444,0x5445,0x5452,0x5451,0x5454,0x5455,
	0x552A,0x5529,0x5524,0x5525,0x5512,0x5511,0x5514,0x5515,
	0x554A,0x5549,0x5544,0x5545,0x5552,0x5551,0x5554,0x5555
};

static unsigned char * mfmtobinLUT_L;
static unsigned char * mfmtobinLUT_H;

#define MFMTOBIN(W) ( mfmtobinLUT_H[W>>8] | mfmtobinLUT_L[W&0xFF] )

static unsigned short * track_buffer;
static unsigned short * track_buffer_wr;

static unsigned char validcache;

unsigned short sector_pos[16];

unsigned char keyup;

void waitus(int centus)
{
	int cnt;
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

void setvideomode(int mode)
{
	Forbid();

	switch(mode)
	{
		case 0:
			// PAL Mode
			WRITEREG_W( BEAMCON0, READREG_W(BEAMCON0) | 0x0020);
			//WRITEREG_W( BEAMCON0, 0x0020);
			WRITEREG_W( DDFSTRT,  0x0038);
			WRITEREG_W( DDFSTOP,  0x00D8);
			WRITEREG_W( FMODE,    0x0003);
			WRITEREG_W( DIWSTRT,  0x4481);
			WRITEREG_W( DIWSTOP,  0x0CC1);
			WRITEREG_W( BPLCON0,  0x0211);
			WRITEREG_W( BPL1MOD,  0x0000);
			WRITEREG_W( BPL2MOD,  0x0000);

			//WRITEREG_W( DMACON,  0x0082);
			//WRITEREG_W( DMACON,  0x8300);

		break;
		case 1:
			// NTSC Mode
			WRITEREG_W( BEAMCON0, READREG_W(BEAMCON0) & ~0x0020);
			//WRITEREG_W( BEAMCON0, 0x0000);
			WRITEREG_W( DDFSTRT,  0x0038);
			WRITEREG_W( DDFSTOP,  0x00D8);
			WRITEREG_W( FMODE,    0x0003);
			WRITEREG_W( DIWSTRT,  0x2C81);
			WRITEREG_W( DIWSTOP,  0xF4C1);
			WRITEREG_W( BPLCON0,  0x0211);
			WRITEREG_W( BPL1MOD,  0x0000);
			WRITEREG_W( BPL2MOD,  0x0000);

			//WRITEREG_W( DMACON,  0x0082);
			//WRITEREG_W( DMACON,  0x8300);

			break;
	}

	Permit();
}



int jumptotrack(unsigned char t)
{
	unsigned short i,j,k;

	Forbid();
	WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | CIABPRB_DSKSEL ));

#ifdef DBGMODE
	hxc_printf(0,0,0,"-- jumptotrack %d --",t);
#endif
	waitms(100);

#ifdef DBGMODE
	hxc_printf(0,0,0,"-- jumptotrack %d - seek track 0... --",t);
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
	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- jumptotrack %d - track 0 found --",t);
	#endif

		for(j=0;j<t;j++)
		{
			WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | CIABPRB_DSKSEL | CIABPRB_DSKDIREC |CIABPRB_DSKSTEP) );
			waitms(1);
			WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | CIABPRB_DSKSEL | CIABPRB_DSKDIREC ) );
			waitms(1);
		}

		WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | CIABPRB_DSKSEL ) );
	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- jumptotrack %d - jump done    --",t);
	#endif

		Permit();

		return 0;
	}

	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- jumptotrack %d - track 0 not found!! --",t);
	#endif

	Permit();
	return 1;
};

int test_drive(int drive)
{
	int t,j,c;
	Forbid();
	WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3)) ));

	waitms(100);

	// Track 0
	t = 0;
	while((READREG_B(CIAAPRA) & CIAAPRA_DSKTRACK0) && (t<260))
	{
		WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3))  | CIABPRB_DSKSTEP));
		waitus(2);
		WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3)) ) );
		waitus(2);

		t++;
	}

	if(t<260)
	{
		c = 0;
		do
		{
			// Track 30
			for(j=0;j<30;j++)
			{
				WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3)) | CIABPRB_DSKDIREC |CIABPRB_DSKSTEP) );
				waitus(2);
				WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3)) | CIABPRB_DSKDIREC ) );
				waitus(2);
			}

			t = 0;
			while((READREG_B(CIAAPRA) & CIAAPRA_DSKTRACK0) && (t<30))
			{
				WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3))  | CIABPRB_DSKSTEP));
				waitus(2);
				WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3)) ) );
				waitus(2);

				t++;
			}

			c++;
		}while( (t != 30) && c < 4 );

		if(t == 30)
		{
			WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3)) ) );

			Permit();

			return 1;
		}
	}

	WRITEREG_B(CIABPRB, ~(CIABPRB_DSKMOTOR | (CIABPRB_DSKSEL0<<(drive&3)) ) );

	Permit();

	return 0;
}

void waitindex()
{
	do{
		asm("nop");
	}while(!(READREG_B(CIAB_ICR)&0x10));

	do
	{
		asm("nop");
	}while(READREG_B(CIAB_ICR)&0x10);

	do{
		asm("nop");
	}while(!(READREG_B(CIAB_ICR)&0x10));

}


static void setnoclick(ULONG unitnum, ULONG onoff)
{
	struct MsgPort *port;
	port = CreateMsgPort();
	if (port)
	{
		struct IOStdReq *ioreq;
		ioreq = CreateIORequest(port, sizeof(*ioreq));
		if (ioreq)
		{
			if (OpenDevice(TD_NAME, unitnum, (APTR) ioreq, 0) == 0)
			{
				struct TDU_PublicUnit *unit = (APTR) ioreq->io_Unit;
				Forbid();
				if (onoff)
					unit->tdu_PubFlags |= TDPF_NOCLICK;
				else
					unit->tdu_PubFlags &= ~TDPF_NOCLICK;
				Permit();
				CloseDevice((APTR) ioreq);
			}
			DeleteIORequest((APTR) ioreq);
		}
		DeleteMsgPort(port);
	}
}

int readtrack(unsigned short * track,unsigned short size,unsigned char waiti)
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

	if(waiti) waitindex();

	//Put the value you want into the DSKLEN register
	WRITEREG_W( DSKLEN ,size | 0x8000);
	//Write this value again into the DSKLEN register. This actually starts the DMA.
	WRITEREG_W( DSKLEN ,size | 0x8000);

	while(!(READREG_W(INTREQR)&0x0002));
	WRITEREG_W( DSKLEN ,0x4000);
	WRITEREG_W(INTREQ,0x0002);

	validcache=1;

	return 1;

}

int writetrack(unsigned short * track,unsigned short size,unsigned char waiti)
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

	if(waiti)
	{
		while(READREG_B(CIAB_ICR)&0x10);
		while(!(READREG_B(CIAB_ICR)&0x10));
	}

	//Put the value you want into the DSKLEN register
	WRITEREG_W( DSKLEN ,size | 0x8000 | 0x4000 );
	//Write this value again into the DSKLEN register. This actually starts the DMA.
	WRITEREG_W( DSKLEN ,size | 0x8000 | 0x4000 );

	while(!(READREG_W(INTREQR)&0x0002));
	WRITEREG_W( DSKLEN ,0x4000);
	WRITEREG_W(INTREQ,0x0002);

	validcache=0;

	return 1;
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

unsigned char writesector(unsigned char sectornum,unsigned char * data)
{
	unsigned short i,j,len,retry,retry2,lastbit;
	unsigned char sectorfound;
	unsigned char c;
	unsigned char CRC16_High,CRC16_Low,byte;

	Forbid();

	retry2=2;

	i=0;
	validcache=0;

	CRC16_Init(&CRC16_High, &CRC16_Low);
	for(j=0;j<3;j++)
	{
		CRC16_Update(&CRC16_High,&CRC16_Low,0xA1);
	}

	CRC16_Update(&CRC16_High,&CRC16_Low,0xFB);

	for(j=0;j<512;j++)
	{
		CRC16_Update(&CRC16_High,&CRC16_Low,data[j]);
	}

	for(j=0;j<12;j++)
		track_buffer_wr[i++]=0xAAAA;

	track_buffer_wr[i++]=0x4489;
	track_buffer_wr[i++]=0x4489;
	track_buffer_wr[i++]=0x4489;
	lastbit = 0x7FFF;
	byte = 0xFB;
	BuildCylinder((unsigned char*)&track_buffer_wr[i++],1*2,&byte,1,lastbit,&lastbit);
	BuildCylinder((unsigned char*)&track_buffer_wr[i],512*2,data,512,lastbit,&lastbit);
	i=i+512;
	BuildCylinder((unsigned char*)&track_buffer_wr[i++],1*2,&CRC16_High,1,lastbit,&lastbit);
	BuildCylinder((unsigned char*)&track_buffer_wr[i++],1*2,&CRC16_Low,1,lastbit,&lastbit);
	byte = 0x4E;
	for(j=0;j<4;j++)
	{
		BuildCylinder((unsigned char*)&track_buffer_wr[i++],1*2,&byte,1,lastbit,&lastbit);
	}

	len=i;

	sectorfound=0;
	retry=30;

	if(sectornum)
	{
		do
		{

			do
			{

				i=0;

				retry--;

				if(!readtrack(track_buffer,16,0))
				{
					Permit();
					return 0;
				}

				while(track_buffer[i]==0x4489 && (i<16))
				{
					i++;
				}

				if(MFMTOBIN(track_buffer[i])==0xFE && (i<(16-3)))
				{

					CRC16_Init(&CRC16_High, &CRC16_Low);
					for(j=0;j<3;j++)CRC16_Update(&CRC16_High,&CRC16_Low,0xA1);

					for(j=0;j<(1+4+2);j++)
					{
						c = MFMTOBIN(track_buffer[i+j]);
						CRC16_Update(&CRC16_High, &CRC16_Low,c);
					}

					if(!CRC16_High && !CRC16_Low)
					{
						i++;
						if(MFMTOBIN(track_buffer[i])==0xFF) //track
						{
							i++;
							if(MFMTOBIN(track_buffer[i])==0x00) //side
							{
								i++;
								if(MFMTOBIN(track_buffer[i])==sectornum) //sector
								{
									sectorfound=1;
									if(!writetrack(track_buffer_wr,len,0))
									{
										Permit();
										return 0;
									}
								}
							}
						}
					}
				}
			}while(!sectorfound  && retry);

			if(!sectorfound)
			{
				if(jumptotrack(255))
				{
					hxc_printf_box(0,"ERROR: writesector -> failure while seeking the track 00!");
				}
				retry=30;
			}
			retry2--;

		}while(!sectorfound && retry2);

	}
	else
	{
		sectorfound=1;

        if(!writetrack(track_buffer_wr,len,1))
		{
			Permit();
			return 0;
		}

	}

	Permit();
	return sectorfound;
}


unsigned char readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache)
{
	unsigned short i,j,t;
	unsigned char sectorfound,tc;
	unsigned char c,retry,badcrc,retry2;
	unsigned char CRC16_High,CRC16_Low;

	Forbid();
	retry2=2;
	retry=5;

	do
	{

		do
		{
			sectorfound=0;
			i=0;
			badcrc=0;
			if(!validcache || invalidate_cache)
			{
				if(!readtrack(track_buffer,10*1024,0))
				{
					Permit();
					return 0;
				}

				i=1;
				for(j=0;j<9;j++)
				{
					sector_pos[j]=0xFFFF;
				}

				for(j=0;j<9;j++)
				{
					while(track_buffer[i]!=0x4489 && i) i=(i+1)&0x3FFF;
					if(!i) j=9;
					while(track_buffer[i]==0x4489 && i) i=(i+1)&0x3FFF;
					if(!i) j=9;
					if(MFMTOBIN(track_buffer[i])==0xFE)
					{
						sector_pos[MFMTOBIN(track_buffer[i+3])&0xF]=i;
						i=(i+512+2)&0x3FFF;
					}
					else
					{
						i++;
					}
				}
			}

			do
			{
				i=sector_pos[sectornum&0xF];
				if(i<16*1024)
				{
					if(MFMTOBIN(track_buffer[i])==0xFE)
					{
						CRC16_Init(&CRC16_High, &CRC16_Low);
						for(j=0;j<3;j++)CRC16_Update(&CRC16_High,&CRC16_Low,0xA1);

						for(j=0;j<(1+4+2);j++)
						{
							c=MFMTOBIN(track_buffer[i+j]);
							CRC16_Update(&CRC16_High, &CRC16_Low,c);
						}

						if(!CRC16_High && !CRC16_Low)
						{
							i++;
							if(MFMTOBIN(track_buffer[i])==0xFF) //track
							{
								i++;
								if(MFMTOBIN(track_buffer[i])==0x00) //side
								{
									i++;
									if(MFMTOBIN(track_buffer[i])==sectornum) //sector
									{
										i=i+41;

										CRC16_Init(&CRC16_High, &CRC16_Low);
										for(j=0;j<3;j++)
											CRC16_Update(&CRC16_High,&CRC16_Low,0xA1);

										CRC16_Update(&CRC16_High,&CRC16_Low,MFMTOBIN(track_buffer[i]));
										i++;

										for(j=0;j<512;j++)
										{

											tc = MFMTOBIN(track_buffer[i]);

											//CRC16_Update(&CRC16_High, &CRC16_Low,tc);
											i++;
											data[j]=tc;
										}

										for(j=0;j<2;j++)
										{
											c = MFMTOBIN(track_buffer[i++]);
											//CRC16_Update(&CRC16_High, &CRC16_Low,c);
										}

										if(1)//!CRC16_High && !CRC16_Low)
										{
											sectorfound=1;
										}
										else
										{
											badcrc=1;
										}

									}
									else
									{
										i=i+512+2;
									}
								}
								else
								{
									i=i+512+2;
								}
							}
							else
							{
								i=i+512+2;
							}
						}
						else
						{
							i++;
							badcrc=1;
						}
					}
					else
					{
						i++;
					}
				}
				else
				{
					badcrc=1;
				}

			}while( !sectorfound && (i<(16*1024)) && !badcrc);

			retry--;
			if(!sectorfound && retry)
			{
				validcache=0;
			}

		}while(!sectorfound && retry);


		if(!sectorfound)
		{
			if(jumptotrack(255))
			{
				hxc_printf_box(0,"ERROR: readsector -> failure while seeking the track 00!");
			}

			retry2--;
			retry=5;
		}


	}while(!sectorfound && retry2);

	if(!sectorfound)
	{
		validcache=0;
	}

	Permit();

	return sectorfound;
}

void alloc_error()
{
	hxc_printf_box(0,"ERROR: Memory Allocation Error -> No more free mem ?");
	for(;;);
}

void init_amiga_fdc(unsigned char drive)
{
	unsigned short i;

	if(drive==0)
		CIABPRB_DSKSEL=CIABPRB_DSKSEL0;
	else
		CIABPRB_DSKSEL=CIABPRB_DSKSEL1;

	//	for(i=0;i<3;i++) setnoclick(i, 1);

	validcache=0;

	mfmtobinLUT_L=(unsigned char*)AllocMem(256,MEMF_CHIP);
	mfmtobinLUT_H=(unsigned char*)AllocMem(256,MEMF_CHIP);
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
		alloc_error();
	}

	track_buffer=(unsigned short*)AllocMem(24*1024,MEMF_CHIP);
	if(track_buffer)
	{
		memset(track_buffer,0,24*1024);
	}
	else
	{
		alloc_error();
	}

	track_buffer_wr=(unsigned short*)AllocMem(2*1024,MEMF_CHIP);
	if(track_buffer_wr)
	{
		memset(track_buffer_wr,0,2*1024);
	}
	else
	{
		alloc_error();
	}

	Forbid();

	WRITEREG_B(CIABPRB,~(CIABPRB_DSKMOTOR | CIABPRB_DSKSEL));
	WRITEREG_W( DMACON,0x8210);

	if(jumptotrack(255))
	{
		hxc_printf_box(0,"ERROR: init_amiga_fdc -> failure while seeking the track 00!");
		for(;;);
	}
	Delay(12);
	WRITEREG_W(INTREQ,0x0002);

	Permit();
}

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

int kbhit()
{
}

void flush_char()
{
}

void wait_released_key()
{
	while(!(Keyboard()&0x80));
}

unsigned char get_char()
{
	unsigned char buffer;
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
				return FCT_SELECTSAVEREBOOT;
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



unsigned short get_vid_mode()
{
	unsigned short vpos,vpos2;

	vpos=0;
	vpos2=0;

	Forbid();
	do
	{
		vpos = READREG_W(VHPOSR) >> 8;
		while (vpos == (READREG_W(VHPOSR) >> 8));

		vpos=((READREG_W(VPOSR)&1)<<8)  | (READREG_W(VHPOSR)>>8);
		if(vpos>=vpos2)
		{
			vpos2=vpos;
		}
	}while(vpos>=vpos2);
	Permit();
	return vpos2;
}

void disablemousepointer()
{
	WRITEREG_W( DMACON ,0x20);
}

void _reboot();

void reboot()
{
//ColdReboot();
Supervisor(_reboot);
}
asm (
".globl	__reboot;"
".align 4 ;"//IMPORTANT! Longword align!
"__reboot:                              ;"
//"                movea.l  #4,a6        ;"//Pointer to the Exec library base
//"                cmp.w   #36,0x14(a6) ;"//LIB_VERSION
//"                blt.s   old_exec     ;"
//"                jmp     superrst ;"

//"                jmp     -726(a6)     ;"//Let Exec do it...
//"                jmp failreboot;"

//---- manually reset the Amiga ---------------------------------------------

"superrst:    ;"
"               move.w	#0x2700,sr          ;"
"		lea	0x0FC0000,a0       ;"
//"		lea	0x01000004,a0       ;"
//"		sub.l	-0x1C(a0),a0        ;"
//"		movea.l	(a0),a0             ;"
//"		subq.l	#2,a0               ;"
//"		move.l	#0xFFFFFFFF,(4).w   ;"
"		bra.s	resetinst           ;"
".align 4 ;"//IMPORTANT! Longword align!
"resetinst:	            ;"
"               reset       ;"
"		jmp	(a0);"

"failreboot:       ;"
"                rts;"

);

