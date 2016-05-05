#ifndef _HEADERS_BASE_H
#define _HEADERS_BASE_H

#ifndef SMALL_DATA
#define A4(x) #x
#elif defined(mc68020)
#define A4(x) "a4@(" #x ":L)"
#else
#define A4(x) "a4@(" #x ":W)"
#endif

#define GETAGBASE  movel A4(_AmigaGuideBase),a6
#define GETASLBASE movel A4(_AslBase),a6
#define GETBULBASE movel A4(_BulletBase),a6
#define GETDOSBASE movel A4(_DOSBase),a6
#define GETDTBASE  movel A4(_DataTypesBase),a6
#define GETGADBASE movel A4(_GadToolsBase),a6
#define GETGFXBASE movel A4(_GfxBase),a6
#define GETINTBASE movel A4(_IntuitionBase),a6
#define GETLOCBASE movel A4(_LocaleBase),a6
#define GETLOWBASE movel A4(_LowLevelBase),a6
#define GETRETBASE movel A4(_RealTimeBase),a6
//#define GETUTLBASE movel A4(_UtilityBase),a6
#define GETWBBASE  movel A4(_WorkbenchBase),a6

#endif /* _HEADERS_BASE_H */
