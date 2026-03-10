
#include "SCREEN_VisualParamLCD.h"
#include "SCREEN_ReadPanel.h"
#include "Keyboard.h"
#include "lang.h"
#include "timer.h"
#include "_debug.h"
#include "touch.h"
#include <string.h>

/*--------------- Main Macro Settings ------------------*/
#define FILE_NAME(extend) SCREEN_VisualParam_##extend

#define SCREEN_VISUALPARAM_LANG \
	X(LANG_nazwa_0, "LANG_nazwa_0", "LANG_name_0") \
	X(LANG_nazwa_1, "LANG_nazwa_1", "LANG_name_1") \

#define SCREEN_VISUALPARAM_SET_PARAMETERS \
/* id   name							default value */ \
	X(0, FONT_SIZE_Title, 	 		FONT_24_bold) \
	X(1, FONT_SIZE_Descr, 	 		FONT_12) \
	X(2, FONT_SIZE_Press, 	 		FONT_14) \
	X(3, FONT_SIZE_Param_1, 	 	FONT_14) \
	\
	X(4, FONT_STYLE_Title, 			Arial) \
	X(5, FONT_STYLE_Descr, 	 		Arial) \
	X(6, FONT_STYLE_Press, 	 		Arial) \
	X(7, FONT_STYLE_Param_1, 		Arial) \
	\
	X(8, FONT_COLOR_Title,  		WHITE) \
	X(9, FONT_COLOR_Descr, 	 		WHITE) \
	X(0, FONT_COLOR_Press, 	 		WHITE) \
	X(11, FONT_COLOR_Param_1, 		WHITE) \
	\
	X(12, FONT_BKCOLOR_Title,  	MYGRAY2) \
	X(13, FONT_BKCOLOR_Descr, 	 	MYGRAY2) \
	X(14, FONT_BKCOLOR_Press, 	 	MYGRAY2) \
	X(15, FONT_BKCOLOR_Param_1, 	MYGRAY2) \
	\
	X(16, FONT_ID_Title,				fontID_1) \
	X(17, FONT_ID_Descr,				fontID_2) \
	X(18, FONT_ID_Press,				fontID_3) \
	X(19, FONT_ID_Param_1,			fontID_4) \
	\
	X(20, FONT_VAR_Title,			fontVar_1) \
	X(21, FONT_VAR_Descr,			fontVar_2) \
	X(22, FONT_VAR_Press,			fontVar_3) \
	X(23, FONT_VAR_Param_1,			fontVar_4) \
	\
	X(24, COLOR_BkScreen,  			COLOR_GRAY(0x38)) \
	X(25, COLOR_MainFrame,  		COLOR_GRAY(0xD0)) \
	X(26, COLOR_FillMainFrame, 	COLOR_GRAY(0x31)) \
	X(27, COLOR_Frame,  				COLOR_GRAY(0xD0)) \
	X(28, COLOR_FillFrame, 			COLOR_GRAY(0x3B)) \
	X(29, COLOR_FramePress, 		COLOR_GRAY(0xBA)) \
	X(30, COLOR_FillFramePress,	COLOR_GRAY(0x60)) \
	X(31, DEBUG_ON,  					1) \
	X(32, BK_FONT_ROUND,  			1) \
	X(33, LANG_SELECT,  				Polish) \

/*------------ Main Functions of the MACROs -----------------*/
#define SL(name)	(char*)FILE_NAME(Lang)[ v.LANG_SELECT==Polish ? 2*(name) : 2*(name)+1 ]

typedef enum{
	#define X(a,b,c) a,
	SCREEN_VISUALPARAM_LANG
	#undef X
}FILE_NAME(Lang_enum);

static const char *FILE_NAME(Lang)[]={
	#define X(a,b,c) b"\x00",c"\x00",
		SCREEN_VISUALPARAM_LANG
	#undef X
};

typedef enum{
	#define X(a,b,c) b,
	SCREEN_VISUALPARAM_SET_PARAMETERS
	#undef X
}FILE_NAME(enum);

typedef struct{
	#define X(a,b,c) int b;
	SCREEN_VISUALPARAM_SET_PARAMETERS
	#undef X
}FILE_NAME(struct);

static FILE_NAME(struct) v ={
	#define X(a,b,c) c,
		SCREEN_VISUALPARAM_SET_PARAMETERS
	#undef X
};

#define SEL_BITS_SIZE	5
static uint32_t FILE_NAME(SelBits)[SEL_BITS_SIZE] = {0};
static uint32_t FILE_NAME(SelTouch)[SEL_BITS_SIZE] = {0};
/*
static int FILE_NAME(SetDefaultParam)(int param){
	int temp;
	#define X(a,b,c) \
		if(b==param){ v.b=c; temp=c; }
		SCREEN_VISUALPARAM_SET_PARAMETERS
	#undef X
		return temp;
}
*/
static int FILE_NAME(GetDefaultParam)(int param){
	int temp;
	#define X(a,b,c) \
		if(b==param) temp=c;
	SCREEN_VISUALPARAM_SET_PARAMETERS
	#undef X
		return temp;
}

void FILE_NAME(printInfo)(void){
	if(v.DEBUG_ON){
		Dbg(1,Clr_ CoG2_"\r\ntypedef struct{\r\n"_X);
		DbgVar2(1,200,CoGr_"%*s %*s %*s %s\r\n"_X, -8,"id", -18,"name", -15,"default value", "value");
		#define X(a,b,c) DbgVar2(1,200,CoGr_"%*d"_X	"%*s" 	CoGr_"= "_X	 	"%*s" 	"(%s0x%x)\r\n",-4,a,		-23,getName(b),	-15,getName(c), 	CHECK_bit( FILE_NAME(SelBits)[a/32], (a-32*(a/32)) )?CoR_"change to: "_X:"", v.b);
		SCREEN_VISUALPARAM_SET_PARAMETERS
		#undef X
		DbgVar(1,200,CoG2_"}%s;\r\n"_X,getName(FILE_NAME(struct)));
	}
}

int FILE_NAME(funcGet)(int offs){
	return *( (int*)((int*)(&v) + offs) );
}

void FILE_NAME(funcSet)(int offs, int val){
	*( (int*)((int*)(&v) + offs) ) = val;
	SET_bit( FILE_NAME(SelBits)[offs/32], (offs-32*(offs/32)) );
}

void FILE_NAME(setDefaultAllParam)(int rst){
	#define X(a,b,c) FILE_NAME(funcSet)(b,c);
	SCREEN_VISUALPARAM_SET_PARAMETERS
	#undef X
	if(rst){
		for(int i=0;i<SEL_BITS_SIZE;++i)
			FILE_NAME(SelBits)[i]=0;
	}
}

void FILE_NAME(debugRcvStr)(void);
void FILE_NAME(setTouch)(void);

void 	FILE_NAME(main)(int argNmb, char **argVal);

/*------------ Alternative Defines and Functions -----------------*/
#define NONE_TYPE_REQ	-1
#define MAX_NUMBER_OPENED_KEYBOARD_SIMULTANEOUSLY		20
/* #define TOUCH_MAINFONTS_WITHOUT_DESCR */

#define SELECT_CURRENT_FONT(src,dst,txt,coeff) \
	LCD_SetStrVar_fontID		(v.FONT_VAR_##src, v.FONT_ID_##dst);\
	LCD_SetStrVar_fontColor	(v.FONT_VAR_##src, v.FONT_COLOR_##dst);\
	LCD_SetStrVar_bkColor  	(v.FONT_VAR_##src, v.FONT_BKCOLOR_##dst);\
	LCD_SetStrVar_coeff		(v.FONT_VAR_##src, coeff);\
	LCD_StrDependOnColorsVarIndirect(v.FONT_VAR_##src, txt)

#define KEYBUFF_SIZE		500

typedef enum{
	NoTouch = NO_TOUCH,
	Touch_Param_1,
	Touch_Param_2,
	AnyPress,
	AnyPressWithWait,
	KEYBOARD_SETTXT_TOUCHS
}TOUCH_POINTS;		/* MAX_OPEN_TOUCH_SIMULTANEOUSLY */

typedef enum{
	KEY_NO_RELEASE,
	KEY_All_release,
	KEY_Select_one,
	KEY_Timer,
	KEY_Timer2,

	KEY_Param_1,
	KEY_Param_2,
	KEYBOARD_SETTXT_KEYS
}SELECT_PRESS_BLOCK;

typedef enum{
	KEYBOARD_none,
	KEYBOARD_setTxt,
	Touch_NextScreen,
	Touch_PrevScreen,
	KEYBOARD_Param_1,
	KEYBOARD_Param_2,
}KEYBOARD_TYPES;	/* MAX_NUMBER_OPENED_KEYBOARD_SIMULTANEOUSLY */

typedef struct{
	;
} STRUCT_VISUALPARAM;
static STRUCT_VISUALPARAM Test;

static StructTxtPxlLen lenStr;
static KEYBOARD_TYPES actualKeyboardType = KEYBOARD_none;

static void EXPER_FUNC_beforeDispBuffLcd(void);
static void EXPER_FUNC_afterDispBuffLcd(void);

static void FRAMES_GROUP_combined(int argNmb, int startOffsX,int startOffsY, int offsX,int offsY,  int bold);
static void FRAMES_GROUP_separat(int argNmb, int startOffsX,int startOffsY, int offsX,int offsY,  int boldFrame);

static int *ppMain[7] = {(int*)FRAMES_GROUP_combined,(int*)FRAMES_GROUP_separat,(int*)"Rafal", (int*)&Test, NULL, NULL, NULL };
static char keyBuff[KEYBUFF_SIZE]="Test";

static void KEYBOARD_SETTXT_ServiceTxtBuffer(char *buf, int size){
	Dbg(1,"\r\n"); Dbg(1,buf); Dbg(1,"\r\n");
}
static void ResetIndexKeyBuff		  (void){ KEYBOARD_SETTXT_ServiceTxtBuffer(keyBuff,KEYBUFF_SIZE); memset(keyBuff,0,KEYBUFF_SIZE); }
static void CopyKeyBuff2ShowTxtBuff(void){ /* STRING_CopyBuff( buffer,keyBuff, SIZE_TXT_SHOW,strlen(keyBuff) ); */ }
static void ServiceKeyCharBuff	  (void){ CopyKeyBuff2ShowTxtBuff(); ResetIndexKeyBuff(); }

static void LoadFonts(int startFontID, int endFontID){
	if(TakeMutex(Semphr_cardSD,1000))
	{
		#define A(x)	 *((int*)((int*)(&v)+x))

		int d = endFontID-startFontID + 1;
		int j=0;

		for(int i=startFontID; i<=endFontID; ++i){
			*((int*)((int*)(&v)+i)) = LCD_LoadFont_DependOnColors( A(j),A(j+d),A(j+3*d),A(j+2*d), FILE_NAME(GetDefaultParam)(i));
			j++;
		}

		GiveMutex(Semphr_cardSD);
		#undef A
	}
/*
	v.FONT_ID_Title 	 	= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(Title),	FILE_NAME(GetDefaultParam)(FONT_ID_Title));
	v.FONT_ID_Descr		= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(Descr),	FILE_NAME(GetDefaultParam)(FONT_ID_Descr));
	v.FONT_ID_Press 		= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(Press),  	FILE_NAME(GetDefaultParam)(FONT_ID_Press));
*/
}

static StructTxtPxlLen ELEMENT_Param_1(StructFieldPos *field, int xPos,int yPos, int argNmb)
{
	StructTxtPxlLen lenStr = {0};

	*field = LCD_StrDependOnColorsDescrVar_array_xyCorrect(0,STR_FONT_PARAM2(Param_1), xPos, yPos, "Test", fullHight, 0,250, ConstWidth, \
		v.FONT_ID_Descr, v.FONT_COLOR_Descr, v.FONT_BKCOLOR_Descr, 4|(xPos<<16),	Above_left,  SL(LANG_nazwa_0), fullHight, 0,250, NoConstWidth,\
		v.FONT_ID_Descr, v.FONT_COLOR_Descr, v.FONT_BKCOLOR_Descr, 4, 					Left_mid, 	  "7.",  fullHight, 0,250, NoConstWidth, \
		v.FONT_ID_Descr, v.FONT_COLOR_Descr, v.FONT_BKCOLOR_Descr, 4|(xPos<<16),	Under_left,  SL(LANG_nazwa_1), fullHight, 0,250, NoConstWidth, \
		LCD_STR_DESCR_PARAM_NUMBER(3) );

	LCD_SetBkFontShape(v.FONT_VAR_Param_1,BK_LittleRound);

	lenStr.inPixel = field->width;
	lenStr.height 	= field->height;

	return lenStr;
}

static void FRAMES_GROUP_combined(int argNmb, int startOffsX,int startOffsY, int offsX,int offsY, int bold)
{

}

static void FRAMES_GROUP_separat(int argNmb, int startOffsX,int startOffsY, int offsX,int offsY, int boldFrame)		/* Parameters ..Offs.. is counted from STR (not from FRAME) */
{
		#define _FRAME_COLOR		v.COLOR_Frame
		#define _FILL_COLOR		v.COLOR_FillFrame
																										 /* LCD_BoldRoundRectangle */
		#define _Rectan LCD_Shape(field.x-fontsFrameSpace, field.y-fontsFrameSpace, LCD_RoundRectangle, field.width+2*fontsFrameSpace, field.height+2*fontsFrameSpace, SetBold2Color(_FRAME_COLOR,bold), _FILL_COLOR, v.COLOR_FillMainFrame)

		#define _Element(name,nrX,cmdX,Xoffs,nrY,cmdY,Yoffs)	\
				lenStr=ELEMENT_##name(&field, LCD_posX(nrX,lenStr,cmdX,Xoffs), LCD_posY(nrY,lenStr,cmdY,Yoffs), argNmb); \
				_Rectan; \
				lenStr=ELEMENT_##name(&field, LCD_posX(nrX,lenStr,GetPos,0), 	LCD_posY(nrY,lenStr,GetPos,0), 	argNmb); \
				LCD_posY(nrY,lenStr,IncPos,offsY);

		StructFieldPos field={0};
		uint8_t fontsFrameSpace = boldFrame >>8;
		int bold = boldFrame&0x000000FF;

		FILE_NAME(funcSet)(FONT_BKCOLOR_Descr, 	_FILL_COLOR);
		FILE_NAME(funcSet)(FONT_BKCOLOR_Param_1, 	_FILL_COLOR);

		_Element(Param_1,0,SetPos,startOffsX,0,SetPos,startOffsY) 	_Element(Param_1,0,IncPos,offsX,1,SetPos,startOffsY)
		#undef _Element
		#undef _Rectan
		#undef _FRAME_COLOR
		#undef _FILL_COLOR
}

/* ------------ FILE_NAME() functions ------------ */
static void FILE_NAME(timer)(void)  /* alternative RTOS Timer Callback or create new thread vTaskTimer */
{

}

int FILE_NAME(keyboard)(KEYBOARD_TYPES type, SELECT_PRESS_BLOCK selBlockPress, INIT_KEYBOARD_PARAM)
{
	KEYBOARD_SetGeneral(v.FONT_ID_Press, v.FONT_ID_Descr, 	v.FONT_COLOR_Descr,
								  	  	  	  	  	 v.COLOR_MainFrame,  v.COLOR_FillMainFrame,
													 v.COLOR_Frame, 		v.COLOR_FillFrame,
													 v.COLOR_FramePress, v.COLOR_FillFramePress, v.COLOR_BkScreen);

	actualKeyboardType = type;
	if(KEYBOARD_StartUp(type, ARG_KEYBOARD_PARAM)) return 1;

	switch((int)type)
	{
		case KEYBOARD_Param_1:
			break;

		case KEYBOARD_setTxt:
			KEYBOARD__ServiceSetTxt(type-1, selBlockPress, ARG_KEYBOARD_PARAM, KEY_All_release, KEY_Q, KEY_big, KEY_back, KEY_alt, KEY_enter,KEY_field,KEY_style,KEY_exit, v.FONT_COLOR_Descr, keyBuff,KEYBUFF_SIZE);
			break;

		default:
			break;
	}
	return 0;
}

void FILE_NAME(debugRcvStr)(void){	 if(v.DEBUG_ON){

}}

void FILE_NAME(setTouch)(void)
{

	TOUCH_FUNCTIONS_

	TouchScreenInit();

	void CreateKeyboard(KEYBOARD_TYPES keboard){
		switch((int)keboard){
			case KEYBOARD_Param_1:	break;
			case KEYBOARD_Param_2:	break;
	}}


	screenTouchState = LCD_TOUCH_GetTypeAndPosition(&screenTouchPos);

	/*	----- Service press specific Keys for Keyboard ----- */


	switch(screenTouchState)
	{
		/*	----- Initiation new Keyboard ----- */

		/*	----- Touch parameter text and go to action ----- */
	case Touch_NextScreen: SCREEN_SetNr(0); break;
	case Touch_PrevScreen: SCREEN_SetNr(0); break;


		default:		 /* ----- Service release specific Keys for Keyboard ----- */
			break;
	}

	FILE_NAME(timer)();
/*	LCDTOUCH_testFunc(); */

}

void FILE_NAME(main)(int argNmb, char **argVal)
{
	if(NULL == argVal)
		argVal = (char**)ppMain;

	LCD_Clear(v.COLOR_BkScreen);

	if(LoadWholeScreen == argNmb)
	{
		SCREEN_ResetAllParameters();
		LCD_TOUCH_DeleteAllSetTouch();

		DbgVar(v.DEBUG_ON,100, "" Cya_"\r\nStart: %s\r\n"_X, GET_CODE_FUNCTION);

		LoadFonts(FONT_ID_Title, FONT_ID_Param_1);
		DisplayFontsStructState();

		LCDTOUCH_Set( LCD_X-30, LCD_Y/2-30,  30,60,  ID_TOUCH_POINT,Touch_NextScreen,release);
		LCDTOUCH_Set( 30, 		LCD_Y/2-30,  30,60,  ID_TOUCH_POINT,Touch_PrevScreen,release);
	}
	/*FILE_NAME(printInfo)();*/

	INIT(endSetFrame,195);
	LCD_DrawMainFrame(LCD_RoundRectangle,NoIndDisp,0, 0,0, LCD_X,endSetFrame,SHAPE_PARAM(MainFrame,FillMainFrame,BkScreen));

	if		 (*(argVal+0)==(char*)FRAMES_GROUP_combined)
		FRAMES_GROUP_combined(argNmb,15,15,25,25,1);
	else if(*(argVal+0)==(char*)FRAMES_GROUP_separat)
		FRAMES_GROUP_separat(argNmb,15,15,25,25,FRAME_bold2Space(0,6));





	LCD_TxtShadowInit(fontVar_40, v.FONT_ID_Param_1, v.COLOR_BkScreen, BK_Rectangle);
	LCD_Txt(Display, NULL, 0,0, LCD_X,LCD_Y, v.FONT_ID_Param_1, fontVar_40, 20,200, SL(LANG_nazwa_0), BLACK, 0/*v.COLOR_BkScreen*/, fullHight,0,250, NoConstWidth, TXTSHADECOLOR_DEEP_DIR(0x777777,4,RightDown) /*TXTSHADE_NONE*/);

	LCD_StrDependOnColors(v.FONT_ID_Descr, LCD_X-FV(GetVal,0,NoUse), LCD_Y-FV(GetVal,1,NoUse), SL(LANG_nazwa_1), fullHight,0, v.COLOR_FillFrame, v.FONT_COLOR_Descr, 255, NoConstWidth);

	//if(LoadWholeScreen  == argNmb) TxtTouch(TouchSetNew);
	if(LoadNoDispScreen != argNmb) LCD_Show();

}


