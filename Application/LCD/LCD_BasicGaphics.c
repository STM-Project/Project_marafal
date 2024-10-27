/*
 * LCD_BasicGaphics.c
 *
 *  Created on: 11.04.2021
 *      Author: Elektronika RM
 */

#include "LCD_BasicGaphics.h"
#include <stdarg.h>
#include <math.h>
#include "LCD_Hardware.h"
#include "LCD_Common.h"
#include <stdbool.h>

#define MAX_SIZE_TAB_AA		500
#define MAX_LINE_BUFF_CIRCLE_SIZE  500
#define MAX_DEGREE_CIRCLE  10

ALIGN_32BYTES(SDRAM uint32_t pLcd[LCD_BUFF_XSIZE*LCD_BUFF_YSIZE]);

static uint32_t k, kCopy;
static uint32_t buff_AA[MAX_SIZE_TAB_AA];  //sprawdzic czy czasem nie za duzo MAX_SIZE_TAB_AA!!!!
static uint32_t buff2_AA[MAX_SIZE_TAB_AA];
static structPosition pos;

typedef struct
{	float c1;
	float c2;
}AACoeff_RoundFrameRectangle;
static AACoeff_RoundFrameRectangle AA;

static uint8_t correctLine_AA=0;

typedef struct
{	uint8_t lineBuff[MAX_LINE_BUFF_CIRCLE_SIZE];
	float outRatioStart;
	float inRatioStart;
	float outRatioStart_prev;
	float inRatioStart_prev;
	uint16_t width;
	uint16_t width_prev;
	uint16_t x0;
	uint16_t y0;
	uint16_t degree[MAX_DEGREE_CIRCLE+1];
	uint32_t degColor[MAX_DEGREE_CIRCLE];
	float tang[MAX_DEGREE_CIRCLE];
	float coeff[MAX_DEGREE_CIRCLE];
	uint8_t rot[MAX_DEGREE_CIRCLE];
	uint16_t correctForWidth;
	uint16_t correctPercDeg[2];
	float errorDecision[2];
}Circle_Param;
static Circle_Param Circle = {.correctForWidth= 100, .correctPercDeg= {70, 80}, .errorDecision= {0.1, 0.4}};

//int test_r;
void SETPARAM_Circle_1(void* correctForWidth, void* correctPercDeg, void* errorDecision, VOID_FUNCTION_TEST xfunc){
	Circle.correctForWidth = *((uint16_t*)correctForWidth);
	Circle.correctPercDeg[0] = *((uint16_t*)correctPercDeg);
	Circle.errorDecision[0] = *((float*)errorDecision);


	//test_r = (int)xfunc( correctForWidth, correctPercDeg );
}

static void Set_AACoeff(int pixelsInOneSide, uint32_t colorFrom, uint32_t colorTo, float ratioStart)
{
	float incr= (1-ratioStart)/pixelsInOneSide;
	buff_AA[0]=pixelsInOneSide;
	for(int i=0;i<pixelsInOneSide;++i){
		if(1+i>MAX_SIZE_TAB_AA)
			break;
		buff_AA[1+i]= GetTransitionColor(colorFrom,colorTo, ratioStart+i*incr);
}}
static void Set_AACoeff2(int pixelsInOneSide, uint32_t colorFrom, uint32_t colorTo, float ratioStart)
{
	float incr= (1-ratioStart)/pixelsInOneSide;
	buff2_AA[0]=pixelsInOneSide;
	for(int i=0;i<pixelsInOneSide;++i){
		if(1+i>MAX_SIZE_TAB_AA)
			break;
		buff2_AA[1+i]= GetTransitionColor(colorFrom,colorTo, ratioStart+i*incr);
}}

static void _FillBuff(int itCount, uint32_t color)
{
	if(itCount>10)
	{
		int j=itCount/2;
		int a=j;

		uint64_t *pLcd64=(uint64_t*) (pLcd+k);
		uint64_t color64=(((uint64_t)color)<<32)|((uint64_t)color);

		j--;
		while (j)
			pLcd64[j--]=color64;

		pLcd64[j]=color64;
		k+=a+itCount/2;

		if (itCount%2)
			pLcd[k++]=color;
	}
	else
	{
		for(int i=0;i<itCount;++i)
			pLcd[k++]=color;
	}
}

static void _CopyDrawPos(void){
	kCopy=k;
}
static void _SetCopyDrawPos(void){
	k=kCopy;
}
static void _IncDrawPos(int pos){
	k+=pos;
}

static void _StartDrawLine(uint32_t posBuff,uint32_t BkpSizeX,uint32_t x,uint32_t y){
	k=posBuff+(y*BkpSizeX+x);
}
static uint32_t _GetPosK(uint32_t posBuff,uint32_t BkpSizeX,uint32_t x,uint32_t y){
	return (posBuff+(y*BkpSizeX+x));
}
static uint32_t _GetPosY(uint32_t posBuff,uint32_t BkpSizeX){
	return ((k-posBuff)/BkpSizeX);
}
static uint32_t _GetPosX(uint32_t posBuff,uint32_t BkpSizeX){
	return ((k-posBuff)-_GetPosY(posBuff,BkpSizeX)*BkpSizeX);
}
static structPosition _GetPosXY(uint32_t posBuff, uint32_t BkpSizeX){
	structPosition temp;
	int kOffs = k-posBuff;
	temp.y = (kOffs/BkpSizeX);
	temp.x = (kOffs-temp.y*BkpSizeX);
	return temp;
}
static structPosition _GetPosXY___(uint32_t posBuff, int k, uint32_t BkpSizeX){
	structPosition temp;
	int kOffs = k-posBuff;
	temp.y = (kOffs/BkpSizeX);
	temp.x = (kOffs-temp.y*BkpSizeX);
	return temp;
}
static void _NextDrawLine(uint32_t BkpSizeX,uint32_t width){
	k+=(BkpSizeX-width);
}

static void _DrawRight(int width, uint32_t color)
{
	int j=width;
	uint32_t *p = pLcd+k;
	while(j--)
		*(p++)=color;
	k+=width;
}

static void _DrawLeft(int width, uint32_t color)
{
	int j=width;
	uint32_t *p = pLcd+k;
	while(j--)
		*(p--)=color;
	k-=width;
}

static void _DrawDown(int height, uint32_t color,uint32_t BkpSizeX)
{
	int j=height;
	while(j--)
	{
		pLcd[k]=color;
		k+=BkpSizeX;
	}
}

static void _DrawUp(int height, uint32_t color,uint32_t BkpSizeX)
{
	int j=height;
	while(j--)
	{
		pLcd[k]=color;
		k-=BkpSizeX;
	}
}

static void _DrawRightDown(int width,int height, uint32_t color,uint32_t BkpSizeX)
{
	int j,i,x;

	if(width >= height)
	{
		x=width/height;
		j=height;
		while(j--)
		{
			i=x;
			while(i--)
				pLcd[k++]=color;
			k+=BkpSizeX;
		}
	}
	else
	{
		x=height/width;
		j=width;
		while(j--)
		{
			i=x;
			while(i--)
			{
				pLcd[k]=color;
				k+=BkpSizeX;
			}
			k++;
		}
	}
}

static void _DrawRightDown_AA(int width,int height, uint32_t color,uint32_t BkpSizeX)
{
	int j,i,x;

	if(width >= height)
	{
		x=width/height;
		j=height;
		while(j--)
		{
			i=x;
			while(i--)
				pLcd[k++]=color;

			if(j){
				for(int a=0;a<buff_AA[0];++a)
					pLcd[k+a]=buff_AA[1+a];
			}
			k+=BkpSizeX;

			for(int a=0;a<buff_AA[0];++a)
				pLcd[k-1-a]=buff_AA[1+a];
		}
	}
	else
	{
		x=height/width;
		j=width;
		while(j--)
		{
			i=x;
			while(i--){
				pLcd[k]=color;
				k+=BkpSizeX;
			}

			if(j){
			for(int a=0;a<buff_AA[0];++a)
				pLcd[k+a*BkpSizeX]=buff_AA[1+a];
			}
			k++;

			for(int a=0;a<buff_AA[0];++a)
				pLcd[k-(a+1)*BkpSizeX]=buff_AA[1+a];
		}
	}
}

static void _DrawArrayRightDown(uint32_t color,uint32_t BkpSizeX, int direction, int len, ...)
{
	int j=len,i;
	va_list va;
	va_start(va,0);

	if(0==direction)
	{
		while(j--)
		{
			i=va_arg(va,int);
			while(i--)
				pLcd[k++]=color;
			k+=BkpSizeX;
		}
	}
	else
	{
		while(j--)
		{
			i=va_arg(va,int);
			while(i--){
				pLcd[k]=color;
				k+=BkpSizeX;
			}
			k++;
		}
	}
	va_end(va);
}

static void _DrawArrayRightDown_AA(uint32_t drawColor, uint32_t bkColor, float ratioStart, uint32_t BkpSizeX, int direction, int len, ...)
{
	va_list va;
	va_start(va,0);
	int j=len, i=va_arg(va,int), i_prev;

	if(0==direction)
	{
		while(j--)
		{
			i_prev=i;
			while(i--)
				pLcd[k++]=drawColor;

			i=va_arg(va,int);
			if(j){
				Set_AACoeff_Draw(i,drawColor,bkColor,ratioStart);
				for(int a=0;a<i;++a)
					pLcd[k+a]=buff_AA[1+a];
			}
			k+=BkpSizeX;
			Set_AACoeff_Draw(i_prev,drawColor,bkColor,ratioStart);
			for(int a=0;a<i_prev;++a)
				pLcd[k-1-a]=buff_AA[1+a];
		}
	}
	else
	{
		while(j--)
		{
			i_prev=i;
			while(i--){
				pLcd[k]=drawColor;
				k+=BkpSizeX;
			}

			i=va_arg(va,int);
			if(j){
				Set_AACoeff_Draw(i,drawColor,bkColor,ratioStart);
				for(int a=0;a<buff_AA[0];++a)
					pLcd[k+a*BkpSizeX]=buff_AA[1+a];
			}
			k++;

			Set_AACoeff_Draw(i_prev,drawColor,bkColor,ratioStart);
			for(int a=0;a<i_prev;++a)
				pLcd[k-(a+1)*BkpSizeX]=buff_AA[1+a];
		}
	}
	va_end(va);
}

static void _DrawRightUp(int width,int height, uint32_t color,uint32_t BkpSizeX)
{
	int j,i,x;

	if(width >= height)
	{
		x=width/height;
		j=height;
		while(j--)
		{
			i=x;
			while(i--)
				pLcd[k++]=color;
			k-=BkpSizeX;
		}
	}
	else
	{
		x=height/width;
		j=width;
		while(j--)
		{
			i=x;
			while(i--)
			{
				pLcd[k]=color;
				k-=BkpSizeX;
			}
			k++;
		}
	}
}

static void _DrawRightUp_AA(int width,int height, uint32_t color,uint32_t BkpSizeX)
{
	int j,i,x;

	if(width >= height)
	{
		x=width/height;
		j=height;
		while(j--)
		{
			i=x;
			while(i--)
				pLcd[k++]=color;

			if(j){
				for(int a=0;a<buff_AA[0];++a)
					pLcd[k+a]=buff_AA[1+a];
			}
			k-=BkpSizeX;

			for(int a=0;a<buff_AA[0];++a)
				pLcd[k-1-a]=buff_AA[1+a];
		}
	}
	else
	{
		x=height/width;
		j=width;
		while(j--)
		{
			i=x;
			while(i--){
				pLcd[k]=color;
				k-=BkpSizeX;
			}

			if(j){
				for(int a=0;a<buff_AA[0];++a)
					pLcd[k-a*BkpSizeX]=buff_AA[1+a];
			}
			k++;

			for(int a=0;a<buff_AA[0];++a)
				pLcd[k+(a+1)*BkpSizeX]=buff_AA[1+a];
		}
	}
}

static void _DrawArrayRightUp(uint32_t color,uint32_t BkpSizeX, int direction, int len, ...)
{
	int j=len,i;
	va_list va;
	va_start(va,0);

	if(0==direction)
	{
		while(j--)
		{
			i=va_arg(va,int);
			while(i--)
				pLcd[k++]=color;
			k-=BkpSizeX;
		}
	}
	else
	{
		while(j--)
		{
			i=va_arg(va,int);
			while(i--){
				pLcd[k]=color;
				k-=BkpSizeX;
			}
			k++;
		}
	}
	va_end(va);
}

static void _DrawArrayRightUp_AA(uint32_t drawColor, uint32_t bkColor, float ratioStart, uint32_t BkpSizeX, int direction, int len, ...)
{
	va_list va;
	va_start(va,0);
	int j=len, i=va_arg(va,int), i_prev;

	if(0==direction)
	{
		while(j--)
		{
			i_prev=i;
			while(i--)
				pLcd[k++]=drawColor;

			i=va_arg(va,int);
			if(j){
				Set_AACoeff_Draw(i,drawColor,bkColor,ratioStart);
				for(int a=0;a<i;++a)
					pLcd[k+a]=buff_AA[1+a];
			}
			k-=BkpSizeX;
			Set_AACoeff_Draw(i_prev,drawColor,bkColor,ratioStart);
			for(int a=0;a<i_prev;++a)
				pLcd[k-1-a]=buff_AA[1+a];
		}
	}
	else
	{
		while(j--)
		{
			i_prev=i;
			while(i--){
				pLcd[k]=drawColor;
				k-=BkpSizeX;
			}

			i=va_arg(va,int);
			if(j){
				Set_AACoeff_Draw(i,drawColor,bkColor,ratioStart);
				for(int a=0;a<buff_AA[0];++a)
					pLcd[k-a*BkpSizeX]=buff_AA[1+a];
			}
			k++;

			Set_AACoeff_Draw(i_prev,drawColor,bkColor,ratioStart);
			for(int a=0;a<i_prev;++a)
				pLcd[k+(a+1)*BkpSizeX]=buff_AA[1+a];
		}
	}
	va_end(va);
}

static void _DrawLeftDown(int width,int height, uint32_t color,uint32_t BkpSizeX)
{
	int j,i,x;

	if(width >= height)
	{
		x=width/height;
		j=height;
		while(j--)
		{
			i=x;
			while(i--)
				pLcd[k--]=color;
			k+=BkpSizeX;
		}
	}
	else
	{
		x=height/width;
		j=width;
		while(j--)
		{
			i=x;
			while(i--)
			{
				pLcd[k]=color;
				k+=BkpSizeX;
			}
			k--;
		}
	}
}

static void _DrawLeftDown_AA(int width,int height, uint32_t color,uint32_t BkpSizeX)
{
	int j,i,x;

	if(width >= height)
	{
		x=width/height;
		j=height;
		while(j--)
		{
			i=x;
			while(i--)
				pLcd[k--]=color;

			if(j){
				for(int a=0;a<buff_AA[0];++a)
					pLcd[k-a]=buff_AA[1+a];
			}
			k+=BkpSizeX;

			for(int a=0;a<buff_AA[0];++a)
				pLcd[k+1+a]=buff_AA[1+a];
		}
	}
	else
	{
		x=height/width;
		j=width;
		while(j--)
		{
			i=x;
			while(i--){
				pLcd[k]=color;
				k+=BkpSizeX;
			}

			if(j){
				for(int a=0;a<buff_AA[0];++a)
					pLcd[k+a*BkpSizeX]=buff_AA[1+a];
			}
			k--;

			for(int a=0;a<buff_AA[0];++a)
				pLcd[k-(a+1)*BkpSizeX]=buff_AA[1+a];
		}
	}
}

static void _DrawArrayLeftDown(uint32_t color,uint32_t BkpSizeX, int direction, int len, ...)
{
	int j=len,i;
	va_list va;
	va_start(va,0);

	if(0==direction)
	{
		while(j--)
		{
			i=va_arg(va,int);
			while(i--)
				pLcd[k--]=color;
			k+=BkpSizeX;
		}
	}
	else
	{
		while(j--)
		{
			i=va_arg(va,int);
			while(i--){
				pLcd[k]=color;
				k+=BkpSizeX;
			}
			k--;
		}
	}
	va_end(va);
}

static void _DrawArrayLeftDown_AA(uint32_t drawColor, uint32_t bkColor, float ratioStart, uint32_t BkpSizeX, int direction, int len, ...)
{
	va_list va;
	va_start(va,0);
	int j=len, i=va_arg(va,int), i_prev;

	if(0==direction)
	{
		while(j--)
		{
			i_prev=i;
			while(i--)
				pLcd[k--]=drawColor;

			i=va_arg(va,int);
			if(j){
				Set_AACoeff_Draw(i,drawColor,bkColor,ratioStart);
				for(int a=0;a<i;++a)
					pLcd[k-a]=buff_AA[1+a];
			}
			k+=BkpSizeX;

			Set_AACoeff_Draw(i_prev,drawColor,bkColor,ratioStart);
			for(int a=0;a<i_prev;++a)
				pLcd[k+1+a]=buff_AA[1+a];
		}
	}
	else
	{
		while(j--)
		{
			i_prev=i;
			while(i--){
				pLcd[k]=drawColor;
				k+=BkpSizeX;
			}

			i=va_arg(va,int);
			if(j){
				Set_AACoeff_Draw(i,drawColor,bkColor,ratioStart);
				for(int a=0;a<buff_AA[0];++a)
					pLcd[k+a*BkpSizeX]=buff_AA[1+a];
			}
			k--;

			Set_AACoeff_Draw(i_prev,drawColor,bkColor,ratioStart);
			for(int a=0;a<i_prev;++a)
				pLcd[k-(a+1)*BkpSizeX]=buff_AA[1+a];
		}
	}
	va_end(va);
}

static void _DrawLeftUp(int width,int height, uint32_t color,uint32_t BkpSizeX)
{
	int j,i,x;

	if(width >= height)
	{
		x=width/height;
		j=height;
		while(j--)
		{
			i=x;
			while(i--)
				pLcd[k--]=color;
			k-=BkpSizeX;
		}
	}
	else
	{
		x=height/width;
		j=width;
		while(j--)
		{
			i=x;
			while(i--)
			{
				pLcd[k]=color;
				k-=BkpSizeX;
			}
			k--;
		}
	}
}

static void _DrawLeftUp_AA(int width,int height, uint32_t color,uint32_t BkpSizeX)
{
	int j,i,x;

	if(width >= height)
	{
		x=width/height;
		j=height;
		while(j--)
		{
			i=x;
			while(i--)
				pLcd[k--]=color;

			if(j){
				for(int a=0;a<buff_AA[0];++a)
					pLcd[k-a]=buff_AA[1+a];
			}
			k-=BkpSizeX;

			for(int a=0;a<buff_AA[0];++a)
				pLcd[k+1+a]=buff_AA[1+a];
		}
	}
	else
	{
		x=height/width;
		j=width;
		while(j--)
		{
			i=x;
			while(i--){
				pLcd[k]=color;
				k-=BkpSizeX;
			}

			if(j){
				for(int a=0;a<buff_AA[0];++a)
					pLcd[k-a*BkpSizeX]=buff_AA[1+a];
			}
			k--;

			for(int a=0;a<buff_AA[0];++a)
				pLcd[k+(a+1)*BkpSizeX]=buff_AA[1+a];
		}
	}
}

static void _DrawArrayLeftUp(uint32_t color,uint32_t BkpSizeX, int direction, int len, ...)
{
	int j=len,i;
	va_list va;
	va_start(va,0);

	if(0==direction)
	{
		while(j--)
		{
			i=va_arg(va,int);
			while(i--)
				pLcd[k--]=color;
			k-=BkpSizeX;
		}
	}
	else
	{
		while(j--)
		{
			i=va_arg(va,int);
			while(i--){
				pLcd[k]=color;
				k-=BkpSizeX;
			}
			k--;
		}
	}
	va_end(va);
}

static void _DrawArrayLeftUp_AA(uint32_t drawColor, uint32_t bkColor, float ratioStart, uint32_t BkpSizeX, int direction, int len, ...)
{
	va_list va;
	va_start(va,0);
	int j=len, i=va_arg(va,int), i_prev;

	if(0==direction)
	{
		while(j--)
		{
			i_prev=i;
			while(i--)
				pLcd[k--]=drawColor;

			i=va_arg(va,int);
			if(j){
				Set_AACoeff_Draw(i,drawColor,bkColor,ratioStart);
				for(int a=0;a<i;++a)
					pLcd[k-a]=buff_AA[1+a];
			}
			k-=BkpSizeX;

			Set_AACoeff_Draw(i_prev,drawColor,bkColor,ratioStart);
			for(int a=0;a<i_prev;++a)
				pLcd[k+1+a]=buff_AA[1+a];
		}
	}
	else
	{
		while(j--)
		{
			i_prev=i;
			while(i--){
				pLcd[k]=drawColor;
				k-=BkpSizeX;
			}

			i=va_arg(va,int);
			if(j){
				Set_AACoeff_Draw(i,drawColor,bkColor,ratioStart);
				for(int a=0;a<buff_AA[0];++a)
					pLcd[k-a*BkpSizeX]=buff_AA[1+a];
			}
			k--;

			Set_AACoeff_Draw(i_prev,drawColor,bkColor,ratioStart);
			for(int a=0;a<i_prev;++a)
				pLcd[k+(a+1)*BkpSizeX]=buff_AA[1+a];
		}
	}
	va_end(va);
}

static void _Middle_RoundRectangleFrame(int rectangleFrame, int fillHeight, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpSizeX, uint32_t width, uint32_t height){
	int _height = height-fillHeight;
	int _width = width-2;
	if(rectangleFrame)
	{
		for (int j=0; j<_height; j++)
		{
			_FillBuff(1, FrameColor);
			_FillBuff(_width, FillColor);
			_FillBuff(1, FrameColor);
			_NextDrawLine(BkpSizeX,width);
		}
	}
	else
	{
		for (int j=0; j<_height; j++)
		{
			_FillBuff(1, FrameColor);
			k+=_width;
			_FillBuff(1, FrameColor);
			_NextDrawLine(BkpSizeX,width);
		}
	}
}

static void LCD_DrawRoundRectangleFrame(int rectangleFrame, uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor)
{
	#define A(a,b) 	_FillBuff(a,b)

	uint8_t thickness = BkpColor>>24;
	uint32_t o1,o2;
	uint32_t i1 = GetTransitionColor(FrameColor,FillColor,AA.c1);
	uint32_t i2 = GetTransitionColor(FrameColor,FillColor,AA.c2);

	if((thickness==0)||(thickness==255)){
		o1 = GetTransitionColor(FrameColor,BkpColor,AA.c1);
		o2 = GetTransitionColor(FrameColor,BkpColor,AA.c2);
	}

	void _Fill(int x)
	{
		if(rectangleFrame)
			A(x,FillColor);
		else
			k+=x;
	}

	void _Out_AA_left(int stage)
	{
		if((thickness==0)||(thickness==255))
		{	switch(stage)
			{
			case 0:	A(3,BkpColor); A(1,o2); A(1,o1);  break;
			case 1:	A(2,BkpColor); A(1,o1);  break;
			case 2:	A(1,BkpColor); A(1,o1);  break;
			case 3:	A(1,o2); break;
			case 4:	A(1,o1); break;
			}
		}
		else
		{  switch(stage)
			{
			case 0:	k+=5; break;
			case 1:	k+=3; break;
			case 2:	k+=2; break;
			case 3:	k+=1; break;
			case 4:	k+=1; break;
			}
		}
	}

	void _Out_AA_right(int stage)
	{
		if((thickness==0)||(thickness==255))
		{	switch(stage)
			{
			case 0:	A(1,o1); A(1,o2); A(3,BkpColor);  break;
			case 1:	A(1,o1); A(2,BkpColor);  break;
			case 2:	A(1,o1); A(1,BkpColor);  break;
			case 3:	A(1,o2); break;
			case 4:	A(1,o1); break;
			}
		}
		else
		{	switch(stage)
			{
			case 0:	k+=5; break;
			case 1:	k+=3; break;
			case 2:	k+=2; break;
			case 3:	k+=1; break;
			case 4:	k+=1; break;
			}
		}
	}

	_StartDrawLine(posBuff,BkpSizeX,x,y);
	_Out_AA_left(0); A(width-10,FrameColor); _Out_AA_right(0);
	_NextDrawLine(BkpSizeX,width);
	_Out_AA_left(1); A(2,FrameColor); A(1,i1);A(1,i2); _Fill(width-14); A(1,i2);A(1,i1);A(2,FrameColor); _Out_AA_right(1);
	_NextDrawLine(BkpSizeX,width);
	_Out_AA_left(2); A(1,FrameColor); A(1,i1); _Fill(width-8); A(1,i1); A(1,FrameColor); _Out_AA_right(2);
	_NextDrawLine(BkpSizeX,width);
	_Out_AA_left(3); A(1,FrameColor); A(1,i1); _Fill(width-6); A(1,i1); A(1,FrameColor); _Out_AA_right(3);
	_NextDrawLine(BkpSizeX,width);
	_Out_AA_left(4); A(1,FrameColor); _Fill(width-4); A(1,FrameColor); _Out_AA_right(4);
	_NextDrawLine(BkpSizeX,width);

	A(1,FrameColor);  A(1,i1); _Fill(width-4); A(1,i1); A(1,FrameColor);
	_NextDrawLine(BkpSizeX,width);
	A(1,FrameColor);  A(1,i2); _Fill(width-4); A(1,i2); A(1,FrameColor);
	_NextDrawLine(BkpSizeX,width);

	_Middle_RoundRectangleFrame(rectangleFrame,14,FrameColor,FillColor,BkpSizeX,width,height);

	A(1,FrameColor);  A(1,i2); _Fill(width-4); A(1,i2); A(1,FrameColor);
	_NextDrawLine(BkpSizeX,width);
	A(1,FrameColor);  A(1,i1); _Fill(width-4); A(1,i1); A(1,FrameColor);
	_NextDrawLine(BkpSizeX,width);

	_Out_AA_left(4); A(1,FrameColor); _Fill(width-4); A(1,FrameColor); _Out_AA_right(4);
	_NextDrawLine(BkpSizeX,width);
	_Out_AA_left(3); A(1,FrameColor); A(1,i1); _Fill(width-6); A(1,i1); A(1,FrameColor); _Out_AA_right(3);
	_NextDrawLine(BkpSizeX,width);
	_Out_AA_left(2); A(1,FrameColor); A(1,i1); _Fill(width-8); A(1,i1); A(1,FrameColor); _Out_AA_right(2);
	_NextDrawLine(BkpSizeX,width);
	_Out_AA_left(1); A(2,FrameColor); A(1,i1);A(1,i2); _Fill(width-14); A(1,i2);A(1,i1);A(2,FrameColor); _Out_AA_right(1);
	_NextDrawLine(BkpSizeX,width);
	_Out_AA_left(0); A(width-10,FrameColor); _Out_AA_right(0);

	#undef  A
}

static void PixelCorrect(uint32_t drawColor, uint32_t bkColor, float ratioStart, uint16_t pxlLen, int multiplier){
	Set_AACoeff_Draw(pxlLen,drawColor,bkColor,ratioStart);
	for(int a=0;a<pxlLen;++a)
		pLcd[k+multiplier*a]=buff_AA[1+a];
}

static void InverseAndCopyBuff(uint8_t *buf_inv, uint8_t *buf){
	buf_inv[0]=buf[0];
	for(int i=0;i<buf[0];i++)
		buf_inv[1+i]=buf[buf[0]-i];
}

static void _OffsetRightDown(uint32_t BkpSizeX, int direction, uint8_t *buf){
	int j=buf[0], i=buf[1], p=2;
	if(0==direction)
	{
		while(j--){
			k+=i;
			if(j) i=buf[p++];
			k+=BkpSizeX;
		}
	}
	else
	{
		while(j--){
			k+=i*BkpSizeX;
			if(j) i=buf[p++];
			k++;
		}
	}
}

static void _OffsetLeftDown(uint32_t BkpSizeX, int direction, uint8_t *buf){
	int j=buf[0], i=buf[1], p=2;
	if(0==direction)
	{
		while(j--){
			k-=i;
			if(j) i=buf[p++];
			k+=BkpSizeX;
		}
	}
	else
	{
		while(j--){
			k+=i*BkpSizeX;
			if(j) i=buf[p++];
			k--;
		}
	}
}

static void _OffsetLeftUp(uint32_t BkpSizeX, int direction, uint8_t *buf){
	int j=buf[0], i=buf[1], p=2;
	if(0==direction)
	{
		while(j--){
			k-=i;
			if(j) i=buf[p++];
			k-=BkpSizeX;
		}
	}
	else
	{
		while(j--){
			k-=i*BkpSizeX;
			if(j) i=buf[p++];
			k--;
		}
	}
}

static void LCD_CircleCorrect(void)
{
/*	LCD_OffsCircleLine(1,1);  LCD_OffsCircleLine(2,-1);
	LCD_OffsCircleLine(1,2);  LCD_OffsCircleLine(2,-1); LCD_OffsCircleLine(3,-1);
	LCD_OffsCircleLine(1,4);  LCD_OffsCircleLine(2,3); LCD_OffsCircleLine(3,2);  LCD_OffsCircleLine(3,1); */
}

static int16_t GetDegFromPosK( uint32_t posBuff, uint16_t x0, uint16_t y0, uint32_t BkpSizeX)
{
	pos = _GetPosXY(posBuff,BkpSizeX);
	float deg = DEG(atan2(pos.y-y0, pos.x-x0));
	return (int16_t)deg + 180;//<0 ? 180.0+deg : deg;
}

static void OffsetKfromLineBuff(int itBuff, uint32_t BkpSizeX){
	uint32_t offs= (itBuff-1)*BkpSizeX;
	for(int i=1;i<itBuff;++i){
		if(i>=MAX_LINE_BUFF_CIRCLE_SIZE) break;
		offs += Circle.lineBuff[i];
	}
	k+=offs;
}

static uint8_t LCD_SearchLinePoints(int startMeasure, int posBuff, int x0,int y0, int deg, uint32_t BkpSizeX)
{
	static float tang, coeff;

	if(startMeasure)
	{
		tang= tan(TANG_ARG(deg));
		coeff = 1/ABS(tang);

		if(deg>0 && deg<=45)
			return 1;
		else if(deg>45 && deg<=90)
			return 2;
		else if(deg>90 && deg<=135)
			return 3;
		else if(deg>135 && deg<=180)
			return 4;
		else if(deg>180 && deg<=225)
			return 5;
		else if(deg>225 && deg<=270)
			return 6;
		else if(deg>270 && deg<=315)
			return 7;
		else if(deg>315 && deg<=360)
			return 8;
	}
	else
	{
   	pos = _GetPosXY(posBuff,BkpSizeX);
   	float xxx = pos.x - x0;
		float yyy = pos.y - y0;
		float _yyy = tang*xxx;

		if(deg>0 && deg<=45)
		{
				  if(yyy<_yyy)	                                return 0;
			else if( (yyy>=_yyy) && (yyy<tang*(xxx+coeff)) )  return 1;
			else															  return 2;
		}
		else if(deg>45 && deg<=90)
		{
			  if(yyy<_yyy)	                                return 0;
		else if( (yyy>=_yyy) && (yyy<tang*(xxx+1)) )      return 1;
		else															  return 2;
		}

		else if(deg>90 && deg<=135)
		{
			  if(yyy>_yyy)	                                return 0;
		else if( (yyy<=_yyy) && (yyy>tang*(xxx+1)) )      return 1;
		else															  return 2;
		}
		else if(deg>135 && deg<=180)
		{
			  if(yyy>_yyy)	                                return 0;
		else if( (yyy<=_yyy) && (yyy>tang*(xxx+coeff)) )  return 1;
		else															  return 2;
		}

		else if(deg>180 && deg<=225)
		{
				  if(yyy<_yyy)	                                return 2;
			else if( (yyy>=_yyy) && (yyy<tang*(xxx+coeff)) )  return 1;
			else															  return 0;
		}
		else if(deg>225 && deg<=270)
		{
			  if(yyy<_yyy)	                                return 2;
		else if( (yyy>=_yyy) && (yyy<tang*(xxx+1)) )      return 1;
		else															  return 0;
		}

		else if(deg>270 && deg<=315)
		{
			  if(yyy>_yyy)	                                return 2;
		else if( (yyy<=_yyy) && (yyy>tang*(xxx+1)) )      return 1;
		else															  return 0;
		}
		else if(deg>315 && deg<=360)
		{
			  if(yyy>_yyy)	                                return 2;
		else if( (yyy<=_yyy) && (yyy>tang*(xxx+coeff)) )  return 1;
		else															  return 0;
		}

		else
			return 3;
	}
	return 0;
}

static uint8_t LCD_SearchRadiusPoints(int posBuff, int nrDeg, uint32_t BkpSizeX)
{
	float tang= Circle.tang[nrDeg];
	float coeff=Circle.coeff[nrDeg];
	uint16_t deg = Circle.degree[1+nrDeg];

   	pos = _GetPosXY(posBuff,BkpSizeX);
   	float xxx = pos.x - Circle.x0;
		float yyy = pos.y - Circle.y0;
		float _yyy = tang*xxx;

		if(deg>0 && deg<=45)
		{
				  if(yyy<_yyy)	                                return 0;
			else if( (yyy>=_yyy) && (yyy<tang*(xxx+coeff)) )  return 1;
			else															  return 2;
		}
		else if(deg>45 && deg<=90)
		{
			  if(yyy<_yyy)	                                return 0;
		else if( (yyy>=_yyy) && (yyy<tang*(xxx+1)) )      return 1;
		else															  return 2;
		}

		else if(deg>90 && deg<=135)
		{
			  if(yyy>_yyy)	                                return 0;
		else if( (yyy<=_yyy) && (yyy>tang*(xxx+1)) )      return 1;
		else															  return 2;
		}
		else if(deg>135 && deg<=180)
		{
			  if(yyy>_yyy)	                                return 0;
		else if( (yyy<=_yyy) && (yyy>tang*(xxx+coeff)) )  return 1;
		else															  return 2;
		}

		else if(deg>180 && deg<=225)
		{
				  if(yyy<_yyy)	                                return 2;
			else if( (yyy>=_yyy) && (yyy<tang*(xxx+coeff)) )  return 1;
			else															  return 0;
		}
		else if(deg>225 && deg<=270)
		{
			  if(yyy<_yyy)	                                return 2;
		else if( (yyy>=_yyy) && (yyy<tang*(xxx+1)) )      return 1;
		else															  return 0;
		}

		else if(deg>270 && deg<=315)
		{
			  if(yyy>_yyy)	                                return 2;
		else if( (yyy<=_yyy) && (yyy>tang*(xxx+1)) )      return 1;
		else															  return 0;
		}
		else if(deg>315 && deg<=360)
		{
			  if(yyy>_yyy)	                                return 2;
		else if( (yyy<=_yyy) && (yyy>tang*(xxx+coeff)) )  return 1;
		else															  return 0;
		}

		else
			return 3;
}

static void _DrawArrayBuffRightDown_AA(uint32_t _drawColor, uint32_t outColor, uint32_t inColor, float outRatioStart, float inRatioStart, uint32_t BkpSizeX, int direction, uint8_t *buf)
{
	int j=buf[0], i=buf[1], p=2, i_prev;
	uint32_t drawColor=_drawColor;
	uint32_t _outColor=outColor;

	if(0==direction)
	{
		while(j--)
		{
			i_prev=i;
			while(i--){
				pLcd[k++]=drawColor;
			}
			if(0==outColor) _outColor = pLcd[k];
			if(j){
				i=buf[p++];
				Set_AACoeff_Draw(i,drawColor,_outColor,outRatioStart);
				for(int a=0;a<buff_AA[0];++a){	if(0==outColor){ if(pLcd[k+a] != _outColor) break; }
					pLcd[k+a]=buff_AA[1+a];
				}
			}
			k+=BkpSizeX;

			Set_AACoeff_Draw(i_prev,drawColor,inColor,inRatioStart);
			for(int a=0;a<buff_AA[0];++a)
				pLcd[k-1-a]=buff_AA[1+a];
		}
	}
	else
	{
		while(j--)
		{
			i_prev=i;
			while(i--){
				pLcd[k]=drawColor;
				k+=BkpSizeX;
			}
			if(j){
				i=buf[p++];
				Set_AACoeff_Draw(i,drawColor,inColor,inRatioStart);
				for(int a=0;a<buff_AA[0];++a)
					pLcd[k+a*BkpSizeX]=buff_AA[1+a];
			}
			k++;
			if(0==outColor) _outColor = pLcd[k-BkpSizeX];
			Set_AACoeff_Draw(i_prev,drawColor,_outColor,outRatioStart);
			for(int a=0;a<buff_AA[0];++a){	if(0==outColor){ if(pLcd[k-(a+1)*BkpSizeX] != _outColor) break; }
				pLcd[k-(a+1)*BkpSizeX]=buff_AA[1+a];
			}
		}
	}
}

static void _DrawArrayBuffLeftDown_AA(uint32_t drawColor, uint32_t outColor, uint32_t inColor, float outRatioStart, float inRatioStart, uint32_t BkpSizeX, int direction, uint8_t *buf)
{
	int j=buf[0], i=buf[1], p=2, i_prev;
	uint32_t _outColor=outColor;

	if(0==direction)
	{
		while(j--)
		{
			i_prev=i;
			while(i--)
				pLcd[k--]=drawColor;

			if(j){
				i=buf[p++];
				Set_AACoeff_Draw(i,drawColor,inColor,inRatioStart);
				for(int a=0;a<buff_AA[0];++a)
					pLcd[k-a]=buff_AA[1+a];
			}
			k+=BkpSizeX;
			if(0==outColor) _outColor = pLcd[k];
			Set_AACoeff_Draw(i_prev,drawColor,_outColor,outRatioStart);
			for(int a=0;a<buff_AA[0];++a){	if(0==outColor){ if(pLcd[k+1+a] != _outColor) break; }
				pLcd[k+1+a]=buff_AA[1+a];
			}
		}
	}
	else
	{
		while(j--)
		{
			i_prev=i;
			while(i--){
				pLcd[k]=drawColor;
				k+=BkpSizeX;
			}
			if(0==outColor) _outColor = pLcd[k];
			if(j){
				i=buf[p++];
				Set_AACoeff_Draw(i,drawColor,_outColor,outRatioStart);
				for(int a=0;a<buff_AA[0];++a){	if(0==outColor){ if(pLcd[k+a*BkpSizeX] != _outColor) break; }
					pLcd[k+a*BkpSizeX]=buff_AA[1+a];
				}
			}
			k--;
			Set_AACoeff_Draw(i_prev,drawColor,inColor,inRatioStart);
			for(int a=0;a<buff_AA[0];++a)
				pLcd[k-(a+1)*BkpSizeX]=buff_AA[1+a];
		}
	}
}

static void _DrawArrayBuffLeftUp_AA(uint32_t drawColor, uint32_t outColor, uint32_t inColor, float outRatioStart, float inRatioStart, uint32_t BkpSizeX, int direction, uint8_t *buf)
{
	int j=buf[0], i=buf[1], p=2, i_prev;
	uint32_t _outColor=outColor;

	if(0==direction)
	{
		while(j--)
		{
			i_prev=i;
			while(i--)
				pLcd[k--]=drawColor;

			if(0==outColor) _outColor = pLcd[k];
			if(j){
				i=buf[p++];
				Set_AACoeff_Draw(i,drawColor,_outColor,outRatioStart);
				for(int a=0;a<buff_AA[0];++a){	if(0==outColor){ if(pLcd[k-a] != _outColor) break; }
					pLcd[k-a]=buff_AA[1+a];
				}
			}
			k-=BkpSizeX;

			Set_AACoeff_Draw(i_prev,drawColor,inColor,inRatioStart);
			for(int a=0;a<buff_AA[0];++a)
				pLcd[k+1+a]=buff_AA[1+a];
		}
	}
	else
	{
		while(j--)
		{
			i_prev=i;
			while(i--){
				pLcd[k]=drawColor;
				k-=BkpSizeX;
			}

			if(j){
				i=buf[p++];
				Set_AACoeff_Draw(i,drawColor,inColor,inRatioStart);
				for(int a=0;a<buff_AA[0];++a)
					pLcd[k-a*BkpSizeX]=buff_AA[1+a];
			}
			k--;
			if(0==outColor) _outColor = pLcd[k+BkpSizeX];
			Set_AACoeff_Draw(i_prev,drawColor,_outColor,outRatioStart);
			for(int a=0;a<buff_AA[0];++a){	if(0==outColor){ if(pLcd[k+(a+1)*BkpSizeX] != _outColor) break; }
				pLcd[k+(a+1)*BkpSizeX]=buff_AA[1+a];
			}
		}
	}
}

static void _DrawArrayBuffRightUp_AA(uint32_t drawColor, uint32_t outColor, uint32_t inColor, float outRatioStart, float inRatioStart, uint32_t BkpSizeX, int direction, uint8_t *buf)
{
	int j=buf[0], i=buf[1], p=2, i_prev;
	uint32_t _outColor=outColor;

	if(0==direction)
	{
		while(j--)
		{
			i_prev=i;
			while(i--)
				pLcd[k++]=drawColor;

			if(j){
				i=buf[p++];
				Set_AACoeff_Draw(i,drawColor,inColor,inRatioStart);
				for(int a=0;a<buff_AA[0];++a){
					pLcd[k+a]=buff_AA[1+a];
				}
			}

			if(k > BkpSizeX) k -= BkpSizeX;
			{	if(0==outColor) _outColor = pLcd[k-1];
				Set_AACoeff_Draw(i_prev,drawColor,_outColor,outRatioStart);
				for(int a=0;a<buff_AA[0];++a){	if(0==outColor){ if(pLcd[k-1-a] != _outColor) break; } 	if(k < 1+a) break;
					pLcd[k-1-a]=buff_AA[1+a];
			}}
		}
	}
	else
	{
		while(j--)
		{
			i_prev=i;
			while(i--){
				pLcd[k]=drawColor;
				k-=BkpSizeX;
			}
			if(0==outColor) _outColor = pLcd[k];
			if(j){
				i=buf[p++];
				Set_AACoeff_Draw(i,drawColor,_outColor,outRatioStart);
				for(int a=0;a<buff_AA[0];++a){	if(0==outColor){ if(pLcd[k-a*BkpSizeX] != _outColor) break; }
					pLcd[k-a*BkpSizeX]=buff_AA[1+a];
				}
			}
			k++;
			Set_AACoeff_Draw(i_prev,drawColor,inColor,inRatioStart);
			for(int a=0;a<buff_AA[0];++a)
				pLcd[k+(a+1)*BkpSizeX]=buff_AA[1+a];
		}
	}
}

static void LCD_DrawCircle(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t __x, uint32_t __y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor, int outColorRead)
{
	int matchWidth=0, circleFlag=0;													/* Circle.degColor[0] - is free space for future variable */
	uint16_t param =width>>16;
	uint32_t _FillColor, _width=(width&0x0000FFFF)-matchWidth;

	if(FillColor!=TRANSPARENT) _FillColor=FillColor;
	else								_FillColor=BkpColor;

	#define PARAM 		FrameColor, BkpColor, Circle.outRatioStart
	#define PARAM_AA 	FrameColor, BkpColor, _FillColor, Circle.outRatioStart, Circle.inRatioStart, BkpSizeX
	#define buf			Circle.lineBuff

	uint32_t bkX = BkpSizeX;
	uint32_t x=__x, y=__y;

	if((_width==height)&&(_width>0)&&(height>0))
	{
		GOTO_ToCalculateRadius:
	   _width=(width&0x0000FFFF)-matchWidth;

		float R=((float)_width)/2;
		uint32_t pxl_width = R/3;

		x=__x+_width/2 + matchWidth/2;
		if(_width%2) x++;
		y=__y;

		float _x=(float)x, _y=(float)y;
		float x0=(float)x, y0=(float)y+R;

		float param_y = pow(y0-_y,2);
		float param_x = pow(_x-x0,2);
		float decision = pow(R,2);

		int pxl_line=0,i=0;
		uint8_t block=1;

		void _CorrectDecision(void){
			if(block){	 int ni= CONDITION(R<=Circle.correctForWidth,0,1);
				if(i >= VALPERC(pxl_width,Circle.correctPercDeg[ni])){	block=0;   decision= pow(R+Circle.errorDecision[ni],2);  }
		}}

		buf[0]=pxl_width;
		do{
			_x++;  pxl_line++;
			param_x = pow(_x-x0,2);		_CorrectDecision();
			if((param_x+param_y) > decision){
				_y++;
				param_y = pow(y0-_y,2);
				buf[1+i++]=pxl_line-1;
				pxl_line=1;
			}
		}while(i<pxl_width);

		LCD_CircleCorrect();

  	 uint16_t _height=buf[0];
  	 for(int i=0;i<buf[0];++i) _height+=buf[buf[0]-i];
  	 _height = 2*_height;

		if(height>=_height)
			y= y+(height-_height)/2;
		else{
			matchWidth+=1;
			goto GOTO_ToCalculateRadius;
		}

		x-=(x-__x-_height/2);
		y-=(y-__y);

		Circle.width=_height;
		Circle.x0=x;
		Circle.y0=y+Circle.width/2;
		circleFlag=1;
	}
	else if((_width==0)&&(height==0));
	else return;

	uint8_t buf_Inv[buf[0]+1];
	InverseAndCopyBuff(buf_Inv,buf);
	_StartDrawLine(posBuff,bkX,x,y);

	uint32_t BkpColor_copy = BkpColor;
	if(outColorRead) BkpColor=0;
		_DrawArrayBuffRightDown_AA(PARAM_AA,0,buf); 	    _CopyDrawPos(); _IncDrawPos(-bkX); PixelCorrect(PARAM, 1,      1);    _SetCopyDrawPos();
		_DrawArrayBuffRightDown_AA(PARAM_AA,1,buf_Inv);                      				   PixelCorrect(PARAM, buf[1],bkX);   _IncDrawPos(-1);

		_DrawArrayBuffLeftDown_AA (PARAM_AA,1,buf); 		 _CopyDrawPos(); _IncDrawPos(1);    PixelCorrect(PARAM, 1,      1);    _SetCopyDrawPos();
		_DrawArrayBuffLeftDown_AA (PARAM_AA,0,buf_Inv);                      					PixelCorrect(PARAM, buf[1],-1);    _IncDrawPos(-bkX);

		_DrawArrayBuffLeftUp_AA  (PARAM_AA,0,buf);       _CopyDrawPos(); _IncDrawPos(bkX);  PixelCorrect(PARAM, 1,     -bkX);  _SetCopyDrawPos();
		_DrawArrayBuffLeftUp_AA  (PARAM_AA,1,buf_Inv);  						    					PixelCorrect(PARAM, buf[1],-bkX);  _IncDrawPos(1);

		_DrawArrayBuffRightUp_AA (PARAM_AA,1,buf);       _CopyDrawPos(); _IncDrawPos(-1);  	PixelCorrect(PARAM, 1,      1);    _SetCopyDrawPos();
		_DrawArrayBuffRightUp_AA (PARAM_AA,0,buf_Inv);                       					PixelCorrect(PARAM, buf[1], 1);
	if(outColorRead) BkpColor = BkpColor_copy;

		if(FillColor!=TRANSPARENT)
		{
	   	 uint16_t fillWidth, fillHeight=0;

			_StartDrawLine(posBuff,bkX,x,y);
			fillWidth=0;
			for(int i=0;i<buf[0];++i){
				_IncDrawPos(bkX-buf[i+1]);	_CopyDrawPos(); k+=buf[i+1];	if(i>0) _DrawRight(2*fillWidth,FillColor);	_SetCopyDrawPos();	fillHeight++;
				fillWidth += buf[i+1];
			}

			for(int i=0;i<buf[0];++i){
				fillWidth++;
				_IncDrawPos(-1);
				for(int j=0; j<buf[buf[0]-i]; ++j){
					_IncDrawPos(bkX);	_CopyDrawPos(); if(j==buf[buf[0]-i]-1){k+=1;_DrawRight(2*(fillWidth-1),FillColor);}else{k+=2;_DrawRight(2*(fillWidth-2),FillColor);}	_SetCopyDrawPos();	fillHeight++;
				}
			}

			fillHeight = 2*fillHeight-1;

			_StartDrawLine(posBuff,bkX,x,y+fillHeight);
			fillWidth=0;
			for(int i=0;i<buf[0];++i){
				_IncDrawPos(-bkX-buf[i+1]);	_CopyDrawPos();	 k+=buf[i+1];	if(i>0) _DrawRight(2*fillWidth,FillColor);	_SetCopyDrawPos();
				fillWidth += buf[i+1];
			}

			for(int i=0;i<buf[0];++i){
				fillWidth++;
				_IncDrawPos(-1);
				for(int j=0; j<buf[buf[0]-i]; ++j){
					_IncDrawPos(-bkX);	_CopyDrawPos();  if(j==buf[buf[0]-i]-1){k+=1;_DrawRight(2*(fillWidth-1),FillColor);}else{k+=2;_DrawRight(2*(fillWidth-2),FillColor);}	 _SetCopyDrawPos();
				}
			}
		}

	if(param==Percent_Circle && circleFlag)
	{
		int circleLinesLenCorrect;
		uint16_t fillWidth=0, fillHeight=0;

		for(int i=0;i<Circle.degree[0];++i)
		{
			circleLinesLenCorrect=0;
			if(IS_RANGE(Circle.degree[1+i], 300, 330) ||
				IS_RANGE(Circle.degree[1+i], 206, 244) ||
				IS_RANGE(Circle.degree[1+i], 122, 149) )
			{
				if(Circle.width/2 < 100 /* CORRECT_FOR_RADIUS */)
					circleLinesLenCorrect = 2;
				else
					circleLinesLenCorrect = 4;
			}
			else if(IS_RANGE(Circle.degree[1+i],  34,  56)){
				if(Circle.width/2 < 100)
					circleLinesLenCorrect = 0;
				else
					circleLinesLenCorrect = 2;
			}


			if(i==0)
				DrawLine(0,Circle.x0,Circle.y0,(Circle.width-4-circleLinesLenCorrect)/2-1,Circle.degree[1+i],FrameColor,BkpSizeX, Circle.outRatioStart,Circle.inRatioStart,_FillColor,Circle.degColor[i+1]);
			else if(i==Circle.degree[0]-1)
				DrawLine(0,Circle.x0,Circle.y0,(Circle.width-4-circleLinesLenCorrect)/2-1,Circle.degree[1+i],FrameColor,BkpSizeX, Circle.outRatioStart,Circle.inRatioStart,Circle.degColor[i],_FillColor);
			else
				DrawLine(0,Circle.x0,Circle.y0,(Circle.width-4-circleLinesLenCorrect)/2-1,Circle.degree[1+i],FrameColor,BkpSizeX, Circle.outRatioStart,Circle.inRatioStart,Circle.degColor[i],Circle.degColor[i+1]);
		}

		void _Fill(int width)
		{
			int j=width;
			uint8_t threshold[Circle.degree[0]];
			int subj=0, deltaDeg;
			uint32_t colorDeg=0;	uint8_t flagPermission=0;

			int _GGGG(void){
				for(int i=0;i<Circle.degree[0]-1;++i){
					deltaDeg=Circle.degree[1+i+1]-Circle.degree[1+i];
					if( ((deltaDeg>0)&&(deltaDeg<=180)) || (deltaDeg<-180) ){
						if((threshold[i]==0)&&(threshold[i+1]==2)){
							colorDeg= Circle.degColor[1+i];
							return 1;
						}
					}
					else{
						if((threshold[i]==0)||(threshold[i+1]==2)){
							colorDeg= Circle.degColor[1+i];
							return 2;
				}}}
				return 0;
			}

			while(j--)
			{
				if(pLcd[k]==FillColor){
					for(int i=0;i<Circle.degree[0];++i)
						threshold[i]=LCD_SearchRadiusPoints(posBuff,i,bkX);
					flagPermission=_GGGG();
				}

				GOTO_ToFillCircle:
				if(pLcd[k]==FillColor)
				{
					if(flagPermission) pLcd[k]= colorDeg;
					k++; subj=1;
				}
				if(pLcd[k]==FillColor){ if(j==0){ k--; break; } j--; subj=0; goto GOTO_ToFillCircle; }
				k++;
				if(subj==1){ subj=0; if(j>0) j--; else{ k--; break; }}
			}
		}

		void _Change_AA(int width)
		{
			int j=width;
			int threshold[Circle.degree[0]];
			int deltaDeg;

			while(j--)
			{
				for(int i=0;i<Circle.degree[0];++i)
					threshold[i]=LCD_SearchRadiusPoints(posBuff,i,bkX);

				for(int i=0;i<Circle.degree[0]-1;++i)
				{
					deltaDeg=Circle.degree[1+i+1]-Circle.degree[1+i];
					if( ((deltaDeg>0)&&(deltaDeg<=180)) || (deltaDeg<-180) )
					{
						if((threshold[i]==0)&&(threshold[i+1]==2)){
							pLcd[k]=GetTransitionColor(FrameColor, Circle.degColor[1+i], GetTransitionCoeff(FrameColor,FillColor,pLcd[k]));
							break;
						}
					}
					else
					{
						if((threshold[i]==0)||(threshold[i+1]==2)){
							pLcd[k]=GetTransitionColor(FrameColor, Circle.degColor[1+i], GetTransitionCoeff(FrameColor,FillColor,pLcd[k]));
							break;
				}}}
				k++;
		}}

		_StartDrawLine(posBuff,bkX,x,y);
		for(int i=0;i<buf[0];++i){
			_IncDrawPos(bkX-buf[i+1]);	  _CopyDrawPos();   _Change_AA(buf[i+1]); 	if(i>0) _Fill(2*fillWidth); _Change_AA(buf[i+1]); _SetCopyDrawPos();	fillHeight++;
			fillWidth += buf[i+1];
		}
		for(int i=0;i<buf[0];++i){
			fillWidth++;
			_IncDrawPos(-1);
			for(int j=0; j<buf[buf[0]-i]; ++j){
				_IncDrawPos(bkX);	_CopyDrawPos(); if(j==buf[buf[0]-i]-1){ _Change_AA(1); _Fill(2*(fillWidth-1)); _Change_AA(1); }else{  _Change_AA(2); _Fill(2*(fillWidth-2)); _Change_AA(2); }	_SetCopyDrawPos();	fillHeight++;
			}
		}

		fillHeight = 2*fillHeight-1;
		_StartDrawLine(posBuff,bkX,x,y+fillHeight);
		fillWidth=0;
		for(int i=0;i<buf[0];++i){
			_IncDrawPos(-bkX-buf[i+1]);	_CopyDrawPos();	_Change_AA(buf[i+1]); if(i>0) _Fill(2*fillWidth); _Change_AA(buf[i+1]);	_SetCopyDrawPos();
			fillWidth += buf[i+1];
		}

		for(int i=0;i<buf[0];++i){
			fillWidth++;
			_IncDrawPos(-1);
			for(int j=0; j<buf[buf[0]-i]; ++j){
				_IncDrawPos(-bkX);	_CopyDrawPos();  if(j==buf[buf[0]-i]-1){ _Change_AA(1); _Fill(2*(fillWidth-1)); _Change_AA(1);}else{ _Change_AA(2); _Fill(2*(fillWidth-2)); _Change_AA(2);}	 _SetCopyDrawPos();
			}
		}
	}
	#undef PARAM
	#undef PARAM_AA
	#undef buf
}

static void LCD_DrawHalfCircle(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t __x, uint32_t __y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor)
{
	int matchWidth=0;
	uint32_t _FillColor, _width=(width&0x0000FFFF)-matchWidth;

	if(FillColor!=TRANSPARENT) _FillColor=FillColor;
	else								_FillColor=BkpColor;

	#define PARAM 		FrameColor, BkpColor, Circle.outRatioStart
	#define PARAM_AA 	FrameColor, BkpColor, _FillColor, Circle.outRatioStart, Circle.inRatioStart, BkpSizeX
	#define buf			Circle.lineBuff

	uint32_t bkX = BkpSizeX;
	uint32_t x=__x, y=__y;

	if((_width==height)&&(_width>0)&&(height>0))
	{
		GOTO_ToCalculateRadius:
	   _width=(width&0x0000FFFF)-matchWidth;

	   float R=((float)_width)/2;
		uint32_t pxl_width = R/3;

		x=__x+_width/2 + matchWidth/2;
		if(_width%2) x++;
		y=__y;

		float _x=(float)x, _y=(float)y;
		float x0=(float)x, y0=(float)y+R;

		float param_y = pow(y0-_y,2);
		float param_x = pow(_x-x0,2);
		float decision = pow(R,2);

		int pxl_line=0,i=0;
		uint8_t block=1;

		void _CorrectDecision(void){
			if(block){	 int ni= CONDITION(R<=Circle.correctForWidth,0,1);
				if(i >= VALPERC(pxl_width,Circle.correctPercDeg[ni])){	block=0;   decision= pow(R+Circle.errorDecision[ni],2);  }
		}}

		buf[0]=pxl_width;
		do{
			_x++;  pxl_line++;
			param_x = pow(_x-x0,2);		_CorrectDecision();
			if((param_x+param_y) > decision){
				_y++;
				param_y = pow(y0-_y,2);
				buf[1+i++]=pxl_line-1;
				pxl_line=1;
			}
		}while(i<pxl_width);

		LCD_CircleCorrect();

  	 uint16_t _height=buf[0];
  	 for(int i=0;i<buf[0];++i) _height+=buf[buf[0]-i];
  	 _height = 2*_height;

		if(height>=_height)
			y= y+(height-_height)/2;
		else{
			matchWidth+=1;
			goto GOTO_ToCalculateRadius;
		}

		x-=(x-__x-_height/2);
		y-=(y-__y);

		Circle.width=_height;

	}
	else if((_width==0)&&(height==0));
	else return;

	uint8_t buf_Inv[buf[0]+1];
	InverseAndCopyBuff(buf_Inv,buf);

	_StartDrawLine(0,bkX,x,y);

	switch(width>>16)
	{
	case Half_Circle_0:
		_DrawArrayBuffRightDown_AA(PARAM_AA,0,buf); 		 _CopyDrawPos(); _IncDrawPos(-bkX); PixelCorrect(PARAM, 1,      1);    _SetCopyDrawPos();
		_DrawArrayBuffRightDown_AA(PARAM_AA,1,buf_Inv);                      				   PixelCorrect(PARAM, buf[1],bkX);   _IncDrawPos(-1);

		_OffsetLeftDown(BkpSizeX,1,buf);
		_OffsetLeftDown(BkpSizeX,0,buf_Inv);  _IncDrawPos(-bkX);

		_OffsetLeftUp(BkpSizeX,0,buf);
		_OffsetLeftUp(BkpSizeX,1,buf_Inv);	_IncDrawPos(1);

		_DrawArrayBuffRightUp_AA (PARAM_AA,1,buf);       _CopyDrawPos(); _IncDrawPos(-1);  	PixelCorrect(PARAM, 1,      1);    _SetCopyDrawPos();
		_DrawArrayBuffRightUp_AA (PARAM_AA,0,buf_Inv);                       					PixelCorrect(PARAM, buf[1], 1);
		break;

	case Half_Circle_90:
		_DrawArrayBuffRightDown_AA(PARAM_AA,0,buf); 		 _CopyDrawPos(); _IncDrawPos(-bkX); PixelCorrect(PARAM, 1,      1);    _SetCopyDrawPos();
		_DrawArrayBuffRightDown_AA(PARAM_AA,1,buf_Inv);                      				   PixelCorrect(PARAM, buf[1],bkX);   _IncDrawPos(-1);

		_DrawArrayBuffLeftDown_AA (PARAM_AA,1,buf); 		 _CopyDrawPos(); _IncDrawPos(1);    PixelCorrect(PARAM, 1,      1);    _SetCopyDrawPos();
		_DrawArrayBuffLeftDown_AA (PARAM_AA,0,buf_Inv);                      					PixelCorrect(PARAM, buf[1],-1);    _IncDrawPos(-bkX);
		break;

	case Half_Circle_180:
		_OffsetRightDown(BkpSizeX,0,buf);
		_OffsetRightDown(BkpSizeX,1,buf_Inv);    _IncDrawPos(-1);

		_DrawArrayBuffLeftDown_AA (PARAM_AA,1,buf); 		 _CopyDrawPos(); _IncDrawPos(1);    PixelCorrect(PARAM, 1,      1);    _SetCopyDrawPos();
		_DrawArrayBuffLeftDown_AA (PARAM_AA,0,buf_Inv);                      					PixelCorrect(PARAM, buf[1],-1);    _IncDrawPos(-bkX);

		_DrawArrayBuffLeftUp_AA  (PARAM_AA,0,buf);       _CopyDrawPos(); _IncDrawPos(bkX);  PixelCorrect(PARAM, 1,     -bkX);  _SetCopyDrawPos();
		_DrawArrayBuffLeftUp_AA  (PARAM_AA,1,buf_Inv);  						    					PixelCorrect(PARAM, buf[1],-bkX);  _IncDrawPos(1);
		break;

	case Half_Circle_270:
		_OffsetRightDown(BkpSizeX,0,buf);
		_OffsetRightDown(BkpSizeX,1,buf_Inv);    _IncDrawPos(-1);

		_OffsetLeftDown(BkpSizeX,1,buf);
		_OffsetLeftDown(BkpSizeX,0,buf_Inv);    _IncDrawPos(-bkX);

		_DrawArrayBuffLeftUp_AA  (PARAM_AA,0,buf);       _CopyDrawPos(); _IncDrawPos(bkX);  PixelCorrect(PARAM, 1,     -bkX);  _SetCopyDrawPos();
		_DrawArrayBuffLeftUp_AA  (PARAM_AA,1,buf_Inv);  						    					PixelCorrect(PARAM, buf[1],-bkX);  _IncDrawPos(1);

		_DrawArrayBuffRightUp_AA (PARAM_AA,1,buf);       _CopyDrawPos(); _IncDrawPos(-1);  	PixelCorrect(PARAM, 1,      1);    _SetCopyDrawPos();
		_DrawArrayBuffRightUp_AA (PARAM_AA,0,buf_Inv);                       					PixelCorrect(PARAM, buf[1], 1);
		break;
	}

	if(FillColor!=TRANSPARENT)
	{
   	 uint16_t fillWidth, fillHeight=0;

   		switch(width>>16)
   		{
   		case Half_Circle_0:
   			_StartDrawLine(0,bkX,x,y);
   			fillWidth=0;
   			for(int i=0;i<buf[0];++i){
   				_IncDrawPos(bkX-buf[i+1]);	_CopyDrawPos(); k+=buf[i+1]; 	if(i>0) _DrawRight(2*fillWidth,FillColor);	_SetCopyDrawPos(); fillHeight++;
   				fillWidth += buf[i+1];
   			}

   			for(int i=0;i<buf[0];++i){
   				fillWidth++;
   				_IncDrawPos(-1);
   				for(int j=0; j<buf[buf[0]-i]; ++j){
   					_IncDrawPos(bkX);	_CopyDrawPos(); if(j==buf[buf[0]-i]-1){if(i<buf[0]-2){k+=1;_DrawRight(2*(fillWidth-1),FillColor);}}else{k+=2;_DrawRight(2*(fillWidth-2),FillColor);}	_SetCopyDrawPos();	fillHeight++;
   				}
   			}
   			break;

   		case Half_Circle_90:
   			_StartDrawLine(0,bkX,x,y);
   			fillWidth=0;
   			for(int i=0;i<buf[0];++i){
   				_IncDrawPos(bkX-buf[i+1]);	_CopyDrawPos(); k+=buf[i+1]+fillWidth; 	if(i>0) _DrawRight(1*fillWidth,FillColor);	_SetCopyDrawPos(); fillHeight++;
   				fillWidth += buf[i+1];
   			}

   			for(int i=0;i<buf[0];++i){
   				fillWidth++;
   				_IncDrawPos(-1);
   				for(int j=0; j<buf[buf[0]-i]; ++j){
   					_IncDrawPos(bkX);	_CopyDrawPos(); if(j==buf[buf[0]-i]-1){k+=1+fillWidth-1;_DrawRight(1*(fillWidth-1),FillColor);}else{k+=fillWidth;_DrawRight(1*(fillWidth-2),FillColor);}	_SetCopyDrawPos();	fillHeight++;
   				}
   			}

   			fillHeight = 2*fillHeight-1;

   			_StartDrawLine(0,bkX,x,y+fillHeight);
   			fillWidth=0;
   			for(int i=0;i<buf[0];++i){
   				_IncDrawPos(-bkX-buf[i+1]);_CopyDrawPos(); k+=buf[i+1]+fillWidth; 	if(i>0) _DrawRight(1*fillWidth,FillColor);  _SetCopyDrawPos();
   				fillWidth += buf[i+1];
   			}

   			for(int i=0;i<buf[0];++i){
   				fillWidth++;
   				_IncDrawPos(-1);
   				for(int j=0; j<buf[buf[0]-i]; ++j){
   					_IncDrawPos(-bkX);	_CopyDrawPos(); if(j==buf[buf[0]-i]-1){k+=1+fillWidth-1;_DrawRight(1*(fillWidth-1),FillColor);}else{k+=fillWidth;_DrawRight(1*(fillWidth-2),FillColor);}	_SetCopyDrawPos();	fillHeight++;
   				}
   			}
   			break;

   			case Half_Circle_180:
   			_StartDrawLine(0,bkX,x,y);
   			fillWidth=0;
   			for(int i=0;i<buf[0];++i){
   				_IncDrawPos(bkX-buf[i+1]);	 fillHeight++;
   				fillWidth += buf[i+1];
   			}

   			for(int i=0;i<buf[0];++i){
   				fillWidth++;
   				_IncDrawPos(-1);
   				for(int j=0; j<buf[buf[0]-i]; ++j){
   					_IncDrawPos(bkX);	 fillHeight++;
   				}
   			}

   			fillHeight = 2*fillHeight-1;

   			_StartDrawLine(0,bkX,x,y+fillHeight);
   			fillWidth=0;
   			for(int i=0;i<buf[0];++i){
   				_IncDrawPos(-bkX-buf[i+1]);	_CopyDrawPos(); k+=buf[i+1]; 	if(i>0) _DrawRight(2*fillWidth,FillColor);	_SetCopyDrawPos();
   				fillWidth += buf[i+1];
   			}

   			for(int i=0;i<buf[0];++i){
   				fillWidth++;
   				_IncDrawPos(-1);
   				for(int j=0; j<buf[buf[0]-i]; ++j){
   					_IncDrawPos(-bkX);	_CopyDrawPos(); if(j==buf[buf[0]-i]-1){if(i<buf[0]-2){k+=1;_DrawRight(2*(fillWidth-1),FillColor);}}else{k+=2;_DrawRight(2*(fillWidth-2),FillColor);}	_SetCopyDrawPos();
   				}
   			}
   			break;

   		case Half_Circle_270:
   			_StartDrawLine(0,bkX,x,y);
   			fillWidth=0;
   			for(int i=0;i<buf[0];++i){
   				_IncDrawPos(bkX-buf[i+1]);	_CopyDrawPos(); k+=buf[i+1]; 	if(i>0) _DrawRight(1*fillWidth,FillColor);	_SetCopyDrawPos(); fillHeight++;
   				fillWidth += buf[i+1];
   			}

   			for(int i=0;i<buf[0];++i){
   				fillWidth++;
   				_IncDrawPos(-1);
   				for(int j=0; j<buf[buf[0]-i]; ++j){
   					_IncDrawPos(bkX);	_CopyDrawPos(); if(j==buf[buf[0]-i]-1){k+=1;_DrawRight(1*(fillWidth-1),FillColor);}else{k+=2;_DrawRight(1*(fillWidth-2),FillColor);}	_SetCopyDrawPos();	fillHeight++;
   				}
   			}

   			fillHeight = 2*fillHeight-1;

   			_StartDrawLine(0,bkX,x,y+fillHeight);
   			fillWidth=0;
   			for(int i=0;i<buf[0];++i){
   				_IncDrawPos(-bkX-buf[i+1]);_CopyDrawPos(); k+=buf[i+1]; 	if(i>0) _DrawRight(1*fillWidth,FillColor);  _SetCopyDrawPos();
   				fillWidth += buf[i+1];
   			}

   			for(int i=0;i<buf[0];++i){
   				fillWidth++;
   				_IncDrawPos(-1);
   				for(int j=0; j<buf[buf[0]-i]; ++j){
   					_IncDrawPos(-bkX);	_CopyDrawPos(); if(j==buf[buf[0]-i]-1){k+=1;_DrawRight(1*(fillWidth-1),FillColor);}else{k+=2;_DrawRight(1*(fillWidth-2),FillColor);}	_SetCopyDrawPos();	fillHeight++;
   				}
   			}
   			break;
   		}
	}

	#undef PARAM
	#undef PARAM_AA
	#undef buf
}

//################################## -- Global Declarations -- #########################################################
void CorrectLineAA_on(void){
	correctLine_AA=1;
}
void CorrectLineAA_off(void){
	correctLine_AA=0;
}

void Set_AACoeff_RoundFrameRectangle(float coeff_1, float coeff_2){
	AA.c1 = coeff_1;
	AA.c2 = coeff_2;
}

void Set_AACoeff_Draw(int pixelsInOneSide, uint32_t colorFrom, uint32_t colorTo, float ratioStart)
{
	//int pixelsInOneSide= pixelsInOneSide_==1?3:pixelsInOneSide_;
	//int pixelsInOneSide;
//
//	if(pixelsInOneSide_==1) pixelsInOneSide=2;
//	else                    pixelsInOneSide=pixelsInOneSide_;

	float incr= (1-ratioStart)/pixelsInOneSide;
	buff_AA[0]=pixelsInOneSide;
	for(int i=0;i<pixelsInOneSide;++i){
		if(1+i>MAX_SIZE_TAB_AA)
			break;
		buff_AA[1+i]= GetTransitionColor(colorFrom,colorTo, ratioStart+i*incr);
	}
}

void LCD_LineH(uint32_t BkpSizeX, uint16_t x, uint16_t y, uint16_t width,  uint32_t color, uint16_t bold){
	_StartDrawLine(0,BkpSizeX, x, y);	_DrawRight(width, color);
	for(int i=0; i<bold; ++i){
		_NextDrawLine(BkpSizeX,width);	_DrawRight(width, color);
	}
}
void LCD_LineV(uint32_t BkpSizeX, uint16_t x, uint16_t y, uint16_t width,  uint32_t color, uint16_t bold){
	_StartDrawLine(0,BkpSizeX, x, y);	_CopyDrawPos();	_DrawDown(width, color, BkpSizeX);
	for(int i=0; i<bold; ++i){
		_SetCopyDrawPos();	_IncDrawPos(1); _CopyDrawPos();  _DrawDown(width, color, BkpSizeX);
	}
}
void LCD_Display(uint32_t posBuff, uint32_t Xpos, uint32_t Ypos, uint32_t width, uint32_t height){
	LCD_DisplayBuff(Xpos,Ypos,width,height,  pLcd+posBuff);
}
void LCD_DisplayPart(uint32_t posBuff, uint32_t Xpos, uint32_t Ypos, uint32_t width, uint32_t height){
/* For another ptr2pLcd: pLcd_*/
/*	int m=0;
	k = posBuff + LCD_X*Ypos + Xpos;
	for(int j=0; j<height; j++){
		for(int i=0; i<width; i++)
			pLcd_[m++] = pLcd[k+i];
		k += LCD_X;
	}
	LCD_DisplayBuff(Xpos,Ypos,width,height, pLcd_);
*/
	uint32_t tab[width];
	k = posBuff + LCD_X*Ypos + Xpos;
	for(int j=0; j<height; j++){
		for(int i=0; i<width; i++)
			tab[i] = pLcd[k+i];
		k += LCD_X;
		LCD_DisplayBuff(Xpos,Ypos+j,width,1, tab);
	}
}
void LCD_Show(void){
	LCD_Display(0,0,0,LCD_X,LCD_Y);
}

void LCD_Shape(uint32_t x,uint32_t y,figureShape pShape,uint32_t width,uint32_t height,uint32_t FrameColor,uint32_t FillColor,uint32_t BkpColor){
	LCD_ShapeWindow(pShape,0,LCD_X,LCD_Y,x,y,width,height,FrameColor,FillColor,BkpColor);
}
void LCD_ShapeWindow(figureShape pShape,uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	pShape(posBuff,BkpSizeX,BkpSizeY,x,y,width,height,FrameColor,FillColor,BkpColor);
}
void LCD_ShapeIndirect(uint32_t xPos,uint32_t yPos,figureShape pShape, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	pShape(0,width,height,0,0,width,height,FrameColor,FillColor,BkpColor);
	LCD_Display(0,xPos,yPos,width,height);
}
void LCD_ShapeWindowIndirect(uint32_t xPos,uint32_t yPos,figureShape pShape,uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	pShape(posBuff,BkpSizeX,BkpSizeY,x,y,width,height,FrameColor,FillColor,BkpColor);
	LCD_Display(posBuff,xPos,yPos,BkpSizeX,BkpSizeY);
}

void LCD_Clear(uint32_t color){
	LCD_ShapeWindow(LCD_Rectangle,0,LCD_X,LCD_Y, 0,0, LCD_X, LCD_Y, color,color,color);
}
void LCD_ClearPartScreen(uint32_t posBuff, uint32_t BkpSizeX, uint32_t BkpSizeY, uint32_t color){
	LCD_ShapeWindow(LCD_Rectangle,posBuff,BkpSizeX,BkpSizeY, 0,0, BkpSizeX, BkpSizeY, color,color,color);
}
uint32_t SetBold2Color(uint32_t frameColor, uint8_t thickness){
	return (frameColor&0xFFFFFF)|thickness<<24;
}

void LCD_LittleRoundRectangle(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	_StartDrawLine(posBuff,BkpSizeX,x,y);
	_FillBuff(2,BkpColor);	 _FillBuff(width-4,FrameColor);  _FillBuff(2, BkpColor);
	_NextDrawLine(BkpSizeX,width);
	_FillBuff(1,BkpColor);_FillBuff(1,FrameColor);	 _FillBuff(width-4, FillColor);  _FillBuff(1,FrameColor);_FillBuff(1, BkpColor);
	if(height>1)
	{
		_NextDrawLine(BkpSizeX,width);
		for (int j=0; j<height-4; j++)
		{
			if(width>1)
			{
				_FillBuff(1, FrameColor);
				_FillBuff(width-2, FillColor);
				_FillBuff(1, FrameColor);
				_NextDrawLine(BkpSizeX,width);
			}
			else
			{
				_FillBuff(width, FillColor);
				_NextDrawLine(BkpSizeX,width);
			}
		}
		_FillBuff(1,BkpColor);_FillBuff(1,FrameColor);	 _FillBuff(width-4, FillColor);  _FillBuff(1,FrameColor);_FillBuff(1, BkpColor);
		_NextDrawLine(BkpSizeX,width);
		_FillBuff(2, BkpColor);	 _FillBuff(width-4, FrameColor);  _FillBuff(2, BkpColor);
	}
}
void LCD_LittleRoundFrame(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	_StartDrawLine(posBuff,BkpSizeX,x,y);
	_FillBuff(2,BkpColor);	 _FillBuff(width-4,FrameColor);  _FillBuff(2, BkpColor);
	_NextDrawLine(BkpSizeX,width);
	_FillBuff(1,BkpColor);_FillBuff(1,FrameColor);	 k+=width-4; /* _FillBuff(width-4, FillColor); */  _FillBuff(1,FrameColor);_FillBuff(1, BkpColor);
	if(height>1)
	{
		_NextDrawLine(BkpSizeX,width);
		for (int j=0; j<height-4; j++)
		{
			if(width>1)
			{
				_FillBuff(1, FrameColor);
				k+=width-2;			/* _FillBuff(width-2, FillColor); */
				_FillBuff(1, FrameColor);
				_NextDrawLine(BkpSizeX,width);
			}
			else
			{
				k+=width;			/* _FillBuff(width, FillColor); */
				_NextDrawLine(BkpSizeX,width);
			}
		}
		_FillBuff(1,BkpColor);_FillBuff(1,FrameColor);	k+=width-4; /* _FillBuff(width-4, FillColor); */  _FillBuff(1,FrameColor);_FillBuff(1, BkpColor);
		_NextDrawLine(BkpSizeX,width);
		_FillBuff(2, BkpColor);	 _FillBuff(width-4, FrameColor);  _FillBuff(2, BkpColor);
	}
}
void LCD_Rectangle(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	_StartDrawLine(posBuff,BkpSizeX,x,y);
	_FillBuff(width, FrameColor);
	if(height>1)
	{
		_NextDrawLine(BkpSizeX,width);
		for (int j=0; j<height-2; j++)
		{
			_FillBuff(1, FrameColor);
			_FillBuff(width-2, FillColor);
			_FillBuff(1, FrameColor);
			_NextDrawLine(BkpSizeX,width);
		}
		_FillBuff(width, FrameColor);
	}
}
void LCD_Frame(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	_StartDrawLine(posBuff,BkpSizeX,x,y);
	_FillBuff(width, FrameColor);
	if(height>1)
	{
		_NextDrawLine(BkpSizeX,width);
		for (int j=0; j<height-2; j++)
		{
			_FillBuff(1, FrameColor);
			k+=width-2;
			_FillBuff(1, FrameColor);
			_NextDrawLine(BkpSizeX,width);
		}
		_FillBuff(width, FrameColor);
	}
}

void LCD_BoldRectangle(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	uint8_t thickness = FrameColor>>24;
	if((thickness==0)||(thickness==1)||(thickness==2)||(thickness==255))
		thickness=2;
	int thickness2 = 2*thickness;
	int fillHeight = height-thickness2;
	int fillWidth = width-thickness2;

	_StartDrawLine(posBuff,BkpSizeX,x,y);
	_FillBuff(width, FrameColor);
	for(int i=0;i<thickness-1;++i){
		_NextDrawLine(BkpSizeX,width);
		_FillBuff(width, FrameColor);
	}
	_NextDrawLine(BkpSizeX,width);
	if(fillHeight>0)
	{
		for (int j=0; j<fillHeight; j++)
		{	_FillBuff(thickness, FrameColor);
			_FillBuff(fillWidth, FillColor);
			_FillBuff(thickness, FrameColor);
			_NextDrawLine(BkpSizeX,width);
		}
	}
	_FillBuff(width, FrameColor);
	for(int i=0;i<thickness-1;++i){
		_NextDrawLine(BkpSizeX,width);
		_FillBuff(width, FrameColor);
	}
}
void LCD_BoldFrame(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	uint8_t thickness = FrameColor>>24;
	if((thickness==0)||(thickness==1)||(thickness==2)||(thickness==255))
		thickness=2;
	int thickness2 = 2*thickness;
	int fillHeight = height-thickness2;
	int fillWidth = width-thickness2;

	_StartDrawLine(posBuff,BkpSizeX,x,y);
	_FillBuff(width, FrameColor);
	for(int i=0;i<thickness-1;++i){
		_NextDrawLine(BkpSizeX,width);
		_FillBuff(width, FrameColor);
	}
	_NextDrawLine(BkpSizeX,width);
	if(fillHeight>0)
	{
		for (int j=0; j<fillHeight; j++)
		{	_FillBuff(thickness, FrameColor);
			k+=fillWidth;
			_FillBuff(thickness, FrameColor);
			_NextDrawLine(BkpSizeX,width);
		}
	}
	_FillBuff(width, FrameColor);
	for(int i=0;i<thickness-1;++i){
		_NextDrawLine(BkpSizeX,width);
		_FillBuff(width, FrameColor);
	}
}

void LCD_RoundFrame(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	LCD_DrawRoundRectangleFrame(0,posBuff,BkpSizeX,BkpSizeY,x,y,width,height,FrameColor,FillColor,BkpColor);
}
void LCD_RoundRectangle(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	LCD_DrawRoundRectangleFrame(1,posBuff,BkpSizeX,BkpSizeY,x,y,width,height,FrameColor,FillColor,BkpColor);
}

void LCD_BoldRoundRectangle(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	uint8_t thickness = FrameColor>>24;
	if((thickness==0)||(thickness==1)||(thickness==2)||(thickness==255))
		thickness=1;
	else
		thickness--;
	LCD_DrawRoundRectangleFrame(1,posBuff,BkpSizeX,BkpSizeY,x,y,width,height,FrameColor,FrameColor,BkpColor);
	LCD_DrawRoundRectangleFrame(1,posBuff,BkpSizeX,BkpSizeY,x+thickness,y+thickness,width-2*thickness,height-2*thickness,FrameColor,FillColor,1<<24);
}

void LCD_BoldRoundFrame(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	int i,k1,k2;
	uint8_t thickness = FrameColor>>24;
	if((thickness==0)||(thickness==1)||(thickness==2)||(thickness==255))
		thickness=2;
	LCD_DrawRoundRectangleFrame(0,posBuff,BkpSizeX,BkpSizeY,x,y,width,height,FrameColor,FrameColor,BkpColor);
	thickness-=2;
	for(i=0;i<thickness;++i){
		k1=1+i;
		k2=2*k1;
		LCD_DrawRoundRectangleFrame(0,posBuff,BkpSizeX,BkpSizeY,x+k1,y+k1,width-k2,height-k2,FrameColor,FrameColor,1<<24);
	}
	k1=1+i;
	k2=2*k1;
	LCD_DrawRoundRectangleFrame(0,posBuff,BkpSizeX,BkpSizeY,x+k1,y+k1,width-k2,height-k2,FrameColor,FillColor,1<<24);
}

structPosition DrawLine(uint32_t posBuff,uint16_t x0, uint16_t y0, uint16_t len, uint16_t degree, uint32_t lineColor,uint32_t BkpSizeX, float ratioAA1, float ratioAA2 ,uint32_t bk1Color, uint32_t bk2Color)
{
	#define LINES_BUFF_SIZE		len+6

	uint16_t degree_copy=degree;
	uint8_t linesBuff[LINES_BUFF_SIZE];
	int k_iteration=0, searchDirection=0;
	int nrPointsPerLine=0, iteration=1;
	int findNoPoint=0, rot;
	float param_y;
	float param_x;
	float decision;

	void _FFFFFFFFFF(void)
	{
		switch(rot)
		{  case 1: switch(searchDirection){ case 0: k_iteration=-1;	 		 break; case 1: k_iteration=-BkpSizeX-1; break; } break;
			case 2: switch(searchDirection){ case 0: k_iteration=-BkpSizeX; break; case 1: k_iteration=-BkpSizeX-1; break; } break;
			case 3: switch(searchDirection){ case 0: k_iteration=-BkpSizeX; break; case 1: k_iteration=-BkpSizeX+1; break; } break;
			case 4: switch(searchDirection){ case 0: k_iteration=1;     	 break; case 1: k_iteration=-BkpSizeX+1; break; } break;
			case 5: switch(searchDirection){ case 0: k_iteration=1;     	 break; case 1: k_iteration= BkpSizeX+1; break; } break;
			case 6: switch(searchDirection){ case 0: k_iteration=BkpSizeX;  break; case 1: k_iteration= BkpSizeX+1; break; } break;
			case 7: switch(searchDirection){ case 0: k_iteration=BkpSizeX;  break; case 1: k_iteration= BkpSizeX-1; break; } break;
			case 8: switch(searchDirection){ case 0: k_iteration=-1;   		 break; case 1: k_iteration= BkpSizeX-1; break; } break;
		}
	}

	if(degree_copy==0)
		degree_copy=360;

	_StartDrawLine(posBuff,BkpSizeX,x0,y0);
	rot=LCD_SearchLinePoints(1,posBuff,x0,y0,degree_copy,BkpSizeX);
	_FFFFFFFFFF();


	do
	{
		if(LCD_SearchLinePoints(0,posBuff,x0,y0,degree_copy,BkpSizeX)==1)
		{
			nrPointsPerLine++;
		   if(findNoPoint)
		   {
		   	findNoPoint=0;
		   	searchDirection=1-searchDirection;
		   	_FFFFFFFFFF();
		   }
		}
		else
		{
		   k-=k_iteration;
		   searchDirection=1-searchDirection;
		   if(nrPointsPerLine)
		   {
		   	if(iteration<LINES_BUFF_SIZE-2)
		   		linesBuff[iteration++]=nrPointsPerLine;
		   	else break;
		   }
		   nrPointsPerLine=0;
			findNoPoint=1;
			_FFFFFFFFFF();
		}
		k+=k_iteration;
	   pos = _GetPosXY(posBuff,BkpSizeX);

		param_y = pow(y0-pos.y,2);
		param_x = pow(x0-pos.x,2);
		decision = pow((float)(len+1),2);

	}while((param_x+param_y) <= decision);

	if(nrPointsPerLine)
		linesBuff[iteration++]=nrPointsPerLine;
	linesBuff[0]=iteration-1;

	_StartDrawLine(posBuff,BkpSizeX,x0,y0);

	if(correctLine_AA==0)
	{
		switch(rot)
		{
			case 1:  _DrawArrayBuffLeftUp_AA   (lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,0,linesBuff); break;
			case 2:  _DrawArrayBuffLeftUp_AA   (lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,1,linesBuff); break;
			case 3:  _DrawArrayBuffRightUp_AA  (lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,1,linesBuff); break;
			case 4:  _DrawArrayBuffRightUp_AA  (lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,0,linesBuff); break;
			case 5:  _DrawArrayBuffRightDown_AA(lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,0,linesBuff); break;
			case 6:  _DrawArrayBuffRightDown_AA(lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,1,linesBuff); break;
			case 7:  _DrawArrayBuffLeftDown_AA (lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,1,linesBuff); break;
			case 8:  _DrawArrayBuffLeftDown_AA (lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,0,linesBuff); break;
		}
	}
	else
	{
		int  k_p=k;
		int linePointsStructSize;
		int nmbPxl,repeatPxl,repeatPxl_next;
		int direction, upDown;

		int _GetNmbPxls(void)
		{
			int j,it=0,count1=0;
			char linesBuff_prev=0;

			linesBuff_prev=linesBuff[1];

			for(j=0;j<linesBuff[0];++j){
				if(linesBuff[1+j]==linesBuff_prev) count1++;
				else{
					it++;
					linesBuff_prev=linesBuff[1+j];
					count1=1;
				}
			}
			if(count1){
				it++;
			}
			return it;
		}

		typedef struct
		{	uint16_t nmbPxl;
			uint8_t nmbPxlTheSame;
		}Struct_LinePoints;
		Struct_LinePoints linePoints[_GetNmbPxls()];

		int Pxl_V(uint8_t nmbPxl_V, uint8_t repeat, uint8_t stepAA, int offs, int direction, int upDown)
		{
			uint32_t bk1Color_, bk2Color_;
			if((degree<=135 && degree>=135-11)||
				(degree<315  && degree>=315-11)){	bk1Color_=bk2Color; bk2Color_=bk1Color;	}
			else{												bk1Color_=bk1Color; bk2Color_=bk2Color;	}

			Set_AACoeff(stepAA,lineColor,bk1Color_,0.0);
			Set_AACoeff2(stepAA,lineColor,bk2Color_,0.0);

			if(repeat>1)
			{
				for(int i=0;i<repeat;++i){
					pLcd[k_p-			upDown*BkpSizeX]=	buff_AA[offs+1+i];
					pLcd[k_p+upDown*nmbPxl_V*BkpSizeX]=	buff2_AA[offs+repeat-i];
					k_p=k_p+upDown*nmbPxl_V*BkpSizeX+direction;
				}
			}
			else{
				pLcd[k_p-			upDown*BkpSizeX]=	buff_AA[offs];
				pLcd[k_p+upDown*nmbPxl_V*BkpSizeX]=	buff2_AA[offs];
				k_p=k_p+upDown*nmbPxl_V*BkpSizeX+direction;
			}
			return 0;
		}

		int Pxl_H(uint8_t nmbPxl_H, uint8_t repeat, uint8_t stepAA, int offs, int direction, int upDown)
		{
			uint32_t bk1Color_, bk2Color_;
			if((degree>=45-11  && degree<=45) ||
			   (degree>=225-11 && degree<=225)){	bk1Color_=bk2Color; bk2Color_=bk1Color;	}
			else{												bk1Color_=bk1Color; bk2Color_=bk2Color;	}

			Set_AACoeff(stepAA,lineColor,bk1Color_,0.0);
			Set_AACoeff2(stepAA,lineColor,bk2Color_,0.0);

			if(repeat>1)
			{
				for(int i=0;i<repeat;++i){
					pLcd[k_p-direction]			 =	buff_AA[offs+1+i];
					pLcd[k_p+direction*nmbPxl_H]=	buff2_AA[offs+repeat-i];
					k_p=k_p+upDown*BkpSizeX+direction*nmbPxl_H;
				}
			}
			else{
				pLcd[k_p-direction]			 =	buff_AA[offs];
				pLcd[k_p+direction*nmbPxl_H]=	buff2_AA[offs];
				k_p=k_p+upDown*BkpSizeX+direction*nmbPxl_H;
			}
			return 0;
		}

		void _GetNmbPxlsAndLoadToBuff(void)
		{
			int j,it=0,count1=0;
			char linesBuff_prev=0;

			linesBuff_prev=linesBuff[1];

			for(j=1;j<linesBuff[0]+1;++j){
				if(linesBuff[j]==linesBuff_prev) count1++;
				else{
					linePoints[it].nmbPxl=linesBuff_prev;
					linePoints[it++].nmbPxlTheSame=count1;
					linesBuff_prev=linesBuff[j];
					count1=1;
				}
			}
			if(count1){
				linePoints[it].nmbPxl=linesBuff_prev;
				linePoints[it++].nmbPxlTheSame=count1;
			}
			linePointsStructSize=it;
		}

		switch(rot)
		{
			case 1:
				if(degree>=45-11 && degree<=45)
					_DrawArrayBuffLeftUp_AA   (lineColor, bk1Color, bk2Color, 1.0, 1.0, BkpSizeX,0,linesBuff);
				else
					_DrawArrayBuffLeftUp_AA   (lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,0,linesBuff);
				break;
			case 2:
				if(degree>45 && degree<=45+11)
					_DrawArrayBuffLeftUp_AA   (lineColor, bk1Color, bk2Color, 1.0, 1.0, BkpSizeX,1,linesBuff);
				else
					_DrawArrayBuffLeftUp_AA   (lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,1,linesBuff);
			break;

			case 3:
				if(degree<=135 && degree>=135-11)
					_DrawArrayBuffRightUp_AA  (lineColor, bk1Color, bk2Color, 1.0, 1.0, BkpSizeX,1,linesBuff);
				else
					_DrawArrayBuffRightUp_AA  (lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,1,linesBuff);
				break;
			case 4:
				if(degree>135 && degree<=135+11)
					_DrawArrayBuffRightUp_AA  (lineColor, bk1Color, bk2Color, 1.0, 1.0, BkpSizeX,0,linesBuff);
				else
					_DrawArrayBuffRightUp_AA  (lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,0,linesBuff);
			break;

			case 5:
				if(degree>=225-11 && degree<=225)
					_DrawArrayBuffRightDown_AA(lineColor, bk1Color, bk2Color, 1.0, 1.0, BkpSizeX,0,linesBuff);
				else
					_DrawArrayBuffRightDown_AA(lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,0,linesBuff);
				break;
			case 6:
				if(degree>225 && degree<=225+11)
					_DrawArrayBuffRightDown_AA(lineColor, bk1Color, bk2Color, 1.0, 1.0, BkpSizeX,1,linesBuff);
				else
					_DrawArrayBuffRightDown_AA(lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,1,linesBuff);
				break;

			case 7:
				if(degree<315 && degree>=315-11)
					_DrawArrayBuffLeftDown_AA (lineColor, bk1Color, bk2Color, 1.0, 1.0, BkpSizeX,1,linesBuff);
				else
					_DrawArrayBuffLeftDown_AA (lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,1,linesBuff);
				break;
			case 8:
				if(degree>=315 && degree<=315+11)
					_DrawArrayBuffLeftDown_AA (lineColor, bk1Color, bk2Color, 1.0, 1.0, BkpSizeX,0,linesBuff);
				else
					_DrawArrayBuffLeftDown_AA (lineColor, bk1Color, bk2Color, ratioAA1, ratioAA2, BkpSizeX,0,linesBuff);
				break;
		}


		if((degree>=225-11 && degree<=225)||
		   (degree>=315    && degree<=315+11)||
		   (degree>135     && degree<=135+11)||
		   (degree>=45-11  && degree<=45))
		{
			_GetNmbPxlsAndLoadToBuff();

			if((degree>=225-11 && degree<=225)||
				(degree>135     && degree<=135+11)) direction=1;
			else                                   direction=-1;

			if((degree>=225-11 && degree<=225)||
				(degree>=315    && degree<=315+11)) upDown=1;
			else                                   upDown=-1;

			for(int j=0; j<linePointsStructSize; ++j)
			{
				nmbPxl    = linePoints[j].nmbPxl;
				repeatPxl = linePoints[j].nmbPxlTheSame;

				if(repeatPxl>1)
					Pxl_H(nmbPxl,repeatPxl,repeatPxl+2,1,direction,upDown);
				else{
					if(nmbPxl==2)
						Pxl_H(nmbPxl,repeatPxl,4,4,direction,upDown);
					else
						Pxl_H(nmbPxl,repeatPxl,3,2,direction,upDown);
				}
			}
		}
		else if((degree>225 && degree<=225+11)||
			    (degree<315  && degree>=315-11)||
				 (degree<=135 && degree>=135-11)||
				 (degree>45   && degree<=45+11))
		{
			_GetNmbPxlsAndLoadToBuff();

			if((degree>225  && degree<=225+11)||
				(degree<=135 && degree>=135-11)) direction=1;
			else                               direction=-1;

			if((degree>225 && degree<=225+11)||
				(degree<315 && degree>=315-11)) upDown=1;
			else                               upDown=-1;

			if((degree>=225+6 && degree<=225+11)||
				(degree<=315-6 && degree>=315-11)||
				(degree<=135-6 && degree>=135-11)||
				(degree>=45+6  && degree<=45+11))
			{
				for(int j=0; j<linePointsStructSize; ++j)
				{
					nmbPxl    = linePoints[j].nmbPxl;
					repeatPxl = linePoints[j].nmbPxlTheSame;

					if(repeatPxl>1)
					{
						if(repeatPxl==2)
							Pxl_V(nmbPxl,repeatPxl,4,1,direction,upDown);
						else if(repeatPxl>=3)
							Pxl_V(nmbPxl,repeatPxl,repeatPxl+1,0,direction,upDown);
					}
					else
					{
						if(nmbPxl==1)
							Pxl_V(nmbPxl,repeatPxl,3,2,direction,upDown);
						else
							Pxl_V(nmbPxl,repeatPxl,3,3,direction,upDown);
					}
				}
			}
			else
			{
				for(int j=0; j<linePointsStructSize; ++j)
				{
					nmbPxl = linePoints[j].nmbPxl;
					repeatPxl = linePoints[j].nmbPxlTheSame;
					if(j+1<linePointsStructSize)
						repeatPxl_next = linePoints[j+1].nmbPxlTheSame;

					if(repeatPxl>1){
						if(Pxl_V(nmbPxl,repeatPxl,repeatPxl,0,direction,upDown)) break;
					}
					else{
						if(repeatPxl_next<3){
							if(repeatPxl==1 && j==linePointsStructSize-1){
								if(Pxl_V(nmbPxl,repeatPxl,2,2,direction,upDown)) break;
							}
							else{
								if(Pxl_V(nmbPxl,repeatPxl,5,5,direction,upDown)) break;
							}
						}
						else{
							if(Pxl_V(nmbPxl,repeatPxl,repeatPxl_next,repeatPxl_next,direction,upDown)) break;
						}
					}
				}
			}
		}

	}

	#undef LINES_BUFF_SIZE

	return pos;
}

SHAPE_PARAMS LCD_KeyBackspace(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor)
{
	SHAPE_PARAMS params = {.bkSize.x=BkpSizeX, .bkSize.y=BkpSizeY, .pos[0].x=x, .pos[0].y=y, .size[0].w=width, .size[0].h=height, .color[0].frame=FrameColor, .color[0].fill=FillColor, .color[0].bk=BkpColor};

	if(ToStructAndReturn == posBuff)
		return params;

	width = MASK(width,FFFF);
	height = MASK(height,FFFF);

	int widthSign=width/2;
	int heightSign=height/2;
	int sizeSignX=height/4;

	if((heightSign%2)!=0) heightSign++;

	 LCD_Rectangle(posBuff,BkpSizeX,BkpSizeY,x,y,width,height,FrameColor,FillColor,BkpColor);

	_StartDrawLine(posBuff,BkpSizeX,x+(width-widthSign)/2,y+height/2);
	_DrawRightUp(heightSign/2,heightSign/2, FrameColor,BkpSizeX);
	_DrawRight(widthSign-heightSign/2,FrameColor);
	_DrawDown(heightSign,FrameColor,BkpSizeX);
	_DrawLeft(widthSign-heightSign/2,FrameColor);
	_DrawLeftUp(heightSign/2,heightSign/2, FrameColor,BkpSizeX);

	_StartDrawLine(posBuff,BkpSizeX,x+(width-widthSign)/2+1,y+height/2);
	_CopyDrawPos();
	_DrawRightUp(heightSign/2,heightSign/2, FrameColor,BkpSizeX);
	_SetCopyDrawPos();
	_DrawRightDown(heightSign/2,heightSign/2, FrameColor,BkpSizeX);

	_StartDrawLine(posBuff,BkpSizeX,x+(width-sizeSignX)/2+sizeSignX/2,y+(height-sizeSignX)/2);
	_DrawRightDown(heightSign/2,heightSign/2, FrameColor,BkpSizeX);
	_StartDrawLine(posBuff,BkpSizeX,x+(width-sizeSignX)/2+sizeSignX/2-1,y+(height-sizeSignX)/2);
	_DrawRightDown(heightSign/2,heightSign/2, FrameColor,BkpSizeX);

	_StartDrawLine(posBuff,BkpSizeX,x+(width-sizeSignX)/2+sizeSignX/2+sizeSignX,y+(height-sizeSignX)/2);
	_DrawLeftDown(heightSign/2,heightSign/2, FrameColor,BkpSizeX);
	_StartDrawLine(posBuff,BkpSizeX,x+(width-sizeSignX)/2+sizeSignX/2+sizeSignX-1,y+(height-sizeSignX)/2);
	_DrawLeftDown(heightSign/2,heightSign/2, FrameColor,BkpSizeX);
	return params;
}
SHAPE_PARAMS LCDSHAPE_KeyBackspace(uint32_t posBuff, SHAPE_PARAMS param){
	return LCD_KeyBackspace(posBuff,param.bkSize.x,param.bkSize.y, param.pos[0].x,param.pos[0].y, param.size[0].w,param.size[0].h, param.color[0].frame, param.color[0].fill, param.color[0].bk);
}

void LCD_SignStar(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor)
{																												/*	'width/height' or 'height/width'  must be divisible */
	#define PARAM1			WHITE,MYGRAY
	#define PARAM2		FrameColor,BkpSizeX

	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(20,PARAM1,0.39); _DrawRightDown_AA(100,5,PARAM2);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(5,PARAM1, 0.45);_DrawRightDown_AA(100,20,PARAM2);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(2,PARAM1, 0.39);_DrawRightDown_AA(100,50,PARAM2);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(1,PARAM1, 0.39);_DrawRightDown_AA(100,100,PARAM2);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(2,WHITE,MYGRAY, 0.39);_DrawRightDown_AA(50,100,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(5,WHITE,MYGRAY, 0.45); _DrawRightDown_AA(20,100,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(20,WHITE,MYGRAY,0.39);_DrawRightDown_AA(5,100,FrameColor,BkpSizeX);

	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(10,WHITE,MYGRAY,0.39); _DrawRightUp_AA(100,10,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(4,WHITE,MYGRAY, 0.47);_DrawRightUp_AA(100,25,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(2,WHITE,MYGRAY, 0.39); _DrawRightUp_AA(100,50,FrameColor,BkpSizeX);

	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(1,WHITE,MYGRAY, 0.39);_DrawRightUp_AA(100,100,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(2,WHITE,MYGRAY, 0.39);_DrawRightUp_AA(50,100,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(4,WHITE,MYGRAY, 0.47); _DrawRightUp_AA(25,100,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(10,WHITE,MYGRAY,0.39); _DrawRightUp_AA(10,100,FrameColor,BkpSizeX);

	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(10,WHITE,MYGRAY,0.39); _DrawLeftUp_AA(100,10,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(4,WHITE,MYGRAY, 0.47);_DrawLeftUp_AA(100,25,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(2,WHITE,MYGRAY, 0.39);_DrawLeftUp_AA(100,50,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(1,WHITE,MYGRAY, 0.39); _DrawLeftUp_AA(100,100,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(2,WHITE,MYGRAY, 0.39); _DrawLeftUp_AA(50,100,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(4,WHITE,MYGRAY, 0.47);_DrawLeftUp_AA(25,100,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(10,WHITE,MYGRAY,0.39);_DrawLeftUp_AA(10,100,FrameColor,BkpSizeX);

	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(10,WHITE,MYGRAY,0.39);_DrawLeftDown_AA(100,10,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(4,WHITE,MYGRAY, 0.47);_DrawLeftDown_AA(100,25,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(2,WHITE,MYGRAY, 0.39); _DrawLeftDown_AA(100,50,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(1,WHITE,MYGRAY, 0.39); _DrawLeftDown_AA(100,100,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(2,WHITE,MYGRAY, 0.39);_DrawLeftDown_AA(50,100,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(4,WHITE,MYGRAY, 0.47);_DrawLeftDown_AA(25,100,FrameColor,BkpSizeX);
	_StartDrawLine(0,BkpSizeX,x,y);  Set_AACoeff_Draw(10,WHITE,MYGRAY,0.39);_DrawLeftDown_AA(10,100,FrameColor,BkpSizeX);

	#undef PARAM1
	#undef PARA2
}

void LCD_SimpleTriangle(uint32_t posBuff,uint32_t BkpSizeX, uint32_t x,uint32_t y, uint32_t halfBaseWidth,uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor, DIRECTIONS direct)
{																										/*	'halfBaseWidth/height' or 'height/halfBaseWidth'  must be divisible */
	int i;
	int coeff = halfBaseWidth > height ? halfBaseWidth/height : height/halfBaseWidth;

	if(FillColor != 0) FrameColor=FillColor;
	Set_AACoeff_Draw(coeff,FrameColor,BkpColor, 0.1);
	_StartDrawLine(0,BkpSizeX,x,y);

	switch((int)direct)
	{
		case Right:
			_DrawRightDown_AA(height,halfBaseWidth,FrameColor,BkpSizeX);
			_DrawLeftDown_AA(height,halfBaseWidth,FrameColor,BkpSizeX);
			if(FillColor != 0)
			{
				if(halfBaseWidth > height)
				{
					for(i=coeff; i<halfBaseWidth; ++i){
						_DrawRight(i/coeff+1,FillColor); _IncDrawPos(-(i/coeff+1)-BkpSizeX);
					}
					for(int j=i; j>0; --j){
						_DrawRight(j/coeff+1,FillColor); _IncDrawPos(-(j/coeff+1)-BkpSizeX);
					}
				}
				else
				{
					for(i=0; i<halfBaseWidth; ++i){
						_DrawRight(coeff*i,FillColor); _IncDrawPos(-(coeff*i)-BkpSizeX);
					}
					for(int j=i; j>0; --j){
						_DrawRight(coeff*j,FillColor); _IncDrawPos(-(coeff*j)-BkpSizeX);
					}
				}
			}
			else _DrawUp(2*halfBaseWidth,FrameColor,BkpSizeX);
			break;

		case Left:
			_DrawLeftDown_AA(height,halfBaseWidth,FrameColor,BkpSizeX);
			_DrawRightDown_AA(height,halfBaseWidth,FrameColor,BkpSizeX);
			if(FillColor != 0)
			{
				if(halfBaseWidth > height)
				{
					for(i=coeff; i<halfBaseWidth; ++i){
						_DrawLeft(i/coeff+1,FillColor); _IncDrawPos((i/coeff+1)-BkpSizeX);
					}
					for(int j=i; j>0; --j){
						_DrawLeft(j/coeff+1,FillColor); _IncDrawPos((j/coeff+1)-BkpSizeX);
					}
				}
				else
				{
					for(i=0; i<halfBaseWidth; ++i){
						_DrawLeft(coeff*i,FillColor); _IncDrawPos((coeff*i)-BkpSizeX);
					}
					for(int j=i; j>0; --j){
						_DrawLeft(coeff*j,FillColor); _IncDrawPos((coeff*j)-BkpSizeX);
					}
				}
			}
			else _DrawUp(2*halfBaseWidth,FrameColor,BkpSizeX);
			break;

		case Up:
			_DrawRightUp_AA(halfBaseWidth,height,FrameColor,BkpSizeX);
			_DrawRightDown_AA(halfBaseWidth,height,FrameColor,BkpSizeX);
			if(FillColor != 0)
			{
				if(halfBaseWidth > height)
				{
					for(i=coeff; i<halfBaseWidth; ++i){
						_DrawUp(i/coeff+1,FillColor,BkpSizeX); _IncDrawPos((i/coeff+1)*BkpSizeX-1);
					}
					for(int j=i; j>0; --j){
						_DrawUp(j/coeff+1,FillColor,BkpSizeX); _IncDrawPos((j/coeff+1)*BkpSizeX-1);
					}
				}
				else
				{
					for(i=0; i<halfBaseWidth; ++i){
						_DrawUp(coeff*i,FillColor,BkpSizeX); _IncDrawPos((coeff*i)*BkpSizeX-1);
					}
					for(int j=i; j>0; --j){
						_DrawUp(coeff*j,FillColor,BkpSizeX); _IncDrawPos((coeff*j)*BkpSizeX-1);
					}
				}
			}
			else _DrawLeft(2*halfBaseWidth,FrameColor);
			break;

		case Down:
			_DrawRightDown_AA(halfBaseWidth,height,FrameColor,BkpSizeX);
			_DrawRightUp_AA(halfBaseWidth,height,FrameColor,BkpSizeX);
			if(FillColor != 0)
			{
				if(halfBaseWidth > height)
				{
					for(i=coeff; i<halfBaseWidth; ++i){
						_DrawDown(i/coeff+1,FillColor,BkpSizeX); _IncDrawPos(-(i/coeff+1)*BkpSizeX-1);
					}
					for(int j=i; j>0; --j){
						_DrawDown(j/coeff+1,FillColor,BkpSizeX); _IncDrawPos(-(j/coeff+1)*BkpSizeX-1);
					}
				}
				else
				{
					for(i=0; i<halfBaseWidth; ++i){
						_DrawDown(coeff*i,FillColor,BkpSizeX); _IncDrawPos(-(coeff*i)*BkpSizeX-1);
					}
					for(int j=i; j>0; --j){
						_DrawDown(coeff*j,FillColor,BkpSizeX); _IncDrawPos(-(coeff*j)*BkpSizeX-1);
					}
				}
			}
			else _DrawLeft(2*halfBaseWidth,FrameColor);
			break;
	}
}

structPosition LCD_ShapeExample(uint32_t posBuff,uint32_t BkpSizeX, uint32_t x,uint32_t y, uint32_t lineLen, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor, int angleInclination)
{
	structPosition pos={x,y};
	 for(int i=angleInclination; i<360+angleInclination; i+=45)
		 pos = DrawLine(0,pos.x,pos.y, lineLen, i, FrameColor,BkpSizeX, 0.1, 0.1 ,BkpColor,BkpColor);
	 return pos;
}

int ChangeElemSliderColor(SLIDER_PARAMS sel, uint32_t color){
	return ((color&0xFFFFFF) | sel<<24);
}
uint32_t ChangeElemSliderSize(uint16_t width, uint8_t coeffHeightTriang, uint8_t coeffLineBold, uint8_t coeffHeightPtr, uint8_t coeffWidthPtr){
	return ((coeffHeightTriang<<28) | (coeffLineBold<<24) | (coeffHeightPtr<<20) | (coeffWidthPtr<<16) | width);		/*	standard set: (1,6,2,1) */
}
uint32_t SetSpaceTriangLineSlider(uint16_t height, uint16_t param){
	return ((height&0xFFFF) | param<<16);
}
uint32_t SetValType(uint16_t slidPos, uint16_t param){
	return ((slidPos&0xFFFF) | param<<16);
}

SHAPE_PARAMS LCD_SimpleSliderH(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t widthParam, uint32_t heightParam, uint32_t ElementsColor, uint32_t LineColor, uint32_t LineSelColor, uint32_t BkpColor, uint32_t slidPos, int elemSel)
{
	#define TRIANG_HEIGHT	(height / heightTriang_coeff)
	#define LINE_BOLD			(height / lineBold_coeff)
	#define PTR_HEIGHT		(height / heightPtr_coeff)
	#define PTR_WIDTH			(PTR_HEIGHT / widthPtr_coeff)

	SHAPE_PARAMS elements;
	int width  = SHIFT_RIGHT(widthParam,0,FFFF);
	int height = SHIFT_RIGHT(heightParam,0,FFFF);
	int spaceTriangLine = CONDITION(  DelTriang==(int8_t)SHIFT_RIGHT(heightParam,16,FF),  0,  SHIFT_RIGHT(heightParam,16,FF)  );

	int heightTriang_coeff = CONDITION(SHIFT_RIGHT(widthParam,28,F), SHIFT_RIGHT(widthParam,28,F), 2);
	int lineBold_coeff 	  = CONDITION(SHIFT_RIGHT(widthParam,24,F), SHIFT_RIGHT(widthParam,24,F), 6);
	int heightPtr_coeff 	  = CONDITION(SHIFT_RIGHT(widthParam,20,F), SHIFT_RIGHT(widthParam,20,F), 4);
	int widthPtr_coeff 	  = CONDITION(SHIFT_RIGHT(widthParam,16,F), SHIFT_RIGHT(widthParam,16,F), 2);

	int triang_Height = CONDITION(DelTriang==(int8_t)SHIFT_RIGHT(heightParam,16,FF), 0, TRIANG_HEIGHT);
	int triang_Width 	= CONDITION(DelTriang==(int8_t)SHIFT_RIGHT(heightParam,16,FF), 0, height);
	int triangRight_posX = x + width - triang_Height;

	int ptr_height = PTR_HEIGHT;
	int ptr_width 	= PTR_WIDTH;

	int line_Bold 	= CONDITION(LINE_BOLD,LINE_BOLD,1);
	int line_width = width-2*triang_Height - 2*spaceTriangLine;
	int lineSel_posX 	= x + triang_Height + spaceTriangLine;

	int width_sel 	= CONDITION(	Percent==SHIFT_RIGHT(slidPos,16,FF), (MASK(slidPos,FFFF)*line_width)/100, SET_IN_RANGE(MASK(slidPos,FFFF)-lineSel_posX,0,triangRight_posX-spaceTriangLine)	);

	int lineSel_width 	= width_sel 					- ptr_width/2;
	int lineUnSel_posX 	= lineSel_posX + width_sel + ptr_width/2;
	int lineUnSel_width 	= line_width 	- width_sel - ptr_width/2;
	int ptr_posX 			= lineSel_posX + width_sel - ptr_width/2;

	ptr_posX  		 = SET_IN_RANGE( ptr_posX, 		 lineSel_posX, 				triangRight_posX-ptr_width-spaceTriangLine-2 );
	lineSel_width 	 = SET_IN_RANGE( lineSel_width,   0, 				 			  	line_width-ptr_width+1 							  	);
	lineUnSel_posX  =	SET_IN_RANGE( lineUnSel_posX,  lineSel_posX+ptr_width-1, triangRight_posX 									  	);
	lineUnSel_width = SET_IN_RANGE( lineUnSel_width, 0, 							  	line_width-ptr_width 								);

	uint32_t elemColor[NMB_SLIDER_ELEMENTS] = { ElementsColor, ElementsColor, ElementsColor };

	switch(elemSel>>24){
		case LeftSel:	elemColor[0] = (elemSel&0xFFFFFF)|0xFF000000;  break;
		case PtrSel:	elemColor[1] = (elemSel&0xFFFFFF)|0xFF000000;  break;
		case RightSel:	elemColor[2] = (elemSel&0xFFFFFF)|0xFF000000;  break;
	}

	elements.pos[0].x = x;
	elements.pos[0].y = y;
	elements.size[0].w = triang_Height;
	elements.size[0].h = triang_Width;

	elements.pos[1].x = lineSel_posX;
	elements.pos[1].y = y;
	elements.size[1].w = line_width;
	elements.size[1].h = height;

	elements.pos[2].x = triangRight_posX;
	elements.pos[2].y = y;
	elements.size[2].w = triang_Height;
	elements.size[2].h = triang_Width;

	elements.param[0] = width_sel;
	elements.param[1] = line_width;
	elements.param[2] = ptr_width/2;

	if(DelTriang!=(int8_t)SHIFT_RIGHT(heightParam,16,FF))
		LCD_SimpleTriangle	(posBuff,BkpSizeX, 				lineSel_posX-spaceTriangLine, MIDDLE(y,height,triang_Width), 	triang_Width/2,    triang_Height, 	elemColor[0], elemColor[0], BkpColor, 	Left);
	LCD_LineH					(			BkpSizeX, 				lineSel_posX+1,					MIDDLE(y,height,line_Bold), 		lineSel_width, 							LineSelColor, 									line_Bold );
	LCD_LittleRoundRectangle(posBuff,BkpSizeX, BkpSizeY, 	ptr_posX+1, 						MIDDLE(y,height,ptr_height)+1,	ptr_width, 		    ptr_height, 		elemColor[1], elemColor[1], BkpColor);
	if(0<lineUnSel_width-2)
		LCD_LineH				(			BkpSizeX, 				ptr_posX+1+ptr_width,			MIDDLE(y,height,line_Bold), 		lineUnSel_width-2, 						LineColor, 										line_Bold );
	if(DelTriang!=(int8_t)SHIFT_RIGHT(heightParam,16,FF))
		LCD_SimpleTriangle	(posBuff,BkpSizeX, 				triangRight_posX-1, 				MIDDLE(y,height,triang_Width), 	triang_Width/2,  	 triang_Height, 	elemColor[2], elemColor[2], BkpColor, 	Right);

	return elements;

	#undef TRIANG_HEIGHT
	#undef LINE_BOLD
	#undef PTR_HEIGHT
	#undef PTR_WIDTH
}

SHAPE_PARAMS LCD_SimpleSliderV(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t widthParam, uint32_t heightParam, uint32_t ElementsColor, uint32_t LineColor, uint32_t LineSelColor, uint32_t BkpColor, uint32_t slidPos, int elemSel)
{
	#define TRIANG_HEIGHT	(height / heightTriang_coeff)
	#define LINE_BOLD			(height / lineBold_coeff)
	#define PTR_HEIGHT		(height / heightPtr_coeff)
	#define PTR_WIDTH			(PTR_HEIGHT / widthPtr_coeff)

	SHAPE_PARAMS elements;
	int width  = SHIFT_RIGHT(widthParam,0,FFFF);
	int height = SHIFT_RIGHT(heightParam,0,FFFF);
	int spaceTriangLine = CONDITION(  DelTriang==(int8_t)SHIFT_RIGHT(heightParam,16,FF),  0,  SHIFT_RIGHT(heightParam,16,FF)  );

	int heightTriang_coeff = CONDITION(SHIFT_RIGHT(widthParam,28,F), SHIFT_RIGHT(widthParam,28,F), 2);
	int lineBold_coeff 	  = CONDITION(SHIFT_RIGHT(widthParam,24,F), SHIFT_RIGHT(widthParam,24,F), 6);
	int heightPtr_coeff 	  = CONDITION(SHIFT_RIGHT(widthParam,20,F), SHIFT_RIGHT(widthParam,20,F), 4);
	int widthPtr_coeff 	  = CONDITION(SHIFT_RIGHT(widthParam,16,F), SHIFT_RIGHT(widthParam,16,F), 2);

	int triang_Height = CONDITION(DelTriang==(int8_t)SHIFT_RIGHT(heightParam,16,FF), 0, TRIANG_HEIGHT);
	int triang_Width 	= CONDITION(DelTriang==(int8_t)SHIFT_RIGHT(heightParam,16,FF), 0, height);
	int triangRight_posY = y + width - triang_Height;

	int ptr_height = PTR_HEIGHT;
	int ptr_width 	= PTR_WIDTH;

	int line_Bold 	= CONDITION(LINE_BOLD,LINE_BOLD,1);
	int line_width = width-2*triang_Height - 2*spaceTriangLine;
	int lineSel_posY 	= y + triang_Height + spaceTriangLine;

	int width_sel 	= CONDITION(	Percent==SHIFT_RIGHT(slidPos,16,FF), (MASK(slidPos,FFFF)*line_width)/100, SET_IN_RANGE(MASK(slidPos,FFFF)-lineSel_posY,0,triangRight_posY-spaceTriangLine)	);

	int lineSel_width 	= width_sel 					- ptr_width/2;
	int lineUnSel_posY 	= lineSel_posY + width_sel + ptr_width/2;
	int lineUnSel_width 	= line_width 	- width_sel - ptr_width/2;
	int ptr_posY 			= lineSel_posY + width_sel - ptr_width/2;

	ptr_posY  		 = SET_IN_RANGE( ptr_posY, 		 lineSel_posY, 				triangRight_posY-ptr_width-spaceTriangLine-2 );
	lineSel_width 	 = SET_IN_RANGE( lineSel_width,   0, 				 			  	line_width-ptr_width+1 							  	);
	lineUnSel_posY  =	SET_IN_RANGE( lineUnSel_posY,  lineSel_posY+ptr_width-1, triangRight_posY 									  	);
	lineUnSel_width = SET_IN_RANGE( lineUnSel_width, 0, 							  	line_width-ptr_width 								);

	uint32_t elemColor[NMB_SLIDER_ELEMENTS] = { ElementsColor, ElementsColor, ElementsColor };

	switch(elemSel>>24){
		case LeftSel:	elemColor[0] = (elemSel&0xFFFFFF)|0xFF000000;  break;
		case PtrSel:	elemColor[1] = (elemSel&0xFFFFFF)|0xFF000000;  break;
		case RightSel:	elemColor[2] = (elemSel&0xFFFFFF)|0xFF000000;  break;
	}

	elements.pos[0].x = x;
	elements.pos[0].y = y;
	elements.size[0].w = triang_Height;
	elements.size[0].h = triang_Width;

	elements.pos[1].x = x;
	elements.pos[1].y = lineSel_posY;
	elements.size[1].w = line_width;
	elements.size[1].h = height;

	elements.pos[2].x = x;
	elements.pos[2].y = triangRight_posY;
	elements.size[2].w = triang_Height;
	elements.size[2].h = triang_Width;

	elements.param[0] = width_sel;
	elements.param[1] = line_width;
	elements.param[2] = ptr_width/2;

	if(DelTriang!=(int8_t)SHIFT_RIGHT(heightParam,16,FF))
		LCD_SimpleTriangle	(posBuff,BkpSizeX, 				MIDDLE(x,height,triang_Width), lineSel_posY-spaceTriangLine, 	triang_Width/2,    triang_Height, 	elemColor[0], elemColor[0], BkpColor, 	Up);
	LCD_LineV					(			BkpSizeX, 				MIDDLE(x,height,line_Bold), 	 lineSel_posY+1,						lineSel_width, 							LineSelColor, 									line_Bold );
	LCD_LittleRoundRectangle(posBuff,BkpSizeX, BkpSizeY, 	MIDDLE(x,height,ptr_height)+1, ptr_posY+1, 					 		ptr_height,			 ptr_width, 		elemColor[1], elemColor[1], BkpColor);
	if(0<lineUnSel_width-2)
		LCD_LineV				(			BkpSizeX, 				MIDDLE(x,height,line_Bold), 	 ptr_posY+1+ptr_width,				lineUnSel_width-2, 						LineColor, 										line_Bold );
	if(DelTriang!=(int8_t)SHIFT_RIGHT(heightParam,16,FF))
		LCD_SimpleTriangle	(posBuff,BkpSizeX, 				MIDDLE(x,height,triang_Width), triangRight_posY-1, 				triang_Width/2,  	 triang_Height, 	elemColor[2], elemColor[2], BkpColor, 	Down);

	return elements;

	#undef TRIANG_HEIGHT
	#undef LINE_BOLD
	#undef PTR_HEIGHT
	#undef PTR_WIDTH
}

uint32_t SetLineBold2Width(uint32_t width, uint8_t bold){
	return SHIFT_LEFT(width,bold,16);
}
uint32_t SetTriangHeightCoeff2Height(uint32_t height, uint8_t coeff){
	return SHIFT_LEFT(height,coeff,16);
}
SHAPE_PARAMS LCD_Arrow(uint32_t posBuff,uint32_t bkpSizeX,uint32_t bkpSizeY, uint32_t x,uint32_t y, uint32_t width,uint32_t height, uint32_t frameColor, uint32_t fillColor, uint32_t bkpColor, DIRECTIONS direct)
{
	SHAPE_PARAMS params = {.bkSize.x=bkpSizeX, .bkSize.y=bkpSizeY, .pos[0].x=x, .pos[0].y=y, .size[0].w=width, .size[0].h=height, .color[0].frame=frameColor, .color[0].fill=fillColor, .color[0].bk=bkpColor, .param[0]=direct};

	if(ToStructAndReturn == posBuff)
		return params;

	posBuff = posBuff & 0x7FFFFFFF;

	int heightShape = MASK(height,FFFF);
	int widthShape = MASK(width,FFFF);

	int boldLine = SHIFT_RIGHT(width,16,FF);
	int widthTiang = 0, heightTiang = 0, lenLine = 0;

	void _CalcHeightTriang(void){
		switch(SHIFT_RIGHT(height,16,FF))
		{	default:
			case 0: heightTiang = widthTiang;   break;
			case 1: heightTiang = widthTiang*2; break;
			case 2: heightTiang = widthTiang*3; break;
			case 3: heightTiang = widthTiang/2; break;
			case 4: heightTiang = widthTiang/4; break; }
	}

	switch((int)direct)
	{
		case Right:
			widthTiang = heightShape;
			_CalcHeightTriang();
			lenLine = CONDITION(widthShape>heightTiang, widthShape-heightTiang, 1);
			LCD_LineH(bkpSizeX, x,  MIDDLE(y,widthTiang,boldLine), lenLine,  frameColor, boldLine);
			LCD_SimpleTriangle(posBuff,bkpSizeX, x+lenLine, y, widthTiang/2,heightTiang, frameColor, fillColor, bkpColor, direct);
			break;

		case Left:
			widthTiang = heightShape;
			_CalcHeightTriang();
			lenLine = CONDITION(widthShape>heightTiang, widthShape-heightTiang, 1);
			LCD_LineH(bkpSizeX, x+heightTiang,  MIDDLE(y,widthTiang,boldLine), lenLine,  frameColor, boldLine);
			LCD_SimpleTriangle(posBuff,bkpSizeX, x+heightTiang, y, widthTiang/2,heightTiang, frameColor, fillColor, bkpColor, direct);
			break;

		case Up:
			widthTiang = widthShape;
			_CalcHeightTriang();
			lenLine = CONDITION(heightShape>heightTiang, heightShape-heightTiang, 1);
			LCD_LineV(bkpSizeX, MIDDLE(x,widthTiang,boldLine),  y+heightTiang, lenLine,  frameColor, boldLine);
			LCD_SimpleTriangle(posBuff,bkpSizeX, x, y+heightTiang, widthTiang/2,heightTiang, frameColor, fillColor, bkpColor, direct);
			break;

		case Down:
			widthTiang = widthShape;
			_CalcHeightTriang();
			lenLine = CONDITION(heightShape>heightTiang, heightShape-heightTiang, 1);
			LCD_LineV(bkpSizeX, MIDDLE(x,widthTiang,boldLine),  y, lenLine,  frameColor, boldLine);
			LCD_SimpleTriangle(posBuff,bkpSizeX, x, y+lenLine, widthTiang/2,heightTiang, frameColor, fillColor, bkpColor, direct);
			break;
	}
	return params;
}
SHAPE_PARAMS LCDSHAPE_Arrow(uint32_t posBuff, SHAPE_PARAMS param){
	return LCD_Arrow(posBuff,param.bkSize.x,param.bkSize.y, param.pos[0].x,param.pos[0].y, param.size[0].w,param.size[0].h, param.color[0].frame, param.color[0].fill, param.color[0].bk, param.param[0]);
}

SHAPE_PARAMS LCD_Enter(uint32_t posBuff,uint32_t bkpSizeX,uint32_t bkpSizeY, uint32_t x,uint32_t y, uint32_t width,uint32_t height, uint32_t frameColor, uint32_t fillColor, uint32_t bkpColor)
{
	SHAPE_PARAMS params = {.bkSize.x=bkpSizeX, .bkSize.y=bkpSizeY, .pos[0].x=x, .pos[0].y=y, .size[0].w=width, .size[0].h=height, .color[0].frame=frameColor, .color[0].fill=fillColor, .color[0].bk=bkpColor};

	if(ToStructAndReturn == posBuff)
		return params;

	#define LEN_TAIL	(widthTiang/2)

	posBuff = posBuff & 0x7FFFFFFF;

	int heightShape = MASK(height,FFFF);
	int widthShape = MASK(width,FFFF);

	int boldLine = SHIFT_RIGHT(width,16,FF);
	int widthTiang = 0, heightTiang = 0, lenLine = 0;

	void _CalcHeightTriang(void){
		switch(SHIFT_RIGHT(height,16,FF))
		{	default:
			case 0: heightTiang = widthTiang;   break;
			case 1: heightTiang = widthTiang*2; break;
			case 2: heightTiang = widthTiang*3; break;
			case 3: heightTiang = widthTiang/2; break;
			case 4: heightTiang = widthTiang/4; break; }
	}

	widthTiang = heightShape;
	_CalcHeightTriang();
	lenLine = CONDITION(widthShape>heightTiang, widthShape-heightTiang, 1);
	LCD_LineH(bkpSizeX, x+heightTiang,  		  					MIDDLE(y,widthTiang,(boldLine+1)), 		  				  				 			  lenLine,  frameColor, boldLine);
	LCD_LineV(bkpSizeX, x+heightTiang+lenLine-(boldLine+1),  MIDDLE(y,widthTiang,(boldLine+1))+(boldLine+1)-(LEN_TAIL+(boldLine+1)/2), LEN_TAIL+(boldLine+1)/2,  frameColor, boldLine);
	LCD_SimpleTriangle(posBuff,bkpSizeX, x+heightTiang, y, widthTiang/2,heightTiang, frameColor, fillColor, bkpColor, Left);

	return params;
	#undef LEN_TAIL
}
SHAPE_PARAMS LCDSHAPE_Enter(uint32_t posBuff, SHAPE_PARAMS param){
	return LCD_Enter(posBuff,param.bkSize.x,param.bkSize.y, param.pos[0].x,param.pos[0].y, param.size[0].w,param.size[0].h, param.color[0].frame, param.color[0].fill, param.color[0].bk);
}

SHAPE_PARAMS LCD_Exit(uint32_t posBuff,uint32_t bkpSizeX,uint32_t bkpSizeY, uint32_t x,uint32_t y, uint32_t width,uint32_t height, uint32_t frameColor, uint32_t fillColor, uint32_t bkpColor)
{
	SHAPE_PARAMS params = {.bkSize.x=bkpSizeX, .bkSize.y=bkpSizeY, .pos[0].x=x, .pos[0].y=y, .size[0].w=width, .size[0].h=height, .color[0].frame=frameColor, .color[0].fill=fillColor, .color[0].bk=bkpColor};
	if(ToStructAndReturn == posBuff)
		return params;

	int coeff = width > height ? width/height : height/width;
	Set_AACoeff_Draw(coeff,frameColor,bkpColor, 0.39);

	_StartDrawLine(0,bkpSizeX, x ,y-1);		  _DrawRightDown_AA(width,  height,  frameColor,bkpSizeX);
	_StartDrawLine(0,bkpSizeX, x, y);		  _DrawRightDown_AA(width+1,height+1,frameColor,bkpSizeX);
	_StartDrawLine(0,bkpSizeX, x, y+1);		  _DrawRightDown_AA(width,  height,	 frameColor,bkpSizeX);

	_StartDrawLine(0,bkpSizeX, x+width, y-1);  _DrawLeftDown_AA(width,  height,  frameColor,bkpSizeX);
	_StartDrawLine(0,bkpSizeX, x+width, y);	 _DrawLeftDown_AA(width+1,height+1,frameColor,bkpSizeX);
	_StartDrawLine(0,bkpSizeX, x+width, y+1);  _DrawLeftDown_AA(width,  height,  frameColor,bkpSizeX);
	return params;
}
SHAPE_PARAMS LCDSHAPE_Exit(uint32_t posBuff, SHAPE_PARAMS param){
	return LCD_Exit(posBuff,param.bkSize.x,param.bkSize.y, param.pos[0].x,param.pos[0].y, param.size[0].w,param.size[0].h, param.color[0].frame, param.color[0].fill, param.color[0].bk);
}

SHAPE_PARAMS LCDSHAPE_Window(ShapeFunc pShape, uint32_t posBuff, SHAPE_PARAMS param){
	return pShape(posBuff,param);
}

void LCD_SetCircleParam(float outRatio, float inRatio, int len, ...){
	va_list va;
	va_start(va,0);
	if(len<MAX_LINE_BUFF_CIRCLE_SIZE){
		Circle.lineBuff[0]=len;
		for(int i=0;i<len;i++){
			if(i>=MAX_LINE_BUFF_CIRCLE_SIZE) break;
			Circle.lineBuff[1+i]=va_arg(va,int);
		}
		Circle.outRatioStart=outRatio;
		Circle.inRatioStart=inRatio;
	}
	va_end(va);
}
void LCD_SetCircleDegrees(int len, ...){
	int i,deg;
	va_list va;
	va_start(va,0);
	for(i=0;i<len;i++){
		if(i==MAX_DEGREE_CIRCLE) break;
		Circle.degree[1+i]=va_arg(va,int);
	}
	Circle.degree[0]=i;
	va_end(va);

	for(i=0;i<len;i++)
	{
		deg=Circle.degree[1+i];
		Circle.tang[i]= tan(TANG_ARG(deg));
		Circle.coeff[i] = 1/ABS(Circle.tang[i]);

		if(deg>0 && deg<=45)
			Circle.rot[i]=1;
		else if(deg>45 && deg<=90)
			Circle.rot[i]=2;
		else if(deg>90 && deg<=135)
			Circle.rot[i]=3;
		else if(deg>135 && deg<=180)
			Circle.rot[i]=4;
		else if(deg>180 && deg<=225)
			Circle.rot[i]=5;
		else if(deg>225 && deg<=270)
			Circle.rot[i]=6;
		else if(deg>270 && deg<=315)
			Circle.rot[i]=7;
		else if(deg>315 && deg<=360)
			Circle.rot[i]=8;
	}

	for(int i=0;i<Circle.degree[0];++i){
		if(Circle.degree[1+i]==0)
			Circle.degree[1+i]=360;
	}
}
void LCD_SetCircleDegreesBuff(int len, uint16_t *buf){
	int i,deg;
	for(i=0;i<len;i++){
		if(i==MAX_DEGREE_CIRCLE) break;
		Circle.degree[1+i]=buf[i];
	}
	Circle.degree[0]=i;

	for(i=0;i<len;i++)
	{
		deg=Circle.degree[1+i];
		Circle.tang[i]= tan(TANG_ARG(deg));
		Circle.coeff[i] = 1/ABS(Circle.tang[i]);

		if(deg>0 && deg<=45)
			Circle.rot[i]=1;
		else if(deg>45 && deg<=90)
			Circle.rot[i]=2;
		else if(deg>90 && deg<=135)
			Circle.rot[i]=3;
		else if(deg>135 && deg<=180)
			Circle.rot[i]=4;
		else if(deg>180 && deg<=225)
			Circle.rot[i]=5;
		else if(deg>225 && deg<=270)
			Circle.rot[i]=6;
		else if(deg>270 && deg<=315)
			Circle.rot[i]=7;
		else if(deg>315 && deg<=360)
			Circle.rot[i]=8;
	}

	for(int i=0;i<Circle.degree[0];++i){
		if(Circle.degree[1+i]==0)
			Circle.degree[1+i]=360;
	}
}
void LCD_SetCircleDegColors(int len, ...){
	int i;
	va_list va;
	va_start(va,0);
	for(i=0;i<len;i++){
		if(i==MAX_DEGREE_CIRCLE) break;
		Circle.degColor[i]=va_arg(va,int);
	}
	va_end(va);
}
void LCD_SetCircleDegColorsBuff(int len, uint32_t *buf){
	int i;
	for(i=0;i<len;i++){
		if(i==MAX_DEGREE_CIRCLE) break;
		Circle.degColor[i]=buf[i];
	}
}
void LCD_SetCirclePercentParam(int len, uint16_t *degBuff, uint32_t *degColorBuff){
	LCD_SetCircleDegreesBuff(len,degBuff);
	LCD_SetCircleDegColorsBuff(len,degColorBuff);
}
void LCD_SetCircleDegree(uint8_t degNr, uint8_t deg){
	if(degNr<MAX_DEGREE_CIRCLE+1) Circle.degree[degNr]=deg;
}
uint16_t LCD_GetCircleDegree(uint8_t degNr){
	if(degNr<MAX_DEGREE_CIRCLE+1) return Circle.degree[degNr];
	else return 0;
}
uint32_t SetParamWidthCircle(uint16_t param, uint32_t width){
	return (width&0xFFFF)|param<<16;
}
uint16_t CenterOfCircle(uint16_t xy, uint16_t width){
	return xy+width/2;
}
void LCD_SetCircleLine(uint8_t lineNr, uint8_t val){
	Circle.lineBuff[lineNr]=val;
}
void LCD_OffsCircleLine(uint8_t lineNr, int offs){
	Circle.lineBuff[lineNr]+=offs;
}
void LCD_SetCircleAA(float outRatio, float inRetio){
	Circle.outRatioStart=outRatio;
	Circle.inRatioStart=inRetio;
}
void LCD_CopyCircleAA(void){
	Circle.outRatioStart_prev=Circle.outRatioStart;
	Circle.inRatioStart_prev=Circle.inRatioStart;
}
void LCD_SetCopyCircleAA(void){
	Circle.outRatioStart=Circle.outRatioStart_prev;
	Circle.inRatioStart=Circle.inRatioStart_prev;
}

uint16_t LCD_GetCircleWidth(void){
	return Circle.width;
}
void LCD_CopyCircleWidth(void){
	Circle.width_prev=Circle.width;
}
void LCD_SetCopyCircleWidth(void){
	Circle.width=Circle.width_prev;
}

uint16_t LCD_CalculateCircleWidth(uint32_t width)
{
	#define buf	 Circle.lineBuff

	uint32_t x=0, y=0;
	int matchWidth=0;
	uint32_t _width=(width&0x0000FFFF)-matchWidth;
	uint32_t height=_width;

	GOTO_ToCalculateRadius:
	  _width=(width&0x0000FFFF)-matchWidth;

	float R=((float)_width)/2;
	uint32_t pxl_width = R/3;

	x=_width/2 + matchWidth/2;
	if(_width%2) x++;
	y=0;

	float _x=(float)x, _y=(float)y;
	float x0=(float)x, y0=(float)y+R;

	float param_y = pow(y0-_y,2);
	float param_x = pow(_x-x0,2);
	float decision = pow(R,2);

	int pxl_line=0,i=0;
	uint8_t block=1;

	void _CorrectDecision(void){
		if(block){	 int ni= CONDITION(R<=Circle.correctForWidth,0,1);
			if(i >= VALPERC(pxl_width,Circle.correctPercDeg[ni])){	block=0;   decision= pow(R+Circle.errorDecision[ni],2);  }
	}}

	buf[0]=pxl_width;
	do{
		_x++;  pxl_line++;
		param_x = pow(_x-x0,2);		_CorrectDecision();
		if((param_x+param_y) > decision){
			_y++;
			param_y = pow(y0-_y,2);
			buf[1+i++]=pxl_line-1;
			pxl_line=1;
		}
	}while(i<pxl_width);

   uint16_t _height=buf[0];
  	for(int i=0;i<buf[0];++i) _height+=buf[buf[0]-i];
  	_height = 2*_height;

	if(height>=_height)
		y= y+(height-_height)/2;
	else{
		matchWidth+=1;
		goto GOTO_ToCalculateRadius;
	}
	return _height;
	#undef buf
}
uint16_t LCD_IncrCircleBold(uint16_t width, uint16_t bold){
	uint16_t width_temp= width-2*bold, bold_temp= bold, i=0;
	while(LCD_CalculateCircleWidth(width-2*INCR_WRAP(bold_temp,1,0,width/2-1)) == LCD_CalculateCircleWidth(width_temp) && 20>i) i++;
	if(20==i) return bold;
	else		 return bold_temp;
}
uint16_t LCD_DecrCircleBold(uint16_t width, uint16_t bold){
	uint16_t width_temp= width-2*bold, bold_temp= bold, i=0;
	while(LCD_CalculateCircleWidth(width-2*DECR_WRAP(bold_temp,1,0,width/2-1)) == LCD_CalculateCircleWidth(width_temp) && 20>i) i++;
	if(20==i) return bold;
	else		 return bold_temp;
}
uint16_t LCD_IncrWrapPercCircleBold(uint16_t radius, uint16_t bold, uint8_t minPerc, uint8_t maxPerc, uint8_t stepPerc){
	uint16_t _bold= bold;
	INIT(minVal, VALPERC(radius,minPerc));
	INIT(maxVal, VALPERC(radius,maxPerc));
	INIT(stepVal,VALPERC(radius,stepPerc));
	if(minVal >_bold) _bold= minVal;
	else{
		INCR_WRAP( _bold, stepVal, minVal, maxVal);
		if(minVal==_bold) _bold= 0;
	}
	return _bold;
}
uint16_t LCD_GetNextIncrCircleWidth(uint32_t width){
	uint32_t width_start= width;
	for(int i=0;	i<20 && width_start<=width; 	width_start=LCD_CalculateCircleWidth(width+i++));
	return width_start;
}
uint16_t LCD_GetNextDecrCircleWidth(uint32_t width){
	uint32_t width_start= width;
	for(int i=0;   i<20 && width_start>=width; 	width_start=LCD_CalculateCircleWidth(width-i++));
	return width_start;
}

structPosition GetCircleMiddPoint(uint16_t *radius){
	structPosition pos = {Circle.x0, Circle.y0};
	*radius = Circle.width;
	return pos;
}

void LCD_Circle(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x, uint32_t y, uint32_t _width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	#define EASY_BOLD_CIRCLE	0==param && thickness
	uint32_t width = _width&0xFFFF;
	uint16_t param = _width>>16;
	uint8_t thickness = FrameColor>>24;

	if(EASY_BOLD_CIRCLE) LCD_DrawCircle(posBuff,BkpSizeX,BkpSizeY,x,y, _width,height, FrameColor, FrameColor, BkpColor, 0);
	else						LCD_DrawCircle(posBuff,BkpSizeX,BkpSizeY,x,y, _width,height, FrameColor, FillColor,  BkpColor, 0);

	if(thickness){
		LCD_CopyCircleWidth();
   	uint32_t width_new = width-2*thickness;
		int offs= (Circle.width-LCD_CalculateCircleWidth(width_new))/2;
		if(EASY_BOLD_CIRCLE) LCD_DrawCircle(posBuff,BkpSizeX,BkpSizeY,x+offs,y+offs, width_new,width_new, FrameColor, FillColor, FrameColor, 0);
		else						LCD_DrawCircle(posBuff,BkpSizeX,BkpSizeY,x+offs,y+offs, width_new,width_new, FrameColor, FillColor, FillColor,  1);
		LCD_SetCopyCircleWidth();
	}
	#undef EASY_BOLD_CIRCLE
}

void LCD_HalfCircle(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor){
	uint8_t thickness = FrameColor>>24;
	if((thickness==0)||(thickness==255))
		thickness=0;
	else
		thickness=3*thickness;

	if(thickness){
		LCD_DrawHalfCircle(posBuff,BkpSizeX,BkpSizeY,x,y, width,height, FrameColor, FrameColor, BkpColor);
		LCD_CopyCircleWidth();

		int whatRotate= width>>16;
   	uint32_t width_new = (width&0x0000FFFF) - 2*thickness;
   	int offs= (Circle.width-LCD_CalculateCircleWidth(width_new))/2;

		LCD_DrawHalfCircle(posBuff,BkpSizeX,BkpSizeY,x+offs,y+offs, SetParamWidthCircle(whatRotate,width_new),width_new, FrameColor, FillColor, FrameColor);
		LCD_SetCopyCircleWidth();
	}
	else LCD_DrawHalfCircle(posBuff,BkpSizeX,BkpSizeY,x,y, width,height, FrameColor, FillColor, BkpColor);
}

