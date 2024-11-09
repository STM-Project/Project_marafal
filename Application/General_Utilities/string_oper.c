/*
 * string_oper.c
 *
 *  Created on: 07.05.2021
 *      Author: RafalMar
 */
#include "stm32f7xx_hal.h"
#include "string_oper.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "double_float.h"
#include "common.h"

#define SIZE_STRBUFF		200
#define SIZE_STRBUFF_OUT	150

static char strBuff[SIZE_STRBUFF]={0};
static char strBuffOut[SIZE_STRBUFF_OUT]={0};
static int idx=0;

char* Int2Str(int value, char freeSign, int maxDigits, int plusMinus)
{
	int i=10,k=1,j,idx_copy;
	int absolutValue;
	char sign;

	idx_copy=idx;

	int _IsSign(void)
	{
		if(value<0)
		{
			switch(plusMinus)
			{
			case Sign_plusMinus:
			case Sign_minus:
				return 1;
			}
		}
		else if(value>0)
		{
			switch(plusMinus)
			{
			case Sign_plusMinus:
			case Sign_plus:
				return 1;
			}
		}
		return 0;
	}

	if(value<0){
		absolutValue=-value;
		sign='-';
	}
	else{
		absolutValue=value;
		sign='+';
	}

	while(1)
	{
		if(absolutValue<i)
		{
		   if(k<maxDigits)
			{
		   	if(idx+maxDigits >= SIZE_STRBUFF){
		   		idx=0;
		   		idx_copy=0;
		   	}

		   	if(Sign_none!=plusMinus)
		   	{
			   	if(Space==freeSign)
			   		strBuff[idx++]=Space;
			   	else
			   	{
			   		if(1==_IsSign())
			   			strBuff[idx++]=sign;
			   		else
			   			strBuff[idx++]=Space;
			   	}
		   	}

		   	if(None!=freeSign)
		   	{
			   	j=maxDigits-k;
			   	memset(&strBuff[idx],freeSign,j);
			   	idx+=j;
		   	}

		   	if(Space==freeSign)
		   	{
		   		if(1==_IsSign())
		   			strBuff[idx-1]=sign;
		   	}
			}
		   else
		   {
		   	if(idx+k >= SIZE_STRBUFF){
		   		idx=0;
		   		idx_copy=0;
		   	}

		   	if(Sign_none!=plusMinus)
		   	{
		   		if(1==_IsSign())
		   			strBuff[idx++]=sign;
		   		else
		   			strBuff[idx++]=Space;
		   	}
		   }
			itoa(absolutValue,&strBuff[idx],10);
			idx+=k;
			break;
		}
		else
		{
			i*=10;
			k++;
		}
	}
	strBuff[idx++]=0;
	return strBuff+idx_copy;
}

char* Float2Str(float value, char freeSign, int maxDigits, int plusMinus, int dec_digits)
{
	int i=10,k=1,j,idx_copy;
	float absolutValue;
	char sign;

	idx_copy=idx;

	int _IsSign(void)
	{
		if(value<0)
		{
			switch(plusMinus)
			{
			case Sign_plusMinus:
			case Sign_minus:
				return 1;
			}
		}
		else if(value>0)
		{
			switch(plusMinus)
			{
			case Sign_plusMinus:
			case Sign_plus:
				return 1;
			}
		}
		return 0;
	}

	if(value<0){
		absolutValue=-value;
		sign='-';
	}
	else{
		absolutValue=value;
		sign='+';
	}

	while(1)
	{
		if(absolutValue<i)
		{
		   if(k<maxDigits)
			{
		   	if(idx+maxDigits+dec_digits+1 >= SIZE_STRBUFF){
		   		idx=0;
		   		idx_copy=0;
		   	}

		   	if(Sign_none!=plusMinus)
		   	{
		   		if(Space==freeSign)
			   		strBuff[idx++]=Space;
			   	else
			   	{
			   		if(1==_IsSign())
			   			strBuff[idx++]=sign;
			   		else
			   			strBuff[idx++]=Space;
			   	}
		   	}

		   	if(None!=freeSign)
		   	{
			   	j=maxDigits-k;
			   	memset(&strBuff[idx],freeSign,j);
			   	idx+=j;
		   	}

		   	if(Space==freeSign)
		   	{
		   		if(1==_IsSign())
		   			strBuff[idx-1]=sign;
		   	}
			}
		   else
		   {
		   	if(idx+k+dec_digits+1 >= SIZE_STRBUFF){
		   		idx=0;
		   		idx_copy=0;
		   	}

		   	if(Sign_none!=plusMinus)
		   	{
		   		if(1==_IsSign())
		   			strBuff[idx++]=sign;
		   		else
		   			strBuff[idx++]=Space;
		   	}
		   }
			float2stri(&strBuff[idx], absolutValue,dec_digits);
			idx+=k+dec_digits+1;
			break;
		}
		else
		{
			i*=10;
			k++;
		}
	}
	strBuff[idx++]=0;
	return strBuff+idx_copy;
}

char* StrAll(int nmbStr, ...)
{
	int i,len,lenOut;
	char *ptr;
	va_list va;

	va_start(va,0);
	strBuffOut[0]=0;

	for(i=0;i<nmbStr;++i)
	{
		ptr=va_arg(va,char*);
		len=strlen(ptr);
		lenOut=strlen(strBuffOut);

		if(lenOut+len>=SIZE_STRBUFF_OUT){
			strncat(strBuffOut,ptr,SIZE_STRBUFF_OUT-lenOut-1);
			break;
		}
		else
			strncat(strBuffOut,ptr,len);
	}

	va_end(va);
	return strBuffOut;
}

void SwapUint16(uint16_t *a, uint16_t *b)
{
	uint16_t a_temp=*a;
	uint16_t b_temp=*b;
	*a=b_temp;
	*b=a_temp;
}

int STRING_GetTheLongestTxt(int nmb, char **txt)
{
	int maxLen=0, maxLen_temp=0, itMaxLen=0;

	for(int i=0; i<nmb; ++i){
		maxLen_temp = strlen(*txt++);
		if(maxLen_temp > maxLen){
			maxLen = maxLen_temp;
			itMaxLen = i;
		}
	}
	return itMaxLen;
}

int STRING_CmpTxt(char* src, char* dst){
	for(int i=0; i<strlen(dst); ++i){
		if(src[i]!=dst[i])
			return 0;
	}
	return 1;
}

char* DispLongNmb(uint32_t nmb, char* bufStr){
	static char buf[20]={0};
	char *ptr=NULL;
	int i=0, n=0;
	uint32_t nmb_ = nmb;
	uint32_t nBillion  = nmb_/_BILLION;
	uint32_t nMillion  = (nmb_ = CONDITION(nBillion, nmb_-nBillion *_BILLION, nmb_)) / _MILLION;
	uint32_t nThousand = (nmb_ = CONDITION(nMillion, nmb_-nMillion *_MILLION, nmb_)) / _THOUSAND;
	uint32_t nUnity 	 = (nmb_ = CONDITION(nThousand,nmb_-nThousand*_THOUSAND,nmb_));

	buf[0]= None;
	if(nBillion){
		n = CONDITION(nBillion<10,1,CONDITION(IS_RANGE(nBillion,10,99),2,CONDITION(nBillion>99,3,0)));
		ptr = Int2Str(nBillion,Space,n,Sign_none);
		for(int j=0;j<n;++j) buf[i++]=*(ptr+j);
		buf[i++] = Space;	}

	if(nBillion|nMillion){
		n = CONDITION(nMillion<10,1,CONDITION(IS_RANGE(nMillion,10,99),2,CONDITION(nMillion>99,3,0)));
		ptr = Int2Str(nMillion,CONDITION(nBillion,Zero,Space),n,Sign_none);
		for(int j=0;j<n;++j) buf[i++]=*(ptr+j);
		buf[i++] = Space;	}

	if(nBillion|nMillion|nThousand){
		n = CONDITION(nThousand<10,1,CONDITION(IS_RANGE(nThousand,10,99),2,CONDITION(nThousand>99,3,0)));
		ptr = Int2Str(nThousand,CONDITION(nBillion|nMillion,Zero,Space),n,Sign_none);
		for(int j=0;j<n;++j) buf[i++]=*(ptr+j);
		buf[i++] = Space;	}

	n = CONDITION(nUnity<10,1,CONDITION(IS_RANGE(nUnity,10,99),2,CONDITION(nUnity>99,3,0)));
	ptr = Int2Str(nUnity,CONDITION(nBillion|nMillion|nThousand,Zero,Space),n,Sign_none);
	for(int j=0;j<n;++j) buf[i++]=*(ptr+j);
	buf[i++]= None;

	if(NULL != bufStr){
		for(int j=0; j<strlen(buf)+1; ++j) *(bufStr+j)=buf[j];
		return bufStr;
	}
	else return buf;
}
