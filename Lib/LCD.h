#ifndef LCD_H
#define LCD_H


#define RGB(red, green, blue)     (unsigned int)(((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF))
#define BLACK	                    0x000000
#define RED	                      0x0000FC
#define BLUE 	                    0xFC0000
#define GREEN                     0x00FC00
#define CYAN	                    0x00FCFC
#define MAGENTA                   0xFC00FC
#define YELLOW                    0xFCFC00
#define WHITE	                    0xFCFCFC
#define LcdRot(n)				rot = n
#define LcdFontFixed(n)			FontFixed = n
#define LcdNonTransparence(n)	NonTransparenz = n
#define SetFgColor(n)			FgColor = n
#define SetBkColor(n)			BkColor = n
#define GetFgColor()			FgColor
#define GetBkColor()			BkColor
#define GetMaxX GetMaxX2()//((unsigned int)SCREEN_HOR_SIZE-1)
#define GetMaxY GetMaxY2()//((unsigned int)SCREEN_VER_SIZE-1) 
#define SetCursorX(x) 	cursorX = x
#define SetCursorY(y) 	cursorY = y
#define SetColor(color) _color = color
#define GetColor        _color
#define SCREEN_HOR_SIZE    480
#define SCREEN_VER_SIZE    800
#define ALINE_LEFT	 0
#define ALINE_CENTER	 1
#define ALINE_RIGHT	 2
#define ALINE_MARK		(ALINE_LEFT | ALINE_CENTER | ALINE_RIGHT)
#define   R2L     0  // Paragraph Direction
#define   L2R     1  


	
void Init_TFT24(void);
void LCDSetCur24(unsigned int col,unsigned int row);
void LCD24_SetArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCDClear(unsigned int Color);
void LcdFont(unsigned char *pointer);
unsigned int GetMaxX2 (void);
unsigned int  GetMaxY2 (void);
void write_pixel(unsigned int y,unsigned int x,int color);
void LCD_PutPixel(unsigned int x, unsigned int y);
void PutChar(unsigned char c);
void Puts(unsigned char *Text);
void PutText(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom, char *str, unsigned char style);
unsigned char Special_Charachters(unsigned char C);
int isdigit(int _C);
unsigned char Dos2Farsi(unsigned char *str1,unsigned char *OutStr,unsigned char Direction);
void PutChar2(unsigned char c,unsigned char Direction);
unsigned char Get_CharWidth(unsigned char c);
void Show_TextInTFT(unsigned int X_Start,unsigned int Y_Start,unsigned int X_End,unsigned int Y_End,const unsigned char *str1,unsigned char Direction,unsigned char Center_Justify);
void Line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCDSetCur(unsigned int row,unsigned int col);
void LCD_Rect(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom, unsigned int color);
void RectangleFill(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int radius);
void Line2(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void RectangleFill2(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2,char Type);
void Rectangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int radius);
void Show_Image(unsigned char *Image,unsigned int Width,unsigned int Height,unsigned int X,unsigned int Y, unsigned int Color,char Mode);
void ProgressBar2(unsigned int X_Start,unsigned int Y_Start,unsigned int X_End,unsigned int Y_End, unsigned long int position, unsigned long int max,unsigned int Color);
#endif