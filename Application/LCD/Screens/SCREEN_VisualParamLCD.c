
#include "SCREEN_VisualParamLCD.h"
#include "SCREEN_ReadPanel.h"
#include "Keyboard.h"
#include "lang.h"
#include "timer.h"
#include "_debug.h"
#include "touch.h"
#include <string.h>

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

/*------------ End Main Settings -----------------*/

/*------------ Main Screen MACROs -----------------*/
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
//ZROBIC szablon z macro by dla kazdego pliku szybko skopioowac !!!!!!!!!!!!!!!
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
/*------------ End Main Screen MACRO -----------------*/

#define CHECK_TOUCH(state)		CHECK_bit(FILE_NAME(SelTouch)[state/32],(state-32*(state/32)-1))
#define SET_TOUCH(state) 		SET_bit(FILE_NAME(SelTouch)[state/32],(state-32*(state/32)-1))
#define CLR_TOUCH(state) 		RST_bit(FILE_NAME(SelTouch)[state/32],(state-32*(state/32)-1))
#define CLR_ALL_TOUCH 			for(int i=0;i<SEL_BITS_SIZE;++i) FILE_NAME(SelTouch)[i]=0
#define GET_TOUCH 				FILE_NAME(SelTouch)[0]!=0 || FILE_NAME(SelTouch)[1]!=0 || FILE_NAME(SelTouch)[2]!=0 || FILE_NAME(SelTouch)[3]!=0 || FILE_NAME(SelTouch)[4]!=0		/* determine by 'SEL_BITS_SIZE' */

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
	Touch_Q,Touch_W,Touch_E,Touch_R,Touch_T,Touch_Y,Touch_U,Touch_I,Touch_O,Touch_P,Touch_A,Touch_S,Touch_D,Touch_F,Touch_G,Touch_H,Touch_J,Touch_K,TOouch_L,Touch_big,Touch_Z,Touch_X,Touch_C,Touch_V,Touch_B,Touch_N,Touch_M,Touch_back,Touch_alt,Touch_exit,Touch_space,Touch_comma,Touch_dot,Touch_enter,Touch_field,Touch_keyStyle
}TOUCH_POINTS;		/* MAX_OPEN_TOUCH_SIMULTANEOUSLY */

typedef enum{
	KEY_NO_RELEASE,
	KEY_All_release,
	KEY_Select_one,
	KEY_Timer,
	KEY_Timer2,

	KEY_Param_1,
	KEY_Param_2,

	KEY_Q,KEY_W,KEY_E,KEY_R,KEY_T,KEY_Y,KEY_U,KEY_I,KEY_O,KEY_P,KEY_A,KEY_S,KEY_D,KEY_F,KEY_G,KEY_H,KEY_J,KEY_K,KEY_L,KEY_big,KEY_Z,KEY_X,KEY_C,KEY_V,KEY_B,KEY_N,KEY_M,KEY_back,KEY_alt,KEY_exit,KEY_space,KEY_comma,KEY_dot,KEY_enter,KEY_field,KEY_style,
}SELECT_PRESS_BLOCK;

typedef enum{
	KEYBOARD_none,
	KEYBOARD_setTxt,
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
	v.FONT_ID_Title 	 		= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(Title),	  	FILE_NAME(GetDefaultParam)(FONT_ID_Title));
	v.FONT_ID_FontColor		= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(FontColor),	FILE_NAME(GetDefaultParam)(FONT_ID_FontColor));
	v.FONT_ID_BkColor 		= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(BkColor),  	FILE_NAME(GetDefaultParam)(FONT_ID_BkColor));
	v.FONT_ID_FontType 		= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(FontType), 	FILE_NAME(GetDefaultParam)(FONT_ID_FontType));
	v.FONT_ID_FontSize 		= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(FontSize), 	FILE_NAME(GetDefaultParam)(FONT_ID_FontSize));
	v.FONT_ID_FontStyle  	= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(FontStyle),	FILE_NAME(GetDefaultParam)(FONT_ID_FontStyle));

	v.FONT_ID_Coeff 			= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(Coeff), 		 FILE_NAME(GetDefaultParam)(FONT_ID_Coeff));
	v.FONT_ID_LenWin 			= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(LenWin), 		 FILE_NAME(GetDefaultParam)(FONT_ID_LenWin));
	v.FONT_ID_OffsWin 		= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(OffsWin), 	 FILE_NAME(GetDefaultParam)(FONT_ID_OffsWin));
	v.FONT_ID_LoadFontTime 	= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(LoadFontTime),FILE_NAME(GetDefaultParam)(FONT_ID_LoadFontTime));
	v.FONT_ID_PosCursor 		= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(PosCursor), 	 FILE_NAME(GetDefaultParam)(FONT_ID_PosCursor));
	v.FONT_ID_CPUusage 		= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(CPUusage), 	 FILE_NAME(GetDefaultParam)(FONT_ID_CPUusage));
	v.FONT_ID_Speed 			= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(Speed), 		 FILE_NAME(GetDefaultParam)(FONT_ID_Speed));
	v.FONT_ID_Press 			= LCD_LoadFont_DependOnColors( LOAD_FONT_PARAM(Press), 		 FILE_NAME(GetDefaultParam)(FONT_ID_Press));
*/
}

static void FRAMES_GROUP_combined(int argNmb, int startOffsX,int startOffsY, int offsX,int offsY, int bold)
{

}

static void FRAMES_GROUP_separat(int argNmb, int startOffsX,int startOffsY, int offsX,int offsY, int boldFrame)		/* Parameters ..Offs.. is counted from STR (not from FRAME) */
{

}

/* ------------ FILE_NAME() functions ------------ */
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
	#define CASE_TOUCH_STATE(state,touchPoint, src,dst, txt,coeff, touchX, touchX2) \
		case touchPoint:\
		if(NotServiceTouchAboveWhenWasClearedThis(touchX) & NotServiceTouchAboveWhenWasClearedThis(touchX2)){\
			if(0==CHECK_TOUCH(state)){\
				if(GET_TOUCH){ FILE_NAME(main)(LoadPartScreen,(char**)ppMain); CLR_ALL_TOUCH; }\
				SELECT_CURRENT_FONT(src, dst, txt, coeff);\
				SET_TOUCH(state);\
				SetFunc();\
			}\
			else{\
				FILE_NAME(main)(LoadPartScreen,(char**)ppMain);\
				KEYBOARD_TYPE(KEYBOARD_none,0);\
				CLR_TOUCH(state);\
			}}

	static uint16_t statePrev=0, statePrev2=0;
	uint16_t state, function=0;
	XY_Touch_Struct pos;

	void _SaveState (void){ statePrev =state; }
/*	void _RstState	 (void){ statePrev =0; 		} */
	void _SaveState2(void){ statePrev2=state; }
	void _RstState2 (void){ statePrev2=0; 		}

	int _WasState(int point){
		if(release==LCD_TOUCH_isPress() && point==statePrev){
			statePrev = state;
			return 1;
		}
		else return 0;
	}
	int _WasStateRange(int point1, int point2){
		if(release==LCD_TOUCH_isPress() && IS_RANGE(statePrev,point1,point2)){
			statePrev = state;
			return 1;
		}
		else return 0;
	}
	int _WasStatePrev(int rangeMin,int rangeMax){
		return (IS_RANGE(statePrev,rangeMin,rangeMax) && statePrev!=state);
	}

	void SetFunc(void){
		function=1;
	}
	int IsFunc(void){
		if(function){
			function=0;
			return 1;
		}
		return 0;
	}

	int NotServiceTouchAboveWhenWasClearedThis(TOUCH_POINTS touch){
		return CONDITION(NoTouch==touch, 1, !CHECK_TOUCH(touch) && !_WasState(touch));
	}
	void _TouchService(TOUCH_POINTS touchStart,TOUCH_POINTS touchStop, KEYBOARD_TYPES keyboard, SELECT_PRESS_BLOCK releaseAll,SELECT_PRESS_BLOCK keyStart, TOUCH_FUNC *func){
		if(IS_RANGE(state, touchStart, touchStop)){
			int nr = state-touchStart;
			if(releaseAll){  if(_WasStatePrev(touchStart,touchStop)) KEYBOARD_TYPE(keyboard,releaseAll);  }
			if(func) func(nr);
			if(KEY_Select_one==keyStart) nr=0;
			KEYBOARD_TYPE_PARAM(keyboard,keyStart+nr,pos.x,pos.y,0,0,0); _SaveState();
	}}
	void _TouchEndService(TOUCH_POINTS touchStart,TOUCH_POINTS touchStop, KEYBOARD_TYPES keyboard, SELECT_PRESS_BLOCK releaseAll, TOUCH_FUNC *func){
		if(_WasStateRange(touchStart, touchStop)){
			KEYBOARD_TYPE(keyboard, releaseAll);
			if(func) func(-1);
	}}
	void CreateKeyboard(KEYBOARD_TYPES keboard){
		switch((int)keboard){
			case KEYBOARD_Param_1:	break;
			case KEYBOARD_Param_2:	break;
	}}
	void _RestoreSusspendedTouchsByAnotherClickItem(TOUCH_POINTS prev,TOUCH_POINTS prevStart,TOUCH_POINTS prevStop, 	TOUCH_POINTS not1,TOUCH_POINTS not2,TOUCH_POINTS not3,TOUCH_POINTS not4,TOUCH_POINTS not5,TOUCH_POINTS not6,TOUCH_POINTS not7,TOUCH_POINTS not8,TOUCH_POINTS not9,TOUCH_POINTS not10, 		TOUCH_POINTS unblock1,TOUCH_POINTS unblock2,TOUCH_POINTS unblock3,TOUCH_POINTS unblock4,TOUCH_POINTS unblock5,TOUCH_POINTS unblock6,TOUCH_POINTS unblock7,TOUCH_POINTS unblock8,TOUCH_POINTS unblock9,TOUCH_POINTS unblock10){
		if(state){
			if((prev==statePrev2 || IS_RANGE(statePrev2,prevStart,prevStop)) && (prev!=state && !IS_RANGE(state,prevStart,prevStop)) && (not1!=state && not2!=state && not3!=state && not4!=state && not5!=state && not6!=state && not7!=state && not8!=state && not9!=state && not10!=state)){
				LCD_TOUCH_RestoreSusspendedTouchs2(unblock1,unblock2,unblock3,unblock4,unblock5,unblock6,unblock7,unblock8,unblock9,unblock10);
				statePrev2=0;
	}}}
	int _KEYBOARD_setTxt__SERVICE(u16 state,int touchStart,int touchStop, int keyStart){
			  if( IS_RANGE(state,touchStart,touchStop))									 			{	if(_WasStatePrev(touchStart,touchStop)) KEYBOARD_TYPE(KEYBOARD_setTxt,KEY_All_release);								 KEYBOARD_TYPE(KEYBOARD_setTxt, keyStart+(state-touchStart));  _SaveState();	return 1;  }
		else if(_WasStateRange(Touch_exit,Touch_exit) && _SET==LCDTOUCH_UserStatus(_GET)){	LCD_TOUCH_RestoreAllSusspendedTouchs(); ServiceKeyCharBuff(); 	FILE_NAME(main)(LoadPartScreen,(char**)ppMain);	 KEYBOARD_TYPE(KEYBOARD_none,0);	 															return 1;  }
		else if(_WasStateRange(touchStart,touchStop))									 			{																																						 KEYBOARD_TYPE(KEYBOARD_setTxt, KEY_All_release);  									return 1;  }
		return 0;
	}


	state = LCD_TOUCH_GetTypeAndPosition(&pos);
													/*if prevTouch is this... and actualTouch is not this...*/				/*and yet actualTouch not this...*/							/*then unblock touches this...*/
	//_RestoreSusspendedTouchsByAnotherClickItem();		/* depended on _SaveState2() */

	/*	----- Service press specific Keys for Keyboard ----- */


	switch(state)
	{
		/*	----- Initiation new Keyboard ----- */

		/*	----- Touch parameter text and go to action ----- */


		default:		 /* ----- Service release specific Keys for Keyboard ----- */
			break;
	}

/*	FILE_NAME(timer)();	*/
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

		LoadFonts(FONT_ID_Title, FONT_SIZE_Param_1);
	}



	LCD_TxtShadowInit(fontVar_40, v.FONT_ID_Param_1, v.COLOR_BkScreen, BK_Rectangle);
	LCD_Txt(Display, NULL, 0,0, LCD_X,LCD_Y, v.FONT_ID_Param_1, fontVar_40, 20,200, SL(LANG_nazwa_0), BLACK, 0/*v.COLOR_BkScreen*/, fullHight,0,250, NoConstWidth, TXTSHADECOLOR_DEEP_DIR(0x777777,4,RightDown) /*TXTSHADE_NONE*/);

}


