 
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
	//##############################################

#endif


