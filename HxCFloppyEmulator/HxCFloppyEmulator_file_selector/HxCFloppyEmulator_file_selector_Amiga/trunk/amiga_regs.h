#define WRITEREG_B(addr,value) *((volatile unsigned char  *) (addr)) = ((unsigned char)(value))
#define WRITEREG_W(addr,value) *((volatile unsigned short *) (addr)) = ((unsigned short)(value))
#define WRITEREG_L(addr,value) *((volatile unsigned long  *) (addr)) = ((unsigned long)(value))

#define READREG_B(addr) *((volatile unsigned char  *) (addr))
#define READREG_W(addr) *((volatile unsigned short *) (addr))
#define READREG_L(addr) *((volatile unsigned long  *) (addr))

#define DDFSTRT (0xdff092) // Display bit plane data fetch start.hor pos
#define DDFSTOP (0xdff094) // Display bit plane data fetch stop.hor pos
#define FMODE   (0xdff1FC) // Fetch mode register
#define DIWSTRT (0xdff08E) // Display window start (upper left vert-hor pos)
#define DIWSTOP (0xdff090) // Display window stop (lower right vert-hor pos)

#define BPLCON0 (0xdff100) // Bit plane control reg (misc control bits)
#define BPLCON1 (0xdff102) // Bit plane control reg (scroll val PF1,PF2)
#define BPLCON2 (0xdff104) // Bit plane control reg (priority control)
#define BPLCON3 (0xdff106) // Bit plane control reg (enhanced features)
#define BPL1MOD (0xdff108) // Bit plane modulo (odd planes,or active- fetch lines if bitplane scan-doubling is enabled
#define BPL2MOD (0xdff10A) // Bit plane modulo (even planes or inactive- fetch lines if bitplane scan-doubling is enabled
#define BEAMCON0 (0xdff1DC) // Beam counter control register (SHRES,UHRES,PAL)

#define CIAAPRA (0xBFE001) //four input bits for disk sensing
#define CIABPRB (0xBFD100) //eight output bits for disk selection,
#define ADKCON  (0xDFF09E) //control bits (write only register)
#define ADKCONR (0xDFF010) //control bits (read only register)

#define DSKPTH   (0xDFF020) //DMA pointer (32 bits)
#define CIAB_ICR (0xBFDD00) // CIAB interrupt control register
#define INTREQR  (0xDFF01E)
#define INTREQ   (0xDFF09C)

#define VPOSR   (0xDFF004)
#define VHPOSR  (0xDFF006)
/*
DSKLEN Register ($DFF024)
Bit
15 DMAEN Secondary disk DMA enable
14 WRITE Disk write (RAM disk if 1)
13-0 LENGTH Number of words to transfer
*/
#define DSKLEN  (0xDFF024) //length of DMA

/*
Table 8-7: DSKBYTR Register
Bit
Number Name Function
15 DSKBYT When set, indicates that this register contains a valid byte of data (reset by reading this register).
14 DMAON Indicates when DMA is actually enabled. All the various DMA bits must be true. This means the DMAEN bit in DKSLEN, and the DSKEN & DMAEN bits in DMACON.
13 DISKWRITE The disk write bit (in DSKLEN) is enabled.
12 WORDEQUAL Indicates the DISKSYNC register equals the disk input stream. This bit is true only while the input stream matches the sync register (as little as two microseconds).
11-8 Currently unused; don't depend on read value.
7-0 DATA Disk byte data.
*/
#define DSKBYTR (0xDFF01A) //Disk data byte and status read

#define DSKSYNC (0xDFF07E) //Disk sync finder; holds a match word

#define DMACONR (0xDFF002) //DMA control (and blitter status) read
#define DMACON  (0xDFF096) //DMA control write (clear or set)

////////////////////////////////////////////////////////////////////////////////////////////
// CIAAPRA
////////////////////////////////////////////////////////////////////////////////////////////

/*Disk ready (active low). The drive will pull this line
low when the motor is known to be rotating at full
speed. This signal is only valid when the motor is ON,
at other times configuration information may obscure
the meaning of this input.*/
#define CIAAPRA_DSKRDY 0x20

/*Track zero detect. The drive will pull this line low
when the disk heads are positioned over track zero.
Software must not attempt to step outwards when this
signal is active. Some drives will refuse to step,
others will attempt the step, possibly causing
alignment damage.
All new drives must refuse to step outward in this
condition.*/
#define CIAAPRA_DSKTRACK0 0x10

/*Disk is write protected (active low).*/
#define CIAAPRA_DSKPROT 0x08

/*Disk has been removed from the drive. The signal goes
low whenever a disk is removed. It remains low until a
disk is inserted AND a step pulse is received.*/
#define CIAAPRA_DSKCHANGE 0x04

////////////////////////////////////////////////////////////////////////////////////////////
// CIABPRB
////////////////////////////////////////////////////////////////////////////////////////////

/*PB7 DSKMOTOR* Disk motor control (active low). This signal is
nonstandard on the Amiga system. Each drive will latch
the motor signal at the time its select signal turns
on. The disk drive motor will stay in this state until
the next time select turns on. DSKMOTOR* also controls
the activity light on the front of the disk drive.
All software that selects drives must set up the motor
signal before selecting any drives. The drive will
"remember" the state of its motor when it is not
selected. All drive motors turn off after system reset.
- 238 Interface Hardware -
After turning on the motor, software must further wait
for one half second (500ms), or for the DSKRDY* line to
go low.*/

#define CIABPRB_DSKMOTOR 0x80


#define CIABPRB_DSKSEL3  0x40
#define CIABPRB_DSKSEL2  0x20
#define CIABPRB_DSKSEL1  0x10
#define CIABPRB_DSKSEL0  0x08

/*PB2 DSKSIDE Specify which disk head to use. Zero indicates the
upper head. DSKSIDE must be sTable for 100
microseconds before writing. After writing, at least
1.3 milliseconds must pass before switching DSKSIDE.
*/
#define CIABPRB_DSKSIDE  0x04

/*PB1 DSKDIREC Specify the direction to seek the heads. Zero implies
seek towards the centre spindle. Track zero is at the
outside of the disk. This line must be set up before
the actual step pulse, with a separate write to the
register.
*/
#define CIABPRB_DSKDIREC  0x02

/*
PB0 DSKSTEP* Step the heads of the disk. This signal must always be
used as a quick pulse (high, momentarily low, then high).
The drives used for the Amiga are guaranteed to get to
the next track within 3 milliseconds. Some drives will
support a much faster rate, others will fail. Loops
that decrement a counter to provide delay are not
accepTable. See Appendix F for a better solution.
When reversing directions, a minimum of 18 milliseconds
delay is required from the last step pulse. Settle time
for Amiga drives is specified at 15 milliseconds.
*/
#define CIABPRB_DSKSTEP  0x01

/*
FLAG DSKINDEX* Disk index pulse ($BFDD00, bit 4). Can be used to
create a level 6 interrupt. See Appendix F for details.
*/
