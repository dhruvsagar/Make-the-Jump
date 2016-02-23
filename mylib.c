#include "mylib.h"

typedef unsigned short U16;
U16 *videobuffer = (U16 *) 0x6000000;
void setPixel(int x, int y, U16 pixel);
void drawRect(int x, int y, int width, int height, U16 pixel);
void drawHollowRect(int x, int y, int width, int height, U16 pixel);
void drawImage3(int x, int y, int width, int height, const U16 *image);
void waitForVblank();
void drawChar(int row, int col, char ch, unsigned short color);
void drawString(int row, int col, char *str, unsigned short color);


void setPixel(int x, int y, U16 pixel) {
	videobuffer[240 * y + x] = pixel;
}

void drawRect(int x, int y, int width, int height, U16 pixel) {
	for (int i = x; i < x + width; i++) {
		for (int j = y; j < y + height; j++) {
			setPixel(i, j, pixel);
		}
	}
}

void drawHollowRect(int x, int y, int width, int height, U16 pixel) {
	for (int a = x; a < x + width; a++) {
		setPixel(a, y, pixel);
		setPixel(a, y + height, pixel);
	}
	for (int b = y; b <= y + height; b++) {
		setPixel(x, b, pixel);
		setPixel(x + width, b, pixel);
	}
}

void drawImage3(int x, int y, int width, int height, const U16 *image) {

	int c = 0;
	for (int i = y; i < height + y; i++) {
		for (int j = x; j < width + x; j++) {
			setPixel(j, i, image[c]);
			c++;
		}
	}
}

void drawChar(int row, int col, char ch, unsigned short color)
{
	int r,c;
	for(r=0; r<8; r++)
	{
		for(c=0; c<6; c++)
		{
			if(fontdata_6x8[OFFSET(r, c, 6)+ch*48])
			{
				setPixel(col+c, row+r, color);
			}
		}
	}
}

void drawString(int row, int col, char *str, unsigned short color)
{
	while(*str)
	{
		drawChar(row, col, *str++, color);
		col += 6;
	}
}

void waitForVblank() {
	while(SCANLINECOUNTER > 160);
	while(SCANLINECOUNTER < 160);
}

