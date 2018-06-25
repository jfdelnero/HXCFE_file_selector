FONT_BUILDER=../font_builder/build_font

all: font_st_8x8.c font_charset_8x6.c font_dangen_8x12.c font_x_b_8x13.c

font_st_8x8.c: bmp/Font_ST_8x8.bmp
	$(FONT_BUILDER) -file:bmp/Font_ST_8x8.bmp         -fontname:font_st_8x8         -nbchars:128 -cxsize:8 -cysize:8  -cxstep:8 -cystep:8  -xoffset:0 -yoffset:0 -charsperline:16 -charsperrow:16 -srcstartchar:0

font_charset_8x6.c: bmp/Font_charset_8x6.bmp
	$(FONT_BUILDER) -file:bmp/Font_charset_8x6.bmp    -fontname:font_charset_8x6    -nbchars:128 -cxsize:8 -cysize:6  -cxstep:8 -cystep:6  -xoffset:0 -yoffset:0 -charsperline:16 -charsperrow:16 -srcstartchar:32

font_dangen_8x12.c: bmp/Font_Dangen_8x12.bmp
	$(FONT_BUILDER) -file:bmp/Font_Dangen_8x12.bmp    -fontname:font_dangen_8x12    -nbchars:128 -cxsize:8 -cysize:12 -cxstep:8 -cystep:12 -xoffset:0 -yoffset:0 -charsperline:16 -charsperrow:16 -srcstartchar:32

font_atari_8x16.c: bmp/Font_Atari_8x16.bmp
	$(FONT_BUILDER) -file:bmp/Font_Atari_8x16.bmp     -fontname:font_atari_8x16     -nbchars:128 -cxsize:8 -cysize:16 -cxstep:8 -cystep:16 -xoffset:0 -yoffset:0 -charsperline:16 -charsperrow:16 -srcstartchar:0

font_xwindow_8x16.c: bmp/Font_Xwindow_8x16.bmp
	$(FONT_BUILDER) -file:bmp/Font_Xwindow_8x16.bmp   -fontname:font_xwindow_8x16   -nbchars:128 -cxsize:8 -cysize:16 -cxstep:8 -cystep:16 -xoffset:0 -yoffset:0 -charsperline:16 -charsperrow:16 -srcstartchar:0

font_xwindow_8x12.c: bmp/Font_Xwindow_8x12.bmp
	$(FONT_BUILDER) -file:bmp/Font_Xwindow_8x12.bmp   -fontname:font_xwindow_8x12   -nbchars:128 -cxsize:8 -cysize:12 -cxstep:8 -cystep:12 -xoffset:0 -yoffset:0 -charsperline:16 -charsperrow:16 -srcstartchar:0

font_x_b_8x13.c: bmp/Font_X_B_8x13.bmp
	$(FONT_BUILDER) -file:bmp/Font_X_B_8x13.bmp   -fontname:font_x_b_8x13           -nbchars:128 -cxsize:8 -cysize:13 -cxstep:8 -cystep:13 -xoffset:0 -yoffset:0 -charsperline:32 -charsperrow:8 -srcstartchar:0

clean:
	rm -f *.c *.h

mrproper: clean

.PHONY: clean mrproper
