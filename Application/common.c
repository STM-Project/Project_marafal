/*
 * common.c
 *
 *  Created on: Aug 31, 2024
 *      Author: maraf
 */

#include "stm32f7xx_hal.h"
#include "common.h"

#define _VAL_SIZE		10

int* 			_Int		(int nr, int val)			{ static int 		_intVal[_VAL_SIZE]={0}; 	_intVal[nr]=val;  	return &_intVal[nr]; 	};
uint16_t*	_Uint16	(int nr, uint16_t val)	{ static uint16_t _uint16val[_VAL_SIZE]={0}; _uint16val[nr]=val;  return &_uint16val[nr]; };
uint32_t* 	_Uint32	(int nr, uint32_t val)	{ static uint32_t _uint32val[_VAL_SIZE]={0}; _uint32val[nr]=val;  return &_uint32val[nr]; };
int16_t*		_Int16	(int nr, int16_t val)	{ static int16_t  _int16val[_VAL_SIZE]={0}; 	_int16val[nr]=val;  	return &_int16val[nr]; 	};
int32_t* 	_Int32	(int nr, int32_t val)	{ static int32_t  _int32val[_VAL_SIZE]={0}; 	_int32val[nr]=val;  	return &_int32val[nr]; 	};
float*		_Float	(int nr, float val)		{ static float 	_floatVal[_VAL_SIZE]={0}; 	_floatVal[nr]=val;  	return &_floatVal[nr]; 	};

int _ReturnVal (int val, int in)							{ return val; };
int _ReturnVal2(int val, int in1, int in2)			{ return val; };
int _ReturnVal3(int val, int in1, int in2,int in3)	{ return val; };

int _RetVal	(int in, 						int val)	{ return val; };
int _RetVal2(int in1,int in2, 			int val)	{ return val; };
int _RetVal3(int in1,int in2,	int in3, int val)	{ return val; };

void Int16ToCharBuff(char* buff, uint16_t val){
	buff[0] = val>>8;
	buff[1] = val;
}
void Int32ToCharBuff(char* buff, uint32_t val){
	buff[0] = val>>24;
	buff[1] = val>>16;
	buff[2] = val>>8;
	buff[3] = val;
}
uint16_t CharBuffToInt16(char* buff){
	return ((buff[0]<<8) | buff[1]);
}
uint32_t CharBuffToInt32(char* buff){
	return ((buff[0]<<24) | (buff[1]<<16) | (buff[2]<<8) | buff[3]);
}

int FV(VARIABLE_ACTIONS type, int nrMem, int val){
	static int mem[10];
	switch((int)type){
		case SetVal:
			mem[nrMem]=val;
			return mem[nrMem];
		case GetVal:
			return mem[nrMem];
		default:
			return 0;
}}
int FV2(char* descr, VARIABLE_ACTIONS type, int nrMem, int val){
	static int mem[10];
	switch((int)type){
		case SetVal:
			mem[nrMem]=val;
			return mem[nrMem];
		case GetVal:
			return mem[nrMem];
		default:
			return 0;
}}
