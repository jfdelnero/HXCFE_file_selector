#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

extern unsigned long ioreq;

struct Library * DOSBase;
struct Library * GfxBaseptr;
struct Library * IntuitionBase;

int bootblock_main()
{
	DOSBase = OpenLibrary((CONST_STRPTR)"dos.library", 0);
	if(DOSBase)
	{
		for(;;)
		{
			PutStr((CONST_STRPTR)"Texte Test !!!! :)\r\n");
		}
	}

	return 1;
}