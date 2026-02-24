
#ifndef TOUCH_H_
#define TOUCH_H_

#include "stm32f7xx_hal.h"
#include "common.h"

typedef enum{
//	Coeff_A1 = 1070,
//	Coeff_B1 = -26479,
//	Coeff_A2 = 1085,
//	Coeff_B2 = -26279
	Coeff_A1 = 1074,
	Coeff_B1 = -28966,
	Coeff_A2 = 1129,
	Coeff_B2 = -46096
}DEFAULT_TOUCH_COEFF_CALIBR;

#define SERVICE_TOUCH_PROB_TIME_MS	20

#define TOUCH_GET_PER_X_PROBE		3
#define TOUCH_GET_PER_ANY_PROBE		1

#define END_FREEROLL__NOSEL		-1

#define CHECK_TOUCH(screenTouchState)		CHECK_bit(FILE_NAME(SelTouch)[screenTouchState/32],(screenTouchState-32*(screenTouchState/32)-1))
#define SET_TOUCH(screenTouchState) 		SET_bit(FILE_NAME(SelTouch)[screenTouchState/32],(screenTouchState-32*(screenTouchState/32)-1))
#define CLR_TOUCH(screenTouchState) 		RST_bit(FILE_NAME(SelTouch)[screenTouchState/32],(screenTouchState-32*(screenTouchState/32)-1))
#define CLR_ALL_TOUCH 							for(int i=0;i<SEL_BITS_SIZE;++i) FILE_NAME(SelTouch)[i]=0
#define GET_TOUCH 								FILE_NAME(SelTouch)[0]!=0 || FILE_NAME(SelTouch)[1]!=0 || FILE_NAME(SelTouch)[2]!=0 || FILE_NAME(SelTouch)[3]!=0 || FILE_NAME(SelTouch)[4]!=0		/* determine by 'SEL_BITS_SIZE' */

enum TOUCH_TYPE{
  ID_TOUCH_POINT,
  ID_TOUCH_POINT_WITH_HOLD,				/* if press at least specific time */
  ID_TOUCH_POINT_RELEASE_WITH_HOLD,		/* if press and release in only specific range time */
  ID_TOUCH_MOVE_LEFT,
  ID_TOUCH_MOVE_RIGHT,
  ID_TOUCH_MOVE_UP,
  ID_TOUCH_MOVE_DOWN,
  ID_TOUCH_GET_ANY_POINT,
  ID_TOUCH_GET_ANY_POINT_WITH_WAIT		/* buttons touch for counting */
};

enum TOUCH_PRESS_RELEASE{
  release,
  press,
  pressRelease,
  neverMind,
  checkPress
};

typedef enum{
  TouchSetNew,
  TouchUpdate,
}TOUCH_SET_UPDATE;

typedef struct{
  uint16_t x;
  uint16_t y;
}XY_Touch_Struct;

extern uint8_t touchDetect;
extern XY_Touch_Struct  touchTemp[];
extern XY_Touch_Struct  structXY_Zero;

extern XY_Touch_Struct screenTouchPos;
extern uint16_t 		  screenTouchState;
extern uint16_t 		  screenTouchStatePrev, screenTouchStatePrev2;

void		LCD_TOUCH_Service					(void);
uint16_t LCD_TOUCH_GetTypeAndPosition	(XY_Touch_Struct *posXY);
void 		LCD_TOUCH_DeleteAllSetTouch	(void);
int 		GetTouchToTemp						(uint16_t idx);
int 		LCD_TOUCH_SetNewPos				(uint16_t idx, uint16_t x, uint16_t y, uint16_t xLen, uint16_t yLen);
void 		LCD_TOUCH_DeleteSelectTouch	(uint16_t idx);
void 		DeleteAllTouchWithout			(uint16_t idx);

void LCD_TOUCH_SusspendAllTouchsWithout(uint16_t start_idx, uint16_t stop_idx);
void LCD_TOUCH_SusspendAllTouchs(void);
void LCD_TOUCH_SusspendTouch(uint16_t idx);
void LCD_TOUCH_SusspendTouchs2(uint16_t idx1,uint16_t idx2,uint16_t idx3,uint16_t idx4,uint16_t idx5,uint16_t idx6,uint16_t idx7,uint16_t idx8,uint16_t idx9,uint16_t idx10);
void LCD_TOUCH_SusspendNmbTouch(int nmb, uint16_t start_idx);
void LCD_TOUCH_SusspendTouchs(uint16_t start_idx, uint16_t stop_idx);
void LCD_TOUCH_RestoreAllSusspendedTouchs(void);
void LCD_TOUCH_RestoreSusspendedTouch(uint16_t idx);
void LCD_TOUCH_RestoreSusspendedTouchs2(uint16_t idx1,uint16_t idx2,uint16_t idx3,uint16_t idx4,uint16_t idx5,uint16_t idx6,uint16_t idx7,uint16_t idx8,uint16_t idx9,uint16_t idx10);
void LCD_TOUCH_RestoreSusspendedTouchs(uint16_t start_idx, uint16_t stop_idx);
void LCDTOUCH_ActiveOnly(uint16_t idx1,uint16_t idx2,uint16_t idx3,uint16_t idx4,uint16_t idx5,uint16_t idx6,uint16_t idx7,uint16_t idx8,uint16_t idx9,uint16_t idx10, uint16_t start_idx, uint16_t stop_idx);
void LCD_TOUCH_DeleteSelectAndSusspendTouch(uint16_t idx);

int LCD_TOUCH_Set(uint16_t ID, uint16_t idx, uint8_t param);
int LCD_TOUCH_Update(uint16_t ID, uint16_t idx, uint8_t param);
int LCD_TOUCH_isPress(void);

XY_Touch_Struct LCD_TOUCH_GetPos		(void);
uint8_t 			 LCD_TOUCH_testFunc	(GET_SET action);
void 				 LCDTOUCH_testFunc	(void);

void SetLogXY(XY_Touch_Struct *pos, uint16_t *width, int maxSize);
void SetPhysXY(XY_Touch_Struct *pos, int maxSize);
void WaitForTouchState(uint8_t Pressed);
int LCD_TOUCH_CalcCoeffCalibr(int maxSize);
uint8_t IsCalibrationDone(void);
void CalibrationWasDone(void);
void DisplayCoeffCalibration(void);
void DisplayTouchPosXY(int touchIdx, XY_Touch_Struct pos, char *txt);
void DisplayAnyTouchPosXY(void);
uint16_t LCD_TOUCH_SetTimeParam_ms(uint16_t time);
int LCD_TOUCH_ScrollSel_Service(uint8_t nr, uint8_t pressRelease, uint16_t *y, uint8_t rollRateCoeff);
int LCD_TOUCH_ScrollSel_SetCalculate(uint8_t nr, uint16_t *offsWin, uint16_t *selWin, uint16_t WinposY, uint16_t heightAll, uint16_t heightKey, uint16_t heightWin);
int LCD_TOUCH_ScrollSel_GetSel(uint8_t nr);
int LCD_TOUCH_ScrollSel_GetRateCoeff(uint8_t nr);
void LCD_TOUCH_ScrollSel_FreeRolling(uint8_t nr, FUNC1_DEF(pFunc), VOID_FUNCTION *pfuncBlocking);
uint8_t LCD_TOUCH_ScrollSel_DetermineRateRoll(uint8_t nr, uint16_t touchState, uint16_t xPos);

int LCDTOUCH_IsScrollPress	 (int IDroll, uint16_t state, XY_Touch_Struct *pos, int timID);
int LCDTOUCH_IsScrollRelease(uint8_t IDroll, FUNC1_DEF(pFunc), VOID_FUNCTION *pfuncBlocking, int timID);

int LCDTOUCH_Set(uint16_t startX, uint16_t startY, uint16_t width, uint16_t height, uint16_t ID, uint16_t idx, uint8_t param);
int LCDTOUCH_Update(uint16_t startX, uint16_t startY, uint16_t width, uint16_t height, uint16_t ID, uint16_t idx, uint8_t param);
int LCDTOUCH_UserStatus(int nr);

void SetTouchFlag		(void);
int  IsSetTouchFlag	(void);
void TouchScreenInit (void);
void _SaveState (void);
/*	void _RstState	 (void); */
void _SaveState2(void);
void _RstState2 (void);
int _WasState(int point);
int _WasStateRange(int point1, int point2);
int _WasStatePrev(int rangeMin,int rangeMax);

//void _RestoreSusspendedTouchsByAnotherClickItem___(int state,int statePrev2, int prev,int prevStart,int prevStop, 	int not1,int not2,int not3,int not4,int not5,int not6,int not7,int not8,int not9,int not10, 		int unblock1,int unblock2,int unblock3,int unblock4,int unblock5,int unblock6,int unblock7,int unblock8,int unblock9,int unblock10);

#endif
