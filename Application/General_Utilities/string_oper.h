/*
 * string_oper.h
 *
 *  Created on: 07.05.2021
 *      Author: RafalMar
 */

#ifndef GENERAL_UTILITIES_STRING_OPER_H_
#define GENERAL_UTILITIES_STRING_OPER_H_

#define _BILLION		1000000000
#define _MILLION		1000000
#define _THOUSAND		1000

typedef enum{
	SetVal,
	GetVal,
	NoUse,
}VARIABLE_ACTIONS;

typedef enum{
	Sign_plusMinus,
	Sign_minus,
	Sign_plus,
	Sign_none
}Int2Str_plusminus;

typedef enum{
	None,
	Space = ' ',
	Zero = '0'
}Int2Str_freeSign;

char* Int2Str(int value, char freeSign, int maxDigits, int plusMinus);
char* Float2Str(float value, char freeSign, int maxDigits, int plusMinus, int dec_digits);
char* StrAll(int nmbStr, ...);

void SwapUint16(uint16_t *a, uint16_t *b);
int STRING_GetTheLongestTxt(int nmb, char **txt);
int STRING_CmpTxt(char* src, char* dst);
char* DispLongNmb(uint32_t nmb, char* bufStr);

#endif /* GENERAL_UTILITIES_STRING_OPER_H_ */
