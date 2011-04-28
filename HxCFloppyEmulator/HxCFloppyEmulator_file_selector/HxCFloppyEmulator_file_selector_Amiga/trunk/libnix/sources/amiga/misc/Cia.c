#include "stabs.h"

/* Absolute definitions for the cia registers
 * some of the are recommended to use
 * and very useful when hacking hardware ;-)
 */

ABSDEF(ciab,      0x00bfd000);
ABSDEF(ciabpra,   0x00bfd000);
ABSDEF(ciabprb,   0x00bfd100);
ABSDEF(ciabddra,  0x00bfd200);
ABSDEF(ciabddrb,  0x00bfd300);
ABSDEF(ciabtalo,  0x00bfd400);
ABSDEF(ciabtahi,  0x00bfd500);
ABSDEF(ciabtblo,  0x00bfd600);
ABSDEF(ciabtbhi,  0x00bfd700);
ABSDEF(ciabtodlow,0x00bfd800);
ABSDEF(ciabtodmid,0x00bfd900);
ABSDEF(ciabtodhi, 0x00bfda00);
ABSDEF(ciabsdr,   0x00bfdc00);
ABSDEF(ciabicr,   0x00bfdd00);
ABSDEF(ciabcra,   0x00bfde00);
ABSDEF(ciabcrb,   0x00bfdf00);
ABSDEF(ciaa,      0x00bfe001);
ABSDEF(ciaapra,   0x00bfe001);
ABSDEF(ciaaprb,   0x00bfe101);
ABSDEF(ciaaddra,  0x00bfe201);
ABSDEF(ciaaddrb,  0x00bfe301);
ABSDEF(ciaatalo,  0x00bfe401);
ABSDEF(ciaatahi,  0x00bfe501);
ABSDEF(ciaatblo,  0x00bfe601);
ABSDEF(ciaatbhi,  0x00bfe701);
ABSDEF(ciaatodlow,0x00bfe801);
ABSDEF(ciaatodmid,0x00bfe901);
ABSDEF(ciaatodhi, 0x00bfea01);
ABSDEF(ciaasdr,   0x00bfec01);
ABSDEF(ciaaicr,   0x00bfed01);
ABSDEF(ciaacra,   0x00bfee01);
ABSDEF(ciaacrb,   0x00bfef01);
