 
#ifdef PRZYKLADY_
ZROBIC AUTOMATYCZNE TESTY PO PRZYGOTOWANY UART FUNKCJE SPRAWDZAJACE CALOSC UKLADU JAK ODCZYTY POPRANOSAC ICH ITD !!!!!!!!!!!!!!!!
//https://www.geeksforgeeks.org/x-macros-in-c/
Nie dawaj duzych funkcji wewnatrz funkcji bo trzepie obrazem  -FALSZ !!!!
#line 10

//1 ################## -- xQueueHandle -- ###############################
float archDataVal[100]={0};
xQueueHandle xDataQueue
xDataQueue = xQueueCreate(100, sizeof(float));
xQueueSend(xDataQueue, (void * ) &Channels[channelNumber].value, 0);
while (uxQueueMessagesWaiting(xDataQueue) > 0)
{
	xQueueReceive(xDataQueue, &archDataVal[queueDataSize], 0);
	queueDataSize++;
}

int queueSize = uxQueueMessagesWaiting(xMessageQueue);
for (int i = 0; i < queueSize; i++)
{
	uxQueueMessagesWaiting(xMessageQueue);
	if (xQueueReceive(xMessageQueue, &(eventArchMessage), portMAX_DELAY))
	{
		...
		uxQueueMessagesWaiting(xMessageQueue);
	}
}


xQueueReset(xDataQueue);

void ARCHIVE_SendEvent(char* eventDesc)
{
	char* eventMessage = pvPortMalloc(100);
	if (NULL == eventMessage)
	{

	}
	else
	{
		strncpy(eventMessage,eventDesc,100);
		if(pdPASS != xQueueSend(xMessageQueue, &eventMessage, 20))
			vPortFree(eventMessage);
	}
}

//2 ################## -- xQueueHandle -- ###############################
xSemaphoreHandle xSemaphoreChannels;
uint32_t TakeChannelsMutex(uint32_t timeout)
{
	if(xSemaphoreTake( xSemaphoreChannels, timeout) == pdTRUE)
		return 1;
	else
		return 0;
}
void GiveChannelsMutex(void)
{
	xSemaphoreGive(xSemaphoreChannels);
}
if (TakeChannelsMutex(100)){}

//3 ########################## - continue - ############################
for (int i = 0; i < 16; i++)
{
	if(i==7)
		continue;
}
//4 ############################################################

typedef void MESSAGE_FUNCTION(void);
static MESSAGE_FUNCTION *ackFun=NULL;
CreateConfirmMessage(const char * pText, GUI_COLOR bkColor, GUI_COLOR textColor, MESSAGE_FUNCTION *accept,MESSAGE_FUNCTION *cancel)

lub:

typedef void CONFIRM_FUNCTION(void);

static CONFIRM_FUNCTION *confirmFunction;
void CreateSetConfirm(CONFIRM_FUNCTION *function)
{
	confirmFunction = function;
		.......
	confirmFunction();
}

i gdzies w kodzie:  CreateSetConfirm(CreateMenuWindow);

void CreateMenuWindow(void)
{
	....
}

//5 #################################################################################################################
EventBits_t sendUSBEvent(EventBits_t event)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//	if(NULL == USBGroup)
//		USBGroup = xEventGroupCreate();
	return xEventGroupSetBitsFromISR(USBGroup, event, &xHigherPriorityTaskWoken);
}

//6 #################################################################################################################
void FRAM_ReadMultiple(uint16_t Reg, uint8_t *pBuffer, uint16_t Length)
{
	if(pdTRUE == xSemaphoreTake(xSemaphoreFRAM,100))
	{
		FRAM_I2C_ReadBuffer(Reg, pBuffer, Length);
		xSemaphoreGive(xSemaphoreFRAM);
	}
}
//7 #################################################################################################################
SDRAM MODBUS_TCP_REGISTER ModbusTCPregisters[MAX_MODBUS_TCP_REGISTERS]={0};  // zapis ten NIE inicjalizuje zerami  !!!!
      MODBUS_TCP_REGISTER ModbusTCPregisters[MAX_MODBUS_TCP_REGISTERS]={0};  // zapis ten rzeczywiscie inicjalizuje zerami 
//8 #################################################################################################################
popatrz na screenSaver.c
//9 #################################################################################################################
enum MODBUS_FUNCTION
{
	FUNCTION_NONE,
	READ_HOLDING_REG = 0x03,

};

typedef enum {
	OK,
	IO_DISABLED,
	OVERRANGE,
}INPUT_STATE;

moze byc samo ENUM albo TYPEDEF ENUM by pozniej definiowac np:   static void IO_SetValue(WM_HWIN hItem,INPUT_STATE state, double value, const char resolution)  albo:  INPUT_STATE newState = OK;

//##############################################TASK TIMER ##########################################################
void vTouchpanelTimerCallback(TimerHandle_t pxTimer)
{
	TOUCHPANEL_UpdateState();
}

xTouchpanelTimer = xTimerCreate("TouchPanelTimer", TOUCHPANEL_REFRESH_PERIOD_MS, 1, 0, vTouchpanelTimerCallback);
			xTimerStart(xTouchpanelTimer,TOUCHPANEL_REFRESH_PERIOD_MS);

w funkcji touch:
if(ts.TouchDetected)
{
		PASSWORDS_ResetLogOutTimer();
		    	SCREENSAVER_ResetTimer();
}


//##############################################UNION ##########################################################
struct WM_MESSAGE {
  int MsgId;            /* type of message */
  WM_HWIN hWin;         /* Destination window */
  WM_HWIN hWinSrc;      /* Source window  */
  union {
    const void * p;     /* Message specific data pointer */
    int v;
    GUI_COLOR Color;
    void (* pFunc)(void);
  } Data;
};
REMOTE_GENERAL_SET *pRemoteGeneralSet;
pRemoteGeneralSet = (REMOTE_GENERAL_SET *) pMsg->Data.p;

userMessage.Data.p = &tempEthSettings;
WM_SendMessage(userMessage.hWin, &userMessage);

//#############################################TOUCH TYPE ##########################################################
#define TOUCH_GET_PER_X_PROBE		3

if(0==argNmb)
{
	 	touchTemp[0].x= 0;
	 	touchTemp[0].y= 0;
	 	touchTemp[1].x= touchTemp[0].x+200;
	 	touchTemp[1].y= touchTemp[0].y+150;
	 	SetTouch(ID_TOUCH_POINT,Point_1,press);

	 	touchTemp[0].x= 0;
	 	touchTemp[0].y= 300;
	 	touchTemp[1].x= touchTemp[0].x+200;
	 	touchTemp[1].y= touchTemp[0].y+180;
	 	SetTouch(ID_TOUCH_POINT,Point_2,pressRelease);
//
//	 	touchTemp[0].x= 600;
//	 	touchTemp[0].y= 0;
//	 	touchTemp[1].x= touchTemp[0].x+200;
//	 	touchTemp[1].y= touchTemp[0].y+150;
//	 	SetTouch(ID_TOUCH_POINT,Point_3,release);
//
//	 	touchTemp[0].x= LCD_GetXSize()-LCD_GetXSize()/5;
//	 	touchTemp[1].x= LCD_GetXSize()/5;
//	 	touchTemp[0].y= 150;
//	 	touchTemp[1].y= 300;
//	 	SetTouch(ID_TOUCH_MOVE_LEFT,Move_1,press);
//
//	 	touchTemp[0].x= LCD_GetXSize()/5;
//	 	touchTemp[1].x= LCD_GetXSize()-LCD_GetXSize()/5;
//	 	touchTemp[0].y= 150;
//	 	touchTemp[1].y= 300;
//	 	SetTouch(ID_TOUCH_MOVE_RIGHT,Move_2,release);
//
//	 	touchTemp[0].y= LCD_GetYSize()-LCD_GetYSize()/5;
//	 	touchTemp[1].y= LCD_GetYSize()/5;
//	 	touchTemp[0].x= 300;
//	 	touchTemp[1].x= 450;
//	 	SetTouch(ID_TOUCH_MOVE_UP,Move_3,press);
//
//	 	touchTemp[0].y= LCD_GetYSize()/5;
//	 	touchTemp[1].y= LCD_GetYSize()-LCD_GetYSize()/5;
//	 	touchTemp[0].x= 500;
//	 	touchTemp[1].x= 650;
//	 	SetTouch(ID_TOUCH_MOVE_DOWN,Move_4,release);

	 	touchTemp[0].x= 400;
	 	touchTemp[1].x= 800;
	 	touchTemp[0].y= 240;
	 	touchTemp[1].y= 480;
	 	//SetTouch(ID_TOUCH_GET_ANY_POINT,AnyPress,TOUCH_GET_PER_X_PROBE);
	 	SetTouch(ID_TOUCH_GET_ANY_POINT_WITH_WAIT,AnyPressWithWait,TOUCH_GET_PER_X_PROBE);  //W DEBUG FPNTS WPISZ JESZCZE JAKI LCD_STR !!!!!
}

//---------------------------------------

static char *readBuffer = NULL;

readBuffer = pvPortMalloc(BUFFER_SIZE);

		vPortFree(readBuffer);
		readBuffer = NULL;
		readBuffer = pvPortMalloc(BUFFER_SIZE);

//-------------------------Przekazywanioe funkcji jako argument innej funkcji--------------------
#define _FUNC(func,a,b)	func,a,b

typedef int MESSAGE_FUNCTION(int, int);

int Plus(int a, int b)
{
	return a+b;
}
int Minus(int a, int b)
{
	return a-b;
}

int LCD_TOUCH_ScrollSel_FreeRolling(uint8_t nr, MESSAGE_FUNCTION *x1,int a1,int b1, MESSAGE_FUNCTION *x2,int a2,int b2)
{
	int wynik[2];

	wynik[0] = (int)x1(a1,b1);
	wynik[1] = (int)x2(a2,b2);

	if(wynik[0]==300)
		return 0;
	if(wynik[1]==556)
		return 1;

   return -1;
}

LCD_TOUCH_ScrollSel_FreeRolling(1,_FUNC(Plus,1,2), _FUNC(Minus,5,1));

//------------------------------------------------------------------------------

w CUBE MX dla dnp DMA2 daj nie HAL tylko LL w advence project


///////////////-----------------------------------------------------

  //	char *pRead = NULL;
  //
  //	char tab[50]="Rafal\r\nMarkielowski";
  //	char *ptr = tab;
  //	ptr = strtok_r(txt,"\r\n",&pRead);

//-------------------------PRAGMA LOCATION --------------------

#pragma location=0x30040000
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x30040060
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x30040000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x30040060))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

//##########################################
		for(int i=0; i<countKey; ++i){
			colorTxtKey[i] 	  = colorTxt[i];
			colorTxtPressKey[i] = colorTxtPress[i];
			for(int j=0; j<strlen(*(txtKey+i))+1; ++j)
				*(*(txtKey+i)+j) = *(*(txt+i)+j);
		}
//############################################

do_memp_free_pool(const struct memp_desc *desc, void *mem){  //zobacz rzut w funkcji voiod *mem !!!!!
  struct memp *memp;
  SYS_ARCH_DECL_PROTECT(old_level);

  LWIP_ASSERT("memp_free: mem properly aligned",
              ((mem_ptr_t)mem % MEM_ALIGNMENT) == 0);

  /* cast through void* to get rid of alignment warnings */
  memp = (struct memp *)(void *)((u8_t *)mem - MEMP_SIZE);
}
//############################################
event = xEventGroupWaitBits(ALARMGroup, 0x8, pdTRUE, 0x0, 5);
xEventGroupSetBits(ALARMGroup, ALARM_OFF);
//############################################

if (__fpclassifyd(Channels[channelNumber].totalizers[0].value) != FP_NORMAL)

if (isfinite(Channels[channelNumber].minValue) == 0)

	int status = f_unlink(DeleteFilePath);
//###############----- FF.h --- #####################
res = f_readdir(&dir, &fno); /* Read a directory item */
					if (res != FR_OK || fno.fname[0] == 0)
					{
						strcpy(str+len-1,"]\0");
						netconn_write(conn, data, (size_t )len, NETCONN_NOFLAG);
						break; /* Break on error or end of dir */
					}
					if (fno.fname[0] == '.')
						continue; /* Ignore dot entry */
					if (AM_DIR != (fno.fattrib & AM_DIR))
					{
						dbl2stri(file_size, ((double) fno.fsize) / 1024, 2);
						len += mini_snprintf(str + len,100,"[\"%s\",\"%02u-%02u-%02u %02u:%02u\",\"%s\"],",fno.fname,(fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31, (fno.ftime >> 11),
								((fno.ftime >> 5) & 63), file_size);
					}








FILINFO fno;
FRESULT fresult;
UINT bytesr = 0;
UINT allbytesread = 0;
WORD pParametersCRCWord = 0xFFFF;

char computedParametersCRC[5] =
{ '0', '0', '0', '0', '\0' };
char readParametersCRC[5] =
{ '0', '0', '0', '0', '\0' };

memset(ParametersReadWriteBuffer, 0, BUFFER_SIZE);

fresult = f_open(&ParametersFile, path, FA_READ | FA_WRITE);
f_stat(path,&fno);

if (fresult != FR_OK)
	return 1;
else if (fno.fsize == 0 || fno.fsize >= 10485760)
{
	f_close(&ParametersFile);
	return 2;
}
else
{
	UINT fileSizeWithoutCRC = fno.fsize-CRC_SIZE;
	while (ParametersFile.fptr<fileSizeWithoutCRC)
	{
		memset(ParametersReadWriteBuffer, 0, bytesr);
		f_read(&ParametersFile, ParametersReadWriteBuffer, BUFFER_SIZE, &bytesr);
		allbytesread += bytesr;
		if (ParametersFile.fptr<fileSizeWithoutCRC)
			generateCRC(ParametersReadWriteBuffer, bytesr, &pParametersCRCWord);
		else
		{
			bytesr -= (ParametersFile.fptr - fileSizeWithoutCRC);
			generateCRC(ParametersReadWriteBuffer, bytesr, &pParametersCRCWord);
		}
	}
	hashCRC(computedParametersCRC, &pParametersCRCWord);

	f_lseek(&ParametersFile, fileSizeWithoutCRC);
	f_read(&ParametersFile, readParametersCRC, 5, &bytesr);
	f_close(&ParametersFile);

	if (strcmp(computedParametersCRC, readParametersCRC))
		return 2;
	else
		return 0;
}








UINT bytesReadFromFile = 0;
char *ptrSdram = (char*)(0x60000000);

//char ptrSdram[2000];

char buHARD[3] __attribute__ ((section(".sdram")));

void StartUpTask(void const * argument)
{
/* USER CODE BEGIN StartUpTask */
MX_USB_HOST_Init();

TOUCHPANEL_Init();
MX_FATFS_Init();

res = f_open(&FilData, "DL5.list", FA_READ | FA_OPEN_EXISTING);

//int ddd= f_size(&FilData);


char *ddddd=NULL;

//f_gets(ptrSdram, 1000, &FilData);

dsfdfdf:
res = f_read(&FilData, ptrSdram, 7000000, &bytesReadFromFile);
ddddd=strstr(ptrSdram, "811a48c:");
if(ddddd!=NULL)
{
	f_close(&FilData);
}
else
{
	//f_lseek(&FilData, bytesReadFromFile);

	if(bytesReadFromFile < 7000000)
		goto sdfgdfhggfh;

	goto dsfdfdf;
}
sdfgdfhggfh:
char fff1 = ddddd[24];
char fff2 = ddddd[25];
char fff3 = ddddd[26];
char fff4 = ddddd[27];

f_close(&FilData);

//#################################################


//      	FILINFO fno;
//      	FIL graphicFile;
//      	f_open(&graphicFile, "0:image/blogo.dta", FA_READ);
//      	f_stat("0:image/blogo.dta",&fno);
//      	if( fno.fsize <= 768000)
//      			f_read(&graphicFile, bufferForLogoBigGraphic, fno.fsize, &bytesreadFromLogoBig);
//      	f_close(&graphicFile);




//				res = f_readdir(&dir, &fno); /* Read a directory item */
//				if (res != FR_OK || fno.fname[0] == 0)
//				{
//					strcpy(str+len-1,"]\0");
//					netconn_write(conn, data, (size_t )len, NETCONN_NOFLAG);
//					break; /* Break on error or end of dir */
//				}
//				if (fno.fname[0] == '.')
//					continue; /* Ignore dot entry */
//				if (AM_DIR != (fno.fattrib & AM_DIR))
//				{
//					dbl2stri(file_size, ((double) fno.fsize) / 1024, 2);
//					len += mini_snprintf(str + len,100,"[\"%s\",\"%02u-%02u-%02u %02u:%02u\",\"%s\"],",fno.fname,(fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31, (fno.ftime >> 11),
//							((fno.ftime >> 5) & 63), file_size);
//				}

//######################################################################################################

//		case Touch_LenWin_plus: 		_KEYS_RELEASE_LenOffsWin;	 Inc_lenWin();  					KEYBOARD_TYPE( KEYBOARD_LenOffsWin, KEY_LenWin_plus ); 	 	_SaveState();  break;
//		case Touch_LenWin_minus: 		_KEYS_RELEASE_LenOffsWin;	 Dec_lenWin();  					KEYBOARD_TYPE( KEYBOARD_LenOffsWin, KEY_LenWin_minus ); 	 	_SaveState();  break;
//		case Touch_OffsWin_plus: 		_KEYS_RELEASE_LenOffsWin;	 Inc_offsWin();  					KEYBOARD_TYPE( KEYBOARD_LenOffsWin, KEY_OffsWin_plus );   	_SaveState();  break;
//		case Touch_OffsWin_minus: 		_KEYS_RELEASE_LenOffsWin;	 Dec_offsWin();  					KEYBOARD_TYPE( KEYBOARD_LenOffsWin, KEY_OffsWin_minus );  	_SaveState();  break;
//		case Touch_PosInWin_plus: 		_KEYS_RELEASE_LenOffsWin;	 Dec_PosCursor();  				KEYBOARD_TYPE( KEYBOARD_LenOffsWin, KEY_PosInWin_plus );  	_SaveState();  break;
//		case Touch_PosInWin_minus: 	_KEYS_RELEASE_LenOffsWin;	 Inc_PosCursor();  				KEYBOARD_TYPE( KEYBOARD_LenOffsWin, KEY_PosInWin_minus ); 	_SaveState();  break;
//		case Touch_SpaceFonts_plus: 	_KEYS_RELEASE_LenOffsWin;	 IncDec_SpaceBetweenFont(1);  KEYBOARD_TYPE( KEYBOARD_LenOffsWin, KEY_SpaceFonts_plus );  _SaveState();  break;
//		case Touch_SpaceFonts_minus: 	_KEYS_RELEASE_LenOffsWin;	 IncDec_SpaceBetweenFont(0);  KEYBOARD_TYPE( KEYBOARD_LenOffsWin, KEY_SpaceFonts_minus ); _SaveState();  break;
//		case Touch_DispSpaces: 	_KEYS_RELEASE_LenOffsWin;	 /* here do nothing */  				KEYBOARD_TYPE( KEYBOARD_LenOffsWin, KEY_DispSpaces ); _SaveState();  break;
//		case Touch_WriteSpaces: _KEYS_RELEASE_LenOffsWin;	LCD_WriteSpacesBetweenFontsOnSDcard();  KEYBOARD_TYPE( KEYBOARD_LenOffsWin, KEY_WriteSpaces ); _SaveState();  break;
//		case Touch_ResetSpaces: _KEYS_RELEASE_LenOffsWin;	LCD_ResetSpacesBetweenFonts();  			 KEYBOARD_TYPE( KEYBOARD_LenOffsWin, KEY_ResetSpaces ); _SaveState();  break;
//		case Touch_SpacesInfoUp: 									 /* here do nothing */  				KEYBOARD_TYPE( KEYBOARD_LenOffsWin, KEY_InfoSpacesUp );  break;
//		case Touch_SpacesInfoDown: 								 /* here do nothing */  				KEYBOARD_TYPE( KEYBOARD_LenOffsWin, KEY_InfoSpacesDown ); break;  //gdy reset i pusto to button zawiesza sie !!!!!!
//		case Touch_SpacesInfoStyle: 								 /* here do nothing */  				KEYBOARD_TYPE( KEYBOARD_LenOffsWin, KEY_InfoSpacesStyle ); break;
//		case Touch_SpacesInfoRoll: 								 /* here do nothing */  				KEYBOARD_TYPE_PARAM(KEYBOARD_LenOffsWin,KEY_InfoSpacesRoll,pos.x,pos.y,0,0,0); break;
//		case Touch_SpacesInfoSel: 								 /* here do nothing */  					KEYBOARD_TYPE_PARAM(KEYBOARD_LenOffsWin,KEY_InfoSpacesSel,pos.x,pos.y,0,0,0);  break;

//######################################################################################################




#endif


