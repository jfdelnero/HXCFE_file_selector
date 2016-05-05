|
| coldboot.asm
|
| Here is a source code listing of the only supported reboot code:
|

|   NAME
|       ColdReboot - Official code to reset any Amiga (Version 2)
|
|   SYNOPSIS
|       ColdReboot()
|       void ColdReboot(void);
|
|   FUNCTION
|       Reboot the machine.  All external memory and peripherals will be
|       RESET, and the machine will start its power up diagnostics.
|
|       Rebooting an Amiga in software is very tricky.  Differing memory
|       configurations and processor cards require careful treatment.  This
|       code represents the best available general purpose reset.  The
|       MagicResetCode must be used exactly as specified here. The code
|       _must_ be longword aligned.  Failure to duplicate the code EXACTLY
|       may result in improper operation under certain system configurations.
|
|   RESULT
|   This function never returns.
|
|   (Some minors modifications done on the code to make is work with GNU GCC/AS)
|

        .globl __reboot
        .align 4
        .text

        _LVOSupervisor   = -30

        ABSEXECBASE      = 4          |Pointer to the Exec library base
        LIB_VERSION      = 6
        MAGIC_ROMEND     = 0x01000000 |End of Kickstart ROM
        MAGIC_SIZEOFFSET = -0x14      |Offset from end of ROM to Kickstart size
        V36_EXEC         = 36         |Exec with the ColdReboot() function
        TEMP_ColdReboot  = -726       |Offset of the V36 ColdReboot function

__reboot:
                move.l  ABSEXECBASE,a6

                jmp     old_exec                | For the moment just use the old method cause
                                                | the following part doesn't seems to work properly...

                cmp.w   #V36_EXEC,LIB_VERSION(a6)
                blt.s   old_exec

                jmp     TEMP_ColdReboot(a6)     | Let Exec do it...
                |NOTE: Control flow never returns to here
                jmp     __reboot


|---- manually reset the Amiga ---------------------------------------------
old_exec:       lea.l   GoAway(pc),a5           |address of code to execute
                jsr     _LVOSupervisor(a6)      |trap to code at (a5)...
                |NOTE: Control flow never returns to here

|-------------- MagicResetCode ---------DO NOT CHANGE-----------------------
                .align 4                        |IMPORTANT! Longword align!

GoAway:
                lea.l   MAGIC_ROMEND,a0         |(end of ROM)
                sub.l   MAGIC_SIZEOFFSET(a0),a0 |(end of ROM)-(ROM size)=PC
                move.l  4(a0),a0                |Get Initial Program Counter
                subq.l  #2,a0                   |now points to second RESET
                reset                           |first RESET instruction
                jmp     (a0)                    |CPU Prefetch executes this

                |NOTE: the RESET and JMP instructions must share a longword!
|---------------------------------------DO NOT CHANGE-----------------------
