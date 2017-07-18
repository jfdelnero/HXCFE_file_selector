#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

extern struct IOStdReq *ioreq;

struct Library * DOSBase;
struct Library * GfxBaseptr;
struct Library * IntuitionBase;

#define TRACK_SIZE 11*512

int bootblock_main()
{
	unsigned char * loading_mem;
	int track;

	loading_mem = AllocMem(256*1024, MEMF_CHIP|MEMF_CLEAR|MEMF_PUBLIC);
	if( loading_mem )
	{
		for(;;)
		{
			for(track=0;track<160;track++)
			{
				ioreq->io_Length = TRACK_SIZE;
				ioreq->io_Data = (APTR)loading_mem;
				ioreq->io_Offset = (ULONG)(TRACK_SIZE * track);
				ioreq->io_Command = CMD_READ;
				if(DoIO((struct IORequest *)ioreq))
				{
					// Error...
					FreeMem(loading_mem, 256*1024);
					return 1;
				}
			}
		}
	}

	// Something wrong happened...
	return 1;

	/*DOSBase = OpenLibrary((CONST_STRPTR)"dos.library", 0);
	if(DOSBase)
	{
		for(;;)
		{
			PutStr((CONST_STRPTR)"Texte Test !!!! :)\r\n");
		}
	}*/
}