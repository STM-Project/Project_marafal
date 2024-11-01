/*
 * common.h
 *
 *  Created on: 11.04.2021
 *      Author: Elektronika RM
 */

#ifndef COMMON_H_
#define COMMON_H_
#include "string_oper.h"

#define EXAMPLE(x)  TEST_##x

#define GET_DATE_COMPILATION  __DATE__
#define GET_TIME_COMPILATION  __TIME__
#define GET_CODE_FILE  __FILE__
#define GET_CODE_LINE  __LINE__
#define GET_CODE_FUNCTION  __FUNCTION__

#define GET_CODE_PLACE 	StrAll(4,"\r\n", GET_CODE_FILE, ":", GET_CODE_LINE)

#define TEXT2PRINT(message,type)		(type == 0 ? StrAll(9,"\r\n", GET_CODE_FILE, " : ", GET_CODE_FUNCTION, "() : ", Int2Str(GET_CODE_LINE,None,6,Sign_none), " \"", message,"\"") \
															  : (type == 1  ? StrAll(7,"\r\n", GET_CODE_FUNCTION, "() :", Int2Str(GET_CODE_LINE,None,6,Sign_none), " \"", message,"\"") \
																	  	  	  	 : StrAll(3,"\r\n\"", message,"\"") ))

#define getName(var)  #var
#define ABS(X)  ((X) > 0 ? (X) : -(X))
#define TOOGLE(x)  ((x)=1-(x))
#define INCR(x,step,max)  (((x)+(step)>(max)) ? ((x)=(max)) : ((x)+=(step)))
#define DECR(x,step,min)  (((x)-(step)<(min)) ? ((x)=(min)) : ((x)-=(step)))
#define INCR_WRAP(x,step,min,max)  (((x)+(step)>(max)) ? ((x)=(min)) : ((x)+=(step)))
#define DECR_WRAP(x,step,min,max)  (((x)-(step)<(min)) ? ((x)=(max)) : ((x)-=(step)))
#define INTEGER(f)	 ((int)f)
#define FRACTION(f)	 (f-(int)f)
#define ROUND_VAL(val,fraction)	 ((FRACTION(val)>fraction) ? (INTEGER(val)+1) : (INTEGER(val)));
#define EVEN_INT(x)   ( (x%2) ? (x+1) : (x) )
#define TANG_ARG(deg) 0.0174532*deg
#define DEG(tang_arg) tang_arg/0.0174532

#define INCR_FLOAT_WRAP(x,step,min,max)  ((x<max-step/2) ? (x+=step) : (x=min))
#define DECR_FLOAT_WRAP(x,step,min,max)  ((x<step/2) ?     (x=max)   : (x-=step))

#define MIDDLE(startPos,widthBk,width)	((startPos)+((widthBk)-(width))/2)
#define IS_RANGE(val,min,max) (((val)>=(min))&&((val)<=(max)))
#define EQUAL2(val,eq1,eq2)			((eq1==val)||(eq2==val))

#define SET_IN_RANGE(val,min,max) (((val)<(min))?(min):(((val)>(max))?(max):(val)))
#define SET_NEW_RANGE(val,min,max,newMin,newMax) (((val)<(min))?(newMin):(((val)>(max))?(newMax):(val)))

#define STRUCT_TAB_SIZE(_struct) (sizeof(_struct) / sizeof(&_struct[0]))

#define SET_bit(allBits,bitNr)	((allBits) |=   1<<(bitNr))
#define RST_bit(allBits,bitNr)	((allBits) &= ~(1<<(bitNr)))
#define CHECK_bit(allBits,bitNr) (((allBits)>>((bitNr)>31?31:(bitNr))) & 0x00000001)

#define INIT_MAXVAL(ptr,nmb,minVal,maxValCalc)	 int maxValCalc=0; 	for(int i=0,j=(minVal); i<(nmb); (ptr[i]>j ? j=ptr[i] : 0), (maxValCalc)=j, ++i);
#define MAXVAL(ptr,nmb,minVal,maxValCalc)		for(int i=0,j=(minVal); i<(nmb); (ptr[i]>j ? j=ptr[i] : 0), (maxValCalc)=j, ++i);
#define INIT_INCVAL(nmb,val,valinc)		int val=0; for(int i=0; i<nmb; val+=valinc[i++])
#define INCVAL(nmb,val,valinc)				 val=0; for(int i=0; i<nmb; val+=valinc[i++])

#define BKCOPY_VAL(dst,src,val)	dst=src; src=val
#define BKCOPY(dst,src)	dst=src

#define SDRAM __attribute__ ((section(".sdram")))

#define PTR2CHAR(txt,ptr)	char *ptr
#define PTR2INT(txt,ptr)	int *ptr

#define LCD_BUFF_XSIZE		800
#define LCD_BUFF_YSIZE		480

#define MASK(val,hexMask)		((val)&0x##hexMask)
#define SHIFT_RIGHT(val,shift,hexMask)		(((val)>>(shift))&0x##hexMask)
#define SHIFT_LEFT(val,val2,shift)		((val)|((val2)<<(shift)))
#define DO_NOTHING(x)	((x)=(x))

#define PERCENT_SCALE(val,maxVal)	(((val)*100)/(maxVal))
#define VALPERC(val,perc)		(((perc)*(val))/100)

/* use only in thread 'LCD display' */
#define pCHAR_PLCD(offs)	((char*)(pLcd+(offs)))
#define pUINT8_PLCD(offs)	((uint8_t*)(pLcd+(offs)))
#define pUINT16_PLCD(offs)	((uint16_t*)(pLcd+(offs)))
#define pINT_PLCD(offs)		((int*)(pLcd+(offs)))

#define vCHAR_PLCD(offs)	(*((char*)(pLcd+(offs))))
#define vUINT8_PLCD(offs)	(*((uint8_t*)(pLcd+(offs))))
#define vUINT16_PLCD(offs)	(*((uint16_t*)(pLcd+(offs))))
#define vINT_PLCD(offs)		(*((int*)(pLcd+(offs))))

#define INIT(name,val)	int name=val
#define CONDITION(condition,val1,val2)	((condition) ? (val1) : (val2))
#define MAXVAL2(value1,value2)	((value1) >= (value2) ? (value1) : (value2))
#define MINVAL2(value1,value2)	((value1) <= (value2) ? (value1) : (value2))

#define STRUCT_SIZE_SHAPE_POS		3

#define FUNC_MAIN_INIT		int argNmb, char **argVal
#define FUNC_MAIN_ARG		argNmb,argVal

#define TXT_CUTTOFF(txt,len)	if(strlen(txt)>len-1) *((txt)+(len-1))=0

typedef enum{
	LoadWholeScreen,
	LoadPartScreen,
	LoadUserScreen,
	LoadNoDispScreen
}LOAD_SCREEN;

typedef enum{
	_int,
	_int16,
	_int32,
	_uint16,
	_uint32,
	_float,
	_double,
}DATA_TYPE;

typedef enum{ _Incr,  _Decr, }DATA_ACTION;
typedef enum{ _ON,    _OFF,  }ON_OFF;
typedef enum{ _START, _STOP, }START_STOP;
typedef enum{ _MIN,   _MID,  _MAX,  }MAX_MIN;
typedef enum{ _GET=-2, _SET=-1, }GET_SET;

typedef struct{
	char *pData;
	uint32_t len;
}DATA_TO_SEND;

//typedef struct { I16P x,y; } GUI_POINT;
//typedef struct { I16 x0,y0,x1,y1; } LCD_RECT;

typedef struct{
	int x;
	int y;
}structPosition;

typedef struct{  //to zienic !!!!!!
	int w;
	int h;
}structSize;

typedef struct{
	int frame;
	int fill;
	int bk;
}structColor;

typedef struct{  //to zienic !!!!!!
	int x;
	int y;
}structBkSize;

typedef struct{
	structBkSize 	bkSize;
	structPosition pos[STRUCT_SIZE_SHAPE_POS];
	structSize 		size[STRUCT_SIZE_SHAPE_POS];
	structColor 	color[STRUCT_SIZE_SHAPE_POS];
	uint16_t 		param[STRUCT_SIZE_SHAPE_POS];
	char           *ptr[STRUCT_SIZE_SHAPE_POS];
}SHAPE_PARAMS;

typedef struct{
	structPosition pos;
	structSize 		size;
}POS_SIZE;

/* ----------- Functions Definitations ----------- */
typedef void (*WindowFunc)(uint16_t,uint16_t,uint16_t,uint16_t);

typedef void (*figureShape)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);

typedef int FUNC_Keyboard(uint8_t,uint8_t,figureShape,uint8_t,uint16_t,uint16_t,uint16_t,uint16_t,uint8_t,uint16_t,uint16_t,uint8_t);
#define FUNC1_SET(func,a,b,c,d,e,f,g,h,i,j,k,l)		func,a,b,c,d,e,f,g,h,i,j,k,l
#define FUNC1_DEF(pfunc)		FUNC_Keyboard *pfunc,uint8_t a,uint8_t b,figureShape c,uint8_t d,uint16_t e,uint16_t f,uint16_t g,uint16_t h,uint8_t i,uint16_t j,uint16_t k,uint8_t l
#define FUNC1_ARG		a,b,c,d,e,f,g,h,i,j,k,l

typedef void VOID_FUNCTION(void);
typedef SHAPE_PARAMS (*ShapeFunc)(uint32_t,SHAPE_PARAMS);
typedef void FUNC_MAIN(int, char**);
typedef void TOUCH_FUNC(int);

typedef void* (*VOID_FUNCTION_TEST)(void*,void*);


/* --------- End Functions Definitations ----------- */
int* 			_Int		(int 	val);
uint16_t* 	_Uint16	(uint16_t val);
uint32_t* 	_Uint32	(uint32_t val);
int16_t* 	_Int16	(int16_t val);
int32_t* 	_Int32	(int32_t val);
float* 		_Float	(float val);

int _ReturnVal(int val, int in);
int _ReturnVal2(int val, int in1,int in2);
int _ReturnVal3(int val, int in1,int in2,int in3);

int _RetVal(int val, int in);
int _RetVal2(int val, int in1,int in2);
int _RetVal3(int val, int in1,int in2,int in3);

void Int16ToCharBuff(char* buff, uint16_t val);
void Int32ToCharBuff(char* buff, uint32_t val);
uint16_t CharBuffToInt16(char* buff);
uint32_t CharBuffToInt32(char* buff);
int FV(VARIABLE_ACTIONS type, int nrMem, int val);
int FV2(char* descr, VARIABLE_ACTIONS type, int nrMem, int val);



#endif /* COMMON_H_ */
